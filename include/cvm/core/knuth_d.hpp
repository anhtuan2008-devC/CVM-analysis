#pragma once

#include "types.hpp"
#include "concepts.hpp"
#include "rng.hpp"
#include "../ds/cutoff64.hpp"
#include "../ds/indexed_heap.hpp"
#include <cstdint>
#include <cstddef>
#include <string_view>
#include <stdexcept>
#include <cmath>

namespace cvm::core {

/**
 * @brief Canonical implementation of Knuth Algorithm D from S2 (Knuth 2023)
 * according to PLAN Section 7.3, Section 14, Section 15, Section 84, and AGENTS.md Case 08.
 *
 * Sequence of operations:
 * - D1: Initialize buffer B = empty, cutoff p = 1 (Cutoff64::one()).
 * - D2: On termination, estimate F_hat = |B| / p.
 * - D3: Stream key a_t.
 * - D4: If a_t exists in B, remove old pair (duplicate deletion).
 * - D5: Generate fresh volatility deviate r = next_uint64().
 *       If r >= cutoff (or r == cutoff), reject.
 *       If r < cutoff and |B| < s, insert (a_t, r).
 * - D6: If r < cutoff and |B| == s:
 *       Let M = max stored rank in B.
 *       Case 1 (r > M): cutoff <- RANK(r), incoming key not stored.
 *       Case 2 (r < M): cutoff <- RANK(M), purge stored rank >= M, insert (a_t, r).
 *       Case 3 (r == M): cutoff <- RANK(M), purge stored rank >= M, incoming key rejected.
 */
template <typename Key = std::uint64_t>
class KnuthD {
private:
    std::size_t s_{0};
    RngInterface* rng_{nullptr};

    cvm::ds::IndexedHeap<Key> heap_;
    cvm::ds::Cutoff64 cutoff_{cvm::ds::Cutoff64::one()};

    // Diagnostics & metrics
    std::size_t peak_buffer_size_{0};
    std::uint64_t reduction_rounds_{0};
    std::uint64_t dup_removals_{0};
    std::uint64_t cutoff_tie_events_{0};
    std::uint64_t rank_collision_events_{0};
    std::uint64_t equal_max_rank_events_{0};

public:
    explicit KnuthD(std::size_t s, RngInterface& rng)
        : s_(s), rng_(&rng), heap_(s)
    {
        if (s == 0) {
            throw std::invalid_argument("KnuthD buffer capacity s must be >= 1");
        }
        heap_.reserve(s_);
    }

    void reset() {
        heap_.clear();
        cutoff_ = cvm::ds::Cutoff64::one();
        peak_buffer_size_ = 0;
        reduction_rounds_ = 0;
        dup_removals_ = 0;
        cutoff_tie_events_ = 0;
        rank_collision_events_ = 0;
        equal_max_rank_events_ = 0;
    }

    /**
     * @brief Process stream item a_t according to steps D3-D6.
     */
    void update(const Key& item) {
        // Step D4: If current key exists in B, remove old pair
        if (heap_.erase(item)) {
            ++dup_removals_;
        }

        // Step D5: Generate fresh volatility deviate r
        const cvm::ds::rank_t r = rng_->next_uint64();

        // Check acceptance against cutoff
        if (!cutoff_.accepts(r)) {
            if (cutoff_.is_rank() && r == cutoff_.rank_value()) {
                ++cutoff_tie_events_;
            }
            return;
        }

        // If buffer has room (|B| < s), insert immediately
        if (heap_.size() < s_) {
            heap_.insert(item, r);
            if (heap_.size() > peak_buffer_size_) {
                peak_buffer_size_ = heap_.size();
            }
            return;
        }

        // Step D6: |B| == s
        const cvm::ds::rank_t M = heap_.max_rank();

        if (r > M) {
            // Case 1: incoming rank > max stored rank
            cutoff_ = cvm::ds::Cutoff64::rank(r);
            ++reduction_rounds_;
            // Incoming key is NOT stored
        } else if (r < M) {
            // Case 2: incoming rank < max stored rank
            cutoff_ = cvm::ds::Cutoff64::rank(M);
            ++reduction_rounds_;
            const std::size_t purged = heap_.purge_ge(M);
            if (purged > 1) {
                ++equal_max_rank_events_;
            }
            heap_.insert(item, r);
        } else {
            // Case 3: r == M (finite tie)
            ++rank_collision_events_;
            ++cutoff_tie_events_;
            cutoff_ = cvm::ds::Cutoff64::rank(M);
            ++reduction_rounds_;
            const std::size_t purged = heap_.purge_ge(M);
            if (purged > 1) {
                ++equal_max_rank_events_;
            }
            // Incoming rank equals new cutoff, so rejected (not stored)
        }
    }

    /**
     * @brief Direct update with explicit rank deviate (useful for deterministic tests).
     */
    void update_with_rank(const Key& item, cvm::ds::rank_t r) {
        if (heap_.erase(item)) {
            ++dup_removals_;
        }

        if (!cutoff_.accepts(r)) {
            if (cutoff_.is_rank() && r == cutoff_.rank_value()) {
                ++cutoff_tie_events_;
            }
            return;
        }

        if (heap_.size() < s_) {
            heap_.insert(item, r);
            if (heap_.size() > peak_buffer_size_) {
                peak_buffer_size_ = heap_.size();
            }
            return;
        }

        const cvm::ds::rank_t M = heap_.max_rank();

        if (r > M) {
            cutoff_ = cvm::ds::Cutoff64::rank(r);
            ++reduction_rounds_;
        } else if (r < M) {
            cutoff_ = cvm::ds::Cutoff64::rank(M);
            ++reduction_rounds_;
            const std::size_t purged = heap_.purge_ge(M);
            if (purged > 1) {
                ++equal_max_rank_events_;
            }
            heap_.insert(item, r);
        } else {
            ++rank_collision_events_;
            ++cutoff_tie_events_;
            cutoff_ = cvm::ds::Cutoff64::rank(M);
            ++reduction_rounds_;
            const std::size_t purged = heap_.purge_ge(M);
            if (purged > 1) {
                ++equal_max_rank_events_;
            }
        }
    }

    [[nodiscard]] EstimateResult estimate() const {
        if (heap_.empty()) {
            return EstimateResult::success(0.0);
        }

        if (cutoff_.is_one()) {
            return EstimateResult::success(static_cast<double>(heap_.size()));
        }

        static constexpr double TWO_TO_64 = 18446744073709551616.0;
        const double c_val = static_cast<double>(cutoff_.rank_value()) + 0.5;
        const double est = static_cast<double>(heap_.size()) * (TWO_TO_64 / c_val);

        if (!std::isfinite(est) || est < 0.0) {
            return EstimateResult::numeric_failure();
        }
        return EstimateResult::success(est);
    }

    [[nodiscard]] std::string_view name() const noexcept {
        return "Knuth-D";
    }

    // Accessors & Diagnostics
    [[nodiscard]] std::size_t size() const noexcept { return heap_.size(); }
    [[nodiscard]] std::size_t capacity() const noexcept { return s_; }
    [[nodiscard]] bool empty() const noexcept { return heap_.empty(); }
    [[nodiscard]] cvm::ds::Cutoff64 cutoff() const noexcept { return cutoff_; }
    [[nodiscard]] std::size_t peak_buffer_size() const noexcept { return peak_buffer_size_; }
    [[nodiscard]] std::uint64_t reduction_rounds() const noexcept { return reduction_rounds_; }
    [[nodiscard]] std::uint64_t dup_removals() const noexcept { return dup_removals_; }
    [[nodiscard]] std::uint64_t cutoff_tie_events() const noexcept { return cutoff_tie_events_; }
    [[nodiscard]] std::uint64_t rank_collision_events() const noexcept { return rank_collision_events_; }
    [[nodiscard]] std::uint64_t equal_max_rank_events() const noexcept { return equal_max_rank_events_; }
    [[nodiscard]] const cvm::ds::IndexedHeap<Key>& heap() const noexcept { return heap_; }

    /**
     * @brief Checks mandatory invariants:
     * 1. |B| <= s
     * 2. Heap and index mapping consistency
     * 3. Strict cutoff: for all stored elements, rank < cutoff
     */
    [[nodiscard]] bool check_invariants() const noexcept {
        if (heap_.size() > s_) {
            return false;
        }
        if (!heap_.check_invariants()) {
            return false;
        }
        for (const auto& node : heap_.nodes()) {
            if (!cutoff_.accepts(node.rank)) {
                return false;
            }
        }
        return true;
    }
};

static_assert(CardinalityEstimator<KnuthD<std::uint64_t>, std::uint64_t>, "KnuthD must satisfy CardinalityEstimator concept");

} // namespace cvm::core
