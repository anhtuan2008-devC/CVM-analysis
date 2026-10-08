#include "cvm/core/cvm_w.hpp"
#include "cvm/core/rng.hpp"
#include "test_utils.hpp"
#include <vector>

using namespace cvm::core;

void test_w_basic_and_distinct() {
    Xoshiro256StarStar rng(1ULL);
    CvmW<std::uint64_t> w(5, rng);

    REQUIRE(w.capacity() == 5);
    REQUIRE(w.buffer_size() == 0);

    w.update(1);
    w.update(2);
    REQUIRE(w.buffer_size() == 2);

    auto res = w.estimate();
    REQUIRE(res.has_value());
    REQUIRE(res.value() == 2.0);
}

void test_w_post_update_invariant() {
    // For W with capacity s, after every completed update: |X| <= s - 1
    const std::size_t s = 4;
    Xoshiro256StarStar rng(42ULL);
    CvmW<std::uint64_t> w(s, rng);

    for (std::uint64_t i = 1; i <= 1000; ++i) {
        w.update(i % 100);
        REQUIRE(w.buffer_size() <= s - 1);
    }
}

void test_w_duplicate_removal() {
    Xoshiro256StarStar rng(1ULL);
    CvmW<std::uint64_t> w(5, rng);

    w.update(10);
    w.update(10);
    w.update(10);

    REQUIRE(w.buffer_size() == 1);
    REQUIRE(w.dup_removals() == 2);
    auto res = w.estimate();
    REQUIRE(res.has_value());
    REQUIRE(res.value() == 1.0);
}

void test_w_same_step_cascade_and_delta_t() {
    // We construct a specific bit tape for OracleRng that forces a cascade:
    // With s = 2:
    // Insert item 1 (l=0 -> inserted, size=1)
    // Insert item 2 (l=0 -> inserted, size=2 == s)
    // First halving:
    //   Flips for item 1, item 2: false, false (both kept, size still 2!)
    //   l becomes 1.
    // Loop checks while |X| == 2, so second halving runs at the SAME step_t (t=2)!
    // Second halving:
    //   Flips for item 1, item 2: true, false (item 1 discarded, item 2 kept -> size 1 <= s - 1).
    //   l becomes 2.
    // Loop exits!
    // Expected reduction events:
    // Event 1: step_t = 2, cascade_index = 0, delta_t = 2, delta_t_valid = true
    // Event 2: step_t = 2, cascade_index = 1, delta_t = 0, delta_t_valid = false!

    std::vector<bool> bits = {
        false, false, // 1st halving: both kept
        true, false   // 2nd halving: one discarded, one kept
    };
    OracleRng oracle(bits);

    CvmW<std::uint64_t> w(2, oracle);
    w.update(1);
    w.update(2);

    REQUIRE(w.buffer_size() == 1);
    REQUIRE(w.halving_count() == 2);
    REQUIRE(w.reduction_events().size() == 2);

    const auto& ev1 = w.reduction_events()[0];
    const auto& ev2 = w.reduction_events()[1];

    // Check Event 1
    REQUIRE(ev1.step_t == 2);
    REQUIRE(ev1.cascade_index == 0);
    REQUIRE(ev1.level_before == 0);
    REQUIRE(ev1.level_after == 1);

    // Check Event 2 (Same-step cascade)
    REQUIRE(ev2.step_t == 2);
    REQUIRE(ev2.cascade_index == 1);
    REQUIRE(ev2.level_before == 1);
    REQUIRE(ev2.level_after == 2);
    REQUIRE(ev2.delta_t == 0);
    REQUIRE(ev2.delta_t_valid == false);
    // Explicit verification per AGENTS.md Case 06: delta_t must be 0, NOT converted to 1
    REQUIRE(ev2.delta_t != 1);
}

void test_w_reset() {
    Xoshiro256StarStar rng(1ULL);
    CvmW<std::uint64_t> w(4, rng);

    for (std::uint64_t i = 1; i <= 20; ++i) {
        w.update(i);
    }
    w.reset();

    REQUIRE(w.buffer_size() == 0);
    REQUIRE(w.halving_count() == 0);
    REQUIRE(w.reduction_events().empty());
    REQUIRE(w.dup_removals() == 0);
    auto res = w.estimate();
    REQUIRE(res.has_value());
    REQUIRE(res.value() == 0.0);
}

void test_w_capacity_boundaries() {
    // For W, capacity parameter s_W = k + 1.
    // Retained capacity is s_W - 1 = k.
    // For F0 distinct items:
    // If s_W = F0 + 1, buffer capacity is F0 + 1, and size never reaches s_W!
    // So all F0 items fit without any reduction, guaranteeing exact result.
    const std::size_t f0 = 4;
    {
        Xoshiro256StarStar rng(777ULL);
        CvmW<std::uint64_t> w(f0 + 1, rng);
        for (std::uint64_t i = 1; i <= f0; ++i) {
            w.update(i);
        }
        auto res = w.estimate();
        REQUIRE(res.has_value());
        REQUIRE(res.value() == static_cast<double>(f0));
        REQUIRE(w.halving_count() == 0);
    }
}

int main() {
    std::cout << "Running test_case06: CVM-W\n";
    test_w_basic_and_distinct();
    test_w_post_update_invariant();
    test_w_duplicate_removal();
    test_w_same_step_cascade_and_delta_t();
    test_w_reset();
    test_w_capacity_boundaries();

    ::cvm::test::report_and_exit();
    return 0;
}
