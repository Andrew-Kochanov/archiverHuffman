#pragma once
#include <stdint.h>
#include <stdio.h>

// ||           Битовый ввод         ||

// Читатель битов

typedef struct {
    FILE* file;
    uint8_t currentByte;
    int bitsRemaining;
} BitReader;

// Инициализация битового читателя

void bitReaderInit(BitReader* reader, FILE* file);

// Чтение одного бита

int bitReaderReadBit(BitReader* reader);

// ||           Битовый вывод         ||

// Хранение битов

typedef struct {
    FILE* file;
    uint8_t pendingByte;
    int bitCount;
} BitWriter;

// Инициализация битового писателя

void bitWriterInit(BitWriter* writer, FILE* file);

// Запись одного бита в хранилище

void bitWriterWriteBit(BitWriter* writer, int bit);

// Запись неполного байта

void bitWriterFlush(BitWriter* writer);