#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array_list_t.h"
#include "array_list_t_private.h"
#include "../string_builder_t/string_builder_t.h"

#define MAX_TYPES 64
char* registered_types[64];
int type_register_counter = 0;

arr_status arr_verify_custom_type(array_list_t* arr, arr_value arr_v, int aftermalloc) {
    arr_status st = arr_check_for_errors(arr, aftermalloc);
    if (st != ARR_OK) return st;
    if (arr_v.value == NULL) return ARR_VALUE_IS_NULL;
    if (arr->type != ARR_CUSTOM) return ARR_INCONSISTENT_TYPE_PROVIDED;
    if (arr->type == ARR_CUSTOM && (arr->custom_type == "" || arr->custom_type == NULL))
        return ARR_CUSTOM_BUT_TYPE_NOT_SPECIFIED;
    if (arr->type != arr_v.type || strcmp(arr->custom_type, arr_v.custom_type) == 1)
        return ARR_INCONSISTENT_TYPE_PROVIDED;
    return ARR_OK;
}

array_list_t* arr_create_custom(char* generic_name, size_t element_size) {
    array_list_t* arr = arr_allocate(ARR_CUSTOM, arr_basic_capacity, element_size);
    arr_status status = arr_check_for_errors(arr, after_malloc);
    arr_handle_status(status);
    if (status != ARR_OK) {
        return NULL;
    }
    arr->custom_type = strdup(generic_name);
    return arr;
}

int is_type_registered(char* type) {
    for (int i = 0; i < MAX_TYPES; i++) {
        if (registered_types[i] != NULL) {
            if (strcmp(registered_types[i], type) == 0){
                return 1;
           }
    }
    }
    return 0;
}

arr_status arr_custom_register_type(char* type) {
    if (type_register_counter + 1 == MAX_TYPES) return ARR_CUSTOM_REGISTER_REACHED_MAX_AMOUNT;
    if (is_type_registered(type) == 1) return ARR_CUSTOM_TYPE_IS_ALREADY_REGISTERED;
    registered_types[type_register_counter] = strdup(type);
    type_register_counter++;
    return ARR_OK;
};

arr_status arr_custom_unregister_type(char* type) {
    if (is_type_registered(type) == 0) return ARR_CUSTOM_TYPE_IS_NOT_REGISTERED;
    for (int i = 0; i < MAX_TYPES; i++) {
        if (strcmp(registered_types[i], type) == 0) {
            free(registered_types[i]);
        };
    }
    return ARR_OK;
};

arr_value using_custom(void* value, char* name, size_t size) {
    arr_value arr_v;
    arr_v.type = ARR_CUSTOM;
    arr_v.custom_type = strdup(name);
    arr_v.value = malloc(size);
    arr_v.value = value;
    return arr_v;
}

arr_status arr_custom_add(array_list_t* arr, arr_value arr_v) {
    arr_status st1 = arr_verify_custom_type(arr, arr_v, not_after_malloc);
    if (st1 != ARR_OK) return st1;
    if (is_type_registered(arr_v.custom_type) == 0) return ARR_CUSTOM_TYPE_IS_NOT_REGISTERED;
    memcpy(arr->values + arr->size_of_one_element*arr->length, arr_v.value, arr->size_of_one_element);
    free_using_container(arr_v);
    arr->length++;
    return ARR_OK;
}

arr_status print_custom(array_list_t* arr, void (print)(const void* b)){
    if (is_type_registered(arr->custom_type) == 0) return ARR_CUSTOM_TYPE_IS_NOT_REGISTERED;
    for(int i = 0; i<arr->length; i++){
        void* value = arr->values + i * arr->size_of_one_element;
        print(value);
    }
    return ARR_OK;
}
