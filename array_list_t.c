#include "array_list_t.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define arr_basic_capacity 1024
#define types_supported

typedef struct array_list_t {
    int length;
    int capacity;
    void* values;
    ARR_TYPE type;
} array_list_t;


/* ============================= USED INSIDE C FILE =============================*/

/* ================ CREATION OF ARRAY ================*/
array_list_t* arr_allocate(ARR_TYPE DataType, int capacity){
    array_list_t* arr = malloc(sizeof(array_list_t));
    if(arr == NULL) return NULL;
    arr->capacity = capacity;
    arr->length = 0;
    arr->type = DataType;
    arr->values = malloc(capacity);
    if(arr->values == NULL) return NULL;
    return arr;
}

/* ================ FOR PRINTING ================*/

void itterate_string(array_list_t* arr) { 
    char** target = (char**) arr->values; 
    for (size_t i = 0; i < arr->length; i++){
        printf("\n%s", target[i]); 
    } 
}

void itterate_int(array_list_t* arr) {
    int* list = (int*) arr->values;
    for (size_t i = 0; i < arr->length; i++){
        printf("\n%u", list[i]);
    }
}

void itterate_char(array_list_t* arr) {
    char* list = (char*) arr->values;
    for (size_t i = 0; i < arr->length; i++){
        printf("\n%c", list[i]);
    }
}

void itterate_double(array_list_t* arr) {
    double* list = (double*) arr->values;
    for (size_t i = 0; i < arr->length; i++){
        printf("\n%f", list[i]);
    }
}

void itterate_float(array_list_t* arr) {
    float* list = (float*) arr->values;
    for (size_t i = 0; i < arr->length; i++){
        printf("\n%f", list[i]);
    }
}

void (*arr_print_map[types_supported])(array_list_t* arr);
void arr_init(){
    *arr_print_map;
    arr_print_map[ARR_INT] = itterate_int;
    arr_print_map[ARR_CHAR] = itterate_char;
    arr_print_map[ARR_STRINGS] = itterate_string;
    arr_print_map[ARR_DOUBLE] = itterate_double;
    arr_print_map[ARR_FLOAT] = itterate_float;
}



/* ============================= PUBLIC FUNCTIONS TO USE =============================*/

void arr_print(array_list_t* arr){
    arr_print_map[arr->type](arr);
}



array_list_t* arr_create (ARR_TYPE DataType){
    return arr_allocate(DataType, arr_basic_capacity);
}



int arr_equals(array_list_t* arr1, array_list_t* arr2){
    
    //TO DO


    // int x = arr1->length;
    // int y = arr2->length;
    // if(x != y) return 0;
    // ARR_TYPE type = arr1->type;
    // if(type != arr2->type) return 0;

    // int* arr_int = NULL;
    // char** arr_str = NULL;
    // char* arr_char = NULL;
    
    // arr_cast_values_and_apply(type,
    //     int* a = (int*) arr1->values;
    //     int* b = (int*) arr2->values;
    //     for (size_t i = 0; i < x; i++){
    //         if(a[i] != b[i]) return 0;
    //     }
    //     return 1;
    //     ,
    //     char* a = (char*) arr1->values;
    //     char* b = (char*) arr2->values;
    //     for (size_t i = 0; i < x; i++){
    //         if(a[i] != b[i]) return 0;
    //     }
    //     return 1;,

    //     char** a = (char**) arr1->values;
    //     char** b = (char**) arr2->values;
    //     for (size_t i = 0; i < x; i++){
    //         if(a[i] != b[i]) return 0;
    //     }
    //     return 1;
    // );

    // return 1;
}





void check_errors(ARR_TYPE type, void* value){
    
}

void free_using_container(arr_value av){
    free(av.value);
}

arr_status arr_add(array_list_t* arr, arr_value arr_v){
   
    ARR_TYPE type = arr_v.type;
        if(arr->type != type) return ARR_INCONSISTENT_TYPE_PROVIDED;
        switch(type){
            case ARR_INT:
                int* target_1 = (int*)arr->values;
                ((int*)target_1)[arr->length] = *(int*)arr_v.value;
                arr->length++;
                free_using_container(arr_v);
            break;
            case ARR_CHAR:
                char* target_2 = (char*) arr->values;
                ((char*)target_2)[arr->length] = *(char*)arr_v.value;
                arr->length++;
                free_using_container(arr_v);
            break;
            case ARR_STRINGS:
                char** target_3 = (char**) arr->values;
                ((char**)target_3)[arr->length] = *(char**)arr_v.value;
                arr->length++;
                free_using_container(arr_v);
            break;
             case ARR_DOUBLE:
                double* target_4 = (double*)arr->values;
                ((double*)target_4)[arr->length] = *(double*)arr_v.value;
                arr->length++;
                free_using_container(arr_v);
            break;
            case ARR_FLOAT:
                float* target_5 = (float*)arr->values;
                ((float*)target_5)[arr->length] = *(float*)arr_v.value;
                arr->length++;
                free_using_container(arr_v);
            break;
            default:
                break;
        }
        return ARR_OK;
}

arr_value using_int(int i){
    arr_value arr_v;
    arr_v.type = ARR_INT;
    arr_v.value = malloc(sizeof(i));
    *(int*)arr_v.value = i;
    return arr_v;
};

arr_value using_char(char c){
    arr_value arr_v;
    arr_v.type = ARR_CHAR;
    arr_v.value = malloc(sizeof(c));
    *(char*)arr_v.value = c;
    return arr_v;
};
arr_value using_string(char* s){
    arr_value arr_v;
    arr_v.type = ARR_STRINGS;
    arr_v.value = malloc(sizeof(s));
    *(char**)arr_v.value = s;
    return arr_v;
};
arr_value using_double(double d){
        arr_value arr_v;
    arr_v.type = ARR_DOUBLE;
    arr_v.value = malloc(sizeof(d));
    *(double*)arr_v.value = d;
    return arr_v;
};
arr_value using_float(float f){
    arr_value arr_v;
    arr_v.type = ARR_FLOAT;
    arr_v.value = malloc(sizeof(f));
    *(float*)arr_v.value = f;
    return arr_v;
};
