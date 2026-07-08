#include "DisplayManager.h"

#include <Arduino_GFX_Library.h>

// -----------------------------------------------------------------------------
// IMPORTANT
// -----------------------------------------------------------------------------
// This is an initial bring-up configuration for an ESP32-S3 4.0" 480x480 RGB
// display board. Pin assignments can vary between board revisions.
// The goal of this commit is to get a first image on screen.
// If your board uses different RGB pins, we will adjust them in the next step.
// -----------------------------------------------------------------------------

namespace
{
    constexpr uint16_t SCREEN_WIDTH  = 480;
    constexpr uint16_t SCREEN_HEIGHT = 480;

    // LVGL draw buffer
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t* buf1 = nullptr;
    static lv_color_t* buf2 = nullptr;

    // Global display pointer used by the LVGL flush callback
    Arduino_ESP32RGBPanel* rgbPanel = nullptr;
    Arduino_RGB_Display* gfx = nullptr;
}

bool DisplayManager::begin()
{
    Serial.println("[Display] begin()");

    initDisplayHardware();
    initLvgl();

    Serial.println("[Display] ready");
    return true;
}

void DisplayManager::loop()
{
    lv_timer_handler();
    delay(5);
}

void DisplayManager::initDisplayHardware()
{
    Serial.println("[Display] init hardware");

    // -------------------------------------------------------------------------
    // Pin mapping
    //
    // This mapping is based on a common ESP32-S3 4.0" 480x480 RGB panel layout.
    // It may need adjustment for your exact board revision.
    // -------------------------------------------------------------------------

    // Example timing / RGB pin mapping for ESP32-S3 480x480 RGB boards
    rgbPanel = new Arduino_ESP32RGBPanel(
        18 /* DE */, 17 /* VSYNC */, 16 /* HSYNC */, 21 /* PCLK */,
        11 /* R0 */, 12 /* R1 */, 13 /* R2 */, 14 /* R3 */, 0 /* R4 */,
        8  /* G0 */, 20 /* G1 */, 3  /* G2 */, 46 /* G3 */, 9 /* G4 */, 10 /* G5 */,
        4  /* B0 */, 5  /* B1 */, 6  /* B2 */, 7  /* B3 */, 15 /* B4 */,
        1  /* hsync polarity */, 10 /* hsync front porch */, 8 /* hsync pulse width */, 50 /* hsync back porch */,
        1  /* vsync polarity */, 10 /* vsync front porch */, 8 /* vsync pulse width */, 20 /* vsync back porch */
    );

    gfx = new Arduino_RGB_Display(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        rgbPanel,
        0 /* rotation */,
        true /* auto_flush */
    );

    if (!gfx->begin())
    {
        Serial.println("[Display] ERROR: gfx->begin() failed");
        return;
    }

    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);
    gfx->setCursor(20, 20);
    gfx->println("Pool Control Display");
    gfx->println("Display init OK");
}

void DisplayManager::initLvgl()
{
    Serial.println("[Display] init LVGL");

    lv_init();

    const size_t buffer_pixels = SCREEN_WIDTH * 40; // partial buffer
    buf1 = static_cast<lv_color_t*>(heap_caps_malloc(buffer_pixels * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    buf2 = static_cast<lv_color_t*>(heap_caps_malloc(buffer_pixels * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));

    if (!buf1 || !buf2)
    {
        Serial.println("[Display] ERROR: LVGL buffer allocation failed");
        return;
    }

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, buffer_pixels);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = lvglFlushCallback;
    disp_drv.draw_buf = &draw_buf;

    lv_disp_drv_register(&disp_drv);
}

void DisplayManager::lvglFlushCallback(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    if (!gfx)
    {
        lv_disp_flush_ready(disp);
        return;
    }

    const int32_t x = area->x1;
    const int32_t y = area->y1;
    const int32_t w = area->x2 - area->x1 + 1;
    const int32_t h = area->y2 - area->y1 + 1;

    gfx->draw16bitRGBBitmap(x, y, reinterpret_cast<uint16_t*>(color_p), w, h);

    lv_disp_flush_ready(disp);
}