#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "huffman.h"

//дебаг 
#if defined(DEBUG)
#define INDENT (8)

//рекурсивное сохранение узлов дерева в файл
void dump_huffman_tree(struct huffman_node *root, int space)
{
    int i = 0;
    if (root->right)
        dump_huffman_tree(root->right, space + INDENT);
    for (i = 0; i < space; i++)
        LOGI(" ");
    if (!root->left)
        LOGI("[%d:%ld]\n", root->value, root->weight);
    else
        LOGI("R:%ld\n", root->weight);
    if (root->left)
        dump_huffman_tree(root->left, space + INDENT);
}

// вывод списка кодов Хаффмана.
void dump_code_list(struct huffman_code *code_list) {
    int i = 0, j = 0;
    for (i = 0; i < TABLE_SIZE; i++) {
        if (code_list[i].length != 0) {
            LOGI("value = %d, length = %d code = ", i, code_list[i].length);
            for (j = 0; j < code_list[i].length; j++) {
                LOGI("%d", code_list[i].code[j] ? 1 : 0);
            }
            putchar(10);
        }
    }
}
#endif
// функция для быстрой сортировки (compare)
int table_unit_compar(const void *a, const void *b) {
    uint64_t x = *(uint64_t *)a;
    uint64_t y = *(uint64_t *)b;
    if (x > y)
        return 1;
    else if (x < y)
        return -1;
    else
        return 0;
}

//удаление
void clean_code_list(struct huffman_code *code_list) {
    ZAP(code_list, TABLE_SIZE);
}

//создание кода Хаффмана 
void generate_huffman_code_recusive(struct huffman_code *code_list, struct huffman_node *tree, char *code, int len) {
    if (!tree->left) {
        memcpy(code_list[tree->value].code, code, len);
        code_list[tree->value].length = len;
        return;
    }
    generate_huffman_code_recusive(code_list, tree->left, code, len + 1);
    code[len] = 1;
    generate_huffman_code_recusive(code_list, tree->right, code, len + 1);
    code[len] = 0;
}
//создание кода Хаффмана (инициализирование 0)
void generate_huffman_code(struct huffman_code * code_list, struct huffman_node *tree) {
    char code[TABLE_SIZE] = { 0 };
    generate_huffman_code_recusive(code_list, tree, code, 0);
}

//создание дерева Хаффмана
struct huffman_node * build_huffman_tree(uint64_t *table, int size) {

    int i = 0, j = 0, first_unused = 0;
    struct huffman_node *p_tree[TABLE_SIZE];
    // Максимальный номер узла это 2 * (размер таблицы) - 1
    struct huffman_node *huffman_node_list = malloc(TABLE_SIZE * 2 * sizeof(struct huffman_node));

    if (huffman_node_list == NULL) { //out of memory
        LOGE("OOM.\n");
        return NULL;
    }

    ZAP(huffman_node_list, TABLE_SIZE * 2 * sizeof(struct huffman_node)); // выделение памяти
    // инициализирование каждого узла в дереве 
    for (i = 0; i < size; i++) {
        p_tree[i] = &huffman_node_list[first_unused++];
        p_tree[i]->weight = table[i] >> 8;
        p_tree[i]->value = table[i] & 0xff;
    }
    //объединяет два дерева Хаффмана в одно
    for (i = 0; i < size - 1; i++) {
        struct huffman_node *p_node = &huffman_node_list[first_unused++];
        p_node->weight = p_tree[i]->weight + p_tree[i + 1]->weight;
        p_node->left = p_tree[i];
        p_node->right = p_tree[i + 1];
        p_tree[i + 1] = p_node;
        p_tree[i] = NULL;
        //удостовериться, что список упорядочен 
        for (j = i + 1; j < size - 1; j++) {
            if (p_tree[j]->weight > p_tree[j + 1]->weight) {
                struct huffman_node *node = p_tree[j + 1];
                p_tree[j + 1] = p_tree[j];
                p_tree[j] = node;
            } else {
                break;
            }
        }
    }
    p_tree[size - 1]->__free_handle = huffman_node_list;
    return p_tree[size - 1];
}

//удаление дерева Хаффмана
void desotry_huffman_tree(struct huffman_node * p_tree) {
    if (p_tree->__free_handle != NULL)
        free(p_tree->__free_handle);
}

//входной буфер in  и выходной буфер out 
int encode(struct buffer_ops *in, struct buffer_ops *out) {

    int c;
    uint8_t cached_c = 0, used_bits = 0;
    uint64_t table[TABLE_SIZE] = { 0 };
    struct huffman_node *tree = NULL;
    struct huffman_file_header fh;
    struct huffman_code code_list[TABLE_SIZE];
    // генерирует таблицу
    while(!in->eof(in)) {
        in->read(in, &c, 1);
        table[c]++;
    }
    in->rewind(in);
    // преобразует каждый элемент в таблице
    for (c = 0; c < TABLE_SIZE; c++) { 
        table[c] = GEN_TABLE_UNIT(c, table[c]);
    }
    //сортируем
    qsort(table, TABLE_SIZE, sizeof(uint64_t), table_unit_compar);
    // найти первый ненулевой элемент
    for (c = 0; c < TABLE_SIZE; c++)
        if (table[c] >> 8)
            break;
    // нужно минимум 2 ед. в таблице
    if (c > TABLE_SIZE - 2) {
        c = TABLE_SIZE - 2;
    }
    // строится дерево
    tree = build_huffman_tree(table + c, TABLE_SIZE - c);
    if (!tree)
        return 1;
//вывод в консоль при дебаге 
#ifdef DEBUG 
    dump_huffman_tree(tree, 0); 
#endif
    clean_code_list(code_list);
    generate_huffman_code(code_list, tree);
////вывод в консоль при дебаге
#ifdef DEBUG
    dump_code_list(code_list);
#endif


    ZAP(&fh, sizeof(fh));// инициализация памяти
    memcpy(fh.magic, MAGIC, sizeof(MAGIC)); // для отладки
    fh.file_size = tree->weight;
    LOGI("File size = %lu\n", tree->weight); // вывод размера файла
    fh.table_size = TABLE_SIZE - c;
    LOGI("Table size = %u\n", TABLE_SIZE - c); // вывод размера таблицы
    out->write(out, &fh, sizeof(struct huffman_file_header)); // записывает размер для декодера 
    out->write(out, table + c, (TABLE_SIZE - c) * sizeof(uint64_t));

    // читает букву -> переводит ее по таблице -> зписывает в out
    while (!in->eof(in)) {
        int i = 0;
        in->read(in, &c, 1);
        for (i = 0; i < code_list[c].length; i++) {
            if (code_list[c].code[i]) {
                SETB(cached_c, 7 - used_bits);
            }
            used_bits++;
            if (used_bits == 8) {
                out->write(out, &cached_c, 1);
                used_bits = 0;
                cached_c = 0;
            }
        }
    }
    if (used_bits)
        out->write(out, &cached_c, 1);

    desotry_huffman_tree(tree);  // удаление дерева Хаффмана

    return 0;
}
// декодирование 
int decode(struct buffer_ops *in, struct buffer_ops *out) {
    int cached_c = 0, used_bits = 0;
    uint64_t table[TABLE_SIZE] = { 0 };
    uint64_t size = 0;
    struct huffman_file_header fh;
    struct huffman_node *tree = NULL,*walk;
    // обраюотка ошибок и вывод их в консоль
    if (in->read(in, &fh, sizeof(fh)) != sizeof(fh)) {
        LOGE("Read file header failed.\n");
        return 1;
    }
    if (strcmp(fh.magic, MAGIC)) {
        LOGE("Miss magic number, abort.\n");
        return 1;
    }
    if (in->read(in, table, sizeof(uint64_t) * fh.table_size) != fh.table_size * sizeof(uint64_t)) {
        LOGE("Read table failed.\n");
        return 1;
    }

    if (fh.table_size > TABLE_SIZE) {
        LOGE("Table size is invalid.\n");
        return 1;
    }

    tree = build_huffman_tree(table, fh.table_size); // построение дерева
    if (!tree)
        return 1;
#ifdef DEBUG
    dump_huffman_tree(tree, 0); // вывод дерева в консоль при дебаге
#endif
    // идем по дереву
    walk = tree;
    while (size < fh.file_size) {
        if (!used_bits) {
            if (in->eof(in)) {
                LOGE("Unexpect file end, size = %lu\n", size); // неожиданый конец файла (ошибка)
                return 1;
            }
            in->read(in, &cached_c, 1);
        }

        if (GETB(cached_c, 7 - used_bits))
            walk = walk->right;
        else
            walk = walk->left;

        used_bits++;
        if (used_bits == 8)
            used_bits = 0;

        if (!walk->left) {
            out->write(out, &walk->value, 1);
            walk = tree;
            size++;
        }
    }
    desotry_huffman_tree(tree); // / удаление дерева
    return 0;
}
