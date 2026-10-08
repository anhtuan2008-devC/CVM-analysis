#pragma once

#include "types.hpp"
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <bit>

namespace cvm::core {

/**
 * @brief Canonical SplitMix64 generator (Sebastiano Vigna).
 * Used for seed mixing and initializing 256-bit PRNG state.
 */
class SplitMix64 {
private:
    std::uint64_t state_;

public:
    constexpr explicit SplitMix64(std::uint64_t seed = 0) noexcept : state_(seed) {}

    constexpr std::uint64_t next() noexcept {
        std::uint64_t z = (state_ += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    [[nodiscard]] constexpr std::uint64_t state() const noexcept {
        return state_;
    }

    constexpr void seed(std::uint64_t s) noexcept {
        state_ = s;
    }
};

/**
 * @brief Deterministic seed hierarchy derivation per PLAN Section 25 and AGENTS.md Case 03:
 * Master Seed -> Experiment Seed -> Algorithm Seed -> Trial Seed.
 */
class SeedHierarchy {
public:
    [[nodiscard]] static constexpr std::uint64_t mix_step(std::uint64_t current, std::uint64_t qualifier) noexcept {
        std::uint64_t z = current ^ (qualifier + 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    [[nodiscard]] static constexpr std::uint64_t derive_experiment_seed(
        std::uint64_t master_seed, std::uint64_t experiment_id) noexcept
    {
        return mix_step(master_seed, experiment_id);
    }

    [[nodiscard]] static constexpr std::uint64_t derive_algorithm_seed(
        std::uint64_t experiment_seed, AlgorithmId algorithm_id) noexcept
    {
        return mix_step(experiment_seed, static_cast<std::uint64_t>(algorithm_id));
    }

    [[nodiscard]] static constexpr std::uint64_t derive_trial_seed(
        std::uint64_t algorithm_seed, std::uint64_t trial_index) noexcept
    {
        return mix_step(algorithm_seed, trial_index);
    }

    [[nodiscard]] static constexpr std::uint64_t derive_full_trial_seed(
        std::uint64_t master_seed,
        std::uint64_t experiment_id,
        AlgorithmId algorithm_id,
        std::uint64_t trial_index) noexcept
    {
        const auto exp_seed = derive_experiment_seed(master_seed, experiment_id);
        const auto alg_seed = derive_algorithm_seed(exp_seed, algorithm_id);
        return derive_trial_seed(alg_seed, trial_index);
    }
};

/**
 * @brief Abstract interface for pseudo-random number generation across the project.
 */
class RngInterface {
public:
    virtual ~RngInterface() = default;

    /**
     * @brief Extract a single pseudo-random bit. Consumed by A1 / W.
     */
    virtual bool next_bit() = 0;

    /**
     * @brief Extract a 64-bit unsigned integer. Consumed by D / D' for volatility.
     */
    virtual std::uint64_t next_uint64() = 0;

    /**
     * @brief Cumulative count of individual bits consumed via next_bit().
     */
    [[nodiscard]] virtual std::uint64_t bits_consumed() const noexcept = 0;

    /**
     * @brief Cumulative count of 64-bit words consumed directly via next_uint64().
     */
    [[nodiscard]] virtual std::uint64_t uint64_consumed() const noexcept = 0;

    /**
     * @brief Reset RNG to its initial seeded state.
     */
    virtual void reset() = 0;
};

/**
 * @brief Production PRNG: xoshiro256** 1.0 (David Blackman & Sebastiano Vigna).
 * Initialized via SplitMix64 from a 64-bit seed.
 */
class Xoshiro256StarStar final : public RngInterface {
private:
    std::uint64_t initial_seed_;
    std::uint64_t s_[4];

    std::uint64_t bit_buffer_{0};
    std::uint8_t bits_remaining_{0};

    std::uint64_t bits_consumed_{0};
    std::uint64_t uint64_consumed_{0};

    static constexpr std::uint64_t rotl(std::uint64_t x, int k) noexcept {
        return (x << k) | (x >> (64 - k));
    }

    void init_state(std::uint64_t seed) noexcept {
        initial_seed_ = seed;
        SplitMix64 sm(seed);
        s_[0] = sm.next();
        s_[1] = sm.next();
        s_[2] = sm.next();
        s_[3] = sm.next();
        if ((s_[0] | s_[1] | s_[2] | s_[3]) == 0ULL) {
            s_[0] = 1ULL;
        }
        bit_buffer_ = 0;
        bits_remaining_ = 0;
        bits_consumed_ = 0;
        uint64_consumed_ = 0;
    }

    std::uint64_t raw_next_uint64() noexcept {
        const std::uint64_t result = rotl(s_[1] * 5, 7) * 9;
        const std::uint64_t t = s_[1] << 17;

        s_[2] ^= s_[0];
        s_[3] ^= s_[1];
        s_[1] ^= s_[2];
        s_[0] ^= s_[3];

        s_[2] ^= t;
        s_[3] = rotl(s_[3], 45);

        return result;
    }

public:
    explicit Xoshiro256StarStar(std::uint64_t seed = 0) noexcept {
        init_state(seed);
    }

    void reseed(std::uint64_t seed) noexcept {
        init_state(seed);
    }

    bool next_bit() override {
        if (bits_remaining_ == 0) {
            bit_buffer_ = raw_next_uint64();
            bits_remaining_ = 64;
        }
        --bits_remaining_;
        const bool bit = static_cast<bool>((bit_buffer_ >> bits_remaining_) & 1ULL);
        ++bits_consumed_;
        return bit;
    }

    std::uint64_t next_uint64() override {
        ++uint64_consumed_;
        return raw_next_uint64();
    }

    [[nodiscard]] std::uint64_t bits_consumed() const noexcept override {
        return bits_consumed_;
    }

    [[nodiscard]] std::uint64_t uint64_consumed() const noexcept override {
        return uint64_consumed_;
    }

    [[nodiscard]] std::uint64_t seed() const noexcept {
        return initial_seed_;
    }

    void reset() override {
        init_state(initial_seed_);
    }
};

/**
 * @brief Deterministic Oracle RNG tape for controlled testing and fixtures.
 */
class OracleRng final : public RngInterface {
private:
    std::vector<bool> bit_tape_;
    std::vector<std::uint64_t> word_tape_;
    std::size_t bit_index_{0};
    std::size_t word_index_{0};
    std::uint64_t bits_consumed_{0};
    std::uint64_t uint64_consumed_{0};

public:
    OracleRng(std::vector<bool> bit_tape, std::vector<std::uint64_t> word_tape)
        : bit_tape_(std::move(bit_tape)), word_tape_(std::move(word_tape)) {}

    explicit OracleRng(std::vector<bool> bit_tape)
        : bit_tape_(std::move(bit_tape)) {}

    explicit OracleRng(std::vector<std::uint64_t> word_tape)
        : word_tape_(std::move(word_tape)) {}

    bool next_bit() override {
        if (bit_tape_.empty()) {
            throw std::runtime_error("OracleRng bit tape is empty");
        }
        if (bit_index_ >= bit_tape_.size()) {
            throw std::runtime_error("OracleRng bit tape exhausted");
        }
        const bool b = bit_tape_[bit_index_++];
        ++bits_consumed_;
        return b;
    }

    std::uint64_t next_uint64() override {
        if (word_tape_.empty()) {
            throw std::runtime_error("OracleRng word tape is empty");
        }
        if (word_index_ >= word_tape_.size()) {
            throw std::runtime_error("OracleRng word tape exhausted");
        }
        const std::uint64_t w = word_tape_[word_index_++];
        ++uint64_consumed_;
        return w;
    }

    [[nodiscard]] std::uint64_t bits_consumed() const noexcept override {
        return bits_consumed_;
    }

    [[nodiscard]] std::uint64_t uint64_consumed() const noexcept override {
        return uint64_consumed_;
    }

    void reset() override {
        bit_index_ = 0;
        word_index_ = 0;
        bits_consumed_ = 0;
        uint64_consumed_ = 0;
    }
};

} // namespace cvm::core
