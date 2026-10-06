#include "RadioController.h"

void RadioController::begin() {
    rx.setup();
    rx.setVolume(5);
    rx.setFrequency(9850); // Default boot frequency: 98.5 MHz
    currentFreq = 9850;
    currentVol = 5;
}

void RadioController::setFrequency(uint16_t freq) {
    rx.setFrequency(freq);
    currentFreq = freq;
}

void RadioController::setVolume(uint8_t vol) {
    if (vol == 0) {
        rx.setMute(true);   // Engage hardware mute for dead silence
        rx.setVolume(0); 
    } else {
        rx.setMute(false);  // Disengage mute
        rx.setVolume(vol);
    }
    currentVol = vol;
}

void RadioController::seekUp() {
    rx.seek(RDA_SEEK_WRAP, RDA_SEEK_UP);
    currentFreq = rx.getFrequency();
}

void RadioController::seekDown() {
    rx.seek(RDA_SEEK_WRAP, RDA_SEEK_DOWN);
    currentFreq = rx.getFrequency();
}

uint16_t RadioController::getFrequency() { return currentFreq; }
uint8_t RadioController::getVolume() { return currentVol; }