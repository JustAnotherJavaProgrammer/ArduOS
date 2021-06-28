#ifndef MAIN_H_FILE_ALREADY_SEEN
#define MAIN_H_FILE_ALREADY_SEEN

#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

#include "sysvarStore.h"

extern MCUFRIEND_kbv tft;
extern SysvarStore sysvars;
TSPoint readTFT();
void drawFatalErrorMsg(const __FlashStringHelper *text);
bool isPressed(TSPoint p);

#endif