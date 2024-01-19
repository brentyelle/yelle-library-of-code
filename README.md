# Yelle's Library of Code

This repository is intended to be a holding room where I can upload all of the general-purpose scripts, classes, templates, and such for C++... perhaps to be joined by other languages in the future.

## C++ Contents:

### `array2d.cpp`

Provides a class template for `Array2D<T>`, which is a two-dimensinoal array of objects of type `T`, guaranteed to be rectangular (throwing errors if attempts are made to make it non-rectangular). Nearly all methods (both const and non-const) return a reference to `*this`, meaning that they can be chained together easily.
At the moment, it is implemented as a `std::vector<std::vector<T>>` with lots of methods piled on top of it. However, due to this implementation, and in spite of the many checks I've put in place, a user *can* make the array non-rectangular via directly accessing the sub-vectors; to that end, I've created a `verifyDimensions` function that can be called whenever to double-check, throwing a `std::length_error` on the first row whose length does not match the 0-indexed row's length.

  * Constructors:
    * default: empty array
    * vector-of-vectors / initializer-list-of-initializer-lists: converts directly to a 2D array
    * single vector / single initializer-list: converts into a single-row array
  * Builders:
    * file & function, string & function: given a file/string, and a function `char -> T`, reads through the file/string and runs the given function on each `char` in it, constructing a 2D array of `T` type. New rows are assumed delimited by `'\n'`, and `'\r'` is ignored.
    * `fill`: deletes all previous entries, then creates a new array filled with the given value
  * Shape-Changing
    * `resize`: change dimensions, keeping number of elements exactly the same
    * `safe_resize_keeporder`: change dimensions, keeping all elements in order of `begin() -> end()` access if they fit in the new array, all other cells padded
    * `safe_resize_topleft`: change dimensions, keeping all elements at their `(i,j)` indices if still in-bounds, all other cells padded
    * `flip_vertical`, `flip_horizontal`, and `transpose` -- all done in-place
    * `rotate_CW` and `rotate_CCW` -- done by composing flips & transpositions
    * `shift_horizontal`: positive shifts right, negative shifts left
    * `shift_vertical`: positive shifts down, negative shifts up
  * Element Access:
    * overloaded `operator[]` and method `at`: gives direct (const and non-const) access to the underlying array
    * `size_at`: if the user wants to access the list not by `i=0..max_i` and `j=0..max_j`, but instead by `k=0..total_number_of_elements`, this is how to do so
    * `safeLook(i,j)`: returns a `std::optional<T>` of the value at `[i,j]`, or a null-optional if the indices `[i,j]` point out-of-range. The value returned in the `optional` is a *copy* of the original, as C++ forbids optionals of references.
    * total-array iterators: `begin`, `end`, `rbegin`, `rend`, as well as `c`onstant versions of each
    * column iterators: `begin_col`, `end_col`, as well as `c`onstant versions
  * Attribute Calculation:
    * `empty`: returns `true` if the array has 0 elements
    * `size`: number of elements in the array
    * `rowCount` and `colCount`: vertical & horizontal dimension, respectively
    * `isSquare`: if number of rows == number of columns
    * `to_string`: returns a printable `std::string`; requires the contained type `T` to have `operator<<` overloaded for printing. Columns and rows separated by `\t` and `\n`, respectively.
    * `verifyDimensions`: const void function that throws a `std::length_error` if the array is non-rectangular.
  * Insertion:
    * `insert_row` and `append_row`: can insert via a vector or initializer-list, throwing `std::length_error` if too long, or `std::out_of_range` if `insert_row` is used on an index too big
    * `safe_insert_row` and `safe_append_row`: "safe" versions of the above that either trim or pad the incoming vector/initializer-list so that no exceptions can be thrown
    * `insertfill_row`: inserts a new row filled with the given value, only throws if out-of-bounds
    * `safe_insertfill_row`: inserts a new row filled with the given value, never throws
    * `insert_col` and `safe_insert_col`
    * `insertfill_col` and `safe_insertfill_col`
  * Deletion:
    * `clear`: empties the array completely
    * `delete_row` and `delete_rows`: deletes a row or range of rows
    * `delete_col` and `delete_cols`: deletes a column or range of columns
  * Functional Programming:
    * `clone`: creates a deep copy, for use in long chains of methods without altering the original array
    * `map`: given a function of type `T -> S`, applies the function to a copy of the given array, creating a new array of type `Array2D<S>`
    * `map2`: given a function of type `T,T2 -> S`, applies the function to each pair (position-wise) between the self-array and the other-array, returning a new array of type `Array2D<S>`; throws if the two input arrays have different dimensions
    * `map_inplace`: given a function of type `T -> T`, maps the function in-place to each of the elements of the array
    * `foldl` and `foldr` functions, taking the first/last element as the first accumulator, throws if empty
    * `foldl` template for any type accumulator, does not throw if empty
    * `foldl_col`, does `foldl` down a column
    * `sum` and `sum_col`
    * `product` and `product_col`
    * the whole gamut of operators, requiring implementation of the operator on the contained type
      * boolean operators like `==` currently do not create booleans, but rather a new Array2D of the same type as the operands
    * `all` and `any` functions, which take an Array2D and a *"boolifier"* function mapping an element to a boolean; `all` returns true when *all* elements map to true (or there are no elements); `any` returns true when *at least one* element maps to true.
      
**TO DO**
  * extend single vector / single initializer-list constructors to be able to make a single *column* and not just a single row
  * `foldr_col` and templated `foldr`
  * `rbegin_col` and `rend_col` and `c`onstant versions
  * think more about the implementation and usages of operators like `==` that usually return booleans
