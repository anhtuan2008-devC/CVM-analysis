#include "cvm/core/knuth_d.hpp"
#include "cvm/ds/indexed_heap.hpp"
#include "cvm/ds/cutoff64.hpp"
#include "cvm/core/rng.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include <algorithm>
#include <cmath>

using namespace cvm::core;
using namespace cvm::ds;

void test_indexed_heap_basic() {
    IndexedHeap<std::uint64_t> heap(16);
    assert(heap.empty());
    assert(heap.size() == 0);
    assert(heap.check_invariants());

    heap.insert(10, 500);
    heap.insert(20, 800);
    heap.insert(30, 300);
    assert(heap.size() == 3);
    assert(heap.max_rank() == 800);
    assert(heap.max_node().key == 20);
    assert(heap.check_invariants());

    // Update existing key
    heap.insert(30, 900); // 30 becomes max
    assert(heap.max_rank() == 900);
    assert(heap.max_node().key == 30);
    assert(heap.check_invariants());

    // Erase element
    assert(heap.erase(20));
    assert(!heap.contains(20));
    assert(heap.size() == 2);
    assert(heap.check_invariants());

    // Purge elements >= 600
    const std::size_t purged = heap.purge_ge(600);
    assert(purged == 1); // key 30 purged
    assert(heap.size() == 1);
    assert(heap.contains(10));
    assert(heap.check_invariants());

    std::cout << "[PASS] test_indexed_heap_basic\n";
}

void test_mandatory_d1_aba() {
    // S2 Eq (7): Stream (a, b, a) with s = 1 has E[F_hat] = 2.
    // 1. Controlled deterministic behavior:
    {
        Xoshiro256StarStar dummy_rng(1);
        KnuthD<std::uint64_t> alg(1, dummy_rng);

        // Item a with rank 1000
        alg.update_with_rank(1, 1000);
        assert(alg.size() == 1 && alg.heap().contains(1));
        assert(alg.cutoff().is_one());

        // Item b with rank 2000 > 1000: incoming rank > max
        alg.update_with_rank(2, 2000);
        assert(alg.cutoff() == Cutoff64::rank(2000));
        assert(alg.size() == 1 && alg.heap().contains(1)); // 2 not stored

        // Item a repeated: old a deleted, buffer empty!
        // Rank 500 < 2000: inserted into empty buffer
        alg.update_with_rank(1, 500);
        assert(alg.size() == 1 && alg.heap().contains(1));
        assert(alg.dup_removals() == 1);
        assert(alg.check_invariants());
    }

    // 2. Statistical expectation test over 50,000 trials: E[F_hat] = 2.0
    {
        constexpr std::size_t TRIALS = 50000;
        double sum_est = 0.0;
        Xoshiro256StarStar trial_rng(42);

        for (std::size_t t = 0; t < TRIALS; ++t) {
            KnuthD<std::uint64_t> alg(1, trial_rng);
            alg.update(1);
            alg.update(2);
            alg.update(1);
            auto res = alg.estimate();
            assert(res.has_value());
            sum_est += res.value();
        }

        const double mean_est = sum_est / static_cast<double>(TRIALS);
        std::cout << "  (a,b,a) with s=1: empirical mean = " << mean_est << " (expected 2.0)\n";
        // Check within +/- 0.08
        assert(std::abs(mean_est - 2.0) < 0.08);
    }

    std::cout << "[PASS] test_mandatory_d1_aba\n";
}

void test_mandatory_d1_abc() {
    // S2 Answers Ex 3 (C17): For distinct streams under Algorithm D, E[F_hat] = m.
    // For stream (a, b, c) of m=3 distinct items with s=1: E[F_hat] = 3.0.
    // In continuous D, cutoff p ~ Beta(2, 2), so E[1/p] = \int_0^1 (1/p) * 6p(1-p) dp = 3.0.
    constexpr std::size_t TRIALS = 50000;
    double sum_est = 0.0;
    Xoshiro256StarStar trial_rng(12345);

    for (std::size_t t = 0; t < TRIALS; ++t) {
        KnuthD<std::uint64_t> alg(1, trial_rng);
        alg.update(1);
        alg.update(2);
        alg.update(3);
        auto res = alg.estimate();
        assert(res.has_value());
        sum_est += res.value();
    }

    const double empirical_mean = sum_est / static_cast<double>(TRIALS);
    std::cout << "  (a,b,c) with s=1: empirical mean = " << empirical_mean << " (expected 3.0)\n";
    // Check within +/- 0.1
    assert(std::abs(empirical_mean - 3.0) < 0.1);

    std::cout << "[PASS] test_mandatory_d1_abc\n";
}


void test_mandatory_all_distinct() {
    Xoshiro256StarStar rng(999);
    KnuthD<std::uint64_t> alg(30, rng);

    for (std::uint64_t i = 1; i <= 500; ++i) {
        alg.update(i);
        assert(alg.size() <= 30);
        assert(alg.check_invariants());
    }
    assert(alg.reduction_rounds() > 0);
    assert(alg.dup_removals() == 0);

    auto res = alg.estimate();
    assert(res.has_value());
    assert(res.value() > 0.0);

    std::cout << "[PASS] test_mandatory_all_distinct\n";
}

void test_mandatory_duplicates() {
    Xoshiro256StarStar rng(777);
    KnuthD<std::uint64_t> alg(20, rng);

    // Stream of 1000 items with only 15 distinct keys
    for (int step = 0; step < 1000; ++step) {
        alg.update(static_cast<std::uint64_t>(step % 15 + 1));
        assert(alg.size() <= 20);
        assert(alg.check_invariants());
    }
    assert(alg.dup_removals() > 0);
    // Since F0 = 15 <= s = 20, cutoff must still be ONE!
    assert(alg.cutoff().is_one());
    assert(alg.size() == 15);
    assert(alg.estimate().value() == 15.0);

    std::cout << "[PASS] test_mandatory_duplicates\n";
}

void test_mandatory_cutoff_replacement() {
    Xoshiro256StarStar dummy_rng(1);
    KnuthD<std::uint64_t> alg(2, dummy_rng);

    assert(alg.cutoff().is_one());
    alg.update_with_rank(10, 1000);
    alg.update_with_rank(20, 2000);
    assert(alg.size() == 2);
    assert(alg.cutoff().is_one());

    // Incoming rank 3000 > max (2000)
    alg.update_with_rank(30, 3000);
    assert(alg.cutoff() == Cutoff64::rank(3000));

    // Incoming rank 1500 < max (2000)
    alg.update_with_rank(40, 1500);
    assert(alg.cutoff() == Cutoff64::rank(2000)); // Lowered to 2000

    // Cutoff monotonicity: cutoff never increases
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_cutoff_replacement\n";
}

void test_mandatory_incoming_gt_max() {
    Xoshiro256StarStar dummy_rng(1);
    KnuthD<std::uint64_t> alg(2, dummy_rng);

    alg.update_with_rank(1, 100);
    alg.update_with_rank(2, 200);
    assert(alg.size() == 2);

    // Incoming rank 300 > M (200)
    alg.update_with_rank(3, 300);
    assert(alg.cutoff() == Cutoff64::rank(300));
    assert(!alg.heap().contains(3)); // 3 not stored
    assert(alg.heap().contains(1) && alg.heap().contains(2));
    assert(alg.size() == 2);
    assert(alg.reduction_rounds() == 1);
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_incoming_gt_max\n";
}

void test_mandatory_incoming_lt_max() {
    Xoshiro256StarStar dummy_rng(1);
    KnuthD<std::uint64_t> alg(2, dummy_rng);

    alg.update_with_rank(1, 100);
    alg.update_with_rank(2, 200);

    // Incoming rank 150 < M (200)
    alg.update_with_rank(3, 150);
    assert(alg.cutoff() == Cutoff64::rank(200));
    assert(!alg.heap().contains(2)); // Old max (2) purged
    assert(alg.heap().contains(1) && alg.heap().contains(3)); // 3 inserted
    assert(alg.size() == 2);
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_incoming_lt_max\n";
}

void test_mandatory_incoming_eq_max() {
    Xoshiro256StarStar dummy_rng(1);
    KnuthD<std::uint64_t> alg(2, dummy_rng);

    alg.update_with_rank(1, 100);
    alg.update_with_rank(2, 200);

    // Incoming rank 200 == M (200) (finite tie)
    alg.update_with_rank(3, 200);
    assert(alg.cutoff() == Cutoff64::rank(200));
    assert(!alg.heap().contains(2)); // Old max (2) purged
    assert(!alg.heap().contains(3)); // 3 rejected (rank == cutoff)
    assert(alg.heap().contains(1));  // Only 1 remains
    assert(alg.size() == 1);
    assert(alg.cutoff_tie_events() == 1);
    assert(alg.rank_collision_events() == 1);
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_incoming_eq_max\n";
}

void test_mandatory_multiple_equal_maxima() {
    Xoshiro256StarStar dummy_rng(1);
    KnuthD<std::uint64_t> alg(4, dummy_rng);

    // Fill buffer with two elements having the exact same maximum rank 500
    alg.update_with_rank(1, 100);
    alg.update_with_rank(2, 500);
    alg.update_with_rank(3, 500);
    alg.update_with_rank(4, 200);
    assert(alg.size() == 4);

    // Incoming rank 80 < M (500)
    alg.update_with_rank(5, 80);
    assert(alg.cutoff() == Cutoff64::rank(500));
    // BOTH elements 2 and 3 must be purged!
    assert(!alg.heap().contains(2));
    assert(!alg.heap().contains(3));
    // Elements 1, 4, 5 remain
    assert(alg.heap().contains(1));
    assert(alg.heap().contains(4));
    assert(alg.heap().contains(5));
    assert(alg.size() == 3);
    assert(alg.equal_max_rank_events() == 1);
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_multiple_equal_maxima\n";
}

void test_mandatory_cutoff_rank_0() {
    Xoshiro256StarStar dummy_rng(1);
    KnuthD<std::uint64_t> alg(2, dummy_rng);

    // Force cutoff to RANK(0) by streaming item with rank 0 when full
    alg.update_with_rank(1, 0);
    alg.update_with_rank(2, 10);
    // Incoming rank 0 < 10 -> cutoff becomes RANK(10)
    alg.update_with_rank(3, 0); // Purges 2, keeps {1, 3} with rank 0
    // Incoming rank 0 when max is 0: tie -> cutoff becomes RANK(0)
    alg.update_with_rank(4, 0);

    assert(alg.cutoff() == Cutoff64::rank(0));
    assert(alg.empty()); // All items purged, none can be accepted under RANK(0)

    // Further items must all be rejected
    alg.update_with_rank(5, 100);
    alg.update_with_rank(6, 0);
    assert(alg.empty());
    assert(alg.estimate().value() == 0.0);
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_cutoff_rank_0\n";
}

void test_mandatory_reset() {
    Xoshiro256StarStar rng(555);
    KnuthD<std::uint64_t> alg(5, rng);

    for (std::uint64_t i = 1; i <= 20; ++i) {
        alg.update(i);
    }
    assert(!alg.empty());
    assert(!alg.cutoff().is_one());

    alg.reset();
    assert(alg.empty());
    assert(alg.size() == 0);
    assert(alg.cutoff().is_one());
    assert(alg.reduction_rounds() == 0);
    assert(alg.dup_removals() == 0);
    assert(alg.estimate().value() == 0.0);
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_reset\n";
}

void test_mandatory_capacity_boundaries() {
    Xoshiro256StarStar rng(888);

    // 1. s = 1
    {
        KnuthD<std::uint64_t> alg(1, rng);
        for (std::uint64_t i = 1; i <= 10; ++i) {
            alg.update(i);
            assert(alg.size() <= 1);
            assert(alg.check_invariants());
        }
    }

    // 2. s = F0: Exact capacity guarantee
    // S2 Exercise 7: Knuth-D is exact when s >= n
    {
        constexpr std::size_t F0 = 25;
        KnuthD<std::uint64_t> alg(F0, rng);
        for (std::uint64_t i = 1; i <= F0; ++i) {
            alg.update(i);
        }
        assert(alg.size() == F0);
        assert(alg.cutoff().is_one());
        auto res = alg.estimate();
        assert(res.has_value());
        assert(res.value() == static_cast<double>(F0)); // Exactly F0!
        assert(alg.check_invariants());
    }

    // 3. s = F0 - 1
    {
        constexpr std::size_t F0 = 25;
        KnuthD<std::uint64_t> alg(F0 - 1, rng);
        for (std::uint64_t i = 1; i <= F0; ++i) {
            alg.update(i);
        }
        assert(alg.size() <= F0 - 1);
        // Cutoff must have transitioned from ONE to RANK
        assert(alg.cutoff().is_rank());
        assert(alg.check_invariants());
    }

    std::cout << "[PASS] test_mandatory_capacity_boundaries\n";
}

int main() {
    std::cout << "=== Running CASE-08 Knuth-D Tests ===\n";

    test_indexed_heap_basic();

    // Mandatory tests per AGENTS.md Case 08
    test_mandatory_d1_aba();
    test_mandatory_d1_abc();
    test_mandatory_all_distinct();
    test_mandatory_duplicates();
    test_mandatory_cutoff_replacement();
    test_mandatory_incoming_gt_max();
    test_mandatory_incoming_lt_max();
    test_mandatory_incoming_eq_max();
    test_mandatory_multiple_equal_maxima();
    test_mandatory_cutoff_rank_0();
    test_mandatory_reset();
    test_mandatory_capacity_boundaries();

    std::cout << "All CASE-08 tests PASSED cleanly!\n";
    return 0;
}
