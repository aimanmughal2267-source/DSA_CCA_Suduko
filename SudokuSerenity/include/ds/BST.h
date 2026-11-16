// ============================================================================
//  BST.h  -  DSA concept: BINARY SEARCH TREE (order-statistic variant).
//  Every node stores the size of its subtree, so the RANK of a value
//  (how many stored values are greater) is found in O(height).
//  Traversal callbacks must return bool: true = keep going, false = stop.
//  Used by:  Database (all solved-case scores; in-order traversal = ranking)
// ============================================================================
#pragma once
#include <algorithm>
#include <cstddef>
#include <functional>

template <typename T, typename Less = std::less<T>>
class BST {
    struct Node {
        T           data;
        Node*       left    = nullptr;
        Node*       right   = nullptr;
        std::size_t subtree = 1;
        explicit Node(const T& d) : data(d) {}
    };

    Node*       root_ = nullptr;
    std::size_t size_ = 0;
    Less        less_;

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }
    static int heightOf(const Node* n) {
        if (!n) return 0;
        return 1 + std::max(heightOf(n->left), heightOf(n->right));
    }
    static std::size_t sizeOf(const Node* n) { return n ? n->subtree : 0; }

    template <typename F>
    static bool inorderRec(const Node* n, F& fn) {
        if (!n) return true;
        if (!inorderRec(n->left, fn)) return false;
        if (!fn(n->data)) return false;
        return inorderRec(n->right, fn);
    }
    template <typename F>
    static bool reverseRec(const Node* n, F& fn) {
        if (!n) return true;
        if (!reverseRec(n->right, fn)) return false;
        if (!fn(n->data)) return false;
        return reverseRec(n->left, fn);
    }

public:
    BST() = default;
    BST(const BST&)            = delete;
    BST& operator=(const BST&) = delete;
    ~BST() { clear(); }

    void insert(const T& value) {
        Node* fresh = new Node(value);
        ++size_;
        if (!root_) { root_ = fresh; return; }
        Node* cur = root_;
        for (;;) {
            ++cur->subtree;
            if (less_(value, cur->data)) {
                if (!cur->left)  { cur->left = fresh;  return; }
                cur = cur->left;
            } else {
                if (!cur->right) { cur->right = fresh; return; }
                cur = cur->right;
            }
        }
    }

    bool contains(const T& value) const {
        const Node* cur = root_;
        while (cur) {
            if (less_(value, cur->data))      cur = cur->left;
            else if (less_(cur->data, value)) cur = cur->right;
            else return true;
        }
        return false;
    }

    // Number of stored values strictly greater than `value` (rank - 1).
    std::size_t countGreater(const T& value) const {
        const Node* cur = root_;
        std::size_t count = 0;
        while (cur) {
            if (less_(value, cur->data)) {            // cur and its right subtree are greater
                count += 1 + sizeOf(cur->right);
                cur    = cur->left;
            } else if (less_(cur->data, value)) {
                cur = cur->right;
            } else {                                  // equal: everything on the right is greater
                count += sizeOf(cur->right);
                break;
            }
        }
        return count;
    }

    const T* minimum() const {
        const Node* cur = root_;
        if (!cur) return nullptr;
        while (cur->left) cur = cur->left;
        return &cur->data;
    }
    const T* maximum() const {
        const Node* cur = root_;
        if (!cur) return nullptr;
        while (cur->right) cur = cur->right;
        return &cur->data;
    }

    template <typename F> void inorder(F fn) const        { inorderRec(root_, fn); }   // ascending
    template <typename F> void reverseInorder(F fn) const { reverseRec(root_, fn); }   // descending

    std::size_t size()   const { return size_; }
    bool        empty()  const { return size_ == 0; }
    int         height() const { return heightOf(root_); }

    void clear() {
        destroy(root_);
        root_ = nullptr;
        size_ = 0;
    }
};
