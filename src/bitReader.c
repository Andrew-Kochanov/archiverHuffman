#include "bitReader.h"
#include <stdint.h>
#include <stdlib.h>

// Читатель битов
struct BitReader {
    FILE* file;
    uint8_t currentByte;
    int bitsRemaining;
};

// Создание битового читателя
BitReader* bitReaderCreate(FILE* file)
{
    BitReader* reader = (BitReader*)malloc(sizeof(BitReader));
    if (reader == NULL)
        return NULL;

    reader->file = file;
    reader->currentByte = 0;
    reader->bitsRemaining = 0;
    return reader;
}

// Чтение одного бита
int bitReaderReadBit(BitReader* reader)
{
    if (reader->bitsRemaining == 0) {
        int inputByte = fgetc(reader->file);
        if (inputByte == EOF)
            return -1;
        reader->currentByte = (uint8_t)inputByte;
        reader->bitsRemaining = 8;
    }

    int bit = (reader->currentByte >> 7) & 1;
    reader->currentByte <<= 1;
    reader->bitsRemaining--;
    return bit;
}

void bitReaderFree(BitReader* reader)
{
    free(reader);
}