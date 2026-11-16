// ============================================================================
//  HashTable.h  -  DSA concept: HASH TABLE with separate chaining.
//  Keys are std::string (djb2 hash), values are any copyable type.
//  The table grows (rehashes) automatically when the load factor exceeds 0.75.
//  Used by:  Database (player profiles keyed by detective name -> O(1) lookup)
//            Database::topDetectives (set of already-ranked detectives)
// ============================================================================
#pragma once
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

template <typename V>
class HashTable {
    struct Node {
        std::string key;
        V           value;
        Node*       next;
        Node(const std::string& k, const V& v, Node* n) : key(k), value(v), next(n) {}
    };

    std::vector<Node*> buckets_;
    std::size_t        count_ = 0;

    static std::size_t hashKey(const std::string& s) {      // djb2
        std::size_t h = 5381;
        for (unsigned char c : s) h = ((h << 5) + h) + c;
        return h;
    }

    // Relinks existing nodes into a bigger bucket array (node addresses stay stable).
    void rehash(std::size_t newCapacity) {
        std::vector<Node*> fresh(newCapacity, nullptr);
        for (Node* head : buckets_) {
            while (head) {
                Node* next       = head->next;
                std::size_t idx  = hashKey(head->key) % newCapacity;
                head->next       = fresh[idx];
                fresh[idx]       = head;
                head             = next;
            }
        }
        buckets_.swap(fresh);
    }

public:
    explicit HashTable(std::size_t capacity = 16)
        : buckets_(capacity < 4 ? 4 : capacity, nullptr) {}
    HashTable(const HashTable&)            = delete;
    HashTable& operator=(const HashTable&) = delete;
    ~HashTable() { clear(); }

    // Returns true if a NEW key was inserted, false if an existing key was overwritten.
    bool insert(const std::string& key, const V& value) {
        if (V* existing = find(key)) {
            *existing = value;
            return false;
        }
        if (static_cast<double>(count_ + 1) / static_cast<double>(buckets_.size()) > 0.75)
            rehash(buckets_.size() * 2);
        std::size_t idx = hashKey(key) % buckets_.size();
        buckets_[idx]   = new Node(key, value, buckets_[idx]);
        ++count_;
        return true;
    }

    V* find(const std::string& key) {
        for (Node* n = buckets_[hashKey(key) % buckets_.size()]; n; n = n->next)
            if (n->key == key) return &n->value;
        return nullptr;
    }
    const V* find(const std::string& key) const {
        for (const Node* n = buckets_[hashKey(key) % buckets_.size()]; n; n = n->next)
            if (n->key == key) return &n->value;
        return nullptr;
    }

    bool contains(const std::string& key) const { return find(key) != nullptr; }

    bool erase(const std::string& key) {
        std::size_t idx = hashKey(key) % buckets_.size();
        Node* prev = nullptr;
        for (Node* n = buckets_[idx]; n; prev = n, n = n->next) {
            if (n->key == key) {
                if (prev) prev->next = n->next;
                else      buckets_[idx] = n->next;
                delete n;
                --count_;
                return true;
            }
        }
        return false;
    }

    std::size_t size()        const { return count_; }
    bool        empty()       const { return count_ == 0; }
    std::size_t bucketCount() const { return buckets_.size(); }
    double      loadFactor()  const { return static_cast<double>(count_) / static_cast<double>(buckets_.size()); }

    // Visits every (key, value) pair. Order is the internal bucket order.
    template <typename F>
    void forEach(F fn) const {
        for (const Node* head : buckets_)
            for (const Node* n = head; n; n = n->next)
                fn(n->key, n->value);
    }

    void clear() {
        for (Node*& head : buckets_) {
            while (head) {
                Node* n = head;
                head    = head->next;
                delete n;
            }
        }
        count_ = 0;
    }
};
