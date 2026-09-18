#include "prefCodeTable.h"
#include <string.h>

// Инициализация таблицы

void tableInit(Table* table)
{
    memset(table, 0, sizeof(*table));
}

// Возвращает указатель на запись в таблице с кодом для переданного листа
// Если код ещё не вычислялся — вычисляет его и сохраняет в таблице

const PrefCode* tableGet(Table* table, Node* leaf)
{
    PrefCode* entry = &table->entries[leaf->byte];

    if (entry->isCached)
        return entry;

    // Поднимаемся от листа к корню, собирая биты в обратном порядке
    uint8_t reversedBits[256];
    int reversedLength = 0;

    for (Node* currentNode = leaf; currentNode->parent != NULL; currentNode = currentNode->parent) {
        reversedBits[reversedLength++] = (uint8_t)(currentNode->isLeft ? 0 : 1);
    }

    // Разворачиваем последовательность: получаем код от корня к листу

    entry->length = reversedLength;
    for (int index = 0; index < reversedLength; index++)
        entry->bits[index] = reversedBits[reversedLength - 1 - index];

    entry->isCached = 1;
    return entry;
}