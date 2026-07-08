#include <Arduino.h>
#include "AppController.h"

AppController app;

void setup()
{
    Serial.begin(115200);
    delay(500);
    app.begin();
}

void loop()
{
    app.loop();
}