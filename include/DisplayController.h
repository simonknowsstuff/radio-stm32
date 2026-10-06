#pragma once
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "config.h"

enum AppMode {
    MODE_RADIO,
    MODE_MENU_LIST,
    MODE_PRESET_EDIT
};

class DisplayController {
public:
    DisplayController();
    bool begin();
    
    // Main FM radio screen
    void drawRadioScreen(uint16_t freq, uint8_t vol);
    
    // Menu selection screen showing preset stations
    void drawMenuList(const uint16_t presets[NUM_PRESETS], uint8_t selectedIndex);
    
    // Preset edit/modify screen
    void drawPresetEdit(uint8_t presetIndex, uint16_t freq);

private:
    Adafruit_SSD1306 display;
};