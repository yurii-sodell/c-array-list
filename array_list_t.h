typedef struct array_list_t array_list_t;

typedef enum {
    ARR_INT, //1
    ARR_STRINGS, //2
    ARR_CHAR, //3
    ARR_DOUBLE, //4
    ARR_FLOAT //5
} ARR_TYPE;


typedef struct arr_value{
    void* value;
    ARR_TYPE type;
} arr_value;

typedef enum{
    ARR_OK,
    ARR_OUT_OF_MEMORY_BOUNDS,
    ARR_MEMORY_FAULT,
    ARR_INCONSISTENT_TYPE_PROVIDED
} arr_status;


void arr_init();
int arr_equals(array_list_t* arr1, array_list_t* arr2);
void arr_print(array_list_t* arr);
array_list_t* arr_create (ARR_TYPE DataType);
arr_status arr_add(array_list_t* arr, arr_value value);

arr_value using_int(int i);
arr_value using_char(char c);
arr_value using_string(char* s);
arr_value using_double(double d);
arr_value using_float(float f);

