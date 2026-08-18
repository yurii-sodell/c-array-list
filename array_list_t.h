#include <stddef.h>

typedef struct array_list_t array_list_t;
#ifndef ARRAY_LIST_T_H
#define ARRAY_LIST_T_H

#define arr_basic_capacity 1024
#define types_supported 5

#define has(status) arr_handle_status(status)

typedef enum {
    ARR_INT,      // 1
    ARR_STRINGS,  // 2
    ARR_CHAR,     // 3
    ARR_DOUBLE,   // 4
    ARR_FLOAT,    // 5
    ARR_CUSTOM    // 6
} ARR_TYPE;

typedef struct arr_value {
    void* value;
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
    ARR_SIZES_OF_ARRAY_ELEMENT_AND_PROVIDED_ARE_DEFER = -13
} arr_status;

void arr_init();

array_list_t* arr_create(ARR_TYPE DataType);
array_list_t* arr_create_greedy(ARR_TYPE DataType, int basic_capacity);

array_list_t* arr_create_custom(char* generic, size_t size);
array_list_t* arr_create_custom_greedy(char* generic, size_t size, int basic_capacity);

array_list_t* arr_create_from_ints(int ints[], int len);
array_list_t* arr_create_from_chars(char chars[], int len);
array_list_t* arr_create_from_strings(char* strings[], int len);
array_list_t* arr_create_from_floats(float floats[], int len);
array_list_t* arr_create_from_doubles(double doubles[], int len);

array_list_t* arr_create_from_customs(void* values[], int len, char* generic_name,
                                      size_t element_size);

int arr_equals(array_list_t* arr1, array_list_t* arr2);
arr_value arr_get_int(array_list_t* arr, int index);
arr_value arr_get_char(array_list_t* arr, int index);
arr_value arr_get_string(array_list_t* arr, int index);
arr_value arr_get_float(array_list_t* arr, int index);
arr_value arr_get_double(array_list_t* arr, int index);
arr_value arr_get_custom(array_list_t* arr, int index);

arr_status arr_add(array_list_t* arr, arr_value value);
arr_status arr_set(array_list_t* arr, arr_value arr_v, int index);

arr_status arr_print(array_list_t* arr);
arr_status arr_sort(array_list_t* arr, int(sorting_algorithm)(const void* a, const void* b));

arr_value using_int(int i);
arr_value using_char(char c);
arr_value using_string(char* s);
arr_value using_double(double d);
arr_value using_float(float f);

void free_using_container(arr_value av);
void arr_handle_status(arr_status st);

arr_status arr_custom_add(array_list_t* arr, arr_value arr_v);
arr_status arr_custom_set(array_list_t* arr, arr_value arr_v, int index);

arr_status arr_delete(array_list_t* arr, size_t index);

arr_value using_custom(void* value, char* name, size_t size);
arr_status arr_custom_unregister_type(char* type);
arr_status arr_custom_register_type(char* type);

arr_status arr_custom_provide_print(char* type, void(print)(const void* b));
arr_status arr_custom_print(array_list_t* arr);

arr_status arr_custom_provide_equals(char* type, int(comp)(const void* arr_v1, const void* arr_v2));
int arr_custom_equals(array_list_t* arr1, array_list_t* arr2);
arr_status arr_free(array_list_t* arr);

int arr_get_size_of_element(array_list_t* arr);
ARR_TYPE arr_get_type(array_list_t* arr);
int arr_get_mem_capacity(array_list_t* arr);
int arr_get_elements_capacity(array_list_t* arr);
int arr_get_length(array_list_t* arr);

#endif //ARRAY_LIST_T_H