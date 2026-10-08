#pragma once

#include "cvm/ds/cutoff64.hpp"
#include <cstdint>
#include <bit>
#include <limits>

namespace cvm::core {

using rank_t = cvm::ds::rank_t;
using Cutoff64 = cvm::ds::Cutoff64;

/**
 * @brief Maximum possible rank value (2^64 - 1).
 */
inline constexpr rank_t MAX_RANK = std::numeric_limits<rank_t>::max();

/**
 * @brief Dprime binary level acceptance predicate per PLAN Section 16:
 * - l = 0: accept all
 * - 1 <= l <= 64: accept iff r < 2^(64 - l)
 * - l >= 65: reject all
 */
[[nodiscard]] constexpr bool dprime_accepts(rank_t r, std::uint32_t level) noexcept {
    if (level == 0) {
        return true;
    }
    if (level >= 65) {
        return false;
    }
    const std::uint64_t threshold = (level == 64) ? 1ULL : (1ULL << (64 - level));
    return r < threshold;
}

/**
 * @brief Cutoff value matching D' binary level l per PLAN Section 10.1 and Section 16.
 */
[[nodiscard]] constexpr Cutoff64 dprime_cutoff_for_level(std::uint32_t level) noexcept {
    if (level == 0) {
        return Cutoff64::one();
    }
    if (level >= 65) {
        return Cutoff64::rank(0);
    }
    const std::uint64_t threshold = (level == 64) ? 1ULL : (1ULL << (64 - level));
    return Cutoff64::rank(threshold);
}

/**
 * @brief Exact mathematical coupling function computing the expected Dprime level
 * for a given Knuth-D Cutoff64 per PLAN Section 10.1:
 * - D = ONE => expected level = 0
 * - D = RANK(c) => q = 2*c + 1, e = floor(log2(q)), expected level = 65 - e
 */
[[nodiscard]] constexpr std::uint32_t dprime_expected_level_from_cutoff(const Cutoff64& cutoff) noexcept {
    if (cutoff.is_one()) {
        return 0;
    }
    const rank_t c = cutoff.rank_value();
    if (c == 0) {
        // q = 1 => floor(log2(1)) = 0 => 65 - 0 = 65
        return 65;
    }
    // For c > 0, q = 2*c + 1. Since 2*c has highest bit at (64 - countl_zero(c)),
    // and +1 cannot overflow past that highest bit:
    // e = floor(log2(2*c + 1)) = 64 - std::countl_zero(c).
    const std::uint32_t e = 64 - static_cast<std::uint32_t>(std::countl_zero(c));
    return 65 - e;
}

} // namespace cvm::core
