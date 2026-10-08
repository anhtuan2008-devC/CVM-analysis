#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <cmath>
#include <stdexcept>

namespace cvm::core {

/**
 * @brief Output failure status enum per PLAN Section 5.1 and Section 81.
 */
enum class FailureStatus : std::uint8_t {
    Success = 0,
    Abort = 1,
    NumericFailure = 2,
    ResourceFailure = 3
};

[[nodiscard]] constexpr std::string_view to_string(FailureStatus status) noexcept {
    switch (status) {
        case FailureStatus::Success: return "Success";
        case FailureStatus::Abort: return "Abort";
        case FailureStatus::NumericFailure: return "NumericFailure";
        case FailureStatus::ResourceFailure: return "ResourceFailure";
    }
    return "Unknown";
}

[[nodiscard]] constexpr std::optional<FailureStatus> failure_status_from_string(std::string_view sv) noexcept {
    if (sv == "Success") return FailureStatus::Success;
    if (sv == "Abort") return FailureStatus::Abort;
    if (sv == "NumericFailure") return FailureStatus::NumericFailure;
    if (sv == "ResourceFailure") return FailureStatus::ResourceFailure;
    return std::nullopt;
}

/**
 * @brief Distinct elements cardinality estimate result per PLAN Section 81.
 *
 * Invariant: has_value() <=> status() == FailureStatus::Success
 */
class EstimateResult {
private:
    std::optional<double> estimate_;
    FailureStatus status_;

    constexpr explicit EstimateResult(FailureStatus s, std::optional<double> est) noexcept
        : estimate_(est), status_(s) {}

public:
    static constexpr EstimateResult success(double v) noexcept {
        return EstimateResult(FailureStatus::Success, v);
    }

    static constexpr EstimateResult abort() noexcept {
        return EstimateResult(FailureStatus::Abort, std::nullopt);
    }

    static constexpr EstimateResult numeric_failure() noexcept {
        return EstimateResult(FailureStatus::NumericFailure, std::nullopt);
    }

    static constexpr EstimateResult resource_failure() noexcept {
        return EstimateResult(FailureStatus::ResourceFailure, std::nullopt);
    }

    [[nodiscard]] constexpr bool has_value() const noexcept {
        return status_ == FailureStatus::Success && estimate_.has_value();
    }

    [[nodiscard]] constexpr FailureStatus status() const noexcept {
        return status_;
    }

    [[nodiscard]] double value() const {
        if (!has_value()) {
            throw std::logic_error("EstimateResult contract violation: attempted to access value() on non-success result");
        }
        return *estimate_;
    }

    [[nodiscard]] constexpr std::optional<double> as_optional() const noexcept {
        return estimate_;
    }

    [[nodiscard]] constexpr bool operator==(const EstimateResult& other) const noexcept {
        return status_ == other.status_ && estimate_ == other.estimate_;
    }
};

/**
 * @brief Execution-valid predicate per PLAN Section 5.3:
 * execution_valid <=> status in {Success, Abort}
 */
[[nodiscard]] constexpr bool is_execution_valid(FailureStatus status) noexcept {
    return status == FailureStatus::Success || status == FailureStatus::Abort;
}

[[nodiscard]] constexpr bool is_execution_valid(const EstimateResult& res) noexcept {
    return is_execution_valid(res.status());
}

/**
 * @brief Approximation-valid predicate per PLAN Section 5.3:
 * - true if status == Success and RE <= epsilon
 * - false if status == Abort or (status == Success and RE > epsilon)
 * - nullopt if not execution_valid (NumericFailure / ResourceFailure)
 */
[[nodiscard]] inline std::optional<bool> is_approximation_valid(
    const EstimateResult& res, double true_f0, double epsilon) noexcept
{
    if (!is_execution_valid(res)) {
        return std::nullopt;
    }
    if (res.status() == FailureStatus::Abort) {
        return false;
    }
    if (true_f0 <= 0.0) {
        return false;
    }
    const double re = std::abs(res.value() - true_f0) / true_f0;
    return re <= epsilon;
}

[[nodiscard]] inline std::optional<bool> is_approximation_failure(
    const EstimateResult& res, double true_f0, double epsilon) noexcept
{
    auto valid = is_approximation_valid(res, true_f0, epsilon);
    if (!valid.has_value()) return std::nullopt;
    return !(*valid);
}

/**
 * @brief Algorithm identifiers per PLAN Section 2.2.
 */
enum class AlgorithmId : std::uint8_t {
    CVM_A1,
    CVM_W,
    Knuth_D,
    Knuth_Dprime
};

[[nodiscard]] constexpr std::string_view to_string(AlgorithmId alg) noexcept {
    switch (alg) {
        case AlgorithmId::CVM_A1: return "CVM-A1";
        case AlgorithmId::CVM_W: return "CVM-W";
        case AlgorithmId::Knuth_D: return "Knuth-D";
        case AlgorithmId::Knuth_Dprime: return "Knuth-Dprime";
    }
    return "Unknown";
}

[[nodiscard]] constexpr std::optional<AlgorithmId> algorithm_id_from_string(std::string_view sv) noexcept {
    if (sv == "CVM-A1") return AlgorithmId::CVM_A1;
    if (sv == "CVM-W") return AlgorithmId::CVM_W;
    if (sv == "Knuth-D") return AlgorithmId::Knuth_D;
    if (sv == "Knuth-Dprime") return AlgorithmId::Knuth_Dprime;
    return std::nullopt;
}

/**
 * @brief Controlled workload identifiers per PLAN Section 21.
 */
enum class WorkloadId : std::uint8_t {
    WL_UNIFORM,
    WL_DUP
};

[[nodiscard]] constexpr std::string_view to_string(WorkloadId wl) noexcept {
    switch (wl) {
        case WorkloadId::WL_UNIFORM: return "WL-UNIFORM";
        case WorkloadId::WL_DUP: return "WL-DUP";
    }
    return "Unknown";
}

[[nodiscard]] constexpr std::optional<WorkloadId> workload_id_from_string(std::string_view sv) noexcept {
    if (sv == "WL-UNIFORM") return WorkloadId::WL_UNIFORM;
    if (sv == "WL-DUP") return WorkloadId::WL_DUP;
    return std::nullopt;
}

/**
 * @brief RQ1 threshold classification states per PLAN Section 0.4 and Section 32.
 */
enum class ThresholdState : std::uint8_t {
    IDENTIFIED,
    STATISTICALLY_INCONCLUSIVE,
    LEFT_CENSORED,
    RIGHT_CENSORED
};

[[nodiscard]] constexpr std::string_view to_string(ThresholdState st) noexcept {
    switch (st) {
        case ThresholdState::IDENTIFIED: return "IDENTIFIED";
        case ThresholdState::STATISTICALLY_INCONCLUSIVE: return "STATISTICALLY_INCONCLUSIVE";
        case ThresholdState::LEFT_CENSORED: return "LEFT-CENSORED";
        case ThresholdState::RIGHT_CENSORED: return "RIGHT-CENSORED";
    }
    return "Unknown";
}

[[nodiscard]] constexpr std::optional<ThresholdState> threshold_state_from_string(std::string_view sv) noexcept {
    if (sv == "IDENTIFIED") return ThresholdState::IDENTIFIED;
    if (sv == "STATISTICALLY_INCONCLUSIVE") return ThresholdState::STATISTICALLY_INCONCLUSIVE;
    if (sv == "LEFT-CENSORED") return ThresholdState::LEFT_CENSORED;
    if (sv == "RIGHT-CENSORED") return ThresholdState::RIGHT_CENSORED;
    return std::nullopt;
}

/**
 * @brief Execution provenance metadata structure per PLAN Section 25.
 */
struct RunProvenance {
    std::string git_commit;
    std::string source_s1_version{"arXiv:2301.10191v2"};
    std::string source_s2_version{"Stanford-CS-revised-2023-12-29"};
    std::string compiler_version;
    std::string build_flags;
    std::string os_name;
    std::string cpu_model;
    std::string binary_hash;
    std::string source_tree_hash;
    std::string config_hash;
    std::string analysis_environment{"C++23-standalone"};
    std::string workload_generator_version{"1.0.0"};
    std::string schema_version{"3.1.2"};
};

/**
 * @brief Serialization helper for CSV output preserving null semantics.
 */
[[nodiscard]] inline std::string serialize_estimate_csv(const EstimateResult& res) {
    if (res.has_value()) {
        return std::to_string(res.value());
    }
    return "null";
}

} // namespace cvm::core
