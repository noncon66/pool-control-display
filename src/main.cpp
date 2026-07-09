#include <Arduino.h>
#include "AppController.h"

// Es gibt genau eine zentrale Anwendungsinstanz. Sie besitzt alle Teilbereiche
// wie WLAN, MQTT und später auch die Benutzeroberfläche.
AppController app;

void setup()
{
    // Das Arduino-Framework ruft setup() nach dem Einschalten genau einmal auf.
    // Die serielle Verbindung hilft bei Statusmeldungen und der Fehlersuche.
    Serial.begin(115200);
    delay(500);
    app.begin();
}

void loop()
{
    // Das Arduino-Framework ruft loop() fortlaufend auf. Lange blockierende
    // Vorgänge sind zu vermeiden, damit WLAN und MQTT regelmäßig bedient werden.
    app.loop();
}
