#include "bitIO.h"

void writeUint64(FILE* file, uint64_t value)
{
    for (int byteIndex = 0; byteIndex < 8; byteIndex++) {
        fputc((int)(value & 0xFF), file);
        value >>= 8;
    }
}

uint64_t readUint64(FILE* file)
{
    uint64_t value = 0;
    for (int byteIndex = 0; byteIndex < 8; byteIndex++) {
        int inputByte = fgetc(file);
        if (inputByte == EOF)
            return 0;
        value |= ((uint64_t)(uint8_t)inputByte) << (byteIndex * 8);
    }
    return value;
}

void writeUint16(FILE* file, uint16_t value)
{
    fputc((int)(value & 0xFF), file);
    fputc((int)((value >> 8) & 0xFF), file);
}

uint16_t readUint16(FILE* file)
{
    int lowByte = fgetc(file);
    int highByte = fgetc(file);
    if (lowByte == EOF || highByte == EOF)
        return 0;
    return (uint16_t)lowByte | (uint16_t)((uint16_t)highByte << 8);
}
