// ============================================================================
//  LinkedList.h  -  DSA concept: DOUBLY LINKED LIST.
//  Used by:  Database (game history = "Previous Cases", newest case at the head)
// ============================================================================
#pragma once
#include <cstddef>
#include <stdexcept>

template <typename T>
class LinkedList {
    struct Node {
        T     data;
        Node* prev;
        Node* next;
        explicit Node(const T& d) : data(d), prev(nullptr), next(nullptr) {}
    };

    Node*       head_ = nullptr;
    Node*       tail_ = nullptr;
    std::size_t size_ = 0;

public:
    // Minimal forward iterator so the list works with range-based for loops.
    class ConstIterator {
        const Node* node_;
    public:
        explicit ConstIterator(const Node* n) : node_(n) {}
        const T& operator*() const { return node_->data; }
        ConstIterator& operator++() { node_ = node_->next; return *this; }
        bool operator!=(const ConstIterator& o) const { return node_ != o.node_; }
        bool operator==(const ConstIterator& o) const { return node_ == o.node_; }
    };

    LinkedList() = default;
    LinkedList(const LinkedList&)            = delete;
    LinkedList& operator=(const LinkedList&) = delete;
    ~LinkedList() { clear(); }

    void push_front(const T& value) {
        Node* n = new Node(value);
        n->next = head_;
        if (head_) head_->prev = n;
        else       tail_       = n;
        head_ = n;
        ++size_;
    }

    void push_back(const T& value) {
        Node* n = new Node(value);
        n->prev = tail_;
        if (tail_) tail_->next = n;
        else       head_       = n;
        tail_ = n;
        ++size_;
    }

    T pop_front() {
        if (!head_) throw std::underflow_error("LinkedList::pop_front on empty list");
        Node* old = head_;
        T value   = old->data;
        head_     = old->next;
        if (head_) head_->prev = nullptr;
        else       tail_       = nullptr;
        delete old;
        --size_;
        return value;
    }

    T pop_back() {
        if (!tail_) throw std::underflow_error("LinkedList::pop_back on empty list");
        Node* old = tail_;
        T value   = old->data;
        tail_     = old->prev;
        if (tail_) tail_->next = nullptr;
        else       head_       = nullptr;
        delete old;
        --size_;
        return value;
    }

    const T& front() const {
        if (!head_) throw std::underflow_error("LinkedList::front on empty list");
        return head_->data;
    }
    const T& back() const {
        if (!tail_) throw std::underflow_error("LinkedList::back on empty list");
        return tail_->data;
    }

    bool        empty() const { return head_ == nullptr; }
    std::size_t size()  const { return size_; }

    void clear() {
        while (head_) {
            Node* n = head_;
            head_   = head_->next;
            delete n;
        }
        tail_ = nullptr;
        size_ = 0;
    }

    ConstIterator begin() const { return ConstIterator(head_); }
    ConstIterator end()   const { return ConstIterator(nullptr); }
};
