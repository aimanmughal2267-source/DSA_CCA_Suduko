// ============================================================================
//  MaxHeap.h  -  DSA concept: BINARY MAX-HEAP (priority queue).
//  Supports O(log n) push/pop and O(n) bottom-up heap construction (heapify).
//  Used by:  Database::topDetectives (leaderboard: repeatedly extract the best)
// ============================================================================
#pragma once
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename T, typename Less = std::less<T>>
class MaxHeap {
    std::vector<T> a_;
    Less           less_;

    void siftUp(std::size_t i) {
        while (i > 0) {
            std::size_t parent = (i - 1) / 2;
            if (!less_(a_[parent], a_[i])) break;
            std::swap(a_[parent], a_[i]);
            i = parent;
        }
    }
    void siftDown(std::size_t i) {
        const std::size_t n = a_.size();
        for (;;) {
            std::size_t l = 2 * i + 1, r = 2 * i + 2, best = i;
            if (l < n && less_(a_[best], a_[l])) best = l;
            if (r < n && less_(a_[best], a_[r])) best = r;
            if (best == i) break;
            std::swap(a_[i], a_[best]);
            i = best;
        }
    }

public:
    MaxHeap() = default;

    void push(const T& v) {
        a_.push_back(v);
        siftUp(a_.size() - 1);
    }

    T pop() {
        if (a_.empty()) throw std::underflow_error("MaxHeap::pop on empty heap");
        T top = a_.front();
        a_.front() = a_.back();
        a_.pop_back();
        if (!a_.empty()) siftDown(0);
        return top;
    }

    const T& top() const {
        if (a_.empty()) throw std::underflow_error("MaxHeap::top on empty heap");
        return a_.front();
    }

    // Floyd's O(n) heap construction.
    void build(const std::vector<T>& items) {
        a_ = items;
        if (a_.size() < 2) return;
        for (std::size_t i = a_.size() / 2; i-- > 0;) siftDown(i);
    }

    bool        empty() const { return a_.empty(); }
    std::size_t size()  const { return a_.size(); }
    void        clear()       { a_.clear(); }
};
