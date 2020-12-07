unsigned long combineBytesToUnsignedLong(byte longArr[], int length) {
  unsigned long result = 0UL;
  for (int i = 0; i < length; i++)
  {
    // Serial.println(result);
    if (i > 0)
      result <<= 8;
    result |= longArr[i];
  }
  return result;
}

void sleep(unsigned long delayMillis) {
  unsigned long targetTime = millis() + delayMillis;
  while (millis() < targetTime) {}
}

void sleepMicroseconds(unsigned long delayMicros) {
  unsigned long targetTime = micros() + delayMicros;
  while (micros() < targetTime) {}
}

byte sign(int num) {
  return num < 0 ? -1 : (num > 0 ? 1 : 0);
}

/** 
 *  Taken from https://www.instructables.com/How-to-Display-Images-on-24inch-TFT-and-Make-It-a-/
 *  by geekrex on instructables
 **/
// Copy string from flash to serial port
// Source string MUST be inside a PSTR() declaration!
void progmemPrint(const char *str) {
  char c;
  while(c = pgm_read_byte(str++)) Serial.print(c);
}

// Same as above, with trailing newline
void progmemPrintln(const char *str) {
  progmemPrint(str);
  Serial.println();
}

// Taken from https://github.com/mpflaga/Arduino-MemoryFree/blob/master/MemoryFree.cpp
// Arduino-MemoryFree by Michael P. Flaga on GitHub
/**
 * The MIT License (MIT)
 * 
 * Copyright (c) 2012 Michael P. Flaga
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 */
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
