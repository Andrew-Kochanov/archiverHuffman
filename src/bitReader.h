#pragma once
#include <stdio.h>

// Читатель битов
typedef struct BitReader BitReader;

// Создание битового читателя
BitReader* bitReaderCreate(FILE* file);

// Чтение одного бита
int bitReaderReadBit(BitReader* reader);

// Освобождение битового читателя
void bitReaderFree(BitReader* reader);
