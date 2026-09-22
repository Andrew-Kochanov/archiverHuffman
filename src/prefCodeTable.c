#include "prefCodeTable.h"
#include <stdlib.h>

// Одна запись таблицы кодов
struct PrefCode {
    uint8_t bits[256];
    int length;
    int isCached;
};

// Таблица префиксных кодов
struct Table {
    PrefCode entries[256];
};

// Создание таблицы
Table* tableCreate(void)
{
    return (Table*)calloc(1, sizeof(Table));
}

void tableFree(Table* table)
{
    free(table);
}

// Возвращает указатель на запись в таблице с кодом для переданного листа
// Если код ещё не вычислялся — вычисляет его и сохраняет в таблице
const PrefCode* tableGet(Table* table, Node* leaf)
{
    PrefCode* entry = &table->entries[nodeGetByte(leaf)];

    if (entry->isCached)
        return entry;

    // Поднимаемся от листа к корню, собирая биты в обратном порядке
    uint8_t reversedBits[256];
    int reversedLength = 0;

    for (Node* currentNode = leaf; nodeGetParent(currentNode) != NULL; currentNode = nodeGetParent(currentNode)) {
        reversedBits[reversedLength++] = (uint8_t)(nodeIsLeft(currentNode) ? 0 : 1);
    }

    // Разворачиваем последовательность: получаем код от корня к листу
    entry->length = reversedLength;
    for (int index = 0; index < reversedLength; index++)
        entry->bits[index] = reversedBits[reversedLength - 1 - index];

    entry->isCached = 1;
    return entry;
}

int prefCodeGetLength(const PrefCode* code)
{
    return code->length;
}

int prefCodeGetBit(const PrefCode* code, int bitIndex)
{
    return code->bits[bitIndex];
}
