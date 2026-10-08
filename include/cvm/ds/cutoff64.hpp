#pragma once

#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace cvm::ds {

using rank_t = std::uint64_t;

/**
 * @brief Exact finite representation of Knuth-D cutoff per PLAN Section 14.1.
 *
 * Cutoff64 has two disjoint states:
 * - ONE: represents p = 1.0; all ranks are accepted.
 * - RANK(c): represents p = (c + 1/2) / 2^64; rank r is accepted iff r < c.
 */
class Cutoff64 {
public:
    enum class Kind : std::uint8_t {
        One,
        Rank
    };

private:
    Kind kind_{Kind::One};
    rank_t rank_c_{0};

    constexpr explicit Cutoff64(Kind k, rank_t c) noexcept
        : kind_(k), rank_c_(c) {}

public:
    constexpr Cutoff64() noexcept = default;

    [[nodiscard]] static constexpr Cutoff64 one() noexcept {
        return Cutoff64(Kind::One, 0);
    }

    [[nodiscard]] static constexpr Cutoff64 rank(rank_t c) noexcept {
        return Cutoff64(Kind::Rank, c);
    }

    [[nodiscard]] constexpr bool is_one() const noexcept {
        return kind_ == Kind::One;
    }

    [[nodiscard]] constexpr bool is_rank() const noexcept {
        return kind_ == Kind::Rank;
    }

    [[nodiscard]] constexpr Kind kind() const noexcept {
        return kind_;
    }

    [[nodiscard]] constexpr rank_t rank_value() const {
        if (!is_rank()) {
            throw std::logic_error("Cutoff64 contract violation: attempted to access rank_value() on ONE cutoff");
        }
        return rank_c_;
    }

    /**
     * @brief Acceptance predicate:
     * - If ONE: accepts all ranks.
     * - If RANK(c): accepts iff r < c.
     */
    [[nodiscard]] constexpr bool accepts(rank_t r) const noexcept {
        if (is_one()) {
            return true;
        }
        return r < rank_c_;
    }

    [[nodiscard]] constexpr bool operator==(const Cutoff64& other) const noexcept {
        if (kind_ != other.kind_) return false;
        if (kind_ == Kind::One) return true;
        return rank_c_ == other.rank_c_;
    }

    [[nodiscard]] constexpr bool operator!=(const Cutoff64& other) const noexcept {
        return !(*this == other);
    }
};

} // namespace cvm::ds
