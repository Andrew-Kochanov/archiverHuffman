#include "bitIO.h"

// ||           Битовый ввод         ||

// Инициализация битового читателя

void bitReaderInit(BitReader* reader, FILE* file)
{
    reader->file = file;
    reader->currentByte = 0;
    reader->bitsRemaining = 0;
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

// ||           Битовый вывод         ||

// Инициализация битового писателя

void bitWriterInit(BitWriter* writer, FILE* file)
{
    writer->file = file;
    writer->pendingByte = 0;
    writer->bitCount = 0;
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