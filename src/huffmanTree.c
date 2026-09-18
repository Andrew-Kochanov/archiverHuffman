#include "huffmanTree.h"
#include <stdlib.h>

// ||           Бинарное дерево           ||

// Функция создания узла

static Node* nodeCreate(uint64_t frequency, uint8_t byte)
{
    Node* node = (Node*)calloc(1, sizeof(Node));
    if (node == NULL)
        return NULL;
    node->frequency = frequency;
    node->byte = byte;
    return node;
}

// Функция очистки дерева

void freeTree(Node* root)
{
    if (root == NULL)
        return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

// ||           Min-куча           ||

typedef struct {
    Node** nodes;
    int size;
    int capacity;
} MinHeap;

// Создание min-кучи

static MinHeap* minHeapCreate(int initialCapacity)
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

static void freeMinHeap(MinHeap* heap)
{
    if (heap == NULL)
        return;
    free(heap->nodes);
    free(heap);
}

// Перестановка 2-ух узлов в куче

static void minHeapSwap(Node** first, Node** second)
{
    Node* temporary = *first;
    *first = *second;
    *second = temporary;
}

// Добавление узла в кучу

static int minHeapPush(MinHeap* heap, Node* node)
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
        if (heap->nodes[parentIndex]->frequency <= heap->nodes[index]->frequency)
            break;
        minHeapSwap(&heap->nodes[parentIndex], &heap->nodes[index]);
        index = parentIndex;
    }
    return 0;
}

// Извлечение min узла в куче

static Node* minHeapPop(MinHeap* heap)
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

        if (leftIndex < heap->size && heap->nodes[leftIndex]->frequency < heap->nodes[smallestIndex]->frequency)
            smallestIndex = leftIndex;
        if (rightIndex < heap->size && heap->nodes[rightIndex]->frequency < heap->nodes[smallestIndex]->frequency)
            smallestIndex = rightIndex;

        if (smallestIndex == index)
            break;
        minHeapSwap(&heap->nodes[index], &heap->nodes[smallestIndex]);
        index = smallestIndex;
    }
    return minimum;
}

// ||       Построение дерева Хаффмана        ||

Node* huffmanTreeBuild(const uint64_t frequencies[256], Node* leafNodes[256])
{
    MinHeap* heap = minHeapCreate(256);
    if (heap == NULL)
        return NULL;

    int nonZeroCount = 0;

    for (int byteIndex = 0; byteIndex < 256; byteIndex++) {
        if (leafNodes != NULL)
            leafNodes[byteIndex] = NULL;

        if (frequencies[byteIndex] == 0)
            continue;

        Node* leaf = nodeCreate(frequencies[byteIndex],
            (uint8_t)byteIndex);
        if (leaf == NULL || minHeapPush(heap, leaf) != 0) {
            freeTree(leaf);
            for (int heapIndex = 0; heapIndex < heap->size; heapIndex++)
                freeTree(heap->nodes[heapIndex]);
            freeMinHeap(heap);
            return NULL;
        }

        if (leafNodes != NULL)
            leafNodes[byteIndex] = leaf;
        nonZeroCount++;
    }

    if (nonZeroCount == 0) {
        freeMinHeap(heap);
        return NULL;
    }

    if (nonZeroCount == 1) {
        Node* onlyLeaf = minHeapPop(heap);
        freeMinHeap(heap);
        return onlyLeaf;
    }

    while (heap->size > 1) {
        Node* leftLeaf = minHeapPop(heap);
        Node* rightLeaf = minHeapPop(heap);

        Node* parent = nodeCreate(leftLeaf->frequency + rightLeaf->frequency, 0);
        if (parent == NULL) {
            freeTree(leftLeaf);
            freeTree(rightLeaf);
            for (int heapIndex = 0; heapIndex < heap->size; heapIndex++)
                freeTree(heap->nodes[heapIndex]);
            freeMinHeap(heap);
            return NULL;
        }

        parent->left = leftLeaf;
        parent->right = rightLeaf;
        leftLeaf->parent = parent;
        leftLeaf->isLeft = 1;
        rightLeaf->parent = parent;
        rightLeaf->isLeft = 0;

        if (minHeapPush(heap, parent) != 0) {
            freeTree(parent);
            for (int heapIndex = 0; heapIndex < heap->size; heapIndex++)
                freeTree(heap->nodes[heapIndex]);
            freeMinHeap(heap);
            return NULL;
        }
    }

    Node* root = minHeapPop(heap);
    freeMinHeap(heap);
    return root;
}