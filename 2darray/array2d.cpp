#ifndef YELLE_ARRAY2D
#define YELLE_ARRAY2D

#include <vector>
#include <functional>
#include <algorithm>
#include <optional>
#include <sstream>

namespace yelle {

enum class iterator_direction { FORWARD, REVERSED };

template <class T>
class Array2D {
protected:
    // Holds the data of the 2D array.
    std::vector<std::vector<T>> array;

    // Iterator for looping over the whole list, cell-by-cell, row-by-row.
    // Reverse iterators are set up to behave as Standard Library ones do: that performing ++ on them makes them iterate backwards.
    struct EntireIterator {
        // --- for standard library interoperability (or so I've heard)
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;
        // internals of the iterator
        private:
        const Array2D* const referent;
        pointer held_ptr;
        long curr_row;
        long curr_col;
        const long max_cols;
        const iterator_direction direction;
        
        public:
        // constructor
        EntireIterator (const Array2D *const refrnt, pointer ptr, const size_t colct, iterator_direction dir = iterator_direction::FORWARD)
        : referent(refrnt), held_ptr(ptr), curr_row(0), curr_col(0), max_cols(static_cast<long>(colct)), direction(dir) {
            if (direction == iterator_direction::REVERSED) {
                curr_row = referent->rowCount() - 1;
                curr_col = max_cols - 1;
            }
            return;
        }
        // access by `*` operator
        reference operator*() const {
            return *held_ptr;
        }
        // access by `->` operator
        pointer operator->() {
            return held_ptr;
        }

        // prefix increment
        EntireIterator& operator++() {
            if (direction == iterator_direction::FORWARD) {
                if (curr_col == max_cols - 1) {
                    curr_col = 0;
                    curr_row++;
                }
                else {
                    curr_col++;
                }
            } else {
                if (curr_col == 0) {
                    curr_col = max_cols - 1;
                    curr_row--;
                }
                else {
                    curr_col--;
                }
            }
            held_ptr = const_cast<pointer>(&(referent->array[curr_row][curr_col]));
            return *this;
        }

        // postfix increment
        EntireIterator operator++(int) {
            EntireIterator temp = *this;
            ++(*this);
            return temp;
        }

        // prefix decrement
        EntireIterator& operator--() {
            if (direction == iterator_direction::FORWARD) {
                if (curr_col == 0) {
                    curr_col = max_cols - 1;
                    curr_row--;
                } else {
                    curr_col--;
                }
            } else {
                if (curr_col == max_cols - 1) {
                    curr_col = 0;
                    curr_row++;
                }
                else {
                    curr_col++;
                }
            }
            held_ptr = const_cast<pointer>(&(referent->array[curr_row][curr_col]));
            return *this;
        }

        // postifix decrement
        EntireIterator operator--(int) {
            EntireIterator temp = *this;
            --(*this);
            return temp;
        }

        EntireIterator operator+(const int offset) const {
            EntireIterator newCopy = *this;
            for (int i=0; i < offset; ++i) {
                ++newCopy;
            }
            return newCopy;
        }

        EntireIterator operator-(const int offset) const {
            EntireIterator newCopy = *this;
            for (int i=0; i < offset; ++i) {
                --newCopy;
            }
            return newCopy;
        }

        friend bool operator==(const EntireIterator& a, const EntireIterator& b) {
            return a.held_ptr == b.held_ptr;
        }

        friend bool operator!=(const EntireIterator& a, const EntireIterator& b) {
            return a.held_ptr != b.held_ptr;
        }
    };

    // Iterator for looping over just one vertical column of the array.
    // Reverse iterators are set up to behave as Standard Library ones do: that performing ++ on them makes them iterate backwards (or "upwards" in this case).
    struct ColumnIterator {
        // --- for standard library interoperability (or so I've heard)
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;
        // internals of the iterator
        private:
        pointer held_ptr;
        long curr_row;
        const size_t fixed_col;
        const Array2D* const referent;
        const iterator_direction direction;

        public:
        // constructor
        ColumnIterator(const Array2D *const refrnt, pointer ptr, const size_t col, iterator_direction dir = iterator_direction::FORWARD)
        : referent(refrnt), held_ptr(ptr), curr_row(0), fixed_col(col), direction(dir) {
            curr_row = (direction == iterator_direction::FORWARD ? 0 : referent->rowCount());
            return;
        }

        // access by `*` operator
        reference operator*() const {
            return *held_ptr;
        }
        // access by `->` operator
        pointer operator->() {
            return held_ptr;
        }

        // prefix increment
        ColumnIterator& operator++() {
            curr_row = curr_row + (direction == iterator_direction::FORWARD ? 1 : -1);
            held_ptr = const_cast<pointer>(&(referent->array[curr_row][fixed_col]));
            return *this;
        }

        // postfix increment
        ColumnIterator operator++(int) {
            ColumnIterator temp = *this;
            ++(*this);
            return temp;
        }

        // prefix decrement
        ColumnIterator& operator--() {
            curr_row = curr_row - (direction == iterator_direction::FORWARD ? 1 : -1);
            held_ptr = const_cast<pointer>(&(referent->array[curr_row][fixed_col]));
            return *this;
        }

        // postfix decrement
        ColumnIterator operator--(int) {
            ColumnIterator temp = *this;
            --(*this);
            return temp;
        }

        ColumnIterator operator+(const int offset) const {
            ColumnIterator newCopy = *this;
            for (int i=0; i < offset; ++i) {
                ++newCopy;
            }
            return newCopy;
        }

        ColumnIterator operator-(const int offset) const {
            ColumnIterator newCopy = *this;
            for (int i=0; i < offset; ++i) {
                --newCopy;
            }
            return newCopy;
        }

        friend bool operator==(const ColumnIterator& a, const ColumnIterator& b) {
            return a.held_ptr == b.held_ptr;
        }

        friend bool operator!=(const ColumnIterator& a, const ColumnIterator& b) {
            return a.held_ptr != b.held_ptr;
        }
    };

public:
    // Default constructor: Creates an empty array without any rows.
    Array2D() {
        //fprintf(stdout, "Creating with default constructor!\n");
        return;
    }

    // Double initializer-list constructor: Converts the list of lists into a 2D array.
    Array2D(std::initializer_list<std::initializer_list<T>> list_of_lists) {
        //fprintf(stdout, "Creating with double-initlist constructor!\n");
        for (const std::initializer_list<T>& list : list_of_lists) {
            array.push_back(list);
        }
        verifyDimensions();
        return;
    }

    // Vector-of-Vectors constructor: Copies the values into the `array`.
    explicit Array2D(const std::vector<std::vector<T>>& vec_of_vecs) : array(vec_of_vecs) {
        //fprintf(stdout, "Creating with vec-of-vecs constructor!\n");
        verifyDimensions();
        return;
    }
    // Vector-of-Vectors constructor: Moves the values into the `array`.
    explicit Array2D(std::vector<std::vector<T>>&& vec_of_vecs) : array(vec_of_vecs) {
        //fprintf(stdout, "Creating with vec-of-vecs constructor!\n");
        verifyDimensions();
        return;
    }

    // File-and-function constructor: Given an `fopen`ed input file `infile` and a function from `char` type to the desired parameter type for this Array2D, constructs an Array2D of that type.
    explicit Array2D(FILE* infile, const std::function<T(char)> func) {
        //fprintf(stdout, "Creating with file&func constructor!\n");
        build_from_file(infile, func);
        return;
    }

    // String-and-function constructor: Given a `std::string` and a function from `char` type to the desired parameter type for this Array2D, constructs an Array2D of that type.
    explicit Array2D(const std::string instring, const std::function<T(char)> func) {
        //fprintf(stdout, "Creating with string&func constructor!\n");
        build_from_string(instring, func);
        return;
    }

    /* ============================
        Building/Rebuilding, but Non-Constructor
    ============================= */
    // Returns the array to its original empty state.
    Array2D& clear() {
        array.clear();
        return *this;
    }

    // Empties the array, then fills it as a `newRowCt` x `newColCt` array of `value`s.
    Array2D& fill(const size_t newRowCt, const size_t newColCt, const T& value) {
        array.clear();
        array.reserve(newRowCt);
        for (size_t i=0; i < newRowCt; ++i) {
            array.emplace_back();
            //array[i].reserve(newColCt);
            array[i] = std::move(std::vector(newColCt, value));
        }
        return *this;
    }

    // Given an `fopen`ed input file `infile` and a function from `char` type to the desired parameter type for this Array2D, constructs an Array2D of that type. Row separator is '\n', while EOF marks end of array.
    Array2D& build_from_file(FILE* infile, const std::function<T(char)> func) {
        if (!array.empty()) {
            array.clear();
        }
        // copy each character from the file line-by-line, adding new rows whenever '\n' is encountered
        std::vector<T> currList;
        char currChar = fgetc(infile);
        while (currChar != EOF) {
            if (currChar == '\n') {
                array.push_back(currList);
                currList.clear();
            } else if (currChar == '\r') {
                // do nothing
            } else {
                currList.push_back(func(currChar));
            }
            currChar = fgetc(infile);
        }
        // if last line of the file didn't end with '\n', then we need to add the last line to the Array2D as well
        if (!currList.empty()) {
            array.push_back(currList);
        }

        verifyDimensions();
        return *this;
    }

    // Given a `std::string` and a function from `char` type to the desired parameter type for this Array2D, constructs an Array2D of that type. Row separator is '\n', while EOF marks end of array.
    Array2D& build_from_string(const std::string instring, const std::function<T(char)> func) {
        if (!array.empty()) {
            array.clear();
        }
        // copy each character from the string line-by-line, adding new rows whenever '\n' is encountered
        std::vector<T> currList;
        for (const char currChar : instring) {
            if (currChar == '\n') {
                array.push_back(currList);
                currList.clear();
            } else if (currChar == '\r') {
                // do nothing
            } else {
                currList.push_back(func(currChar));
            }
        }
        // if the string didn't end with '\n', then we need to add the last line to the Array2D as well
        if (!currList.empty()) {
            array.push_back(currList);
        }

        verifyDimensions();
        return *this;
    }

    // Change the dimensions of the array to a new shape. Will throw a `std::length_error` if resizing would cause a change in the number of elements.
    Array2D& resize(const size_t new_max_i, const size_t new_max_j) {
        const size_t area = size();
        if (new_max_i * new_max_j != area) {
            throw std::length_error("resizing would cause change in element count");
        }
        else if (area > 1) {
            Array2D oldself;
            oldself.array.swap(array);
            fill(new_max_i, new_max_j, oldself[0][0]);

            for (size_t k=1; k < area; ++k) {   // we can skip k=0 because it's already copied in the `fill` above
                at_bysize(k) = std::move(oldself.at_bysize(k));
            }
        }

        return *this;
    }

    // Change the dimensions of the array to a new shape, keeping all elements in the order that the begin()...end() iterators would access.
    // If new dimensions are smaller than original, then elements will be trimmed off the end; if larger, then `backup_val` will fill the added cells. 
    Array2D& safe_resize_keeporder(const size_t new_max_i, const size_t new_max_j, const T& backup_val) noexcept {
        const size_t old_area = size();
        const size_t new_area = new_max_i * new_max_j;
        if (new_area == old_area) {
            resize(new_max_i, new_max_j);
        }
        else {
            // move all current elements into "oldself"
            Array2D oldself;
            oldself.array.swap(array);
            // give `this` an array of new elements 
            fill(new_max_i, new_max_j, backup_val);
            // copy everything possible from the original array
            for (size_t k=0; k < std::min<size_t>(old_area, new_area); ++k) {
                at_bysize(k) = std::move(oldself.at_bysize(k));
            }
            // this second for-loop only runs if the new array is bigger than the old one
            for (size_t k=old_area; k < new_area; ++k) {
                at_bysize(k) = backup_val;
            }
        }

        return *this;
    }

    // Change the dimensions of the array to a new shape, keeping all elements in their (row,column) positions, if possible.
    // If new dimension(s) are smaller than original, then elements will be trimmed off; if larger, then `backup_val` will fill the added cells. 
    Array2D& safe_resize_topleft(const size_t new_max_i, const size_t new_max_j, const T& backup_val) noexcept {
        const size_t old_max_i = rowCount();
        const size_t old_max_j = colCount();
        if (old_max_i * old_max_j == new_max_i * new_max_j) {
            resize(new_max_i, new_max_j);
        }
        else {
            // first, resize all the individual rows' lengths
            const size_t old_max_i = rowCount();
            for (size_t i=0; i < old_max_i; ++i) {
                array[i].resize(new_max_j, backup_val);
            }
            // then, trim or add rows as needed
            const long i_difference = static_cast<long>(new_max_i) - static_cast<long>(old_max_i);
            if (i_difference > 0) {
                for (long k=0; k < i_difference; ++k) {
                    array.emplace_back(new_max_j, backup_val);
                }
            } else {
                array.resize(new_max_i);
            }
        }
        // ----- old implementation
        // else {
        //     Array2D oldself;
        //     oldself.array.swap(array);
        //     fill(new_max_i, new_max_j, backup_val);
        //     for (size_t i=0; i < std::min<size_t>(new_max_i, old_max_i); ++i) {
        //         for (size_t j=0; j < std::min<size_t>(new_max_j, old_max_j); ++j) {
        //             array[i][j] = std::move(oldself.array[i][j]);
        //         }
        //     }
        //     oldself.clear();
        // }

        return *this;
    }

    // Reverses every individual row, or equivalently, the element at `(i,j)` is moved to `(i, colCount - j - 1)`.
    Array2D& flip_horizontal() noexcept {
        const size_t row_ct = rowCount();
        for (size_t i=0; i < row_ct; ++i) {
            std::reverse(array[i].begin(), array[i].end());
        }
        return *this;
    }

    // Reverses every individual column, or equivalently, the element at `(i,j)` is moved to `(rowCount - i - 1, j)`.
    Array2D& flip_vertical() noexcept {
        std::reverse(array.begin(), array.end());
        return *this;
    }

    // Transposes the array, moving the element at `(i,j)` to `(j,i)`.
    Array2D& transpose() noexcept {
        size_t old_rowCt = rowCount();
        size_t old_colCt = colCount();
        Array2D oldself;
        oldself.array.swap(array);
        array.clear();
        array.reserve(old_colCt);

        /* --------- option 1 --------- */
        for (size_t i=0; i < old_colCt; ++i) {
            array.emplace_back();
            array[i].reserve(old_rowCt);
            for (size_t j=0; j < old_rowCt; ++j) {
                array[i].push_back(std::move(oldself.array[j][i]));
            }
        }

        oldself.clear();
        return *this;
    }

    // Rotates the array 90° clockwise, equivalent to transposing followed by flipping horizontally.
    Array2D& rotateCW() noexcept {
        transpose();
        flip_horizontal();
        return *this;
    }

    // Rotates the array 90° counter-clockwise, equivalent to transposing followed by flipping vertically.
    Array2D& rotateCCW() noexcept {
        transpose();
        flip_vertical();
        return *this;
    }

    // Shifts the array horizontally so that the element at `[i,j]` is moved to `[i, (j+shift_j) % colCount]`.
    // Elements that spill over are looped around to the other end of the array.
    Array2D& shift_horizontal(long shift_j) noexcept {
        const long row_ct = static_cast<long>(rowCount());
        const long col_ct = static_cast<long>(colCount());

        // negative because std::rotate does +rotation to the left, whereas I want +rotation to the right
        shift_j = -(shift_j % col_ct);
        if (shift_j == 0) {
            return *this;
        }
        if (shift_j < 0) {
            shift_j = col_ct + shift_j;
        }

        for (long i=0; i < row_ct; ++i) {
            fprintf(stderr, "shifting row %li by %li; array size %li,%li\n", i, shift_j, row_ct, col_ct);
            auto startpoint = 
            std::rotate(array[i].begin(), array[i].begin() + static_cast<size_t>(shift_j), array[i].end());
        }

        return *this;
    }

    // Shifts the array horizontally so that the element at `[i,j]` is moved to `[i, j+shift_j]`.
    // Spaces left empty by the shifting are filled with the `filler` value.
    Array2D& shift_horizontal_fill(long shift_j, const T& filler) noexcept {
        const long row_ct = static_cast<long>(rowCount());
        const long col_ct = static_cast<long>(colCount());
        shift_j = shift_j % col_ct;

        shift_horizontal(shift_j);
        const long start_fill_index = (shift_j > 0 ? 0       : col_ct + shift_j );
        const long final_fill_index = (shift_j > 0 ? shift_j : col_ct           );

        for (long i=0; i < row_ct; ++i) {
            for (long j=start_fill_index; j < final_fill_index; ++j) {
                array[i][j] = filler;
            }
        }

        return *this;
    }

    // Shifts the array vertically so that the element at `[i,j]` is moved to `[(i+shift_i) % rowCount, j]`.
    // Elements that spill over are looped around to the other end of the array.
    Array2D& shift_vertical(long shift_i) noexcept {
        const long row_ct = static_cast<long>(rowCount());
        // negative because std::rotate does +rotation to up, whereas I want +rotation down
        shift_i = -(shift_i % row_ct);
        if (shift_i == 0) {
            return *this;
        }
        if (shift_i < 0) {
            shift_i = row_ct + shift_i;
        }

        std::rotate(array.begin(), array.begin() + static_cast<int>(shift_i), array.end());

        return *this;
    }

    // Shifts the array vertically so that the element at `[i,j]` is moved to `[i+shift_i, j]`.
    // Spaces left empty by the shifting are filled with the `filler` value.
    Array2D& shift_vertical_fill(long shift_i, const T& filler) noexcept {
        const long row_ct = static_cast<long>(rowCount());
        const size_t col_ct = colCount();
        shift_i = shift_i % row_ct;

        shift_vertical(shift_i);
        const long start_fill_index = (shift_i > 0 ? 0       : row_ct + shift_i );
        const long final_fill_index = (shift_i > 0 ? shift_i : row_ct           );

        for (long i=start_fill_index; i < final_fill_index; ++i) {
            array[i].clear();
            array[i] = std::move(std::vector(col_ct, filler));
        }

        return *this;
    }

    /* ============================
        Element Access by Index
    ============================= */
    // subscript operator: allows access to elements of `array` directly
    std::vector<T>& operator[](const size_t index) {
        return array[index];
    }
    // `const` subscript operator: allows `const`-access to elements of `array` directly
    const std::vector<T>& operator[](const size_t index) const {
        return array[index];
    }

    // at-method: allows access to the elements of `array` directly, with bounds checking on the row
    std::vector<T>& at(const size_t i, const size_t j) {
        if (i < array.size() && j < array[i].size()) {
            return array[i][j];
        } else {
            throw std::out_of_range("tried to access nonexistent row or column of array");
        }
    }

    // `const` at-method: allows `const`-access to the elements of `array` directly, with bounds checking on the row
    const std::vector<T>& at(const size_t i, const size_t j) const {
        if (i < array.size() && j < array[i].size()) {
            return array[i][j];
        } else {
            throw std::out_of_range("tried to access nonexistent row or column of array");
        }
    }

    // returns a `std::optional` copy of the element stored at `[i,j]`. If there is no element at `[i,j]`, then returns a null optional.
    std::optional<T> safe_look(const size_t i, const size_t j) const noexcept {
        if (i < array.size() && j < array[i].size()) {
            return std::optional<T>(array[i][j]);
        } else {
            return std::optional<T>();
        }
    }

    // Allows access to the elements of `array` directly, but one-dimensionally. For example:
    // ```
    // for (size_t k=0; k < myarray.size(); k++) {
    //     extracted = myarray.at_bysize(k);
    // }```
    T& at_bysize(const size_t index) {
        const size_t cct = colCount();
        return array.at(index / cct).at(index % cct);
    }

    // Allows `const`-access to the elements of `array` directly, but one-dimensionally. For example:
    // ```
    // for (size_t k=0; k < myarray.size(); k++) {
    //     extracted = myarray.at_bysize(k);
    // }```
    const T& at_bysize(const size_t index) const {
        const size_t cct = colCount();
        return array.at(index / cct).at(index % cct);
    }

    /* ============================
        Constant Attributes
    ============================= */
    // Returns `true` if the `array` is empty.
    bool empty() const {
        return (array.empty());
    }

    // Returns the number of elements contained in the 2D array, assuming the array is properly rectangular.
    size_t size() const {
        return rowCount() * colCount();
    }

    // Returns the number of rows in the array, i.e., the number of `vector<T>`s contained in `array`.
    size_t rowCount() const {
        return array.size();
    }

    // Returns the number of elements of the row at index 0, or 0 if there is no index-0 row.
    // Assuming the array is properly rectangular, this is precisely the number of columns in the 2D array.
    size_t colCount() const {
        return (array.empty() ? 0 : array[0].size());
    }

    // Returns `true` if the 2D array has equal counts of rows and columns.
    bool isSquare() const {
        return rowCount() == colCount();
    }

    // Produces a `std::string` representation of the 2D array; elements are separated by `\t`, and rows by `\n`.
    // Requires there to be an overload of `operator<<` for the elements onto a `std::stringstream`.
    std::string to_string() const {
        std::stringstream oss;
        for (size_t i=0; i < rowCount(); ++i) {
            for (size_t j=0; j < colCount(); ++j) {
                oss << array[i][j] << "\t";
            }
            oss << "\n";
        }
        return oss.str();
    }

    // Verifies that the matrix is rectangular, viz. by requiring that every row beyond index 0 have the same `.size()` as the index-0 row.
    // Throws a `std::length_error` if invalid dimensions, returns nothing otherwise.
    void verifyDimensions() const {
        if (array.empty()) {
            //fprintf(stderr, "DEBUG: Called `verifyDimensions` on empty array!\n");
            return;
        } else if (array.size() == 1 && array[0].size() == 0) {
            //fprintf(stderr, "DEBUG: Called `verifyDimensions` on an array with 1 empty row!\n");
            return;
        }
        for (size_t i=1; i < rowCount(); ++i) {
            if (array[i].size() != colCount()) {
                fprintf(stderr, "Rows not of equal length! Row 0 is length %zu, but row %zu is length %zu.\n", colCount(), i, array[i].size());
                throw std::length_error("incompatible row lengths for rectangular 2D array\n");
            }
        }
        return;
    }

    // Verifies that the matrix is rectangular, viz. by requiring that every row beyond index 0 have the same `.size()` as the index-0 row.
    // Returns `true` if valid, `false` if invalid.
    bool safe_verifyDimensions() const noexcept {
        try {
            verifyDimensions();
        } catch (std::length_error& e) {
            return false;
        } 
        // else
        return true;
    }

    /* ============================
        Const Calculations over the Array
    ============================= */
    // Folding from the left. Setting `first_is_accumulator=true` will cause the algorithm to assume that the first element (at `[0,0]`) is already given as the accumulator.
    // `myarray.foldl(f,acc) == f( f( f(acc, a[0]), a[1]), a[2])`
    template <class S>
    S foldl(const std::function<S(S,T)> func, S accumulator, const bool first_is_accumulator=false) const {
        if (empty()) {
            throw std::length_error("cannot use foldl on empty array");
        } else {
            auto iter = cbegin() + static_cast<int>(first_is_accumulator);
            for (; iter != cend(); ++iter) {
                accumulator = func(accumulator, *iter);
            }
            return accumulator;
        }
    }

    // Folding from the left, assuming that the first element of the array (at `[0,0]`) is the starting accumulator. 
    // `myarray.foldl(f) == f( f( f(a[0], a[1]), a[2]), a[3])`
    T foldl(const std::function<T(T,T)> func) const {
        return foldl<T>(func, *cbegin(), true);
    }

    // Folding from the right. Setting `last_is_accumulator=true` will cause the algorithm to assume that the last element (at `[max_i,max_j]`) is already given as the accumulator.
    // `myarray.foldr(f,acc) == f(a[0], f(a[1], f(a[2], acc)))`
    template <class S>
    S foldr(const std::function<S(T,S)> func, S accumulator, const bool last_is_accumulator=false) const {
        if (empty()) {
            throw std::length_error("cannot use foldr on empty array");
        } else {
            auto iter = crbegin() + static_cast<int>(last_is_accumulator);
            for (; iter != crend(); ++iter) {
                accumulator = func(*iter, accumulator);
            }
            return accumulator;
        }
    }

    // Folding from the right, assuming that the first element of the array (at `[max_i,max_j]`) is the starting accumulator. 
    // `myarray.foldr(f) == f(a[0], f(a[1], f(a[2], a[3])))`
    T foldr(const std::function<T(T,T)> func) const {
        return foldr<T>(func, *crbegin(), true);
    }

    // Folding from the left (top) of a column. Setting `first_is_accumulator=true` will cause the algorithm to assume that the first element (at `[0,col_j]`) is already given as the accumulator.
    // `myarray.foldl_col(f,acc) == f( f( f(acc, a[0]), a[1]), a[2])`
    template <class S>
    S foldl_col(const size_t col_j, const std::function<S(S,T)> func, S accumulator, const bool first_is_accumulator=false) const {
        if (empty()) {
            throw std::length_error("cannot use foldl_col on empty array");
        } else if (col_j >= colCount()) {
            throw std::out_of_range("cannot use foldl_col on too high of column index");
        } else {
            auto iter = cbegin_col(col_j) + static_cast<int>(first_is_accumulator);
            for (; iter != cend_col(col_j); ++iter) {
                accumulator = func(accumulator, *iter);
            }
            return accumulator;
        }
    }

    // Folding from the left (top) of a column, assuming that the first element of the array (at `[0,col_j]`) is the starting accumulator. 
    // `myarray.foldl_col(f) == f( f( f(a[0], a[1]), a[2]), a[3])`
    T foldl_col(const size_t col_j, const std::function<T(T,T)> func) const {
        return foldl_col<T>(col_j, func, *cbegin_col(col_j), true);
    }

    // Folding from the right (bottom) of a column. Setting `last_is_accumulator=true` will cause the algorithm to assume that the last element (at `[max_i,col_j]`) is already given as the accumulator.
    // `myarray.foldr_col(f,acc) == f(a[0], f(a[1], f(a[2], acc)))`
    template <class S>
    S foldr_col(const size_t col_j, const std::function<S(T,S)> func, S accumulator, const bool last_is_accumulator=false) const {
        if (empty()) {
            throw std::length_error("cannot use foldr_col on empty array");
        } else if (col_j >= colCount()) {
            throw std::out_of_range("cannot use foldr_col on too high of column index");
        } else {
            auto iter = crbegin_col(col_j) + static_cast<int>(last_is_accumulator);
            for (; iter != crend_col(col_j); ++iter) {
                accumulator = func(*iter, accumulator);
            }
            return accumulator;
        }
    }

    // Folding from the right (bottom) of a column, assuming that the first element of the array (at `[max_i,max_j]`) is the starting accumulator. 
    // `myarray.foldr_col(f) == f(a[0], f(a[1], f(a[2], a[3])))`
    T foldr_col(const size_t col_j, const std::function<T(T,T)> func) const {
        return foldr_col<T>(col_j, func, *crbegin_col(col_j), true);
    }

    // Use `foldl` to sum all elements in the 2D array. Requires that `operator+` be defined for `T + T -> T`.
    T sum() const {
        std::plus<T> T_addition;
        return foldl(T_addition);
    }

    // Use `foldl_col` to sum all elements in a column of the 2D array. Requires that `operator+` be defined for `T + T -> T`.
    T sum_col(const size_t column_j) const {
        std::plus<T> T_addition;
        return foldl_col(column_j, T_addition);
    }

    // Use `foldl` to multiply all elements in the 2D array. Requires that `operator*` be defined for `T * T -> T`.
    T product() const {
        std::multiplies<T> T_multiplication;
        return foldl(T_multiplication);
    }

    // Use `foldl_col` to multiply all elements in a column of the 2D array. Requires that `operator*` be defined for `T * T -> T`.
    T product_col(const size_t column_j) const {
        std::multiplies<T> T_multiplication;
        return foldl_col(column_j, T_multiplication);
    }

    /* ============================
        Adding rows to the array
    ============================= */
    // Insert a vector `new_row` as a new row in the array via the copy constructor.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `rowCount()` are valid.
    // Will throw a `std::length_error` if new row does not have the same length as `colCount()`.
    Array2D& insert_row(const size_t position, const std::vector<T>& new_row) {
        if (position > rowCount()) {
            fprintf(stderr, "Attempted to insert a row at index %zu into an array with only %zu rows!\n", position, rowCount());
            throw std::out_of_range("tried to add row at too-high index value");
        } else if (new_row.size() == colCount() || size() == 0) {
            array.insert(array.begin()+position, new_row);
        } else {
            fprintf(stderr, "Attempted to insert a row of length %zu into an array with row-length %zu!\n", new_row.size(), colCount());
            throw std::length_error("tried to add row of incorrect length");
        }
        //verifyDimensions();
        return *this;
    }

    // Insert a vector `new_row` as a new row in the array via the `std::move` constructor.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `rowCount()` are valid.
    // Will throw a `std::length_error` if new row does not have the same length as `colCount()`.
    Array2D& insert_row(const size_t position, std::vector<T>&& new_row) {
        if (position > rowCount()) {
            fprintf(stderr, "Attempted to insert a row at index %zu into an array with only %zu rows!\n", position, rowCount());
            throw std::out_of_range("tried to add row at too-high index value");
        } else if (new_row.size() == colCount() || size() == 0) {
            array.insert(array.begin()+position, std::move(new_row));
        } else {
            fprintf(stderr, "Attempted to insert a row of length %zu into an array with row-length %zu!\n", new_row.size(), colCount());
            throw std::length_error("tried to add row of incorrect length");
        }
        //verifyDimensions();
        return *this;
    }

    // Insert an initializer list `new_row` as a new row in the array via the value-constructor of the contained type `T`.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `rowCount()` are valid.
    // Will throw a `std::length_error` if new row does not have the same length as `colCount()`.
    Array2D& insert_row(const size_t position, std::initializer_list<T> new_row) {
        if (position > rowCount()) {
            fprintf(stderr, "Attempted to insert a row at index %zu into an array with only %zu rows!\n", position, rowCount());
            throw std::out_of_range("tried to add row at too-high index value");
        } else if (new_row.size() == colCount() || size() == 0) {
            array.insert(array.begin()+position, new_row);
        } else {
            fprintf(stderr, "Attempted to insert a row of length %zu into an array with row-length %zu!\n", new_row.size(), colCount());
            throw std::length_error("tried to add row of incorrect length");
        }
        // verifyDimensions();
        return *this;
    }

    // Insert a vector `new_row` as a new row in the array via the copy constructor.
    // Will trim the row if too long, or pad the row with `backup_val` if too short.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_insert_row(const size_t position, const std::vector<T>& new_row, const T& backup_val) noexcept {
        const size_t col_ct = colCount();
        const size_t newrow_ct = new_row.size();
        if (position > rowCount()) {
            fprintf(stderr, "Attempted to insert row (of length %zu) into row-index %zu of array, but array has only %zu rows... Returning array unchanged.\n", newrow_ct, position, rowCount());
        }
        else if (empty()) {
            array.push_back(new_row);
        }
        else if (newrow_ct >= col_ct) {
            const size_t overshoot_columns = newrow_ct - col_ct;
            array.insert(array.begin()+position, new_row);
            for (size_t k=0; k < overshoot_columns; ++k) {
                array[position].erase(array[position].begin() + col_ct);
            }
        }
        else {
            const size_t missing_columns = col_ct - newrow_ct;
            array.insert(array.begin()+position, new_row);
            for (size_t k=0; k < missing_columns; ++k) {
                array[position].push_back(backup_val);
            }
        }
        //verifyDimensions();
        return *this;
    }

    // Insert a vector `new_row` as a new row in the array via the `std::move` constructor.
    // Will trim the row if too long, or pad the row with `backup_val` if too short.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_insert_row(const size_t position, std::vector<T>&& new_row, const T& backup_val) noexcept {
        const size_t col_ct = colCount();
        const size_t newrow_ct = new_row.size();
        if (position > rowCount()) {
            fprintf(stderr, "Attempted to insert row (of length %zu) into row-index %zu of array, but array has only %zu rows... Returning array unchanged.\n", newrow_ct, position, rowCount());
        }
        else if (empty()) {
            array.push_back(std::move(new_row));
        }
        else if (newrow_ct >= col_ct) {
            const size_t overshoot_columns = newrow_ct - col_ct;
            array.insert(array.begin()+position, std::move(new_row));
            for (size_t k=0; k < overshoot_columns; ++k) {
                array[position].erase(array[position].begin() + col_ct);
            }
        }
        else {
            const size_t missing_columns = col_ct - newrow_ct;
            array.insert(array.begin()+position, std::move(new_row));
            for (size_t k=0; k < missing_columns; ++k) {
                array[position].push_back(backup_val);
            }
        }
        //verifyDimensions();
        return *this;
    }

    // Insert an initializer list `new_row` as a new row in the array via the value-constructor of the contained type `T`.
    // Will trim the row if too long, or pad the row with `backup_val` if too short.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_insert_row(const size_t position, std::initializer_list<T> new_row, const T& backup_val) noexcept {
        const size_t col_ct = colCount();
        const size_t newrow_ct = new_row.size();
        if (position > rowCount()) {
            fprintf(stderr, "Attempted to insert row (of length %zu) into row-index %zu of array, but array has only %zu rows... Returning array unchanged.\n", newrow_ct, position, rowCount());
        }
        else if (empty()) {
            array.emplace_back(new_row);
        }
        else if (newrow_ct >= col_ct) {
            const size_t overshoot_columns = newrow_ct - col_ct;
            array.insert(array.begin()+position, new_row);
            for (size_t k=0; k < overshoot_columns; ++k) {
                array[position].erase(array[position].begin() + col_ct);
            }
        }
        else {
            const size_t missing_columns = col_ct - newrow_ct;
            array.insert(array.begin()+position, new_row);
            for (size_t k=0; k < missing_columns; ++k) {
                array[position].push_back(backup_val);
            }
        }
        //verifyDimensions();
        return *this;
    }

    // Append a vector `new_row` as a new row at the bottom edge of the array via the copy constructor.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `rowCount()` are valid.
    // Will throw a `std::length_error` if new row does not have the same length as `colCount()`.
    Array2D& append_row(const std::vector<T>& new_row) {
        return insert_row(array.size(), new_row);
    }

    // Append a vector `new_row` as a new row at the bottom edge of the array via the `std::move` constructor.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `rowCount()` are valid.
    // Will throw a `std::length_error` if new row does not have the same length as `colCount()`.
    Array2D& append_row(std::vector<T>&& new_row) {
        return insert_row(array.size(), new_row);
    }

    // Append an initializer list `new_row` as a new row at the bottom edge of the array via the value-constructor of the contained type `T`.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `rowCount()` are valid.
    // Will throw a `std::length_error` if new row does not have the same length as `colCount()`.
    Array2D& append_row(std::initializer_list<T> new_row) {
        return insert_row(array.size(), new_row);
    }

    // Append a vector `new_row` as a new row at the bottom edge of the array via the copy constructor.
    // Will trim the row if too long, or pad the row with `backup_val` if too short.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_append_row(const std::vector<T>& new_row, const T& backup_val) {
        return safe_insert_row(array.size(), new_row, backup_val);
    }

    // Append a vector `new_row` as a new row at the bottom edge of the array via the `std::move` constructor.
    // Will trim the row if too long, or pad the row with `backup_val` if too short.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_append_row(std::vector<T>&& new_row, const T& backup_val) {
        return safe_insert_row(array.size(), new_row, backup_val);
    }

    // Append an initializer list `new_row` as a new row at the bottom edge of the array via the value-constructor of the contained type `T`.
    // Will trim the row if too long, or pad the row with `backup_val` if too short.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_append_row(std::initializer_list<T> new_row, const T& backup_val) {
        return safe_insert_row(array.size(), new_row, backup_val);
    }

    // Insert a new row at index `position`, with each element filled as `fillvalue`.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `rowCount()` are valid.
    Array2D& insertfill_row(const size_t position, const T& fillvalue) {
        if (position > rowCount()) {
            fprintf(stderr, "Attempted to insertfill a row at index %zu into an array with only %zu rows!\n", position, rowCount());
            throw std::out_of_range("tried to add row at too-high index value");
        } else {
            array.insert(array.begin()+position, std::vector<T>(colCount(), fillvalue));
        }
        return *this;
    }

    // Insert a new row at index `position`, with each element filled as `fillvalue`.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_insertfill_row(const size_t position, const T& fillvalue) {
        if (position > rowCount()) {
            fprintf(stderr, "Attempted to insertfill a row at index %zu into an array with only %zu rows!\nReturning array unchanged.\n", position, array.size());
        } else {
            array.insert(array.begin()+position, std::move(std::vector<T>(colCount(), fillvalue)));
        }
        return *this;
    }

    /* ============================
        Adding columns to the array
    ============================= */
    // Insert elements from the vector `new_col` as a new column in the array via the copy constructor.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `colCount()` are valid.
    // Will throw a `std::length_error` if new row does not have the same length as `rowCount()`.
    Array2D& insert_col(const size_t position, const std::vector<T>& new_col) {
        const size_t row_ct = rowCount();
        
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insert a column at index %zu into an array with only %zu columns!\n", position, colCount());
            throw std::out_of_range("tried to add column at too-high index value");
        }
        else if (row_ct == 0) {
            for (size_t i=0; i < new_col.size(); ++i) {
                array.emplace_back();
                array[i].push_back(new_col[i]);
            }
        }
        else if (new_col.size() == row_ct) {
            for (size_t i=0; i < row_ct; ++i) {
                array[i].insert(array[i].begin()+position, new_col[i]);
            }
        }
        else {
            fprintf(stderr, "Attempted to insert a column of length %zu into an array with column-length %zu!\n", new_col.size(), row_ct);
            throw std::length_error("tried to add column of incorrect length");
        }
        // verifyDimensions();
        return *this;
    }

    // Insert elements from the vector `new_col` as a new column in the array via the `std::move` constructor.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `colCount()` are valid.
    // Will throw a `std::length_error` if new row does not have the same length as `rowCount()`.
    Array2D& insert_col(const size_t position, const std::vector<T>&& new_col) {
        const size_t row_ct = rowCount();
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insert a column at index %zu into an array with only %zu columns!\n", position, colCount());
            throw std::out_of_range("tried to add column at too-high index value");
        }
        else if (row_ct == 0) {
            for (size_t i=0; i < new_col.size(); ++i) {
                array.emplace_back();
                array[i].push_back(std::move(new_col[i]));
            }
        }
        else if (new_col.size() == row_ct) {
            for (size_t i=0; i < row_ct; ++i) {
                array[i].insert(array[i].begin()+position, std::move(new_col[i]));
            }
        }
        else {
            fprintf(stderr, "Attempted to insert a column of length %zu into an array with column-length %zu!\n", new_col.size(), row_ct);
            throw std::length_error("tried to add column of incorrect length");
        }
        // verifyDimensions();
        return *this;
    }

    // Insert elements from the initializer list `new_col` as a new column in the array via the value-constructor of the contained type `T`.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `colCount()` are valid.
    // Will throw a `std::length_error` if new row does not have the same length as `rowCount()`.
    Array2D& insert_col(const size_t position, std::initializer_list<T> new_col) {
        const size_t row_ct = rowCount();
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insert a column at index %zu into an array with only %zu columns!\n", position, colCount());
            throw std::out_of_range("tried to add column at too-high index value");
        }
        else if (row_ct == 0) {
            for (size_t i=0; i < new_col.size(); ++i) {
                array.emplace_back();
                array[i].push_back(*(new_col.begin()+i));
            }
        }
        else if (new_col.size() == row_ct) {
            for (size_t i=0; i < row_ct; ++i) {
                array[i].insert(array[i].begin() + position, *(new_col.begin()+i));
            }
        }
        else {
            fprintf(stderr, "Attempted to insert a column of length %zu into an array with column-length %zu!\n", new_col.size(), row_ct);
            throw std::length_error("tried to add column of incorrect length");
        }
        // verifyDimensions();
        return *this;
    }

    // Insert elements from the vector `new_col` as a new column in the array via the copy constructor.
    // Will trim the column if too long, or pad the column with `backup_val` if too short.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_insert_col(const size_t position, const std::vector<T>& new_col, const T& backup_val) {
        const size_t row_ct = rowCount();
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insert a column at index %zu into an array with only %zu columns... Returning array unchanged.\n", position, colCount());
        }
        else if (row_ct == 0) {
            insert_col(position, new_col);
        }
        else {
            for (size_t i=0; i < row_ct; ++i) {
                if (i < new_col.size()) {
                    array[i].insert(array[i].begin() + position, new_col[i]);
                }
                else {
                    array[i].insert(array[i].begin() + position, backup_val);
                }
            }
        }

        // verifyDimensions();
        return *this;
    }

    // Insert elements from the vector `new_col` as a new column in the array via the `std::move` constructor.
    // Will trim the column if too long, or pad the column with `backup_val` if too short.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_insert_col(const size_t position, std::vector<T>&& new_col, const T& backup_val) {
        const size_t row_ct = rowCount();
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insert a column at index %zu into an array with only %zu columns... Returning array unchanged.\n", position, colCount());
        }
        else if (row_ct == 0) {
            insert_col(position, std::move(new_col));
        }
        else {
            for (size_t i=0; i < row_ct; ++i) {
                if (i < new_col.size()) {
                    array[i].insert(array[i].begin() + position, std::move(new_col[i]));
                }
                else {
                    array[i].insert(array[i].begin() + position, backup_val);
                }
            }
        }

        // verifyDimensions();
        return *this;
    }

    // Insert elements from the initializer list `new_col` as a new column in the array via the value-constructor of the contained type `T`.
    // Will trim the column if too long, or pad the column with `backup_val` if too short.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_insert_col(const size_t position, std::initializer_list<T> new_col, const T& backup_val) {
        const size_t row_ct = rowCount();
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insert a column at index %zu into an array with only %zu columns... Returning array unchanged.\n", position, colCount());
        }
        else if (row_ct == 0) {
            insert_col(position, new_col);
        }
        else {
            for (size_t i=0; i < row_ct; ++i) {
                if (i < new_col.size()) {
                    array[i].insert(array[i].begin() + position, *(new_col.begin()+i));
                }
                else {
                    array[i].insert(array[i].begin() + position, backup_val);
                }
            }
        }

        // verifyDimensions();
        return *this;
    }

    // Append elements from the vector `new_col` as a new column at the right edge of the array via the copy constructor.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `colCount()` are valid.
    // Will throw a `std::length_error` if new row does not have the same length as `rowCount()`.
    Array2D& append_col(const std::vector<T>& new_col) {
        return insert_col(colCount(), new_col);
    }

    // Append elements from the vector `new_col` as a new column at the right edge of the array via the `std::move` constructor.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `colCount()` are valid.
    // Will throw a `std::length_error` if new row does not have the same length as `rowCount()`.
    Array2D& append_col(std::vector<T>&& new_col) {
        return insert_col(colCount(), std::move(new_col));
    }

    // Append elements from the initializer list `new_col` as a new column at the right edge of the array via the value-constructor of the contained type `T`.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `colCount()` are valid.
    // Will throw a `std::length_error` if new row does not have the same length as `rowCount()`.
    Array2D& append_col(std::initializer_list<T> new_col) {
        return insert_col(colCount(), new_col);
    }

    // Append elements from the vector `new_col` as a new column at the right edge of the array via the copy constructor.
    // Will trim the column if too long, or pad the column with `backup_val` if too short.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_append_col(const std::vector<T>& new_col, const T& backup_val) {
        return safe_insert_col(colCount(), new_col, backup_val);
    }

    // Append elements from the vector `new_col` as a new column at the right edge of the array via the `std::move` constructor.
    // Will trim the column if too long, or pad the column with `backup_val` if too short.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_append_col(std::vector<T>&& new_col, const T& backup_val) {
        return safe_insert_col(colCount(), std::move(new_col), backup_val);
    }

    // Append elements from the initializer list `new_col` as a new column at the right edge of the array via the value-constructor of the contained type `T`.
    // Will trim the column if too long, or pad the column with `backup_val` if too short.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_append_col(std::initializer_list<T> new_col, const T& backup_val) {
        return safe_insert_col(colCount(), new_col, backup_val);
    }

    // Insert a new column at index `position`, with each element filled as `fillvalue`.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `colCount()` are valid.
    Array2D& insertfill_col(const size_t position, const T& backup_val) {
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insertfill a column at index %zu into an array with only %zu columns!\n", position, colCount());
            throw std::out_of_range("tried to add column at too-high index value");
        }
        else {
            return safe_insert_col(position, std::initializer_list<T>(), backup_val);
        }
    }

    // Insert a new column at index `position`, with each element filled as `fillvalue`.
    // If insertion is attempted out-of-range, a message is printed to `stderr`, and the array is returned unchanged.
    Array2D& safe_insertfill_col(const size_t position, const T& backup_val) {
        return safe_insert_col(position, std::initializer_list<T>(), backup_val);
    }

    /* ============================
        Deletion
    ============================= */

    // Delete a single row at row-index `i`.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `rowCount()-1` are valid.
    Array2D& delete_row(const size_t i) {
        if (i >= rowCount()) {
            fprintf(stderr, "Tried to delete row %zu from an Array2D with %zu rows!\n", i, rowCount());
            throw std::out_of_range("cannot delete row outside of valid range");
        } else {
            array.erase(array.cbegin()+i, array.cbegin()+i+1);
            return *this;
        }
    }

    // Delete all rows in the range `[i_start, i_past_end)`. Note that `i_past_end` is excluded.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `rowCount()-1` are valid for `i_start`, and only indices from `0` to `rowCount()` for `i_past_end`.
    // If indices are valid, but `i_start >= i_past_end`, then silently no deletion occurs.
    Array2D& delete_rows(const size_t i_start, const size_t i_past_end) {
        if (i_start >= rowCount() || i_past_end > rowCount()) {
            fprintf(stderr, "Tried to delete rows [%zu,%zu) from an Array2D with %zu rows!\n", i_start, i_past_end, rowCount());
            throw std::out_of_range("cannot delete rows outside of valid range");
        }
        else if (i_start < i_past_end) {
            array.erase(array.cbegin()+i_start, array.cbegin()+i_past_end);
        }
        // else {
        //     // do nothing
        // }
        return *this;
    }

    // Delete a single column at column-index `i`.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `colCount()-1` are valid.
    Array2D& delete_col(const size_t j) {
        if (j >= colCount()) {
            fprintf(stderr, "Tried to delete column %zu from an Array2D with %zu columns!\n", j, colCount());
            throw std::out_of_range("cannot delete column outside of valid range");
        } else {
            const size_t row_ct = rowCount();
            // would a for-each loop work here?
            for (size_t i=0; i < row_ct; ++i) {
                std::vector<T>& thisRow = array[i];
                thisRow.erase(thisRow.cbegin()+j, thisRow.cbegin()+j+1);
            }
            return *this;
        }
    }

    // Delete all rows in the range `[j_start, j_past_end)`. Note that `j_past_end` is excluded.
    // Will throw a `std::out_of_range` error if at an invalid position--only indices from `0` to `colCount()-1` are valid for `j_start`, and only indices from `0` to `colCount()` for `j_past_end`.
    // If indices are valid, but `j_start >= j_past_end`, then silently no deletion occurs.
    Array2D& delete_cols(const size_t j_start, const size_t j_past_end) {
        if (j_start >= colCount() || j_past_end > colCount()) {
            fprintf(stderr, "Tried to delete columns [%zu,%zu) from an Array2D with %zu columns!\n", j_start, j_past_end, colCount());
            throw std::out_of_range("cannot delete columns outside of valid range");
        }
        else if (j_start < j_past_end) {
            const size_t row_ct = rowCount();
            // would a for-each loop work here?
            for (size_t i=0; i < row_ct; ++i) {
                std::vector<T>& thisRow = array[i];
                thisRow.erase(thisRow.cbegin()+j_start, thisRow.cbegin()+j_past_end);
            }
        }
        // else {
        //     // do nothing
        // }
        return *this;
    }

    /* ============================
        Full Array Access by Iterator
            These iterators are invalid (pointing to NULL) if the array is empty.
    ============================= */

    // Forward iterator, points to the element at index (0,0).
    // Incrementing this iterator goes to the next element of the same row, or--if already at the end of the row--to position 0 in the next row.
    EntireIterator begin() {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0, iterator_direction::FORWARD);
        } else {
            return EntireIterator(this, &(array[0][0]), colCount(), iterator_direction::FORWARD);
        }
    }

    // Constant forward iterator, points to the element at index (0,0).
    // Incrementing this iterator goes to the next element of the same row, or--if already at the end of the row--to position 0 in the next row.
    const EntireIterator cbegin() const {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0, iterator_direction::FORWARD);
        } else {
            return EntireIterator(this, const_cast<T*>(&(array[0][0])), colCount(), iterator_direction::FORWARD);
        }
    }

    // Forward iterator end, points to the nonexistent element at index `[rowCount,0]` just beyond the end of the array.
    // Equal to `begin() + size()`.
    EntireIterator end() {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0, iterator_direction::FORWARD);
        } else {
            return EntireIterator(this, &(array[rowCount()][0]), colCount(), iterator_direction::FORWARD);
        }
    }

    // Constant forward iterator end, points to the nonexistent element at index `[rowCount,0]` just beyond the end of the array.
    // Equal to `cbegin() + size()`.
    const EntireIterator cend() const {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0, iterator_direction::FORWARD);
        } else {
            return EntireIterator(this, const_cast<T*>(&(array[rowCount()][0])), colCount(), iterator_direction::FORWARD);
        }
    }

    // Backward iterator, points to the final element, at index `[rowCount-1,colCount-1]`.
    // Incrementing this iterator goes to the previous element of the row, or--if already at the start of the row--to position `colCount` in the previous row.
    EntireIterator rbegin() const {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0, iterator_direction::REVERSED);
        } else {
            return EntireIterator(this, &(array[rowCount()-1][colCount()-1]), colCount(), iterator_direction::REVERSED);
        }
    }

    // Constant backward iterator, points to the final element, at index `[rowCount-1,colCount-1]`.
    // Incrementing this iterator goes to the previous element of the row, or--if already at the start of the row--to position `colCount` in the previous row.
    const EntireIterator crbegin() const {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0, iterator_direction::REVERSED);
        } else {
            return EntireIterator(this, const_cast<T*>(&(array[rowCount()-1][colCount()-1])), colCount(), iterator_direction::REVERSED);
        }
    }

    // Backward iterator end, points to the nonexistent element at index `[-1,colCount-1]`.
    // Equal to `rbegin() + size()`.
    EntireIterator rend() const {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0, iterator_direction::REVERSED);
        } else {
            return EntireIterator(this, &(array[-1][colCount()-1]), colCount(), iterator_direction::REVERSED);
        }
    }

    // Backward iterator end, points to the nonexistent element at index `[-1,colCount-1]`.
    // Equal to `crbegin() + size()`.
    const EntireIterator crend() const {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0, iterator_direction::REVERSED);
        } else {
            return EntireIterator(this, const_cast<T*>(&(array[-1][colCount()-1])), colCount(), iterator_direction::REVERSED);
        }
    }

    /* ============================
        Column Access by Iterator
    ============================= */

    // Forward iterator for column access, points to the element at index `[0,fixed_col_j]`.
    // Incrementing this iterator goes to the next element of the same column.
    ColumnIterator begin_col(const size_t fixed_col_j) {
        if (fixed_col_j >= colCount()) {
            throw std::out_of_range("column index out of range");
        } else {
            return ColumnIterator(this, &(array[0][fixed_col_j]), fixed_col_j, iterator_direction::FORWARD);
        }
    }

    // Constant forward iterator for column access, points to the element at index `[0,fixed_col_j]`.
    // Incrementing this iterator goes to the next element of the same column.
    const ColumnIterator cbegin_col(const size_t fixed_col_j) const {
        if (fixed_col_j >= colCount()) {
            throw std::out_of_range("column index out of range");
        } else {
            return ColumnIterator(this, const_cast<T*>(&(array[0][fixed_col_j])), fixed_col_j, iterator_direction::FORWARD);
        }
    }

    // Forward iterator end for column access, points to the nonexistent element at index `[rowCount,fixed_col_j]`.
    // Equivalent to `begin_col(fixed_col_j) + rowCount()`.
    ColumnIterator end_col(const size_t fixed_col_j) {
        if (fixed_col_j >= colCount()) {
            throw std::out_of_range("column index out of range");
        } else {
            return ColumnIterator(this, &(array[rowCount()][fixed_col_j]), fixed_col_j, iterator_direction::FORWARD);
        }
    }

    // Constant forward iterator end for column access, points to the nonexistent element at index `[rowCount,fixed_col_j]`.
    // Equivalent to `cbegin_col(fixed_col_j) + rowCount()`.
    const ColumnIterator cend_col(const size_t fixed_col_j) const {
        if (fixed_col_j >= colCount()) {
            throw std::out_of_range("column index out of range");
        } else {
            return ColumnIterator(this, const_cast<T*>(&(array[rowCount()][fixed_col_j])), fixed_col_j, iterator_direction::FORWARD);
        }
    }

    // Backward iterator for column access, points to the element at index `[rowCount-1,fixed_col_j]`.
    // Incrementing this iterator goes to the previous element of the same column.
    ColumnIterator rbegin_col(const size_t fixed_col_j) {
        if (fixed_col_j >= colCount()) {
            throw std::out_of_range("column index out of range");
        } else {
            return ColumnIterator(this, &(array[rowCount()-1][fixed_col_j]), fixed_col_j, iterator_direction::REVERSED);
        }
    }

    // Constant backward iterator for column access, points to the element at index `[rowCount-1,fixed_col_j]`.
    // Incrementing this iterator goes to the previous element of the same column.
    const ColumnIterator crbegin_col(const size_t fixed_col_j) const {
        if (fixed_col_j >= colCount()) {
            throw std::out_of_range("column index out of range");
        } else {
            return ColumnIterator(this, const_cast<T*>(&(array[rowCount()-1][fixed_col_j])), fixed_col_j, iterator_direction::REVERSED);
        }
    }

    // Backward iterator end for column access, points to the nonexistent element at index `[-1,fixed_col_j]`.
    // Equal to `rbegin_col(fixed_col_j) + rowCount()`.
    ColumnIterator rend_col(const size_t fixed_col_j) {
        if (fixed_col_j >= colCount()) {
            throw std::out_of_range("column index out of range");
        } else {
            return ColumnIterator(this, &(array[-1][fixed_col_j]), fixed_col_j, iterator_direction::REVERSED);
        }
    }

    // Backward iterator end for column access, points to the nonexistent element at index `[-1,fixed_col_j]`.
    // Equal to `crbegin_col(fixed_col_j) + rowCount()`.
    const ColumnIterator crend_col(const size_t fixed_col_j) const {
        if (fixed_col_j >= colCount()) {
            throw std::out_of_range("column index out of range");
        } else {
            return ColumnIterator(this, const_cast<T*>(&(array[-1][fixed_col_j])), fixed_col_j, iterator_direction::REVERSED);
        }
    }

    /* ============================
        Functional Programming
    ============================= */

    // Creates a deep copy of this Array2D, for usage in method-chaining.
    Array2D clone() const {
        return Array2D(*this);
    }

    // Map a unary function onto this Array2D, returning a new array and leaving the original unchanged.
    // Given a lambda function or function pointer from type `T` to `S`, creates and returns a new `Array2D` of type `S` whose elements are equal to `func(t)` for each corresponding `t` in the input array.
    template <class S>
    Array2D<S> map(const std::function<S(T)>& func) const {
        Array2D<S> outArray;
        std::vector<S> temp_row;

        for (size_t i=0; i < rowCount(); ++i) {
            temp_row.clear();
            for (size_t j=0; j < colCount(); ++j) {
                temp_row.push_back(func(array[i][j]));
            }
            outArray.append_row(std::move(temp_row));
        }
        return outArray;
    }

    // Map a binary function onto this Array2D<T> and one other Array2D<T2>, returning a new array and leaving the original unchanged.
    // Given a lambda function or function pointer from type `(T,T2)` to `S`, creates and returns a new `Array2D` of type `S` whose elements are equal to `func(t,t2)` for each corresponding `t,t2` in the input arrays.
    template <class S, class T2>
    Array2D<S> map2(const std::function<S(T,T2)&> func, const Array2D<T2>& arr2) const {
        Array2D<S> outArray;
        std::vector<S> temp_row;

        for (size_t i=0; i < rowCount(); ++i) {
            temp_row.clear();
            for (size_t j=0; j < colCount(); ++j) {
                temp_row.push_back(func(array[i][j], arr2.array[i][j]));
            }
            outArray.append_row(std::move(temp_row));
        }
        return outArray;
    }

    // Map a unary function of type `T -> T` onto this Array2D, changing each value in-place.
    // Given a lambda function or function pointer from type `T` to `T`, applies the function in-place to each element of this array.
    Array2D& map_inplace(const std::function<T(T)>& func) {
        // tiny optimization, since this isn't a const method
        const size_t row_ct = rowCount();
        const size_t col_ct = colCount();

        for (size_t i=0; i < row_ct; ++i) {
            for (size_t j=0; j < col_ct; ++j) {
                array[i][j] = func(array[i][j]);
            }
        }
        return *this;
    }

    /* ============================
        Unary Operators
    ============================= */

protected:
    // Boilerplate code for implementing unary operators.
    Array2D impl_operator1(const std::function<T(const T&)> op) const {
        const size_t row_ct = rowCount();
        const size_t col_ct = colCount();
        Array2D new2D;
        new2D.array.reserve(row_ct);
        for (size_t i=0; i < row_ct; ++i) {
            new2D.array.emplace_back();
            new2D.array[i].reserve(col_ct);
            for (size_t j=0; j < col_ct; ++j) {
                new2D.array[i].push_back(op(array[i][j]));
            }
        }
        return new2D;
    }

public:
    Array2D operator!() const {
        std::logical_not<T> op;
        return impl_operator1(op);
    }

    Array2D operator-() const {
        std::negate<T> op;
        return impl_operator1(op);
    }

    /* ============================
        Binary Operators
    ============================= */
    // Boilerplate code for implementing binary operators.
    // Allows a binary operator to work with Array2D + Array2D, etc.
    Array2D impl_operator2(const std::function<T(const T&, const T&)> op, const Array2D& other) const {
        const size_t row_ct = rowCount();
        const size_t col_ct = colCount();
        if (row_ct != other.rowCount() || col_ct != other.colCount()) {
            throw std::length_error("called boolean operator on Array2D objects of unequal dimension");
        }
        Array2D new2D;
        new2D.array.reserve(row_ct);
        for (size_t i=0; i < row_ct; ++i) {
            new2D.array.emplace_back();
            new2D.array[i].reserve(col_ct);
            for (size_t j=0; j < col_ct; ++j) {
                new2D.array[i].push_back(op(array[i][j], other.array[i][j]));
            }
        }

        return new2D;
    }

    // Boilerplate code for implementing binary operators.
    // Allows a binary operator to work with Array2D + T, etc.
    Array2D impl_operator2(const std::function<T(const T&, const T&)> op, const T& other) const {
        const size_t row_ct = rowCount();
        const size_t col_ct = colCount();

        Array2D new2D;
        new2D.array.reserve(row_ct);
        for (size_t i=0; i < row_ct; ++i) {
            new2D.array.emplace_back();
            new2D.array[i].reserve(col_ct);
            for (size_t j=0; j < col_ct; ++j) {
                new2D.array[i].push_back(op(array[i][j], other));
            }
        }

        return new2D;
    }

    /* ----- OPERATORS ON OTHER ARRAYS ----- */

    // Array2D operator==(const Array2D& other) const {
    //     std::equal_to<T> op;
    //     return impl_operator2(op, other);
    // }
    // Array2D operator!=(const Array2D& other) const {
    //     std::not_equal_to<T> op;
    //     return impl_operator2(op, other);
    // }
    // Array2D operator<(const Array2D& other) const {
    //     std::less<T> op;
    //     return impl_operator2(op, other);
    // }
    // Array2D operator<=(const Array2D& other) const {
    //     std::less_equal<T> op;
    //     return impl_operator2(op, other);
    // }
    // Array2D operator>(const Array2D& other) const {
    //     std::greater<T> op;
    //     return impl_operator2(op, other);
    // }
    // Array2D operator>=(const Array2D& other) const {
    //     std::greater_equal<T> op;
    //     return impl_operator2(op, other);
    // }
    // Array2D operator&&(const Array2D& other) const {
    //     std::logical_and<T> op;
    //     return impl_operator2(op, other);
    // }
    // Array2D operator||(const Array2D& other) const {
    //     std::logical_or<T> op;
    //     return impl_operator2(op, other);
    // }

    Array2D operator+(const Array2D& other) const {
        std::plus<T> op;
        return impl_operator2(op, other);
    }
    Array2D operator-(const Array2D& other) const {
        std::minus<T> op;
        return impl_operator2(op, other);
    }
    Array2D operator*(const Array2D& other) const {
        std::multiplies<T> op;
        return impl_operator2(op, other);
    }
    Array2D operator/(const Array2D& other) const {
        std::divides<T> op;
        return impl_operator2(op, other);
    }
    Array2D operator%(const Array2D& other) const {
        std::modulus<T> op;
        return impl_operator2(op, other);
    }

    /* ----- OPERATORS ON SINGLE <T>-TYPES ----- */

    // Array2D operator==(const T& other_val) const {
    //     std::equal_to<T> op;
    //     return impl_operator2(op, other_val);
    // }
    // Array2D operator!=(const T& other_val) const {
    //     std::not_equal_to<T> op;
    //     return impl_operator2(op, other_val);
    // }
    // Array2D operator<(const T& other_val) const {
    //     std::less<T> op;
    //     return impl_operator2(op, other_val);
    // }
    // Array2D operator<=(const T& other_val) const {
    //     std::less_equal<T> op;
    //     return impl_operator2(op, other_val);
    // }
    // Array2D operator>(const T& other_val) const {
    //     std::greater<T> op;
    //     return impl_operator2(op, other_val);
    // }
    // Array2D operator>=(const T& other_val) const {
    //     std::greater_equal<T> op;
    //     return impl_operator2(op, other_val);
    // }
    // Array2D operator&&(const T& other_val) const {
    //     std::logical_and<T> op;
    //     return impl_operator2(op, other_val);
    // }
    // Array2D operator||(const T& other_val) const {
    //     std::logical_or<T> op;
    //     return impl_operator2(op, other_val);
    // }

    Array2D operator+(const T& other_val) const {
        std::plus<T> op;
        return impl_operator2(op, other_val);
    }
    Array2D operator-(const T& other_val) const {
        std::minus<T> op;
        return impl_operator2(op, other_val);
    }
    Array2D operator*(const T& other_val) const {
        std::multiplies<T> op;
        return impl_operator2(op, other_val);
    }
    Array2D operator/(const T& other_val) const {
        std::divides<T> op;
        return impl_operator2(op, other_val);
    }
    Array2D operator%(const T& other_val) const {
        std::modulus<T> op;
        return impl_operator2(op, other_val);
    }

};  // end class

// Returns `true` if every element of the array maps via `boolifier` to `true`.
// If the array is empty, returns `true` vacuously.
template <class T>
bool all(const Array2D<T>& myarray, const std::function<bool(T)>& boolifier) {
    for (auto iter = myarray.cbegin(); iter != myarray.cend(); ++iter) {
        if (!boolifier(*iter)) {
            return false;
        }
    }
    return true;
}

// Returns `true` if at least one element of the array maps via `boolifier` to `true`.
// If the array is empty, returns `false` vacuously.
template <class T> bool any(const Array2D<T>& myarray, const std::function<bool(T)>& boolifier) {
    for (auto iter = myarray.cbegin(); iter != myarray.cend(); ++iter) {
        if (boolifier(*iter)) {
            return true;
        }
    }
    return false;
}

};  // end namespace
#endif


int main() {
    using namespace yelle;
    // constructor from nested initializer list (among several other constructors)
    Array2D<int> testArray = {{1,2,3},{3,4,5}};
    // safely append/insert a row that's too long (it'll be trimmed)
    testArray.safe_append_row({11,12,13,14}, 0);
    // safely append/insert a row that's too short (fills remaining spaces with default value, here -99)
    testArray.safe_insert_row(1, {21,22}, -99);

    // UNsafely append/insert a row...
    testArray.append_row({55,77,1345});
    // ...throwing an error if not exactly the right size
    try {
        fprintf(stdout, "Trying to insert a row that's too long...\n");
        testArray.insert_row(4, {8,3,-14, 11, 12});
    } catch (std::length_error& e) {
        fprintf(stdout, "Insertion failed: %s\n", e.what());
    }
    // ...or at a position that's too far
    try {
        fprintf(stdout, "Trying to insert a row too low...\n");
        testArray.insert_row(99999, {18,21,34});
    } catch (std::out_of_range& e) {
        fprintf(stdout, "Insertion failed: %s\n", e.what());
    }

    // initial printing
    fprintf(stdout, "testArray has rows=%zu, cols=%zu\n", testArray.rowCount(), testArray.colCount());
    fprintf(stdout, "testArray:\n%s", testArray.to_string().c_str());

    Array2D<int> copyArray = testArray;
    fprintf(stdout, "testArray after deletion of column index 1:\n%s", testArray.delete_col(1).to_string().c_str());
    fprintf(stdout, "earlier copy of testArray before deletion:\n%s", copyArray.to_string().c_str());

    // create a new array with all values converted to floats 
    fprintf(stdout, "testArray + 0.5:\n%s",
        testArray.map<float>(
            [](int x) { return static_cast<float>(x) + 0.5; }
        ).to_string().c_str()
    );

    // map the integer-squaring function in-place to the testArray
    fprintf(stdout, "testArray after squaring:\n%s", testArray.map_inplace(
            [](int x) { return x*x; }
        ).to_string().c_str()
    );

    // test some in-place operations
    fprintf(stdout, "the sum of all elements is %d\n", testArray.sum());
    fprintf(stdout, "transposing:\n%s", testArray.transpose().to_string().c_str());
    fprintf(stdout, "flipping vertically:\n%s", testArray.flip_vertical().to_string().c_str());
    fprintf(stdout, "flipping horizontally:\n%s", testArray.flip_horizontal().to_string().c_str());
    fprintf(stdout, "rotating clockwise:\n%s", testArray.rotateCW().to_string().c_str());
    fprintf(stdout, "rotating counterclockwise:\n%s", testArray.rotateCCW().to_string().c_str());
    fprintf(stdout, "doing resize-topleft:\n%s", testArray.safe_resize_topleft(6, 6, -20).to_string().c_str());
    fprintf(stdout, "testArray after filling with 7s:\n%s", testArray.fill(3, 5, 7).to_string().c_str());
    fprintf(stdout, "testArray after resizing:\n%s", testArray.safe_resize_keeporder(4, 6, -1).to_string().c_str());
    fprintf(stdout, "testArray after resizing again:\n%s", testArray.safe_resize_topleft(7, 4, -1).to_string().c_str());
    fprintf(stdout, "inserting column:\n%s", testArray.insertfill_col(4, 8).to_string().c_str());
    fprintf(stdout, "shifting up by 1, filling new:\n%s", testArray.shift_vertical_fill(-1, -5).to_string().c_str());

    // test some out-of-place operations:
    fprintf(stdout, "sum of array + itself:\n%s", (testArray + testArray).to_string().c_str());
    fprintf(stdout, "negative of array:\n%s", (-testArray).to_string().c_str());
    fprintf(stdout, "are any bigger than 3?\n%s\n", any<int>(testArray, 
        [](int x) { return x > 3; }
    ) ? "true" : "false");
    fprintf(stdout, "are any bigger than 99?\n%s\n", any<int>(testArray, 
        [](int x) { return x > 99; }
    ) ? "true" : "false");
    fprintf(stdout, "are all bigger than 3?\n%s\n", all<int>(testArray, 
        [](int x) { return x > 3; }
    ) ? "true" : "false");
    fprintf(stdout, "are all bigger than -99?\n%s\n", all<int>(testArray, 
        [](int x) { return x > -99; }
    ) ? "true" : "false");

    return 0;
}
