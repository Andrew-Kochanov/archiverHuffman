#pragma once

// Коды ошибок
typedef enum {
    HUFFMAN_ERROR_OPEN_INPUT = -1,
    HUFFMAN_ERROR_CREATE_OUTPUT = -2,
    HUFFMAN_ERROR_BUILD_TREE = -3,
    HUFFMAN_ERROR_FIND_LIST = -4,
    HUFFMAN_ERROR_READ = -5,
    HUFFMAN_ERROR_WRITE = -6,
    HUFFMAN_ERROR_INVALID_FORMAT = -7,
    HUFFMAN_ERROR_SAME_FILES = -8,
    HUFFMAN_ERROR_MEMORY = -9,
} HuffmanError;

// Сжимает файл по алгоритму Хаффмана (возвращает 0 при успехе, -1 при неудаче)
int huffmanCompress(const char* inputPath, const char* outputPath);

// Восстанавливает сжатый файл(возвращает 0 при успехе, -1 при неудаче)
int huffmanDecompress(const char* inputPath, const char* outputPath);