#pragma once
#include "huffmanTree.h"
#include <string.h>
#include <stdint.h>

// Одна запись таблицы кодов

typedef struct {
    uint8_t bits[256];
    int length;
    int isCached;
} PrefCode;

// Таблица префиксных кодов

typedef struct {
    PrefCode entries[256];
} Table;

// Инициализация таблицы

void tableInit(Table* table);

// Возвращает указатель на запись в таблице с кодом для переданного листа
// Если код ещё не вычислялся — вычисляет его и сохраняет в таблице

const PrefCode* tableGet(Table* table, Node* leaf);