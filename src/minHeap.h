#pragma once
#include "huffmanTree.h"

// Min-куча
typedef struct MinHeap MinHeap;

// Создание min-кучи
MinHeap* minHeapCreate(int initialCapacity);

// Очистка min-кучи
void freeMinHeap(MinHeap* heap);

// Перестановка 2-ух узлов в куче
void minHeapSwap(Node** first, Node** second);

// Добавление узла в кучу
int minHeapPush(MinHeap* heap, Node* node);

// Извлечение min узла в куче
Node* minHeapPop(MinHeap* heap);

int minHeapGetSize(const MinHeap* heap);

Node* minHeapGetNode(const MinHeap* heap, int indexNode);