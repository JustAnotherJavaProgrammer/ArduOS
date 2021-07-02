#ifndef BYTECODE_EXECUTOR_ALREADY_SEEN
#define BYTECODE_EXECUTOR_ALREADY_SEEN
// #pragma once

#include <Arduino.h>
#include <SD.h>
// #include <SDFat.h>
// #include <HardwareSerial.h>

#include "executor.h"
#include "graphics.h"
#include "main.h"
#include "util.h"

// #define CLEARMEMFILE_EVERY_PROGRAM

#define FLAG_CARRY 0
#define FLAG_EQUAL 1
#define FLAG_GREATER_THAN 2
#define FLAG_BIT_COPY_STORE 3

#define MEM_MAX_ADDRESS 0x7FFFFFF  // 128 MiB
#define pushToStack(a) setMemAddr(stack_pointer--, a)

class BytecodeExecutor : public Executor {
    File sourceFile;
    File memFile;
    File fileioFiles[2];
    bool currFileIoFile = 0;
    uint16_t sourceVersion;
    uint16_t registers[32];
    // uint32_t stack_pointer = MEM_MAX_ADDRESS / 2 + 1;
    uint32_t stack_pointer;  // No initial assignment necessary, assignment happens later ~~dynamically~~ in openProgram()
    byte flags = 0;
    byte id;

   public:
    BytecodeExecutor(byte ID) {
        id = ID;  // best assignment ever, not confusing at all
    }

    virtual ~BytecodeExecutor(){};

    bool openProgram(const char* fileName) {
        // if (sourceFile != NULL) {
        if (sourceFile) {
            sourceFile.close();
        }
        if (!SD.exists(fileName)) return false;
        sourceFile = SD.open(fileName);
        if (sourceFile) {
            sourceFile.seek(15);
            sourceVersion = (sourceFile.read() << 8) + sourceFile.read();
            Serial.begin(9600);
#ifdef CLEARMEMFILE_EVERY_PROGRAM
            if (memFile) memFile.close();
#else
            if (!memFile)
#endif
            memFile = createMemoryFile();
            stack_pointer = MEM_MAX_ADDRESS / 2 + 1;
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

    bool execCommand() {
        // execution complete or sourceFile corrupted
        Serial.println(sourceFile.available());
        Serial.println(sourceFile.position() - sourceFile.size());
        if (!sourceFile || !sourceFile.available()) return false;
        byte instruction[4];
        sourceFile.readBytes(instruction, 4);
        switch (instruction[0]) {
            case 0x00:  // ADD regA, regB
            case 0x01:  // ADC regA, regB
            case 0x02:  // ADDI regA, byte1~2
            case 0x03:  // ADCI regA, byte1~2
            case 0x04:  // SUB regA, regB
            case 0x05:  // SUC regA, regB
            case 0x06:  // SUBI regA, byte1~2
            case 0x07:  // SUCI regA, byte1~2
            {
                uint16_t result = instruction[0] > 0x03
                                      ? getRegister(instruction[1]) - (instruction[0] > 0x05 ? (uint16_t)constFromBytes(&instruction[2], 2) : getRegister(instruction[2])) -
                                            (instruction[0] % 2 == 1 ? getFlag(FLAG_CARRY) : 0)
                                      : getRegister(instruction[1]) + (instruction[0] > 0x01 ? (uint16_t)constFromBytes(&instruction[2], 2) : getRegister(instruction[2])) +
                                            (instruction[0] % 2 == 1 ? getFlag(FLAG_CARRY) : 0);
                setRegister(instruction[1], result);
                setFlag(FLAG_CARRY, instruction[0] > 0x03 ? result > getRegister(instruction[1]) : result < getRegister(instruction[1]));
            } break;
            case 0x08:  // AND regA, regB
            case 0x09:  // ANDI regA, byte1~2
                setRegister(instruction[1], getRegister(instruction[1]) & (instruction[0] == 0x08 ? getRegister(instruction[2]) : constFromBytes(&instruction[2], 2)));
                break;
            case 0x0A:  // OR regA, regB
            case 0x0B:  // ORI regA, byte1~2
                setRegister(instruction[1], getRegister(instruction[1]) | (instruction[0] == 0x0A ? getRegister(instruction[2]) : constFromBytes(&instruction[2], 2)));
                break;
            case 0x0C:  // XOR regA, regB
            case 0x0D:  // XORI regA, byte1~2
                setRegister(instruction[1], getRegister(instruction[1]) ^ (instruction[0] == 0x0C ? getRegister(instruction[2]) : constFromBytes(&instruction[2], 2)));
                break;
            case 0x0E:  // COM regA
            case 0x0F:  // NEG regA
                setRegister(instruction[1], ~getRegister(instruction[1]) + (instruction[0] == 0x0F));
                break;
            case 0x10:  // SBR regA, byte1~2
                setRegister(instruction[1], getRegister(instruction[1]) | constFromBytes(&instruction[2], 2));
                break;
            case 0x11:                                                                                             // CBR regA, byte1~2
                setRegister(instruction[1], getRegister(instruction[1]) & (~constFromBytes(&instruction[2], 2)));  // A & !B
                break;
            case 0x12:  // TST regA
            case 0x13:  // CMP regA, regB
            case 0x14:  // CMPI regA, byte1~2
            {
                uint16_t valueB = instruction[0] == 0x12 ? 0 : instruction[0] == 0x13 ? getRegister(instruction[2]) : constFromBytes(&instruction[2], 2);
                setFlag(FLAG_EQUAL, getRegister(instruction[1]) == valueB);
                setFlag(FLAG_GREATER_THAN, getRegister(instruction[1]) > valueB);
            } break;
            case 0x15:  // CLR regA
            case 0x16:  // SER regA
                setRegister(instruction[1], instruction[0] == 0x15 ? 0 : 0xFFFF);
                break;
            case 0x17:  // MUL regA, regB
                setRegisters(0, 1, ((uint32_t)getRegister(instruction[1])) * getRegister(instruction[2]));
                break;
            case 0x18:  // MULS regA, regB
                setRegisters(0, 1, ((int32_t)getRegister(instruction[1])) * ((int32_t)getRegister(instruction[2])));
                break;
            case 0x19:  // MULSU regA, regB
                setRegisters(0, 1, ((int32_t)getRegister(instruction[1])) * ((uint32_t)getRegister(instruction[2])));
                break;
            case 0x1A:  // LSL regA
            case 0x1C:  // ROL regA
            {
                uint16_t regA = getRegister(instruction[1]);
                bool tmp = instruction[0] == 0x1C && getFlag(FLAG_CARRY);
                setFlag(FLAG_CARRY, bitRead(regA, 15));
                setRegister(instruction[1], (regA << 1) + tmp);
            } break;
            case 0x1B:  // RSL regA
            case 0x1D:  // ROR regA
            case 0x1E:  // ASR regA
            {
                uint16_t regA = getRegister(instruction[1]);
                bool tmp = instruction[0] == 0x1D && getFlag(FLAG_CARRY);
                setFlag(FLAG_CARRY, bitRead(regA, 0));
                if (instruction[0] == 0x1E)
                    setRegister(instruction[1], ((int16_t)regA) >> 1);
                else {
                    regA = regA >> 1;
                    bitWrite(regA, 15, tmp);
                    setRegister(instruction[1], regA);
                }
            } break;
            case 0x1F:  // SWAP regA
            {
                byte leastSignificantByte = getRegister(instruction[1]);
                setRegister(instruction[1], ((getRegister(instruction[1]) >> 8) << 8) + ((leastSignificantByte & 0x0F) << 4 | (leastSignificantByte & 0xF0) >> 4));
            } break;
            case 0x20:  // FSET byte1
            case 0x21:  // FCLR byte1
                setFlag(instruction[1], instruction[0] == 0x20);
                break;
            case 0x22:  // BST regA, byte1
                if (instruction[2] < 16) setFlag(FLAG_BIT_COPY_STORE, bitRead(getRegister(instruction[1]), instruction[2]));
                break;
            case 0x23:  // BLD regA, byte1
                if (instruction[2] < 16) {
                    uint16_t tmp = getRegister(instruction[1]);
                    bitWrite(tmp, instruction[2], getFlag(FLAG_BIT_COPY_STORE));
                    setRegister(instruction[1], tmp);
                }
                break;
            case 0x24:  // MOV regA, regB
            case 0x25:  // LDI regA, byte~2
                setRegister(instruction[1], instruction[0] == 0x24 ? getRegister(instruction[2]) : constFromBytes(&instruction[2], 2));
                break;
            case 0x26:  // LDR regA, regB~regC
                setRegister(instruction[1], getMemAddr(constFromRegisters(instruction[2], instruction[3])));
                break;
            case 0x27:  // STR regA, regB~regC
                setMemAddr(constFromRegisters(instruction[2], instruction[3]), instruction[1]);
                break;
            case 0x28:  // PUSH regA
                pushToStack(getRegister(instruction[1]));
                break;
            case 0x29:  // POP regA
                setRegister(instruction[1], popFromStack());
                break;
            case 0x2A:  // SEB regA~regB
            case 0x2B:  // SEBI byte1~3
                Serial.begin(instruction[0] == 0x2A ? constFromRegisters(instruction[1], instruction[2]) : constFromBytes(&instruction[1], 3));
                while (!Serial) {
                }
                break;
            case 0x2C:  // SOUT regA
            case 0x2D:  // SOUTI byte1
                Serial.write(instruction[0] == 0x2C ? (uint8_t)getRegister(instruction[1]) : instruction[1]);
                break;
            case 0x2E:  // SIN regA
                setRegister(instruction[1], Serial.read());
                break;
            case 0x2F:  // RJMP byte1~3;
            {
                int32_t tmp = constFromBytes(&instruction[1], 3);
                bitWrite(tmp, 31, bitRead(tmp, 23));
                bitClear(tmp, 23);
                tmp *= 4;
                sourceFile.seek(sourceFile.position() + tmp);
            } break;
            case 0x30:  // JMP regA~regB
            case 0x31:  // JMPI byte1~3
            case 0x32:  // CALL regA~regB
            case 0x33:  // CALLI byte1~3
            case 0x3B:  // BREQ byte1~3
            case 0x3C:  // BRNE byte1~3
            case 0x3D:  // BRGR byte1~3
            case 0x3E:  // BRLE byte1~3
            case 0x3F:  // BREQGR byte1~3
            case 0x40:  // BREQLE byte1~3
            case 0x41:  // RBREQ regA~B
            case 0x42:  // RBRNE regA~B
            case 0x43:  // RBRGR regA~B
            case 0x44:  // RBRLE regA~B
            case 0x45:  // RBREQGR regA~B
            case 0x46:  // RBREQLE regA~B
                if (instruction[0] > 0x31 && instruction[0] < 0x34) {
                    uint32_t currPos = sourceFile.position();
                    pushToStack((uint16_t)currPos);
                    pushToStack((uint16_t)(currPos >> 16));
                }
                // Please forgive me
                if (instruction[0] < 0x34 ||
                    ((instruction[0] == 0x3B || (instruction[0] > 0x3E && instruction[0] < 0x42) || instruction[0] == 0x45 || instruction[0] == 0x46) && getFlag(FLAG_EQUAL)) ||
                    ((instruction[0] == 0x3C || instruction[0] == 0x41) && !getFlag(FLAG_EQUAL)) ||
                    ((instruction[0] == 0x3D || instruction[0] == 0x3F || instruction[0] == 0x43 || instruction[0] == 0x45) && getFlag(FLAG_GREATER_THAN)) ||
                    ((instruction[0] == 0x3E || instruction[0] == 0x40 || instruction[0] == 0x44 || instruction[0] == 0x46) && !getFlag(FLAG_GREATER_THAN)))
                    sourceFile.seek(((instruction[0] < 0x34 && instruction[0] % 2 == 0) || instruction[0] > 0x40 ? constFromRegisters(instruction[1], instruction[2])
                                                                                                                 : constFromBytes(&instruction[1], 3)) *
                                    4);
                break;
            case 0x34:  // RET
            {
                uint32_t targetAddr = ((uint32_t)popFromStack()) << 16;
                sourceFile.seek(targetAddr + popFromStack());
            } break;
            case 0x35:  // SEQ
            case 0x36:  // SNE
            case 0x37:  // SGR
            case 0x38:  // SLE
            case 0x39:  // SEQGR
            case 0x3A:  // SEQLE
                if (((instruction[0] == 0x35 || instruction[0] > 0x38) && getFlag(FLAG_EQUAL)) || (instruction[0] == 0x36 && !getFlag(FLAG_EQUAL)) ||
                    (instruction[0] > 0x36 && (instruction[0] % 2 == 0 ? !getFlag(FLAG_GREATER_THAN) : getFlag(FLAG_GREATER_THAN))))
                    sourceFile.seek(sourceFile.position() + 4);
                break;
            case 0x47:  // PXL regA, regB, regC
                tft.drawPixel(getRegister(instruction[1]), getRegister(instruction[2]), getRegister(instruction[3]));
                break;
            case 0x48:  // SCLR regA
            case 0x49:  // SCLRI byte1~2
                tft.fillScreen(instruction[0] == 0x48 ? getRegister(instruction[1]) : constFromBytes(&instruction[1], 2));
                break;
            case 0x4A:  // TSIZ regA
            case 0x4B:  // TSIZI byte1
                tft.setTextSize(instruction[0] == 0x4A ? getRegister(instruction[1]) : instruction[1]);
                break;
            case 0x4C:  // TCOL regA
            case 0x4E:  // TCOLI byte1~2
                tft.setTextColor(instruction[1] == 0x4C ? getRegister(instruction[1]) : constFromBytes(&instruction[1], 2));
                break;
            case 0x4D:  // TCOLB regA, regB
                tft.setTextColor(getRegister(instruction[1]), getRegister(instruction[2]));
                break;
            case 0x4F:  // TWRAP regA
            case 0x50:  // TWRAPI byte1
                tft.setTextWrap((instruction[0] == 0x4F ? getRegister(instruction[1]) : instruction[1]) != 0);
                break;
            case 0x51:  // TCPOS regA, regB
                tft.setCursor(getRegister(instruction[1]), getRegister(instruction[2]));
                break;
            case 0x52:  // TOUT regA
            case 0x53:  // TOUTI byte1
                tft.print((char)(instruction[0] == 0x52 ? getRegister(instruction[1]) : instruction[1]));
                break;
            case 0x54:  // IMG regA~B
            case 0x55:  // IMGI byte1~3
            case 0x6D:  // RUN regA~B
            {
                char* filepath = readStringFromMemory(instruction[0] == 0x55 ? constFromBytes(&instruction[1], 3) : constFromRegisters(instruction[1], instruction[2]));
                if (instruction[0] == 0x6D)
                    openProgram(filepath);
                else
                    drawBmp(filepath, tft.getCursorX(), tft.getCursorY());
                free(filepath);
            } break;
            case 0x56:  // FEXISTS regA, regB~C
            case 0x57:  // FMKDIR   regA, regB~C
            case 0x58:  // FOPEN regA, regB~C
            case 0x59:  // FREM regA, regB~C
            case 0x5A:  // FRMDIR regA, regB~C
            {
                char* filepath = readStringFromMemory(constFromRegisters(instruction[2], instruction[3]));
                bool success = false;
                switch (instruction[0]) {
                    case 0x56:  // FEXISTS regA, regB~C
                        success = SD.exists(filepath);
                        break;
                    case 0x57:  // FMKDIR   regA, regB~C
                        success = SD.mkdir(filepath);
                        break;
                    case 0x58:  // FOPEN regA, regB~C
                        if (fileioFiles[currFileIoFile]) fileioFiles[currFileIoFile].close();
                        fileioFiles[currFileIoFile] = SD.open(filepath, FILE_WRITE);
                        fileioFiles[currFileIoFile].seek(0);
                        success = (bool)fileioFiles[currFileIoFile];
                        break;
                    case 0x59:  // FREM regA, regB~C
                        success = SD.remove(filepath);
                        break;
                    case 0x5A:  // FRMDIR regA, regB~C
                        success = SD.rmdir(filepath);
                        break;
                }
                free(filepath);
                setRegister(instruction[1], success);
            } break;
            case 0x5B:  // FNAME regA, regB
                setRegister(instruction[1], instruction[0] == 0x5B ? fileioFiles[currFileIoFile].name()[getRegister(instruction[2])] : fileioFiles[currFileIoFile].available());
                break;
            case 0x5C:  // FAV regA
                setRegister(instruction[1], fileioFiles[currFileIoFile].available());
                break;
            case 0x5D:  // FCLOS
                fileioFiles[currFileIoFile].close();
                break;
            case 0x5E:  // FFLUS
                fileioFiles[currFileIoFile].flush();
                break;
            case 0x5F:  // FPEK regA
            case 0x68:  // FIN regA
                setRegister(instruction[1], instruction[0] == 0x5F ? fileioFiles[currFileIoFile].peek() : fileioFiles[currFileIoFile].read());
                break;
            case 0x60:  // FPOS
                setRegisters(0, 1, fileioFiles[currFileIoFile].position());
                break;
            case 0x61:  // FSEK regA, regB~C
            case 0x63:  // FISDIR regA
                setRegister(instruction[1], instruction[0] == 0x61 ? fileioFiles[currFileIoFile].seek(constFromRegisters(instruction[2], instruction[3]))
                                                                   : fileioFiles[currFileIoFile].isDirectory());
                break;
            case 0x62:  // FSEKI byte1~3
                setFlag(FLAG_BIT_COPY_STORE, fileioFiles[currFileIoFile].seek(constFromBytes(&instruction[1], 3)));
                break;
            case 0x64:  // FNEXT
                currFileIoFile = !currFileIoFile;
                if (fileioFiles[currFileIoFile]) fileioFiles[currFileIoFile].close();
                fileioFiles[currFileIoFile] = fileioFiles[!currFileIoFile].openNextFile();
                setFlag(FLAG_BIT_COPY_STORE, fileioFiles[currFileIoFile]);
                break;
            case 0x65:  // FREW
                fileioFiles[currFileIoFile].rewindDirectory();
                break;
            case 0x66:  // FOUT regA
            case 0x67:  // FOUTI byte1
                fileioFiles[currFileIoFile].write(instruction[0] == 0x66 ? getRegister(instruction[1]) : instruction[1]);
                break;
            case 0x69:  // SET regA, regB~C
            case 0x6A:  // SETI byte1, regB~C
                setSysvar(instruction[0] == 0x69 ? getRegister(instruction[1]) : instruction[1], constFromRegisters(instruction[1], instruction[2]));
                break;
            case 0x6B:  // GET regA
            case 0x6C:  // GETI byte1
                setRegisters(0, 1, getSysvar(instruction[0] == 0x6B ? getRegister(instruction[1]) : instruction[1]));
                break;
            default:  // noop
                break;
        }
        return true;
    }

    uint32_t getSysvar(byte varId) {
        switch (varId) {
            case 0:
                return freeMemory();
            case 1:
                return tft.width();
            case 2:
                return tft.height();
            case 3:
                return sysvars.publicWidthChange;
            case 4:
                return sysvars.publicHeightChange;
            case 5:
                return sysvars.xOffset;
            case 6:
                return sysvars.yOffset;
            case 7:
                return currFileIoFile;
            case 8:
                return tft.getRotation();
            default:
                return 0;
        }
        return 0;
    }

    void setSysvar(byte varId, uint32_t value) {
        switch (varId) {
            case 3:
                sysvars.publicWidthChange = value;
                break;
            case 4:
                sysvars.publicHeightChange = value;
                break;
            case 5:
                sysvars.xOffset = value;
                break;
            case 6:
                sysvars.yOffset = value;
                break;
            case 7:
                currFileIoFile = value;
                break;
            case 8:
                tft.setRotation(value);
                break;
            default:
                // do nothing, variable is read-only
                break;
        }
    }

    /**
     * WARNING: Please call free() to free up memory afrer using the string!!!
     */
    char* readStringFromMemory(const uint32_t addr) {
        uint16_t str_len = 0;
        while (((byte)getMemAddr(addr + str_len)) != 0) {
            str_len++;
        }
        str_len++;
        char* memStr = (char*)malloc(str_len);
        for (uint16_t len = 0; len < str_len; len++) {
            memStr[len] = (char)getMemAddr(addr + len);
        }
        return memStr;
    }

    uint16_t popFromStack() {
        uint16_t retVal = 0;
        if (stack_pointer < MEM_MAX_ADDRESS / 2 + 1) {
            retVal = getMemAddr(stack_pointer);
            stack_pointer++;
        }
        return retVal;
    }

    uint16_t getMemAddr(const uint32_t addr) {
        memFile.seek(addr * 2);
        uint16_t res = 0;
        memFile.readBytes((uint8_t*)(&res), 2);
        return res;
    }

    void setMemAddr(const uint32_t addr, const uint16_t value) {
        memFile.seek(addr * 2);
        memFile.write((char*)(&value), 2);
        // Does not invoke flush(), will be cleared on next boot anyways
    }

    void setFlag(byte flag, bool value) {
        if (flag > 7) return;
        bitWrite(flags, flag, value);
    }

    bool getFlag(byte flag) {
        if (flag > 7) return false;
        return bitRead(flags, flag);
    }

    uint32_t constFromBytes(const byte* bytes, byte byteCount) {
        if (byteCount == 0) return 0;
        if (byteCount > 4) byteCount = 4;
        uint32_t result = 0;
        while (byteCount > 0) {
            byteCount--;
            result += (result << 8) + bytes[byteCount];
        }
        return result;
    }

    uint32_t constFromRegisters(const byte regA, const byte regB) { return (((uint32_t)getRegister(regB)) << 16) + ((uint32_t)getRegister(regA)); }

    uint16_t getRegister(byte regID) {
        if (regID < 32) return registers[regID];
        if (regID < 34) {
            if (regID == 32)
                return (uint16_t)stack_pointer;
            else
                return (uint16_t)(stack_pointer >> 16);
        }
        return 0;
    }

    void setRegisters(const byte regA, byte regB, uint32_t value) {
        setRegister(regA, (uint16_t)value);
        setRegister(regB, (uint16_t)(value >> 16));
    }

    void setRegister(const byte regID, const uint16_t value) {
        if (regID < 32) {
            registers[regID] = value;
            return;
        }
        return;
    }

    File createMemoryFile() {
        const __FlashStringHelper* filename = id == 1 ? F("mem1.mem") : F("mem0.mem");
        // if (SD.exists(filename)) {
        //     SD.remove(filename);
        // }
        File memfile = SD.open(filename, FILE_WRITE);
        uint32_t bytesWritten = memfile.size();
        while (bytesWritten < MEM_MAX_ADDRESS + 1) {
            while (!memfile.availableForWrite()) Serial.println(F("memfile not available for write..."));
            Serial.println(bytesWritten);
            memfile.print((char)bytesWritten);
            bytesWritten++;
            // if (memfile.size() > 0 && memfile.size() % 512 == 0) {
            //     Serial.println("flushing...");
            //     memfile.flush();
            // }
        }
        memfile.flush();
        return memfile;
    }
};

#endif