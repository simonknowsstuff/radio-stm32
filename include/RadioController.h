#pragma once
#include <RDA5807.h>

class RadioController {
public:
    void begin();
    void setFrequency(uint16_t freq);
    void setVolume(uint8_t vol);
    void seekUp();
    void seekDown();
    uint16_t getFrequency();
    uint8_t getVolume();

private:
    RDA5807 rx;
    uint16_t currentFreq;
    uint8_t currentVol;
};