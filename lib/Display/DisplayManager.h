#pragma once

#include <Arduino.h>
#include <lvgl.h>

class DisplayManager
{
public:
    bool begin();
    void loop();

    uint16_t width() const { return 480; }
    uint16_t height() const { return 480; }

private:
    void initLvgl();
    void initDisplayHardware();

    static void lvglFlushCallback(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p);
};