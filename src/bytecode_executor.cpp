// #pragma once

#include <Arduino.h>
#include <SD.h>
// #include <HardwareSerial.h>

#include "executor.h"

class BytecodeExecutor : public Executor {
    File sourceFile;
    uint16_t sourceVersion;
    uint16_t registers[32];
    byte flags = 0;

   public:
    virtual ~BytecodeExecutor(){};

    bool openProgram(const char *fileName) {
        // if (sourceFile != NULL) {
        if (sourceFile) {
            sourceFile.close();
        }
        sourceFile = SD.open(fileName);
        if (sourceFile) {
            sourceFile.seek(15);
            sourceVersion = (sourceFile.read() << 8) + sourceFile.read();
            // Serial.begin(9600);
            // Serial.print(F("Hello there! File version is: "));
            // Serial.println(sourceVersion);
            // byte test[2];
            // test[0] = 0xff;
            // test[1] = 0x01;
            // uint16_t testint = *((uint16_t*)test);
            // Serial.println(testint, HEX);
            // Serial.end();
            return true;
        }
        return false;
    }

    void execCommand() {
        // DO SOMETHING
        byte instruction[4];
        sourceFile.readBytes(instruction, 4);
        
    }
};