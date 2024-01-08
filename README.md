# Yelle's Library of C(++)

This repository is intended to be a holding room where I can upload all of the general-purpose scripts, classes, templates, and such for C++... perhaps to be joined by other languages in the future.

## C++ Contents:

### `array2d.cpp`

Provides a class template for `Array2D<T>`, which is a two-dimensinoal array of objects of type `T`, guaranteed to be rectangular (throwing errors if attempts are made to make it non-rectangular). Nearly all methods (both const and non-const) return a reference to `*this`, meaning that they can be chained together easily.
At the moment, it is implemented as a `std::vector<std::vector<T>>` with lots of methods piled on top of it. However, due to this implementation, and in spite of the many checks I've put in place, a user *can* make the array non-rectangular via directly accessing the sub-vectors; to that end, I've created a `verifyDimensions` function that can be called whenever to double-check, throwing a `std::length_error` on the first row whose length does not match the 0-indexed row's length.

  * Constructors:
    * default: empty array
    * vector-of-vectors / initializer-list-of-initializer-lists: converts directly to a 2D array
    * single vector / single initializer-list: converts into a single-row array
    * file & function, string & function: given a file/string, and a function `char -> T`, reads through the file/string and runs the given function on each `char` in it, constructing a 2D array of `T` type. New rows are assumed delimited by `'\n'`, and `'\r'` is ignored.
  * Access:
    * overloaded `operator[]` and method `at`: gives direct (const and non-const) access to the underlying array
    * `size_at`: if the user wants to access the list not by `i=0..max_i` and `j=0..max_j`, but instead by `k=0..total_number_of_elements`, this is how to do so
    * `safeLook(i,j)`: returns a `std::optional<T>` of the value at `[i,j]`, or a null-optional if the indices `[i,j]` point out-of-range. The value returned in the `optional` is a *copy* of the original, as C++ forbids optionals of references.
  * Attribute Calculation:
    * `empty`: returns `true` if the array has 0 elements
    * `size`: number of elements in the array
    * `rowCount` and `colCount`: vertical & horizontal dimension, respectively
    * `isSquare`: if number of rows == number of columns
    * `to_string`: returns a printable `std::string`; requires the contained type `T` to have `operator<<` overloaded for printing. Columns and rows separated by `\t` and `\n`, respectively.
    * `verifyDimensions`: const void function that throws a `std::length_error` if the array is non-rectangular.
  * Insertion:
    * `insert_row` and `append_row`: can insert via a vector or initializer-list, throwing `std::length_error` if too long, or `std::out_of_range` if `insert_row` is used on an index too big
    * `insert_row_safe` and `append_row_safe`: "safe" versions of the above that either trim or pad the incoming vector/initializer-list so that no exceptions can be thrown
  * Deletion:
    * `clear`: empties the array completely
    * `delete_row` and `delete_rows`: deletes a row or range of rows
    * `delete_col` and `delete_cols`: deletes a column or range of columns
  * Functional Programming:
    * `clone`: creates a deep copy, for use in long chains of methods without altering the original array
    * `map`: given a function of type `T -> S`, applies the function to a copy of the given array, creating a new array of type `Array2D<S>`
    * `map_inplace`: given a function of type `T -> T`, maps the function in-place to each of the elements of the array
      
**TO DO**
  * perhaps move the file/string & function constructors to instead be their own functions, called something like "read_from" or "interpret"
  * add C preprocessor tags
  * implement `sum` and `product` methods to sum/multiply over the whole array (assuming `T` has `operator+` and `operator*` properly overridden)
  * implement `==` and `!=`, probably the former in terms of the latter (assuming `T` has `==` and/or `!=` implemented)
  * extend single vector / single initializer-list constructors to be able to make a single *column* and not just a single row
  * implement `fill`, `fill_row`(s) and `fill_col`(s)
  * implement `insert_fill`
  * implement `insert_col`
  * implement resizing and transposition
  * implement `_safe` versions of the many of the methods above... and perhaps rename all safe methods to *start* with `safe_`
  * add optional boolean argument `verify` (default value `false`) to methods that could non-rectangularize the list
  * add function for applying a function `f(P p, Q q) -> R` to a pair of arrays `Array2D<P>` and `Array2D<Q>` to return a new `Array2D<R>`
