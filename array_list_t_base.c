#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array_list_t.h"
#include "array_list_t_private.h"

/*============================= BASIC CHECK BEFORE ANY OPERATION =============================*/
arr_status arr_verify_array(array_list_t* arr, int aftermalloc) {
    arr_status status = aftermalloc == 1 ? ARR_MEMORY_FAULT : ARR_IS_NULL;
    if (arr == NULL) return status;
    if (arr->values == NULL) return status;
    if (arr->length <= -1) return ARR_LENGTH_IS_CORRUPTED;
    return ARR_OK;
};

void arr_handle_internal_operation_status(arr_status st, char* additional_information) {
    if (st == ARR_OK) return;
    char* message = "";
    switch (st) {
        case ARR_OUT_OF_BOUNDS:
            message = "Error: Array index out of bounds.";
            break;
        case ARR_IS_NULL:
            message = "Error: Array is NULL.";
            break;
        case ARR_MEMORY_FAULT:
            message = "Error: Memory allocation went wrong.";
            break;
        case ARR_LENGTH_IS_CORRUPTED:
            message = "Error: Length of array is corrupted. Something went terribly wrong!";
            break;
        case ARR_INCONSISTENT_TYPE_PROVIDED:
            message =
                "Error: Type of the array does not correspond to the type of the provided value.";
            break;
        case ARR_CUSTOM_BUT_TYPE_NOT_SPECIFIED:
            message = "Error: Array is custom, but type is not specified.";
            break;
        case ARR_CUSTOM_TYPE_IS_NOT_REGISTERED:
            message = "Error: Custom type is not registered.";
            break;
        case ARR_CUSTOM_REGISTER_REACHED_MAX_AMOUNT:
            message = "Error: Custom type register reached max amount.";
            break;
        case ARR_CUSTOM_TYPE_IS_ALREADY_REGISTERED:
            message = "Error: Custom type is already registered.";
            break;
        case ARR_VALUE_IS_NULL:
            message = "Error: Provided array value is NULL.";
            break;
        case ARR_PRINT_IS_NOT_REGISTERED_FOR_THAT_TYPE:
            message = "Error: print function is not registered for this type";
            break;
        case ARR_EQUALS_IS_NOT_REGISTERED_FOR_THAT_TYPE:
            message =
                "Error: comparing function (expected return values: -1, 0, 1) is not registered "
                "for this type";
            break;
        case ARR_SIZES_OF_ARRAY_ELEMENT_AND_PROVIDED_ARE_DEFER:
            message = "Error: sizes of provided element and size of one array element are defer";
            break;
        default:
            message = "Unknown error has occured in array.";
            break;
    }

    if (additional_information == NULL) {
        fprintf(stderr, "\n%s", message);
    } else {
        fprintf(stderr, "\n%s: %s", additional_information, message);
    }
}

void arr_handle_status(arr_status st) { arr_handle_internal_operation_status(st, NULL); }
/* ============================= CREATION OF ARRAY =============================*/
/* =================== PRIVATE FOR USER. SHARED WITH CUSTOM. ===================*/
array_list_t* arr_allocate(ARR_TYPE DataType, int capacity, int element_size) {
    array_list_t* arr = malloc(sizeof(array_list_t));
    if (arr == NULL) return NULL;
    arr->capacity = capacity;
    arr->length = 0;
    arr->type = DataType;
    arr->size_of_one_element = element_size;
    arr->values = calloc(capacity, element_size);
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
    int expected_capcaity = (arr->length + 10) * arr->size_of_one_element;
    while (arr->capacity < expected_capcaity) {
        int new_cap = arr->capacity / 2 + arr->capacity;

        void* tmp = realloc(arr->values, new_cap);

        if (tmp == NULL) return ARR_MEMORY_FAULT;
        arr->values = tmp;
        arr->capacity = new_cap;
    }

    return ARR_OK;
}

/*============================================================================*/

void free_using_container(arr_value av) {
    free(av.value);
    if (av.type == ARR_CUSTOM) free(av.custom_type);
}

int map_sizes[types_supported];
void arr_init_map_sizes() {
    map_sizes[ARR_INT] = sizeof(int);
    map_sizes[ARR_CHAR] = sizeof(char);
    map_sizes[ARR_STRINGS] = sizeof(char*);
    map_sizes[ARR_DOUBLE] = sizeof(double);
    map_sizes[ARR_FLOAT] = sizeof(float);
}

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

/* =========== PUBLIC ===========*/
array_list_t* arr_create(ARR_TYPE DataType) {
    if (DataType == ARR_CUSTOM) {
        fprintf(
            stderr,
            "Wrong arr type is provided. For custom types use arr_create_custom(). NULL returned");
        return NULL;
    }
    array_list_t* arr = arr_allocate(DataType, arr_basic_capacity, map_sizes[DataType]);
    if (arr == NULL) arr_handle_internal_operation_status(ARR_MEMORY_FAULT, "Array create");
    return arr;
}

array_list_t* arr_create_greedy(ARR_TYPE DataType, int basic_capacity){
    if (DataType == ARR_CUSTOM) {
        fprintf(
            stderr,
            "Wrong arr type is provided. For custom types use arr_create_custom(). NULL returned");
        return NULL;
    }
    array_list_t* arr = arr_allocate(DataType, basic_capacity, map_sizes[DataType]);
    if (arr == NULL) arr_handle_internal_operation_status(ARR_MEMORY_FAULT, "Array create");
    return arr;
}

array_list_t* arr_create_from_ints(int ints[], int len) {
    size_t element_size = map_sizes[ARR_INT];
    array_list_t* arr = arr_allocate(ARR_INT, arr_basic_capacity, element_size);
    if (arr == NULL) {
        arr_handle_internal_operation_status(ARR_MEMORY_FAULT, "Array create from ints");
        return NULL;
    }
    for (int i = 0; i < len; i++) {
        arr_add(arr, using_int(ints[i]));
    }
    return arr;
}

array_list_t* arr_create_from_chars(char chars[], int len) {
    array_list_t* arr = arr_allocate(ARR_CHAR, arr_basic_capacity, map_sizes[ARR_CHAR]);
    if (arr == NULL) {
        arr_handle_internal_operation_status(ARR_MEMORY_FAULT, "Array create from ints");
        return NULL;
    }
    for (int i = 0; i < len; i++) {
        arr_add(arr, using_char(chars[i]));
    }
    return arr;
}

array_list_t* arr_create_from_strings(char* strings[], int len) {
    array_list_t* arr = arr_allocate(ARR_STRINGS, arr_basic_capacity, map_sizes[ARR_STRINGS]);
    if (arr == NULL) {
        arr_handle_internal_operation_status(ARR_MEMORY_FAULT, "Array create from ints");
        return NULL;
    }
    for (int i = 0; i < len; i++) {
        arr_add(arr, using_string(strings[i]));
    }
    return arr;
}

array_list_t* arr_create_from_floats(float floats[], int len) {
    array_list_t* arr = arr_allocate(ARR_FLOAT, arr_basic_capacity, map_sizes[ARR_FLOAT]);
    if (arr == NULL) {
        arr_handle_internal_operation_status(ARR_MEMORY_FAULT, "Array create from ints");
        return NULL;
    }
    for (int i = 0; i < len; i++) {
        arr_add(arr, using_float(floats[i]));
    }
    return arr;
}

array_list_t* arr_create_from_doubles(double doubles[], int len) {
    array_list_t* arr = arr_allocate(ARR_DOUBLE, arr_basic_capacity, map_sizes[ARR_DOUBLE]);
    if (arr == NULL) {
        arr_handle_internal_operation_status(ARR_MEMORY_FAULT, "Array create from ints");
        return NULL;
    }
    for (int i = 0; i < len; i++) {
        arr_add(arr, using_double(doubles[i]));
    }
    return arr;
}

/*============================= PRINTIING OF ARRAY =============================*/
void* arr_get_address(array_list_t* arr, int index) {
    if (index < 0 || index >= arr->length) {
        arr_handle_internal_operation_status(ARR_OUT_OF_BOUNDS, "Array get");
        return NULL;
    }
    return arr->values + index * arr->size_of_one_element;
}

int is_sector_empty(void* target, size_t elem_size) {
    const unsigned char* buffer = (const unsigned char*)target;
    for (int i = 0; i < elem_size; i++) {
        if (buffer[i] != 0) return 0;
    }
    return 1;
}

int is_slot_empty(array_list_t* arr, int id) {
    void* sector = arr_get_address(arr, id);
    if (sector == NULL) return 1;
    return is_sector_empty(sector, arr->size_of_one_element);
}
/* ================ PRIVATE ================*/
void itterate_string(array_list_t* arr) {
    char** list = (char**)arr->values;
    for (size_t i = 0; i < arr->length; i++) {
        void* memory_target = arr_get_address(arr, i);
        char* x = is_sector_empty(memory_target, arr->size_of_one_element) != 1 ? list[i] : "null";
        printf("\n%s ", x);
    }
}
void itterate_int(array_list_t* arr) {
    int* list = (int*)arr->values;
    for (size_t i = 0; i < arr->length; i++) {
        void* memory_target = arr_get_address(arr, i);
        if (is_sector_empty(memory_target, arr->size_of_one_element) != 1) {
            printf("\n%u", list[i]);
        } else {
            printf("\n%s", "null");
        }
    }
}
void itterate_char(array_list_t* arr) {
    char* list = (char*)arr->values;
    for (size_t i = 0; i < arr->length; i++) {
        void* memory_target = arr_get_address(arr, i);
        if (is_sector_empty(memory_target, arr->size_of_one_element) != 1) {
            printf("\n%c", list[i]);
        } else {
            printf("\n%s", "null");
        }
    }
}
void itterate_double(array_list_t* arr) {
    double* list = (double*)arr->values;
    for (size_t i = 0; i < arr->length; i++) {
        void* memory_target = arr_get_address(arr, i);
        if (is_sector_empty(memory_target, arr->size_of_one_element) != 1) {
            printf("\n%f", list[i]);
        } else {
            printf("\n%s", "null");
        }
    }
}
void itterate_float(array_list_t* arr) {
    float* list = (float*)arr->values;
    for (size_t i = 0; i < arr->length; i++) {
        void* memory_target = arr_get_address(arr, i);
        if (is_sector_empty(memory_target, arr->size_of_one_element) != 1) {
            printf("\n%f", list[i]);
        } else {
            printf("\n%s", "null");
        }
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
arr_status arr_print(array_list_t* arr) {
    arr_status st = arr_verify_array(arr, not_after_malloc);
    if (st != ARR_OK) return st;
    arr_print_map[arr->type](arr);
    return ARR_OK;
}

/*============================= SETTING VALUE TO ARRAY =============================*/
/* ================== PRIVATE ================== */
arr_status set_int(array_list_t* arr, arr_value arr_v, size_t index) {
    int* target_1 = (int*)arr->values;
    ((int*)target_1)[index] = *(int*)arr_v.value;
}
arr_status set_char(array_list_t* arr, arr_value arr_v, size_t index) {
    char* target_2 = (char*)arr->values;
    ((char*)target_2)[index] = *(char*)arr_v.value;
}
arr_status set_string(array_list_t* arr, arr_value arr_v, size_t index) {
    char** target_3 = (char**)arr->values;
    ((char**)target_3)[index] = *(char**)arr_v.value;
}
arr_status set_double(array_list_t* arr, arr_value arr_v, size_t index) {
    double* target_4 = (double*)arr->values;
    ((double*)target_4)[index] = *(double*)arr_v.value;
}
arr_status set_float(array_list_t* arr, arr_value arr_v, size_t index) {
    float* target_5 = (float*)arr->values;
    ((float*)target_5)[index] = *(float*)arr_v.value;
}

arr_status (*arr_set_map[types_supported])(array_list_t* arr, arr_value, size_t index);
arr_status arr_init_map_sets() {
    arr_set_map[ARR_INT] = set_int;
    arr_set_map[ARR_CHAR] = set_char;
    arr_set_map[ARR_STRINGS] = set_string;
    arr_set_map[ARR_DOUBLE] = set_double;
    arr_set_map[ARR_FLOAT] = set_float;
}

arr_status arr_clear_sector(void* target, size_t elem_size) {
    unsigned char* t = (unsigned char*)target;
    for (int i = 0; i < elem_size; i++) {
        t[i] = 0;
    }
    return ARR_OK;
}

/* ================== PUBLIC ================== */

arr_status arr_delete(array_list_t* arr, size_t index) {
    arr_status st = arr_verify_array(arr, not_after_malloc);
    if (st != ARR_OK) return st;
    if (arr->length == 0) return ARR_OK;

    size_t elem_size = arr->size_of_one_element;
    void* target = arr_get_address(arr, index);
    if (target == NULL) return ARR_OUT_OF_BOUNDS;

    arr_clear_sector(target, elem_size);
    int is_cleared = 0;
    while (index == arr->length - 1 && arr->length != 0 && index != 0 && is_cleared != 1) {
        void* target_to_shrink = arr_get_address(arr, index);
        if (target_to_shrink == NULL) return ARR_OUT_OF_BOUNDS;
        if (is_sector_empty(target_to_shrink, elem_size) != 1) {
            is_cleared = 1;
        } else {
            arr->length--;
            index--;
        };
    }
    // arr->length--;
    return ARR_OK;
}

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

arr_status arr_set(array_list_t* arr, arr_value arr_v, int index) {
    ARR_TYPE type = arr_v.type;
    if (arr->type != type || arr->type == ARR_CUSTOM) return ARR_INCONSISTENT_TYPE_PROVIDED;
    if (index < 0 || index > arr->length) return ARR_OUT_OF_BOUNDS;
    arr_set_map[arr->type](arr, arr_v, index);
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

arr_value arr_get_int(array_list_t* arr, int index) {
    arr_value val = {0};
    arr_status st = arr_verify_array(arr, not_after_malloc);
    char* operation_name = "Array get char";
    if (st != ARR_OK) {
        arr_handle_internal_operation_status(st, operation_name);
        return val;
    }
    if (arr->type != ARR_INT) {
        arr_handle_internal_operation_status(ARR_INCONSISTENT_TYPE_PROVIDED, operation_name);
        return val;
    }
    void* addres = arr_get_address(arr, index);
    if (addres == NULL) return val;
    val = using_int(*(int*)addres);
    return val;
}

arr_value arr_get_char(array_list_t* arr, int index) {
    arr_value val = {0};
    arr_status st = arr_verify_array(arr, not_after_malloc);
    char* operation_name = "Array get char";
    arr_handle_internal_operation_status(st, operation_name);
    if (st != ARR_OK) return val;
    if (arr->type != ARR_CHAR) {
        arr_handle_internal_operation_status(ARR_INCONSISTENT_TYPE_PROVIDED, operation_name);
        return val;
    }
    void* addres = arr_get_address(arr, index);
    if (addres == NULL) return val;
    val = using_char(*(char*)addres);
    return val;
}

arr_value arr_get_string(array_list_t* arr, int index) {
    arr_value val = {0};
    arr_status st = arr_verify_array(arr, not_after_malloc);
    char* operation_name = "Array get string";
    arr_handle_internal_operation_status(st, operation_name);
    if (st != ARR_OK) return val;
    if (arr->type != ARR_STRINGS) {
        arr_handle_internal_operation_status(ARR_INCONSISTENT_TYPE_PROVIDED, operation_name);
        return val;
    }
    void* addres = arr_get_address(arr, index);
    if (addres == NULL) return val;
    val = using_string(*(char**)addres);
    return val;
}

arr_value arr_get_float(array_list_t* arr, int index) {
    arr_value val = {0};
    char* operation_name = "Array get float";
    arr_status st = arr_verify_array(arr, not_after_malloc);
    if (st != ARR_OK) {
        arr_handle_internal_operation_status(st, operation_name);
        return val;
    }
    if (arr->type != ARR_FLOAT) {
        arr_handle_internal_operation_status(ARR_INCONSISTENT_TYPE_PROVIDED, operation_name);
        return val;
    }
    void* addres = arr_get_address(arr, index);
    if (addres == NULL) return val;
    val = using_float(*(float*)addres);
    return val;
}
arr_value arr_get_double(array_list_t* arr, int index) {
    arr_value val = {0};
    char* operation_name = "Array get double";
    arr_status st = arr_verify_array(arr, not_after_malloc);
    if (st != ARR_OK) {
        arr_handle_internal_operation_status(st, operation_name);
        return val;
    }
    if (arr->type != ARR_DOUBLE) {
        arr_handle_internal_operation_status(ARR_INCONSISTENT_TYPE_PROVIDED, operation_name);
        return val;
    }
    void* addres = arr_get_address(arr, index);
    if (addres == NULL) return val;
    val = using_double(*(double*)addres);
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
    arr_status st = arr_verify_array(arr1, not_after_malloc);
    if (st != ARR_OK) return st;
    qsort(arr1->values, arr1->length, arr1->size_of_one_element, sorting_algorithm);
    return ARR_OK;
};

/*=============================== CLEARING ==================================*/
arr_status arr_free(array_list_t* arr) {
    if (arr == NULL) return ARR_IS_NULL;
    if (arr->values == NULL) return ARR_VALUE_IS_NULL;
    free(arr->values);
    arr->values = NULL;
    free(arr);
    return ARR_OK;
}

/*================================ GETTERS ================================*/
int arr_get_size_of_element(array_list_t* arr) {
    arr_status st = arr_verify_array(arr, not_after_malloc);
    if (st != ARR_OK) {
        arr_handle_status(st);
        return 0;
    }

    return arr->size_of_one_element;
};

ARR_TYPE arr_get_type(array_list_t* arr) {
    arr_status st = arr_verify_array(arr, not_after_malloc);
    if (st != ARR_OK) {
        arr_handle_status(st);
        return 0;
    }

    ARR_TYPE type = arr->type;
    if (type == ARR_CUSTOM) printf("Custom type: %s", arr->custom_type);
    return type;
};
int arr_get_mem_capacity(array_list_t* arr) {
    arr_status st = arr_verify_array(arr, not_after_malloc);
    if (st != ARR_OK) {
        arr_handle_status(st);
        return 0;
    }

    return arr->capacity;
};
int arr_get_elements_capacity(array_list_t* arr) {
    arr_status st = arr_verify_array(arr, not_after_malloc);
    if (st != ARR_OK) {
        arr_handle_status(st);
        return 0;
    }

    return arr->capacity / arr->size_of_one_element;
};
int arr_get_length(array_list_t* arr) {
    arr_status st = arr_verify_array(arr, not_after_malloc);
    if (st != ARR_OK) {
        arr_handle_status(st);
        return 0;
    }

    return arr->length;
};

void arr_init() {
    arr_init_map_prints();
    arr_init_map_adds();
    arr_init_map_sizes();
    arr_init_map_equals();
    arr_init_map_sets();
}