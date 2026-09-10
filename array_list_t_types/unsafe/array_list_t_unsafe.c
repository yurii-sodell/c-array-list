#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../array_list_t.h"
#include "../../array_list_t_accesability/array_list_t_shared.h"

array_list_t* arr_create_unsafe(int len, int element_size) {
    array_list_t* arr = arr_allocate(ARR_UNSAFE, len, element_size);
    char* x = "Array create unsafe";
    if (arr == NULL) {
        arr_handle_internal_operation_status(ARR_IS_NULL, x);
        return NULL;
    }
    return arr;
};

array_list_t* arr_unsafe_realloc(array_list_t* arr, int new_len) {
    if (arr == NULL) return NULL;
    if (arr->length < new_len) {
        arr_handle_error_message_without_status(
            "New array len is smaller then existing one. Try to delete elements at the end, before "
            "reallocating array");
        return arr;
    }
    arr_status result = reallocate_array_value(arr, new_len);
    if (result != ARR_OK)
        arr_handle_internal_operation_status(ARR_MEMORY_FAULT, "Unsafe array reallocation");
    return arr;
}

arr_status arr_unsafe_add(array_list_t* arr, void* value) {
    char* x = "Adding value from to array";
    if (arr->type != ARR_UNSAFE) {
        arr_handle_internal_operation_status(ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED, x);
        return ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED;
    }
    memcpy(arr->values + arr->length * arr->size_of_one_element, value, arr->size_of_one_element);
    arr->length++;
    return ARR_OK;
}

arr_status arr_unsafe_set(array_list_t* arr, void* value, int index) {
    char* x = "Setting value to unsafe array";
    if (arr->type != ARR_UNSAFE) {
        arr_handle_internal_operation_status(ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED, x);
        return ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED;
    }
    memcpy(arr->values + index * arr->size_of_one_element, value, arr->size_of_one_element);
    return ARR_OK;
}

void* arr_unsafe_get(array_list_t* arr, int index) {
    char* x = "Getting value from unsafe array";
    if (arr->type != ARR_UNSAFE) {
        arr_handle_internal_operation_status(ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED, x);
        return NULL;
    }
    return arr_get_address_in_values(arr, index);
}

arr_status arr_unsafe_delete(array_list_t* arr, int index) {
    char* x = "Deleting value from unsafe array";
    if (arr->type != ARR_UNSAFE) {
        arr_handle_internal_operation_status(ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED, x);
        return ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED;
    }
    memset(arr->values + arr->length * arr->size_of_one_element, 0, arr->size_of_one_element);
    return ARR_OK;
}
