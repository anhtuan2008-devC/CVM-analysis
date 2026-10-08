#include "cvm/core/knuth_dprime.hpp"
#include "cvm/core/rng.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

using namespace cvm::core;

void test_mandatory_l0() {
    Xoshiro256StarStar rng(123);
    KnuthDprime<std::uint64_t> alg(5, rng);

    // Stream 5 items with distinct ranks, including ranks > 2^63
    alg.update_with_rank(1, 0xFFFFFFFFFFFFFFFFULL);
    alg.update_with_rank(2, 0x8000000000000000ULL);
    alg.update_with_rank(3, 100);
    alg.update_with_rank(4, 500);
    alg.update_with_rank(5, 1000);

    assert(alg.size() == 5);
    assert(alg.level() == 0);
    assert(alg.estimate().value() == 5.0);
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_l0\n";
}

void test_mandatory_l1() {
    Xoshiro256StarStar rng(456);
    KnuthDprime<std::uint64_t> alg(2, rng);

    // Initial fill at level 0:
    // Item 1: rank < 2^63
    // Item 2: rank >= 2^63
    alg.update_with_rank(1, 1000);
    alg.update_with_rank(2, 0x9000000000000000ULL); // >= 2^63
    assert(alg.size() == 2 && alg.level() == 0);

    // Item 3: rank < 2^63
    // Triggers halving: level increases to 1. Threshold for level 1 is 2^63.
    // Item 2 has rank >= 2^63, so it is purged!
    // Item 3 has rank < 2^63, so it is inserted!
    alg.update_with_rank(3, 2000);

    assert(alg.level() == 1);
    assert(!alg.heap().contains(2)); // Item 2 purged
    assert(alg.heap().contains(1) && alg.heap().contains(3)); // 1 and 3 kept
    assert(alg.size() == 2);
    assert(alg.estimate().value() == 4.0); // 2 * 2^1 = 4.0
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_l1\n";
}

void test_mandatory_levels_63_64_65() {
    Xoshiro256StarStar rng(789);
    KnuthDprime<std::uint64_t> alg(1, rng);

    // Stream items forcing level cascades
    // At level 63: threshold is 2^1 = 2 (accepts r < 2, i.e., r=0 and r=1)
    // At level 64: threshold is 2^0 = 1 (accepts r < 1, i.e., r=0)
    // At level 65: threshold is 0 (rejects all)

    // Fill buffer with item with rank 0
    alg.update_with_rank(1, 0);

    // Stream item with rank 0 when buffer full with rank 0:
    // In Dprime, both stored and incoming have rank 0.
    // At every level l from 1 to 64:
    // threshold is >= 1, so rank 0 is NEVER purged, and incoming rank 0 is ALWAYS accepted!
    // So the cascade proceeds continuously through l = 1, 2, ..., 64, and triggers transition to 65!
    alg.update_with_rank(2, 0);

    assert(alg.level() == 65);
    assert(alg.finite_rank_resolution_events() == 1);
    assert(alg.empty());
    assert(alg.estimate().status() == FailureStatus::NumericFailure);

    // Subsequent updates must all be rejected under l >= 65
    alg.update_with_rank(3, 0);
    assert(alg.empty());
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_levels_63_64_65\n";
}

void test_mandatory_multiple_level_cascade() {
    Xoshiro256StarStar rng(111);
    KnuthDprime<std::uint64_t> alg(2, rng);

    // Insert 2 items with small ranks (< 256 = 2^8)
    alg.update_with_rank(1, 50);
    alg.update_with_rank(2, 120);
    assert(alg.size() == 2 && alg.level() == 0);

    // Item 3 with rank 70 (< 128 = 2^7)
    // When halving occurs:
    // Threshold 2^(64-l): for l from 1 to 56, threshold > 256.
    // None of the items are purged for 56 levels!
    // At level 57, threshold is 2^(64-57) = 2^7 = 128.
    // Item 2 has rank 120 < 128, so still not purged!
    // At level 58, threshold is 2^(64-58) = 2^6 = 64.
    // Item 2 has rank 120 >= 64 -> PURGED!
    // Item 3 has rank 70 >= 64 -> not accepted!
    // Item 1 has rank 50 < 64 -> kept!
    alg.update_with_rank(3, 70);

    assert(alg.level() == 58);
    assert(alg.reduction_rounds() == 58);
    assert(alg.size() == 1);
    assert(alg.heap().contains(1));
    assert(!alg.heap().contains(2));
    assert(!alg.heap().contains(3));
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_multiple_level_cascade\n";
}

void test_mandatory_duplicate() {
    Xoshiro256StarStar rng(222);
    KnuthDprime<std::uint64_t> alg(5, rng);

    alg.update_with_rank(1, 100);
    alg.update_with_rank(2, 200);
    assert(alg.size() == 2);

    // Duplicate update of key 1
    alg.update_with_rank(1, 300);
    assert(alg.size() == 2);
    assert(alg.dup_removals() == 1);
    assert(alg.heap().rank_of(1) == 300);
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_duplicate\n";
}

void test_mandatory_cutoff_equality() {
    Xoshiro256StarStar rng(333);
    KnuthDprime<std::uint64_t> alg(2, rng);

    // Transition to level 1: threshold is 2^63 = 0x8000000000000000ULL
    alg.update_with_rank(1, 100);
    alg.update_with_rank(2, 0x9000000000000000ULL);
    alg.update_with_rank(3, 200); // Forces level 1
    assert(alg.level() == 1);

    // Stream item with rank exactly equal to threshold 2^63
    alg.update_with_rank(4, 0x8000000000000000ULL);
    assert(alg.cutoff_tie_events() == 1);
    assert(!alg.heap().contains(4)); // Rejected
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_cutoff_equality\n";
}

void test_mandatory_multiple_maxima() {
    Xoshiro256StarStar rng(444);
    KnuthDprime<std::uint64_t> alg(3, rng);

    // Fill buffer with two elements having rank >= 2^63
    alg.update_with_rank(1, 10);
    alg.update_with_rank(2, 0xA000000000000000ULL);
    alg.update_with_rank(3, 0xA000000000000000ULL);
    assert(alg.size() == 3 && alg.level() == 0);

    // Incoming rank 50 < 2^63
    // Halving to level 1 purges BOTH keys 2 and 3 simultaneously
    alg.update_with_rank(4, 50);
    assert(alg.level() == 1);
    assert(!alg.heap().contains(2));
    assert(!alg.heap().contains(3));
    assert(alg.heap().contains(1) && alg.heap().contains(4));
    assert(alg.size() == 2);
    assert(alg.equal_max_rank_events() == 1);
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_multiple_maxima\n";
}

void test_mandatory_exact_capacity() {
    Xoshiro256StarStar rng(555);
    constexpr std::size_t F0 = 20;
    KnuthDprime<std::uint64_t> alg(F0, rng);

    for (std::uint64_t i = 1; i <= F0; ++i) {
        alg.update(i);
    }
    assert(alg.size() == F0);
    assert(alg.level() == 0);
    assert(alg.estimate().value() == static_cast<double>(F0));
    assert(alg.check_invariants());

    std::cout << "[PASS] test_mandatory_exact_capacity\n";
}

void test_source_fixture_f4_dprime_abc() {
    // S2 Answers Ex 8 (C19): Stream (a, b, c) with s=1 under Algorithm D'
    // has Pr[F_hat = 0] = 2/7 (~0.285714)
    constexpr std::size_t TRIALS = 50000;
    std::size_t zero_count = 0;
    Xoshiro256StarStar trial_rng(9999);

    for (std::size_t t = 0; t < TRIALS; ++t) {
        KnuthDprime<std::uint64_t> alg(1, trial_rng);
        alg.update(1);
        alg.update(2);
        alg.update(3);
        auto res = alg.estimate();
        assert(res.has_value());
        if (res.value() == 0.0) {
            ++zero_count;
        } else {
            // Estimates must be powers of two: |B| * 2^l = 2^l
            const double v = res.value();
            int exp = 0;
            const double frac = std::frexp(v, &exp);
            assert(frac == 0.5); // Powers of 2 have frexp mantissa 0.5
        }
    }

    const double empirical_p0 = static_cast<double>(zero_count) / static_cast<double>(TRIALS);
    constexpr double expected_p0 = 2.0 / 7.0;
    std::cout << "  (a,b,c) with s=1 under D': Pr[F_hat = 0] = " << empirical_p0
              << " (expected " << expected_p0 << ")\n";
    // Standard error sqrt(0.2857 * 0.7143 / 50000) ~ 0.002
    assert(std::abs(empirical_p0 - expected_p0) < 0.01);

    std::cout << "[PASS] test_source_fixture_f4_dprime_abc\n";
}

int main() {
    std::cout << "=== Running CASE-09 Knuth-Dprime Tests ===\n";

    test_mandatory_l0();
    test_mandatory_l1();
    test_mandatory_levels_63_64_65();
    test_mandatory_multiple_level_cascade();
    test_mandatory_duplicate();
    test_mandatory_cutoff_equality();
    test_mandatory_multiple_maxima();
    test_mandatory_exact_capacity();

    // Source Fixture F4 (S2 Exercise 8) verification
    test_source_fixture_f4_dprime_abc();

    std::cout << "All CASE-09 tests PASSED cleanly!\n";
    return 0;
}
