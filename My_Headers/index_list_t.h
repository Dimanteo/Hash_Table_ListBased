
#ifndef LIST_INDEX_LIST_T_H
#define LIST_INDEX_LIST_T_H

#include <stdio.h>
#include <assert.h>
#include <cstdlib>
#include <time.h>

#define VERIFY_CONTEXT __FILE__, __PRETTY_FUNCTION__, __LINE__

#define LIST_ASSERT(condition, message) \
if (!(condition)) {\
    list_dump(list, ERR_STATE, #condition ". " #message, file, function, line);\
    assert(condition);\
    return false;\
}

#define LIST_NODE_ASSERT(condition, msg) \
if (!(condition)) {\
    sprintf(message, msg "Failed: " #condition " data[%d]", i);\
    list_dump(list, ERR_STATE, message, file, function, line);\
    assert(condition);\
    return false;\
}

typedef int element_t;
#define LIST_ELEMENT_PRINT "%d"
typedef unsigned int canary_t;
#define CANARY_PRINT "%#X"
const canary_t CANARY_VALUE = 0xBEDA;
const element_t POISON = -1337;
enum LIST_T_ERRORS {LIST_T_INDEX_OUT_OF_BOUNDS_ERROR = -42, LIST_T_MEMORY_ALLOCATION_ERROR = -69, LIST_T_UNDERFLOW = -137};
const int FREE_POINTER = -1;
const char ERR_STATE[] = "ERROR";
const char OK_STATE[] = "ok";
char LIST_DEFAULT_LOG_NAME[] = "../ListLog.txt";
char DOT_LOG_NAME[] = "../DotDumps/dot_ListLog.dot";
int DOT_FILE_COUNTER = 1;
const size_t LIST_DUMP_MSG_LENGTH = 100;
const size_t DOT_QUERY_SIZE = 30;
const size_t DEFAULT_LIST_SIZE = 10;

struct List_Node {
    element_t value;
    int prev;
    int next;
};

struct List_t {
    canary_t canary1 = CANARY_VALUE;
    List_Node* data;
    int free;
    int head;
    int tail;
    size_t size;
    size_t max_size;
    char* log_name;
    canary_t canary2 = CANARY_VALUE;
};

void list_init(List_t* list, size_t size, const char log_filename[]);

void list_init(List_t* list, size_t size);

void list_init(List_t* list, const char log_filename[]);

void list_init(List_t* list);

void list_destruct(List_t* list);

void list_dump(List_t* list, const char state[], const char message[], const char file[], const char function[], int line );

void graphviz_dump(List_t* list);

bool list_verify(List_t *list, const char file[], const char function[], int line);

element_t list_get_val(List_t* list, int index, int* error_code = nullptr);

int list_get_num(List_t* list, int index);

int list_at(List_t* list, int index, element_t value);

int list_push_back(List_t* list, element_t elem);

element_t list_pop_back(List_t *list, int* error_code = nullptr);

int list_push_front(List_t* list, element_t elem);

element_t list_pop_front(List_t* list, int* error_code = nullptr);

int list_in_next(List_t* list, int index, element_t elem);

int list_in_prev(List_t* list, int index, element_t elem);

int list_seek(List_t *list, int num);

element_t list_remove(List_t* list, int index, int* error_code = nullptr);

bool list_isEmpty(List_t* list);

void data_dump(List_t* list);

int list_resize(List_t *list, size_t size, const char file[], const char function[], int line);

bool index_correct(List_t* list, int index, const char file[], const char function[], int line);

bool list_underflow_check(List_t* list, int* error_code);

bool free_check(List_t* list, const char file[], const char function[], int line);

bool data_check(List_t* list, const char file[], const char function[], int line);



void list_init(List_t *list, size_t size, const char log_filename[]) {
    assert(list);
    assert(size > 0);

    list->max_size = size;
    list->size = 0;
    list->data = (List_Node*)calloc(list->max_size + 1, sizeof(list->data[0]));
    assert(list->data);
    list->free = 1;
    list->head = 0;
    list->tail = list->head;
    list->data[0] = {0, 0, 0};//forbidden zone
    for(int i = list->free; i <= list->max_size; i++) {
        list->data[i] = {POISON, FREE_POINTER, (i == list->max_size) ? 0 : i + 1};
    }
    list->log_name = (char*)log_filename;

    list_verify(list, VERIFY_CONTEXT);
}

void list_init(List_t *list, size_t size) {
    list_init(list, size, LIST_DEFAULT_LOG_NAME);
}

void list_init(List_t *list, const char log_filename[]) {
    list_init(list, DEFAULT_LIST_SIZE, log_filename);
}

void list_init(List_t *list) {
    list_init(list, DEFAULT_LIST_SIZE, LIST_DEFAULT_LOG_NAME);
}

void list_destruct(List_t *list) {
    list_verify(list, VERIFY_CONTEXT);

    free(list->data);
    list->data = nullptr;
    list->size = 0;
    list->canary1 = 0;
    list->canary2 = 0;
    list->free = 0;
    list->max_size = 0;
    list->tail = 0;
    list->head = 0;
    list->log_name = nullptr;
}

bool data_check(List_t* list, const char file[], const char function[], int line) {
    int i = list->head;
    for (int cnt = 0; cnt < list->size; ++cnt) {
        char message[LIST_DUMP_MSG_LENGTH] = "";
        if (cnt == 0) {
            LIST_NODE_ASSERT(list->data[i].prev == 0, "Stores. First element doesn't point to 0. ")
        } else {
            LIST_NODE_ASSERT(list->data[list->data[i].prev].next == i, "Stores. Previous element doesn't point to this. ")
        }
        LIST_NODE_ASSERT(list->data[i].prev >= 0, "Stores. Next < 0. ")
        LIST_NODE_ASSERT(list->data[i].next >= 0, "Stores. Prev < 0. ")
        LIST_NODE_ASSERT(list->data[list->data[i].prev].prev >= 0, "Stores. Previous element is free. ")
        if(cnt == list->size - 1) {
            LIST_NODE_ASSERT(list->data[i].next == 0, "Stores. Last element doesn't point to 0. ")
            LIST_NODE_ASSERT(i == list->tail, "Stores. Tail not equal to last element. ")
        }else{
            LIST_NODE_ASSERT(list->data[list->data[i].next].prev == i, "Stores. Next element doesn't point to this. ")
            i = list->data[i].next;
        }
    }
    return true;
}

bool free_check(List_t* list, const char file[], const char function[], int line) {
    int i = list->free;
    for (int cnt = 0; cnt < list->max_size - list->size; ++cnt) {
        char message[100] = "";
        LIST_NODE_ASSERT(list->data[i].prev == FREE_POINTER, "Free. this prev not equal to FREE_POINTER. ")
        if (cnt == list->max_size - 1) {
            LIST_NODE_ASSERT(list->data[i].next == 0, "Free. Last element not point to 0. ")
        }
        i = list->data[i].next;
    }
    return true;
}

bool list_verify(List_t *list, const char file[], const char function[], int line) {
#ifndef NDEBUG
    if(list == nullptr) {
        fprintf(stderr, "ERROR. Null pointer to list. %s; %s (%d)\n", file, function, line);
        assert(list);
        return false;
    }

    LIST_ASSERT(list->canary1 == CANARY_VALUE, "list.canary1 has changed")
    LIST_ASSERT(list->canary2 == CANARY_VALUE, "list.canary1 has changed")
    LIST_ASSERT(list->head >= 0, "Incorrect head value")
    LIST_ASSERT(list->tail >= 0, "Incorrect tail  value")
    LIST_ASSERT(list->max_size > 0, "Incorrect max_size value")
    LIST_ASSERT(list->free > 0, "Incorrect free value")

    if(!data_check(list, file, function, line)) {
        return false;
    }
    if(!free_check(list, file, function, line)) {
        return false;
    }
#ifdef OK_DUMP
    list_dump(list, OK_STATE, "It's ok ^..^", file, function, line);
#endif

#endif
    return true;
}

void list_dump(List_t *list, const char *state, const char *message, const char *file, const char *function, int line) {
    assert(list);
    FILE* log = fopen(list->log_name, "ab");
    assert(log);

    time_t now = time(nullptr);
    fprintf(log, "\nlog from %s"
                 "List_Dump(%s) from %s; %s (%d)\n"
                 "\tList_t [%p] (%s)\n"
                 "\t{\n"
                 "\t\tcanary1 = " CANARY_PRINT "\n"
                 "\t\tmax_size = %d\n"
                 "\t\tsize = %d\n"
                 "\t\thead = %d\n"
                 "\t\ttail = %d\n"
                 "\t\tfree = %d\n"
                 "\t\tdata = %p\n"
                 "\t\tcanary2 = " CANARY_PRINT "\n"
                 "\t\tdata[%p]:\n"
                 "\t\t{\n",
            ctime(&now), message, file, function, line, list, state, list->canary1, list->max_size, list->size, list->head, list->tail,
            list->free, list->data, list->canary2, list->data);
    fclose(log);
    data_dump(list);
    log = fopen(list->log_name, "ab");
    fprintf(log, "\t\t}\n\t}\n");
    fclose(log);
}

void data_dump(List_t *list) {
    FILE* log = fopen(list->log_name, "ab");
    for (int i = 0; i <= list->max_size; ++i) {
        fprintf(log, "\t\t\t[%d]{value = " LIST_ELEMENT_PRINT "; prev = %d; next = %d;}\n", i, list->data[i].value, list->data[i].prev, list->data[i].next);
    }

    fprintf(log, "\t\t\tForward order: ");
    int i = list->head;
    for (; i != 0 ; i = list->data[i].next) {
        fprintf(log, "[%d] -> ", i);
    }

    fprintf(log, "[%d]\n\t\t\tReverse order: ", i);
    i = list->tail;
    for (; i != 0; i = list->data[i].prev) {
        fprintf(log, "[%d] <- ", i);
    }
#ifdef GRAPH_DUMPS
    fprintf(log, "[%d]\n"
                 "%d_ListDot.png contains dot representation.\n", i, DOT_FILE_COUNTER);
    graphviz_dump(list);
#endif
    fclose(log);
}

void graphviz_dump(List_t* list) {
    FILE* dotlog = fopen(DOT_LOG_NAME, "wb");

    fprintf(dotlog, "digraph G{\n"
                    "\trankdir = LR;\n"
                    "\t");

    fprintf(dotlog, "%d [shape = record, style = filled, color = \"#ff6e84\", label = \" Forbidden_zone %d | value\\n%d | {<n%d> prev\\n %d | <n%d> next\\n %d} \"];\n\t",
            0, 0, list->data[0].value, 0, list->data[0].prev, 0, list->data[0].next);
    for (int i = 1; i <= list->max_size; ++i) {

        if (i == list->head && i == list->tail) {
            fprintf(dotlog, "%d [shape = record, color = \"#4afa8a\", label = \"Head and Tail %d | value\\n%d | { <p%d> prev\\n %d | <n%d> next\\n %d} \"];\n\t",
                    i, i, list->data[i].value, i, list->data[i].prev, i, list->data[i].next);
            continue;
        }

        if (i == list->head) {
            fprintf(dotlog, "%d [shape = record, color = \"#6e9aff\", label = \"Head %d | value\\n%d | {<p%d> prev\\n %d | <n%d> next\\n %d } \"];\n\t",
                    i, i, list->data[i].value, i, list->data[i].prev, i, list->data[i].next);
            continue;
        }

        if (i == list->tail) {
            fprintf(dotlog, "%d [shape = record, color = \"#ebf527\", label = \"Tail %d | value\\n%d | {<p%d> prev\\n %d | <n%d> next\\n %d} \"];\n\t",
                    i, i, list->data[i].value, i, list->data[i].prev, i, list->data[i].next);
            continue;
        }

        fprintf(dotlog, "%d [shape = record, label = \"index %d | value\\n%d | {<p%d> prev\\n %d | <n%d> next\\n %d} \"];\n\t",
                i, i, list->data[i].value, i, list->data[i].prev, i, list->data[i].next);
    }

    int i = list->data[list->head].next;
    fprintf(dotlog, "%d: n%d", list->head, list->head);
    for (; i != 0 ; i = list->data[i].next) {
        fprintf(dotlog, " -> %d; %d : n%d", i, i, i);
    }
    fprintf(dotlog, " -> %d;\n", i);

    i = list->tail;
    for (; i != 0; i = list->data[i].prev) {
        fprintf(dotlog, "%d : p%d -> %d; ", i, i, list->data[i].prev);
    }

    for (i = list->free; i != 0; i = list->data[i].next) {
        fprintf(dotlog, "%d : n%d -> %d; ", i, i, list->data[i].next);
    }

    fprintf(dotlog, "\n}");
    fclose(dotlog);

    char sys_query[sizeof(DOT_LOG_NAME) + sizeof(DOT_FILE_COUNTER) + DOT_QUERY_SIZE];
    sprintf(sys_query, "dot -Tpng %s -o ../DotDumps/%d_ListDot.png", DOT_LOG_NAME, DOT_FILE_COUNTER);
    system(sys_query);
    DOT_FILE_COUNTER++;
}

bool index_correct(List_t* list, int index, const char file[], const char function[], int line) {
    if (index <= 0 || index > list->max_size) {
#ifndef NDEBUG
        FILE* log = fopen(list->log_name, "ab");
        fprintf(log, "\nERROR in %s; %s (%d).\n List_t index out of bounds exception. List_t[%p]: index = %d\n", file, function, line, list, index);
        fprintf(stderr, "\nERROR in %s; %s (%d).\n List_t index out of bounds exception. List_t[%p]: index = %d\n", file, function, line, list, index);
        assert(index > 0);
        assert(index <= list->max_size);
        fclose(log);
#endif
        return false;
    }
    if(list->data[index].prev == FREE_POINTER) {
#ifndef NDEBUG
        FILE* log = fopen(list->log_name, "ab");
        fprintf(log, "\nERROR. Trying to access free block. List_t[%p]: list_at(%d)\n", list, index);
        fprintf(stderr, "\nERROR. Trying to access free block. List_t[%p]: list_at(%d)\n", list, index);
        fclose(log);
#endif
        return false;
    }
    return true;
}

int list_at(List_t *list, int index, element_t elem) {
    list_verify(list, VERIFY_CONTEXT);

    if (!index_correct(list, index, VERIFY_CONTEXT)) {
        return LIST_T_INDEX_OUT_OF_BOUNDS_ERROR;
    }

    list->data[index].value = elem;

    list_verify(list, VERIFY_CONTEXT);
    return index;
}

element_t list_get_val(List_t *list, int index, int* error_code /*= nullptr*/) {
    list_verify(list, VERIFY_CONTEXT);

    if (!index_correct(list, index, VERIFY_CONTEXT)) {
        if(error_code != nullptr) {
            *error_code = LIST_T_INDEX_OUT_OF_BOUNDS_ERROR;
        }
        return 0;
    }

    if (error_code != nullptr) {
        *error_code = 0;
    }
    list_verify(list, VERIFY_CONTEXT);
    return list->data[index].value;
}

int list_push_back(List_t *list, element_t elem) {
    list_verify(list, VERIFY_CONTEXT);

    int elem_index = list->free;
    list->free = list->data[list->free].next;

    if (list->tail != 0) {
        list->data[list->tail].next = elem_index;
    }
    if(list->head == 0) {
        list->head = elem_index;
    }
    list->data[elem_index].prev = list->tail;
    list->data[elem_index].next = 0;
    list->data[elem_index].value = elem;

    list->tail = elem_index;
    list->size++;

    if (list->free == 0) {
        int er_cd = list_resize(list, list->max_size * 2, VERIFY_CONTEXT);
        if(er_cd) {
            assert(er_cd == 0);
            return 0;
        }
    }

    list_verify(list, VERIFY_CONTEXT);
    return list->tail;
}

int list_resize(List_t *list, size_t size, const char file[], const char function[], int line) {
    assert(size > 0);

    List_Node* new_data =  (List_Node*)realloc(list->data, sizeof(List_Node) * (size + 1));

    if(new_data == nullptr) {
        list_verify(list, VERIFY_CONTEXT);
#ifndef NDEBUG
        fprintf(stderr, "Allocation ERROR in %s; %s (%d)", file, function, line);
        FILE* log = fopen(list->log_name, "ab");
        fprintf(log, "\nAllocation ERROR in %s; %s (%d). List_t[%p] size = %d\n", file, function, line, list, size);
        fclose(log);
#endif
        return LIST_T_MEMORY_ALLOCATION_ERROR;
    }

    list->data = new_data;
    for (size_t i = list->max_size + 1; i <= size; ++i) {
        list->data[i] = {POISON, FREE_POINTER, (i == size) ? list->free : (int)(i + 1)};
    }
    list->free = list->max_size + 1;
    list->max_size = size;

    return 0;
}

element_t list_pop_back(List_t *list, int* error_code /*= nullptr*/) {
    list_verify(list, VERIFY_CONTEXT);
    if (!list_underflow_check(list, error_code)) {
        return POISON;
    }

    int tail = list->tail;
    int value = list->data[tail].value;
    list->data[list->data[tail].prev].next = 0;
    list->tail = list->data[tail].prev;
    if(list->size == 1) {
        list->head = list->data[tail].prev; //0
    }

    list->data[tail].next = list->free;
    list->data[tail].prev = FREE_POINTER;
    list->data[tail].value = POISON;

    list->free = tail;
    list->size--;

    list_verify(list, VERIFY_CONTEXT);
    return value;
}

bool list_isEmpty(List_t *list) {
    list_verify(list, VERIFY_CONTEXT);
    return list->size == 0;
}

int list_push_front(List_t *list, element_t elem) {
    list_verify(list, VERIFY_CONTEXT);

    int elem_index = list->free;
    list->free = list->data[list->free].next;

    if (list->head != 0) {
        list->data[list->head].prev = elem_index;
    }
    if(list->tail == 0) {
        list->tail = elem_index;
    }
    list->data[elem_index].prev = 0;
    list->data[elem_index].next = list->head;
    list->data[elem_index].value = elem;

    list->head = elem_index;
    list->size++;

    if (list->free == 0) {
        int er_cd = list_resize(list, list->max_size * 2, VERIFY_CONTEXT);
        if (er_cd) {
            assert(er_cd == 0);
            return 0;
        }
    }

    list_verify(list, VERIFY_CONTEXT);
    return list->head;
}

element_t list_pop_front(List_t *list, int *error_code) {
    list_verify(list, VERIFY_CONTEXT);
    if (!list_underflow_check(list, error_code)) {
        return POISON;
    }

    int value = list->data[list->head].value;
    int head = list->head;
    list->data[list->data[head].next].prev = 0;
    list->head = list->data[head].next;
    if (list->size == 1) {
        list->tail = list->data[head].next; //0
    }

    list->data[head].next = list->free;
    list->data[head].prev = FREE_POINTER;
    list->data[head].value = POISON;

    list->free = head;
    list->size--;

    list_verify(list, VERIFY_CONTEXT);
    return value;
}

bool list_underflow_check(List_t *list, int *error_code) {
    if (list->size == 0) {
#ifndef NDEBUG
        FILE* log = fopen(list->log_name, "ab");
        fprintf(log, "ERROR. Pop from empty List_t.[%p]", list);
        fprintf(stderr, "ERROR. Pop from empty List_t.[%p]", list);
        fclose(log);
#endif
        if (error_code != nullptr) {
            *error_code = LIST_T_UNDERFLOW;
        }
        return false;
    }
    if (error_code != nullptr) {
        *error_code = 0;
    }
    return true;
}

int list_in_next(List_t *list, int index, element_t elem) {
    list_verify(list, VERIFY_CONTEXT);
    if (!index_correct(list, index, VERIFY_CONTEXT)) {
        return 0;
    }

    int elem_index = list->free;
    list->free = list->data[list->free].next;

    list->data[elem_index].value = elem;
    list->data[elem_index].prev = index;
    list->data[elem_index].next = list->data[index].next;

    list->data[list->data[index].next].prev = elem_index;
    list->data[index].next = elem_index;

    if (list->tail == index) {
        list->tail = elem_index;
    }
    list->size++;

    if(list->free == 0) {
        int er_cd = list_resize(list, list->max_size * 2 + 1, VERIFY_CONTEXT);
        if (er_cd) {
            assert(er_cd == 0);
            return 0;
        }
    }

    list_verify(list, VERIFY_CONTEXT);
    return elem_index;
}

int list_in_prev(List_t *list, int index, element_t elem) {
    list_verify(list, VERIFY_CONTEXT);
    if (!index_correct(list, index, VERIFY_CONTEXT)) {
        return 0;
    }

    int elem_index = list->free;
    list->free = list->data[list->free].next;

    list->data[elem_index].value = elem;
    list->data[elem_index].prev = list->data[index].prev;
    list->data[elem_index].next = index;

    list->data[list->data[index].prev].next = elem_index;
    list->data[index].prev = elem_index;

    if(list->head == index) {
        list->head = elem_index;
    }
    list->size++;

    if(list->free == 0) {
        int er_cd = list_resize(list, list->max_size * 2 + 1, VERIFY_CONTEXT);
        if (er_cd) {
            assert(er_cd == 0);
            return 0;
        }
    }

    list_verify(list, VERIFY_CONTEXT);
    return elem_index;
}

element_t list_remove(List_t *list, int index, int* error_code /*= nullptr*/) {
    list_verify(list, VERIFY_CONTEXT);
    if(!index_correct(list, index, VERIFY_CONTEXT)) {
        return POISON;
    }
    if (!list_underflow_check(list, error_code)) {
        return POISON;
    }

    int ret_val = list->data[index].value;

    if(list->head == index) {
        list->head = list->data[index].next;
    }
    if (list->tail == index) {
        list->tail = list->data[index].prev;
    }
    list->data[list->data[index].next].prev = list->data[index].prev;
    list->data[list->data[index].prev].next = list->data[index].next;

    list->data[index].next = list->free;
    list->data[index].prev = FREE_POINTER;
    list->data[index].value = POISON;

    list->free = index;
    list->size--;

    list_verify(list, VERIFY_CONTEXT);
    return ret_val;
}

int list_seek(List_t *list, int num) {
    list_verify(list, VERIFY_CONTEXT);
    if (num > list->size || num <= 0) {
        return 0;
    }

    int elem_index = list->head;
    for (int i = 1; i < num; ++i) {
        elem_index = list->data[elem_index].next;
    }

    list_verify(list, VERIFY_CONTEXT);
    return elem_index;
}

int list_get_num(List_t *list, int index) {
    list_verify(list, VERIFY_CONTEXT);
    if (!index_correct(list, index, VERIFY_CONTEXT)) {
        return 0;
    }

    int num = 0;
    while (index != 0) {
        num++;
        index = list->data[index].prev;
    }

    list_verify(list, VERIFY_CONTEXT);
    return num;
}

#endif //LIST_INDEX_LIST_T_H
