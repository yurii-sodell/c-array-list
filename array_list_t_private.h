#include "array_list_t.h"
#include <stdbool.h>
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
    int size_of_one_element;
    void* values;
    ARR_TYPE type;
    char* custom_type;
    bool is_auto_shrink_enabled;
} array_list_t;

arr_status check_memory_allocation(array_list_t* arr);
array_list_t* arr_allocate(ARR_TYPE DataType, int capacity, int element_size);
arr_status arr_verify_array(array_list_t* arr, int aftermalloc);
void free_using_container(arr_value av);
void arr_handle_internal_operation_status(arr_status st, char* source);
void* arr_get_address_in_values(array_list_t* arr, int index);
int is_slot_empty(array_list_t* arr, int id);