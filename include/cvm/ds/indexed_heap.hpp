#pragma once

#include "key_index_map.hpp"
#include <vector>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <utility>

namespace cvm::ds {

using rank_t = std::uint64_t;

/**
 * @brief Node stored in the max-heap.
 * Bookkeeping order: (rank, serial).
 * Serial ensures deterministic tie-breaking without affecting scientific acceptance.
 */
template <typename Key>
struct HeapNode {
    Key key{};
    rank_t rank{0};
    std::uint64_t serial{0};
};

/**
 * @brief Indexed max-heap with fast O(1) key lookup and O(log s) arbitrary deletion.
 *
 * Implements PLAN Section 84, 85 and AGENTS.md Case 08 requirements:
 * - Max-heap property: parent.rank >= child.rank
 * - Reverse key -> heap slot mapping via KeyIndexMap
 * - Efficient max extraction and purge of elements with rank >= cutoff
 * - Zero general-purpose allocations on steady-state updates after reserve()
 */
template <typename Key = std::uint64_t,
          typename Hash = KeyHasher<Key>,
          typename Equal = std::equal_to<Key>>
class IndexedHeap {
private:
    std::vector<HeapNode<Key>> heap_;
    KeyIndexMap<Key, std::uint32_t, Hash, Equal> index_map_;
    std::uint64_t next_serial_{0};

    [[nodiscard]] bool higher_priority(const HeapNode<Key>& a, const HeapNode<Key>& b) const noexcept {
        if (a.rank != b.rank) {
            return a.rank > b.rank;
        }
        return a.serial < b.serial;
    }

    void swap_nodes(std::size_t i, std::size_t j) {
        std::swap(heap_[i], heap_[j]);
        index_map_.insert_or_assign(heap_[i].key, static_cast<std::uint32_t>(i));
        index_map_.insert_or_assign(heap_[j].key, static_cast<std::uint32_t>(j));
    }

    void sift_up(std::size_t i) {
        while (i > 0) {
            const std::size_t parent = (i - 1) / 2;
            if (higher_priority(heap_[i], heap_[parent])) {
                swap_nodes(i, parent);
                i = parent;
            } else {
                break;
            }
        }
    }

    void sift_down(std::size_t i) {
        const std::size_t n = heap_.size();
        while (true) {
            const std::size_t left = 2 * i + 1;
            const std::size_t right = 2 * i + 2;
            std::size_t best = i;

            if (left < n && higher_priority(heap_[left], heap_[best])) {
                best = left;
            }
            if (right < n && higher_priority(heap_[right], heap_[best])) {
                best = right;
            }

            if (best != i) {
                swap_nodes(i, best);
                i = best;
            } else {
                break;
            }
        }
    }

public:
    explicit IndexedHeap(std::size_t capacity = 0)
        : heap_(), index_map_(capacity)
    {
        if (capacity > 0) {
            heap_.reserve(capacity);
        }
    }

    void reserve(std::size_t capacity) {
        heap_.reserve(capacity);
        index_map_.reserve(capacity);
    }

    void clear() noexcept {
        heap_.clear();
        index_map_.clear();
        next_serial_ = 0;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return heap_.size();
    }

    [[nodiscard]] std::size_t capacity() const noexcept {
        return heap_.capacity();
    }

    [[nodiscard]] bool empty() const noexcept {
        return heap_.empty();
    }

    [[nodiscard]] bool contains(const Key& key) const noexcept {
        return index_map_.contains(key);
    }

    [[nodiscard]] std::optional<rank_t> rank_of(const Key& key) const noexcept {
        const auto* ptr = index_map_.find(key);
        if (ptr == nullptr) {
            return std::nullopt;
        }
        return heap_[*ptr].rank;
    }

    [[nodiscard]] rank_t max_rank() const {
        if (heap_.empty()) {
            throw std::logic_error("IndexedHeap::max_rank called on empty heap");
        }
        return heap_[0].rank;
    }

    [[nodiscard]] const HeapNode<Key>& max_node() const {
        if (heap_.empty()) {
            throw std::logic_error("IndexedHeap::max_node called on empty heap");
        }
        return heap_[0];
    }

    /**
     * @brief Inserts (key, rank) into heap.
     * If key is already present, updates its rank and re-heaps.
     */
    void insert(const Key& key, rank_t rank) {
        const auto* ptr = index_map_.find(key);
        if (ptr != nullptr) {
            const std::size_t idx = *ptr;
            heap_[idx].rank = rank;
            heap_[idx].serial = next_serial_++;
            sift_down(idx);
            sift_up(idx);
            return;
        }

        const std::size_t idx = heap_.size();
        heap_.push_back(HeapNode<Key>{key, rank, next_serial_++});
        index_map_.insert(key, static_cast<std::uint32_t>(idx));
        sift_up(idx);
    }

    /**
     * @brief Erases key from heap.
     * @return true if key was present and erased, false otherwise.
     */
    bool erase(const Key& key) {
        const auto* ptr = index_map_.find(key);
        if (ptr == nullptr) {
            return false;
        }

        const std::size_t idx = *ptr;
        const std::size_t last_idx = heap_.size() - 1;

        index_map_.erase(key);

        if (idx == last_idx) {
            heap_.pop_back();
            return true;
        }

        heap_[idx] = std::move(heap_[last_idx]);
        heap_.pop_back();
        index_map_.insert_or_assign(heap_[idx].key, static_cast<std::uint32_t>(idx));

        sift_down(idx);
        sift_up(idx);
        return true;
    }

    /**
     * @brief Purges all nodes with rank >= cutoff_rank.
     * Since heap_[0] is always the maximum, this repeatedly erases heap_[0]
     * until empty or heap_[0].rank < cutoff_rank.
     *
     * @return Number of nodes purged.
     */
    std::size_t purge_ge(rank_t cutoff_rank) {
        std::size_t count = 0;
        while (!heap_.empty() && heap_[0].rank >= cutoff_rank) {
            erase(heap_[0].key);
            ++count;
        }
        return count;
    }

    [[nodiscard]] const std::vector<HeapNode<Key>>& nodes() const noexcept {
        return heap_;
    }

    /**
     * @brief Verifies internal max-heap and index mapping invariants.
     */
    [[nodiscard]] bool check_invariants() const noexcept {
        if (heap_.size() != index_map_.size()) {
            return false;
        }
        for (std::size_t i = 0; i < heap_.size(); ++i) {
            // Check reverse index map
            const auto* ptr = index_map_.find(heap_[i].key);
            if (ptr == nullptr || *ptr != static_cast<std::uint32_t>(i)) {
                return false;
            }
            // Check max-heap property
            const std::size_t left = 2 * i + 1;
            const std::size_t right = 2 * i + 2;
            if (left < heap_.size() && higher_priority(heap_[left], heap_[i])) {
                return false;
            }
            if (right < heap_.size() && higher_priority(heap_[right], heap_[i])) {
                return false;
            }
        }
        return true;
    }
};

} // namespace cvm::ds
