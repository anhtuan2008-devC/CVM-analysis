#pragma once

#include "types.hpp"
#include "concepts.hpp"
#include "rng.hpp"
#include <vector>
#include <algorithm>
#include <cstdint>
#include <string_view>
#include <cmath>
#include <stdexcept>

namespace cvm::core {

/**
 * @brief Reduction transition event record per PLAN Section 37 and 38.
 */
struct ReductionEvent {
    std::uint64_t episode_index{0};
    std::uint64_t transition_index{0};
    std::uint64_t step_t{0};
    std::uint32_t level_before{0};
    std::uint32_t level_after{0};
    std::uint32_t cascade_index{0};
    std::uint64_t delta_t{0};
    bool delta_t_valid{false};
};

/**
 * @brief Canonical implementation of CVM-W (Knuth's modified Algorithm X from S2 Appendix)
 * according to PLAN Section 7.2, Section 8, Section 37, and AGENTS.md Case 06.
 *
 * Differences from A1:
 * - 'if |X| == s' becomes 'while |X| == s'.
 * - No source-level abort.
 * - Post-update invariant: |X| <= s - 1.
 * - s >= 2.
 * - Same-step cascades record delta_t = 0 and delta_t_valid = false without converting 0 to 1.
 */
template <typename Key = std::uint64_t>
class CvmW {
private:
    std::size_t s_{0};
    RngInterface* rng_{nullptr};

    std::vector<Key> elements_;
    std::uint32_t l_{0};
    std::uint64_t current_step_t_{0};

    // Reduction tracing
    std::vector<ReductionEvent> reduction_events_;
    std::uint64_t episode_count_{0};
    std::uint64_t total_transitions_{0};
    std::uint64_t prev_transition_step_t_{0};
    bool has_prev_transition_{false};

    // Metrics
    std::size_t peak_buffer_size_{0};
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
    explicit CvmW(std::size_t s, RngInterface& rng)
        : s_(s), rng_(&rng)
    {
        if (s < 2) {
            throw std::invalid_argument("CvmW buffer capacity s must be >= 2 per PLAN Section 7.2");
        }
        elements_.reserve(s_);
    }

    void update(const Key& item) {
        ++current_step_t_;

        // Step W1: Remove existing key from X if present
        auto it = std::find(elements_.begin(), elements_.end(), item);
        if (it != elements_.end()) {
            *it = elements_.back();
            elements_.pop_back();
            ++dup_removals_;
        }

        // Step W2: Sample with probability p = 2^-l
        if (bernoulli_pow2(l_)) {
            elements_.push_back(item);
            if (elements_.size() > peak_buffer_size_) {
                peak_buffer_size_ = elements_.size();
            }
        }

        // Step W3: While |X| == s, repeatedly halving until |X| <= s - 1
        if (elements_.size() == s_) {
            ++episode_count_;
            std::uint32_t cascade_in_episode = 0;

            while (elements_.size() == s_) {
                const std::uint32_t lvl_before = l_;

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

                // p <- p/2, l <- l+1
                ++l_;
                const std::uint32_t lvl_after = l_;

                // Compute delta_t
                std::uint64_t dt = 0;
                bool dt_valid = false;
                if (has_prev_transition_) {
                    dt = current_step_t_ - prev_transition_step_t_;
                    dt_valid = (dt > 0);
                } else {
                    dt = current_step_t_;
                    dt_valid = true;
                }

                ReductionEvent ev{
                    .episode_index = episode_count_,
                    .transition_index = ++total_transitions_,
                    .step_t = current_step_t_,
                    .level_before = lvl_before,
                    .level_after = lvl_after,
                    .cascade_index = cascade_in_episode++,
                    .delta_t = dt,
                    .delta_t_valid = dt_valid
                };
                reduction_events_.push_back(ev);

                prev_transition_step_t_ = current_step_t_;
                has_prev_transition_ = true;
            }
        }
    }

    [[nodiscard]] EstimateResult estimate() const {
        if (elements_.empty()) {
            return EstimateResult::success(0.0);
        }

        const double est = static_cast<double>(elements_.size()) * std::ldexp(1.0, static_cast<int>(l_));
        if (!std::isfinite(est) || est < 0.0) {
            return EstimateResult::numeric_failure();
        }
        return EstimateResult::success(est);
    }

    void reset() {
        elements_.clear();
        l_ = 0;
        current_step_t_ = 0;
        reduction_events_.clear();
        episode_count_ = 0;
        total_transitions_ = 0;
        prev_transition_step_t_ = 0;
        has_prev_transition_ = false;
        peak_buffer_size_ = 0;
        dup_removals_ = 0;
    }

    [[nodiscard]] std::string_view name() const noexcept {
        return "CVM-W";
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

    [[nodiscard]] const std::vector<ReductionEvent>& reduction_events() const noexcept {
        return reduction_events_;
    }

    [[nodiscard]] std::uint64_t episode_count() const noexcept {
        return episode_count_;
    }

    [[nodiscard]] std::uint64_t total_transitions() const noexcept {
        return total_transitions_;
    }

    [[nodiscard]] std::size_t peak_buffer_size() const noexcept {
        return peak_buffer_size_;
    }

    [[nodiscard]] std::uint64_t dup_removals() const noexcept {
        return dup_removals_;
    }
};

static_assert(CardinalityEstimator<CvmW<std::uint64_t>, std::uint64_t>);

} // namespace cvm::core
