#ifndef __HUNFFMAN_H__
#define __HUNFFMAN_H__
// страж включения 

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

// макросы для бинарных операций 
#define ZAP(x, len) (memset((x), 0, (len)))
#define SETB(x, b) ((x) |= ((1) << (b)))
#define RSETB(x, b) ((x) &= (~((1) << (b))))
#define GETB(x, b) ((x) & (1 << (b)))

#define TABLE_SIZE (256) // таблица для хранения кодов 

//структура для узла в дереве
struct huffman_node {
    uint64_t weight;
    uint8_t value;
    struct huffman_node *left;
    struct huffman_node *right;
    //указатель на прошлый корневой узел 
    struct huffman_node *__free_handle;
};

//структура для таблицы
struct huffman_code {
    // длина кода должна быть меньше, чем длина таблицы
    uint8_t code[TABLE_SIZE];
    uint8_t length;
};

//размер файла и размер его таблицы
struct huffman_file_header {
    char magic[8];
    uint64_t file_size;
    uint32_t table_size;
};

//буфер для чтения из файла
struct buffer_ops {
    void *data;
    int (*eof)(struct buffer_ops *handle);
    int (*read)(struct buffer_ops *handle, void *buffer, size_t);
    int (*write)(struct buffer_ops *handle, void *data, size_t);
    int (*rewind)(struct buffer_ops *handle);
    void *priv_data;
};

//кодирование и декодирование
int encode(struct buffer_ops *in, struct buffer_ops *out);
int decode(struct buffer_ops *in, struct buffer_ops *out);

// нижние 8 бит хранят длину, верхние 56 бит хранят размер кода
#define GEN_TABLE_UNIT(d, f) ((d) | ((f) << 8))

//дебаг
#define LOGE(msg, ...) fprintf(stderr, msg, ## __VA_ARGS__)
#define LOGI(msg, ...) fprintf(stderr, msg, ## __VA_ARGS__)

//макросы для отладки 
#define MAGIC ("HUFFMAN")
#if defined(DEBUG)
#define INDENT (8)
#endif

#endif
