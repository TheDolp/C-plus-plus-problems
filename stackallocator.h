#include <iostream>

template <size_t N>
class StackStorage {
    char arr[N];
    size_t begin;
public:

    StackStorage(): arr(), begin(0) {}

    StackStorage(const StackStorage& other) = delete;

    char* get_pointer(size_t size, size_t align_) {
        void* ptr = &arr[begin];
        size_t space = N - begin - size;
        char* result = reinterpret_cast<char*>(std::align(align_, size, ptr, space));
        begin = N - space;
        return result;
    }
};

template <typename T, size_t N>
class StackAllocator {
    StackStorage<N>* storage;

public:
    using value_type = T;

    StackAllocator() {};

    explicit StackAllocator(StackStorage<N>& storage): storage(&storage) {}

    template<typename U>
    StackAllocator(const StackAllocator<U, N>& other): storage(other.storage) {}

    template<typename U>
    StackAllocator& operator=(const StackAllocator<U, N>& other) {
        storage = other.storage;
        return *this;
    }

    T* allocate(size_t count) {
        return reinterpret_cast<T*>(storage->get_pointer(count * sizeof(T), alignof(T)));
    }

    void deallocate(T*, size_t) {}

    template <typename U>
    bool operator==(const StackAllocator<U, N>& other) {
        return storage == other.storage;
    }

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

    template<typename U, size_t K>
    friend class StackAllocator;
};

template <typename T, typename Alloc = std::allocator<T>>
class List {
    struct BaseNode {
        BaseNode* prev;
        BaseNode* next;
        BaseNode(): prev(this), next(this) {}
        BaseNode(const BaseNode& other): prev(other.prev), next(other.next) {}
    };

    struct Node: public BaseNode {
        T value;
        Node(): BaseNode(), value() {}
        Node(const T& value): BaseNode(), value(value) {}
    };

    template <bool IsConst>
    class base_iterator {
    public:
        using pointer_type = std::conditional_t<IsConst, const T*, T*>;
        using reference_type = std::conditional_t<IsConst, const T&, T&>;
        using value_type = std::remove_reference_t<reference_type>;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;

        reference_type operator*() const {
            return node->value;
        };

        pointer_type operator->() const {
            return &(node->value);
        }

        base_iterator& operator++() {
            node = static_cast<Node*>(node->next);
            return *this;
        }

        base_iterator operator++(int) {
            base_iterator it = *this;
            ++*this;
            return it;
        }

        base_iterator& operator--() {
            node = static_cast<Node*>(node->prev);
            return *this;
        }

        base_iterator operator--(int) {
            base_iterator it = *this;
            --*this;
            return it;
        }

        bool operator==(const base_iterator& other) const = default;

        bool operator!=(const base_iterator& other) const {
            return node != other.node;
        }

        base_iterator& operator=(const base_iterator<false>& other) {
            node = other.node;
            return *this;
        }

        base_iterator(const base_iterator<false>& other): node(other.node) {}

        base_iterator(Node* node): node(node) {}

        friend class List<T, Alloc>;
    private:
        Node* node;
    };

public:
    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    List() = default;

    List(size_t sz): fake(), size_(0), alloc() {
        try {
            for (size_t pos = 0; pos < sz; ++pos) {
                add_node(alloc, &fake);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    List(size_t sz, const T& value): fake(), size_(0), alloc() {
        try {
            for (size_t pos = 0; pos < sz; ++pos) {
                add_node(alloc, &fake, value);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    List(Alloc alloc): fake(), size_(0), alloc(alloc) {}

    List(size_t sz, Alloc alloc_): fake(), size_(0), alloc(alloc_) {
        try {
            for (size_t pos = 0; pos < sz; ++pos) {
                add_node(alloc, &fake);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    List(size_t sz, const T& value, Alloc alloc): fake(), size_(0), alloc(alloc) {
        try {
            for (size_t pos = 0; pos < sz; ++pos) {
                add_node(alloc, &fake, value);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    List(const List& other): fake(), size_(0), alloc(alloc_traits::select_on_container_copy_construction(other.alloc)){
        try {
            for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
                add_node(alloc, &fake, *it);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    List& operator=(const List& other) {
        Alloc_ newalloc = alloc_traits::propagate_on_container_copy_assignment::value ? other.alloc : alloc;
        BaseNode newfake;
        try {
            for (const_iterator it = other.begin(); it != other.end(); ++it) {
                add_node(newalloc, &newfake, *it);
                --size_;
            }
        } catch (...) {
            for (iterator it = iterator(static_cast<Node*>(newfake.next)); it.node != &newfake;) {
                iterator prev = it++;
                alloc_traits::destroy(newalloc, prev.node);
                alloc_traits::deallocate(newalloc, prev.node, 1);
            }
            throw;
        }
        clear();
        size_ = other.size_;
        alloc = newalloc;
        merge(newfake.prev, &fake, newfake.next);
        return *this;
    }

    ~List() {
        clear();
    }

    void push_back(const T& value) {
        add_node(alloc,&fake, value);
    }

    void push_front(const T& value) {
        add_node(alloc, fake.next, value);
    }

    void insert(const_iterator it, const T& value) {
        add_node(alloc, it.node, value);
    }

    void pop_front() {
        del_node(static_cast<Node*>(fake.next));
    }

    void pop_back() {
        del_node(static_cast<Node*>(fake.prev));
    }

    void erase(const_iterator it) {
        del_node(it.node);
    }

    iterator begin() {
        return {static_cast<Node*>(fake.next)};
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator cbegin() const {
        return {static_cast<Node*>(fake.next)};
    }

    iterator end() {
        return {static_cast<Node*>(&fake)};
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cend() const {
        return {static_cast<Node*>(const_cast<BaseNode*>(&fake))};
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

    size_t size() const {
        return size_;
    }

private:
    BaseNode fake;
    size_t size_;
    using Alloc_ = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    using alloc_traits = typename std::allocator_traits<Alloc_>;
    [[no_unique_address]]Alloc_ alloc;

    void clear() {
        for (iterator it = begin(); it != end();) {
            iterator prev = it++;
            alloc_traits::destroy(alloc, prev.node);
            alloc_traits::deallocate(alloc, prev.node, 1);
        }
    }

    void merge(BaseNode* left, BaseNode* middle, BaseNode* right) {
        left->next = middle;
        middle->prev = left;
        middle->next = right;
        right->prev = middle;
    }

    void del_node(Node* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        alloc_traits::destroy(alloc, node);
        alloc_traits::deallocate(alloc, node, 1);
        --size_;
    }

    void add_node(Alloc_ alloc, BaseNode* next, const T& value) {
        Node* node = alloc_traits::allocate(alloc, 1);
        try {
            alloc_traits::construct(alloc, node, value);
        } catch (...) {
            alloc_traits::deallocate(alloc, node, 1);
            throw;
        }
        merge(next->prev, static_cast<BaseNode*>(node), next);
        ++size_;
    }

    void add_node(Alloc_ alloc, BaseNode* next) {
        Node* node = alloc_traits::allocate(alloc, 1);
        try {
            alloc_traits::construct(alloc, node);
        } catch (...) {
            alloc_traits::deallocate(alloc, node, 1);
            throw;
        }
        merge(next->prev, static_cast<BaseNode*>(node), next);
        ++size_;
    }
public:
    Alloc_ get_allocator() const {
        return alloc;
    }
};
