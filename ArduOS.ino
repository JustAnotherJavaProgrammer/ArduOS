unsigned long startTimeMillis = 0;
// const PROGMEM unsigned long baud = 1250000;
const unsigned long baud = 1000000;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  // Serial.setTimeout(4294967295);
  Serial.begin(baud);
  while (!Serial) {
  }
  Serial.println(F("Hello ArduOS!"));
  byte longBuffer[4];
  flickerUntilBytesAvailable(4, 500);
  Serial.readBytes(longBuffer, 4);
  digitalWrite(LED_BUILTIN, HIGH);
  // startTimeMillis = (((unsigned long)longBuffer[0] << 24) | ((unsigned long)longBuffer[1] << 16) | ((unsigned long)longBuffer[2] << 8) | ((unsigned long)longBuffer[3]));
  startTimeMillis = combineBytesToUnsignedLong(longBuffer, 4) - millis();
  randomSeed(startTimeMillis);

  Serial.println(startTimeMillis);
  digitalWrite(LED_BUILTIN, HIGH);
  clear();
}

void loop() {
  yield();
  // Program code
}

void yield() {
  // UI code
}

void yield(unsigned long timeMillis) {
  unsigned long targetTime = millis() + timeMillis;
  while (millis() < targetTime) {
    yield();
  }
}

void yieldMicroseconds(unsigned long timeMicros) {
  unsigned long targetTime = micros() + timeMicros;
  while (micros() < targetTime) {
    yield();
  }
}

void flickerUntilBytesAvailable(byte numOfBytes, unsigned long delayMillis) {
  boolean high = false;
  unsigned long targetTime = millis();
  while (Serial.available() < numOfBytes) {
    if (millis() >= targetTime) {
      digitalWrite(LED_BUILTIN, high ? HIGH : LOW);
      high = !high;
      targetTime = millis() + delayMillis;
    }
  }
}
