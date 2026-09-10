#include <stdbool.h>

#ifndef ARRAY_LIST_T_SHARED_H
#define ARRAY_LIST_T_SHARED_H

#include "../array_list_t.h"
#include "../array_list_t_lib_messages_handler/array_list_t_lib_messages_handler.h"

#define after_malloc 1
#define not_after_malloc 0
#define SAFE_FREE(p) \
    do {             \
        free(p);     \
        p = NULL;    \
    } while (0);

typedef struct array_list_t {
    int length;
    int capacity;
    int starting_capacity;
    int size_of_one_element;
    void* values;
    ARR_TYPE type;
    char* custom_type;
    bool is_auto_shrink_enabled;
    bool* bit_mask_of_presence;
} array_list_t;

// used in custom and base, defined in base
array_list_t* arr_allocate(ARR_TYPE DataType, int capacity, int element_size);
arr_status check_memory_allocation(array_list_t* arr);
arr_status arr_verify_array(array_list_t* arr, int aftermalloc);
void* arr_get_address_in_values(array_list_t* arr, int index);
int is_slot_empty(array_list_t* arr, int id);

// for unsafe, defined in base
arr_status reallocate_array_value(array_list_t* arr, int new_capacity);

#endif //ARRAY_LIST_T_SHARED_H