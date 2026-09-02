#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../string_builder_t/string_builder_t.h"
#include "array_list_t.h"
#include "array_list_t_private.h"

#define MAX_TYPES 64
char* registered_types[MAX_TYPES];
void (*provided_prints[MAX_TYPES])(const void* b);
int (*provided_equals[MAX_TYPES])(const void* arr_v1, const void* arr_v2);

int type_register_counter = 0;

char* build_error_message(char* source, char* type1, char* type2) {
    if (source == NULL || type1 == NULL)
        fprintf(stderr, "Error occured while constructing error messagge");
    int to_alloc =
        sizeof(source) + sizeof(type1) + (type2 == NULL ? 0 : sizeof(type2)) + sizeof(char) * 4;
    char* message = malloc(to_alloc);

    strcpy(message, source);
    if (type2 != NULL) {
        strcat(message, type1);
        strcat(message, " | ");
        strcat(message, type2);
    } else {
        strcat(message, type1);
    }
}

arr_status arr_verify_custom_array_and_type(array_list_t* arr, arr_value arr_v, int aftermalloc) {
    arr_status st = arr_verify_array(arr, aftermalloc);
    if (st != ARR_OK) return st;
    if (arr_v.value == NULL) return ARR_VALUE_IS_NULL;
    if (arr->type != ARR_CUSTOM) return ARR_INCONSISTENT_TYPE_PROVIDED;
    if (arr->type == ARR_CUSTOM && (arr->custom_type == NULL || arr->custom_type[0] == '\0'))
        return ARR_CUSTOM_BUT_TYPE_NOT_SPECIFIED;
    if (arr->type != arr_v.type || strcmp(arr->custom_type, arr_v.custom_type) != 0)
        return ARR_INCONSISTENT_TYPE_PROVIDED;
    return ARR_OK;
}

int is_type_registered(char* type) {
    for (int i = 0; i < MAX_TYPES; i++) {
        if (registered_types[i] != NULL) {
            if (strcmp(registered_types[i], type) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

int get_type_id(char* type) {
    if (is_type_registered(type) != 1) return -1;
    for (int i = 0; i < MAX_TYPES; i++) {
        if (registered_types[i] != NULL) {
            if (strcmp(registered_types[i], type) == 0) {
                return i;
            }
        }
    }
    return -1;
}

array_list_t* arr_create_custom(char* generic_name, size_t element_size) {
    if (is_type_registered(generic_name) == 0) {
        char* message = build_error_message("Error: Custom array create for ", generic_name, NULL);
        arr_handle_internal_operation_status(ARR_CUSTOM_TYPE_IS_NOT_REGISTERED, message);
        free(message);
        return NULL;
    }

    array_list_t* arr = arr_allocate(ARR_CUSTOM, arr_basic_capacity, element_size);
    arr_status status = arr_verify_array(arr, after_malloc);

    if (status != ARR_OK) {
        char* message = build_error_message("Custom array create for ", generic_name, NULL);
        arr_handle_internal_operation_status(status, message);
        free(message);
        return NULL;
    }
    arr->custom_type = strdup(generic_name);
    return arr;
}

array_list_t* arr_create_custom_greedy(char* generic_name, size_t element_size,
                                       int basic_capacity) {
    if (is_type_registered(generic_name) == 0) {
        char* message = build_error_message("Error: Custom array create for ", generic_name, NULL);
        arr_handle_internal_operation_status(ARR_CUSTOM_TYPE_IS_NOT_REGISTERED, message);
        free(message);
        return NULL;
    }

    array_list_t* arr = arr_allocate(ARR_CUSTOM, basic_capacity, element_size);
    arr_status status = arr_verify_array(arr, after_malloc);

    if (status != ARR_OK) {
        char* message = build_error_message("Custom array create for ", generic_name, NULL);
        arr_handle_internal_operation_status(status, message);
        free(message);
        return NULL;
    }
    arr->custom_type = strdup(generic_name);
    return arr;
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
    arr_v.size = size;
    arr_v.value = malloc(size);
    memcpy(arr_v.value, value, size);
    return arr_v;
}

arr_value arr_get_custom(array_list_t* arr, int index) {
    arr_value val = {0};
    void* addres = arr_get_address_in_values(arr, index);
    if (addres == NULL) return val;
    val = using_custom(addres, arr->custom_type, arr->size_of_one_element);
    return val;
}

arr_status arr_custom_add(array_list_t* arr, arr_value arr_v) {
    arr_status st1 = arr_verify_custom_array_and_type(arr, arr_v, not_after_malloc);

    if (st1 != ARR_OK) return st1;

    if (is_type_registered(arr_v.custom_type) == 0) return ARR_CUSTOM_TYPE_IS_NOT_REGISTERED;

    if (arr->size_of_one_element != arr_v.size)
        return ARR_SIZES_OF_ARRAY_ELEMENT_AND_PROVIDED_ARE_DEFER;
    arr_status status = check_memory_allocation(arr);

    if (status != ARR_OK) return status;

    memcpy(arr->values + arr->size_of_one_element * arr->length, arr_v.value,
           arr->size_of_one_element);

    free_using_container(arr_v);
    arr->length++;
    return ARR_OK;
}

arr_status arr_custom_set(array_list_t* arr, arr_value arr_v, int index) {
    arr_status st1 = arr_verify_custom_array_and_type(arr, arr_v, not_after_malloc);
    if (st1 != ARR_OK) return st1;
    if (is_type_registered(arr_v.custom_type) == 0) return ARR_CUSTOM_TYPE_IS_NOT_REGISTERED;
    if (arr->size_of_one_element != arr_v.size)
        return ARR_SIZES_OF_ARRAY_ELEMENT_AND_PROVIDED_ARE_DEFER;

    memcpy(arr->values + arr->size_of_one_element * index, arr_v.value, arr->size_of_one_element);
    free_using_container(arr_v);
    return ARR_OK;
}

arr_status arr_custom_provide_print(char* type, void(print)(const void* b)) {
    int id = get_type_id(type);
    if (id == -1) return ARR_CUSTOM_TYPE_IS_NOT_REGISTERED;
    provided_prints[id] = print;
    return ARR_OK;
};
arr_status arr_custom_provide_equals(char* type,
                                     int(comp)(const void* arr_v1, const void* arr_v2)) {
    int id = get_type_id(type);
    if (id == -1) return ARR_CUSTOM_TYPE_IS_NOT_REGISTERED;
    provided_equals[id] = comp;
    return ARR_OK;
};

int is_print_registered_for_that_type(char* type) {
    int id = get_type_id(type);
    if (id == -1) return 0;
    if (provided_prints[id] == NULL) return 0;
    return 1;
}

int is_equals_registered_for_that_type(char* type) {
    int id = get_type_id(type);
    if (id == -1) return 0;
    if (provided_equals[id] == NULL) return 0;
    return 1;
}

arr_status arr_custom_print(array_list_t* arr) {
    arr_status st1 = arr_verify_array(arr, not_after_malloc);
    if (st1 != ARR_OK) return st1;
    if (is_type_registered(arr->custom_type) == 0) return ARR_CUSTOM_TYPE_IS_NOT_REGISTERED;

    if (is_print_registered_for_that_type(arr->custom_type) != 1)
        return ARR_PRINT_IS_NOT_REGISTERED_FOR_THAT_TYPE;

    void (*func)(const void*) = provided_prints[get_type_id(arr->custom_type)];

    for (int i = 0; i < arr->length; i++) {
        if (is_slot_empty(arr, i) == 1) {
            printf("\n%s", "null");
        } else {
            void* value = arr->values + i * arr->size_of_one_element;
            func(value);
        }
    }

    return ARR_OK;
}

int arr_custom_equals(array_list_t* arr1, array_list_t* arr2) {
    arr_status st1 = arr_verify_array(arr1, not_after_malloc);
    arr_status st2 = arr_verify_array(arr2, not_after_malloc);

    if (st1 != ARR_OK) {
        arr_handle_internal_operation_status(st1, "Custom array equality check");
        return -1;
    }
    if (st2 != ARR_OK) {
        arr_handle_internal_operation_status(st1, "Custom array equality check");
        return -1;
    };

    char* type = arr1->custom_type;

    if (strcmp(type, arr2->custom_type) != 0) return 0;

    if (is_type_registered(type) == 0) {
        arr_handle_internal_operation_status(ARR_CUSTOM_TYPE_IS_NOT_REGISTERED,
                                             "Custom array equality check");
        return -1;
    }

    if (is_equals_registered_for_that_type(type) == 0) {
        arr_handle_internal_operation_status(ARR_EQUALS_IS_NOT_REGISTERED_FOR_THAT_TYPE,
                                             "Custom array equality check");
        return -1;
    }

    int (*func)(const void* arr_v1, const void* arr_v2) = provided_equals[get_type_id(type)];

    for (int i = 0; i < arr1->length; i++) {
        void* value1 = arr1->values + i * arr1->size_of_one_element;
        void* value2 = arr2->values + i * arr2->size_of_one_element;
        if (func(value1, value2) != 1) return 0;
    }

    return 1;
}

array_list_t* arr_create_from_customs(void* values[], int len, char* generic_name,
                                      size_t element_size) {
    if (is_type_registered(generic_name) == 0) {
        char* message = build_error_message("Error: Custom array create for ", generic_name, NULL);
        arr_handle_internal_operation_status(ARR_CUSTOM_TYPE_IS_NOT_REGISTERED, message);
        free(message);
        return NULL;
    }

    array_list_t* arr = arr_allocate(ARR_CUSTOM, arr_basic_capacity, element_size);
    arr_status status = arr_verify_array(arr, after_malloc);

    if (status != ARR_OK) {
        char* message = build_error_message("Custom array create for ", generic_name, NULL);
        arr_handle_internal_operation_status(status, message);
        free(message);
        return NULL;
    }
    arr->custom_type = strdup(generic_name);

    for (int i = 0; i < len; i++) {
        arr_custom_add(arr, using_custom(values[i], generic_name, element_size));
    }
    return arr;
}
