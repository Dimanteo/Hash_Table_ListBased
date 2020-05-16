#include <valgrind/callgrind.h>

#define NDEBUG
#include "My_Headers/txt_files.h"
#include "My_Headers/Hash_Table_t/My_Hash_Table.h"

const size_t MAX_WORD_LEN = 100;
const size_t HASH_TABLE_SIZE = 20000;

extern "C" unsigned int gnu_hash(char* data, size_t size);
/*unsigned int gnu_hash(char* name, size_t size)
{
    unsigned int h = 5381;

    for (int i = 0; i < size; i++) {
        h = (h << 5) + h + name[i];
    }

    return h;
}*/

void inflate_table(Hash_Table_t *table, const char *buffer, size_t size);

void gen_csv (Hash_Table_t* table, const char hash_name[]);

void gen_hash_distribution(char* dataset, size_t data_size, const char *hashname, unsigned int (*hash)(char*, size_t));


int main() {
    size_t size = 0;
    char* buffer = read_file_to_buffer_alloc("engmix.txt", "r", &size);
    assert(buffer);
    gen_hash_distribution(buffer, size, "GNU hash", gnu_hash);

    printf("Finished\n");

    free(buffer);
    return 0;
}

void inflate_table(Hash_Table_t *table, char *buffer, size_t size) {
    char* ptr = buffer;
    while (ptr < buffer + size - 1) {
        char word[MAX_WORD_LEN] = {};
        int step = 0;
        sscanf(ptr, "%s%n", word, &step);
        ptr += step;
        htable_add(table, word, word);
    }
}

void emulate_search(Hash_Table_t* table, char* buffer, size_t size)
{
    char* ptr = buffer;
    while (ptr < buffer + size - 1)
    {
        char word[MAX_WORD_LEN] = {};
        int step = 0;
        sscanf(ptr, "%s%n", word, &step);
        ptr += step;
        bool valid = true;
        for (size_t i = 0; i < 5; i++)      // on every put 5 searches performs
        {
            CALLGRIND_TOGGLE_COLLECT;       // profiling on
            htable_get(table, word, &valid);
            CALLGRIND_TOGGLE_COLLECT;       // profiling off
        }
    }
}

void gen_hash_distribution(char* dataset, size_t data_size, const char *hashname, unsigned int (*hash)(char*, size_t)) {
    Hash_Table_t table = {};
    htable_init(&table, HASH_TABLE_SIZE, hash);

    inflate_table(&table, dataset, data_size);
    emulate_search(&table, dataset, data_size);
    htable_destruct(&table);
}

int rot(int x, int step) {
    return (x >> (sizeof(x) - step)) | (x << step);
}
