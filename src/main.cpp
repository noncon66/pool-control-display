#include <Arduino.h>

#include "PoolState.h"
#include "DisplayManager.h"
#include "GuiManager.h"

PoolState g_poolState;
DisplayManager g_display;
GuiManager g_gui;

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("=== Pool Control Display ===");
    Serial.println("Starting system...");

    g_poolState.waterTemperature = 27.4f;
    g_poolState.targetTemperature = 29.0f;
    g_poolState.mode = PoolMode::Auto;

    g_display.begin();
    g_gui.begin();

    Serial.println("System started.");
}

void loop()
{
    g_display.loop();
    g_gui.loop();
}