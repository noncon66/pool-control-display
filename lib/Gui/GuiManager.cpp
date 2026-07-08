#include "GuiManager.h"

#include <lvgl.h>

void GuiManager::begin()
{
    createTestScreen();
}

void GuiManager::loop()
{
    // currently nothing to do here
}

void GuiManager::createTestScreen()
{
    lv_obj_t* screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1C1C1C), LV_PART_MAIN);

    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "POOL CONTROL");
    lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    lv_obj_t* subtitle = lv_label_create(screen);
    lv_label_set_text(subtitle, "Display bring-up successful");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xA0A0A0), LV_PART_MAIN);
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 80);

    lv_obj_t* box = lv_obj_create(screen);
    lv_obj_set_size(box, 360, 180);
    lv_obj_align(box, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x2A2A2A), LV_PART_MAIN);
    lv_obj_set_style_border_width(box, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(box, 16, LV_PART_MAIN);

    lv_obj_t* status = lv_label_create(box);
    lv_label_set_text(
        status,
        "Display: OK\n"
        "LVGL: OK\n"
        "Touch: pending\n"
        "WiFi: pending\n"
        "MQTT: pending"
    );
    lv_obj_set_style_text_color(status, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(status, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_center(status);
}