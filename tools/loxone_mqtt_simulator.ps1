param(
    [Parameter(Mandatory = $true)]
    [string]$Broker,
    [int]$Port = 1883,
    [string]$Username = "",
    [string]$Password = ""
)

$ErrorActionPreference = "Stop"

# Dieser unabhängige MQTT-3.1.1-Client ist bewusst auf die Funktionen des
# Simulators beschränkt: Veröffentlichen und Abonnieren mit QoS 0.

function ConvertTo-MqttString([string]$Value) {
    $bytes = [Text.Encoding]::UTF8.GetBytes($Value)
    [byte[]](@(
        [byte](($bytes.Length -shr 8) -band 0xFF)
        [byte]($bytes.Length -band 0xFF)
    ) + $bytes)
}

function ConvertTo-RemainingLength([int]$Length) {
    $result = [Collections.Generic.List[byte]]::new()
    do {
        $digit = $Length % 128
        $Length = [Math]::Floor($Length / 128)
        if ($Length -gt 0) { $digit = $digit -bor 0x80 }
        $result.Add([byte]$digit)
    } while ($Length -gt 0)
    $result.ToArray()
}

function Send-MqttPacket([byte]$Header, [byte[]]$Body) {
    $packet = [byte[]](@($Header) + (ConvertTo-RemainingLength $Body.Length) + $Body)
    $script:stream.Write($packet, 0, $packet.Length)
    $script:stream.Flush()
}

function Read-MqttPacket {
    $header = $script:stream.ReadByte()
    if ($header -lt 0) { throw "MQTT connection closed" }

    $multiplier = 1
    $length = 0
    do {
        $digit = $script:stream.ReadByte()
        if ($digit -lt 0) { throw "Incomplete MQTT packet" }
        $length += ($digit -band 0x7F) * $multiplier
        $multiplier *= 128
    } while (($digit -band 0x80) -ne 0)

    $body = [byte[]]::new($length)
    $read = 0
    while ($read -lt $length) {
        $count = $script:stream.Read($body, $read, $length - $read)
        if ($count -le 0) { throw "Incomplete MQTT packet body" }
        $read += $count
    }

    [PSCustomObject]@{ Header = [byte]$header; Body = $body }
}

function Publish-Mqtt([string]$Topic, [string]$Payload, [bool]$Retain = $true) {
    $body = [byte[]]((ConvertTo-MqttString $Topic) + [Text.Encoding]::UTF8.GetBytes($Payload))
    $header = if ($Retain) { [byte]0x31 } else { [byte]0x30 }
    Send-MqttPacket $header $body
    Write-Host ("STATUS  {0} = {1}" -f $Topic, $Payload) -ForegroundColor Green
}

function Subscribe-Mqtt([string]$Topic) {
    $packetId = [byte[]](0x00, 0x01)
    $body = [byte[]]($packetId + (ConvertTo-MqttString $Topic) + [byte]0x00)
    Send-MqttPacket 0x82 $body
}

function Get-PublishContent($Packet) {
    $body = $Packet.Body
    $topicLength = ($body[0] -shl 8) + $body[1]
    $topic = [Text.Encoding]::UTF8.GetString($body, 2, $topicLength)
    $offset = 2 + $topicLength
    $payload = [Text.Encoding]::UTF8.GetString($body, $offset, $body.Length - $offset)
    [PSCustomObject]@{ Topic = $topic; Payload = $payload }
}

function Publish-AllStatus {
    Publish-Mqtt "pool/status/waterTemp" $script:state.WaterTemp
    Publish-Mqtt "pool/status/targetTemp" $script:state.TargetTemp
    Publish-Mqtt "pool/status/filterPump" ([int]$script:state.FilterPump)
    Publish-Mqtt "pool/status/heatingPump" ([int]$script:state.HeatingPump)
    Publish-Mqtt "pool/status/heatingAllowed" ([int]$script:state.HeatingAllowed)
    Publish-Mqtt "pool/status/isHeating" ([int]$script:state.IsHeating)
    Publish-Mqtt "pool/status/mode" $script:state.Mode
}

function Test-TargetTemperature([double]$Value) {
    if ($Value -lt 20.0 -or $Value -gt 32.0) { return $false }
    $steps = ($Value - 20.0) / 0.5
    [Math]::Abs($steps - [Math]::Round($steps)) -lt 0.001
}

function Handle-PanelCommand([string]$Topic, [string]$Payload) {
    Write-Host ("COMMAND {0} = {1}" -f $Topic, $Payload) -ForegroundColor Cyan

    switch ($Topic) {
        "pool/cmd/mode" {
            $mode = 0
            if ([int]::TryParse($Payload, [ref]$mode) -and $mode -ge 0 -and $mode -le 2) {
                $script:state.Mode = $mode
                Publish-Mqtt "pool/status/mode" $mode
            } else {
                Write-Warning "Loxone simulation rejected invalid mode"
            }
        }
        "pool/cmd/targetTemp" {
            $value = 0.0
            $style = [Globalization.NumberStyles]::Float
            $culture = [Globalization.CultureInfo]::InvariantCulture
            if ($script:state.Mode -eq 1 -and
                [double]::TryParse($Payload, $style, $culture, [ref]$value) -and
                (Test-TargetTemperature $value)) {
                $script:state.TargetTemp = $value.ToString("0.0", $culture)
                Publish-Mqtt "pool/status/targetTemp" $script:state.TargetTemp
            } else {
                Write-Warning "Loxone simulation rejected target temperature"
            }
        }
        "pool/cmd/filterPump" {
            if ($script:state.Mode -eq 2 -and ($Payload -eq "0" -or $Payload -eq "1")) {
                $script:state.FilterPump = $Payload -eq "1"
                Publish-Mqtt "pool/status/filterPump" $Payload
            } else {
                Write-Warning "Loxone simulation rejected filter pump command"
            }
        }
    }
}

$script:state = [PSCustomObject]@{
    WaterTemp = "27.4"
    TargetTemp = "29.0"
    FilterPump = $false
    HeatingPump = $false
    HeatingAllowed = $true
    IsHeating = $false
    Mode = 1
}

$clientId = "pool-loxone-simulator-" + [Guid]::NewGuid().ToString("N").Substring(0, 8)
$script:client = [Net.Sockets.TcpClient]::new()
$script:client.Connect($Broker, $Port)
$script:stream = $script:client.GetStream()

$connectFlags = 0x02
$payload = [byte[]](ConvertTo-MqttString $clientId)
if ($Username.Length -gt 0) {
    $connectFlags = $connectFlags -bor 0x80
    $payload = [byte[]]($payload + (ConvertTo-MqttString $Username))
}
if ($Password.Length -gt 0) {
    $connectFlags = $connectFlags -bor 0x40
    $payload = [byte[]]($payload + (ConvertTo-MqttString $Password))
}

$variableHeader = [byte[]](
    (ConvertTo-MqttString "MQTT") +
    [byte]0x04 +
    [byte]$connectFlags +
    [byte]0x00 + [byte]30
)
Send-MqttPacket 0x10 ([byte[]]($variableHeader + $payload))

$connAck = Read-MqttPacket
if (($connAck.Header -shr 4) -ne 2 -or $connAck.Body[1] -ne 0) {
    throw "Broker rejected MQTT connection (code $($connAck.Body[1]))"
}

Subscribe-Mqtt "pool/cmd/#"
Publish-AllStatus

Write-Host ""
Write-Host "Loxone MQTT simulator connected. Keys:" -ForegroundColor Yellow
Write-Host "  A = Automatic mode    M = Manual mode    O = Off"
Write-Host "  H = Toggle isHeating  F = Toggle filter status"
Write-Host "  P = Publish all       Q = Quit"

try {
    while ($true) {
        while ($script:stream.DataAvailable) {
            $packet = Read-MqttPacket
            if (($packet.Header -shr 4) -eq 3) {
                $message = Get-PublishContent $packet
                Handle-PanelCommand $message.Topic $message.Payload
            }
        }

        if ([Console]::KeyAvailable) {
            $key = [Console]::ReadKey($true).Key
            switch ($key) {
                "A" { $script:state.Mode = 1; Publish-Mqtt "pool/status/mode" "1" }
                "M" { $script:state.Mode = 2; Publish-Mqtt "pool/status/mode" "2" }
                "O" { $script:state.Mode = 0; Publish-Mqtt "pool/status/mode" "0" }
                "H" {
                    $script:state.IsHeating = !$script:state.IsHeating
                    Publish-Mqtt "pool/status/isHeating" ([int]$script:state.IsHeating)
                }
                "F" {
                    $script:state.FilterPump = !$script:state.FilterPump
                    Publish-Mqtt "pool/status/filterPump" ([int]$script:state.FilterPump)
                }
                "P" { Publish-AllStatus }
                "Q" { return }
            }
        }

        Start-Sleep -Milliseconds 50
    }
}
finally {
    $script:stream.Dispose()
    $script:client.Dispose()
}
