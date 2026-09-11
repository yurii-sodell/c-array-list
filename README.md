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
same value, so dispatch is O(1) and does not involve string comparison.
`ARR_VARIANT` arrays use the same dispatch mechanism, but each element
is a full `arr_value` container, so a single array can hold values
created by any `using_*` function side by side. For the custom type,
the type is identified by a string literal (for example `"Point"`)
registered in a global registry. Every operation on a custom array
looks up that name with `strcmp`, so custom arrays are only worth
using when the data has no built-in equivalent.

The array is implemented as a contiguous buffer (`values`) with manual
capacity (`capacity`) and logical length (`length`) tracking. The
library tracks buffer usage on its own and grows the buffer when
elements are added, so growth never needs to be triggered manually.
The buffer never shrinks on its own. To reclaim memory the array must
be freed entirely (`arr_free`) or reset back to its starting capacity
with `arr_clear` (see "Freeing and clearing memory").

Deleting an element (`arr_delete`) does not shift the following
elements: the cell of the deleted element is filled with zero bytes
and is treated as empty (`null`) when printed. By default this does
not change the logical length, even if the deleted element was the
last one. Trimming trailing or inner empty cells from the length is
opt-in (see "Automatic length trimming").

A deleted cell is zeroed rather than removed, and some built-in values
are legitimately zero (an `ARR_INT` holding `0`, or a variant slot
created with `using_int(0)`). Because of this, the raw bytes of a cell
alone cannot tell a real zero value apart from an empty or deleted
slot. To resolve this, every array keeps a parallel presence bitmask
(`bit_mask_of_presence`, one `bool` per slot) alongside the value
buffer. A slot is only treated as `null` for printing and iteration
when its bit in this mask is cleared, either by `arr_delete`, or, in an
`ARR_VARIANT` array, by adding or setting an explicit `using_null()`
value. It is not treated as `null` simply because its stored bytes
happen to be zero. This lets a genuine `0` print as `0` instead of
`null`.

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
through the getter functions (see "Metadata getters"), or, for
`ARR_UNSAFE` arrays, through the direct field-access pointers (see
"Unsafe arrays"). Regardless of the folder split above,
`#include "array_list_t.h"` is the only include a user needs.

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
    ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED = -14,
    ARR_SAFE_OPERATION_ON_UNSAFE_TYPE_CALLED = -15
} arr_status;
```

> **Note:** verify the exact numeric values against your local
> `array_list_t.h`. They are listed here in declaration order, but
> what matters is the symbolic name, not the number.

`ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED` is returned by the
`arr_unsafe_*` functions when called on an array that is not
`ARR_UNSAFE`. `ARR_SAFE_OPERATION_ON_UNSAFE_TYPE_CALLED` is the mirror
case: returned by the "safe" operations (`arr_add`, `arr_set`,
`arr_delete`, `arr_print`, ...) when called on an `ARR_UNSAFE` array
(see "Unsafe arrays").

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
is used only for arrays created with `arr_create_custom` or
`arr_create_from_customs`; the actual element type is determined by
the `custom_type` field. `ARR_UNSAFE` is used only for arrays created
with `arr_create_unsafe`, or an existing array switched into unsafe
mode with `arr_cast_to_unsafe`. The normal `arr_add`, `arr_set`, and
`arr_equals` functions reject arrays of this type, so you need to use
the `arr_unsafe_*` functions instead (see "Unsafe arrays").
`ARR_NULL_VALUE` is not a valid array type; it only appears as the
`type` of an `arr_value` produced by `using_null()`, used to store an
explicit empty slot in an `ARR_VARIANT` array.

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

A generic container used to pass a value into add and set functions
and to return a value from get functions. For a built-in type the
value lives inline in the `basic_value` union. `custom_value`,
`custom_type` and `size` are only populated for `ARR_CUSTOM` values
(and, for `custom_value`, as an internal marker for `using_null()`).

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
stored inline in the `basic_value` union, so no heap allocation
happens, and there is nothing to free for these containers.
`using_string` still stores the pointer `s` itself, not a copy of the
string contents. `using_null` produces a marker container
(`type == ARR_NULL_VALUE`) for storing an explicit empty slot in an
`ARR_VARIANT` array. `using_custom` copies `size` bytes from `value`
into a new heap block and duplicates `name` with `strdup`.

Add and set functions (`arr_add`, `arr_set`, `arr_custom_add`,
`arr_custom_set`) take ownership of the passed-in container. For
built-in-type containers there is nothing to release. For `ARR_CUSTOM`
containers, `arr_custom_add` and `arr_custom_set` release the heap
data themselves via `free_using_container` after copying it into the
array. Freeing a container again, or using it after it has been passed
to one of these functions, is not supported.

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

Creates an empty array of a built-in or `ARR_VARIANT` type.
`ARR_CUSTOM` and `ARR_UNSAFE` are not supported here (use
`arr_create_custom` and `arr_create_unsafe` instead). Returns a
pointer to the array or `NULL` on error.

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
avoids allocating memory that will not be used. `ARR_CUSTOM` and
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

> **Note:** `basic_capacity` sets the *initial* capacity only.
> Subsequent growth behavior is unchanged: the buffer still grows
> automatically as elements are added and never shrinks back on its
> own. It is also remembered on the array as `starting_capacity`,
> which is what `arr_clear` resets the buffer back to.

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
size in bytes equals `capacity * size_of_one_element`. The initial
value is also stored separately as `starting_capacity`, unaffected by
later growth.

When an element is added (`arr_add`, `arr_custom_add`), the array
checks buffer usage on its own and grows the buffer if needed, so
growth never needs to be triggered manually. Capacity never shrinks on
its own, neither when elements are deleted nor by any other means. To
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
function. `arr_add` rejects arrays of type `ARR_CUSTOM` (returning
`ARR_INCONSISTENT_TYPE_PROVIDED`) or `ARR_UNSAFE` (returning
`ARR_SAFE_OPERATION_ON_UNSAFE_TYPE_CALLED`); use `arr_custom_add` or
the `arr_unsafe_add` functions for those. Returns
`ARR_INCONSISTENT_TYPE_PROVIDED`,
`ARR_SAFE_OPERATION_ON_UNSAFE_TYPE_CALLED`, `ARR_MEMORY_FAULT`, or
`ARR_OK`.

### arr_set

```c
arr_status arr_set(array_list_t* arr, arr_value arr_v, int index);
```

Replaces the value at `index`. Buffer growth is not performed here;
`index` must point to an already occupied position within
`[0, length)`. Same type rules and restrictions as `arr_add`,
including returning `ARR_SAFE_OPERATION_ON_UNSAFE_TYPE_CALLED` for
`ARR_UNSAFE` arrays. Returns `ARR_INCONSISTENT_TYPE_PROVIDED`,
`ARR_SAFE_OPERATION_ON_UNSAFE_TYPE_CALLED`, `ARR_OUT_OF_BOUNDS`, or
`ARR_OK`.

### arr_delete

```c
arr_status arr_delete(array_list_t* arr, size_t index);
```

Deletes the element at `index`: the cell is filled with zero bytes,
its bit in the presence bitmask is cleared (see "Overview"), other
elements are not shifted, and the logical length is not changed by
default (see "Automatic length trimming" to opt into trimming trailing
or inner nulls automatically). Rejects `ARR_CUSTOM` (returning
`ARR_INCONSISTENT_TYPE_PROVIDED`) and `ARR_UNSAFE` (returning
`ARR_SAFE_OPERATION_ON_UNSAFE_TYPE_CALLED`) arrays; `ARR_UNSAFE` arrays
have their own `arr_unsafe_delete` instead. Returns
`ARR_OUT_OF_BOUNDS`, codes from array verification, or `ARR_OK`.

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
arr_status arr_enable_auto_trim_on_trailing_nulls(array_list_t* arr);
arr_status arr_disable_auto_trim_on_trailing_nulls(array_list_t* arr);
arr_status arr_enable_auto_trim_on_inner_nulls(array_list_t* arr);
arr_status arr_disable_auto_trim_on_inner_nulls(array_list_t* arr);
```

By default, deleting an element zeroes its cell and marks it absent in
the presence bitmask (see "Overview"), but leaves `length` unchanged
and other elements untouched. Two independent, opt-in behaviors can be
turned on per array:

- `arr_enable_auto_trim_on_trailing_nulls` trims `length` back past
  any trailing empty cells whenever the array is touched (including on
  subsequent deletes), so a deleted last element disappears from the
  printed array instead of showing as `null`.
- `arr_enable_auto_trim_on_inner_nulls` additionally compacts the
  array on every touch: any empty cell, wherever it is, is removed and
  the following elements are shifted left to close the gap, reducing
  `length` accordingly.

Both settings live on the array itself, are independent of one
another, and are off by default. The corresponding
`arr_disable_auto_trim_on_trailing_nulls` and
`arr_disable_auto_trim_on_inner_nulls` functions turn each behavior
back off without affecting the other.

## Iterating and reversing

```c
arr_status arr_for_each(array_list_t* arr, void (*fn)(void* value));
arr_status arr_reverse(array_list_t* arr);
```

`arr_for_each` calls `fn` once for every element in `[0, length)`.
For a slot marked present in the presence bitmask (see "Overview") it
passes a pointer to the raw bytes of that element, which the callback
is responsible for interpreting according to the array's type; for a
slot marked absent (deleted, or an explicit `using_null()` in a
variant array) it passes `NULL` instead, so callbacks should check for
`NULL` before dereferencing. `arr_reverse` reverses the elements of
the array in place.

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
the array's return status or the getter's preconditions, not the
`type` field, since `ARR_INT` equals `0` and matches the `type` field
of an empty container. `arr_get_string` returns the same pointer that
is stored in the array, without copying the string contents.
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
its own individual type, and empty or null slots print as `null`.

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
that buffer manually; unlike every other array type, an unsafe array
does not grow itself automatically. `arr_unsafe_add`, `arr_unsafe_set`,
`arr_unsafe_get` and `arr_unsafe_delete` copy raw bytes in and out
with `memcpy` and do not validate the index or the size of `value`
against the array. Passing an out-of-range index or a `value` of the
wrong size is undefined behavior. All six functions return
`ARR_UNSAFE_OPERATION_ON_SAFE_TYPE_CALLED` (or, for `arr_unsafe_get`,
`NULL`) if called on an array that is not `ARR_UNSAFE`, and
`ARR_LENGTH_IS_CORRUPTED` if the array's `length` has gone negative.
Conversely, the normal `arr_add`, `arr_set`, `arr_delete`, and
`arr_print` reject `ARR_UNSAFE` arrays and return
`ARR_SAFE_OPERATION_ON_UNSAFE_TYPE_CALLED`; `arr_equals` simply
returns `0` for an `ARR_UNSAFE` array instead of a status code.

These checks (`check_unsafe`) only run at the start of each unsafe
call, so they are a best-effort diagnostic, not a safety net. If the
caller corrupts the array's state through direct field access (see
below) in a way `check_unsafe` does not catch, or ignores a `NULL`
returned after a caught corruption, subsequent memory access can still
segfault. This is expected for an API whose entire purpose is to allow
direct, unchecked access to the array's internals.

### Direct field access

Every `ARR_UNSAFE` array also exposes raw pointers into its own struct
fields, for callers that need to read or mutate the array's internal
state directly instead of going through the regular getters:

```c
int*      arr_unsafe_length_ptr(array_list_t* arr);
int*      arr_unsafe_capacity_ptr(array_list_t* arr);
int*      arr_unsafe_starting_capacity_ptr(array_list_t* arr);
int*      arr_unsafe_element_size_ptr(array_list_t* arr);
void*     arr_unsafe_values_ptr(array_list_t* arr);
ARR_TYPE* arr_unsafe_type_ptr(array_list_t* arr);
char*     arr_unsafe_custom_type_ptr(array_list_t* arr);
bool*     arr_unsafe_shrink_on_tail_nulls_ptr(array_list_t* arr);
bool*     arr_unsafe_shrink_on_inner_nulls_ptr(array_list_t* arr);
bool*     arr_unsafe_presence_mask_ptr(array_list_t* arr);
```

Each function first runs the same validity check as the other unsafe
functions (non-`NULL` array, type is `ARR_UNSAFE`, `length` not
already corrupted) and returns `NULL` on failure. On success it
returns a live pointer directly into the array's struct, not a copy;
writing through it mutates the array in place. There is no protection
against putting the array into an inconsistent state this way. For
example, `*arr_unsafe_length_ptr(arr) = -1` corrupts `length`, and
every subsequent operation on that array, safe or unsafe, will then
fail its validity check (and, as noted above, code that ignores a
resulting `NULL` can still crash).

`arr_unsafe_presence_mask_ptr` returns a pointer to the array's `bool`
presence bitmask itself (see "Overview"), letting the caller mark
slots present or absent manually.

### Casting an array to unsafe

```c
arr_status arr_cast_to_unsafe(array_list_t* arr);
```

Changes an existing array's `type` field to `ARR_UNSAFE` in place,
without touching its buffer or element size. It does not allocate a
new array and does not check whether the array's existing content
still makes sense as raw unsafe data. It lets the caller drop any safe
array (built-in, variant, or custom) into unsafe mode to bypass type
checking on it. There is no corresponding function to cast an unsafe
array back to a safe type.

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
`stdout`. Slots marked absent in the presence bitmask (see "Overview")
are printed as `null`; this is independent of the raw bytes stored in
the cell, so a slot legitimately holding `0` still prints as `0`.
Rejects `ARR_CUSTOM` and `ARR_UNSAFE` arrays.

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
elements); that remains the caller's responsibility.

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

int cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }

typedef struct { int x, y; } Point;

int point_equals(const void* a, const void* b) {
    const Point* p1 = (const Point*)a;
    const Point* p2 = (const Point*)b;
    return p1->x == p2->x && p1->y == p2->y;
}

void point_print(const void* point){
    printf("{x: %d, y: %d}", ((Point*) point)->x, ((Point*) point)->y);
}

void print_int(void* integer){
    if (integer != NULL) printf("%d ", *((int*) integer));
    else printf("null ");
}

int main(void) {
    arr_lib_init();

    array_list_t* nums = arr_create(ARR_INT);
    has(arr_add(nums, using_int(5)));
    has(arr_add(nums, using_int(1)));
    has(arr_add(nums, using_int(3)));

    has(arr_print(nums));                  // [5, 1, 3]
    has(arr_sort(nums, cmp_int));
    has(arr_print(nums));                  // [1, 3, 5]

    arr_value first = arr_get_int(nums, 0);
    printf("\n%d", first.basic_value.integer);  // 1

    has(arr_delete(nums, 1));
    has(arr_print(nums));                  // [1, null, 5]

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
    printf("\npoints_a == points_b: %s", equal == 1 ? "true" : "false");  // true

    Point p3a = {3, 5};
    Point p3b = {3, 5};
    has(arr_custom_add(points_b, using_custom(&p3a, "Point", sizeof(Point))));
    has(arr_custom_add(points_b, using_custom(&p3b, "Point", sizeof(Point))));
    int equal2 = arr_custom_equals(points_a, points_b);
    printf("\npoints_a == points_b: %s", equal2 == 1 ? "true" : "false");  // true

    printf("\n\n");
    has(arr_custom_provide_print("Point", point_print));
    has(arr_custom_print(points_a));       // [{x: 1, y: 2}, {x: 3, y: 5}]
    printf("\n");
    // points_a intentionally not freed here, it is reused below to demonstrate
    // calling the wrong (safe) print function on a custom array.
    arr_free(points_b);

    array_list_t* mixed = arr_create(ARR_VARIANT);
    has(arr_add(mixed, using_int(0)));
    has(arr_add(mixed, using_string("text")));
    has(arr_add(mixed, using_short(7)));
    has(arr_add(mixed, using_null()));

    arr_print(mixed);                      // [0, text, 7, null]

    arr_enable_auto_trim_on_trailing_nulls(mixed);
    has(arr_set(mixed, using_string("replaced"), 1));
    arr_print(mixed);                      // [0, replaced, 7]

    arr_reverse(mixed);
    arr_print(mixed);                      // [7, replaced, 0]

    has(arr_delete(mixed, arr_get_length(mixed) - 1));
    arr_print(mixed);                      // [7, replaced], trailing null trimmed automatically

    arr_disable_auto_trim_on_trailing_nulls(mixed);
    has(arr_delete(mixed, arr_get_length(mixed) - 1));
    arr_print(mixed);                      // [7, null], trailing null kept this time

    arr_value copy = arr_get_variant(mixed, 0);
    arr_print_value(&copy);                // 7

    arr_value* ref = arr_get_variant_reference(mixed, 0);
    if (ref != NULL) arr_print_value(ref); // 7
    // mixed intentionally not freed here, it is kept alive for illustration purposes

    array_list_t* raw = arr_create_unsafe(4, sizeof(int));
    int v1 = 42;
    int v2 = 42 * 2;
    has(arr_unsafe_add(raw, &v1));
    has(arr_unsafe_add(raw, &v2));
    int* stored = (int*)arr_unsafe_get(raw, 1);
    printf("\n%d", *stored);               // 84
    arr_free(raw);

    array_list_t* buffer = arr_create(ARR_INT);
    for (int i = 0; i < 1000; i++) has(arr_add(buffer, using_int(i)));
    arr_clear(buffer);  // resets length and capacity, clears all values, keeps the pointer valid

    for (int i = 0; i < 10; i++) arr_add(buffer, using_int(i));

    arr_print(buffer);                     // [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    arr_delete(buffer, 2);
    arr_print(buffer);                     // [0, 1, null, 3, 4, 5, 6, 7, 8, 9]
    arr_delete(buffer, 3);
    arr_print(buffer);                     // [0, 1, null, null, 4, 5, 6, 7, 8, 9]
    arr_enable_auto_trim_on_inner_nulls(buffer);
    arr_print(buffer);                     // [0, 1, 4, 5, 6, 7, 8, 9], nulls compacted, length shrinks
    arr_free(buffer);

    int* x = arr_unsafe_length_ptr(nums);
    // Error: unsafe method was called on safe type. Try to use the method
    // that corresponds the type of array (for example, arr_custom, arr_variant or arr_int)
    if (x == NULL) printf("\nWrong type");  // Wrong type

    has(arr_print(nums));                  // [1, null, 5], still a safe array at this point
    arr_cast_to_unsafe(nums);
    has(arr_print(nums));
    // Error: safe method was called on unsafe type. Try to use unsafe
    // method such (for example arr_unsafe_add, arr_unsafe_get)

    has(arr_print(points_a));
    // Error: Type of the array does not correspond to the type of the provided value.

    printf("\n");
    has(arr_for_each(nums, print_int));    // 1 null 5

    array_list_t* arr = arr_create_unsafe(10, sizeof(Point));
    for (int i = 0; i < 10; i++) {
        Point* p = malloc(sizeof(Point));
        p->x = i;
        p->y = i * 2;
        has(arr_unsafe_add(arr, p));
        free(p);
    }

    Point* found_point_1 = (Point*) arr_unsafe_get(arr, 4);
    printf("\n");
    point_print(found_point_1);            // {x: 4, y: 8}

    int* len = arr_unsafe_length_ptr(arr);
    *len = -1;                             // deliberately corrupt length via the unsafe pointer

    Point* found_point_2 = (Point*) arr_unsafe_get(arr, 5);
    // returns NULL, check_unsafe rejects it and prints:
    // "Error: Length of array is corrupted. Something went terribly wrong!"
    printf("\n");
    point_print(found_point_2);            // segfault, NULL dereferenced, undefined behavior

    return 0;
}
```