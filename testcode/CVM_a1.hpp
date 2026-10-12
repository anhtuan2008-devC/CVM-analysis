#pragma once

#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <cstdint>
#include <cmath>
#include <stdexcept>
#include <iostream>

namespace cvm_standalone {

/**
 * @brief Evaluation status for CVM-A1
 */
enum class Status {
    Success,
    Abort,
    NumericFailure
};

/**
 * @brief Canonical implementation of CVM Algorithm 1 (Chakraborty et al. 2023 - Algorithm 1)
 *
 * Streaming cardinality (F0) estimator.
 *
 * Canonical steps conforming to Paper:
 *  - Step A1.1: X <- X \ {a_t} (Remove item if present in buffer X).
 *  - Step A1.2: Sample with probability p = 2^-l; insert a_t into X if accepted.
 *  - Step A1.3: If |X| == s (buffer full):
 *               Independently discard each element in X with probability 1/2.
 *  - Step A1.4: p <- p / 2, l <- l + 1.
 *  - Step A1.5: If |X| == s after reduction, trigger Abort!
 *
 * Space Complexity Guarantee (Theorem 2):
 *  O( (log n / eps^2) * (log m + log(1/delta)) ) bits
 *  where:
 *   - m: stream length (total elements)
 *   - n: size of universe of elements (each item takes O(log n) bits)
 *   - eps: relative error bound (|F0_hat - F0| <= eps * F0)
 *   - delta: failure probability (success prob >= 1 - delta)
 *
 * Time Complexity:
 *  - Uses an internal hash index pos_ mapping Key -> buffer index.
 *  - Search & removal (A1.1) takes O(1) average time!
 *  - Total stream execution time is optimal O(m) instead of O(m * s).
 *
 * @tparam Key Data type of stream items (default: std::string)
 */
template <typename Key = std::string>
class CvmA1 {
private:
    std::size_t s_{0};                                   // Buffer capacity (s >= 1)
    std::uint32_t l_{0};                                 // Halving count (p = 2^-l)
    std::vector<Key> buffer_;                            // Buffer X storing elements
    std::unordered_map<Key, std::size_t> pos_;           // Hash index: item -> index in buffer_
    bool aborted_{false};                                // Abort flag
    Status status_{Status::Success};

    // Theoretical parameters (if initialized via Theorem 2)
    double epsilon_{0.0};
    double delta_{0.0};
    std::uint64_t stream_bound_m_{0};
    bool configured_by_theory_{false};

    // Internal standalone RNG: std::mt19937_64 (Mersenne Twister 64-bit)
    std::mt19937_64 rng_;

    // Diagnostics & metrics
    std::uint64_t reduction_rounds_{0};  // Number of halving triggers
    std::uint64_t dup_removals_{0};      // Number of duplicate removals (A1.1)
    std::size_t peak_buffer_size_{0};    // Peak recorded buffer size

    /**
     * @brief Generate a single fair random bit (probability 1/2 for 0 and 1)
     */
    bool next_bit() {
        return (rng_() & 1ULL) != 0;
    }

    /**
     * @brief Bernoulli trial with probability p = 2^-l
     * Flips l independent fair coins; accepts if all l coins are 0 (prob (1/2)^l)
     */
    bool bernoulli_pow2(std::uint32_t l) {
        if (l == 0) return true;
        for (std::uint32_t i = 0; i < l; ++i) {
            if (next_bit()) {
                return false;
            }
        }
        return true;
    }

public:
    /**
     * @brief Compute theoretical buffer capacity s according to Theorem 2 of Chakraborty et al. (ESA 2023):
     *        s = ceil( (12 / eps^2) * log2( (8 * m) / delta ) )
     *
     * @param epsilon Relative error bound (0 < epsilon < 1)
     * @param delta Failure probability (0 < delta < 1)
     * @param m Upper bound on stream length (m >= 1)
     * @return std::size_t Theoretical buffer capacity
     */
    static std::size_t compute_theoretical_capacity(double epsilon, double delta, std::uint64_t m) {
        if (epsilon <= 0.0 || epsilon >= 1.0) {
            throw std::invalid_argument("Epsilon must be in (0, 1)");
        }
        if (delta <= 0.0 || delta >= 1.0) {
            throw std::invalid_argument("Delta must be in (0, 1)");
        }
        if (m == 0) {
            throw std::invalid_argument("Stream bound m must be >= 1");
        }
        const double term = (8.0 * static_cast<double>(m)) / delta;
        const double s_val = (12.0 / (epsilon * epsilon)) * std::log2(term);
        return static_cast<std::size_t>(std::ceil(s_val));
    }

    /**
     * @brief Mode 1: Initialize CVM-A1 with an explicit buffer capacity s
     * @param buffer_capacity Buffer capacity s (must be >= 1)
     * @param seed Random seed
     */
    explicit CvmA1(std::size_t buffer_capacity, std::uint64_t seed = 1337ULL)
        : s_(buffer_capacity), rng_(seed)
    {
        if (s_ == 0) {
            throw std::invalid_argument("Buffer capacity s must be >= 1");
        }
        buffer_.reserve(s_);
        pos_.reserve(s_);
    }

    /**
     * @brief Mode 2: Initialize CVM-A1 using theoretical parameters (eps, delta, m)
     *        Automatically calculates s = ceil( (12 / eps^2) * log2(8m / delta) )
     * @param epsilon Relative error tolerance (e.g. 0.05 for 5%)
     * @param delta Failure probability (e.g. 0.01 for 99% confidence)
     * @param m Upper bound on stream length
     * @param seed Random seed
     */
    CvmA1(double epsilon, double delta, std::uint64_t m, std::uint64_t seed = 1337ULL)
        : s_(compute_theoretical_capacity(epsilon, delta, m)),
          epsilon_(epsilon),
          delta_(delta),
          stream_bound_m_(m),
          configured_by_theory_(true),
          rng_(seed)
    {
        buffer_.reserve(s_);
        pos_.reserve(s_);
    }

    /**
     * @brief Process a new item a_t from the stream
     * @param item Stream item
     */
    void update(const Key& item) {
        if (aborted_) {
            return;
        }

        // Step A1.1: X <- X \ {a_t} (O(1) lookup & swap-pop)
        auto it = pos_.find(item);
        if (it != pos_.end()) {
            std::size_t idx = it->second;
            std::size_t last_idx = buffer_.size() - 1;
            if (idx != last_idx) {
                buffer_[idx] = std::move(buffer_.back());
                pos_[buffer_[idx]] = idx;
            }
            buffer_.pop_back();
            pos_.erase(it);
            ++dup_removals_;
        }

        // Step A1.2: Insert item into X with probability p = 2^-l
        if (bernoulli_pow2(l_)) {
            pos_[item] = buffer_.size();
            buffer_.push_back(item);
            if (buffer_.size() > peak_buffer_size_) {
                peak_buffer_size_ = buffer_.size();
            }
        }

        // Step A1.3: If |X| == s, independently discard each element with probability 1/2
        if (buffer_.size() == s_) {
            ++reduction_rounds_;

            pos_.clear();
            std::size_t write_idx = 0;
            const std::size_t n = buffer_.size();
            for (std::size_t read_idx = 0; read_idx < n; ++read_idx) {
                const bool discard = next_bit(); // 1/2 probability to discard
                if (!discard) {
                    pos_[buffer_[read_idx]] = write_idx;
                    buffer_[write_idx++] = std::move(buffer_[read_idx]);
                }
            }
            buffer_.resize(write_idx);

            // Step A1.4: p <- p / 2, equivalent to l <- l + 1
            ++l_;

            // Step A1.5: If |X| == s after halving, trigger Abort!
            if (buffer_.size() == s_) {
                aborted_ = true;
                status_ = Status::Abort;
            }
        }
    }

    /**
     * @brief Compute the cardinality estimate: \hat{F}_0 = |X| * 2^l
     * @return double Estimate value, or -1.0 if aborted
     */
    [[nodiscard]] double estimate() const {
        if (aborted_ || status_ == Status::Abort) {
            return -1.0;
        }
        if (buffer_.empty()) {
            return 0.0;
        }

        // F_hat = |X| * 2^l
        const double est = static_cast<double>(buffer_.size()) * std::ldexp(1.0, static_cast<int>(l_));
        if (!std::isfinite(est) || est < 0.0) {
            return -1.0;
        }
        return est;
    }

    /**
     * @brief Reset algorithm state for a new run
     */
    void reset(std::uint64_t new_seed = 0) {
        buffer_.clear();
        pos_.clear();
        l_ = 0;
        aborted_ = false;
        status_ = Status::Success;
        reduction_rounds_ = 0;
        dup_removals_ = 0;
        peak_buffer_size_ = 0;
        if (new_seed != 0) {
            rng_.seed(new_seed);
        }
    }

    // Diagnostics and inspection getters
    [[nodiscard]] bool is_aborted() const noexcept { return aborted_; }
    [[nodiscard]] Status status() const noexcept { return status_; }
    [[nodiscard]] std::size_t buffer_size() const noexcept { return buffer_.size(); }
    [[nodiscard]] std::size_t capacity() const noexcept { return s_; }
    [[nodiscard]] std::uint32_t halving_rounds() const noexcept { return l_; }
    [[nodiscard]] std::uint64_t reduction_rounds() const noexcept { return reduction_rounds_; }
    [[nodiscard]] std::uint64_t dup_removals() const noexcept { return dup_removals_; }
    [[nodiscard]] std::size_t peak_buffer_size() const noexcept { return peak_buffer_size_; }
    [[nodiscard]] double current_probability() const noexcept {
        return std::ldexp(1.0, -static_cast<int>(l_));
    }
    [[nodiscard]] bool configured_by_theory() const noexcept { return configured_by_theory_; }
    [[nodiscard]] double theoretical_epsilon() const noexcept { return epsilon_; }
    [[nodiscard]] double theoretical_delta() const noexcept { return delta_; }
    [[nodiscard]] std::uint64_t stream_bound_m() const noexcept { return stream_bound_m_; }
};

} // namespace cvm_standalone
