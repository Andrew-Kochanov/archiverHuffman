#pragma once
#include "huffmanTree.h"

// Min-куча
typedef struct MinHeap MinHeap;

// Создание min-кучи
MinHeap* minHeapCreate(int initialCapacity);

// Очистка min-кучи
void freeMinHeap(MinHeap* heap);

// Добавление узла в кучу
int minHeapPush(MinHeap* heap, Node* node);

// Извлечение min узла в куче
Node* minHeapPop(MinHeap* heap);

// Получение размера кучи
int minHeapGetSize(const MinHeap* heap);

// Получение узла кучи
Node* minHeapGetNode(const MinHeap* heap, int indexNode);
