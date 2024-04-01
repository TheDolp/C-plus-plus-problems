#include <iostream>
#include <exception>

template <typename T>
class Deque {
private:
    template <bool IsConst>
    class base_iterator {
    public:
        using pointer_type = std::conditional_t<IsConst, const T*, T*>;
        using reference_type = std::conditional_t<IsConst, const T&, T&>;
        using value_type = std::remove_reference_t<reference_type>;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = ptrdiff_t;

        reference_type operator*() const {
            return arr[row][col];
        };

        pointer_type operator->() const {
            return arr[row] + col;
        }

        base_iterator& operator++() {
            ++col;
            if (col == Deque<T>::BLOCK_SIZE) {
                col = 0;
                ++row;
            }
            return *this;
        }

        base_iterator operator++(int) {
            base_iterator it = *this;
            ++*this;
            return it;
        }

        base_iterator& operator--() {
            if (col == 0) {
                col = Deque<T>::BLOCK_SIZE - 1;
                --row;
            } else {
                --col;
            }
            return *this;
        }

        base_iterator operator--(int) {
            base_iterator it = *this;
            --*this;
            return it;
        }

        base_iterator& operator+=(int num) {
            row = (row * Deque<T>::BLOCK_SIZE + col + num) / Deque<T>::BLOCK_SIZE;
            col = (col + num) % Deque<T>::BLOCK_SIZE;
            return *this;
        }

        base_iterator operator+(int num) const {
            base_iterator result = *this;
            return result += num;
        }

        base_iterator& operator-=(int num) {
            row = (row * Deque<T>::BLOCK_SIZE + col - num) / Deque<T>::BLOCK_SIZE;
            col = ((col - num) % Deque<T>::BLOCK_SIZE + Deque<T>::BLOCK_SIZE) % Deque<T>::BLOCK_SIZE;
            return *this;
        }

        base_iterator operator-(int num) const {
            base_iterator result = *this;
            return result -= num;
        }

        difference_type operator-(const base_iterator& other) const {
            return (difference_type)row * Deque<T>::BLOCK_SIZE + col - other.row * Deque<T>::BLOCK_SIZE - other.col;
        }

        std::strong_ordering operator<=>(const base_iterator& other) const {
            return row * Deque<T>::BLOCK_SIZE + col <=> other.row * Deque<T>::BLOCK_SIZE + other.col;
        }

        bool operator==(const base_iterator& other) const = default;

        base_iterator& operator=(const base_iterator<false>& other) {
            arr = other.arr;
            row = other.row;
            col = other.col;
            return *this;
        }

        base_iterator(const base_iterator<false>& other): arr(other.arr), row(other.row), col(other.col) {}

        friend class Deque<T>;
    private:
        T** arr;
        size_t row;
        size_t col;
        base_iterator(T** arr, size_t row, size_t col):arr(arr), row(row), col(col) {}
    };

public:
    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    static const size_t BLOCK_SIZE = 64;
    size_t min_row;
    size_t min_col;
    size_t size_;
    size_t capacity;
    T** arr;

    void relocate() {
        size_t add = (capacity + 1) / 2;
        T** new_arr = nullptr;
        try {
            new_arr = new T*[capacity + add * 2];
        } catch (...) {
            throw;
        }
        std::fill(new_arr, new_arr + capacity + add * 2, nullptr);
        for (size_t i = 0; i < capacity; ++i) {
            new_arr[add + i] = arr[i];
        }
        arr = new_arr;
        min_row += add;
        capacity += add * 2;
    }

    void next() {
        ++min_col;
        if (min_col == BLOCK_SIZE) {
            min_col = 0;
            ++min_row;
        }
    }

    void prev() {
        if (min_col == 0) {
            min_col = BLOCK_SIZE;
            --min_row;
        }
        --min_col;
    }

    void make_exist(size_t row) {
        if (arr[row] == nullptr) {
            arr[row] = reinterpret_cast<T*>(new char[BLOCK_SIZE * sizeof(T)]);
        }
    }

    bool full_at_back() {
        return end().row == capacity;
    }

    void clear(size_t row, bool arr_created) {
        while (row) {
            delete[] reinterpret_cast<char*>(arr[--row]);
        }
        if (arr_created) delete[] arr;
    }

    void clear_elements(iterator last) {
        for (iterator del = begin(); del != last; ++del) {
            del->~T();
        }
        clear(capacity, true);
    }

    void init() {
        bool arr_created = false;
        size_t row = 0;
        try {
            arr = new T*[capacity];
            arr_created = true;
            for (; row < capacity; ++row) {
                arr[row] = reinterpret_cast<T*>(new char[BLOCK_SIZE * sizeof(T)]);
            }
        } catch (...) {
            clear(row, arr_created);
            throw;
        }
    }

public:
    Deque(): min_row(0), min_col(BLOCK_SIZE / 2), size_(0), capacity(1), arr(new T*[1]) {
        arr[0] = nullptr;
    }

    Deque(const Deque& other): min_row(other.min_row), min_col(other.min_col)
        , size_(other.size_), capacity(other.capacity){
        size_t row = 0;
        bool arr_created = false;
        try {
            arr = new T*[other.capacity];
            arr_created = true;
            for (; row < capacity; ++row) {
                if (other.arr[row] == nullptr) {
                    arr[row] = nullptr;
                } else {
                    arr[row] = reinterpret_cast<T*>(new char[BLOCK_SIZE * sizeof(T)]);
                }
            }
        } catch (...) {
            clear(row, arr_created);
            throw;
        }

        iterator it = begin();
        try {
            for (const_iterator j = other.cbegin(); j != other.cend(); ++it, ++j) {
                new(&*it) T(*j);
            }
        } catch (...) {
            clear_elements(it);
            throw;
        }
    }

    Deque(int size): min_row(0), min_col(0), size_(size), capacity((size + BLOCK_SIZE - 1) / BLOCK_SIZE) {
        init();

        iterator it = begin();
        try {
            for (; it != end(); ++it) {
                new(&*it) T();
            }
        } catch (...) {
            clear_elements(it);
            throw;
        }
    }

    Deque(int size, const T& elem): min_row(0), min_col(0), size_(size), capacity((size + BLOCK_SIZE - 1) / BLOCK_SIZE) {
        init();

        iterator it = begin();
        try {
            for (; it != end(); ++it) {
                new(&*it) T(elem);
            }
        } catch (...) {
            clear_elements(it);
            throw;
        }
    }

    Deque& operator=(Deque other) {
        swap(other);
        return *this;
    }

    ~Deque() {
        clear_elements(end());
    }

    void swap(Deque& other) {
        std::swap(arr, other.arr);
        std::swap(min_row, other.min_row);
        std::swap(min_col, other.min_col);
        std::swap(size_, other.size_);
        std::swap(capacity, other.capacity);
    }


    size_t size() const {
        return size_;
    }

    T& operator[](size_t pos) {
        return *(begin() + pos);
    };

    const T& operator[](size_t pos) const {
        return *(begin() + pos);
    }

    T& at(size_t pos) {
        if (size_ <= pos) throw std::out_of_range("");
        return *(begin() + pos);
    };

    const T& at(size_t pos) const {
        if (size_ <= pos) throw std::out_of_range("");
        return *(begin() + pos);
    }


    iterator begin() {
        return {arr, min_row, min_col};
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator cbegin() const {
        return {arr, min_row, min_col};
    }

    iterator end() {
        return begin() + size_;
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cend() const {
        return cbegin() + size_;
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const {
        return crbegin();
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const {
        return crend();
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }


    void push_front(const T& elem) {
        T** previous = arr;
        size_t previous_row = min_row;
        size_t old_capacity = capacity;
        if (min_row == 0 && min_col == 0) {
            relocate();
        }
        bool new_row = false;
        try {
            iterator it = --begin();
            if (arr[it.row] == nullptr) new_row = true;
            make_exist(it.row);
            new(&*it) T(elem);
            prev();
            ++size_;
            if (arr != previous) delete[] previous;
        } catch (...) {
            if (new_row) {
                delete[] reinterpret_cast<char*>(arr[begin().row - 1]);
                arr[begin().row - 1] = nullptr;
            }
            if (arr != previous) {
                delete[] arr;
                arr = previous;
            }
            min_row = previous_row;
            capacity = old_capacity;
            throw;
        }
    }

    void push_back(const T& elem) {
        T** previous = arr;
        size_t previous_row = min_row;
        size_t old_capacity = capacity;
        if (full_at_back()) {
            relocate();
        }
        bool new_row = false;
        try {
            if (arr[end().row] == nullptr) new_row = true;
            make_exist(end().row);
            new(&*end()) T(elem);
            ++size_;
            if (arr != previous) delete[] previous;
        } catch (...) {
            if (new_row) {
               delete[] reinterpret_cast<char*>(arr[end().row]);
               arr[end().row] = nullptr;
            }
            if (arr != previous) {
                delete[] arr;
                arr = previous;
            }
            min_row = previous_row;
            capacity = old_capacity;
            throw;
        }
    }

    void pop_front() {
        begin()->~T();
        next();
        --size_;
    }

    void pop_back() {
        (--end())->~T();
        --size_;
    }

    void insert(iterator it, const T& elem) {
        if (full_at_back()) {
            it.row += (capacity + 1) / 2;
        }
        push_back(elem);

        it.arr = arr;
        for (iterator iter = --end(); iter != it; --iter) {
            std::swap(*iter, *(iter - 1));
        }
    }

    void erase(iterator it) {
        while (it + 1 != end()) {
            std::swap(*it, *(it + 1));
            ++it;
        }
        pop_back();
    }
};