#pragma once
#include <stdint.h>
#include <stdio.h>

// Сохранения исходного размера файла и частот символов в заголовке сжатого файла
void writeUint64(FILE* file, uint64_t value);

// Восстанавливает из заголовка исходный размер файла и частоты символов, возвращает 0 при преждевременном EOF
uint64_t readUint64(FILE* file);

// Сохраняет в заголовке количество уникальных символов
void writeUint16(FILE* file, uint16_t value);

// Восстанавливает количество уникальных символов
uint16_t readUint16(FILE* file);