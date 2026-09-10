#ifndef ARRAY_LIST_T_UNSAFE_H
#define ARRAY_LIST_T_UNSAFE_H

#include "../../array_list_t.h"

array_list_t* arr_create_unsafe(int len, int element_size);
array_list_t* arr_unsafe_realloc(array_list_t* arr, int new_len);
arr_status arr_unsafe_add(array_list_t* arr, void* value);
arr_status arr_unsafe_set(array_list_t* arr, void* value, int index);
void* arr_unsafe_get(array_list_t* arr, int index);
arr_status arr_unsafe_delete(array_list_t* arr, int index);
#endif //ARRAY_LIST_T_UNSAFE_H