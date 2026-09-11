#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../array_list_t_accesability/array_list_t_shared.h"

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
                "Error: comparing function (expected return values: -1, 0, 1) is not registered for this type";
            break;
        case ARR_SIZES_OF_ARRAY_ELEMENT_AND_PROVIDED_ARE_DEFER:
            message = "Error: sizes of provided element and size of one array element are defer";
            break;
        case ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED:
            message = "Error: unsafe method was called on safe type. Try to use the method that corresponds the type of array. (for example, arr_custom, arr_variant or arr_int)";
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

void arr_handle_status(arr_status st) {
    arr_handle_internal_operation_status(st, NULL);
}

void arr_handle_error_message_without_status(char* message) {
    if (message != NULL) {
        fprintf(stderr, "\n%s", message);
    } else {
        fprintf(stderr, "\nError message handler was called but no message was provided. ", message);
    }
}

char* arr_build_error_message_for_custom_types(char* source, char* type1, char* type2) {
    if (source == NULL || type1 == NULL)
        fprintf(stderr, "Error occured while constructing error messagge");
    int len = strlen(source) + strlen(type1) + (type2 == NULL ? 0 : strlen(type2)) + 4;
    int to_alloc = len * sizeof(char);
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
