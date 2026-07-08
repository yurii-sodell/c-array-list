typedef struct array_list_t array_list_t;
#ifndef ARRAY_LIST_T_H
#define ARRAY_LIST_T_H

#define arr_basic_capacity 1024
#define types_supported 5

typedef enum {
    ARR_INT, //1
    ARR_STRINGS, //2
    ARR_CHAR, //3
    ARR_DOUBLE, //4
    ARR_FLOAT, //5
    ARR_CUSTOM
} ARR_TYPE;

typedef struct arr_value{
    void* value;
    ARR_TYPE type;
    char* custom_type;
} arr_value;

typedef enum {
    ARR_OK,
    ARR_IS_NULL,
    ARR_OUT_OF_BOUNDS,
    ARR_MEMORY_FAULT,
    ARR_LENGTH_IS_CORRUPTED,
    ARR_INCONSISTENT_TYPE_PROVIDED,
    ARR_CUSTOM_BUT_TYPE_NOT_SPECIFIED,
    ARR_CUSTOM_TYPE_IS_NOT_REGISTERED,
    ARR_CUSTOM_REGISTER_REACHED_MAX_AMOUNT,
    ARR_CUSTOM_TYPE_IS_ALREADY_REGISTERED,
    ARR_VALUE_IS_NULL
} arr_status;

void arr_init();

array_list_t* arr_create (ARR_TYPE DataType);
array_list_t* arr_create_custom(char* generic, size_t size);

int arr_equals(array_list_t* arr1, array_list_t* arr2);
arr_value arr_get_int(array_list_t* arr, int index);
arr_value arr_get_char(array_list_t* arr, int index);
int arr_find_index(array_list_t*, arr_value);

arr_status arr_add(array_list_t* arr, arr_value value);
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
arr_value using_custom(void* value, char* name, size_t size);
arr_status arr_custom_unregister_type(char* type);
arr_status arr_custom_register_type(char* type) ;

arr_status print_custom(array_list_t* arr, void (print)(const void* b));

#endif