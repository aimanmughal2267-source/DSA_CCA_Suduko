// ============================================================================
//  Stack.h  -  DSA concept: STACK (LIFO), implemented as a singly linked list.
//  Used by:  GameSession (Undo stack and Redo stack for player moves)
//            SudokuGraph::isConnected (iterative depth-first search)
// ============================================================================
#pragma once
#include <cstddef>
#include <stdexcept>

template <typename T>
class Stack {
    struct Node {
        T     data;
        Node* next;
        Node(const T& d, Node* n) : data(d), next(n) {}
    };

    Node*       head_ = nullptr;
    std::size_t size_ = 0;

public:
    Stack() = default;
    Stack(const Stack&)            = delete;   // owns raw nodes -> no shallow copies
    Stack& operator=(const Stack&) = delete;
    ~Stack() { clear(); }

    void push(const T& value) {
        head_ = new Node(value, head_);
        ++size_;
    }

    T pop() {
        if (!head_) throw std::underflow_error("Stack::pop on empty stack");
        Node* old = head_;
        T value   = old->data;
        head_     = old->next;
        delete old;
        --size_;
        return value;
    }

    const T& top() const {
        if (!head_) throw std::underflow_error("Stack::top on empty stack");
        return head_->data;
    }

    bool        empty() const { return head_ == nullptr; }
    std::size_t size()  const { return size_; }

    void clear() {
        while (head_) {
            Node* n = head_;
            head_   = head_->next;
            delete n;
        }
        size_ = 0;
    }
};
