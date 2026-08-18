# array_list_t

## Overview

`array_list_t` is a generic dynamic array implementation in C. The
library supports two classes of types: built-in types (`ARR_INT`,
`ARR_CHAR`, `ARR_STRINGS`, `ARR_DOUBLE`, `ARR_FLOAT`) and a custom type
(`ARR_CUSTOM`). For built-in types, the element type is checked by
direct comparison of `ARR_TYPE` enum values (`arr->type == ARR_INT`),
and the correct implementation of an operation is selected through a
function pointer table indexed by that same value, so dispatch is O(1)
with no string comparison involved. For the custom type, the type is
identified by a string literal (for example `"Point"`) registered in a
global registry, and every operation on a custom array requires
looking up that name with `strcmp`. As a result, operations on
built-in-type arrays are faster than operations on custom arrays, and
the custom type is worth using only when the data has no built-in
equivalent.

The array is implemented as a contiguous buffer (`values`) with manual
capacity (`capacity`) and logical length (`length`) tracking. The
library tracks buffer usage on its own and grows the buffer when
elements are added, so growth never needs to be triggered manually.
The buffer never shrinks back. To reclaim memory the array must be
freed entirely (`arr_free`) and, if needed, a new one created.

Deleting an element (`arr_delete`) does not shift the following
elements and does not shrink the buffer: the cell of the deleted
element is filled with zero bytes and is treated as empty (`null`)
when printed. The logical length is reduced only when the deleted
element is the last one in the array; in that case the length is
trimmed back to the last non-empty element from the end.

## File layout

| File | Purpose |
|---|---|
| `array_list_t.h` | Public interface. The only header a user of the library should include. |
| `array_list_t_private.h` | Internal functions and the array struct definition, shared between `array_list_t_base.c` and `array_list_t_custom.c`. Not meant to be included by library users. |
| `array_list_t_base.c` | Operations on built-in types. |
| `array_list_t_custom.c` | Operations on the custom type and the type registry. |

The `array_list_t` struct is opaque: fields can only be accessed
through the getter functions (see "Metadata getters").

## Initialization

```c
void arr_init(void);
```

Fills the dispatch tables for built-in types (sizes, add, set, equals,
print). Must be called before any other library call.

## Return codes

Most functions return `arr_status`. `ARR_OK` (1) means success, any
other value is an error code.

```c
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
```

A macro is provided for uniform error handling:

```c
#define has(status) arr_handle_status(status)
```

Calls that return `arr_status` should generally be wrapped in
`has(...)`: on any code other than `ARR_OK` the error message is
printed to `stderr` automatically.

```c
has(arr_add(arr, using_int(5)));
```

## Data types

### ARR_TYPE

```c
typedef enum {
    ARR_INT,      // 0
    ARR_STRINGS,  // 1
    ARR_CHAR,     // 2
    ARR_DOUBLE,   // 3
    ARR_FLOAT,    // 4
    ARR_CUSTOM    // 5
} ARR_TYPE;
```

The first five values are built-in types, dispatched through function
pointer tables indexed directly by the `ARR_TYPE` value. `ARR_CUSTOM`
is used only for arrays created with `arr_create_custom` /
`arr_create_from_customs`; the actual element type is determined by
the `custom_type` field.

### arr_value

```c
typedef struct arr_value {
    void* value;
    ARR_TYPE type;
    char* custom_type;
    size_t size;
} arr_value;
```

A generic container used to pass a value into add/set functions and to
return a value from get functions. `custom_type` and `size` are only
populated for `ARR_CUSTOM`.

## Memory ownership model: arr_value containers

Values are passed into and read from the array through an `arr_value`
wrapper. A container is created by the `using_*` functions:

```c
arr_value using_int(int i);
arr_value using_char(char c);
arr_value using_string(char* s);
arr_value using_double(double d);
arr_value using_float(float f);
arr_value using_custom(void* value, char* name, size_t size);
```

Each function allocates a heap copy of the value (`using_string` copies
the pointer `s`, not the string itself; `using_custom` copies `size`
bytes from `value` and duplicates `name` with `strdup`).

Add and set functions (`arr_add`, `arr_set`, `arr_custom_add`,
`arr_custom_set`) free the passed-in container themselves after
copying its value into the array, by calling `free_using_container`.
Freeing a container again, or using it after it has been passed to
`arr_add` / `arr_set`, is not supported.

Get functions (`arr_get_int`, `arr_get_custom`, etc.) return a new
container, which must be released manually with
`free_using_container` once the value is no longer needed.

## Creating an array

### arr_create

```c
array_list_t* arr_create(ARR_TYPE DataType);
```

Creates an empty array of a built-in type. `ARR_CUSTOM` is not
supported here (use `arr_create_custom` instead). Returns a pointer to
the array or `NULL` on error.

### arr_create_custom

```c
array_list_t* arr_create_custom(char* generic_name, size_t element_size);
```

Creates an empty custom-type array named `generic_name` with element
size `element_size` bytes. The type must already be registered with
`arr_custom_register_type`.

## Creating arrays with a custom initial capacity

### arr_create_greedy

```c
array_list_t* arr_create_greedy(ARR_TYPE DataType, int basic_capacity);
```

Same as `arr_create`, but lets the caller set the initial capacity
explicitly instead of using the default `arr_basic_capacity` (1024).
Useful when the expected size of the array is known in advance: a
larger `basic_capacity` avoids early reallocations, a smaller one
avoids allocating memory that won't be used. `ARR_CUSTOM` is not
supported here (use `arr_create_custom_greedy` instead). Returns a
pointer to the array or `NULL` on error.

### arr_create_custom_greedy

```c
array_list_t* arr_create_custom_greedy(char* generic_name, size_t element_size, int basic_capacity);
```

Same as `arr_create_custom`, but with an explicit initial capacity
instead of the default `arr_basic_capacity`. The type must already be
registered with `arr_custom_register_type`, exactly as for
`arr_create_custom`.

> **Note:** `basic_capacity` sets the *initial* capacity only —
> subsequent growth behavior is unchanged (the buffer still grows
> automatically as elements are added and never shrinks back). 

### arr_create_from_ints / arr_create_from_chars / arr_create_from_strings / arr_create_from_floats / arr_create_from_doubles

```c
array_list_t* arr_create_from_ints(int ints[], int len);
array_list_t* arr_create_from_chars(char chars[], int len);
array_list_t* arr_create_from_strings(char* strings[], int len);
array_list_t* arr_create_from_floats(float floats[], int len);
array_list_t* arr_create_from_doubles(double doubles[], int len);
```

Create an array of the corresponding type and fill it with the first
`len` elements of the given C array. For `arr_create_from_strings` the
array stores pointers to the supplied strings, not copies of them.

### arr_create_from_customs

```c
array_list_t* arr_create_from_customs(void* values[], int len, char* generic_name, size_t element_size);
```

Creates a custom-type array named `generic_name` and fills it with
`len` elements, copying `element_size` bytes from each pointer in
`values`.

## Capacity and buffer growth

The initial capacity of an array is `arr_basic_capacity` (1024),
expressed in elements multiplied by element size: the actual buffer
size in bytes equals `capacity * size_of_one_element`.

When an element is added (`arr_add`, `arr_custom_add`), the array
checks buffer usage on its own and grows the buffer if needed; growth
never needs to be triggered manually. Capacity never shrinks, neither
when elements are deleted nor by any other means. The only way to
reclaim memory is to free the array entirely (`arr_free`) and create a
new one afterwards.

## Adding, updating and deleting elements

### arr_add

```c
arr_status arr_add(array_list_t* arr, arr_value value);
```

Appends a value to the end of a built-in-type array. The type of
`value` must match the array type. Returns
`ARR_INCONSISTENT_TYPE_PROVIDED`, `ARR_MEMORY_FAULT`, or `ARR_OK`.

### arr_set

```c
arr_status arr_set(array_list_t* arr, arr_value arr_v, int index);
```

Replaces the value at `index`. Buffer growth is not performed here;
`index` must point to an already occupied position within
`[0, length)`. Returns `ARR_INCONSISTENT_TYPE_PROVIDED`,
`ARR_OUT_OF_BOUNDS`, or `ARR_OK`.

### arr_delete

```c
arr_status arr_delete(array_list_t* arr, size_t index);
```

Deletes the element at `index`: the cell is filled with zero bytes,
other elements are not shifted. If the deleted element is the last one
in the array, the length is additionally trimmed while trailing cells
remain empty. Returns `ARR_OUT_OF_BOUNDS`, codes from array
verification, or `ARR_OK`.

### arr_custom_add

```c
arr_status arr_custom_add(array_list_t* arr, arr_value arr_v);
```

Equivalent of `arr_add` for a custom array. Checks the validity of the
array and value, that the array is `ARR_CUSTOM`, that the type name
and element size match, and that the type is registered. Grows the
buffer if needed.

### arr_custom_set

```c
arr_status arr_custom_set(array_list_t* arr, arr_value arr_v, int index);
```

Equivalent of `arr_set` for a custom array. Index bounds checking and
buffer growth are not performed.

## Reading values

```c
arr_value arr_get_int(array_list_t* arr, int index);
arr_value arr_get_char(array_list_t* arr, int index);
arr_value arr_get_string(array_list_t* arr, int index);
arr_value arr_get_float(array_list_t* arr, int index);
arr_value arr_get_double(array_list_t* arr, int index);
arr_value arr_get_custom(array_list_t* arr, int index);
```

Return an `arr_value` container holding the value at `index`. On error
an empty container `{0}` is returned (`value == NULL`). The reliable
way to detect an error is checking `value == NULL`, not the `type`
field, since `ARR_INT` equals `0` and matches the `type` field of an
empty container. `arr_get_string` returns the same pointer that is
stored in the array, without copying the string contents.
`arr_get_custom` returns a copy of the custom-array element.

## Comparing arrays

### arr_equals

```c
int arr_equals(array_list_t* arr1, array_list_t* arr2);
```

Compares two built-in-type arrays element by element. Returns `1` if
types, lengths and all values match, `0` otherwise. Only for built-in
types; use `arr_custom_equals` for custom arrays.

### arr_custom_equals

```c
int arr_custom_equals(array_list_t* arr1, array_list_t* arr2);
```

Compares two custom arrays using the comparison function registered
for that type with `arr_custom_provide_equals`. Returns `1` on
equality, `0` on inequality, `-1` on error. Array lengths are not
checked before comparison; the loop runs over the length of `arr1`.

### arr_custom_provide_equals

```c
arr_status arr_custom_provide_equals(char* type, int (*comp)(const void* arr_v1, const void* arr_v2));
```

Registers a comparison function for the custom type `type`, used by
`arr_custom_equals`. The function must return exactly `1` when
elements are equal; any other value is treated as "not equal".

## Printing

### arr_print

```c
arr_status arr_print(array_list_t* arr);
```

Prints the elements of a built-in-type array to `stdout`. Deleted
(zeroed) cells are printed as `null`.

### arr_custom_print

```c
arr_status arr_custom_print(array_list_t* arr);
```

Prints the elements of a custom array using the print function
registered with `arr_custom_provide_print`. Deleted cells are printed
as `null`.

### arr_custom_provide_print

```c
arr_status arr_custom_provide_print(char* type, void (*print)(const void* b));
```

Registers a print function for one element of the custom type `type`.
The callback receives a pointer to the raw bytes of the element.

## Sorting

### arr_sort

```c
arr_status arr_sort(array_list_t* arr, int (*sorting_algorithm)(const void* a, const void* b));
```

Sorts the array in place using `qsort`; the comparator has the same
semantics as a standard `qsort` comparator. For `ARR_STRINGS` the
buffer elements are `char*` pointers, so the comparator receives a
`const void*` pointing to a `char*` and needs a double dereference:

```c
int cmp_strings(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}
```

## Registering custom types

Before creating a custom array or using `arr_custom_add`,
`arr_custom_set`, `arr_custom_provide_print`,
`arr_custom_provide_equals`, the type must be registered by name:

```c
arr_status arr_custom_register_type(char* type);
```

The registry is a fixed-size array; the `MAX_TYPES` constant in
`array_list_t_custom.c` is `64`, not `1024`, of which at most 63 slots
can actually be used.

```c
arr_status arr_custom_unregister_type(char* type);
```

Unregisters a type, freeing the memory holding its name in the
registry. The slot is not cleared or reused afterwards, so
re-registering the same name after unregistering it is not
recommended.

## Freeing memory

```c
arr_status arr_free(array_list_t* arr);
```

Frees the `values` buffer and the array struct itself. Does not free
data pointed to by the elements (strings in `ARR_STRINGS`, nested
pointers inside custom elements) or the `custom_type` field; that is
the caller's responsibility. Since capacity never shrinks, reclaiming
part of the memory requires recreating the array.

## Metadata getters

```c
int arr_get_size_of_element(array_list_t* arr);
ARR_TYPE arr_get_type(array_list_t* arr);
int arr_get_mem_capacity(array_list_t* arr);
int arr_get_elements_capacity(array_list_t* arr);
int arr_get_length(array_list_t* arr);
```

All of these check array validity and return `0` on error.

- `arr_get_size_of_element`: size of one element in bytes.
- `arr_get_type`: array type. For a custom array, also prints the type
  name to `stdout`.
- `arr_get_mem_capacity`: current array capacity (see "Capacity and
  buffer growth").
- `arr_get_elements_capacity`: capacity in elements
  (`capacity / size_of_one_element`).
- `arr_get_length`: current number of elements.

## Usage example

```c
#include "array_list_t.h"

int cmp_int(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

typedef struct { int x, y; } Point;

int point_equals(const void* a, const void* b) {
    const Point* p1 = (const Point*)a;
    const Point* p2 = (const Point*)b;
    return (p1->x == p2->x && p1->y == p2->y) ? 1 : 0;
}

int main(void) {
    arr_init();

    array_list_t* nums = arr_create(ARR_INT);
    has(arr_add(nums, using_int(5)));
    has(arr_add(nums, using_int(1)));
    has(arr_add(nums, using_int(3)));

    has(arr_sort(nums, cmp_int));
    has(arr_print(nums));

    arr_value first = arr_get_int(nums, 0);
    printf("%d\n", *(int*)first.value);
    free_using_container(first);

    has(arr_delete(nums, 1));
    arr_free(nums);

    has(arr_custom_register_type("Point"));
    has(arr_custom_provide_equals("Point", point_equals));

    array_list_t* points_a = arr_create_custom("Point", sizeof(Point));
    array_list_t* points_b = arr_create_custom("Point", sizeof(Point));

    Point p1 = {1, 2};
    Point p2 = {3, 4};
    has(arr_custom_add(points_a, using_custom(&p1, "Point", sizeof(Point))));
    has(arr_custom_add(points_a, using_custom(&p2, "Point", sizeof(Point))));
    has(arr_custom_add(points_b, using_custom(&p1, "Point", sizeof(Point))));
    has(arr_custom_add(points_b, using_custom(&p2, "Point", sizeof(Point))));

    int equal = arr_custom_equals(points_a, points_b);
    printf("points_a == points_b: %d\n", equal);

    arr_free(points_a);
    arr_free(points_b);

    return 0;
}
```
