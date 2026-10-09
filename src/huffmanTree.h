#pragma once
#include <stdint.h>

// Узел бинарного дерева
typedef struct Node Node;

// Создание дерева Хаффмана
Node* huffmanTreeBuild(const uint64_t frequencies[256], Node* leafNodes[256]);

// Очистка дерева
void freeTree(Node* root);

// Проверка на левого ребенка
int nodeIsLeft(const Node* node);

// Проверка на лист
int nodeIsLeaf(const Node* node);

// Получение частоты узла
uint64_t nodeGetFrequency(const Node* node);

// Получение байта из узла
uint8_t nodeGetByte(const Node* node);

// Получение левого ребенка
Node* nodeGetLeft(const Node* node);

// Получение правого ребенка
Node* nodeGetRight(const Node* node);

// Получение родителя
Node* nodeGetParent(const Node* node);
