#include "DisplayController.h"

DisplayController::DisplayController() : display(128, 64, &Wire, -1) {}

bool DisplayController::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        return false;
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
    return true;
}

void DisplayController::drawRadioScreen(uint16_t freq, uint8_t vol) {
    display.clearDisplay();
    
    // Header
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("FM RECEIVER");

    display.setCursor(85, 0);
    display.print("[MENU]");

    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
    
    // Frequency
    display.setTextSize(2);
    display.setCursor(10, 18);
    display.print(freq / 100.0, 1);
    display.setTextSize(1);
    display.setCursor(80, 24);
    display.print("MHz");
    
    // Volume text & bar
    display.setCursor(0, 42);
    display.print("Vol: ");
    display.print(vol);
    display.print("/");
    display.print(VOL_MAX);
    
    // Volume bar
    display.drawRect(50, 42, 77, 8, SSD1306_WHITE);
    int barWidth = map(vol, 0, VOL_MAX, 0, 73);
    if (barWidth > 0) {
        display.fillRect(52, 44, barWidth, 4, SSD1306_WHITE);
    }

    // Footer hint
    display.setCursor(0, 56);
    display.print("SEL:Menu  A4/A5:Tune");
    
    display.display();
}

void DisplayController::drawMenuList(const uint16_t presets[NUM_PRESETS], uint8_t selectedIndex) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Title header
    display.setCursor(18, 0);
    display.print("PRESET STATIONS");
    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    // List of 3 presets
    for (uint8_t i = 0; i < NUM_PRESETS; i++) {
        int y = 14 + (i * 13);
        if (i == selectedIndex) {
            display.fillRect(0, y - 1, 128, 12, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
            display.setCursor(2, y + 1);
            display.print(">");
        } else {
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(2, y + 1);
            display.print(" ");
        }

        display.setCursor(12, y + 1);
        display.print("Preset ");
        display.print(i + 1);
        display.print(": ");
        display.print(presets[i] / 100.0, 1);
        display.print("MHz");
    }

    // Footer hints
    display.setTextColor(SSD1306_WHITE);
    display.drawLine(0, 53, 127, 53, SSD1306_WHITE);
    display.setCursor(0, 56);
    display.print("SEL:Edit/Tune  A0:Back");

    display.display();
}

void DisplayController::drawPresetEdit(uint8_t presetIndex, uint16_t freq) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Header
    display.setCursor(20, 0);
    display.print("EDIT PRESET ");
    display.print(presetIndex + 1);
    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    // Frequency display box
    display.drawRoundRect(10, 16, 108, 26, 3, SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(24, 21);
    display.print(freq / 100.0, 1);
    display.setTextSize(1);
    display.setCursor(86, 26);
    display.print("MHz");

    // Help instructions
    display.setCursor(0, 46);
    display.print("A4/A5 or Knob to change");
    display.setCursor(0, 56);
    display.print("SEL:Save & Play A0:Cancel");

    display.display();
}