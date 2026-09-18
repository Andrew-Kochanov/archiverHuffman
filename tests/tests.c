#include "huffman.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEMPORARY_COMPRESSED_PATH "temporary_compressed.bin"
#define TEMPORARY_RESTORED_PATH "temporary_restored.bin"

static const char* testFileNames[] = {
    "tests/input/test.txt"
};

static const int testFileCount = (int)(sizeof(testFileNames) / sizeof(testFileNames[0]));

// Побайтовое сравнение двух файлов
static bool filesAreEqual(const char* firstPath, const char* secondPath)
{
    FILE* firstFile = fopen(firstPath, "rb");
    if (firstFile == NULL) {
        fprintf(stderr, "Не удалось открыть файл: %s\n", firstPath);
        return false;
    }

    FILE* secondFile = fopen(secondPath, "rb");
    if (secondFile == NULL) {
        fprintf(stderr, "Не удалось открыть файл: %s\n", secondPath);
        fclose(firstFile);
        return false;
    }

    bool areEqual = true;
    int firstByte;
    int secondByte;

    do {
        firstByte = fgetc(firstFile);
        secondByte = fgetc(secondFile);
        if (firstByte != secondByte) {
            areEqual = false;
            break;
        }
    } while (firstByte != EOF && secondByte != EOF);

    fclose(firstFile);
    fclose(secondFile);
    return areEqual;
}

static void runRoundTripTest(const char* inputPath)
{
    int compressResult = huffmanCompress(inputPath, TEMPORARY_COMPRESSED_PATH);
    assert(compressResult == 0 && "Сжатие завершилось с ошибкой");

    int decompressResult = huffmanDecompress(TEMPORARY_COMPRESSED_PATH,
        TEMPORARY_RESTORED_PATH);
    assert(decompressResult == 0 && "Распаковка завершилась с ошибкой");

    assert(filesAreEqual(inputPath, TEMPORARY_RESTORED_PATH) && "Восстановленный файл не совпадает с оригиналом");

    remove(TEMPORARY_COMPRESSED_PATH);
    remove(TEMPORARY_RESTORED_PATH);

    printf("PASS: %s\n", inputPath);
}

int main(void)
{
    for (int testIndex = 0; testIndex < testFileCount; testIndex++)
        runRoundTripTest(testFileNames[testIndex]);

    printf("All tests passed.\n");
    return 0;
}