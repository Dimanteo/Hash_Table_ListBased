//! @file
#ifndef MY_TXT_FILES
    #define MY_TXT_FILES
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <cctype>

/**
 * Structure containing pointer to the beginning and length of string
 */

struct String {
    size_t length;
    char *begin;
};

/**
 * Calculate size of file.
 * @param fp Pointer to file.
 * @return Size of file in bytes.
 */

size_t f_size(FILE *fp) {
    assert(fp != nullptr);

    int current_pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, current_pos);
    return size;
}

/**
 * Convert buffer to array of Strings.
 * @param buffer Pointer to buffer to convert.
 * @param index Pointer to array of Strings to inflate.
 * @param buf_size Size of buffer.
 * @return Size of index.
 */

size_t fill_index(char *buffer, String *index, int buf_size) {
    assert(buffer != nullptr);
    assert(index != nullptr);

    int size = 1;
    index[0].begin = buffer;

    for (char *str = strchr(buffer + 1, '\n'); str; str = strchr(str + 1, '\n')) {
        index[size - 1].length = str - index[size - 1].begin;
        *str = '\0';

        while ((str != buffer + buf_size) && isspace(str[1])) {
            str++;
        }

        if (!isspace(str[1]) && str[1] != '\0') {
            index[size++].begin = str + 1;
        }
    }
    return size;
}

/**
 * Calculate number of char in line.
 * @param Buffer Line.
 * @param Sample Char to count.
 * @return Number of sample in buffer.
 */

int count_char(char *buffer, char sample) {
    assert(buffer != nullptr);

    int counter = 0;
    for (char* ptr = buffer; ptr; ptr = strchr(ptr + 1, sample)) {
        counter++;
    }

    return counter;
}

/**
 * Print array of String to file with message before it. Message separates by \\n.
 * @param fout File pointer.
 * @param index Array of String.
 * @param n_lines number of lines in index.
 * @param message Message that will be printed before text.
 */

void file_output(FILE *fout, String *index, int n_lines, const char message[]) {
    assert(fout != nullptr);
    assert(index != nullptr);

    fprintf(fout, "%s\n", message);
    for (int i = 0; i < n_lines; ++i) {
        fprintf(fout, "%s\n", index[i].begin);
    }

    fprintf(fout, "\n");
}

/**
 * Compare String from back to begin.
 * @param s1 First string.
 * @param s2 Second string.
 * @return 1 if s1 > s2. 0 if s1 == s2. -1 if s1 < s2.
 */

int str_reverse_comparator(String *s1, String *s2) {
    assert(s1 != nullptr);
    assert(s2 != nullptr);
    assert(s1->begin != nullptr);
    assert(s2->begin != nullptr);

    char *p1 = s1->begin + s1->length;
    char *p2 = s2->begin + s2->length;

    while (!isalpha(*p1) && p1 >= s1->begin) {
        p1--;
    }
    while (!isalpha(*p2) && p2 >= s2->begin) {
        p2--;
    }

    while (p1 >= s1->begin && p2 >= s2->begin) {
        char chr1 = tolower(*p1);
        char chr2 = tolower(*p2);
        char tmp1[2] = {chr1, '\0'};
        char tmp2[2] = {chr2, '\0'};

        int comparision = strcmp(tmp1, tmp2);
        if(comparision != 0) {
            return comparision;
        }

        p1--;
        p2--;
    }

    p1++;
    p2++;

    return (p1 != s1->begin) - (p2 != s2->begin);
}

/**
 * strcmp() fo Strings.
 * @param s1 First string.
 * @param s2  Second string.
 * @return strcmp(s1, s2).
 */

int string_strcmp(String *s1, String *s2) {
    char *line1 = s1->begin;
    char *line2 = s2->begin;
    while (!isalpha(*line1) && *line1 != '\0') {
        line1++;
    }
    while (!isalpha(*line2) && *line2 != '\0') {
        line2++;
    }
    if (*line1 == '\0' || *line2 == '\0') {
        return (*line2 != '\0') - (*line1 != '\0');
    }
    return strcmp(line1, line2);
}

/**
 * Takes file "name", opens it in mode and returns pointer to array, with text from file. Puts size of array in size.
 * @param name File name.
 * @param mode "r" or "rb".
 * @param size Pointer to variable, storing array size.
 * @return Pointer to array with text from file.
 * @attention Free memory after use.
 */

char *read_file_to_buffer_alloc(const char name[], const char mode[], size_t* size) {

    FILE *fin = fopen(name, mode);
    if (fin == nullptr) {
        fprintf(stderr, "Can't read from file: %s", name);
        return nullptr;
    }

    *size = f_size(fin);
    char *buffer = (char *) calloc(*size + 1, sizeof(buffer[0]));

    if (strcmp(mode, "r") == 0) {
        *size = fread(buffer, sizeof(buffer[0]), *size, fin);
        realloc(buffer, *size + 1);
        buffer[*size] = '\0';
        fclose(fin);
        return buffer;
    }

    if (strcmp(mode, "rb") == 0) {
        fread(buffer, sizeof(buffer[0]), *size, fin);
        fclose(fin);
        return buffer;
    }

    return nullptr;
}

/**
 * Opens file for writing.
 * @param name name of file.
 * @return pointer to FILE.
 */

FILE* open_write_file(const char name[]) {
    FILE *fout = fopen(name, "w");

    if (fout == nullptr) {
        fprintf(stderr, "Can't write to file: %s", name);
        return nullptr;
    }

    return fout;
}

/**
 * Generate array of Strings. Each String ends with '\0'.
 * @param buffer Text to convert.
 * @param strings_count Variable to put size of generated array.
 * @return Pointer to array of Strings.
 */

String* parse_buffer_strings_alloc(char *buffer, size_t *strings_count) {
    int n_lines = count_char(buffer, '\n');
    String *index = (String*)calloc(n_lines, sizeof(index[0]));
    *strings_count = fill_index(buffer, index, strchr(buffer, '\0') - buffer);

    return index;
}

/**
 * Write Strings array to file
 * @param fout File pointer.
 * @param buffer Array of Strings.
 * @param buffer_size Size of buffer.
 */

void write_strings_file(FILE *fout, char *buffer, size_t buffer_size) {
    for (char *ptr = buffer; ptr <= buffer + buffer_size ; ptr = strchr(ptr, '\0') + 1) {
        fprintf(fout, "%s\n", ptr);
    }
}

#endif