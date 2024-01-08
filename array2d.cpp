#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <functional>
#include <optional>
#include <sstream>
#include <cstdio>

namespace Yelle {
template <class T> class Array2D {
private:
    // Holds the data of the 2D array.
    std::vector<std::vector<T>> array;

    // Iterator for looping over the whole list, cell-by-cell, row-by-row.
    struct EntireIterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;
    public:
        EntireIterator (Array2D *const refrnt, pointer ptr, const size_t colct)
            : referent(refrnt), held_ptr(ptr), curr_row(0), curr_col(0), max_cols(colct) {}
        // access
        reference operator*() const {
            return *held_ptr;
        }
        pointer operator->() {
            return held_ptr;
        }

        // prefix increment
        EntireIterator& operator++() {
            if (curr_col == max_cols - 1) {
                curr_col = 0;
                curr_row++;
            } else {
                curr_col++;
            }
            held_ptr = &((*referent)[curr_row][curr_col]);
            return *this;
        }

        // postfix increment
        EntireIterator operator++(int _) {
            EntireIterator temp = *this;
            ++(*this);
            return temp;
        }

        friend bool operator==(const EntireIterator& a, const EntireIterator& b) {
            return a.held_ptr == b.held_ptr;
        }

        friend bool operator!=(const EntireIterator& a, const EntireIterator& b) {
            return a.held_ptr != b.held_ptr;
        }
    private:
        pointer held_ptr;
        size_t curr_row;
        size_t curr_col;
        const size_t max_cols;
        Array2D* const referent;
    };

    // Iterator for looping over just one vertical column of the array.
    struct ColumnIterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;
    public:
        ColumnIterator(Array2D *const refrnt, pointer ptr, const size_t col)
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
            held_ptr = &((*referent)[curr_row][fixed_col]);
            return *this;
        }

        // postfix increment
        ColumnIterator operator++(int _) {
            ColumnIterator temp = *this;
            ++(*this);
            return temp;
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
        Array2D* const referent;

    };

    // PRIVATE: Verifies that the matrix is rectangular, viz. by requiring that every row beyond index 0 have the same `.size()` as the index-0 row.
    void verifyDimensions() const {
        if (array.empty()) {
            fprintf(stderr, "DEBUG: Called `verifyDimensions` on empty array!\n");
            return;
        } else if (array.size() == 1 && array[0].size() == 0) {
            fprintf(stderr, "DEBUG: Called `verifyDimensions` on an array with 1 empty row!\n");
            return;
        }
        size_t row_ct = rowCount(); //tiny optimization
        size_t col_ct = colCount();
        for (size_t i=1; i < row_ct; i++) {
            if (array[i].size() != col_ct) {
                fprintf(stderr, "Rows not of equal length! Row 0 is length %llu, but row %llu is length %llu.\n", col_ct, i, array[i].size());
                throw std::length_error("incompatible row lengths for rectangular 2D array\n");
            }
        }
        return;
    }

public:
    // Default constructor: Creates an empty array without any rows.
    Array2D() {
        //fprintf(stderr, "Creating with default constructor!\n");
        //array.clear();
        return;
    }

    // Double initializer-list constructor: Converts the list of lists into a 2D array.
    Array2D(std::initializer_list<std::initializer_list<T>> list_of_lists) {
        //fprintf(stderr, "Creating with double-initlist constructor!\n");
        for (const std::initializer_list<T>& list : list_of_lists) {
            array.emplace_back(list);
        }
        verifyDimensions();
        return;
    }

    // Single initializer-list constructor: Converts the list into the first row of the array.
    Array2D(std::initializer_list<T> list_of_entries) {
        //fprintf(stderr, "Creating with single-initlist constructor!\n");
        array.emplace_back(list_of_entries);
        return;
    }

    // Vector-of-Vectors constructor: Moves them directly into the `array`.
    Array2D(const std::vector<std::vector<T>>& vec_of_vecs) : array(vec_of_vecs) {
        //fprintf(stderr, "Creating with vec-of-vecs constructor!\n");
        verifyDimensions();
        return;
    }

    // File-and-function constructor: Given an `fopen`ed input file `infile` and a function from `char` type to the desired parameter type for this Array2D, constructs an Array2D of that type.
    Array2D(FILE* infile, const std::function<T(char)> func) {
        //fprintf(stderr, "Creating with file&func constructor!\n");
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

        return;
    }

    // String-and-function constructor: Given a `std::string` and a function from `char` type to the desired parameter type for this Array2D, constructs an Array2D of that type.
    Array2D(const std::string instring, const std::function<T(char)> func) {
        //fprintf(stderr, "Creating with string&func constructor!\n");
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

        return;
    }

    Array2D<T> clone() const {
        //fprintf(stderr, "Cloning!\n");
        Array2D newClone(*this);
        return newClone;
    }

    /* ============================
        Adding rows to the array
    ============================= */
    Array2D& insert_row(const size_t position, const std::vector<T>& new_row) {
        if (position > rowCount()) {
            fprintf(stderr, "Attempted to insert a row at index %llu into an array with only %llu rows!\n", position, rowCount());
            throw std::out_of_range("tried to add row at too-high index value");
        } else if (new_row.size() == colCount() || size() == 0) {
            array.insert(array.begin()+position, new_row);
        } else {
            fprintf(stderr, "Attempted to insert a row of length %llu into an array with %llu columns!\n", new_row.size(), colCount());
            throw std::length_error("tried to add row of incorrect length");
        }
        //verifyDimensions();
        return *this;
    }

    Array2D& insert_row(const size_t position, std::initializer_list<T> new_row) {
        if (position > rowCount()) {
            fprintf(stderr, "Attempted to insert a row at index %llu into an array with only %llu rows!\n", position, rowCount());
            throw std::out_of_range("tried to add row at too-high index value");
        } else if (new_row.size() == colCount() || size() == 0) {
            array.insert(array.begin()+position, new_row);
        } else {
            fprintf(stderr, "Attempted to insert a row of length %llu into an array with %llu columns!\n", new_row.size(), colCount());
            throw std::length_error("tried to add row of incorrect length");
        }
        // verifyDimensions();
        return *this;
    }

    Array2D& insert_row_safe(const size_t position, const std::vector<T>& new_row, const T& backup_val) noexcept {
        if (position > array.size()) {
            fprintf(stderr, "Attempted to insert row (of length %llu) into row-index %llu of array, but array has only %llu rows.\nReturning array unchanged.\n", new_row.size(), position, array.size());
        } else if (new_row.size() >= colCount()) {
            array.insert(array.begin()+position, std::vector<T>(new_row.begin(), new_row.begin() + colCount()));
        } else {
            array.insert(array.begin()+position, new_row);
            const size_t missing_columns = colCount() - new_row.size();
            for (int i=0; i < missing_columns; i++) {
                array[position].push_back(backup_val);
            }
        }
        //verifyDimensions();
        return *this;
    }

    Array2D& insert_row_safe(const size_t position, std::initializer_list<T> new_row, const T& backup_val) noexcept {
        if (position > array.size()) {
            fprintf(stderr, "Attempted to insert row (of length %llu) into row-index %llu of array, but array has only %llu rows.\nReturning array unchanged.\n", new_row.size(), position, array.size());
        } else if (new_row.size() >= colCount()) {
            array.insert(array.begin()+position, std::vector<T>(new_row.begin(), new_row.begin() + colCount()));
        } else {
            array.insert(array.begin()+position, new_row);
            const size_t missing_columns = colCount() - new_row.size();
            for (int i=0; i < missing_columns; i++) {
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

    // Default append, with list: Converts the list into a new row. Will throw a `std::length_error` if not exactly the right width.
    Array2D& append_row(std::initializer_list<T> new_row) {
        return insert_row(array.size(), new_row);
    }

    // Safe append, with vector: Converts the vector into a new row; if not exactly the right width, the vector is trimmed (if too long) or padded with `backup_val` (if too short).
    Array2D& append_row_safe(const std::vector<T>& new_row, const T& backup_val) {
        return insert_row_safe(array.size(), new_row, backup_val);
    }

    // Safe append, with list: Converts the list into a new row; if not exactly the right width, the list is trimmed (if too long) or padded with `backup_val` (if too short).
    Array2D& append_row_safe(std::initializer_list<T> new_row, const T& backup_val) {
        return insert_row_safe(array.size(), new_row, backup_val);
    }

    /* ============================
        Adding columns to the array
    ============================= */

    // TODO
    Array2D insert_col();
    // TODO
    Array2D insert_col_safe();

    /* ============================
        Deletion
    ============================= */

    // Re-Initializer: Returns the array to its original empty state.
    Array2D& clear() {
        array.clear();
        return *this;
    }

    Array2D& delete_row(const size_t i) {
        if (i >= rowCount()) {
            fprintf(stderr, "Tried to delete row %llu from an Array2D with %llu rows!\n", i, rowCount());
            throw std::out_of_range("cannot delete row outside of valid range");
        } else {
            array.erase(array.cbegin()+i, array.cbegin()+i+1);
            return *this;
        }
    }

    Array2D& delete_rows(const size_t i_start, const size_t i_past_end) {
        if (i_start >= rowCount() || i_past_end > rowCount()) {
            fprintf(stderr, "Tried to delete rows [%llu,%llu) from an Array2D with %llu rows!\n", i_start, i_past_end, rowCount());
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
            fprintf(stderr, "Tried to delete column %llu from an Array2D with %llu columns!\n", j, colCount());
            throw std::out_of_range("cannot delete column outside of valid range");
        } else {
            size_t row_ct = rowCount();
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
            fprintf(stderr, "Tried to delete columns [%llu,%llu) from an Array2D with %llu columns!\n", j_start, j_past_end, colCount());
            throw std::out_of_range("cannot delete columns outside of valid range");
        }
        else if (j_start < j_past_end) {
            size_t row_ct = rowCount();
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

        size_t row_ct = rowCount(); // tiny optimizations
        size_t col_ct = colCount();

        for (size_t i=0; i < row_ct; i++) {
            for (size_t j=0; j < col_ct; j++) {
                oss << array[i][j] << "\t";
            }
            oss << "\n";
        }

        return oss.str();
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
    std::optional<T> safeLook(const size_t i, const size_t j) const noexcept {
        if (i < rowCount() && j < array[i].size()) {
            return std::optional<T>(array[i][j]);
        } else {
            return std::optional<T>();
        }
    }

    // ELEMENT ACCESS, for k=0...(rowCt-1)*(colCt-1), traversing each row 
    T& size_at(const size_t index) {
        return array.at(index / rowCount()).at(index % rowCount());
    }

    const T& size_at(const size_t index) const {
        return array.at(index / rowCount()).at(index % rowCount());
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
            return EntireIterator(this, &(array[0][0]), colCount());
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
            return EntireIterator(this, &(array[rowCount()][0]), colCount());
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
            return ColumnIterator(this, &(array[0][fixed_col_j]), fixed_col_j);
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
            return ColumnIterator(this, &(array[rowCount()][fixed_col_j]), fixed_col_j);
        }
    }

    /* ============================
        Function Mapping
    ============================= */

    // Given a lambda function from type `T` to `S`, creates and returns a new `Array2D` of type `S` whose elements are equal to `func(t)` for each corresponding `t` in the input array.
    template <class S>
    Array2D<S> map(const std::function<S(T)>& func) const {
        Array2D<S> outArray;
        std::vector<S> temp_row;

        size_t row_ct = rowCount(); // tiny optimizations
        size_t col_ct = colCount();

        for (size_t i=0; i < row_ct; i++) {
            //fprintf(stderr, "Going to row %llu\n", i);
            temp_row.clear();
            for (size_t j=0; j < col_ct; j++) {
                //fprintf(stderr, "Applying `func` to element %llu,%llu\n", i, j);
                temp_row.push_back(func(array[i][j]));
            }
            outArray.append_row(temp_row);
        }

        return outArray;
    }

    // Given a lambda function from type `T` to `T` and an `Array2D` of type `T`, applies the function in-place to each element of this array.
    Array2D& map_inplace(const std::function<T(T)>& func) {
        size_t row_ct = rowCount(); // tiny optimizations
        size_t col_ct = colCount();

        for (size_t i=0; i < row_ct; i++) {
            for (size_t j=0; j < col_ct; j++) {
                array[i][j] = func(array[i][j]);
            }
        }

        return *this;
    }

};  // end class

};  // end namespace

int main() {
    using namespace Yelle;
    // constructor from nested initializer list (among several other constructors)
    Array2D<int> testArray = {{1,2,3},{3,4,5}};
    // safely append/insert a row that's too long (it'll be trimmed)
    testArray.append_row_safe({11,12,13,14}, 0);
    // safely append/insert a row that's too short (fills remaining spaces with default value, here -99)
    testArray.insert_row_safe(1, {21,22}, -99);

    // UNsafely append/insert a row...
    testArray.append_row({55,77,1345});
    // ...throwing an error if not exactly the right size
    try {
        printf("Trying to insert a row that's too long...\n");
        testArray.insert_row(4, {8,3,-14, 11, 12});
    } catch (std::length_error& e) {
        printf("Insertion failed: %s\n", e.what());
    }
    // ...or at a position that's too far
    try {
        printf("Trying to insert a row too low...\n");
        testArray.insert_row(99999, {18,21,34});
    } catch (std::out_of_range& e) {
        printf("Insertion failed: %s\n", e.what());
    }

    // initial printing
    printf("testArray has rows=%d, cols=%d\n", testArray.rowCount(), testArray.colCount());
    printf("testArray:\n%s", testArray.to_string().c_str());

    Array2D<int> copyArray = testArray;

    printf("Deleting column index=1\n");
    testArray.delete_col(1);
    printf("testArray after deletion:\n%s", testArray.to_string().c_str());
    printf("earlier copy of testArray before deletion:\n%s", copyArray.to_string().c_str());

    // create a new array with all values converted to floats
    Array2D<float> testArrayfloat = testArray.map<float>(
            [](const int x) { return static_cast<float>(x) + 0.5; }
        );
    printf("testArrayFloat:\n%s", testArrayfloat.to_string().c_str());

    // map the integer-squaring function in-place to the testArray
    testArray.map_inplace(
            [](int x) { return x*x; }
        );
    printf("testArray after squaring:\n%s", testArray.to_string().c_str());
    
    return 0;
}