#include "huffmanTree.h"
#include "minHeap.h"
#include <stdlib.h>

// Узел бинарного дерева
struct Node {
    uint64_t frequency;
    uint8_t byte;
    Node* parent;
    Node* left;
    Node* right;
    uint8_t isLeft;

};

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

uint64_t nodeGetFrequency(const Node* node)
{
    return node->frequency;
}

uint8_t nodeGetByte(const Node* node)
{
    return node->byte;
}

int nodeIsLeaf(const Node* node)
{
    return node->left == NULL && node->right == NULL;
}

Node* nodeGetLeft(const Node* node)
{
    return node->left;
}

Node* nodeGetRight(const Node* node)
{
    return node->right;
}

Node* nodeGetParent(const Node* node)
{
    return node->parent;
}

int nodeIsLeft(const Node* node)
{
    return node->isLeft;
}

// Построение дерева Хаффмана
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

        Node* leaf = nodeCreate(frequencies[byteIndex], (uint8_t)byteIndex);
        if (leaf == NULL || minHeapPush(heap, leaf) != 0) {
            freeTree(leaf);
            int nodeCount = minHeapGetSize(heap);
            for (int heapIndex = 0; heapIndex < nodeCount; heapIndex++)
                freeTree(minHeapGetNode(heap, heapIndex));
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

    while (minHeapGetSize(heap) > 1) {
        Node* leftLeaf = minHeapPop(heap);
        Node* rightLeaf = minHeapPop(heap);

        Node* parent = nodeCreate(leftLeaf->frequency + rightLeaf->frequency, 0);
        if (parent == NULL) {
            freeTree(leftLeaf);
            freeTree(rightLeaf);
            int nodeCount = minHeapGetSize(heap);
            for (int heapIndex = 0; heapIndex < nodeCount; heapIndex++)
                freeTree(minHeapGetNode(heap, heapIndex));
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
            int nodeCount = minHeapGetSize(heap);
            for (int heapIndex = 0; heapIndex < nodeCount; heapIndex++)
                freeTree(minHeapGetNode(heap, heapIndex));
            freeMinHeap(heap);
            return NULL;
        }
    }

    Node* root = minHeapPop(heap);
    freeMinHeap(heap);
    return root;
}