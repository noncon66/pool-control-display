#!/usr/bin/env python3
"""Einfacher, unabhängiger Loxone-MQTT-Simulator für Entwicklungstests."""

from __future__ import annotations

import argparse
import math
import select
import socket
import struct
import sys
import time
import uuid
from dataclasses import dataclass


def mqtt_string(value: str) -> bytes:
    encoded = value.encode("utf-8")
    return struct.pack("!H", len(encoded)) + encoded


def remaining_length(length: int) -> bytes:
    result = bytearray()
    while True:
        digit = length % 128
        length //= 128
        if length:
            digit |= 0x80
        result.append(digit)
        if not length:
            return bytes(result)


class MqttClient:
    """Minimale MQTT-3.1.1-Verbindung für QoS-0-Publish und -Subscribe."""

    def __init__(
        self,
        host: str,
        port: int,
        username: str,
        password: str,
    ) -> None:
        self.host = host
        self.port = port
        self.username = username
        self.password = password
        self.socket: socket.socket | None = None

    def connect(self) -> None:
        self.socket = socket.create_connection((self.host, self.port), timeout=10)
        self.socket.settimeout(10)

        flags = 0x02  # Clean Session
        payload = mqtt_string(f"pool-loxone-simulator-{uuid.uuid4().hex[:8]}")
        if self.username:
            flags |= 0x80
            payload += mqtt_string(self.username)
        if self.password:
            flags |= 0x40
            payload += mqtt_string(self.password)

        variable_header = mqtt_string("MQTT") + bytes((0x04, flags, 0x00, 30))
        self.send_packet(0x10, variable_header + payload)

        packet_type, body = self.read_packet()
        if packet_type >> 4 != 2 or len(body) != 2 or body[1] != 0:
            code = body[1] if len(body) > 1 else "unknown"
            raise RuntimeError(f"Broker rejected MQTT connection (code {code})")

    def close(self) -> None:
        if self.socket is not None:
            self.socket.close()
            self.socket = None

    def send_packet(self, header: int, body: bytes = b"") -> None:
        if self.socket is None:
            raise RuntimeError("MQTT client is not connected")
        self.socket.sendall(bytes((header,)) + remaining_length(len(body)) + body)

    def publish(self, topic: str, payload: str, retain: bool = True) -> None:
        header = 0x31 if retain else 0x30
        self.send_packet(header, mqtt_string(topic) + payload.encode("utf-8"))
        print(f"\033[32mSTATUS  {topic} = {payload}\033[0m")

    def subscribe(self, topic: str) -> None:
        body = struct.pack("!H", 1) + mqtt_string(topic) + b"\x00"
        self.send_packet(0x82, body)

    def ping(self) -> None:
        self.send_packet(0xC0)

    def has_data(self) -> bool:
        if self.socket is None:
            return False
        readable, _, _ = select.select((self.socket,), (), (), 0)
        return bool(readable)

    def read_exactly(self, length: int) -> bytes:
        if self.socket is None:
            raise RuntimeError("MQTT client is not connected")
        data = bytearray()
        while len(data) < length:
            chunk = self.socket.recv(length - len(data))
            if not chunk:
                raise ConnectionError("MQTT connection closed")
            data.extend(chunk)
        return bytes(data)

    def read_packet(self) -> tuple[int, bytes]:
        if self.socket is None:
            raise RuntimeError("MQTT client is not connected")

        header = self.read_exactly(1)[0]
        multiplier = 1
        length = 0
        while True:
            digit = self.read_exactly(1)[0]
            length += (digit & 0x7F) * multiplier
            if not digit & 0x80:
                break
            multiplier *= 128
        return header, self.read_exactly(length)


@dataclass
class PoolSimulation:
    water_temp: float = 27.4
    target_temp: float = 29.0
    filter_pump: bool = False
    heating_pump: bool = False
    heating_allowed: bool = True
    is_heating: bool = False
    mode: int = 1


def boolean_payload(value: bool) -> str:
    return "1" if value else "0"


def valid_target_temperature(value: float) -> bool:
    if not math.isfinite(value) or not 20.0 <= value <= 32.0:
        return False
    steps = (value - 20.0) / 0.5
    return abs(steps - round(steps)) < 0.001


def publish_all(client: MqttClient, state: PoolSimulation) -> None:
    client.publish("pool/status/waterTemp", f"{state.water_temp:.1f}")
    client.publish("pool/status/targetTemp", f"{state.target_temp:.1f}")
    client.publish("pool/status/filterPump", boolean_payload(state.filter_pump))
    client.publish("pool/status/heatingPump", boolean_payload(state.heating_pump))
    client.publish(
        "pool/status/heatingAllowed",
        boolean_payload(state.heating_allowed),
    )
    client.publish("pool/status/isHeating", boolean_payload(state.is_heating))
    client.publish("pool/status/mode", str(state.mode))


def parse_publish(body: bytes) -> tuple[str, str]:
    topic_length = struct.unpack("!H", body[:2])[0]
    topic = body[2 : 2 + topic_length].decode("utf-8")
    payload = body[2 + topic_length :].decode("utf-8")
    return topic, payload


def handle_command(
    client: MqttClient,
    state: PoolSimulation,
    topic: str,
    payload: str,
) -> None:
    print(f"\033[36mCOMMAND {topic} = {payload}\033[0m")

    if topic == "pool/cmd/mode":
        try:
            mode = int(payload)
        except ValueError:
            mode = -1
        if mode in (1, 2, 3):
            state.mode = mode
            client.publish("pool/status/mode", str(mode))
        else:
            print("WARNING: Loxone simulation rejected invalid mode")

    elif topic == "pool/cmd/targetTemp":
        try:
            value = float(payload)
        except ValueError:
            value = math.nan
        if state.mode == 1 and valid_target_temperature(value):
            state.target_temp = value
            client.publish("pool/status/targetTemp", f"{value:.1f}")
        else:
            print("WARNING: Loxone simulation rejected target temperature")

    elif topic == "pool/cmd/filterPump":
        if state.mode == 2 and payload in ("0", "1"):
            state.filter_pump = payload == "1"
            client.publish("pool/status/filterPump", payload)
        else:
            print("WARNING: Loxone simulation rejected filter pump command")


def read_publish(
    client: MqttClient,
    timeout: float = 2.0,
) -> tuple[int, str, str]:
    """Liest das naechste PUBLISH-Paket und ignoriert Protokollantworten."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if not client.has_data():
            time.sleep(0.01)
            continue

        header, body = client.read_packet()
        if header >> 4 == 3:
            topic, payload = parse_publish(body)
            return header, topic, payload

    raise TimeoutError("Expected MQTT publish was not received")


def wait_for_packet_type(
    client: MqttClient,
    expected_type: int,
    timeout: float = 2.0,
) -> None:
    """Wartet auf eine bestimmte MQTT-Protokollantwort."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if not client.has_data():
            time.sleep(0.01)
            continue

        header, _ = client.read_packet()
        if header >> 4 == expected_type:
            return

    raise TimeoutError(
        f"Expected MQTT packet type {expected_type} was not received",
    )


def synchronize_broker(client: MqttClient) -> None:
    """Bestaetigt, dass der Broker alle vorher gesendeten Pakete verarbeitet hat."""
    client.ping()
    wait_for_packet_type(client, 13)  # PINGRESP


def expect_status(
    client: MqttClient,
    expected_topic: str,
    expected_payload: str,
) -> None:
    _, topic, payload = read_publish(client)
    if (topic, payload) != (expected_topic, expected_payload):
        raise AssertionError(
            "Expected status "
            f"{expected_topic}={expected_payload}, received {topic}={payload}",
        )


def expect_no_publish(client: MqttClient, timeout: float = 0.3) -> None:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if not client.has_data():
            time.sleep(0.01)
            continue

        header, body = client.read_packet()
        if header >> 4 == 3:
            topic, payload = parse_publish(body)
            raise AssertionError(
                f"Unexpected status response {topic}={payload}",
            )


def process_next_command(client: MqttClient, state: PoolSimulation) -> None:
    header, topic, payload = read_publish(client)
    if not topic.startswith("pool/cmd/"):
        raise AssertionError(f"Unexpected command topic {topic}")
    if header & 0x01:
        raise AssertionError(f"Command topic {topic} must not be retained")
    handle_command(client, state, topic, payload)


def run_self_test(args: argparse.Namespace) -> None:
    """Prueft Status- und Befehlsfluss ueber einen echten MQTT-Broker."""
    simulator = MqttClient(args.broker, args.port, args.username, args.password)
    panel = MqttClient(args.broker, args.port, args.username, args.password)
    state = PoolSimulation()

    expected_initial_status = {
        "pool/status/waterTemp": "27.4",
        "pool/status/targetTemp": "29.0",
        "pool/status/filterPump": "0",
        "pool/status/heatingPump": "0",
        "pool/status/heatingAllowed": "1",
        "pool/status/isHeating": "0",
        "pool/status/mode": "1",
    }

    try:
        print("SELF-TEST: connecting Loxone simulator")
        simulator.connect()
        simulator.subscribe("pool/cmd/#")

        print("SELF-TEST: publishing retained initial status")
        publish_all(simulator, state)
        # QoS-0-Publishes besitzen kein PUBACK. Die nachfolgende PINGRESP wird
        # auf derselben Verbindung erst nach allen vorherigen Paketen erzeugt
        # und beseitigt so das Rennen mit dem anschliessenden Panel-Subscribe.
        synchronize_broker(simulator)

        print("SELF-TEST: validating retained initial status")
        panel.connect()
        panel.subscribe("pool/status/#")

        received_status: dict[str, str] = {}
        while len(received_status) < len(expected_initial_status):
            header, topic, payload = read_publish(panel)
            if topic not in expected_initial_status:
                continue
            if not header & 0x01:
                raise AssertionError(f"Initial status {topic} was not retained")
            received_status[topic] = payload

        if received_status != expected_initial_status:
            raise AssertionError(
                "Initial retained status differs: "
                f"expected {expected_initial_status}, received {received_status}",
            )

        print("SELF-TEST: validating accepted commands")
        panel.publish("pool/cmd/mode", "2", retain=False)
        process_next_command(simulator, state)
        expect_status(panel, "pool/status/mode", "2")

        panel.publish("pool/cmd/filterPump", "1", retain=False)
        process_next_command(simulator, state)
        expect_status(panel, "pool/status/filterPump", "1")

        print("SELF-TEST: validating rejected command")
        panel.publish("pool/cmd/targetTemp", "30.0", retain=False)
        process_next_command(simulator, state)
        expect_no_publish(panel)

        print("SELF-TEST: validating target temperature confirmation")
        panel.publish("pool/cmd/mode", "1", retain=False)
        process_next_command(simulator, state)
        expect_status(panel, "pool/status/mode", "1")

        panel.publish("pool/cmd/targetTemp", "30.0", retain=False)
        process_next_command(simulator, state)
        expect_status(panel, "pool/status/targetTemp", "30.0")

        print("SELF-TEST: validating off mode and obsolete zero rejection")
        panel.publish("pool/cmd/mode", "3", retain=False)
        process_next_command(simulator, state)
        expect_status(panel, "pool/status/mode", "3")

        panel.publish("pool/cmd/mode", "0", retain=False)
        process_next_command(simulator, state)
        expect_no_publish(panel)
    finally:
        panel.close()
        simulator.close()

    print("SELF-TEST PASSED: MQTT status and command flow is valid")


class Keyboard:
    """Nicht blockierende Einzelzeicheneingabe für Windows und Unix-Terminals."""

    def __enter__(self) -> "Keyboard":
        self.is_windows = sys.platform == "win32"
        self.original_settings = None
        if not self.is_windows:
            import termios
            import tty

            if not sys.stdin.isatty():
                raise RuntimeError("Interactive keyboard control requires a terminal")
            self.original_settings = termios.tcgetattr(sys.stdin)
            tty.setcbreak(sys.stdin.fileno())
        return self

    def __exit__(self, *_: object) -> None:
        if not self.is_windows and self.original_settings is not None:
            import termios

            termios.tcsetattr(
                sys.stdin,
                termios.TCSADRAIN,
                self.original_settings,
            )

    def read(self) -> str | None:
        if self.is_windows:
            import msvcrt

            return msvcrt.getwch().lower() if msvcrt.kbhit() else None

        readable, _, _ = select.select((sys.stdin,), (), (), 0)
        return sys.stdin.read(1).lower() if readable else None


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--broker", required=True, help="MQTT broker hostname or IP")
    parser.add_argument("--port", type=int, default=1883)
    parser.add_argument("--username", default="")
    parser.add_argument("--password", default="")
    parser.add_argument(
        "--self-test",
        action="store_true",
        help="run a non-interactive broker integration test and exit",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()

    if args.self_test:
        try:
            run_self_test(args)
            return 0
        except (AssertionError, ConnectionError, OSError, RuntimeError) as error:
            print(f"SELF-TEST FAILED: {error}", file=sys.stderr)
            return 1

    client = MqttClient(args.broker, args.port, args.username, args.password)
    state = PoolSimulation()

    try:
        client.connect()
    except (TimeoutError, ConnectionRefusedError, socket.gaierror, OSError) as error:
        print(
            f"ERROR: MQTT broker {args.broker}:{args.port} is not reachable.",
            file=sys.stderr,
        )
        print(
            "Check the broker address, port, network connection, firewall, "
            "and whether the MQTT broker is running.",
            file=sys.stderr,
        )
        print(f"Technical detail: {error}", file=sys.stderr)
        client.close()
        return 2

    client.subscribe("pool/cmd/#")
    publish_all(client, state)

    print("\nLoxone MQTT simulator connected. Keys:")
    print("  A = Automatic mode    M = Manual mode    O = Off")
    print("  H = Toggle isHeating  F = Toggle filter status")
    print("  P = Publish all       Q = Quit")

    last_ping = time.monotonic()
    try:
        with Keyboard() as keyboard:
            while True:
                while client.has_data():
                    packet_type, body = client.read_packet()
                    if packet_type >> 4 == 3:
                        handle_command(client, state, *parse_publish(body))

                key = keyboard.read()
                if key == "a":
                    state.mode = 1
                    client.publish("pool/status/mode", "1")
                elif key == "m":
                    state.mode = 2
                    client.publish("pool/status/mode", "2")
                elif key == "o":
                    state.mode = 3
                    client.publish("pool/status/mode", "3")
                elif key == "h":
                    state.is_heating = not state.is_heating
                    client.publish(
                        "pool/status/isHeating",
                        boolean_payload(state.is_heating),
                    )
                elif key == "f":
                    state.filter_pump = not state.filter_pump
                    client.publish(
                        "pool/status/filterPump",
                        boolean_payload(state.filter_pump),
                    )
                elif key == "p":
                    publish_all(client, state)
                elif key == "q":
                    return 0

                if time.monotonic() - last_ping >= 20:
                    client.ping()
                    last_ping = time.monotonic()

                time.sleep(0.05)
    finally:
        client.close()


if __name__ == "__main__":
    raise SystemExit(main())
