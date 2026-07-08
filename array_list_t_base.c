#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array_list_t.h"
#include "array_list_t_private.h"

/*============================= BASIC CHECK BEFORE ANY OPERATION =============================*/
arr_status arr_check_for_errors(array_list_t* arr, int aftermalloc) {
    arr_status status = aftermalloc == 1 ? ARR_MEMORY_FAULT : ARR_IS_NULL;
    if (arr == NULL) return status;
    if (arr->values == NULL) return status;
    if (arr->length <= -1) return ARR_LENGTH_IS_CORRUPTED;
    return ARR_OK;
};

void arr_handle_status(arr_status st) {
    if (st == ARR_OK) return;
    char* message = "";
    switch (st) {
        case ARR_OUT_OF_BOUNDS:
            message = "Array out of bounds";
            break;
        case ARR_IS_NULL:
            message = "Array is NULL";
            break;
        case ARR_MEMORY_FAULT:
            message = "Memory allocation went wrong";
            break;
        case ARR_LENGTH_IS_CORRUPTED:
            message = "Length of array is corrupted. Something went terribly wrong!";
            break;
        case ARR_INCONSISTENT_TYPE_PROVIDED:
            message = "Type of array and type of provided value are defer.";
            break;
        case ARR_CUSTOM_BUT_TYPE_NOT_SPECIFIED:
            message = "Array is custom, but type is not specified.";
            break;
        case ARR_CUSTOM_TYPE_IS_NOT_REGISTERED:
            message = "Custom type is not registered.";
            break;
        case ARR_CUSTOM_REGISTER_REACHED_MAX_AMOUNT:
            message = "Custom type register reached max amount.";
            break;
        case ARR_CUSTOM_TYPE_IS_ALREADY_REGISTERED:
            message = "Custom type is already registered.";
            break;
        case ARR_VALUE_IS_NULL:
            message = "Provided value is NULL";
            break;
        default:
            message = "Unknown error has occured in array.";
            break;
    }
    printf("\n%s", message);
}
/* ============================= CREATION OF ARRAY =============================*/
/* =================== PRIVATE FOR USER. SHARED WITH CUSTOM. ===================*/
array_list_t* arr_allocate(ARR_TYPE DataType, int capacity, int element_size) {
    array_list_t* arr = malloc(sizeof(array_list_t));
    if (arr == NULL) return NULL;
    arr->capacity = capacity;
    arr->length = 0;
    arr->type = DataType;
    arr->size_of_one_element = element_size;
    arr->values = malloc(capacity);
    if (arr->values == NULL) return NULL;
    return arr;
}
/* ============================================================================*/

/* ============================= CREATION OF ARRAY UTILITY CONTAINERS ====================== */
arr_value using_int(int i) {
    arr_value arr_v;
    arr_v.type = ARR_INT;
    arr_v.value = malloc(sizeof(i));
    *(int*)arr_v.value = i;
    return arr_v;
};
arr_value using_char(char c) {
    arr_value arr_v;
    arr_v.type = ARR_CHAR;
    arr_v.value = malloc(sizeof(c));
    *(char*)arr_v.value = c;
    return arr_v;
};
arr_value using_string(char* s) {
    arr_value arr_v;
    arr_v.type = ARR_STRINGS;
    arr_v.value = malloc(sizeof(s));
    *(char**)arr_v.value = s;

    return arr_v;
};
arr_value using_double(double d) {
    arr_value arr_v;
    arr_v.type = ARR_DOUBLE;
    arr_v.value = malloc(sizeof(d));
    *(double*)arr_v.value = d;
    return arr_v;
};
arr_value using_float(float f) {
    arr_value arr_v;
    arr_v.type = ARR_FLOAT;
    arr_v.value = malloc(sizeof(f));
    *(float*)arr_v.value = f;
    return arr_v;
};

/*===============================================================================================*/

/* =================== PRIVATE FOR USER. SHARED WITH CUSTOM. ===================*/
arr_status check_memory_allocation(array_list_t* arr) {
    int len = arr->length + 1;
    int expected_capcaity = len * arr->size_of_one_element * 2;
    while (arr->capacity < expected_capcaity) {
        int new_cap = arr->capacity / 2 + arr->capacity;
        void* tmp = realloc(arr->values, new_cap);
        if (tmp != NULL) return ARR_MEMORY_FAULT;
        arr->values = tmp;
        arr->capacity = new_cap;
    }
    return ARR_OK;
}

/*============================================================================*/

void free_using_container(arr_value av) { free(av.value); }

int map_sizes[types_supported];
void arr_init_map_sizes() {
    map_sizes[ARR_INT] = sizeof(int);
    map_sizes[ARR_CHAR] = sizeof(char);
    map_sizes[ARR_STRINGS] = sizeof(char*);
    map_sizes[ARR_DOUBLE] = sizeof(double);
    map_sizes[ARR_FLOAT] = sizeof(float);
}

/* =========== PUBLIC ===========*/
array_list_t* arr_create(ARR_TYPE DataType) {
    if (DataType == ARR_CUSTOM) {
        fprintf(
            stderr,
            "Wrong arr type is provided. For custom types use arr_create_custom(). NULL returned");
        return NULL;
    }
    array_list_t* arr = arr_allocate(DataType, arr_basic_capacity, map_sizes[DataType]);
    if (arr == NULL) arr_handle_status(ARR_MEMORY_FAULT);
    return arr;
}

/*============================= PRINTIING OF ARRAY =============================*/
/* ================ PRIVATE ================*/
void itterate_string(array_list_t* arr) {
    char** target = (char**)arr->values;
    for (size_t i = 0; i < arr->length; i++) {
        printf("\n%s", target[i]);
    }
}
void itterate_int(array_list_t* arr) {
    int* list = (int*)arr->values;
    for (size_t i = 0; i < arr->length; i++) {
        printf("\n%u", list[i]);
    }
}
void itterate_char(array_list_t* arr) {
    char* list = (char*)arr->values;
    for (size_t i = 0; i < arr->length; i++) {
        printf("\n%c", list[i]);
    }
}
void itterate_double(array_list_t* arr) {
    double* list = (double*)arr->values;
    for (size_t i = 0; i < arr->length; i++) {
        printf("\n%f", list[i]);
    }
}
void itterate_float(array_list_t* arr) {
    float* list = (float*)arr->values;
    for (size_t i = 0; i < arr->length; i++) {
        printf("\n%f", list[i]);
    }
}
void (*arr_print_map[types_supported])(array_list_t* arr);
arr_status arr_init_map_prints() {
    arr_print_map[ARR_INT] = itterate_int;
    arr_print_map[ARR_CHAR] = itterate_char;
    arr_print_map[ARR_STRINGS] = itterate_string;
    arr_print_map[ARR_DOUBLE] = itterate_double;
    arr_print_map[ARR_FLOAT] = itterate_float;
}
/* ================ PUBLIC ================*/
arr_status arr_print(array_list_t* arr) { arr_print_map[arr->type](arr); }

/*============================= ADDING VALUE TO ARRAY =============================*/
/* ================== PRIVATE ================== */
arr_status add_int(array_list_t* arr, arr_value arr_v) {
    int* target_1 = (int*)arr->values;
    ((int*)target_1)[arr->length] = *(int*)arr_v.value;
}
arr_status add_char(array_list_t* arr, arr_value arr_v) {
    char* target_2 = (char*)arr->values;
    ((char*)target_2)[arr->length] = *(char*)arr_v.value;
}
arr_status add_string(array_list_t* arr, arr_value arr_v) {
    char** target_3 = (char**)arr->values;
    ((char**)target_3)[arr->length] = *(char**)arr_v.value;
}
arr_status add_double(array_list_t* arr, arr_value arr_v) {
    double* target_4 = (double*)arr->values;
    ((double*)target_4)[arr->length] = *(double*)arr_v.value;
}
arr_status add_float(array_list_t* arr, arr_value arr_v) {
    float* target_5 = (float*)arr->values;
    ((float*)target_5)[arr->length] = *(float*)arr_v.value;
}

arr_status (*arr_add_map[types_supported])(array_list_t* arr, arr_value);
arr_status arr_init_map_adds() {
    arr_add_map[ARR_INT] = add_int;
    arr_add_map[ARR_CHAR] = add_char;
    arr_add_map[ARR_STRINGS] = add_string;
    arr_add_map[ARR_DOUBLE] = add_double;
    arr_add_map[ARR_FLOAT] = add_float;
}

/* ================== PUBLIC ================== */
arr_status arr_add(array_list_t* arr, arr_value arr_v) {
    ARR_TYPE type = arr_v.type;
    if (arr->type != type || arr->type == ARR_CUSTOM) return ARR_INCONSISTENT_TYPE_PROVIDED;
    arr_status status = check_memory_allocation(arr);
    if (status != ARR_OK) return status;
    arr_add_map[arr->type](arr, arr_v);
    arr->length++;
    free_using_container(arr_v);
    return ARR_OK;
}

/*============================= CHECKING EQUALITY OF ARRAYS =============================*/
/* ================= PRIVATE ================= */

int arr_equals_int(array_list_t* arr1, array_list_t* arr2) {
    int len = arr1->length;
    for (size_t i = 0; i < len; i++) {
        int v1 = ((int*)arr1->values)[i];
        int v2 = ((int*)arr2->values)[i];
        if (v1 != v2) return 0;
    }
    return 1;
}

int arr_equals_char(array_list_t* arr1, array_list_t* arr2) {
    int len = arr1->length;
    for (size_t i = 0; i < len; i++) {
        char v1 = ((char*)arr1->values)[i];
        char v2 = ((char*)arr2->values)[i];
        if (v1 != v2) return 0;
    }
    return 1;
}

int arr_equals_string(array_list_t* arr1, array_list_t* arr2) {
    int len = arr1->length;
    for (size_t i = 0; i < len; i++) {
        char* v1 = ((char**)arr1->values)[i];
        char* v2 = ((char**)arr2->values)[i];
        if (strcmp(v1, v2) != 0) return 0;
    }
    return 1;
}

int arr_equals_double(array_list_t* arr1, array_list_t* arr2) {
    double len = arr1->length;
    for (size_t i = 0; i < len; i++) {
        double v1 = ((double*)arr1->values)[i];
        double v2 = ((double*)arr2->values)[i];
        if (v1 != v2) return 0;
    }
    return 1;
}

int arr_equals_float(array_list_t* arr1, array_list_t* arr2) {
    int len = arr1->length;
    for (size_t i = 0; i < len; i++) {
        int v1 = ((int*)arr1->values)[i];
        int v2 = ((int*)arr2->values)[i];
        if (v1 != v2) return 0;
    }
    return 1;
}

int (*arr_equals_map[types_supported])(array_list_t* arr1, array_list_t* arr2);
int arr_init_map_equals() {
    arr_equals_map[ARR_INT] = arr_equals_int;
    arr_equals_map[ARR_CHAR] = arr_equals_char;
    arr_equals_map[ARR_STRINGS] = arr_equals_string;
    arr_equals_map[ARR_DOUBLE] = arr_equals_double;
    arr_equals_map[ARR_FLOAT] = arr_equals_float;
}
/* ================ PUBLIC ================*/

int arr_equals(array_list_t* arr1, array_list_t* arr2) {
    ARR_TYPE type = arr1->type;
    if (arr1->length != arr2->length) return 0;
    if (type != arr2->type) return 0;
    if (type == ARR_CUSTOM && !strcmp(arr1->custom_type, arr2->custom_type)) return 0;
    return arr_equals_map[type](arr1, arr2);
}

/*============================= GETTING VALUE =============================*/
void* arr_get_address(array_list_t* arr, int index) {
    if (index < 0 || index >= arr->length) {
        arr_handle_status(ARR_OUT_OF_BOUNDS);
        return NULL;
    }
    return arr->values + index * arr->size_of_one_element;
}

arr_value arr_get_int(array_list_t* arr, int index) {
    arr_value val = {0};
    arr_status st = arr_check_for_errors(arr, not_after_malloc);
    arr_handle_status(st);
    if (st != ARR_OK) return val;
    if (arr->type != ARR_INT) {
        arr_handle_status(ARR_INCONSISTENT_TYPE_PROVIDED);
        return val;
    }
    void* addres = arr_get_address(arr, index);
    if (addres == NULL) return val;
    val = using_int(*(int*)addres);
    return val;
}

arr_value arr_get_char(array_list_t* arr, int index) {
    arr_value val = {0};
    arr_status st = arr_check_for_errors(arr, not_after_malloc);
    arr_handle_status(st);
    if (st != ARR_OK) return val;
    if (arr->type != ARR_CHAR) {
        arr_handle_status(ARR_INCONSISTENT_TYPE_PROVIDED);
        return val;
    }
    void* addres = arr_get_address(arr, index);
    if (addres == NULL) return val;
    val = using_char(*(char*)addres);
    return val;
}

int (*arr_equals_map[types_supported])(array_list_t* arr1, array_list_t* arr2);
int arr_init_map_get() {
    arr_equals_map[ARR_INT] = arr_equals_int;
    arr_equals_map[ARR_CHAR] = arr_equals_char;
    arr_equals_map[ARR_STRINGS] = arr_equals_string;
    arr_equals_map[ARR_DOUBLE] = arr_equals_double;
    arr_equals_map[ARR_FLOAT] = arr_equals_float;
}

/*=============================== SORTING ==================================*/
arr_status arr_sort(array_list_t* arr1, int(sorting_algorithm)(const void* a, const void* b)) {
        arr_status st = arr_check_for_errors(arr1, not_after_malloc);
        if (st != ARR_OK) st;
        qsort(arr1->values, arr1->length, arr1->size_of_one_element, sorting_algorithm);
};

void arr_init() {
    arr_init_map_prints();
    arr_init_map_adds();
    arr_init_map_sizes();
    arr_init_map_equals();
}