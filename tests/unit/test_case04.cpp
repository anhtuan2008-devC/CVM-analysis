#include "cvm/core/rank.hpp"
#include "cvm/ds/cutoff64.hpp"
#include "test_utils.hpp"
#include <limits>

using namespace cvm::core;
using namespace cvm::ds;

void test_rank_boundaries() {
    REQUIRE(MAX_RANK == std::numeric_limits<std::uint64_t>::max());

    rank_t r0 = 0;
    rank_t r_max = MAX_RANK;

    REQUIRE(r0 < r_max);
    REQUIRE(r0 == 0);
    REQUIRE(r_max == 0xFFFFFFFFFFFFFFFFULL);
}

void test_cutoff_one() {
    auto c_one = Cutoff64::one();
    REQUIRE(c_one.is_one());
    REQUIRE(!c_one.is_rank());
    REQUIRE(c_one.kind() == Cutoff64::Kind::One);

    // ONE accepts all ranks
    REQUIRE(c_one.accepts(0));
    REQUIRE(c_one.accepts(1));
    REQUIRE(c_one.accepts(1000000ULL));
    REQUIRE(c_one.accepts(MAX_RANK));

    // rank_value on ONE throws logic_error
    bool threw = false;
    try {
        [[maybe_unused]] auto v = c_one.rank_value();
    } catch (const std::logic_error&) {
        threw = true;
    }
    REQUIRE(threw);
}

void test_cutoff_rank() {
    auto c0 = Cutoff64::rank(0);
    REQUIRE(!c0.is_one());
    REQUIRE(c0.is_rank());
    REQUIRE(c0.rank_value() == 0);

    // RANK(0) rejects all ranks since r < 0 is never true for uint64
    REQUIRE(!c0.accepts(0));
    REQUIRE(!c0.accepts(1));
    REQUIRE(!c0.accepts(MAX_RANK));

    // Acceptance strictly r < c
    rank_t pivot = 500ULL;
    auto c_pivot = Cutoff64::rank(pivot);

    // rank < cutoff
    REQUIRE(c_pivot.accepts(0));
    REQUIRE(c_pivot.accepts(499));

    // rank == cutoff
    REQUIRE(!c_pivot.accepts(500));

    // rank > cutoff
    REQUIRE(!c_pivot.accepts(501));
    REQUIRE(!c_pivot.accepts(MAX_RANK));
}

void test_dprime_level_semantics() {
    // l = 0: accept all
    REQUIRE(dprime_accepts(0, 0));
    REQUIRE(dprime_accepts(1000, 0));
    REQUIRE(dprime_accepts(MAX_RANK, 0));

    // l = 1: threshold = 2^63
    const std::uint64_t t1 = 1ULL << 63;
    REQUIRE(dprime_accepts(0, 1));
    REQUIRE(dprime_accepts(t1 - 1, 1));
    REQUIRE(!dprime_accepts(t1, 1));
    REQUIRE(!dprime_accepts(MAX_RANK, 1));

    // l = 63: threshold = 2^(64-63) = 2^1 = 2
    REQUIRE(dprime_accepts(0, 63));
    REQUIRE(dprime_accepts(1, 63));
    REQUIRE(!dprime_accepts(2, 63));
    REQUIRE(!dprime_accepts(MAX_RANK, 63));

    // l = 64: threshold = 2^(64-64) = 2^0 = 1
    REQUIRE(dprime_accepts(0, 64));
    REQUIRE(!dprime_accepts(1, 64));
    REQUIRE(!dprime_accepts(MAX_RANK, 64));

    // l = 65: reject all
    REQUIRE(!dprime_accepts(0, 65));
    REQUIRE(!dprime_accepts(1, 65));
    REQUIRE(!dprime_accepts(MAX_RANK, 65));

    // l > 65: reject all
    REQUIRE(!dprime_accepts(0, 66));
    REQUIRE(!dprime_accepts(0, 100));
    REQUIRE(!dprime_accepts(MAX_RANK, 1000));
}

void test_cutoff_level_consistency() {
    for (std::uint32_t level = 0; level <= 70; ++level) {
        auto c = dprime_cutoff_for_level(level);
        if (level == 0) {
            REQUIRE(c.is_one());
        } else if (level >= 65) {
            REQUIRE(c.is_rank());
            REQUIRE(c.rank_value() == 0);
        } else {
            REQUIRE(c.is_rank());
            const std::uint64_t expected_thresh = (level == 64) ? 1ULL : (1ULL << (64 - level));
            REQUIRE(c.rank_value() == expected_thresh);
        }

        // Test consistent acceptance
        REQUIRE(c.accepts(0) == dprime_accepts(0, level));
        REQUIRE(c.accepts(1) == dprime_accepts(1, level));
        REQUIRE(c.accepts(MAX_RANK) == dprime_accepts(MAX_RANK, level));
    }
}

void test_coupling_level_derivation() {
    // D = ONE => level = 0
    REQUIRE(dprime_expected_level_from_cutoff(Cutoff64::one()) == 0);

    // D = RANK(0) => level = 65
    REQUIRE(dprime_expected_level_from_cutoff(Cutoff64::rank(0)) == 65);

    // D = RANK(1) => q = 3 => e = 1 => 65 - 1 = 64
    REQUIRE(dprime_expected_level_from_cutoff(Cutoff64::rank(1)) == 64);

    // D = RANK(2^63) => e = 64 => 65 - 64 = 1
    REQUIRE(dprime_expected_level_from_cutoff(Cutoff64::rank(1ULL << 63)) == 1);

    // D = RANK(MAX_RANK) => e = 64 => 65 - 64 = 1
    REQUIRE(dprime_expected_level_from_cutoff(Cutoff64::rank(MAX_RANK)) == 1);
}

int main() {
    std::cout << "Running test_case04: Finite Rank and Cutoff64\n";
    test_rank_boundaries();
    test_cutoff_one();
    test_cutoff_rank();
    test_dprime_level_semantics();
    test_cutoff_level_consistency();
    test_coupling_level_derivation();

    ::cvm::test::report_and_exit();
    return 0;
}
