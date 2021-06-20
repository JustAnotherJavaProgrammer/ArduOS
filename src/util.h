#include <Arduino.h>

unsigned long combineBytesToUnsignedLong(byte longArr[], int length);
void sleep(unsigned long delayMillis);
void sleepMicroseconds(unsigned long delayMicros);
byte sign(int num);
void progmemPrint(const char *str);
void progmemPrintln(const char *str);
int freeMemory();