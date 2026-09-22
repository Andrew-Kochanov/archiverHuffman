#include "minHeap.h"
#include <stdlib.h>

// Min-куча
struct MinHeap {
    Node** nodes;
    int size;
    int capacity;
};

// Создание min-кучи
MinHeap* minHeapCreate(int initialCapacity)
{
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (heap == NULL)
        return NULL;

    heap->nodes = (Node**)malloc(sizeof(Node*) * (size_t)initialCapacity);
    if (heap->nodes == NULL) {
        free(heap);
        return NULL;
    }

    heap->size = 0;
    heap->capacity = initialCapacity;
    return heap;
}

// Очистка min-кучи
void freeMinHeap(MinHeap* heap)
{
    if (heap == NULL)
        return;
    free(heap->nodes);
    free(heap);
}

// Перестановка 2-ух узлов в куче
void minHeapSwap(Node** first, Node** second)
{
    Node* temporary = *first;
    *first = *second;
    *second = temporary;
}

// Добавление узла в кучу
int minHeapPush(MinHeap* heap, Node* node)
{
    if (heap->size >= heap->capacity) {
        int newCapacity = heap->capacity * 2;
        Node** newNodes = (Node**)realloc(heap->nodes, sizeof(Node*) * (size_t)newCapacity);
        if (newNodes == NULL)
            return -1;
        heap->nodes = newNodes;
        heap->capacity = newCapacity;
    }

    int index = heap->size++;
    heap->nodes[index] = node;

    while (index > 0) {
        int parentIndex = (index - 1) / 2;
        if (nodeGetFrequency(heap->nodes[parentIndex]) <= nodeGetFrequency(heap->nodes[index]))
            break;
        minHeapSwap(&heap->nodes[parentIndex], &heap->nodes[index]);
        index = parentIndex;
    }
    return 0;
}

// Извлечение min узла в куче
Node* minHeapPop(MinHeap* heap)
{
    if (heap->size == 0)
        return NULL;

    Node* minimum = heap->nodes[0];
    heap->nodes[0] = heap->nodes[--heap->size];

    int index = 0;
    for (;;) {
        int leftIndex = 2 * index + 1;
        int rightIndex = 2 * index + 2;
        int smallestIndex = index;

        if (leftIndex < heap->size && nodeGetFrequency(heap->nodes[leftIndex]) < nodeGetFrequency(heap->nodes[smallestIndex]))
            smallestIndex = leftIndex;
        if (rightIndex < heap->size && nodeGetFrequency(heap->nodes[rightIndex]) < nodeGetFrequency(heap->nodes[smallestIndex]))
            smallestIndex = rightIndex;

        if (smallestIndex == index)
            break;
        minHeapSwap(&heap->nodes[index], &heap->nodes[smallestIndex]);
        index = smallestIndex;
    }
    return minimum;
}

int minHeapGetSize(const MinHeap* heap)
{
    return heap->size;
}

Node* minHeapGetNode(const MinHeap* heap, int index)
{
    if (heap == NULL || index < 0 || index >= heap->size)
        return NULL;

    return heap->nodes[index];
}