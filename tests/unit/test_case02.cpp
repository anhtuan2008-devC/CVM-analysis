#include "cvm/core/types.hpp"
#include "cvm/core/concepts.hpp"
#include "test_utils.hpp"
#include <cmath>

using namespace cvm::core;

void test_success_result() {
    auto res = EstimateResult::success(42.0);
    REQUIRE(res.has_value());
    REQUIRE(res.status() == FailureStatus::Success);
    REQUIRE(res.value() == 42.0);
    REQUIRE(res.as_optional().has_value());
    REQUIRE(*res.as_optional() == 42.0);
    REQUIRE(is_execution_valid(res));
    REQUIRE(serialize_estimate_csv(res) == "42.000000" || serialize_estimate_csv(res).starts_with("42"));
}

void test_abort_result() {
    auto res = EstimateResult::abort();
    REQUIRE(!res.has_value());
    REQUIRE(res.status() == FailureStatus::Abort);
    REQUIRE(!res.as_optional().has_value());
    REQUIRE(is_execution_valid(res));
    REQUIRE(serialize_estimate_csv(res) == "null");

    bool threw = false;
    try {
        [[maybe_unused]] auto val = res.value();
    } catch (const std::logic_error&) {
        threw = true;
    }
    REQUIRE(threw);
}

void test_numeric_failure_result() {
    auto res = EstimateResult::numeric_failure();
    REQUIRE(!res.has_value());
    REQUIRE(res.status() == FailureStatus::NumericFailure);
    REQUIRE(!res.as_optional().has_value());
    REQUIRE(!is_execution_valid(res));
    REQUIRE(serialize_estimate_csv(res) == "null");

    bool threw = false;
    try {
        [[maybe_unused]] auto val = res.value();
    } catch (const std::logic_error&) {
        threw = true;
    }
    REQUIRE(threw);
}

void test_resource_failure_result() {
    auto res = EstimateResult::resource_failure();
    REQUIRE(!res.has_value());
    REQUIRE(res.status() == FailureStatus::ResourceFailure);
    REQUIRE(!res.as_optional().has_value());
    REQUIRE(!is_execution_valid(res));
    REQUIRE(serialize_estimate_csv(res) == "null");

    bool threw = false;
    try {
        [[maybe_unused]] auto val = res.value();
    } catch (const std::logic_error&) {
        threw = true;
    }
    REQUIRE(threw);
}

void test_status_has_value_equivalence() {
    auto r_succ = EstimateResult::success(100.5);
    auto r_abort = EstimateResult::abort();
    auto r_num = EstimateResult::numeric_failure();
    auto r_res = EstimateResult::resource_failure();

    REQUIRE(r_succ.has_value() == (r_succ.status() == FailureStatus::Success));
    REQUIRE(r_abort.has_value() == (r_abort.status() == FailureStatus::Success));
    REQUIRE(r_num.has_value() == (r_num.status() == FailureStatus::Success));
    REQUIRE(r_res.has_value() == (r_res.status() == FailureStatus::Success));
}

void test_approximation_validity() {
    double true_f0 = 1000.0;
    double eps = 0.1; // 10% tolerance => [900, 1100]

    auto r_good = EstimateResult::success(1050.0);
    auto v_good = is_approximation_valid(r_good, true_f0, eps);
    REQUIRE(v_good.has_value());
    REQUIRE(*v_good == true);

    auto r_bad = EstimateResult::success(1200.0);
    auto v_bad = is_approximation_valid(r_bad, true_f0, eps);
    REQUIRE(v_bad.has_value());
    REQUIRE(*v_bad == false);

    auto r_abort = EstimateResult::abort();
    auto v_abort = is_approximation_valid(r_abort, true_f0, eps);
    REQUIRE(v_abort.has_value());
    REQUIRE(*v_abort == false);

    auto r_num = EstimateResult::numeric_failure();
    auto v_num = is_approximation_valid(r_num, true_f0, eps);
    REQUIRE(!v_num.has_value()); // Not an approximation failure

    auto r_res = EstimateResult::resource_failure();
    auto v_res = is_approximation_valid(r_res, true_f0, eps);
    REQUIRE(!v_res.has_value()); // Not an approximation failure
}

void test_schema_enums_and_strings() {
    // FailureStatus
    REQUIRE(to_string(FailureStatus::Success) == "Success");
    REQUIRE(to_string(FailureStatus::Abort) == "Abort");
    REQUIRE(to_string(FailureStatus::NumericFailure) == "NumericFailure");
    REQUIRE(to_string(FailureStatus::ResourceFailure) == "ResourceFailure");
    REQUIRE(failure_status_from_string("Success") == FailureStatus::Success);
    REQUIRE(failure_status_from_string("Abort") == FailureStatus::Abort);
    REQUIRE(failure_status_from_string("NumericFailure") == FailureStatus::NumericFailure);
    REQUIRE(failure_status_from_string("ResourceFailure") == FailureStatus::ResourceFailure);
    REQUIRE(!failure_status_from_string("Invalid").has_value());

    // AlgorithmId
    REQUIRE(to_string(AlgorithmId::CVM_A1) == "CVM-A1");
    REQUIRE(to_string(AlgorithmId::CVM_W) == "CVM-W");
    REQUIRE(to_string(AlgorithmId::Knuth_D) == "Knuth-D");
    REQUIRE(to_string(AlgorithmId::Knuth_Dprime) == "Knuth-Dprime");
    REQUIRE(algorithm_id_from_string("CVM-A1") == AlgorithmId::CVM_A1);
    REQUIRE(algorithm_id_from_string("CVM-W") == AlgorithmId::CVM_W);
    REQUIRE(algorithm_id_from_string("Knuth-D") == AlgorithmId::Knuth_D);
    REQUIRE(algorithm_id_from_string("Knuth-Dprime") == AlgorithmId::Knuth_Dprime);
    REQUIRE(!algorithm_id_from_string("HLL").has_value());

    // WorkloadId
    REQUIRE(to_string(WorkloadId::WL_UNIFORM) == "WL-UNIFORM");
    REQUIRE(to_string(WorkloadId::WL_DUP) == "WL-DUP");
    REQUIRE(workload_id_from_string("WL-UNIFORM") == WorkloadId::WL_UNIFORM);
    REQUIRE(workload_id_from_string("WL-DUP") == WorkloadId::WL_DUP);
    REQUIRE(!workload_id_from_string("Zipf").has_value());

    // ThresholdState
    REQUIRE(to_string(ThresholdState::IDENTIFIED) == "IDENTIFIED");
    REQUIRE(to_string(ThresholdState::STATISTICALLY_INCONCLUSIVE) == "STATISTICALLY_INCONCLUSIVE");
    REQUIRE(to_string(ThresholdState::LEFT_CENSORED) == "LEFT-CENSORED");
    REQUIRE(to_string(ThresholdState::RIGHT_CENSORED) == "RIGHT-CENSORED");
    REQUIRE(threshold_state_from_string("IDENTIFIED") == ThresholdState::IDENTIFIED);
    REQUIRE(threshold_state_from_string("STATISTICALLY_INCONCLUSIVE") == ThresholdState::STATISTICALLY_INCONCLUSIVE);
    REQUIRE(threshold_state_from_string("LEFT-CENSORED") == ThresholdState::LEFT_CENSORED);
    REQUIRE(threshold_state_from_string("RIGHT-CENSORED") == ThresholdState::RIGHT_CENSORED);
    REQUIRE(!threshold_state_from_string("IDENTIFIED_NON_MONOTONE").has_value());
}

// Mock estimator to verify CardinalityEstimator concept compiles cleanly
struct MockEstimator {
    void update([[maybe_unused]] int item) {}
    [[nodiscard]] EstimateResult estimate() const { return EstimateResult::success(1.0); }
    void reset() {}
    [[nodiscard]] std::string_view name() const { return "Mock"; }
};

static_assert(CardinalityEstimator<MockEstimator, int>);

void test_provenance_structure() {
    RunProvenance prov;
    prov.git_commit = "ecdc56f";
    prov.compiler_version = "g++ 15.2.0";
    REQUIRE(prov.source_s1_version == "arXiv:2301.10191v2");
    REQUIRE(prov.source_s2_version == "Stanford-CS-revised-2023-12-29");
    REQUIRE(prov.schema_version == "3.1.2");
}

int main() {
    std::cout << "Running test_case02: Infrastructure and Result Semantics\n";
    test_success_result();
    test_abort_result();
    test_numeric_failure_result();
    test_resource_failure_result();
    test_status_has_value_equivalence();
    test_approximation_validity();
    test_schema_enums_and_strings();
    test_provenance_structure();

    ::cvm::test::report_and_exit();
    return 0;
}
