#pragma once
#include <stdint.h>

// Узел бинарного дерева
typedef struct Node Node;

// Функция создания дерева Хаффмана
Node* huffmanTreeBuild(const uint64_t frequencies[256], Node* leafNodes[256]);

// Функция очистки дерева
void freeTree(Node* root);

// Функции для извлечение полей узла
uint64_t nodeGetFrequency(const Node* node);
uint8_t nodeGetByte(const Node* node);
int nodeIsLeaf(const Node* node);
Node* nodeGetLeft(const Node* node);
Node* nodeGetRight(const Node* node);
Node* nodeGetParent(const Node* node);
int nodeIsLeft(const Node* node);