#ifndef yelle_ARRAY2D
#define yelle_ARRAY2D

#include <vector>
#include <functional>
#include <algorithm>
#include <optional>
#include <sstream>

namespace yelle {

enum class iterator_direction { FORWARD, REVERSED };

template <class T> class Array2D {
protected:
    // Holds the data of the 2D array.
    std::vector<std::vector<T>> array;

    // Iterator for looping over the whole list, cell-by-cell, row-by-row.
    struct EntireIterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;
    public:
        EntireIterator (const Array2D *const refrnt, pointer ptr, const size_t colct, iterator_direction dir = iterator_direction::FORWARD)
        : referent(refrnt), held_ptr(ptr), curr_row(0), curr_col(0), max_cols(colct) {
            if (dir == iterator_direction::REVERSED) {
                curr_row = referent->rowCount() - 1;
                curr_col = max_cols - 1;
            }
            return;
        }
        // access
        reference operator*() const {
            return *held_ptr;
        }
        pointer operator->() {
            return held_ptr;
        }

        // prefix increment
        EntireIterator& operator++() {
            if (curr_col == static_cast<long>(max_cols) - 1) {
                curr_col = 0;
                curr_row++;
            } else {
                curr_col++;
            }
            held_ptr = const_cast<pointer>(&((*referent)[curr_row][curr_col]));
            return *this;
        }

        // postfix increment
        EntireIterator operator++(int) {
            EntireIterator temp = *this;
            ++(*this);
            return temp;
        }

        EntireIterator& operator--() {
            if (curr_col == 0) {
                curr_col = max_cols - 1;
                curr_row--;
            } else {
                curr_col--;
            }
            held_ptr = const_cast<pointer>(&((*referent)[curr_row][curr_col]));
            return *this;
        }

        EntireIterator operator--(int) {
            EntireIterator temp = *this;
            --(*this);
            return temp;
        }

        EntireIterator operator+(const int offset) const {
            EntireIterator newCopy = *this;
            for (int i=0; i < offset; i++) {
                ++newCopy;
            }
            return newCopy;
        }

        EntireIterator operator-(const int offset) const {
            EntireIterator newCopy = *this;
            for (int i=0; i < offset; i++) {
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
    private:
        const Array2D* const referent;
        pointer held_ptr;
        long curr_row;
        long curr_col;
        const size_t max_cols;
    };

    // Iterator for looping over just one vertical column of the array.
    struct ColumnIterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;
    public:
        ColumnIterator(const Array2D *const refrnt, pointer ptr, const size_t col)
            : referent(refrnt), held_ptr(ptr), curr_row(0), fixed_col(col) {}
        // access
        reference operator*() const {
            return *held_ptr;
        }
        pointer operator->() {
            return held_ptr;
        }
        // prefix increment
        ColumnIterator& operator++() {
            curr_row++;
            held_ptr = const_cast<pointer>(&((*referent)[curr_row][fixed_col]));
            return *this;
        }

        // postfix increment
        ColumnIterator operator++(int) {
            ColumnIterator temp = *this;
            ++(*this);
            return temp;
        }

        ColumnIterator operator+(const int offset) const {
            ColumnIterator newCopy = *this;
            for (int i=0; i < offset; i++) {
                ++newCopy;
            }
            return newCopy;
        }

        friend bool operator==(const ColumnIterator& a, const ColumnIterator& b) {
            return a.held_ptr == b.held_ptr;
        }

        friend bool operator!=(const ColumnIterator& a, const ColumnIterator& b) {
            return a.held_ptr != b.held_ptr;
        }
    private:
        pointer held_ptr;
        size_t curr_row;
        const size_t fixed_col;
        const Array2D* const referent;

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
            array.emplace_back(list);
        }
        verifyDimensions();
        return;
    }

    // Single initializer-list constructor: Converts the list into the first row of the array.
    explicit Array2D(std::initializer_list<T> list_of_entries) {
        //fprintf(stdout, "Creating with single-initlist constructor!\n");
        array.emplace_back(list_of_entries);
        return;
    }

    // Vector-of-Vectors constructor: Copies or moves the values into the `array`.
    explicit Array2D(const std::vector<std::vector<T>>& vec_of_vecs) : array(vec_of_vecs) {
        //fprintf(stdout, "Creating with vec-of-vecs constructor!\n");
        verifyDimensions();
        return;
    }
    explicit Array2D(std::vector<std::vector<T>>&& vec_of_vecs) : array(vec_of_vecs) {
        //fprintf(stdout, "Creating with vec-of-vecs constructor!\n");
        verifyDimensions();
        return;
    }

    // Single vector constructor: Copies or moves the vector into the first row of the array.
    explicit Array2D(const std::vector<T>& vec) {
        //fprintf(stdout, "Creating with single-initlist constructor!\n");
        array.push_back(vec);
        return;
    }
    explicit Array2D(std::vector<T>&& vec) {
        //fprintf(stdout, "Creating with single-initlist constructor!\n");
        array.push_back(vec);
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

    // Re-Initializer: Returns the array to its original empty state.
    Array2D& clear() {
        array.clear();
        return *this;
    }

    Array2D& fill(const size_t newRowCt, const size_t newColCt, const T& value) {
        array.clear();
        array.reserve(newRowCt);
        for (size_t i=0; i < newRowCt; i++) {
            array.emplace_back();
            array[i].reserve(newColCt);
            for (size_t j=0; j < newColCt; j++) {
                array[i].push_back(value);
            }
        }
        return *this;
    }


    Array2D& build_from_string(const std::string instring, const std::function<T(char)> func) {
        if (!array.empty()) {
            array.clear();
        }

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

        return *this;
    }

    Array2D& build_from_file(FILE* infile, const std::function<T(char)> func) {
        if (!array.empty()) {
            array.clear();
        }

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

        return *this;
    }

    Array2D& resize(const size_t new_max_i, const size_t new_max_j) {
        const size_t area = size();
        if (new_max_i * new_max_j != area) {
            throw std::length_error("resizing would cause change in element count");
        }
        else if (area > 1) {
            Array2D oldself;
            oldself.array.swap(array);
            fill(new_max_i, new_max_j, oldself[0][0]);

            for (size_t k=1; k < area; k++) {   // we can skip k=0 because it's already copied in the `fill` above
                at_bysize(k) = std::move(oldself.at_bysize(k));
            }
        }

        return *this;
    }

    Array2D& safe_resize_keeporder(const size_t new_max_i, const size_t new_max_j, const T& backup_val) {
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
            for (size_t k=0; k < std::min<size_t>(old_area, new_area); k++) {
                at_bysize(k) = std::move(oldself.at_bysize(k));
            }
            // this second for-loop only runs if the new array is bigger than the old one
            for (size_t k=old_area; k < new_area; k++) {
                at_bysize(k) = backup_val;
            }
        }

        return *this;
    }

    Array2D& safe_resize_topleft(const size_t new_max_i, const size_t new_max_j, const T& backup_val) {
        const size_t old_max_i = rowCount();
        const size_t old_max_j = colCount();
        if (old_max_i * old_max_j == new_max_i * new_max_j) {
            resize(new_max_i, new_max_j);
        }
        else {
            Array2D oldself;
            oldself.array.swap(array);
            fill(new_max_i, new_max_j, backup_val);
            for (size_t i=0; i < std::min<size_t>(new_max_i, old_max_i); i++) {
                for (size_t j=0; j < std::min<size_t>(new_max_j, old_max_j); j++) {
                    array[i][j] = std::move(oldself.array[i][j]);
                }
            }
            oldself.clear();
        }

        return *this;
    }

    Array2D& flip_horizontal() noexcept {
        const size_t row_ct = rowCount();
        for (size_t i=0; i < row_ct; i++) {
            std::reverse(array[i].begin(), array[i].end());
        }
        return *this;
    }


    Array2D& flip_vertical() noexcept {
        std::reverse(array.begin(), array.end());
        return *this;
    }

    Array2D& transpose() noexcept {
        size_t old_rowCt = rowCount();
        size_t old_colCt = colCount();
        Array2D oldself;
        oldself.array.swap(array);
        array.clear();
        array.reserve(old_colCt);

        /* --------- option 1 --------- */
        for (size_t i=0; i < old_colCt; i++) {
            array.emplace_back();
            array[i].reserve(old_rowCt);
            for (size_t j=0; j < old_rowCt; j++) {
                array[i].push_back(std::move(oldself.array[j][i]));
            }
        }

        oldself.clear();
        return *this;
    }

    Array2D& rotateCW() noexcept {
        transpose();
        flip_horizontal();
        return *this;
    }

    Array2D& rotateCCW() noexcept {
        transpose();
        flip_vertical();
        return *this;
    }

    Array2D& shift_horizontal(long shift_j) noexcept {
        const long row_ct = static_cast<long>(rowCount());
        const long col_ct = static_cast<long>(colCount());

        // negative because std::rotate does +rotation to the left, whereas I want +rotation to the right
        shift_j = -(shift_j % col_ct);
        if (shift_j == 0) {
            return *this;
        }
        if (shift_j < 0) {
            shift_j = col_ct - shift_j;
        }

        for (long i=0; i < row_ct; i++) {
            if (shift_j >= 0) {
                std::rotate(array[i].begin(), array[i].begin() + static_cast<size_t>(shift_j), array[i].end());
            }
        }

        return *this;
    }

    Array2D& shift_vertical(const long shift_i) noexcept {
        const long row_ct = static_cast<long>(rowCount());
        // negative because std::rotate does +rotation to up, whereas I want +rotation down
        shift_i = -(shift_i % row_ct);
        if (shift_i == 0) {
            return *this;
        }
        if (shift_i < 0) {
            shift_i = row_ct - shift_i;
        }

        std::rotate(array.begin(), array.begin() + static_cast<size_t>(shift_i), array.end());

        return *this;
    }

    /* ============================
        Element Access by Index
    ============================= */

    std::vector<T>& operator[](const size_t index) {
        return array[index];
    }

    const std::vector<T>& operator[](const size_t index) const {
        return array[index];
    }

    std::vector<T>& at(const size_t index) {
        return array.at(index);
    }

    const std::vector<T>& at(const size_t index) const {
        return array.at(index);
    }

    // returns a `std::optional` view (but not write access) to the element stored at [i,j]. If there is no element at [i,j], then returns a null optional.
    std::optional<T> safe_look(const size_t i, const size_t j) const noexcept {
        if (i < rowCount() && j < array[i].size()) {
            return std::optional<T>(array[i][j]);
        } else {
            return std::optional<T>();
        }
    }

    // ELEMENT ACCESS, for k=0...(rowCt-1)*(colCt-1), traversing each row 
    T& at_bysize(const size_t index) {
        const size_t cct = colCount();
        return array.at(index / cct).at(index % cct);
    }

    const T& at_bysize(const size_t index) const {
        const size_t cct = colCount();
        return array.at(index / cct).at(index % cct);
    }

    /* ============================
        Constant Attributes
    ============================= */

    bool empty() const {
        return (array.empty() || array[0].empty());
    }

    size_t size() const {
        return rowCount() * colCount();
    }

    size_t rowCount() const {
        return array.size();
    }

    size_t colCount() const {
        return (array.empty() ? 0 : array[0].size());
    }

    bool isSquare() const {
        return rowCount() == colCount();
    }

    std::string to_string() const {
        std::stringstream oss;
        for (size_t i=0; i < rowCount(); i++) {
            for (size_t j=0; j < colCount(); j++) {
                oss << array[i][j] << "\t";
            }
            oss << "\n";
        }

        return oss.str();
    }

    // Verifies that the matrix is rectangular, viz. by requiring that every row beyond index 0 have the same `.size()` as the index-0 row.
    void verifyDimensions() const {
        if (array.empty()) {
            fprintf(stderr, "DEBUG: Called `verifyDimensions` on empty array!\n");
            return;
        } else if (array.size() == 1 && array[0].size() == 0) {
            fprintf(stderr, "DEBUG: Called `verifyDimensions` on an array with 1 empty row!\n");
            return;
        }
        for (size_t i=1; i < rowCount(); i++) {
            if (array[i].size() != colCount()) {
                fprintf(stderr, "Rows not of equal length! Row 0 is length %zu, but row %zu is length %zu.\n", colCount(), i, array[i].size());
                throw std::length_error("incompatible row lengths for rectangular 2D array\n");
            }
        }
        return;
    }

    bool safe_verifyDimensions() const {
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

    T foldl(const std::function<T(T,T)> func) const {
        if (empty()) {
            throw std::length_error("cannot use foldl on empty array");
        } else {
            T accumulator = *cbegin();
            for (auto iter=cbegin()+1; iter != cend(); iter++) {
                accumulator = func(accumulator, *iter);
                //fprintf(stdout, "acc=%d\n", accumulator);
            }
            return accumulator;
        }
    }

    T foldr(const std::function<T(T,T)> func) const {
        if (empty()) {
            throw std::length_error("cannot use foldr on empty array");
        } else {
            T accumulator = *crbegin();
            for (auto iter=crbegin()-1; iter != crend(); iter--) {
                accumulator = func(*iter, accumulator);
            }
            return accumulator;
        }   
    }

    T foldl_col(const size_t col_j, const std::function<T(T,T)> func) const {
        if (empty()) {
            throw std::length_error("cannot use foldl_col on empty array");
        } else if (col_j >= colCount()) {
            throw std::out_of_range("cannot use foldl_col on too high of column index");
        } else {
            T accumulator = *cbegin_col(col_j);
            for (auto iter=cbegin_col(col_j)+1; iter != cend_col(col_j); iter++) {
                accumulator = func(accumulator, *iter);
            }
            return accumulator;
        }
    }

    template <class S>
    S foldl(const std::function<S(S,T)> func, S accumulator) const {
        if (empty()) {
            throw std::length_error("cannot use foldl template on empty array");
        } else {
            for (auto iter=cbegin(); iter != cend(); iter++) {
                accumulator = func(accumulator, *iter);
            }
            return accumulator;
        }
    }

    T sum() const {
        std::plus<T> T_addition;
        return foldl(T_addition);
    }

    T sum_col(const size_t column_j) const {
        std::plus<T> T_addition;
        return foldl_col(column_j, T_addition);
    }

    T product() const {
        std::multiplies<T> T_multiplication;
        return foldl(T_multiplication);
    }

    T product_col(const size_t column_j) const {
        std::multiplies<T> T_multiplication;
        return foldl_col(column_j, T_multiplication);
    }

    /* ============================
        Adding rows to the array
    ============================= */
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
            for (size_t k=0; k < overshoot_columns; k++) {
                array[position].erase(array[position].begin() + col_ct);
            }
        }
        else {
            const size_t missing_columns = col_ct - newrow_ct;
            array.insert(array.begin()+position, new_row);
            for (size_t k=0; k < missing_columns; k++) {
                array[position].push_back(backup_val);
            }
        }
        //verifyDimensions();
        return *this;
    }
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
            for (size_t k=0; k < overshoot_columns; k++) {
                array[position].erase(array[position].begin() + col_ct);
            }
        }
        else {
            const size_t missing_columns = col_ct - newrow_ct;
            array.insert(array.begin()+position, std::move(new_row));
            for (size_t k=0; k < missing_columns; k++) {
                array[position].push_back(backup_val);
            }
        }
        //verifyDimensions();
        return *this;
    }

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
            for (size_t k=0; k < overshoot_columns; k++) {
                array[position].erase(array[position].begin() + col_ct);
            }
        }
        else {
            const size_t missing_columns = col_ct - newrow_ct;
            array.insert(array.begin()+position, new_row);
            for (size_t k=0; k < missing_columns; k++) {
                array[position].push_back(backup_val);
            }
        }
        //verifyDimensions();
        return *this;
    }

    // Default append, with vector: Converts the vector into a new row. Will throw a `std::length_error` if not exactly the right width.
    Array2D& append_row(const std::vector<T>& new_row) {
        return insert_row(array.size(), new_row);
    }
    Array2D& append_row(std::vector<T>&& new_row) {
        return insert_row(array.size(), new_row);
    }

    // Default append, with list: Converts the list into a new row. Will throw a `std::length_error` if not exactly the right width.
    Array2D& append_row(std::initializer_list<T> new_row) {
        return insert_row(array.size(), new_row);
    }

    // Safe append, with vector: Converts the vector into a new row; if not exactly the right width, the vector is trimmed (if too long) or padded with `backup_val` (if too short).
    Array2D& safe_append_row(const std::vector<T>& new_row, const T& backup_val) {
        return safe_insert_row(array.size(), new_row, backup_val);
    }
    Array2D& safe_append_row(std::vector<T>&& new_row, const T& backup_val) {
        return safe_insert_row(array.size(), new_row, backup_val);
    }

    // Safe append, with list: Converts the list into a new row; if not exactly the right width, the list is trimmed (if too long) or padded with `backup_val` (if too short).
    Array2D& safe_append_row(std::initializer_list<T> new_row, const T& backup_val) {
        return safe_insert_row(array.size(), new_row, backup_val);
    }

    Array2D& insertfill_row(const size_t position, const T& fillvalue) {
        if (position > rowCount()) {
            fprintf(stderr, "Attempted to insertfill a row at index %zu into an array with only %zu rows!\n", position, rowCount());
            throw std::out_of_range("tried to add row at too-high index value");
        } else {
            array.insert(array.begin()+position, std::vector<T>(colCount(), fillvalue));
        }
        return *this;
    }

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

    Array2D& insert_col(const size_t position, const std::vector<T>& new_col) {
        const size_t row_ct = rowCount();
        
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insert a column at index %zu into an array with only %zu columns!\n", position, colCount());
            throw std::out_of_range("tried to add column at too-high index value");
        }
        else if (row_ct == 0) {
            for (size_t i=0; i < new_col.size(); i++) {
                array.emplace_back();
                array[i].push_back(new_col[i]);
            }
        }
        else if (new_col.size() == row_ct) {
            for (size_t i=0; i < row_ct; i++) {
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
    Array2D& insert_col(const size_t position, const std::vector<T>&& new_col) {
        const size_t row_ct = rowCount();
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insert a column at index %zu into an array with only %zu columns!\n", position, colCount());
            throw std::out_of_range("tried to add column at too-high index value");
        }
        else if (row_ct == 0) {
            for (size_t i=0; i < new_col.size(); i++) {
                array.emplace_back();
                array[i].push_back(std::move(new_col[i]));
            }
        }
        else if (new_col.size() == row_ct) {
            for (size_t i=0; i < row_ct; i++) {
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

    Array2D& insert_col(const size_t position, std::initializer_list<T> new_col) {
        const size_t row_ct = rowCount();
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insert a column at index %zu into an array with only %zu columns!\n", position, colCount());
            throw std::out_of_range("tried to add column at too-high index value");
        }
        else if (row_ct == 0) {
            for (size_t i=0; i < new_col.size(); i++) {
                array.emplace_back();
                array[i].push_back(*(new_col.begin()+i));
            }
        }
        else if (new_col.size() == row_ct) {
            for (size_t i=0; i < row_ct; i++) {
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

    Array2D& safe_insert_col(const size_t position, const std::vector<T>& new_col, const T& backup_val) {
        const size_t row_ct = rowCount();
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insert a column at index %zu into an array with only %zu columns... Returning array unchanged.\n", position, colCount());
        }
        else if (row_ct == 0) {
            insert_col(position, new_col);
        }
        else {
            for (size_t i=0; i < row_ct; i++) {
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

    Array2D& safe_insert_col(const size_t position, std::vector<T>&& new_col, const T& backup_val) {
        const size_t row_ct = rowCount();
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insert a column at index %zu into an array with only %zu columns... Returning array unchanged.\n", position, colCount());
        }
        else if (row_ct == 0) {
            insert_col(position, std::move(new_col));
        }
        else {
            for (size_t i=0; i < row_ct; i++) {
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

    Array2D& safe_insert_col(const size_t position, std::initializer_list<T> new_col, const T& backup_val) {
        const size_t row_ct = rowCount();
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insert a column at index %zu into an array with only %zu columns... Returning array unchanged.\n", position, colCount());
        }
        else if (row_ct == 0) {
            insert_col(position, new_col);
        }
        else {
            for (size_t i=0; i < row_ct; i++) {
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

    Array2D& append_col(const std::vector<T>& new_col) {
        return insert_col(colCount(), new_col);
    }
    Array2D& append_col(std::vector<T>&& new_col) {
        return insert_col(colCount(), std::move(new_col));
    }
    Array2D& append_col(std::initializer_list<T> new_col) {
        return insert_col(colCount(), new_col);
    }

    Array2D& safe_append_col(const std::vector<T>& new_col, const T& backup_val) {
        return safe_insert_col(colCount(), new_col, backup_val);
    }
    Array2D& safe_append_col(std::vector<T>&& new_col, const T& backup_val) {
        return safe_insert_col(colCount(), std::move(new_col), backup_val);
    }
    Array2D& safe_append_col(std::initializer_list<T> new_col, const T& backup_val) {
        return safe_insert_col(colCount(), new_col, backup_val);
    }

    Array2D& insertfill_col(const size_t position, const T& backup_val) {
        if (position > colCount()) {
            fprintf(stderr, "Attempted to insertfill a column at index %zu into an array with only %zu columns!\n", position, colCount());
            throw std::out_of_range("tried to add column at too-high index value");
        }
        else {
            return safe_insert_col(position, std::initializer_list<T>(), backup_val);
        }
    }
    Array2D& safe_insertfill_col(const size_t position, const T& backup_val) {
        return safe_insert_col(position, std::initializer_list<T>(), backup_val);
    }

    /* ============================
        Deletion
    ============================= */

    Array2D& delete_row(const size_t i) {
        if (i >= rowCount()) {
            fprintf(stderr, "Tried to delete row %zu from an Array2D with %zu rows!\n", i, rowCount());
            throw std::out_of_range("cannot delete row outside of valid range");
        } else {
            array.erase(array.cbegin()+i, array.cbegin()+i+1);
            return *this;
        }
    }

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


    Array2D& delete_col(const size_t j) {
        if (j >= colCount()) {
            fprintf(stderr, "Tried to delete column %zu from an Array2D with %zu columns!\n", j, colCount());
            throw std::out_of_range("cannot delete column outside of valid range");
        } else {
            const size_t row_ct = rowCount();
            // would a for-each loop work here?
            for (size_t i=0; i < row_ct; i++) {
                std::vector<T>& thisRow = array[i];
                thisRow.erase(thisRow.cbegin()+j, thisRow.cbegin()+j+1);
            }
            return *this;
        }
    }

    Array2D& delete_cols(const size_t j_start, const size_t j_past_end) {
        if (j_start >= colCount() || j_past_end > colCount()) {
            fprintf(stderr, "Tried to delete columns [%zu,%zu) from an Array2D with %zu columns!\n", j_start, j_past_end, colCount());
            throw std::out_of_range("cannot delete columns outside of valid range");
        }
        else if (j_start < j_past_end) {
            const size_t row_ct = rowCount();
            // would a for-each loop work here?
            for (size_t i=0; i < row_ct; i++) {
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
            return EntireIterator(nullptr, nullptr, 0);
        } else {
            return EntireIterator(this, &(array[0][0]), colCount());
        }
    }

    // Forward & constant iterator, points to the element at index (0,0).
    // Incrementing this iterator goes to the next element of the same row, or--if already at the end of the row--to position 0 in the next row.
    const EntireIterator cbegin() const {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0);
        } else {
            return EntireIterator(this, const_cast<T*>(&(array[0][0])), colCount());
        }
    }

    // Forward iterator, points to the nonexistent element at index (rowCount,0), just "beyond" the end of the array.
    EntireIterator end() {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0);
        } else {
            return EntireIterator(this, &(array[rowCount()][0]), colCount());
        }
    }

    // Forward iterator, points to the nonexistent element at index (rowCount,0), just "beyond" the end of the array.
    const EntireIterator cend() const {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0);
        } else {
            return EntireIterator(this, const_cast<T*>(&(array[rowCount()][0])), colCount());
        }
    }

    EntireIterator rbegin() const {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0, iterator_direction::REVERSED);
        } else {
            return EntireIterator(this, &(array[rowCount()-1][colCount()-1]), colCount(), iterator_direction::REVERSED);
        }
    }

    const EntireIterator crbegin() const {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0, iterator_direction::REVERSED);
        } else {
            return EntireIterator(this, const_cast<T*>(&(array[rowCount()-1][colCount()-1])), colCount(), iterator_direction::REVERSED);
        }
    }

    EntireIterator rend() const {
        if (empty()) {
            return EntireIterator(nullptr, nullptr, 0, iterator_direction::REVERSED);
        } else {
            return EntireIterator(this, nullptr, colCount(), iterator_direction::REVERSED);
        }
    }

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

    // Forward iterator, points to the element at index (0,fixed_col_j).
    // Incrementing this operator goes from pointing to (i, fixed_col_j) to (i+1, fixed_col_j).
    ColumnIterator begin_col(const size_t fixed_col_j) {
        if (array.empty() || fixed_col_j >= colCount()) {
            return ColumnIterator(nullptr, nullptr, 0);
        } else {
            return ColumnIterator(this, &(array[0][fixed_col_j]), fixed_col_j);
        }
    }

    // Forward & constant iterator, points to the element at index (0,fixed_col_j).
    // Incrementing this operator goes from pointing to (i, fixed_col_j) to (i+1, fixed_col_j).
    const ColumnIterator cbegin_col(const size_t fixed_col_j) const {
        if (array.empty() || fixed_col_j >= colCount()) {
            return ColumnIterator(nullptr, nullptr, 0);
        } else {
            return ColumnIterator(this, const_cast<T*>(&(array[0][fixed_col_j])), fixed_col_j);
        }
    }

    // Forward iterator, points to the nonexistent element at index (rowCount,fixed_col_j), just "beyond" the end of the column.
    ColumnIterator end_col(const size_t fixed_col_j) {
        if (array.empty() || fixed_col_j >= colCount()) {
            return ColumnIterator(nullptr, nullptr, 0);
        } else {
            return ColumnIterator(this, &(array[rowCount()][fixed_col_j]), fixed_col_j);
        }
    }

    // Forward & const iterator, points to the nonexistent element at index (rowCount,fixed_col_j), just "beyond" the end of the column.
    const ColumnIterator cend_col(const size_t fixed_col_j) const {
        if (array.empty() || fixed_col_j >= colCount()) {
            return ColumnIterator(nullptr, nullptr, 0);
        } else {
            return ColumnIterator(this, const_cast<T*>(&(array[rowCount()][fixed_col_j])), fixed_col_j);
        }
    }

    /* ============================
        Functional Programming
    ============================= */

    // Creates a copy of this Array2D, for usage in method-chaining.
    Array2D clone() const {
        return Array2D(*this);
    }

    // Given a lambda function or function pointer from type `T` to `S`, creates and returns a new `Array2D` of type `S` whose elements are equal to `func(t)` for each corresponding `t` in the input array.
    template <class S>
    Array2D<S> map(const std::function<S(T)>& func) const {
        Array2D<S> outArray;
        std::vector<S> temp_row;

        for (size_t i=0; i < rowCount(); i++) {
            //fprintf(stderr, "Going to row %zu\n", i);
            temp_row.clear();
            for (size_t j=0; j < colCount(); j++) {
                //fprintf(stderr, "Applying `func` to element %zu,%zu\n", i, j);
                temp_row.push_back(func(array[i][j]));
            }
            outArray.append_row(std::move(temp_row));
        }
        return outArray;
    }

    template <class S, class T2>
    Array2D<S> map2(const std::function<S(T,T2)&> func, const Array2D<T2>& arr2) const {
        Array2D<S> outArray;
        std::vector<S> temp_row;

        for (size_t i=0; i < rowCount(); i++) {
            temp_row.clear();
            for (size_t j=0; j < colCount(); j++) {
                temp_row.push_back(func(array[i][j], arr2.array[i][j]));
            }
            outArray.append_row(std::move(temp_row));
        }
        return outArray;
    }

    // Given a lambda function or function pointer from type `T` to `T`, applies the function in-place to each element of this array.
    Array2D& map_inplace(const std::function<T(T)>& func) {
        // tiny optimization, since this isn't a const method
        const size_t row_ct = rowCount();
        const size_t col_ct = colCount();

        for (size_t i=0; i < row_ct; i++) {
            for (size_t j=0; j < col_ct; j++) {
                array[i][j] = func(array[i][j]);
            }
        }
        return *this;
    }


    /* ============================
        Unary Operators
    ============================= */

    Array2D impl_operator1(const std::function<T(const T&)> op) const {
        const size_t row_ct = rowCount();
        const size_t col_ct = colCount();
        Array2D new2D;
        new2D.array.reserve(row_ct);
        for (size_t i=0; i < row_ct; i++) {
            new2D.array.emplace_back();
            new2D.array[i].reserve(col_ct);
            for (size_t j=0; j < col_ct; j++) {
                new2D.array[i].push_back(op(array[i][j]));
            }
        }
        fprintf(stdout, "op1 : returning array of size %zu,%zu\n", new2D.rowCount(), new2D.colCount());
        return new2D;
    }

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

    Array2D impl_operator2(const std::function<T(const T&, const T&)> op, const Array2D& other) const {
        const size_t row_ct = rowCount();
        const size_t col_ct = colCount();
        if (row_ct != other.rowCount() || col_ct != other.colCount()) {
            throw std::length_error("called boolean operator on Array2D objects of unequal dimension");
        }
        Array2D new2D;
        new2D.array.reserve(row_ct);
        for (size_t i=0; i < row_ct; i++) {
            new2D.array.emplace_back();
            new2D.array[i].reserve(col_ct);
            for (size_t j=0; j < col_ct; j++) {
                new2D.array[i].push_back(op(array[i][j], other.array[i][j]));
            }
        }

        fprintf(stdout, "op2-a : returning array of size %zu,%zu\n", new2D.rowCount(), new2D.colCount());
        return new2D;
    }

    Array2D impl_operator2(const std::function<T(const T&, const T&)> op, const T& other) const {
        const size_t row_ct = rowCount();
        const size_t col_ct = colCount();

        Array2D new2D;
        new2D.array.reserve(row_ct);
        for (size_t i=0; i < row_ct; i++) {
            new2D.array.emplace_back();
            new2D.array[i].reserve(col_ct);
            for (size_t j=0; j < col_ct; j++) {
                new2D.array[i].push_back(op(array[i][j], other));
            }
        }

        fprintf(stdout, "op2-b : returning array of size %zu,%zu\n", new2D.rowCount(), new2D.colCount());
        return new2D;
    }

    /* ----- OPERATORS ON OTHER ARRAYS ----- */

    Array2D operator==(const Array2D& other) const {
        std::equal_to<T> op;
        return impl_operator2(op, other);
    }
    Array2D operator!=(const Array2D& other) const {
        std::not_equal_to<T> op;
        return impl_operator2(op, other);
    }
    Array2D operator<(const Array2D& other) const {
        std::less<T> op;
        return impl_operator2(op, other);
    }
    Array2D operator<=(const Array2D& other) const {
        std::less_equal<T> op;
        return impl_operator2(op, other);
    }
    Array2D operator>(const Array2D& other) const {
        std::greater<T> op;
        return impl_operator2(op, other);
    }
    Array2D operator>=(const Array2D& other) const {
        std::greater_equal<T> op;
        return impl_operator2(op, other);
    }
    Array2D operator&&(const Array2D& other) const {
        std::logical_and<T> op;
        return impl_operator2(op, other);
    }
    Array2D operator||(const Array2D& other) const {
        std::logical_or<T> op;
        return impl_operator2(op, other);
    }

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

    Array2D operator==(const T& other_val) const {
        std::equal_to<T> op;
        return impl_operator2(op, other_val);
    }
    Array2D operator!=(const T& other_val) const {
        std::not_equal_to<T> op;
        return impl_operator2(op, other_val);
    }
    Array2D operator<(const T& other_val) const {
        std::less<T> op;
        return impl_operator2(op, other_val);
    }
    Array2D operator<=(const T& other_val) const {
        std::less_equal<T> op;
        return impl_operator2(op, other_val);
    }
    Array2D operator>(const T& other_val) const {
        std::greater<T> op;
        return impl_operator2(op, other_val);
    }
    Array2D operator>=(const T& other_val) const {
        std::greater_equal<T> op;
        return impl_operator2(op, other_val);
    }
    Array2D operator&&(const T& other_val) const {
        std::logical_and<T> op;
        return impl_operator2(op, other_val);
    }
    Array2D operator||(const T& other_val) const {
        std::logical_or<T> op;
        return impl_operator2(op, other_val);
    }

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

template <class T>
bool all(const Array2D<T>& myarray, const std::function<bool(T)>& boolifier) {
    for (auto iter = myarray.cbegin(); iter != myarray.cend(); ++iter) {
        if (!boolifier(*iter)) {
            return false;
        }
    }
    return true;
}

template <class T>
bool any(const Array2D<T>& myarray, const std::function<bool(T)>& boolifier) {
    for (auto iter = myarray.cbegin(); iter != myarray.cend(); ++iter) {
        if (boolifier(*iter)) {
            return true;
        }
    }
    return true;
}

};  // end namespace
#endif

/*
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
            [](const int x) { return static_cast<float>(x) + 0.5; }
        ).to_string().c_str()
    );

    // map the integer-squaring function in-place to the testArray
    testArray.map_inplace(
            [](int x) { return x*x; }
        );
    fprintf(stdout, "testArray after squaring:\n%s", testArray.to_string().c_str());
    fprintf(stdout, "the sum of all elements is %d\n", testArray.sum());

    // test flipping stuff
    fprintf(stdout, "transposing:\n%s", testArray.transpose().to_string().c_str());
    fprintf(stdout, "flipping vertically:\n%s", testArray.flip_vertical().to_string().c_str());
    fprintf(stdout, "flipping horizontally:\n%s", testArray.flip_horizontal().to_string().c_str());
    fprintf(stdout, "rotating clockwise:\n%s", testArray.rotateCW().to_string().c_str());
    fprintf(stdout, "rotating counterclockwise:\n%s", testArray.rotateCCW().to_string().c_str());
    fprintf(stdout, "doing resize-topleft:\n%s", testArray.safe_resize_topleft(6, 6, -20).to_string().c_str());
    fprintf(stdout, "testArray after filling with 7s:\n%s", testArray.fill(3, 5, 7).to_string().c_str());
    fprintf(stdout, "testArray after resizing:\n%s", testArray.safe_resize_keeporder(4, 6, -1).to_string().c_str());
    fprintf(stdout, "inserting column:\n%s", testArray.insertfill_col(4, 8).to_string().c_str());

    Array2D<int> negArray = - testArray;
    fprintf(stdout, "negArray has sizes %zu,%zu\n", negArray.rowCount(), negArray.colCount());
    fprintf(stdout, "and here it is:\n%s", negArray.to_string().c_str());

    return 0;
}
*/
