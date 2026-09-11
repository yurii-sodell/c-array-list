# array_list_t

## Overview

`array_list_t` is a generic dynamic array implementation in C. The
library supports several classes of element types: built-in types
(`ARR_INT`, `ARR_LONG`, `ARR_LONG_LONG`, `ARR_LONG_DOUBLE`, `ARR_SHORT`,
`ARR_STRING`, `ARR_CHAR`, `ARR_DOUBLE`, `ARR_FLOAT`), a mixed-type
container (`ARR_VARIANT`), a custom type (`ARR_CUSTOM`), and a raw,
unchecked type (`ARR_UNSAFE`). For built-in types, the element type is
checked by direct comparison of `ARR_TYPE` enum values
(`arr->type == ARR_INT`), and the correct implementation of an
operation is selected through a function pointer table indexed by that
same value, so dispatch is O(1) with no string comparison involved.
`ARR_VARIANT` arrays use the same dispatch mechanism, but each element
is a full `arr_value` container, so a single array can hold values
created by any `using_*` function side by side. For the custom type,
the type is identified by a string literal (for example `"Point"`)
registered in a global registry, and every operation on a custom array
requires looking up that name with `strcmp`, so custom arrays are
worth using only when the data has no built-in equivalent.
`ARR_UNSAFE` arrays skip type and bounds checking entirely and copy
raw bytes in and out with `memcpy`, trading safety for full control
over the buffer (including manual reallocation).

The array is implemented as a contiguous buffer (`values`) with manual
capacity (`capacity`) and logical length (`length`) tracking. The
library tracks buffer usage on its own and grows the buffer when
elements are added, so growth never needs to be triggered manually.
The buffer never shrinks back on its own; to reclaim memory the array
must be freed entirely (`arr_free`) or reset back to its starting
capacity with `arr_clear` (see "Freeing and clearing memory").

Deleting an element (`arr_delete`) does not shift the following
elements: the cell of the deleted element is filled with zero bytes
and is treated as empty (`null`) when printed. By default this does
not change the logical length, even if the deleted element was the
last one; trimming trailing empty cells from the length is an opt-in
behavior (see "Automatic length trimming").

## File layout

| File / folder | Purpose |
|---|---|
| `array_list_t.h` | Public interface. The only header a user of the library needs to include; it pulls in the built-in, custom, and unsafe headers itself. |
| `array_list_t_accesability/array_list_t_shared.h` | Internal struct definition and internal-only functions, shared across all type implementations. Not meant to be included directly by library users. |
| `array_list_t_lib_messages_handler/` | Internal error-message formatting and reporting used by `has(...)` and by the library internally. |
| `array_list_t_types/base/` | Operations on built-in types. |
| `array_list_t_types/custom/` | Operations on the custom type and the type registry. |
| `array_list_t_types/unsafe/` | Operations on the unsafe type. |

The `array_list_t` struct is opaque: fields can only be accessed
through the getter functions (see "Metadata getters"). Regardless of
the folder split above, `#include "array_list_t.h"` is the only
include a user needs.

## Initialization

```c
void arr_lib_init(void);
```

Fills the dispatch tables for built-in and variant types (sizes, add,
set, equals, print). Must be called before any other library call.

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
    ARR_SIZES_OF_ARRAY_ELEMENT_AND_PROVIDED_ARE_DEFER = -13,
    ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED = -14
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
    ARR_INT,          // 0
    ARR_LONG,         // 1
    ARR_LONG_LONG,    // 2
    ARR_LONG_DOUBLE,  // 3
    ARR_SHORT,        // 4
    ARR_STRING,       // 5
    ARR_CHAR,         // 6
    ARR_DOUBLE,       // 7
    ARR_FLOAT,        // 8
    ARR_VARIANT,      // 9
    ARR_CUSTOM,       // 10
    ARR_NULL_VALUE,   // 11
    ARR_UNSAFE        // 12
} ARR_TYPE;
```

`ARR_INT` through `ARR_VARIANT` are dispatched through function
pointer tables indexed directly by the `ARR_TYPE` value. `ARR_CUSTOM`
is used only for arrays created with `arr_create_custom` /
`arr_create_from_customs`; the actual element type is determined by
the `custom_type` field. `ARR_UNSAFE` is used only for arrays created
with `arr_create_unsafe`; the normal `arr_add`, `arr_set`, and
`arr_equals` functions reject arrays of this type, therefore you need to use the
`arr_unsafe_*` functions instead (see "Unsafe arrays"). `ARR_NULL_VALUE`
is not a valid array type; it only appears as the `type` of an
`arr_value` produced by `using_null()`, used to store an explicit
empty slot in an `ARR_VARIANT` array.

### arr_value

```c
typedef struct arr_value {
    void* custom_value;

    union basic_value {
        int integer;
        char character;
        char* string;
        double double_v;
        float float_v;
        long long_v;
        long long long_long_v;
        long double long_double_v;
        short short_v;
    } basic_value;

    ARR_TYPE type;
    char* custom_type;
    size_t size;
} arr_value;
```

A generic container used to pass a value into add/set functions and to
return a value from get functions. For a built-in type the value lives
inline in the `basic_value` union. `custom_value`, `custom_type` and
`size` are only populated for `ARR_CUSTOM` values (and, for
`custom_value`, as an internal marker for `using_null()`).

## Memory ownership model: arr_value containers

Values are passed into and read from the array through an `arr_value`
wrapper. A container is created by the `using_*` functions:

```c
arr_value using_int(int i);
arr_value using_char(char c);
arr_value using_string(char* s);
arr_value using_double(double d);
arr_value using_float(float f);
arr_value using_long(long l);
arr_value using_long_long(long long ll);
arr_value using_long_double(long double ld);
arr_value using_short(short s);
arr_value using_null(void);
arr_value using_custom(void* value, char* name, size_t size);
```

For built-in types (`using_int` through `using_short`) the value is
stored inline in the `basic_value` union, so no heap allocation happens,
and there is nothing to free for these containers. `using_string`
still stores the pointer `s` itself, not a copy of the string
contents. `using_null` produces a marker container
(`type == ARR_NULL_VALUE`) for storing an explicit empty slot in an
`ARR_VARIANT` array. `using_custom` copies `size` bytes from `value`
into a new heap block and duplicates `name` with `strdup`.

Add and set functions (`arr_add`, `arr_set`, `arr_custom_add`,
`arr_custom_set`) take ownership of the passed-in container. For
built-in-type containers there is nothing to release. For `ARR_CUSTOM`
containers, `arr_custom_add` / `arr_custom_set` release the heap data
themselves via `free_using_container` after copying it into the array.
Freeing a container again, or using it after it has been passed to one
of these functions, is not supported.

Get functions (`arr_get_int`, `arr_custom_get`, etc.) return a new
container. For built-in types there is nothing to release. For
`arr_custom_get`, the returned container owns a heap copy and should
be released with `free_using_container` once no longer needed.

```c
void free_using_container(arr_value av);
```

## Creating an array

### arr_create

```c
array_list_t* arr_create(ARR_TYPE DataType);
```

Creates an empty array of a built-in or `ARR_VARIANT` type. `ARR_CUSTOM`
and `ARR_UNSAFE` are not supported here (use `arr_create_custom` and
`arr_create_unsafe` instead). Returns a pointer to the array or `NULL`
on error.

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
explicitly instead of using the default `ARR_BASIC_CAPACITY` (1024).
Useful when the expected size of the array is known in advance: a
larger `basic_capacity` avoids early reallocations, a smaller one
avoids allocating memory that won't be used. `ARR_CUSTOM` and
`ARR_UNSAFE` are not supported here. Returns a pointer to the array or
`NULL` on error.

### arr_create_custom_greedy

```c
array_list_t* arr_create_custom_greedy(char* generic_name, size_t element_size, int basic_capacity);
```

Same as `arr_create_custom`, but with an explicit initial capacity
instead of the default `ARR_BASIC_CAPACITY`. The type must already be
registered with `arr_custom_register_type`, exactly as for
`arr_create_custom`.

> **Note:** `basic_capacity` sets the *initial* capacity only,
> subsequent growth behavior is unchanged (the buffer still grows
> automatically as elements are added and never shrinks back on its
> own).

### arr_create_from_ints / arr_create_from_chars / arr_create_from_strings / arr_create_from_floats / arr_create_from_doubles / arr_create_from_longs / arr_create_from_long_longs / arr_create_from_long_doubles / arr_create_from_shorts

```c
array_list_t* arr_create_from_ints(int ints[], int len);
array_list_t* arr_create_from_chars(char chars[], int len);
array_list_t* arr_create_from_strings(char* strings[], int len);
array_list_t* arr_create_from_floats(float floats[], int len);
array_list_t* arr_create_from_doubles(double doubles[], int len);
array_list_t* arr_create_from_longs(long longs[], int len);
array_list_t* arr_create_from_long_longs(long long longlongs[], int len);
array_list_t* arr_create_from_long_doubles(long double longdoubles[], int len);
array_list_t* arr_create_from_shorts(short shorts[], int len);
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

The initial capacity of an array is `ARR_BASIC_CAPACITY` (1024),
expressed in elements multiplied by element size: the actual buffer
size in bytes equals `capacity * size_of_one_element`.

When an element is added (`arr_add`, `arr_custom_add`), the array
checks buffer usage on its own and grows the buffer if needed; growth
never needs to be triggered manually. Capacity never shrinks on its
own, neither when elements are deleted nor by any other means. To
reclaim memory, either free the array entirely (`arr_free`) or reset
it back to its starting capacity with `arr_clear` (see "Freeing and
clearing memory"). `ARR_UNSAFE` arrays are the exception: their buffer
can also be resized manually with `arr_unsafe_realloc` (see "Unsafe
arrays").

## Adding, updating and deleting elements

### arr_add

```c
arr_status arr_add(array_list_t* arr, arr_value value);
```

Appends a value to the end of a built-in or `ARR_VARIANT` array. For a
built-in-type array the type of `value` must match the array type; an
`ARR_VARIANT` array accepts a container created by any `using_*`
function. `arr_add` rejects arrays of type `ARR_CUSTOM` or
`ARR_UNSAFE`, use `arr_custom_add` or the `arr_unsafe_add` functions for
those. Returns `ARR_INCONSISTENT_TYPE_PROVIDED`, `ARR_MEMORY_FAULT`, or
`ARR_OK`.

### arr_set

```c
arr_status arr_set(array_list_t* arr, arr_value arr_v, int index);
```

Replaces the value at `index`. Buffer growth is not performed here;
`index` must point to an already occupied position within
`[0, length)`. Same type rules and restrictions as `arr_add`. Returns
`ARR_INCONSISTENT_TYPE_PROVIDED`, `ARR_OUT_OF_BOUNDS`, or `ARR_OK`.

### arr_delete

```c
arr_status arr_delete(array_list_t* arr, size_t index);
```

Deletes the element at `index`: the cell is filled with zero bytes,
other elements are not shifted, and the logical length is not changed
by default (see "Automatic length trimming" to opt into the older
trim-on-delete behavior). Returns `ARR_OUT_OF_BOUNDS`, codes from array
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

## Automatic length trimming

```c
arr_status arr_enable_auto_trim_on_trailing_null(array_list_t* arr);
arr_status arr_disable_auto_trim_on_trailing_null(array_list_t* arr);
```

By default, deleting the last element of an array zeroes its cell but
leaves `length` unchanged. Calling
`arr_enable_auto_trim_on_trailing_null` turns on automatic trimming
for that array: from then on, `length` is trimmed back past trailing
empty cells whenever the array is touched (including on subsequent
deletes). `arr_disable_auto_trim_on_trailing_null` turns this back
off. The setting lives on the array itself and is off by default.

## Iterating and reversing

```c
arr_status arr_for_each(array_list_t* arr, void (*fn)(void* value));
arr_status arr_reverse(array_list_t* arr);
```

`arr_for_each` calls `fn` once for every element in `[0, length)`,
passing a pointer to the raw bytes of that element; the callback is
responsible for interpreting it according to the array's type.
`arr_reverse` reverses the elements of the array in place.

## Reading values

```c
arr_value arr_get_int(array_list_t* arr, int index);
arr_value arr_get_char(array_list_t* arr, int index);
arr_value arr_get_string(array_list_t* arr, int index);
arr_value arr_get_float(array_list_t* arr, int index);
arr_value arr_get_double(array_list_t* arr, int index);
arr_value arr_get_long(array_list_t* arr, int index);
arr_value arr_get_long_long(array_list_t* arr, int index);
arr_value arr_get_long_double(array_list_t* arr, int index);
arr_value arr_get_short(array_list_t* arr, int index);
arr_value arr_custom_get(array_list_t* arr, int index);
```

Return an `arr_value` container holding the value at `index`. On error
an empty container `{0}` is returned (`custom_value == NULL` and
`basic_value` zeroed). The reliable way to detect an error is checking
the array's return status / the getter's preconditions, not the `type`
field, since `ARR_INT` equals `0` and matches the `type` field of an
empty container. `arr_get_string` returns the same pointer that is
stored in the array, without copying the string contents.
`arr_custom_get` returns a copy of the custom-array element. (Reading
from `ARR_VARIANT` arrays is covered separately below.)

## Working with variant arrays (ARR_VARIANT)

An `ARR_VARIANT` array is created like any built-in array
(`arr_create(ARR_VARIANT)` or `arr_create_greedy(ARR_VARIANT, ...)`),
but each element is a full `arr_value`, so a single array can hold
values produced by any `using_*` function side by side, for example
an int next to a string next to a double.

```c
arr_value arr_get_variant(array_list_t* arr, int index);
arr_value* arr_get_variant_reference(array_list_t* arr, int index);
```

`arr_get_variant` returns a copy of the `arr_value` stored at `index`.
`arr_get_variant_reference` instead returns a pointer directly into
the array's buffer, avoiding the copy; the pointer is only valid until
the array is next resized or freed.

To store an explicit empty slot in a variant array (as opposed to
leaving a slot untouched), pass `using_null()`:

```c
has(arr_add(arr, using_null()));
```

`arr_print` on an `ARR_VARIANT` array prints each element according to
its own individual type, and empty/null slots print as `null`.

## Unsafe arrays (ARR_UNSAFE)

`ARR_UNSAFE` arrays skip type checking and bounds checking entirely.
They track `length` like any other array, but reading, writing and
resizing the buffer is otherwise left entirely to the caller.

```c
array_list_t* arr_create_unsafe(int len, int element_size);
array_list_t* arr_unsafe_realloc(array_list_t* arr, int new_len);
arr_status arr_unsafe_add(array_list_t* arr, void* value);
arr_status arr_unsafe_set(array_list_t* arr, void* value, int index);
void* arr_unsafe_get(array_list_t* arr, int index);
arr_status arr_unsafe_delete(array_list_t* arr, int index);
```

`arr_create_unsafe` allocates a buffer for `len` elements of
`element_size` bytes each. `arr_unsafe_realloc` lets the caller resize
that buffer manually, unlike every other array type, an unsafe array
does not grow itself automatically. `arr_unsafe_add` / `arr_unsafe_set`
/ `arr_unsafe_get` / `arr_unsafe_delete` copy raw bytes in and out with
`memcpy` and do not validate the index or the size of `value` against
the array, passing an out-of-range index or a `value` of the wrong
size is undefined behavior. All five functions return
`ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED` if called on an array that
isn't `ARR_UNSAFE`. Conversely, the normal `arr_add`, `arr_set`, and
`arr_equals` reject `ARR_UNSAFE` arrays.

## Comparing arrays

### arr_equals

```c
int arr_equals(array_list_t* arr1, array_list_t* arr2);
```

Compares two built-in-type arrays element by element. Returns `1` if
types, lengths and all values match, `0` otherwise. Not supported for
`ARR_CUSTOM`, `ARR_UNSAFE`, or `ARR_VARIANT` arrays. Use
`arr_custom_equals` for custom arrays, and compare `ARR_VARIANT` or
`ARR_UNSAFE` arrays manually if needed.

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

Prints the elements of a built-in-type or `ARR_VARIANT` array to
`stdout`. Deleted (zeroed) cells are printed as `null`.

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
semantics as a standard `qsort` comparator. For `ARR_STRING` the
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

The registry is a fixed-size array; `MAX_TYPES` is `64`, of which at
most 63 slots can actually be used.

```c
arr_status arr_custom_unregister_type(char* type);
```

Unregisters a type, freeing the memory holding its name and clearing
the registry slot, so the same name can safely be registered again
afterwards.

## Freeing and clearing memory

```c
arr_status arr_free(array_list_t* arr);
```

Frees the `values` buffer, the `custom_type` name (if any), and the
array struct itself. Does not free data pointed to by individual
elements (strings in `ARR_STRING`, nested pointers inside custom
elements), that remains the caller's responsibility.

```c
arr_status arr_clear(array_list_t* arr);
```

Resets the array back to an empty state at its original starting
capacity, without freeing the `array_list_t*` itself: the `values`
buffer is released and reallocated at the capacity the array was
created with, and `length` is reset to `0`. The array pointer remains
valid and can keep being used afterwards. This is also the only way to
reclaim capacity gained through growth without destroying and
recreating the array. If the array is already empty, `arr_clear` does
nothing.

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
#include <stdio.h>
#include <stdlib.h>
#include "../array_list_t.h"

int cmp_int(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

typedef struct { int x, y; } Point;

int point_equals(const void* a, const void* b) {
    const Point* p1 = (const Point*)a;
    const Point* p2 = (const Point*)b;
    return p1->x == p2->x && p1->y == p2->y;
}

int main(void) {
    arr_lib_init();

    array_list_t* nums = arr_create(ARR_INT);
    has(arr_add(nums, using_int(5)));
    has(arr_add(nums, using_int(1)));
    has(arr_add(nums, using_int(3)));

    has(arr_print(nums)); // [5, 1, 3]
    has(arr_sort(nums, cmp_int));
    has(arr_print(nums)); // [1, 3, 5]

    arr_value first = arr_get_int(nums, 0);
    printf("\n%d", first.basic_value.integer); //1

    has(arr_delete(nums, 1)); // [1, null, 5]
    has(arr_print(nums));
    arr_free(nums); 

    has(arr_custom_register_type("Point"));
    has(arr_custom_provide_equals("Point", point_equals));

    array_list_t* points_a = arr_create_custom("Point", sizeof(Point));
    array_list_t* points_b = arr_create_custom("Point", sizeof(Point));

    Point p1 = {1, 2};
    Point p2 = {3, 5};
    has(arr_custom_add(points_a, using_custom(&p1, "Point", sizeof(Point))));
    has(arr_custom_add(points_a, using_custom(&p2, "Point", sizeof(Point))));
    has(arr_custom_add(points_b, using_custom(&p1, "Point", sizeof(Point))));
    has(arr_custom_add(points_b, using_custom(&p2, "Point", sizeof(Point))));

    int equal = arr_custom_equals(points_a, points_b);
    printf("\npoints_a == points_b: %s", equal == 1 ? "true" : "false"); //true
    Point p3a = {3, 5};
    Point p3b = {3, 5};
    has(arr_custom_add(points_b, using_custom(&p3a, "Point", sizeof(Point))));
    has(arr_custom_add(points_b, using_custom(&p3b, "Point", sizeof(Point))));
    int equal2 = arr_custom_equals(points_a, points_b);
    printf("\npoints_a == points_b: %s", equal2 == 1 ? "true" : "false"); //true

    arr_free(points_a);
    arr_free(points_b);

    array_list_t* mixed = arr_create(ARR_VARIANT);
    has(arr_add(mixed, using_int(1)));
    has(arr_add(mixed, using_string("text")));
    has(arr_add(mixed, using_short(7)));
    has(arr_add(mixed, using_null()));

    arr_enable_auto_trim_on_trailing_null(mixed);
    has(arr_set(mixed, using_string("replaced"), 1));
    arr_print(mixed); //[1, replaced, 7]

    arr_reverse(mixed); 
    arr_print(mixed); //[7, replaced, 1]

    has(arr_delete(mixed, arr_get_length(mixed) - 1));
    arr_print(mixed);  // [7, replaced] trailing null trimmed automatically

    arr_disable_auto_trim_on_trailing_null(mixed);
    has(arr_delete(mixed, arr_get_length(mixed) - 1));
    arr_print(mixed);   // trailing null kept this time [7, null]

    arr_value copy = arr_get_variant(mixed, 0);
    arr_print_value(&copy); //7

    arr_value* ref = arr_get_variant_reference(mixed, 0);
    if (ref != NULL) arr_print_value(ref); //7

    arr_free(mixed);

    array_list_t* raw = arr_create_unsafe(4, sizeof(int));
    int v1 = 42;
    int v2 = 42 * 2;
    has(arr_unsafe_add(raw, &v1));
    has(arr_unsafe_add(raw, &v2));
    int* stored = (int*)arr_unsafe_get(raw, 1); 
    printf("\n%d", *stored); //84
    arr_free(raw);

    array_list_t* buffer = arr_create(ARR_INT);
    for (int i = 0; i < 1000; i++) has(arr_add(buffer, using_int(i)));
    arr_clear(buffer); // resets length capacity, and clears all values inside, keeps the pointer valid
    has(arr_add(buffer, using_int(54321)));
    has(arr_add(buffer, using_int(54322)));
    arr_print(buffer); //[54321, 54322]
    arr_free(buffer);  

    return 0;
}
```