#define NDEBUG
#include "My_Headers\txt_files.h"
#include "My_Headers\Hash_Table_t\My_Hash_Table.h"

const size_t MAX_WORD_LEN = 100;
const char CSV_FILENAME[] = "../dataBIG.csv";
const size_t HASH_TABLE_SIZE = 80;

unsigned int ascii_letter_hash(char* data, size_t size) {
    return *data;
}

unsigned int length_hash(char* data, size_t size) {
    return size;
}

unsigned int ascii_sum_hash(char* data, size_t size) {
    unsigned int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += data[i];
    }
    return sum;
}

int rot(int x, int step);

unsigned int rot_hash(char* data, size_t size) {
    if (size == 0) {
        return 0;
    }
    return rot(rot_hash(data + size - 1, size - 1), 1) ^ *(unsigned int*)data;
}

unsigned int gnu_hash(char* name, size_t size) {
    unsigned int h = 5381;

    for (int i = 0; i < size; i++) {
        h = (h << 5) + h + name[i];
    }

    return h;
}

//Хэши взятые из интернета

unsigned int JSHash (char *str, unsigned int len)
{
    unsigned int hash = 1315423911;
    unsigned int i = 0;

    for (i = 0; i < len; str++, i++) {
        hash ^= ((hash << 5) + (unsigned char)(*str) + (hash >> 2));
    }
    return hash;
}

unsigned int ELFHash (char *str, unsigned int len)
{
    unsigned int hash = 0;
    unsigned int x = 0;
    unsigned int i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = (hash << 4) + (unsigned char)(*str);
        if ((x = hash & 0xF0000000L) != 0) {
            hash ^= (x >> 24);
            hash &= ~x;
        }
    }
    return hash;
}

unsigned int ROT13Hash (char *str, unsigned int len)
{
    unsigned int hash = 0;
    unsigned int i = 0;

    for (i = 0; i < len; str++, i++) {
        hash += (unsigned char)(*str);
        hash -= (hash << 13) | (hash >> 19);
    }
    return hash;
}

//htable embedded hash - murmur hash

void inflate_table(Hash_Table_t *table, const char *buffer, size_t size);

void gen_csv (Hash_Table_t* table, const char hash_name[]);

void gen_hash_distribution(char* dataset, size_t data_size, const char *hashname, unsigned int (*hash)(char*, size_t));


int main() {

    FILE* file = fopen(HASH_TABLE_LOG_NAME, "wb");
    fclose(file); //clear log for new compilation

    size_t size = 0;
    char* buffer = read_file_to_buffer_alloc("../dataset.txt", "r", &size);
    assert(buffer);

    FILE* csv = fopen(CSV_FILENAME, "wb");
    fprintf(csv, "List №;");
    for (int i = 1; i <= HASH_TABLE_SIZE; ++i) {
        fprintf(csv, " %d; ", i);
    }
    fprintf(csv, "\n");
    fclose(csv);

    gen_hash_distribution(buffer, size, "ASCII first letter", ascii_letter_hash);
    gen_hash_distribution(buffer, size, "strlen", length_hash);
    gen_hash_distribution(buffer, size, "Sum ASCII", ascii_sum_hash);
    gen_hash_distribution(buffer, size, "ROT hash", rot_hash);
    gen_hash_distribution(buffer, size, "GNU hash", gnu_hash);
    gen_hash_distribution(buffer, size, "murmur", htable_embedded_hash);
    gen_hash_distribution(buffer, size, "ELF Hash", ELFHash);
    gen_hash_distribution(buffer, size, "JSHash", JSHash);
    gen_hash_distribution(buffer, size, "ROT13 Hash", ROT13Hash);

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

void gen_csv (Hash_Table_t* table, const char hash_name[]) {
    FILE* csv = fopen(CSV_FILENAME, "ab");

    fprintf(csv, "%s; ", hash_name);
    for (int i = 0; i < table->size; ++i) {
        fprintf(csv, "%d; ", table->index[i].size);
    }
    fprintf(csv, "\n");

    fclose(csv);
}

void gen_hash_distribution(char* dataset, size_t data_size, const char *hashname, unsigned int (*hash)(char*, size_t)) {
    Hash_Table_t table = {};
    htable_init(&table, HASH_TABLE_SIZE, hash);

    inflate_table(&table, dataset, data_size);
    gen_csv(&table, hashname);

    htable_destruct(&table);
}

int rot(int x, int step) {
    return (x >> (sizeof(x) - step)) | (x << step);
}
