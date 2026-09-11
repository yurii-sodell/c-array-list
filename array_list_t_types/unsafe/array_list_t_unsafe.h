#ifndef ARRAY_LIST_T_UNSAFE_H
#define ARRAY_LIST_T_UNSAFE_H

#include "../../array_list_t.h"

array_list_t* arr_create_unsafe(int len, int element_size);
array_list_t* arr_unsafe_realloc(array_list_t* arr, int new_len);
arr_status arr_unsafe_add(array_list_t* arr, void* value);
arr_status arr_unsafe_set(array_list_t* arr, void* value, int index);
void* arr_unsafe_get(array_list_t* arr, int index);
arr_status arr_unsafe_delete(array_list_t* arr, int index);

int* arr_unsafe_length_ptr(array_list_t* arr);
int* arr_unsafe_capacity_ptr(array_list_t* arr);
int* arr_unsafe_starting_capacity_ptr(array_list_t* arr);
int* arr_unsafe_element_size_ptr(array_list_t* arr);
void* arr_unsafe_values_ptr(array_list_t* arr);
ARR_TYPE* arr_unsafe_type_ptr(array_list_t* arr);
char* arr_unsafe_custom_type_ptr(array_list_t* arr);
bool* arr_unsafe_shrink_on_tail_nulls_ptr(array_list_t* arr);
bool* arr_unsafe_shrink_on_inner_nulls_ptr(array_list_t* arr);
bool* arr_unsafe_presence_mask_ptr(array_list_t* arr);

arr_status arr_cast_to_unsafe(array_list_t* arr);

#endif  // ARRAY_LIST_T_UNSAFE_H