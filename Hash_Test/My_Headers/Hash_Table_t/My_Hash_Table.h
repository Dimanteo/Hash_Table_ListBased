#ifndef HASH_TABLE_MY_HASH_TABLE_H
#define HASH_TABLE_MY_HASH_TABLE_H

///@file
/**
 * type of Hash_Table content.
 */
typedef char* value_t;
const value_t value_POISON = nullptr;

/**
 * Type of Hash_Table key
 */
typedef char* key_t;
const key_t key_POISON = nullptr;
/**
 * Hash_Table contains List_t arrays of KV_Pair. This structure is key and value pair.
 */

struct KV_Pair {
    key_t key;
    value_t value;
};

int compare (key_t a, key_t b) {
    return strcmp(a, b);
}

bool operator == (const KV_Pair& a, const KV_Pair&b) {compare(a.key, b.key) == 0;}

//Лист содержит пары ключ значение. Это нужно для поиска элемента по ключу в случае коллизии.

#include "Hash_Table_List.h"

/**
 * Check condition and if it's not true generate htable_dump with messageas parameter.
 */
#define HASH_TABLE_ASSERT(condition, message) \
if (!(condition)) {\
    htable_dump(table, ERR_STATE, message, filename, function, line);\
    assert(condition);\
    return false;\
}

char HASH_TABLE_LOG_NAME[] = "HTable_log.txt";
const size_t HASHT_DUMP_MSG_LENGTH = 100;
const char PARENT_CALL_STATE[] = "Parent call";

/**
 * @param size size of Hash_Table array.
 * @param index array of lists..
 * @param hash pointer to hash function, which used to convert key.
 */
struct Hash_Table_t {
    size_t size;
    List_t* index;
    unsigned int (*hash)(char*, size_t);
};

/**
 * Hash_Table constructor.
 * @param table Hash_Table that will be initialized.
 * @param size Size of Hash_Table array.
 * @param hash Pointer to hash function, which used to convert key. If not initialized will use function htable_embedded_hash.
 */
void htable_init(Hash_Table_t* table, size_t size, unsigned int (*hash)(char*, size_t));

void htable_init(Hash_Table_t* table, size_t size);

/**
 * Hash_Table destructor.
 * @param table
 */

void htable_destruct(Hash_Table_t* table);

/**
 * Default Hash_Table hash function. Used if another not specified in htable_init().
 * @param buffer Pointer to memory part, that will be hashed.
 * @param length Size of memory part in bytes.
 * @return
 */

unsigned int htable_embedded_hash(char* buffer, size_t length);

/**
 * Put pair {key, value} in table.
 * @param table Pointer to Hash_Table where pair will be inserted.
 * @param key Key element in pair. It will be used to calculate hash.
 * @param value Value element in pair. It will be associated with key.
 */

void htable_add(Hash_Table_t *table, key_t key, value_t value);

/**
 * Gets value that associated with key from table. If this key not exist in table, returns value_POISON and set valid  to false.
 * @param table Pointer to Hash_Table where to search.
 * @param key Key of value that need to be extracted.
 * @param valid Set flag true if returned value is correct. Set flag false if value with that key not found.
 * @return Value that associated with key.
 */

value_t htable_get(Hash_Table_t* table, key_t key, bool* valid = nullptr);

/**
 * Verifyer for Hash_Table. Can be disabled by define NDEBUG. If define OK_DUMP generate dumps even if there no errors.
 * @param table Verifyer target.
 * @param filename File where from verifier was called.
 * @param function Function where from verifier was called.
 * @param line Line where from verifier was called.
 * @return TRUE if table is correct. FALSE if table contains error.
 */

bool htable_verify(Hash_Table_t* table, const char filename[], const char function[], int line);

/**
 * Generate dump for Hash_Table. Prints result in file with name HASH_TABLE_LOG_NAME.
 * @param table Dump target.
 * @param state Table state: error, ok or parent_call.
 * @param message Text will be printed in dump. Describe error.
 * @param filename File where from dump was called.
 * @param function Function where from verifier was called.
 * @param line Line where from verifier was called.
 */

void htable_dump(Hash_Table_t* table, const char state[], const char message[], const char filename[], const char function[], int line);

/**
 * Pop (delete and return) value associated with key from table.
 * @param table Table to operate with.
 * @param key Key to search for.
 * @param valid Set flag true if returned value is correct. Set flag false if pair with such key do not exist in this table.
 * @return removed value, associated with key.
 */

value_t htable_remove(Hash_Table_t* table, key_t key, bool* valid = nullptr);

/**
 * Returns pointer to list, where key should be placed.
 * @param table Hash_Table where to search for list.
 * @return Pointer to list, that should contain key.
 */

List_t* htable_getList(Hash_Table_t* table, key_t key);



void htable_init(Hash_Table_t *table, size_t size, unsigned int (*hash_function)(char *, size_t)) {
    table->size = size;
    table->hash = hash_function;
    table->index = (List_t*)calloc(size, sizeof(List_t));
    for (int i = 0; i < size; ++i) {
        List_t list = {};
        table->index[i] = list;
        list_init(&table->index[i], (size_t)1, HASH_TABLE_LOG_NAME);
    }

    htable_verify(table, VERIFY_CONTEXT);
}

void htable_init(Hash_Table_t *table, size_t size) {
    htable_init(table, size, htable_embedded_hash);
}


void htable_destruct(Hash_Table_t *table) {
    htable_verify(table, VERIFY_CONTEXT);

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

bool htable_verify(Hash_Table_t *table, const char *filename, const char *function, int line) {
#ifndef NDEBUG
    if (table == nullptr) {
        FILE* log = fopen(HASH_TABLE_LOG_NAME, "ab");
        fprintf(log, "ERROR. NULL pointer to Hash_Table %s; %s (%d).\n", filename, function, line);
        assert(table);
        return false;
    }
    HASH_TABLE_ASSERT(table->hash != nullptr, "Hash function not specified.")
    HASH_TABLE_ASSERT(table->size > 0, "Incorrect table size")

    for (int i = 0; i < table->size; ++i) {
        bool list_ok = list_verify(&table->index[i], filename, function, line);
        char msg[HASHT_DUMP_MSG_LENGTH] = {};
        sprintf(msg, "List in index[%d] ERROR", i);
        if (!list_ok) {
            htable_dump(table, ERR_STATE, msg, filename, function, line);
            return false;
        }
    }

#ifdef OK_DUMP
    htable_dump(table, OK_STATE, "It's ok ^..^", filename, function, line);
#endif
#endif
    return true;
}

void htable_dump(Hash_Table_t* table, const char *state, const char *message, const char *filename, const char *function, int line) {
    FILE* log = fopen(HASH_TABLE_LOG_NAME, "ab");

    time_t now = time(nullptr);
    fprintf(log, "\nlog from %s"
                 "Hash_Table Dump(%s) from %s; %s (%d)\n"
                 "Hash_Table_t [%p] (%s)\n"
                 "{\n"
                 "\tsize = %d\n"
                 "\thash = %p\n"
                 "\tindex = %p\n"
                 "\t{<index_dump>\n"
            , ctime(&now), message, filename, function, line, table, state, table->size, table->hash, table->index);
    fclose(log);

    for (int i = 0; i < table->size; ++i) {
        char msg[HASHT_DUMP_MSG_LENGTH] = {};
        sprintf(msg, "index[%d]", i);
        list_dump(&table->index[i], PARENT_CALL_STATE, msg, filename, function, line);
    }

    log = fopen(HASH_TABLE_LOG_NAME, "ab");
    fprintf(log, "\t</index_dump>}\n}\n");
    fclose(log);
}

List_t* htable_getList(Hash_Table_t* table, key_t key) {
    size_t size = strlen(key);
    unsigned int hash = table->hash((char*)key, size) % table->size;
    return &table->index[hash];
}

void htable_add(Hash_Table_t *table, key_t key, value_t value) {
    htable_verify(table, VERIFY_CONTEXT);

    List_t* list = htable_getList(table, key);
    for (int i = list->head; i != 0 ; i = list->data[i].next) {
        if (strcmp(key, list->data[i].value.key) == 0) {
            strcpy(list->data[i].value.value, value);
            htable_verify(table, VERIFY_CONTEXT);
            return;
        }
    }
    KV_Pair elem = {nullptr, nullptr};
    elem.value = (char*) calloc(strlen(value) + 1, sizeof(char));
    assert(elem.value);
    strcpy(elem.value, value);
    elem.key = (char*) calloc(strlen(key) + 1, sizeof(char));
    assert(elem.key);
    strcpy(elem.key, key);
    list_push_back(list, elem);
    htable_verify(table, VERIFY_CONTEXT);
}

value_t htable_get(Hash_Table_t *table, key_t key, bool* valid /*= nullptr*/) {
    htable_verify(table, VERIFY_CONTEXT);

    List_t* list = htable_getList(table, key);
    for (int i = list->head; i != 0; i = list->data[i].next) {
        if (list->data[i].value.key == key) {
            if (valid != nullptr){
                *valid = true;
            }
            return list->data[i].value.value;
        }
    }

    if (valid != nullptr){
        *valid = false;
    }
    htable_verify(table, VERIFY_CONTEXT);
    return value_POISON;
}

value_t htable_remove(Hash_Table_t *table, key_t key, bool* valid /*= nullptr*/) {
    htable_verify(table, VERIFY_CONTEXT);

    List_t* list = htable_getList(table, key);
    for (int i = list->head; i != 0; i = list->data[i].next) {
        if (list->data[i].value.key == key) {
            int error_code = 0;
            KV_Pair ret = list_remove(list, i, &error_code);
            if (valid != nullptr) {
                *valid = (error_code == 0);
            }
            return ret.value;
        }
    }

    if (valid != nullptr) {
        *valid = false;
    }
    htable_verify(table, VERIFY_CONTEXT);
    return value_POISON;
}



#endif //HASH_TABLE_MY_HASH_TABLE_H
