#include "huffman.h"
#include <stdio.h>
#include <string.h>

int main(int argumentCount, char** argumentValues)
{
    if (argumentCount != 4) {
        fprintf(stderr,
            "Использование:\n"
            "  %s -c <входной_файл> <сжатый_файл>       # сжатие\n"
            "  %s -d <сжатый_файл>  <восстановленный>   # распаковка\n",
            argumentValues[0], argumentValues[0]);
        return 1;
    }

    const char* mode = argumentValues[1];
    const char* inputPath = argumentValues[2];
    const char* outputPath = argumentValues[3];

    if (strcmp(mode, "-c") == 0)
        return huffmanCompress(inputPath, outputPath) == 0 ? 0 : 1;
    if (strcmp(mode, "-d") == 0)
        return huffmanDecompress(inputPath, outputPath) == 0 ? 0 : 1;

    fprintf(stderr, "Неизвестный режим: %s\n", mode);
    return 1;
}