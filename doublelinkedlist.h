#ifndef DOUBLE_LINKED_LIST_H
#define DOUBLE_LINKED_LIST_H

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <functional>   // std::less, std::greater
#include <type_traits>  // std::void_t, enable_if_t, is_same, is_constructible
#include <utility>      // std::pair
#include <ostream>      // std::ostream

/* ---------- Deducción del T del trait ---------- */
// Caso general
template <typename Trait, typename = void>
struct trait_value_type {};

// Si el trait define 'type'
template <typename Trait>
struct trait_value_type<Trait, std::void_t<typename Trait::type>> {
    using type = typename Trait::type;
};

// Si el trait define 'ValueType'
template <typename Trait>
struct trait_value_type<Trait, std::void_t<typename Trait::ValueType>> {
    using type = typename Trait::ValueType;
};

// Si el trait define 'T'
template <typename Trait>
struct trait_value_type<Trait, std::void_t<typename Trait::T>> {
    using type = typename Trait::T;
};

// Si el trait es un template de 1 parámetro: Trait<U> -> U
template <template<class> class TraitTmpl, class U>
struct trait_value_type<TraitTmpl<U>, void> {
    using type = U;
};

/* ---------- Deducción del comparador del trait ---------- */
template <typename Trait, typename Value, typename = void>
struct trait_less {};

template <typename Trait, typename Value>
struct trait_less<Trait, Value, std::void_t<typename Trait::less>> {
    using type = typename Trait::less;
};

template <typename Trait, typename Value>
struct trait_less<Trait, Value, std::void_t<typename Trait::Less>> {
    using type = typename Trait::Less;
};

template <typename Trait, typename Value>
struct trait_less<Trait, Value, std::void_t<typename Trait::Compare>> {
    using type = typename Trait::Compare;
};

// Fallback: std::less<Value>
template <typename Trait, typename Value>
struct trait_less<Trait, Value, void> {
    using type = std::less<Value>;
};

/* ---------- Lista Doblemente Enlazada con comparador ---------- */
template <typename T, typename Compare = std::less<T>>
class DoublyLinkedList {
private:
    struct Node {
        T value;
        Node* prev;
        Node* next;
        explicit Node(const T& v, Node* p=nullptr, Node* n=nullptr)
            : value(v), prev(p), next(n) {}
        explicit Node(T&& v, Node* p=nullptr, Node* n=nullptr)
            : value(std::move(v)), prev(p), next(n) {}
    };

    Node* head_{nullptr};
    Node* tail_{nullptr};
    std::size_t size_{0};
    Compare comp_{};

    void link_between(Node* left, Node* right, Node* mid) noexcept {
        mid->prev = left;
        mid->next = right;
        if (left) left->next = mid; else head_ = mid;
        if (right) right->prev = mid; else tail_ = mid;
        ++size_;
    }

    void unlink(Node* n) noexcept {
        Node* L = n->prev;
        Node* R = n->next;
        if (L) L->next = R; else head_ = R;
        if (R) R->prev = L; else tail_ = L;
        --size_;
    }

public:
    template <bool IsConst>
    class basic_iterator {
        using NodePtr = std::conditional_t<IsConst, const Node*, Node*>;
        NodePtr node_{nullptr};
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = std::conditional_t<IsConst, const T*, T*>;
        using reference         = std::conditional_t<IsConst, const T&, T&>;

        basic_iterator() = default;
        explicit basic_iterator(NodePtr n) : node_(n) {}
        reference operator*()  const { return const_cast<reference>(node_->value); }
        pointer   operator->() const { return &const_cast<reference>(node_->value); }
        basic_iterator& operator++() { node_ = node_ ? node_->next : nullptr; return *this; }
        basic_iterator  operator++(int){ auto t=*this; ++(*this); return t; }
        basic_iterator& operator--() { node_ = node_ ? node_->prev : node_; return *this; }
        basic_iterator  operator--(int){ auto t=*this; --(*this); return t; }
        friend bool operator==(const basic_iterator& a, const basic_iterator& b){ return a.node_==b.node_; }
        friend bool operator!=(const basic_iterator& a, const basic_iterator& b){ return !(a==b); }
        template<typename, typename> friend class DoublyLinkedList;
    };

    using iterator = basic_iterator<false>;
    using const_iterator = basic_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    DoublyLinkedList() = default;
    DoublyLinkedList(std::initializer_list<T> il) { for (auto& v: il) push_back(v); }
    ~DoublyLinkedList() { clear(); }

    std::size_t size()  const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    iterator begin() noexcept { return iterator(head_); }
    iterator end()   noexcept { return iterator(nullptr); }
    const_iterator begin() const noexcept { return const_iterator(head_); }
    const_iterator end()   const noexcept { return const_iterator(nullptr); }
    const_iterator cbegin() const noexcept { return const_iterator(head_); }
    const_iterator cend()   const noexcept { return const_iterator(nullptr); }

    // reverse
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    reverse_iterator rend()   noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend()   const noexcept { return const_reverse_iterator(begin()); }

    // extremos
    T& front() {
        if (!head_) throw std::out_of_range("front en lista vacía");
        return head_->value;
    }
    const T& front() const {
        if (!head_) throw std::out_of_range("front en lista vacía");
        return head_->value;
    }
    T& back() {
        if (!tail_) throw std::out_of_range("back en lista vacía");
        return tail_->value;
    }
    const T& back() const {
        if (!tail_) throw std::out_of_range("back en lista vacía");
        return tail_->value;
    }

    void clear() noexcept {
        Node* cur = head_;
        while (cur) {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        head_ = tail_ = nullptr;
        size_ = 0;
    }

    void push_front(const T& v) { link_between(nullptr, head_, new Node(v)); }
    void push_back (const T& v) { link_between(tail_, nullptr, new Node(v)); }

    void pop_front() {
        if (!head_) throw std::out_of_range("pop_front en lista vacía");
        Node* n = head_; unlink(n); delete n;
    }
    void pop_back() {
        if (!tail_) throw std::out_of_range("pop_back en lista vacía");
        Node* n = tail_; unlink(n); delete n;
    }

    // Inserción ordenada (traits Asc/Desc)
    iterator insert_ordered(const T& v) {
        Node* cur = head_;
        while (cur && !comp_(v, cur->value))
            cur = cur->next;
        if (!cur) { // al final
            link_between(tail_, nullptr, new Node(v));
            return iterator(tail_);
        } else {
            link_between(cur->prev, cur, new Node(v));
            return iterator(cur->prev);
        }
    }
    // Alias legacy
    iterator Insert(const T& v) { return insert_ordered(v); }

    // Inserción genérica ANTES de pos (no ordenada)
    iterator insert(iterator pos, const T& v) {
        if (pos.node_ == nullptr) {
            link_between(tail_, nullptr, new Node(v));
            return iterator(tail_);
        }
        Node* right = const_cast<Node*>(pos.node_);
        link_between(right->prev, right, new Node(v));
        return iterator(right->prev);
    }

    // Borrado del nodo en pos; devuelve iterador al siguiente
    iterator erase(iterator pos) {
        if (pos.node_ == nullptr) throw std::out_of_range("erase(end())");
        Node* n = const_cast<Node*>(pos.node_);
        iterator ret(n->next);
        unlink(n); delete n;
        return ret;
    }

    void swap(DoublyLinkedList& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }
};

/* ---------- Alias para TRAITS ---------- */
template <typename Trait>
using CDoubleLinkedList =
    DoublyLinkedList<
        typename trait_value_type<Trait>::type,
        typename trait_less<Trait, typename trait_value_type<Trait>::type>::type
    >;

/* ---------- operator<< para imprimir la lista ---------- */
template <typename T, typename Compare>
std::ostream& operator<<(std::ostream& os, const DoublyLinkedList<T,Compare>& lst){
    os << "[";
    bool first = true;
    for (auto it = lst.begin(); it != lst.end(); ++it){
        if (!first) os << ", ";
        first = false;
        os << *it;
    }
    os << "]";
    return os;
}

#endif // DOUBLE_LINKED_LIST_H
