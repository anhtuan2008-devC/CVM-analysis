#include "cvm/core/cvm_a1.hpp"
#include "cvm/core/rng.hpp"
#include "test_utils.hpp"
#include <vector>
#include <numeric>

using namespace cvm::core;

void test_empty_stream() {
    Xoshiro256StarStar rng(1ULL);
    CvmA1<std::uint64_t> a1(10, rng);

    auto res = a1.estimate();
    REQUIRE(res.has_value());
    REQUIRE(res.value() == 0.0);
    REQUIRE(a1.buffer_size() == 0);
    REQUIRE(a1.halving_count() == 0);
}

void test_single_item() {
    Xoshiro256StarStar rng(1ULL);
    CvmA1<std::uint64_t> a1(10, rng);

    a1.update(42);
    auto res = a1.estimate();
    REQUIRE(res.has_value());
    REQUIRE(res.value() == 1.0);
    REQUIRE(a1.buffer_size() == 1);
}

void test_duplicate_item() {
    Xoshiro256StarStar rng(1ULL);
    CvmA1<std::uint64_t> a1(10, rng);

    a1.update(42);
    a1.update(42);
    a1.update(42);

    auto res = a1.estimate();
    REQUIRE(res.has_value());
    REQUIRE(res.value() == 1.0);
    REQUIRE(a1.buffer_size() == 1);
    REQUIRE(a1.dup_removals() == 2);
}

void test_all_distinct_within_capacity() {
    Xoshiro256StarStar rng(1ULL);
    CvmA1<std::uint64_t> a1(10, rng);

    for (std::uint64_t i = 1; i <= 8; ++i) {
        a1.update(i);
    }

    auto res = a1.estimate();
    REQUIRE(res.has_value());
    REQUIRE(res.value() == 8.0);
    REQUIRE(a1.buffer_size() == 8);
    REQUIRE(a1.halving_count() == 0);
}

void test_threshold_crossing_and_reduction() {
    // When buffer reaches s, reduction occurs and halving_count l increases
    Xoshiro256StarStar rng(12345ULL);
    CvmA1<std::uint64_t> a1(5, rng);

    for (std::uint64_t i = 1; i <= 5; ++i) {
        a1.update(i);
    }

    REQUIRE(a1.reduction_rounds() >= 1);
    REQUIRE(a1.halving_count() >= 1);
}

void test_reset() {
    Xoshiro256StarStar rng(1ULL);
    CvmA1<std::uint64_t> a1(5, rng);

    for (std::uint64_t i = 1; i <= 10; ++i) {
        a1.update(i);
    }
    a1.reset();

    REQUIRE(a1.buffer_size() == 0);
    REQUIRE(a1.halving_count() == 0);
    REQUIRE(!a1.is_aborted());
    REQUIRE(a1.reduction_rounds() == 0);
    REQUIRE(a1.dup_removals() == 0);

    auto res = a1.estimate();
    REQUIRE(res.has_value());
    REQUIRE(res.value() == 0.0);
}

void test_capacity_boundaries() {
    // Exact capacity policy per PLAN Section 136:
    // s > F0 guarantees exact result for A1.
    // s = F0 can trigger reduction if stream fills buffer, so it is NOT guaranteed exact.
    const std::size_t f0 = 5;

    // Test s = F0 + 1 (s = 6 > 5)
    {
        Xoshiro256StarStar rng(999ULL);
        CvmA1<std::uint64_t> a1(f0 + 1, rng);
        for (std::uint64_t i = 1; i <= f0; ++i) {
            a1.update(i);
        }
        auto res = a1.estimate();
        REQUIRE(res.has_value());
        REQUIRE(res.value() == static_cast<double>(f0)); // Exactly 5
        REQUIRE(a1.halving_count() == 0); // No reduction
    }

    // Test s >> F0 (s = 100)
    {
        Xoshiro256StarStar rng(999ULL);
        CvmA1<std::uint64_t> a1(100, rng);
        for (std::uint64_t i = 1; i <= f0; ++i) {
            a1.update(i);
        }
        auto res = a1.estimate();
        REQUIRE(res.has_value());
        REQUIRE(res.value() == static_cast<double>(f0));
        REQUIRE(a1.halving_count() == 0);
    }
}

void test_abort_trigger() {
    // Controlled bit tape causing abort:
    // With s = 2, insert item 1, item 2 (size reaches 2).
    // Halving flips: bit1 = false (keep), bit2 = false (keep).
    // Both kept => size is still 2 => Abort!
    std::vector<bool> bits = {false, false};
    OracleRng oracle(bits);

    CvmA1<std::uint64_t> a1(2, oracle);
    a1.update(1);
    a1.update(2);

    REQUIRE(a1.is_aborted());
    REQUIRE(a1.status() == FailureStatus::Abort);
    auto res = a1.estimate();
    REQUIRE(!res.has_value());
    REQUIRE(res.status() == FailureStatus::Abort);
}

void test_source_fixture_f1_exact_tree() {
    // S2 Appendix p. 12 exact analysis of stream (a, b, c) with s = 2:
    // 16 elementary coin flip outcomes with equal probability 1/16:
    // First halving at item 2: 4 outcomes (keep a, keep b):
    //   (K, K): aborts immediately -> 4 of 16 outcomes abort!
    //   (D, D): X becomes empty, l=1
    //   (K, D): X becomes {a}, l=1
    //   (D, K): X becomes {b}, l=1
    // Item 3 has 2 outcomes: insert (bit=0) or not insert (bit=1).
    // If prior size was 1 (2 branches) and item 3 is inserted (1/2):
    //   Second halving occurs: (K, K) aborts -> 1 of 16 outcomes abort!
    // Total aborts: 5 of 16.
    // Non-aborts: 11 of 16.
    // Sum of non-abort estimates: 20/16.
    // Conditional expectation: (20/16) / (11/16) = 20/11.

    // Let's run a Monte Carlo simulation of 160,000 runs to verify F1 convergence
    std::size_t num_runs = 160000;
    std::size_t abort_count = 0;
    double estimate_sum = 0.0;

    Xoshiro256StarStar rng(1337ULL);
    for (std::size_t trial = 0; trial < num_runs; ++trial) {
        CvmA1<std::uint64_t> a1(2, rng);
        a1.update(1);
        a1.update(2);
        a1.update(3);

        auto res = a1.estimate();
        if (res.status() == FailureStatus::Abort) {
            ++abort_count;
        } else {
            estimate_sum += res.value();
        }
    }

    const double empirical_p_fail = static_cast<double>(abort_count) / static_cast<double>(num_runs);
    const double expected_p_fail = 5.0 / 16.0; // 0.3125
    REQUIRE(std::abs(empirical_p_fail - expected_p_fail) < 0.005);

    const std::size_t non_abort_count = num_runs - abort_count;
    const double empirical_cond_mean = estimate_sum / static_cast<double>(non_abort_count);
    const double expected_cond_mean = 20.0 / 11.0; // 1.81818...
    REQUIRE(std::abs(empirical_cond_mean - expected_cond_mean) < 0.015);
}

int main() {
    std::cout << "Running test_case05: CVM-A1\n";
    test_empty_stream();
    test_single_item();
    test_duplicate_item();
    test_all_distinct_within_capacity();
    test_threshold_crossing_and_reduction();
    test_reset();
    test_capacity_boundaries();
    test_abort_trigger();
    test_source_fixture_f1_exact_tree();

    ::cvm::test::report_and_exit();
    return 0;
}
