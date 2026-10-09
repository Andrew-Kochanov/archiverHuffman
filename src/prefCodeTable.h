#pragma once
#include "huffmanTree.h"
#include <stdint.h>
#include <string.h>

// Одна запись таблицы кодов
typedef struct PrefCode PrefCode;

// Таблица префиксных кодов
typedef struct Table Table;

// Создание таблицы
Table* tableCreate(void);

// Возвращает указатель на запись в таблице с кодом для переданного листа
// Если код ещё не вычислялся — вычисляет его и сохраняет в таблице
const PrefCode* tableGet(Table* table, Node* leaf);

// Получение длины кода в битах
int prefCodeGetLength(const PrefCode* code);

// Получение бита кода по индексу, возвращает 0 или 1
int prefCodeGetBit(const PrefCode* code, int bitIndex);

// Освобождение таблицы
void tableFree(Table* table);
