#ifndef MAIN_H_FILE_ALREADY_SEEN
#define MAIN_H_FILE_ALREADY_SEEN

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <TouchScreen.h>
#include <MCUFRIEND_kbv.h>

extern MCUFRIEND_kbv tft;
TSPoint readTFT();
void drawFatalErrorMsg(const __FlashStringHelper *text);
bool isPressed(TSPoint p);

#endif