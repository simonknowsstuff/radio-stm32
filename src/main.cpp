#include <Arduino.h>
#include <Wire.h>
#include <Bounce2.h>
#include "config.h"
#include "RadioController.h"
#include "DisplayController.h"

RadioController radio;
DisplayController oled;

// Push button debouncers using Bounce2
Bounce btnBack   = Bounce();
Bounce btnSelect = Bounce();
Bounce btnDown   = Bounce();
Bounce btnUp     = Bounce();
Bounce btnFmDec  = Bounce();
Bounce btnFmInc  = Bounce();

// Presets (up to 3 radio stations)
uint16_t presets[NUM_PRESETS] = { 9110, 9850, 10400 }; // Default 91.1, 98.5, 104.0 MHz
uint8_t selectedPresetIndex = 0;
uint16_t editingFreq = 9850;

// Application State
AppMode currentMode = MODE_RADIO;

// Potentiometer tracking
int lastTuneAdc = -1;
int lastVolAdc = -1;
const int POT_THRESHOLD = 15;

// Helper to constrain and round frequency to 0.1 MHz (10 kHz internal units: e.g. 100 = 1.0MHz)
uint16_t clampFreq(int32_t freq) {
    if (freq < FM_MIN_FREQ) freq = FM_MIN_FREQ;
    if (freq > FM_MAX_FREQ) freq = FM_MAX_FREQ;
    return (uint16_t)((freq / 10) * 10);
}

void setup() {
    // Potentiometer pins
    pinMode(PIN_POT_TUNE, INPUT);
    pinMode(PIN_POT_VOL, INPUT);

    // Buttons setup with internal pullup and Bounce2
    btnBack.attach(PIN_BTN_BACK, INPUT_PULLUP);
    btnBack.interval(25);

    btnSelect.attach(PIN_BTN_SELECT, INPUT_PULLUP);
    btnSelect.interval(25);

    btnDown.attach(PIN_BTN_DOWN, INPUT_PULLUP);
    btnDown.interval(25);

    btnUp.attach(PIN_BTN_UP, INPUT_PULLUP);
    btnUp.interval(25);

    btnFmDec.attach(PIN_BTN_FM_DEC, INPUT_PULLUP);
    btnFmDec.interval(25);

    btnFmInc.attach(PIN_BTN_FM_INC, INPUT_PULLUP);
    btnFmInc.interval(25);

    // Initialize shared I2C bus
    Wire.setSCL(I2C_SCL);
    Wire.setSDA(I2C_SDA);
    Wire.begin();

    oled.begin();
    radio.begin();

    // Initial draw
    oled.drawRadioScreen(radio.getFrequency(), radio.getVolume());
}

void loop() {
    bool screenNeedsUpdate = false;

    // Update debouncers
    btnBack.update();
    btnSelect.update();
    btnDown.update();
    btnUp.update();
    btnFmDec.update();
    btnFmInc.update();

    // 1. Process Volume Knob (A7)
    analogRead(PIN_POT_VOL);
    delay(2);
    int volAdc = analogRead(PIN_POT_VOL);

    if (abs(volAdc - lastVolAdc) > POT_THRESHOLD || lastVolAdc == -1) {
        lastVolAdc = volAdc;
        uint8_t vol = map(volAdc, 0, 1023, VOL_MIN, VOL_MAX);
        if (vol > VOL_MAX) vol = VOL_MAX;
        if (vol != radio.getVolume()) {
            radio.setVolume(vol);
            if (currentMode == MODE_RADIO) {
                screenNeedsUpdate = true;
            }
        }
    }

    // 2. Process Tuning Knob (A6)
    analogRead(PIN_POT_TUNE);
    delay(2);
    int tuneAdc = analogRead(PIN_POT_TUNE);

    if (abs(tuneAdc - lastTuneAdc) > POT_THRESHOLD || lastTuneAdc == -1) {
        lastTuneAdc = tuneAdc;
        uint16_t mappedFreq = map(tuneAdc, 0, 1023, FM_MIN_FREQ, FM_MAX_FREQ);
        mappedFreq = clampFreq(mappedFreq);

        if (currentMode == MODE_RADIO) {
            if (mappedFreq != radio.getFrequency()) {
                radio.setFrequency(mappedFreq);
                screenNeedsUpdate = true;
            }
        } else if (currentMode == MODE_PRESET_EDIT) {
            if (mappedFreq != editingFreq) {
                editingFreq = mappedFreq;
                screenNeedsUpdate = true;
            }
        }
    }

    // 3. Process Buttons depending on current mode
    switch (currentMode) {
        case MODE_RADIO:
            // In FM screen: Select opens Menu
            if (btnSelect.fell()) {
                currentMode = MODE_MENU_LIST;
                screenNeedsUpdate = true;
            }
            // A5: increase FM value by 0.1 MHz (10 units)
            if (btnFmInc.fell()) {
                uint16_t newFreq = clampFreq((int32_t)radio.getFrequency() + 10);
                radio.setFrequency(newFreq);
                lastTuneAdc = tuneAdc; // prevent pot jump
                screenNeedsUpdate = true;
            }
            // A4: decrease FM value by 0.1 MHz (10 units)
            if (btnFmDec.fell()) {
                uint16_t newFreq = clampFreq((int32_t)radio.getFrequency() - 10);
                radio.setFrequency(newFreq);
                lastTuneAdc = tuneAdc;
                screenNeedsUpdate = true;
            }
            // A3 (Up) & A2 (Down) can also do seek / step
            if (btnUp.fell()) {
                radio.seekUp();
                lastTuneAdc = tuneAdc;
                screenNeedsUpdate = true;
            }
            if (btnDown.fell()) {
                radio.seekDown();
                lastTuneAdc = tuneAdc;
                screenNeedsUpdate = true;
            }
            break;

        case MODE_MENU_LIST:
            // A0: Back button closes menu and returns to FM screen
            if (btnBack.fell()) {
                currentMode = MODE_RADIO;
                screenNeedsUpdate = true;
            }
            // A3: Up button navigate menu up
            if (btnUp.fell()) {
                if (selectedPresetIndex > 0) {
                    selectedPresetIndex--;
                } else {
                    selectedPresetIndex = NUM_PRESETS - 1;
                }
                screenNeedsUpdate = true;
            }
            // A2: Down button navigate menu down
            if (btnDown.fell()) {
                if (selectedPresetIndex < NUM_PRESETS - 1) {
                    selectedPresetIndex++;
                } else {
                    selectedPresetIndex = 0;
                }
                screenNeedsUpdate = true;
            }
            // A1: Select button -> Open Edit/Modify mode for selected preset
            if (btnSelect.fell()) {
                currentMode = MODE_PRESET_EDIT;
                editingFreq = presets[selectedPresetIndex];
                lastTuneAdc = tuneAdc;
                screenNeedsUpdate = true;
            }
            // A5 / A4 can quickly tune to the selected preset directly
            if (btnFmInc.fell() || btnFmDec.fell()) {
                radio.setFrequency(presets[selectedPresetIndex]);
                lastTuneAdc = tuneAdc;
                currentMode = MODE_RADIO;
                screenNeedsUpdate = true;
            }
            break;

        case MODE_PRESET_EDIT:
            // A0: Back button cancels edit without saving and returns to menu
            if (btnBack.fell()) {
                currentMode = MODE_MENU_LIST;
                screenNeedsUpdate = true;
            }
            // A1: Select button saves preset, tunes to it, and returns to radio screen
            if (btnSelect.fell()) {
                presets[selectedPresetIndex] = editingFreq;
                radio.setFrequency(editingFreq);
                lastTuneAdc = tuneAdc;
                currentMode = MODE_RADIO;
                screenNeedsUpdate = true;
            }
            // A5: increase editing FM value by 0.1 MHz
            if (btnFmInc.fell()) {
                editingFreq = clampFreq((int32_t)editingFreq + 10);
                lastTuneAdc = tuneAdc;
                screenNeedsUpdate = true;
            }
            // A4: decrease editing FM value by 0.1 MHz
            if (btnFmDec.fell()) {
                editingFreq = clampFreq((int32_t)editingFreq - 10);
                lastTuneAdc = tuneAdc;
                screenNeedsUpdate = true;
            }
            // A3 / A2 can also step by 1.0 MHz or navigate
            if (btnUp.fell()) {
                editingFreq = clampFreq((int32_t)editingFreq + 100);
                lastTuneAdc = tuneAdc;
                screenNeedsUpdate = true;
            }
            if (btnDown.fell()) {
                editingFreq = clampFreq((int32_t)editingFreq - 100);
                lastTuneAdc = tuneAdc;
                screenNeedsUpdate = true;
            }
            break;
    }

    // 4. Redraw current screen if needed
    if (screenNeedsUpdate) {
        switch (currentMode) {
            case MODE_RADIO:
                oled.drawRadioScreen(radio.getFrequency(), radio.getVolume());
                break;
            case MODE_MENU_LIST:
                oled.drawMenuList(presets, selectedPresetIndex);
                break;
            case MODE_PRESET_EDIT:
                oled.drawPresetEdit(selectedPresetIndex, editingFreq);
                break;
        }
    }

    delay(10);
}