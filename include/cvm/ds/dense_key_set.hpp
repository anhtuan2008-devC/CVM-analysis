#pragma once

#include "key_index_map.hpp"
#include <vector>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <stdexcept>

namespace cvm::ds {

/**
 * @brief Dense contiguous vector of unique keys coupled with an open-addressed index map.
 *
 * Implements PLAN Section 83 and AGENTS.md Case 07 requirements:
 * - Contiguous cache-friendly vector storage for elements: vector<Key>
 * - O(1) membership check via KeyIndexMap
 * - O(1) unordered removal via Swap-Remove:
 *     last element moves into deleted slot, index map updated, pop_back()
 * - Guaranteed tombstone-free and zero stale indices
 * - Zero allocations on the steady-state streaming update path after reserve()
 * - Strictly forbidden: swap-delete inside hash table physical slots
 */
template <typename Key = std::uint64_t,
          typename Hash = KeyHasher<Key>,
          typename Equal = std::equal_to<Key>>
class DenseKeySet {
private:
    std::vector<Key> elements_;
    KeyIndexMap<Key, std::uint32_t, Hash, Equal> index_map_;

public:
    explicit DenseKeySet(std::size_t capacity = 0)
        : elements_(), index_map_(capacity)
    {
        if (capacity > 0) {
            elements_.reserve(capacity);
        }
    }

    void reserve(std::size_t capacity) {
        elements_.reserve(capacity);
        index_map_.reserve(capacity);
    }

    void clear() noexcept {
        elements_.clear();
        index_map_.clear();
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return elements_.size();
    }

    [[nodiscard]] std::size_t capacity() const noexcept {
        return elements_.capacity();
    }

    [[nodiscard]] bool empty() const noexcept {
        return elements_.empty();
    }

    [[nodiscard]] bool contains(const Key& key) const noexcept {
        return index_map_.contains(key);
    }

    [[nodiscard]] std::optional<std::uint32_t> index_of(const Key& key) const noexcept {
        const auto* ptr = index_map_.find(key);
        if (ptr == nullptr) {
            return std::nullopt;
        }
        return *ptr;
    }

    /**
     * @brief Inserts key if not already present.
     * @return true if key was inserted, false if key was already present.
     */
    bool insert(const Key& key) {
        if (index_map_.contains(key)) {
            return false;
        }
        const std::uint32_t new_idx = static_cast<std::uint32_t>(elements_.size());
        elements_.push_back(key);
        index_map_.insert(key, new_idx);
        return true;
    }

    /**
     * @brief Erases key via Swap-Remove and updates reverse mapping in index_map_.
     *
     * Sequence:
     * 1. Locate key in index_map_ to get its index idx.
     * 2. Erase key from index_map_.
     * 3. If idx is not the last element, move elements_.back() into elements_[idx]
     *    and update index_map_[last_key] = idx.
     * 4. Pop elements_.back().
     *
     * @return true if key was found and removed, false if not present.
     */
    bool erase(const Key& key) {
        const auto* ptr = index_map_.find(key);
        if (ptr == nullptr) {
            return false;
        }

        const std::uint32_t idx = *ptr;
        const std::size_t last_idx = elements_.size() - 1;

        // 1. Remove key from index map
        index_map_.erase(key);

        // 2. If removed slot is not the last element, swap last element into idx
        if (idx != static_cast<std::uint32_t>(last_idx)) {
            const Key last_key = elements_[last_idx];
            elements_[idx] = last_key;
            auto* last_ptr = index_map_.find(last_key);
            if (last_ptr != nullptr) {
                *last_ptr = idx;
            }
        }

        // 3. Pop back
        elements_.pop_back();
        return true;
    }

    [[nodiscard]] const Key& operator[](std::size_t idx) const noexcept {
        return elements_[idx];
    }

    [[nodiscard]] const Key& at(std::size_t idx) const {
        if (idx >= elements_.size()) {
            throw std::out_of_range("DenseKeySet::at index out of range");
        }
        return elements_[idx];
    }

    [[nodiscard]] const std::vector<Key>& elements() const noexcept {
        return elements_;
    }

    [[nodiscard]] std::vector<Key>& elements_mut() noexcept {
        return elements_;
    }

    auto begin() const noexcept { return elements_.begin(); }
    auto end() const noexcept { return elements_.end(); }
    auto begin() noexcept { return elements_.begin(); }
    auto end() noexcept { return elements_.end(); }

    /**
     * @brief Rebuilds index_map_ from elements_.
     * Useful after batch in-place filtering/halving passes.
     */
    void rebuild_index() {
        index_map_.clear();
        for (std::size_t i = 0; i < elements_.size(); ++i) {
            index_map_.insert(elements_[i], static_cast<std::uint32_t>(i));
        }
    }

    /**
     * @brief Verifies strict 1-to-1 bijection between elements_ and index_map_.
     * Invariants:
     * - elements_.size() == index_map_.size()
     * - Every element in elements_[i] has index_map_[elements_[i]] == i
     * - No extra keys in index_map_
     */
    [[nodiscard]] bool check_invariants() const noexcept {
        if (elements_.size() != index_map_.size()) {
            return false;
        }
        for (std::size_t i = 0; i < elements_.size(); ++i) {
            const auto* ptr = index_map_.find(elements_[i]);
            if (ptr == nullptr) {
                return false;
            }
            if (*ptr != static_cast<std::uint32_t>(i)) {
                return false;
            }
        }
        return true;
    }
};

} // namespace cvm::ds
