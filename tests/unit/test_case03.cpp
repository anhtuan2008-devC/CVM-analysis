#include "cvm/core/rng.hpp"
#include "test_utils.hpp"
#include <vector>

using namespace cvm::core;

void test_splitmix64_determinism() {
    SplitMix64 sm1(123456789ULL);
    SplitMix64 sm2(123456789ULL);

    for (int i = 0; i < 100; ++i) {
        REQUIRE(sm1.next() == sm2.next());
    }

    SplitMix64 sm3(987654321ULL);
    REQUIRE(sm1.next() != sm3.next());
}

void test_seed_hierarchy() {
    const std::uint64_t master_seed = 0xDEADBEEFCAFEULL;
    const std::uint64_t exp_id = 42;
    const auto alg = AlgorithmId::CVM_W;
    const std::uint64_t trial_idx = 17;

    const auto trial_seed1 = SeedHierarchy::derive_full_trial_seed(master_seed, exp_id, alg, trial_idx);
    const auto trial_seed2 = SeedHierarchy::derive_full_trial_seed(master_seed, exp_id, alg, trial_idx);
    REQUIRE(trial_seed1 == trial_seed2);

    // Changing trial index must change seed
    const auto trial_seed3 = SeedHierarchy::derive_full_trial_seed(master_seed, exp_id, alg, trial_idx + 1);
    REQUIRE(trial_seed1 != trial_seed3);

    // Changing algorithm must change seed
    const auto trial_seed4 = SeedHierarchy::derive_full_trial_seed(master_seed, exp_id, AlgorithmId::Knuth_D, trial_idx);
    REQUIRE(trial_seed1 != trial_seed4);

    // Changing experiment must change seed
    const auto trial_seed5 = SeedHierarchy::derive_full_trial_seed(master_seed, exp_id + 1, alg, trial_idx);
    REQUIRE(trial_seed1 != trial_seed5);
}

void test_xoshiro_determinism_and_replay() {
    const std::uint64_t seed = 0x123456789ABCDEF0ULL;
    Xoshiro256StarStar rng1(seed);
    Xoshiro256StarStar rng2(seed);

    std::vector<std::uint64_t> seq1;
    std::vector<std::uint64_t> seq2;
    for (int i = 0; i < 200; ++i) {
        seq1.push_back(rng1.next_uint64());
        seq2.push_back(rng2.next_uint64());
    }
    REQUIRE(seq1 == seq2);

    // Test reset replay
    rng1.reset();
    std::vector<std::uint64_t> seq1_reset;
    for (int i = 0; i < 200; ++i) {
        seq1_reset.push_back(rng1.next_uint64());
    }
    REQUIRE(seq1 == seq1_reset);

    // Different seed produces different stream
    Xoshiro256StarStar rng_diff(seed + 1);
    std::vector<std::uint64_t> seq_diff;
    for (int i = 0; i < 200; ++i) {
        seq_diff.push_back(rng_diff.next_uint64());
    }
    REQUIRE(seq1 != seq_diff);
}

void test_bit_consumption_accounting() {
    Xoshiro256StarStar rng(42ULL);
    REQUIRE(rng.bits_consumed() == 0);
    REQUIRE(rng.uint64_consumed() == 0);

    for (std::uint64_t i = 1; i <= 250; ++i) {
        [[maybe_unused]] bool b = rng.next_bit();
        REQUIRE(rng.bits_consumed() == i);
    }
    REQUIRE(rng.uint64_consumed() == 0); // next_bit buffered, direct uint64 consumption is 0

    for (std::uint64_t i = 1; i <= 50; ++i) {
        [[maybe_unused]] auto w = rng.next_uint64();
        REQUIRE(rng.uint64_consumed() == i);
    }
    REQUIRE(rng.bits_consumed() == 250);

    // Reset clears consumption counters
    rng.reset();
    REQUIRE(rng.bits_consumed() == 0);
    REQUIRE(rng.uint64_consumed() == 0);
}

void test_independent_instances_no_global_state() {
    Xoshiro256StarStar rng_a(100ULL);
    Xoshiro256StarStar rng_b(200ULL);

    auto a1 = rng_a.next_uint64();
    [[maybe_unused]] auto b1 = rng_b.next_uint64();
    auto a2 = rng_a.next_uint64();

    // Replay rng_a alone with same seed
    Xoshiro256StarStar rng_a_replay(100ULL);
    REQUIRE(rng_a_replay.next_uint64() == a1);
    REQUIRE(rng_a_replay.next_uint64() == a2);
}

void test_oracle_rng() {
    std::vector<bool> bits = {true, false, true, true, false};
    std::vector<std::uint64_t> words = {100, 200, 300};

    OracleRng oracle(bits, words);
    REQUIRE(oracle.next_bit() == true);
    REQUIRE(oracle.next_bit() == false);
    REQUIRE(oracle.next_bit() == true);
    REQUIRE(oracle.bits_consumed() == 3);

    REQUIRE(oracle.next_uint64() == 100);
    REQUIRE(oracle.next_uint64() == 200);
    REQUIRE(oracle.uint64_consumed() == 2);

    oracle.reset();
    REQUIRE(oracle.bits_consumed() == 0);
    REQUIRE(oracle.uint64_consumed() == 0);
    REQUIRE(oracle.next_bit() == true);
    REQUIRE(oracle.next_uint64() == 100);
}

int main() {
    std::cout << "Running test_case03: RNG / Randomness Layer\n";
    test_splitmix64_determinism();
    test_seed_hierarchy();
    test_xoshiro_determinism_and_replay();
    test_bit_consumption_accounting();
    test_independent_instances_no_global_state();
    test_oracle_rng();

    ::cvm::test::report_and_exit();
    return 0;
}
