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
