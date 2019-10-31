#include <iostream>
#include "My_Headers\index_list_t.h"

struct Hash_Table_t {
    size_t size;
    List_t* index;
    unsigned int (*hash)(char*, size_t);
};

void htable_init(Hash_Table_t* table, size_t  size, unsigned int (*hash)(char*, size_t));

void htable_init(Hash_Table_t* table, size_t size);

void htable_destruct(Hash_Table_t* table);

unsigned int htable_embedded_hash(char* buffer, size_t length);

int htable_push(Hash_Table_t* table, element_t elem);

int main() {

    return 0;
}

void htable_init(Hash_Table_t *table, size_t size, unsigned int (*hash_function)(char *, size_t)) {
    table->size = size;
    table->hash = hash_function;
    table->index = (List_t*)calloc(size, sizeof(List_t));
    for (int i = 0; i < size; ++i) {
        list_init(&table->index[i], (size_t)0);
    }
}

void htable_init(Hash_Table_t *table, size_t size) {
    htable_init(table, size, htable_embedded_hash);
}


void htable_destruct(Hash_Table_t *table) {
    for (int i = 0; i < table->size; ++i) {
        list_destruct(&table->index[i]);
    }
    table->size = 0;
    table->hash = nullptr;
}


unsigned int htable_embedded_hash(char* buffer, size_t length) {
    assert(buffer);

    const unsigned int magic_constant = 0x5bd1e995;
    const unsigned int seed = 42;
    unsigned int hash = seed ^ length;
    /*operations with 4 byte blocks*/
    while (length >= 4)
    {
        /*merging together 4 bytes*/
        unsigned int word = buffer[0];//[-][-][-][0]
        word |= buffer[1] << 8;//[-][-][1][0]
        word |= buffer[2] << 16;//[-][2][1][0]
        word |= buffer[3] << 24;//[3][2][1][0]

        word *= magic_constant;
        unsigned int w = word;
        unsigned int s = word;
        word ^= word >> 24;
        word *= magic_constant;
        hash *= magic_constant;
        hash ^= word;
        buffer += 4;
        length -= 4;
    }
    /*operations with unused bytes*/
    switch (length) {
        case 3:
            hash ^= buffer[2] << 16;
        case 2:
            hash ^= buffer[1] << 8;
        case 1:
            hash ^= buffer[0];
            hash *= magic_constant;
    }

    hash ^= hash >> 13;
    hash *= magic_constant;
    hash ^= hash >> 15;

    return hash;
}

