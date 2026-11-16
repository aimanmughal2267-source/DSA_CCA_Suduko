// ============================================================================
//  Algorithms.h - generic SEARCHING and SORTING algorithms (templates).
//     binarySearch, linearSearch, mergeSort (stable), quickSort
// ============================================================================
#pragma once
#include <cstddef>
#include <utility>
#include <vector>

namespace Algo {

// cmp(element, key) returns <0, 0, >0. The vector must be sorted consistently.
template <typename T, typename Key, typename Cmp>
int binarySearch(const std::vector<T>& v, const Key& key, Cmp cmp) {
    int lo = 0, hi = static_cast<int>(v.size()) - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int c = cmp(v[mid], key);
        if (c == 0) return mid;
        if (c < 0) lo = mid + 1; else hi = mid - 1;
    }
    return -1;
}

template <typename T, typename Pred>
int linearSearch(const std::vector<T>& v, Pred pred) {
    for (std::size_t i = 0; i < v.size(); ++i)
        if (pred(v[i])) return static_cast<int>(i);
    return -1;
}

template <typename T, typename Less>
void mergeSortRec(std::vector<T>& v, std::vector<T>& tmp, std::size_t lo, std::size_t hi, Less& less) {
    if (hi - lo < 2) return;
    std::size_t mid = lo + (hi - lo) / 2;
    mergeSortRec(v, tmp, lo, mid, less);
    mergeSortRec(v, tmp, mid, hi, less);
    std::size_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi) tmp[k++] = less(v[j], v[i]) ? v[j++] : v[i++];
    while (i < mid) tmp[k++] = v[i++];
    while (j < hi)  tmp[k++] = v[j++];
    for (k = lo; k < hi; ++k) v[k] = tmp[k];
}
template <typename T, typename Less>
void mergeSort(std::vector<T>& v, Less less) {
    std::vector<T> tmp(v);
    mergeSortRec(v, tmp, 0, v.size(), less);
}

template <typename T, typename Less>
void quickSortRec(std::vector<T>& v, int lo, int hi, Less& less) {
    while (lo < hi) {
        if (hi - lo < 8) {                                   // insertion sort for tiny ranges
            for (int i = lo + 1; i <= hi; ++i) {
                T key = v[i];
                int j = i - 1;
                while (j >= lo && less(key, v[j])) { v[j + 1] = v[j]; --j; }
                v[j + 1] = key;
            }
            return;
        }
        std::swap(v[lo + (hi - lo) / 2], v[hi]);
        T pivot = v[hi];
        int store = lo;
        for (int i = lo; i < hi; ++i)
            if (less(v[i], pivot)) std::swap(v[i], v[store++]);
        std::swap(v[store], v[hi]);
        if (store - lo < hi - store) { quickSortRec(v, lo, store - 1, less); lo = store + 1; }
        else                         { quickSortRec(v, store + 1, hi, less); hi = store - 1; }
    }
}
template <typename T, typename Less>
void quickSort(std::vector<T>& v, Less less) {
    if (v.size() > 1) quickSortRec(v, 0, static_cast<int>(v.size()) - 1, less);
}

} // namespace Algo
