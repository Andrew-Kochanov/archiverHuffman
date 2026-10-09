#pragma once
#include <stdio.h>

// Хранение битов
typedef struct BitWriter BitWriter;

// Создание битового писателя
BitWriter* bitWriterCreate(FILE* file);

// Запись одного бита в хранилище
void bitWriterWriteBit(BitWriter* writer, int bit);

// Запись неполного байта
void bitWriterFlush(BitWriter* writer);

// Освобождение битового писателя
void bitWriterFree(BitWriter* writer);
