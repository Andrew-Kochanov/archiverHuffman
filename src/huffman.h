#pragma once

// Сжимает файл по алгоритму Хаффмана (возвращает 0 при успехе, -1 при неудаче)

int huffmanCompress(const char* inputPath, const char* outputPath);

// Восстанавливает сжатый файл(возвращает 0 при успехе, -1 при неудаче)

int huffmanDecompress(const char* inputPath, const char* outputPath);