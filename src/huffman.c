#include "bitIO.h"
#include "bitReader.h"
#include "bitWriter.h"
#include "huffman.h"
#include "huffmanTree.h"
#include "prefCodeTable.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        return HUFFMAN_ERROR_SAME_FILES;
    }

    FILE* inputFile = fopen(inputPath, "rb");
    if (inputFile == NULL) {
        fprintf(stderr, "Не удалось открыть входной файл: %s\n", inputPath);
        return HUFFMAN_ERROR_OPEN_INPUT;
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
        return HUFFMAN_ERROR_CREATE_OUTPUT;
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
        return HUFFMAN_ERROR_BUILD_TREE;
    }

    // если файл состоит из одного уникального символа, то просто в заголовке пишем "символ X встречается
    // originalSize раз"
    if (nodeIsLeaf(root)) {
        freeTree(root);
        closeFileIfOpen(&inputFile);
        closeFileIfOpen(&outputFile);
        return 0;
    }

    Table* codeTable = tableCreate();
    BitWriter* bitWriter = bitWriterCreate(outputFile);
    if (codeTable == NULL || bitWriter == NULL) {
        tableFree(codeTable);
        bitWriterFree(bitWriter);
        freeTree(root);
        fclose(inputFile);
        fclose(outputFile);
        return HUFFMAN_ERROR_MEMORY;
    }

    // Идём по файлу, для каждого байта берем его код (из таблицы или вычисляем на лету) и пишем биты по одному
    for (uint64_t byteIndex = 0; byteIndex < originalSize; byteIndex++) {
        int currentByte = fgetc(inputFile);
        if (currentByte == EOF) {
            fprintf(stderr, "Неожиданный конец входного файла\n");
            freeTree(root);
            tableFree(codeTable);
            bitWriterFree(bitWriter);
            closeFileIfOpen(&inputFile);
            closeFileIfOpen(&outputFile);
            return HUFFMAN_ERROR_READ;
        }

        Node* leaf = leafNodes[(uint8_t)currentByte];
        if (leaf == NULL) {
            fprintf(stderr, "Не найден лист для байта 0x%02X\n", currentByte);
            freeTree(root);
            tableFree(codeTable);
            bitWriterFree(bitWriter);
            closeFileIfOpen(&inputFile);
            closeFileIfOpen(&outputFile);
            return HUFFMAN_ERROR_FIND_LIST;
        }

        // код символа уже лежит в таблице или вычисляется прямо здесь
        const PrefCode* code = tableGet(codeTable, leaf);
        int codeLenght = prefCodeGetLength(code);
        for (int bitIndex = 0; bitIndex < codeLenght; bitIndex++)
            bitWriterWriteBit(bitWriter, prefCodeGetBit(code, bitIndex));
    }

    bitWriterFlush(bitWriter);

    freeTree(root);
    tableFree(codeTable);
    bitWriterFree(bitWriter);
    closeFileIfOpen(&inputFile);
    closeFileIfOpen(&outputFile);
    return 0;
}

// Восстанавливает сжатый файл(возвращает 0 при успехе, -1 при неудаче)
int huffmanDecompress(const char* inputPath, const char* outputPath)
{
    if (strcmp(inputPath, outputPath) == 0) {
        fprintf(stderr, "Входной и выходной файлы не должны совпадать\n");
        return HUFFMAN_ERROR_SAME_FILES;
    }

    FILE* inputFile = fopen(inputPath, "rb");
    if (inputFile == NULL) {
        fprintf(stderr, "Не удалось открыть входной файл: %s\n", inputPath);
        return HUFFMAN_ERROR_OPEN_INPUT;
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
            return HUFFMAN_ERROR_INVALID_FORMAT;
        }
        frequencies[(uint8_t)symbolByte] = readUint64(inputFile);
    }

    FILE* outputFile = fopen(outputPath, "wb");
    if (outputFile == NULL) {
        fprintf(stderr, "Не удалось создать выходной файл: %s\n", outputPath);
        closeFileIfOpen(&inputFile);
        return HUFFMAN_ERROR_CREATE_OUTPUT;
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
        return HUFFMAN_ERROR_BUILD_TREE;
    }

    // Если был один уникальный символ
    if (nodeIsLeaf(root)) {
        fprintf(stderr, "DEBUG: сработал спецслучай одного символа, "
                        "root->byte = 0x%02X\n",
            nodeGetByte(root));
        uint8_t onlybyte = nodeGetByte(root);
        for (uint64_t byteIndex = 0; byteIndex < originalSize; byteIndex++)
            fputc(onlybyte, outputFile);
        freeTree(root);
        closeFileIfOpen(&inputFile);
        closeFileIfOpen(&outputFile);
        return 0;
    }

    BitReader* bitReader = bitReaderCreate(inputFile);
    if (bitReader == NULL) {
        freeTree(root);
        fclose(inputFile);
        fclose(outputFile);
        return HUFFMAN_ERROR_MEMORY;
    }

    // Цикл декодирования
    for (uint64_t byteIndex = 0; byteIndex < originalSize; byteIndex++) {
        Node* currentNode = root;

        // Обход дерева от корня

        while (!nodeIsLeaf(currentNode)) {
            int bit = bitReaderReadBit(bitReader);
            if (bit == -1) {
                fprintf(stderr, "Неожиданный конец файла при распаковке\n");
                freeTree(root);
                closeFileIfOpen(&inputFile);
                closeFileIfOpen(&outputFile);
                return HUFFMAN_ERROR_READ;
            }

            currentNode = (bit == 0) ? nodeGetLeft(currentNode) : nodeGetRight(currentNode);
            if (currentNode == NULL) {
                fprintf(stderr, "Некорректный битовый поток\n");
                freeTree(root);
                closeFileIfOpen(&inputFile);
                closeFileIfOpen(&outputFile);
                return HUFFMAN_ERROR_WRITE;
            }
        }
        fputc(nodeGetByte(currentNode), outputFile);
    }

    freeTree(root);
    bitReaderFree(bitReader);
    closeFileIfOpen(&inputFile);
    closeFileIfOpen(&outputFile);
    return 0;
}
