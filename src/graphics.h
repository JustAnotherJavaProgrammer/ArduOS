#include <Arduino.h>
#include <SD.h>

byte drawBmp(char *filename, int x, int y);
uint16_t read16(File f);
uint32_t read32(File f);