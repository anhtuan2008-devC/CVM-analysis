#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <utility>
#include <functional>
#include <optional>
#include <stdexcept>
#include <type_traits>

namespace cvm::ds {

/**
 * @brief High-performance 64-bit mixer for integers to ensure uniform distribution
 * and prevent clustering in open-addressing linear probing.
 */
inline constexpr std::uint64_t splitmix64_hash(std::uint64_t x) noexcept {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

template <typename Key>
struct KeyHasher {
    [[nodiscard]] std::size_t operator()(const Key& k) const noexcept {
        if constexpr (std::is_integral_v<Key>) {
            return static_cast<std::size_t>(splitmix64_hash(static_cast<std::uint64_t>(k)));
        } else {
            const std::size_t h = std::hash<Key>{}(k);
            return static_cast<std::size_t>(splitmix64_hash(static_cast<std::uint64_t>(h)));
        }
    }
};

/**
 * @brief Open-addressed hash map with linear probing and backward-shift deletion.
 *
 * Designed specifically for Key -> dense-index mapping in CVM streaming algorithms:
 * - Power-of-two table capacity with bitwise masking.
 * - Maximum load factor <= 0.5 (50%) to guarantee short probe sequences.
 * - Knuth Algorithm 6.4R backward-shift deletion: 100% tombstone-free.
 * - Zero allocations on the steady-state update path once reserve() is called.
 * - Explicit instrumentation for reallocations and table capacity.
 */
template <typename Key = std::uint64_t,
          typename Value = std::uint32_t,
          typename Hash = KeyHasher<Key>,
          typename Equal = std::equal_to<Key>>
class KeyIndexMap {
public:
    enum class SlotState : std::uint8_t {
        Empty,
        Occupied
    };

    struct Slot {
        Key key{};
        Value value{};
        SlotState state{SlotState::Empty};
    };

private:
    std::vector<Slot> slots_;
    std::size_t size_{0};
    std::size_t capacity_mask_{0};
    std::size_t realloc_count_{0};
    Hash hash_{};
    Equal equal_{};

    [[nodiscard]] static std::size_t next_power_of_two(std::size_t x) noexcept {
        if (x <= 16) return 16;
        --x;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        x |= x >> 32;
        return x + 1;
    }

    void rehash(std::size_t new_table_capacity) {
        new_table_capacity = next_power_of_two(new_table_capacity);
        if (new_table_capacity <= slots_.size()) {
            return;
        }

        std::vector<Slot> old_slots(new_table_capacity);
        old_slots.swap(slots_);
        capacity_mask_ = new_table_capacity - 1;
        size_ = 0;
        ++realloc_count_;

        for (const auto& slot : old_slots) {
            if (slot.state == SlotState::Occupied) {
                insert(slot.key, slot.value);
            }
        }
    }

public:
    explicit KeyIndexMap(std::size_t min_elements = 0) {
        const std::size_t table_cap = next_power_of_two(min_elements * 2);
        slots_.resize(table_cap);
        capacity_mask_ = table_cap - 1;
    }

    void reserve(std::size_t num_elements) {
        // Maintain load factor <= 0.5
        const std::size_t needed_table_cap = next_power_of_two(num_elements * 2);
        if (needed_table_cap > slots_.size()) {
            rehash(needed_table_cap);
        }
    }

    void clear() noexcept {
        for (auto& slot : slots_) {
            slot.state = SlotState::Empty;
        }
        size_ = 0;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] std::size_t table_capacity() const noexcept {
        return slots_.size();
    }

    [[nodiscard]] std::size_t realloc_count() const noexcept {
        return realloc_count_;
    }

    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }

    [[nodiscard]] double load_factor() const noexcept {
        if (slots_.empty()) return 0.0;
        return static_cast<double>(size_) / static_cast<double>(slots_.size());
    }

    [[nodiscard]] bool contains(const Key& key) const noexcept {
        return find(key) != nullptr;
    }

    [[nodiscard]] Value* find(const Key& key) noexcept {
        if (slots_.empty()) return nullptr;
        const std::size_t h = hash_(key);
        std::size_t i = h & capacity_mask_;

        while (slots_[i].state != SlotState::Empty) {
            if (slots_[i].state == SlotState::Occupied && equal_(slots_[i].key, key)) {
                return &slots_[i].value;
            }
            i = (i + 1) & capacity_mask_;
        }
        return nullptr;
    }

    [[nodiscard]] const Value* find(const Key& key) const noexcept {
        if (slots_.empty()) return nullptr;
        const std::size_t h = hash_(key);
        std::size_t i = h & capacity_mask_;

        while (slots_[i].state != SlotState::Empty) {
            if (slots_[i].state == SlotState::Occupied && equal_(slots_[i].key, key)) {
                return &slots_[i].value;
            }
            i = (i + 1) & capacity_mask_;
        }
        return nullptr;
    }

    /**
     * @brief Inserts (key, val) if key is not present.
     * @return true if inserted, false if key was already present.
     */
    bool insert(const Key& key, Value val) {
        // If load factor exceeds 0.5, rehash
        if ((size_ + 1) * 2 > slots_.size()) {
            rehash(slots_.size() * 2);
        }

        const std::size_t h = hash_(key);
        std::size_t i = h & capacity_mask_;

        while (slots_[i].state != SlotState::Empty) {
            if (slots_[i].state == SlotState::Occupied && equal_(slots_[i].key, key)) {
                return false;
            }
            i = (i + 1) & capacity_mask_;
        }

        slots_[i].key = key;
        slots_[i].value = val;
        slots_[i].state = SlotState::Occupied;
        ++size_;
        return true;
    }

    /**
     * @brief Inserts or assigns (key, val).
     */
    std::pair<Value*, bool> insert_or_assign(const Key& key, Value val) {
        if ((size_ + 1) * 2 > slots_.size()) {
            rehash(slots_.size() * 2);
        }

        const std::size_t h = hash_(key);
        std::size_t i = h & capacity_mask_;

        while (slots_[i].state != SlotState::Empty) {
            if (slots_[i].state == SlotState::Occupied && equal_(slots_[i].key, key)) {
                slots_[i].value = val;
                return {&slots_[i].value, false};
            }
            i = (i + 1) & capacity_mask_;
        }

        slots_[i].key = key;
        slots_[i].value = val;
        slots_[i].state = SlotState::Occupied;
        ++size_;
        return {&slots_[i].value, true};
    }

    /**
     * @brief Erases key using backward-shift deletion (Knuth Algorithm 6.4R).
     *
     * Invariant: Tombstone-free. If key is deleted, following items in the probe cluster
     * are shifted backward if their ideal hash position allows, preserving short probe
     * chains and search correctness indefinitely.
     */
    bool erase(const Key& key) {
        if (size_ == 0) return false;
        const std::size_t h = hash_(key);
        std::size_t i = h & capacity_mask_;

        while (slots_[i].state != SlotState::Empty) {
            if (slots_[i].state == SlotState::Occupied && equal_(slots_[i].key, key)) {
                // Key found at slot i
                --size_;
                std::size_t j = i;
                while (true) {
                    j = (j + 1) & capacity_mask_;
                    if (slots_[j].state == SlotState::Empty) {
                        break;
                    }
                    const std::size_t k = hash_(slots_[j].key) & capacity_mask_;
                    // Distance from ideal slot k to vacancy i vs to current slot j
                    const std::size_t dist_ki = (i - k) & capacity_mask_;
                    const std::size_t dist_kj = (j - k) & capacity_mask_;
                    if (dist_ki < dist_kj) {
                        slots_[i] = std::move(slots_[j]);
                        i = j;
                    }
                }
                slots_[i].state = SlotState::Empty;
                return true;
            }
            i = (i + 1) & capacity_mask_;
        }
        return false;
    }
};

} // namespace cvm::ds
