#include "bitWriter.h"
#include <stdint.h>
#include <stdlib.h>

// Хранение битов
struct BitWriter {
    FILE* file;
    uint8_t pendingByte;
    int bitCount;
};

// Создание битового писателя
BitWriter* bitWriterCreate(FILE* file)
{
    BitWriter* writer = (BitWriter*)malloc(sizeof(BitWriter));
    if (writer == NULL)
        return NULL;

    writer->file = file;
    writer->pendingByte = 0;
    writer->bitCount = 0;
    return writer;
}

// Запись одного бита в хранилище
void bitWriterWriteBit(BitWriter* writer, int bit)
{
    if (bit)
        writer->pendingByte |= (uint8_t)(1u << (7 - writer->bitCount));

    writer->bitCount++;
    if (writer->bitCount == 8) {
        fputc(writer->pendingByte, writer->file);
        writer->pendingByte = 0;
        writer->bitCount = 0;
    }
}

// Запись неполного байта
void bitWriterFlush(BitWriter* writer)
{
    if (writer->bitCount > 0) {
        fputc(writer->pendingByte, writer->file);
        writer->pendingByte = 0;
        writer->bitCount = 0;
    }
}

void bitWriterFree(BitWriter* writer)
{
    free(writer);
}