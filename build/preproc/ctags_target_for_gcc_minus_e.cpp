# 1 "d:\\lukas\\git\\ArduOS\\ArduOS.ino"
unsigned long startTimeMillis = 0;
// const PROGMEM unsigned long baud = 1250000;
const unsigned long baud = 1000000;

void setup(){
    pinMode((13), OUTPUT);
    digitalWrite((13), LOW);
    Serial.setTimeout(4294967295);
    Serial.begin(baud);
    while (!Serial)
    {
    }
    Serial.println(("Hello ArduOS!"));
    byte longBuffer[4];
    Serial.readBytes(longBuffer, 4);
    digitalWrite((13), HIGH);
    // startTimeMillis = (((unsigned long)longBuffer[0] << 24) | ((unsigned long)longBuffer[1] << 16) | ((unsigned long)longBuffer[2] << 8) | ((unsigned long)longBuffer[3]));
    startTimeMillis = combine(longBuffer, 4) - millis();
    randomSeed(startTimeMillis);

    Serial.println(startTimeMillis);
    digitalWrite((13), HIGH);
}

void loop()
{
}

unsigned long combine(byte longArr[], int length)
{
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
