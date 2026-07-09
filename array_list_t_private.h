#include "array_list_t.h"
#define after_malloc 1
#define not_after_malloc 0

typedef struct array_list_t {
    int length;
    int capacity;
    int size_of_one_element;
    void* values;
    ARR_TYPE type;
    char* custom_type;
} array_list_t;

arr_status check_memory_allocation(array_list_t* arr);
array_list_t* arr_allocate(ARR_TYPE DataType, int capacity, int element_size);
arr_status arr_verify_array(array_list_t* arr, int aftermalloc);
void free_using_container(arr_value av);
void arr_handle_internal_operation_status(arr_status st, char* source);