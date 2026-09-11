#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../array_list_t.h"
#include "../../array_list_t_accesability/array_list_t_private_shared.h"

arr_status check_unsafe(array_list_t* arr, char* operation_name){
    arr_status res = ARR_OK;
    if(arr == NULL) res = ARR_IS_NULL;
    if(arr->type != ARR_UNSAFE) res = ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED;
    if(arr->length < 0) res = ARR_LENGTH_IS_CORRUPTED;
    arr_handle_internal_operation_status(res, operation_name);
    return res;
}

arr_status check_unsafe_without_type(array_list_t* arr, char* operation_name){
    arr_status res = ARR_OK;
    if(arr == NULL) res = ARR_IS_NULL;
    if(arr->length < 0) res = ARR_LENGTH_IS_CORRUPTED;
    arr_handle_internal_operation_status(res, operation_name);
    return res;
}

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
    arr_status st;
    if ((st = check_unsafe(arr, "Adding value from to array")) != ARR_OK) return st;

    memcpy(arr->values + arr->length * arr->size_of_one_element, value, arr->size_of_one_element);
    arr->length++;
    return ARR_OK;
}

arr_status arr_unsafe_set(array_list_t* arr, void* value, int index) {
    arr_status st;
    if ((st = check_unsafe(arr, "Setting value to unsafe array")) != ARR_OK) return st;
    memcpy(arr->values + index * arr->size_of_one_element, value, arr->size_of_one_element);
    return ARR_OK;
}

void* arr_unsafe_get(array_list_t* arr, int index) {
    arr_status st;
    if ((st = check_unsafe(arr, "Getting value from unsafe array")) != ARR_OK) return NULL;
    return arr_get_address_in_values(arr, index);
}

arr_status arr_unsafe_delete(array_list_t* arr, int index) {
    arr_status st;
    if ((st = check_unsafe(arr, "Deleting value from unsafe array")) != ARR_OK) return st;
    memset(arr->values + arr->length * arr->size_of_one_element, 0, arr->size_of_one_element);
    return ARR_OK;
}

int* arr_unsafe_length_ptr(array_list_t* arr){
    if (check_unsafe(arr, "Getting length from unsafe array") != ARR_OK) return NULL;
    return &arr->length;
}

int* arr_unsafe_capacity_ptr(array_list_t* arr){
    if (check_unsafe(arr, "Getting capacity from unsafe array") != ARR_OK) return NULL;
    return &arr->capacity;
}

int* arr_unsafe_starting_capacity_ptr(array_list_t* arr){
    if (check_unsafe(arr, "Getting capacity from unsafe array") != ARR_OK) return NULL;
    return &arr->starting_capacity;
}

int* arr_unsafe_element_size_ptr(array_list_t* arr){
    if (check_unsafe(arr, "Getting element size from unsafe array") != ARR_OK) return NULL;
    return &arr->size_of_one_element;
}

void* arr_unsafe_values_ptr(array_list_t* arr){
    if (check_unsafe(arr, "Getting values from unsafe array") != ARR_OK) return NULL;
    return arr->values;
}

ARR_TYPE* arr_unsafe_type_ptr(array_list_t* arr){
    if (check_unsafe(arr, "Getting type from unsafe array") != ARR_OK) return NULL ;
    return &arr->type;
}

char* arr_unsafe_custom_type_ptr(array_list_t* arr){
    if (check_unsafe(arr, "Getting custom type from unsafe array") != ARR_OK) return NULL;
    return arr->custom_type;
}

bool* arr_unsafe_shrink_on_tail_nulls_ptr(array_list_t* arr){
    if (check_unsafe(arr, "Getting shrink-on-tail-nulls flag from unsafe array") != ARR_OK) return NULL;
    return &arr->is_auto_shrink_on_tailing_nulls_enabled;
}

bool* arr_unsafe_shrink_on_inner_nulls_ptr(array_list_t* arr){
    if (check_unsafe(arr, "Getting shrink-on-inner-nulls flag from unsafe array") != ARR_OK) return NULL;
    return &arr->is_auto_shrink_on_inner_nulls_enabled;
}

bool* arr_unsafe_presence_mask_ptr(array_list_t* arr){
    if (check_unsafe(arr, "Getting presence mask from unsafe array") != ARR_OK) return NULL;
    return arr->bit_mask_of_presence;
}

arr_status arr_cast_to_unsafe(array_list_t* arr){
    arr_status st;
    if ((st = check_unsafe_without_type(arr, "Casting array to unsafe")) != ARR_OK) return st;
    arr->type = ARR_UNSAFE;
    return ARR_OK;
};