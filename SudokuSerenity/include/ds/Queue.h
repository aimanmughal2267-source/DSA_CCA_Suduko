// ============================================================================
//  Queue.h  -  DSA concept: QUEUE (FIFO), implemented with head/tail pointers.
//  Used by:  GameSession (queue of pending Investigation Clues / hints)
//            SudokuGraph::bfsWithin (breadth-first search frontier)
// ============================================================================
#pragma once
#include <cstddef>
#include <stdexcept>

template <typename T>
class Queue {
    struct Node {
        T     data;
        Node* next;
        explicit Node(const T& d) : data(d), next(nullptr) {}
    };

    Node*       head_ = nullptr;
    Node*       tail_ = nullptr;
    std::size_t size_ = 0;

public:
    Queue() = default;
    Queue(const Queue&)            = delete;
    Queue& operator=(const Queue&) = delete;
    ~Queue() { clear(); }

    void enqueue(const T& value) {
        Node* n = new Node(value);
        if (tail_) tail_->next = n;
        else       head_       = n;
        tail_ = n;
        ++size_;
    }

    T dequeue() {
        if (!head_) throw std::underflow_error("Queue::dequeue on empty queue");
        Node* old = head_;
        T value   = old->data;
        head_     = old->next;
        if (!head_) tail_ = nullptr;
        delete old;
        --size_;
        return value;
    }

    const T& front() const {
        if (!head_) throw std::underflow_error("Queue::front on empty queue");
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
        tail_ = nullptr;
        size_ = 0;
    }
};
