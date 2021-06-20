#include <Arduino.h>
#include <SD.h>

#include "executor.h"

class BytecodeExecutor : public Executor {
    File sourceFile;

    bool openProgram(char *fileName) {
        if (sourceFile != NULL) {
            sourceFile.close();
        }
        sourceFile.seek(0);
        sourceFile = SD.open(fileName);
        if (sourceFile) {
            return true;
        }
        return false;
    }

    void execCommand() {
        // DO SOMETHING
    }
};