#pragma once
#include <stdint.h>

// Узел бинарного дерева

typedef struct Node {
    uint64_t frequency;
    uint8_t byte;
    struct Node* parent;
    struct Node* left;
    struct Node* right;
    uint8_t isLeft;

} Node;

// Функция создания дерева Хаффмана

Node* huffmanTreeBuild(const uint64_t frequencies[256], Node* leafNodes[256]);

// Функция очистки дерева

void freeTree(Node* root);