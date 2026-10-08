#pragma once

#include "types.hpp"
#include "concepts.hpp"
#include "rng.hpp"
#include <vector>
#include <algorithm>
#include <cstdint>
#include <string_view>
#include <cmath>

namespace cvm::core {

/**
 * @brief Canonical implementation of CVM Algorithm 1 from S1 (Chakraborty et al. 2023)
 * according to PLAN Section 7.1 and AGENTS.md Case 05.
 *
 * Invariants and sequence:
 * Step A1.1: X <- X \ {a_t} (remove existing key)
 * Step A1.2: Sample with probability p = 2^-l via BernoulliPow2(l); insert if accepted.
 * Step A1.3: If |X| == s, independently discard each element with probability 1/2.
 * Step A1.4: p <- p/2, l <- l+1.
 * Step A1.5: If |X| == s, Abort!
 */
template <typename Key = std::uint64_t>
class CvmA1 {
private:
    std::size_t s_{0};
    RngInterface* rng_{nullptr};

    std::vector<Key> elements_;
    std::uint32_t l_{0};
    FailureStatus status_{FailureStatus::Success};
    bool aborted_{false};

    // Diagnostics & metrics
    std::size_t peak_buffer_size_{0};
    std::uint64_t reduction_rounds_{0};
    std::uint64_t dup_removals_{0};

    [[nodiscard]] bool bernoulli_pow2(std::uint32_t l) {
        if (l == 0) return true;
        for (std::uint32_t i = 0; i < l; ++i) {
            if (rng_->next_bit()) {
                return false;
            }
        }
        return true;
    }

public:
    explicit CvmA1(std::size_t s, RngInterface& rng)
        : s_(s), rng_(&rng)
    {
        if (s == 0) {
            throw std::invalid_argument("CvmA1 buffer capacity s must be >= 1");
        }
        elements_.reserve(s_);
    }

    void update(const Key& item) {
        if (aborted_) {
            return;
        }

        // Step A1.1: Remove existing key from X if present
        auto it = std::find(elements_.begin(), elements_.end(), item);
        if (it != elements_.end()) {
            // Swap-remove for O(1) unordered deletion
            *it = elements_.back();
            elements_.pop_back();
            ++dup_removals_;
        }

        // Step A1.2: Insert with probability p = 2^-l
        if (bernoulli_pow2(l_)) {
            elements_.push_back(item);
            if (elements_.size() > peak_buffer_size_) {
                peak_buffer_size_ = elements_.size();
            }
        }

        // Step A1.3: If |X| == s
        if (elements_.size() == s_) {
            ++reduction_rounds_;
            // Discard each element independently with probability 1/2
            std::size_t write_idx = 0;
            const std::size_t current_size = elements_.size();
            for (std::size_t read_idx = 0; read_idx < current_size; ++read_idx) {
                const bool discard = rng_->next_bit();
                if (!discard) {
                    elements_[write_idx++] = elements_[read_idx];
                }
            }
            elements_.resize(write_idx);

            // Step A1.4: p <- p/2, l <- l+1
            ++l_;

            // Step A1.5: If |X| == s after halving, Abort!
            if (elements_.size() == s_) {
                aborted_ = true;
                status_ = FailureStatus::Abort;
            }
        }
    }

    [[nodiscard]] EstimateResult estimate() const {
        if (aborted_ || status_ == FailureStatus::Abort) {
            return EstimateResult::abort();
        }
        if (elements_.empty()) {
            return EstimateResult::success(0.0);
        }

        // F_hat = |X| * 2^l
        const double est = static_cast<double>(elements_.size()) * std::ldexp(1.0, static_cast<int>(l_));
        if (!std::isfinite(est) || est < 0.0) {
            return EstimateResult::numeric_failure();
        }
        return EstimateResult::success(est);
    }

    void reset() {
        elements_.clear();
        l_ = 0;
        status_ = FailureStatus::Success;
        aborted_ = false;
        peak_buffer_size_ = 0;
        reduction_rounds_ = 0;
        dup_removals_ = 0;
    }

    [[nodiscard]] std::string_view name() const noexcept {
        return "CVM-A1";
    }

    [[nodiscard]] std::size_t buffer_size() const noexcept {
        return elements_.size();
    }

    [[nodiscard]] std::size_t capacity() const noexcept {
        return s_;
    }

    [[nodiscard]] std::uint32_t halving_count() const noexcept {
        return l_;
    }

    [[nodiscard]] bool is_aborted() const noexcept {
        return aborted_;
    }

    [[nodiscard]] FailureStatus status() const noexcept {
        return status_;
    }

    [[nodiscard]] std::size_t peak_buffer_size() const noexcept {
        return peak_buffer_size_;
    }

    [[nodiscard]] std::uint64_t reduction_rounds() const noexcept {
        return reduction_rounds_;
    }

    [[nodiscard]] std::uint64_t dup_removals() const noexcept {
        return dup_removals_;
    }
};

static_assert(CardinalityEstimator<CvmA1<std::uint64_t>, std::uint64_t>);

} // namespace cvm::core
