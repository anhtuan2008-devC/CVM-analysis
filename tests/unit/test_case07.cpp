#include "cvm/ds/key_index_map.hpp"
#include "cvm/ds/dense_key_set.hpp"
#include "cvm/core/rng.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include <algorithm>
#include <random>

using namespace cvm::ds;
using namespace cvm::core;

// Custom collided hasher to stress-test probe clustering and backward-shift deletion
struct CollidingHasher {
    std::size_t operator()(std::uint64_t k) const noexcept {
        // Map all keys to 4 distinct buckets to force extensive probe chains
        return k % 4;
    }
};

void test_key_index_map_basic() {
    KeyIndexMap<std::uint64_t, std::uint32_t> map(16);
    assert(map.empty());
    assert(map.size() == 0);
    assert(!map.contains(10));
    assert(map.find(10) == nullptr);

    assert(map.insert(10, 100));
    assert(!map.insert(10, 200)); // Duplicate rejected
    assert(map.size() == 1);
    assert(map.contains(10));
    assert(*map.find(10) == 100);

    // Update via insert_or_assign
    auto [ptr, inserted] = map.insert_or_assign(10, 300);
    assert(!inserted);
    assert(*ptr == 300);
    assert(*map.find(10) == 300);

    // Erase
    assert(map.erase(10));
    assert(!map.contains(10));
    assert(map.find(10) == nullptr);
    assert(map.size() == 0);
    assert(!map.erase(10)); // Repeated erase returns false

    std::cout << "[PASS] test_key_index_map_basic\n";
}

void test_key_index_map_backward_shift_collisions() {
    // Test backward-shift deletion with artificial collision clusters
    KeyIndexMap<std::uint64_t, std::uint32_t, CollidingHasher> map(32);

    // Keys 0, 4, 8, 12, 16 all hash to bucket 0
    std::vector<std::uint64_t> keys = {0, 4, 8, 12, 16};
    for (std::uint32_t i = 0; i < keys.size(); ++i) {
        assert(map.insert(keys[i], i));
    }
    assert(map.size() == 5);

    // Delete middle of cluster (key 8)
    assert(map.erase(8));
    assert(map.size() == 4);
    assert(!map.contains(8));

    // Verify all other keys are still found despite shift
    assert(map.contains(0) && *map.find(0) == 0);
    assert(map.contains(4) && *map.find(4) == 1);
    assert(map.contains(12) && *map.find(12) == 3);
    assert(map.contains(16) && *map.find(16) == 4);

    // Delete head of cluster (key 0)
    assert(map.erase(0));
    assert(map.size() == 3);
    assert(!map.contains(0));
    assert(map.contains(4));
    assert(map.contains(12));
    assert(map.contains(16));

    // Delete tail of cluster (key 16)
    assert(map.erase(16));
    assert(map.size() == 2);
    assert(!map.contains(16));
    assert(map.contains(4));
    assert(map.contains(12));

    // Clear and check
    map.clear();
    assert(map.empty());
    assert(map.size() == 0);
    assert(!map.contains(4));
    assert(!map.contains(12));

    std::cout << "[PASS] test_key_index_map_backward_shift_collisions\n";
}

void test_dense_key_set_basic() {
    DenseKeySet<std::uint64_t> set(16);
    assert(set.empty());
    assert(set.size() == 0);
    assert(set.check_invariants());

    assert(set.insert(42));
    assert(set.size() == 1);
    assert(set.contains(42));
    assert(set.index_of(42) == 0);
    assert(set[0] == 42);
    assert(set.at(0) == 42);
    assert(set.check_invariants());

    // Duplicate insert
    assert(!set.insert(42));
    assert(set.size() == 1);
    assert(set.check_invariants());

    // Erase
    assert(set.erase(42));
    assert(set.empty());
    assert(!set.contains(42));
    assert(set.index_of(42) == std::nullopt);
    assert(set.check_invariants());

    std::cout << "[PASS] test_dense_key_set_basic\n";
}

// Mandatory Adversarial Tests per AGENTS.md Case 07

void test_adv_remove_first_element() {
    DenseKeySet<std::uint64_t> set(16);
    set.insert(100);
    set.insert(200);
    set.insert(300);
    set.insert(400);
    assert(set.size() == 4);
    assert(set.check_invariants());

    // Remove first element (index 0 -> key 100)
    // Last element (400) should be swapped into index 0
    assert(set.erase(100));
    assert(set.size() == 3);
    assert(!set.contains(100));
    assert(set.index_of(100) == std::nullopt);

    assert(set[0] == 400);
    assert(set.index_of(400) == 0);
    assert(set[1] == 200);
    assert(set.index_of(200) == 1);
    assert(set[2] == 300);
    assert(set.index_of(300) == 2);
    assert(set.check_invariants());

    std::cout << "[PASS] test_adv_remove_first_element\n";
}

void test_adv_remove_middle_element() {
    DenseKeySet<std::uint64_t> set(16);
    set.insert(10);
    set.insert(20);
    set.insert(30);
    set.insert(40);
    set.insert(50);
    assert(set.size() == 5);
    assert(set.check_invariants());

    // Remove middle element (key 30 at index 2)
    // Last element (50) should move into index 2
    assert(set.erase(30));
    assert(set.size() == 4);
    assert(!set.contains(30));
    assert(set.index_of(30) == std::nullopt);

    assert(set[2] == 50);
    assert(set.index_of(50) == 2);
    assert(set.check_invariants());

    std::cout << "[PASS] test_adv_remove_middle_element\n";
}

void test_adv_remove_last_element() {
    DenseKeySet<std::uint64_t> set(16);
    set.insert(1);
    set.insert(2);
    set.insert(3);
    assert(set.size() == 3);
    assert(set.check_invariants());

    // Remove last element (key 3 at index 2)
    // No swap should take place; only pop_back
    assert(set.erase(3));
    assert(set.size() == 2);
    assert(!set.contains(3));
    assert(set.index_of(3) == std::nullopt);

    assert(set[0] == 1 && set.index_of(1) == 0);
    assert(set[1] == 2 && set.index_of(2) == 1);
    assert(set.check_invariants());

    std::cout << "[PASS] test_adv_remove_last_element\n";
}

void test_adv_remove_same_key_repeatedly() {
    DenseKeySet<std::uint64_t> set(16);
    set.insert(77);
    set.insert(88);
    assert(set.size() == 2);

    assert(set.erase(77));
    assert(!set.contains(77));
    assert(set.size() == 1);

    // Repeated removal of same key
    assert(!set.erase(77));
    assert(!set.erase(77));
    assert(!set.erase(77));
    assert(set.size() == 1);
    assert(set.contains(88));
    assert(set.index_of(88) == 0);
    assert(set.check_invariants());

    std::cout << "[PASS] test_adv_remove_same_key_repeatedly\n";
}

void test_adv_remove_key_after_swap() {
    DenseKeySet<std::uint64_t> set(16);
    set.insert(10);
    set.insert(20);
    set.insert(30);
    set.insert(40);

    // Remove 10 -> 40 moves to index 0
    assert(set.erase(10));
    assert(set[0] == 40);
    assert(set.index_of(40) == 0);
    assert(set.check_invariants());

    // Immediately remove 40 (the key that was just swapped into index 0)
    // Last element (30) moves into index 0
    assert(set.erase(40));
    assert(set.size() == 2);
    assert(!set.contains(40));
    assert(set.index_of(40) == std::nullopt);
    assert(set[0] == 30);
    assert(set.index_of(30) == 0);
    assert(set[1] == 20);
    assert(set.index_of(20) == 1);
    assert(set.check_invariants());

    std::cout << "[PASS] test_adv_remove_key_after_swap\n";
}

void test_adv_insert_after_many_swaps() {
    DenseKeySet<std::uint64_t> set(64);
    set.reserve(64);

    SplitMix64 rng(0xDEADBEEFULL);

    // Run 500 interleaved inserts and erases
    std::vector<std::uint64_t> active_keys;
    std::uint64_t next_key = 1;

    for (int step = 0; step < 500; ++step) {
        if (active_keys.size() < 30 && (active_keys.empty() || rng.next() % 2 == 0)) {
            const std::uint64_t k = next_key++;
            assert(set.insert(k));
            active_keys.push_back(k);
        } else if (!active_keys.empty()) {
            const std::size_t victim_idx = rng.next() % active_keys.size();
            const std::uint64_t victim_key = active_keys[victim_idx];
            active_keys[victim_idx] = active_keys.back();
            active_keys.pop_back();

            assert(set.erase(victim_key));
        }
        assert(set.check_invariants());
    }

    // Now insert keys up to capacity 64
    while (set.size() < 64) {
        const std::uint64_t k = next_key++;
        assert(set.insert(k));
    }
    assert(set.size() == 64);
    assert(set.check_invariants());

    std::cout << "[PASS] test_adv_insert_after_many_swaps\n";
}

void test_adv_duplicate_after_swap() {
    DenseKeySet<std::uint64_t> set(16);
    set.insert(100);
    set.insert(200);
    set.insert(300);

    // Swap 300 to index 0 by erasing 100
    assert(set.erase(100));
    assert(set[0] == 300);

    // Attempt to insert duplicate of swapped key 300
    assert(!set.insert(300));
    assert(set.size() == 2);
    assert(set.check_invariants());

    // Attempt to insert duplicate of untouched key 200
    assert(!set.insert(200));
    assert(set.size() == 2);
    assert(set.check_invariants());

    std::cout << "[PASS] test_adv_duplicate_after_swap\n";
}

void test_adv_full_capacity_transition() {
    const std::size_t cap = 256;
    DenseKeySet<std::uint64_t> set(cap);
    set.reserve(cap);

    // Fill to full capacity
    for (std::size_t i = 1; i <= cap; ++i) {
        assert(set.insert(i * 10));
    }
    assert(set.size() == cap);
    assert(set.check_invariants());

    // Verify all keys present
    for (std::size_t i = 1; i <= cap; ++i) {
        assert(set.contains(i * 10));
    }

    // Remove all keys from front (all triggering swap-remove until last 1)
    while (!set.empty()) {
        const std::uint64_t front_key = set[0];
        assert(set.erase(front_key));
    }
    assert(set.empty());
    assert(set.size() == 0);
    assert(set.check_invariants());

    // Refill to full capacity with fresh keys
    for (std::size_t i = 1; i <= cap; ++i) {
        assert(set.insert(10000 + i));
    }
    assert(set.size() == cap);
    assert(set.check_invariants());

    std::cout << "[PASS] test_adv_full_capacity_transition\n";
}

void test_zero_allocation_policy() {
    // PLAN Section 83.4: After reserve(), steady-state update path has zero allocations.
    const std::size_t cap = 128;
    DenseKeySet<std::uint64_t> set(cap);
    set.reserve(cap);

    // In steady state: number of elements <= cap
    const void* const initial_vector_data = set.elements().data();

    // Perform 1000 steady-state insert/erase cycles
    for (std::uint64_t k = 1; k <= 50; ++k) {
        set.insert(k);
    }
    for (int step = 0; step < 1000; ++step) {
        set.erase(static_cast<std::uint64_t>(step % 50 + 1));
        set.insert(static_cast<std::uint64_t>(1000 + step));
        set.erase(static_cast<std::uint64_t>(1000 + step));
        set.insert(static_cast<std::uint64_t>(step % 50 + 1));
    }

    // Verify vector data pointer remained absolutely identical (zero reallocations)
    assert(set.elements().data() == initial_vector_data);
    assert(set.check_invariants());

    std::cout << "[PASS] test_zero_allocation_policy\n";
}

void test_batch_rebuild_index() {
    DenseKeySet<std::uint64_t> set(32);
    set.reserve(32);
    for (std::uint64_t k = 1; k <= 20; ++k) {
        set.insert(k);
    }
    assert(set.size() == 20);

    // Simulate in-place halving: filter elements directly in elements_mut()
    auto& vec = set.elements_mut();
    std::size_t write_idx = 0;
    for (std::size_t i = 0; i < vec.size(); ++i) {
        // Discard even keys
        if (vec[i] % 2 != 0) {
            vec[write_idx++] = vec[i];
        }
    }
    vec.resize(write_idx);
    assert(set.size() == 10);

    // Rebuild index
    set.rebuild_index();
    assert(set.check_invariants());
    for (std::size_t i = 0; i < set.size(); ++i) {
        assert(set[i] % 2 != 0);
        assert(set.contains(set[i]));
        assert(set.index_of(set[i]) == i);
    }

    std::cout << "[PASS] test_batch_rebuild_index\n";
}

int main() {
    std::cout << "=== Running CASE-07 A1/W Data Structures Tests ===\n";

    test_key_index_map_basic();
    test_key_index_map_backward_shift_collisions();
    test_dense_key_set_basic();

    // Mandatory adversarial suite
    test_adv_remove_first_element();
    test_adv_remove_middle_element();
    test_adv_remove_last_element();
    test_adv_remove_same_key_repeatedly();
    test_adv_remove_key_after_swap();
    test_adv_insert_after_many_swaps();
    test_adv_duplicate_after_swap();
    test_adv_full_capacity_transition();

    // Allocation & batch filter verification
    test_zero_allocation_policy();
    test_batch_rebuild_index();

    std::cout << "All CASE-07 tests PASSED cleanly!\n";
    return 0;
}
