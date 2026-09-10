
#ifndef ARRAY_LIST_T_CUSTOM_H
#define ARRAY_LIST_T_CUSTOM_H

#include "../../array_list_t.h"



array_list_t* arr_create_custom(char* generic, size_t size);
array_list_t* arr_create_custom_greedy(char* generic, size_t size, int basic_capacity);
array_list_t* arr_create_from_customs(void* values[], int len, char* generic_name,
                                      size_t element_size);
arr_value arr_get_custom(array_list_t* arr, int index);
arr_value using_custom(void* value, char* name, size_t size);
arr_status arr_custom_unregister_type(char* type);
arr_status arr_custom_register_type(char* type);
arr_status arr_custom_provide_print(char* type, void(print)(const void* b));
arr_status arr_custom_print(array_list_t* arr);
arr_status arr_custom_provide_equals(char* type, int(comp)(const void* arr_v1, const void* arr_v2));
int arr_custom_equals(array_list_t* arr1, array_list_t* arr2);
arr_status arr_custom_add(array_list_t* arr, arr_value arr_v);
arr_status arr_custom_set(array_list_t* arr, arr_value arr_v, int index);
void free_using_container(arr_value av);
#endif //ARRAY_LIST_T_CUSTOM_H