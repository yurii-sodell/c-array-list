#include <stdbool.h>
#include <stddef.h>

typedef struct array_list_t array_list_t;
#ifndef ARRAY_LIST_T_H
#define ARRAY_LIST_T_H

#define ARR_BASIC_CAPACITY 1024
#define types_supported 9

#define has(status) arr_handle_status(status)

typedef enum {
    ARR_INT,          // 0
    ARR_LONG,         // 1
    ARR_LONG_LONG,    // 2
    ARR_LONG_DOUBLE,  // 3
    ARR_SHORT,        // 4
    ARR_STRING,       // 5
    ARR_CHAR,         // 6
    ARR_DOUBLE,       // 7
    ARR_FLOAT,        // 8
    ARR_VARIANT,      // 9
    ARR_CUSTOM,       // 10
    ARR_NULL_VALUE,   // 11
    ARR_UNSAFE        // 12
} ARR_TYPE;

typedef struct arr_value {
    void* custom_value;

    union basic_value {
        int integer;
        char character;
        char* string;
        double double_v;
        float float_v;
        long long_v;
        long long long_long_v;
        long double long_double_v;
        short short_v;
    } basic_value;

    ARR_TYPE type;
    char* custom_type;
    size_t size;
} arr_value;

typedef enum {
    ARR_OK = 1,
    ARR_IS_NULL = -1,
    ARR_OUT_OF_BOUNDS = -2,
    ARR_MEMORY_FAULT = -3,
    ARR_LENGTH_IS_CORRUPTED = -4,
    ARR_INCONSISTENT_TYPE_PROVIDED = -5,
    ARR_CUSTOM_BUT_TYPE_NOT_SPECIFIED = -6,
    ARR_CUSTOM_TYPE_IS_NOT_REGISTERED = -7,
    ARR_CUSTOM_REGISTER_REACHED_MAX_AMOUNT = -8,
    ARR_CUSTOM_TYPE_IS_ALREADY_REGISTERED = -9,
    ARR_PRINT_IS_NOT_REGISTERED_FOR_THAT_TYPE = -10,
    ARR_EQUALS_IS_NOT_REGISTERED_FOR_THAT_TYPE = -11,
    ARR_VALUE_IS_NULL = -12,
    ARR_SIZES_OF_ARRAY_ELEMENT_AND_PROVIDED_ARE_DEFER = -13,
    ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED = -14,
    ARR_SAFE_OPERATION_ON_UNSAFE_TYPE_CALLED = -15
} arr_status;

void arr_lib_init();

arr_status arr_sort(array_list_t* arr, int(sorting_algorithm)(const void* a, const void* b));
int arr_get_size_of_element(array_list_t* arr);
ARR_TYPE arr_get_type(array_list_t* arr);
int arr_get_mem_capacity(array_list_t* arr);
int arr_get_elements_capacity(array_list_t* arr);
int arr_get_length(array_list_t* arr);
arr_status arr_reverse(array_list_t* arr);
arr_status arr_enable_auto_trim_on_trailing_nulls(array_list_t* arr);
arr_status arr_disable_auto_trim_on_trailing_nulls(array_list_t* arr);

arr_status arr_enable_auto_trim_on_inner_nulls(array_list_t* arr);
arr_status arr_disable_auto_trim_on_inner_nulls(array_list_t* arr);

arr_status arr_clear(array_list_t* arr);
void arr_handle_status(arr_status st);

#include "array_list_t_types/base/array_list_t_base.h"
#include "array_list_t_types/custom/array_list_t_custom.h"
#include "array_list_t_types/unsafe/array_list_t_unsafe.h"

#endif  // ARRAY_LIST_T_H