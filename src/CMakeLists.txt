# Создание статической библиотеки
add_library(huffman_lib STATIC
    huffmanTree.c
    bitReader.c
    bitWriter.c
    bitIO.c
    minHeap.c
    prefCodeTable.c
    huffman.c
)

# Указание заголовочного файла
target_include_directories(huffman_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Исполняемый файл
add_executable(huffman main.c)

# Связка исполняемого файла и библиотеки
target_link_libraries(huffman PRIVATE huffman_lib)