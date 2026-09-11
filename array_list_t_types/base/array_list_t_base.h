array_list_t* arr_create(ARR_TYPE DataType);
array_list_t* arr_create_greedy(ARR_TYPE DataType, int basic_capacity);

array_list_t* arr_create_from_ints(int ints[], int len);
array_list_t* arr_create_from_chars(char chars[], int len);
array_list_t* arr_create_from_strings(char* strings[], int len);
array_list_t* arr_create_from_floats(float floats[], int len);
array_list_t* arr_create_from_doubles(double doubles[], int len);
array_list_t* arr_create_from_longs(long longs[], int len);
array_list_t* arr_create_from_long_longs(long long longlongs[], int len);
array_list_t* arr_create_from_long_doubles(long double longdoubles[], int len);
array_list_t* arr_create_from_shorts(short shorts[], int len);
array_list_t* arr_create_from_booleans(bool booleans[], int len);

int arr_equals(array_list_t* arr1, array_list_t* arr2);
arr_value arr_get_int(array_list_t* arr, int index);
arr_value arr_get_char(array_list_t* arr, int index);
arr_value arr_get_string(array_list_t* arr, int index);
arr_value arr_get_float(array_list_t* arr, int index);
arr_value arr_get_double(array_list_t* arr, int index);
arr_value arr_get_long(array_list_t* arr, int index);
arr_value arr_get_long_long(array_list_t* arr, int index);
arr_value arr_get_long_double(array_list_t* arr, int index);
arr_value arr_get_short(array_list_t* arr, int index);
arr_value arr_get_boolean(array_list_t* arr, int index);

arr_value arr_get_variant(array_list_t* arr, int index);
arr_value* arr_get_variant_reference(array_list_t* arr, int index);

arr_status arr_add(array_list_t* arr, arr_value value);
arr_status arr_set(array_list_t* arr, arr_value arr_v, int index);

arr_status arr_print(array_list_t* arr);
void arr_print_value(arr_value* arr_v);

arr_value using_int(int i);
arr_value using_char(char c);
arr_value using_string(char* s);
arr_value using_double(double d);
arr_value using_float(float f);
arr_value using_long(long l);
arr_value using_long_long(long long ll);
arr_value using_long_double(long double ld);
arr_value using_short(short s);
arr_value using_boolean(bool b);
arr_value using_null();

arr_status arr_delete(array_list_t* arr, size_t index);
arr_status arr_for_each(array_list_t* arr, void(fn)(void* value));
arr_status arr_free(array_list_t* arr);
arr_status arr_print_bit_mask_of_presence(array_list_t* arr);