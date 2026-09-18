#include "huffman.h"
#include "bitIO.h"
#include "huffmanTree.h"
#include "prefCodeTable.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ||       Запись и чтение целых чисел (в порядке little-endian)       ||

// Сохранения исходного размера файла и частот символов в заголовке сжатого файла

static void writeUint64(FILE* file, uint64_t value)
{
    for (int byteIndex = 0; byteIndex < 8; byteIndex++) {
        fputc((int)(value & 0xFF), file);
        value >>= 8;
    }
}

// Восстанавливает из заголовка исходный размер файла и частоты символов, возвращает 0 при преждевременном EOF

static uint64_t readUint64(FILE* file)
{
    uint64_t value = 0;
    for (int byteIndex = 0; byteIndex < 8; byteIndex++) {
        int inputByte = fgetc(file);
        if (inputByte == EOF)
            return 0;
        value |= ((uint64_t)(uint8_t)inputByte) << (byteIndex * 8);
    }
    return value;
}

// Сохраняет в заголовке количество уникальных символов

static void writeUint16(FILE* file, uint16_t value)
{
    fputc((int)(value & 0xFF), file);
    fputc((int)((value >> 8) & 0xFF), file);
}

// Восстанавливает количество уникальных символов

static uint16_t readUint16(FILE* file)
{
    int lowByte = fgetc(file);
    int highByte = fgetc(file);
    if (lowByte == EOF || highByte == EOF)
        return 0;
    return (uint16_t)lowByte | (uint16_t)((uint16_t)highByte << 8);
}

// Закрывает файл, если он открыт

static void closeFileIfOpen(FILE** filePointer)
{
    if (filePointer != NULL && *filePointer != NULL) {
        fclose(*filePointer);
        *filePointer = NULL;
    }
}

// Сжимает файл по алгоритму Хаффмана (возвращает 0 при успехе, -1 при неудаче)

int huffmanCompress(const char* inputPath, const char* outputPath)
{

    if (strcmp(inputPath, outputPath) == 0) {
        fprintf(stderr, "Входной и выходной файлы не должны совпадать\n");
        return -1;
    }

    FILE* inputFile = fopen(inputPath, "rb");
    if (inputFile == NULL) {
        fprintf(stderr, "Не удалось открыть входной файл: %s\n", inputPath);
        return -1;
    }

    uint64_t frequencies[256] = { 0 };
    uint64_t originalSize = 0;

    // Подсчет частот символов

    int inputByte;
    while ((inputByte = fgetc(inputFile)) != EOF) {
        frequencies[(uint8_t)inputByte]++;
        originalSize++;
    }
    rewind(inputFile);

    FILE* outputFile = fopen(outputPath, "wb");
    if (outputFile == NULL) {
        fprintf(stderr, "Не удалось создать выходной файл: %s\n", outputPath);
        closeFileIfOpen(&inputFile);
        return -1;
    }

    // Считаем уникальные байты

    uint16_t uniqBytes = 0;
    for (int byteIndex = 0; byteIndex < 256; byteIndex++) {
        if (frequencies[byteIndex] > 0)
            uniqBytes++;
    }

    // Формируем заголовок для сжатого файла
    writeUint64(outputFile, originalSize);
    writeUint16(outputFile, uniqBytes);
    for (int byteIndex = 0; byteIndex < 256; byteIndex++) {
        if (frequencies[byteIndex] > 0) {
            fputc(byteIndex, outputFile);
            writeUint64(outputFile, frequencies[byteIndex]);
        }
    }

    if (originalSize == 0) {
        closeFileIfOpen(&inputFile);
        closeFileIfOpen(&outputFile);
        return 0;
    }

    // Строим дерево и массив указателей на листья

    Node* leafNodes[256];
    Node* root = huffmanTreeBuild(frequencies, leafNodes);
    if (root == NULL) {
        fprintf(stderr, "Не удалось построить дерево Хаффмана\n");
        closeFileIfOpen(&inputFile);
        closeFileIfOpen(&outputFile);
        return -1;
    }

    // если файл состоит из одного уникального символа, то просто в заголовке пишем "символ X встречается
    // originalSize раз"

    if (root->left == NULL && root->right == NULL) {
        freeTree(root);
        closeFileIfOpen(&inputFile);
        closeFileIfOpen(&outputFile);
        return 0;
    }

    Table codeTable;
    tableInit(&codeTable);

    BitWriter bitWriter;
    bitWriterInit(&bitWriter, outputFile);

    // Идём по файлу, для каждого байта берем его код (из таблицы или вычисляем на лету) и пишем биты по одному

    for (uint64_t byteIndex = 0; byteIndex < originalSize; byteIndex++) {
        int currentByte = fgetc(inputFile);
        if (currentByte == EOF) {
            fprintf(stderr, "Неожиданный конец входного файла\n");
            freeTree(root);
            closeFileIfOpen(&inputFile);
            closeFileIfOpen(&outputFile);
            return -1;
        }

        Node* leaf = leafNodes[(uint8_t)currentByte];
        if (leaf == NULL) {
            fprintf(stderr, "Не найден лист для байта 0x%02X\n", currentByte);
            freeTree(root);
            closeFileIfOpen(&inputFile);
            closeFileIfOpen(&outputFile);
            return -1;
        }

        // код символа уже лежит в таблице или вычисляется прямо здесь
        const PrefCode* code = tableGet(&codeTable, leaf);
        for (int bitIndex = 0; bitIndex < code->length; bitIndex++)
            bitWriterWriteBit(&bitWriter, code->bits[bitIndex]);
    }

    bitWriterFlush(&bitWriter);

    freeTree(root);
    closeFileIfOpen(&inputFile);
    closeFileIfOpen(&outputFile);
    return 0;
}

// Восстанавливает сжатый файл(возвращает 0 при успехе, -1 при неудаче)

int huffmanDecompress(const char* inputPath, const char* outputPath)
{

    if (strcmp(inputPath, outputPath) == 0) {
        fprintf(stderr, "Входной и выходной файлы не должны совпадать\n");
        return -1;
    }

    FILE* inputFile = fopen(inputPath, "rb");
    if (inputFile == NULL) {
        fprintf(stderr, "Не удалось открыть входной файл: %s\n", inputPath);
        return -1;
    }

    // Чтение заголовка

    uint64_t originalSize = readUint64(inputFile);
    uint16_t uniqBytes = readUint16(inputFile);

    uint64_t frequencies[256] = { 0 };
    for (uint16_t pairIndex = 0; pairIndex < uniqBytes; pairIndex++) {
        int symbolByte = fgetc(inputFile);
        if (symbolByte == EOF) {
            fprintf(stderr, "Неполный заголовок сжатого файла\n");
            closeFileIfOpen(&inputFile);
            return -1;
        }
        frequencies[(uint8_t)symbolByte] = readUint64(inputFile);
    }

    FILE* outputFile = fopen(outputPath, "wb");
    if (outputFile == NULL) {
        fprintf(stderr, "Не удалось создать выходной файл: %s\n", outputPath);
        closeFileIfOpen(&inputFile);
        return -1;
    }

    // Если исходный файл был пуст

    if (originalSize == 0) {
        closeFileIfOpen(&inputFile);
        closeFileIfOpen(&outputFile);
        return 0;
    }

    // Восстанавливаем дерево по частотам

    Node* root = huffmanTreeBuild(frequencies, NULL);
    if (root == NULL) {
        fprintf(stderr, "Не удалось восстановить дерево Хаффмана\n");
        closeFileIfOpen(&inputFile);
        closeFileIfOpen(&outputFile);
        return -1;
    }

    // Если был один уникальный символ
    if (root->left == NULL && root->right == NULL) {
        uint8_t onlybyte = root->byte;
        for (uint64_t byteIndex = 0; byteIndex < originalSize; byteIndex++)
            fputc(onlybyte, outputFile);
        freeTree(root);
        closeFileIfOpen(&inputFile);
        closeFileIfOpen(&outputFile);
        return 0;
    }

    BitReader bitReader;
    bitReaderInit(&bitReader, inputFile);

    // Цикл декодирования

    for (uint64_t byteIndex = 0; byteIndex < originalSize; byteIndex++) {
        Node* currentNode = root;

        // Обход дерева от корня

        while (currentNode->left != NULL || currentNode->right != NULL) {
            int bit = bitReaderReadBit(&bitReader);
            if (bit == -1) {
                fprintf(stderr, "Неожиданный конец файла при распаковке\n");
                freeTree(root);
                closeFileIfOpen(&inputFile);
                closeFileIfOpen(&outputFile);
                return -1;
            }

            currentNode = (bit == 0) ? currentNode->left : currentNode->right;
            if (currentNode == NULL) {
                fprintf(stderr, "Некорректный битовый поток\n");
                freeTree(root);
                closeFileIfOpen(&inputFile);
                closeFileIfOpen(&outputFile);
                return -1;
            }
        }
        fputc(currentNode->byte, outputFile);
    }

    freeTree(root);
    closeFileIfOpen(&inputFile);
    closeFileIfOpen(&outputFile);
    return 0;
}
