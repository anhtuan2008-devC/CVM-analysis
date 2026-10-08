#pragma once

#include "types.hpp"
#include "concepts.hpp"
#include "rng.hpp"
#include "rank.hpp"
#include "../ds/cutoff64.hpp"
#include "../ds/indexed_heap.hpp"
#include <cstdint>
#include <cstddef>
#include <string_view>
#include <stdexcept>
#include <cmath>

namespace cvm::core {

/**
 * @brief Canonical implementation of Knuth Algorithm D' ("A Coarser Method") from S2 (Knuth 2023)
 * according to PLAN Section 7.6, Section 16, Section 17.2, and AGENTS.md Case 09.
 *
 * Distinct features:
 * - Binary dyadic cutoff: p = 2^-l represented strictly by integer exponent l (0 <= l <= 65).
 * - Exact integer acceptance predicate: dprime_accepts(r, l).
 *   - l = 0: accept all
 *   - 1 <= l <= 64: accept iff r < 2^(64 - l)
 *   - l >= 65: reject all
 * - Zero floating-point comparisons in acceptance decisions.
 * - Cascade halving / purge when buffer is full and incoming rank is accepted.
 */
template <typename Key = std::uint64_t>
class KnuthDprime {
private:
    std::size_t s_{0};
    RngInterface* rng_{nullptr};

    cvm::ds::IndexedHeap<Key> heap_;
    std::uint32_t l_{0};

    // Diagnostics & metrics
    std::size_t peak_buffer_size_{0};
    std::uint64_t reduction_rounds_{0};
    std::uint64_t dup_removals_{0};
    std::uint64_t cutoff_tie_events_{0};
    std::uint64_t equal_max_rank_events_{0};
    std::uint64_t finite_rank_resolution_events_{0};

public:
    explicit KnuthDprime(std::size_t s, RngInterface& rng)
        : s_(s), rng_(&rng), heap_(s)
    {
        if (s == 0) {
            throw std::invalid_argument("KnuthDprime buffer capacity s must be >= 1");
        }
        heap_.reserve(s_);
    }

    void reset() {
        heap_.clear();
        l_ = 0;
        peak_buffer_size_ = 0;
        reduction_rounds_ = 0;
        dup_removals_ = 0;
        cutoff_tie_events_ = 0;
        equal_max_rank_events_ = 0;
        finite_rank_resolution_events_ = 0;
    }

    /**
     * @brief Process stream item a_t according to Algorithm D' semantics.
     */
    void update(const Key& item) {
        // Step D4: Duplicate deletion
        if (heap_.erase(item)) {
            ++dup_removals_;
        }

        // Step D5: Generate fresh volatility deviate r
        const rank_t r = rng_->next_uint64();

        // Check acceptance under current level l_
        if (!dprime_accepts(r, l_)) {
            if (l_ >= 1 && l_ <= 64) {
                const rank_t threshold = (l_ == 64) ? 1ULL : (1ULL << (64 - l_));
                if (r == threshold) {
                    ++cutoff_tie_events_;
                }
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

        // Step D6': Buffer full (|B| == s) and incoming r accepted under current level l_
        // Repeat halving while buffer remains full and incoming r is still accepted
        while (heap_.size() == s_ && dprime_accepts(r, l_)) {
            if (l_ >= 64) {
                l_ = 65;
                ++finite_rank_resolution_events_;
                heap_.clear();
                return;
            }

            ++l_;
            ++reduction_rounds_;

            const rank_t new_threshold = (l_ == 64) ? 1ULL : (1ULL << (64 - l_));
            const std::size_t purged = heap_.purge_ge(new_threshold);
            if (purged > 1) {
                ++equal_max_rank_events_;
            }
        }

        // After purge cascade, insert incoming item iff still accepted and buffer has room
        if (dprime_accepts(r, l_) && heap_.size() < s_) {
            heap_.insert(item, r);
            if (heap_.size() > peak_buffer_size_) {
                peak_buffer_size_ = heap_.size();
            }
        }
    }

    /**
     * @brief Direct update with explicit rank deviate for deterministic test scenarios.
     */
    void update_with_rank(const Key& item, rank_t r) {
        if (heap_.erase(item)) {
            ++dup_removals_;
        }

        if (!dprime_accepts(r, l_)) {
            if (l_ >= 1 && l_ <= 64) {
                const rank_t threshold = (l_ == 64) ? 1ULL : (1ULL << (64 - l_));
                if (r == threshold) {
                    ++cutoff_tie_events_;
                }
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

        while (heap_.size() == s_ && dprime_accepts(r, l_)) {
            if (l_ >= 64) {
                l_ = 65;
                ++finite_rank_resolution_events_;
                heap_.clear();
                return;
            }

            ++l_;
            ++reduction_rounds_;

            const rank_t new_threshold = (l_ == 64) ? 1ULL : (1ULL << (64 - l_));
            const std::size_t purged = heap_.purge_ge(new_threshold);
            if (purged > 1) {
                ++equal_max_rank_events_;
            }
        }

        if (dprime_accepts(r, l_) && heap_.size() < s_) {
            heap_.insert(item, r);
            if (heap_.size() > peak_buffer_size_) {
                peak_buffer_size_ = heap_.size();
            }
        }
    }

    [[nodiscard]] EstimateResult estimate() const {
        if (l_ >= 65) {
            return EstimateResult::numeric_failure();
        }

        if (heap_.empty()) {
            return EstimateResult::success(0.0);
        }

        if (l_ == 0) {
            return EstimateResult::success(static_cast<double>(heap_.size()));
        }

        // F_hat = |B| * 2^l
        const double est = static_cast<double>(heap_.size()) * std::ldexp(1.0, static_cast<int>(l_));
        if (!std::isfinite(est) || est < 0.0) {
            return EstimateResult::numeric_failure();
        }
        return EstimateResult::success(est);
    }

    [[nodiscard]] std::string_view name() const noexcept {
        return "Knuth-Dprime";
    }

    // Accessors & Diagnostics
    [[nodiscard]] std::size_t size() const noexcept { return heap_.size(); }
    [[nodiscard]] std::size_t capacity() const noexcept { return s_; }
    [[nodiscard]] bool empty() const noexcept { return heap_.empty(); }
    [[nodiscard]] std::uint32_t level() const noexcept { return l_; }
    [[nodiscard]] cvm::ds::Cutoff64 cutoff() const noexcept { return dprime_cutoff_for_level(l_); }
    [[nodiscard]] std::size_t peak_buffer_size() const noexcept { return peak_buffer_size_; }
    [[nodiscard]] std::uint64_t reduction_rounds() const noexcept { return reduction_rounds_; }
    [[nodiscard]] std::uint64_t dup_removals() const noexcept { return dup_removals_; }
    [[nodiscard]] std::uint64_t cutoff_tie_events() const noexcept { return cutoff_tie_events_; }
    [[nodiscard]] std::uint64_t equal_max_rank_events() const noexcept { return equal_max_rank_events_; }
    [[nodiscard]] std::uint64_t finite_rank_resolution_events() const noexcept { return finite_rank_resolution_events_; }
    [[nodiscard]] const cvm::ds::IndexedHeap<Key>& heap() const noexcept { return heap_; }

    /**
     * @brief Checks mandatory invariants:
     * 1. |B| <= s
     * 2. Heap and index mapping consistency
     * 3. Strict cutoff: for all stored elements, dprime_accepts(rank, l_) is true
     */
    [[nodiscard]] bool check_invariants() const noexcept {
        if (heap_.size() > s_) {
            return false;
        }
        if (!heap_.check_invariants()) {
            return false;
        }
        for (const auto& node : heap_.nodes()) {
            if (!dprime_accepts(node.rank, l_)) {
                return false;
            }
        }
        return true;
    }
};

static_assert(CardinalityEstimator<KnuthDprime<std::uint64_t>, std::uint64_t>, "KnuthDprime must satisfy CardinalityEstimator concept");

} // namespace cvm::core
