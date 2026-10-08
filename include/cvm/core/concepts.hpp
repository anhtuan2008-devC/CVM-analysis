#pragma once

#include "types.hpp"
#include <concepts>
#include <string_view>

namespace cvm::core {

/**
 * @brief Concept defining the required interface for all CVM cardinality estimators
 * according to PLAN Section 82.
 */
template <typename E, typename T>
concept CardinalityEstimator =
requires(E e, const E ce, const T& item) {
    { e.update(item) } -> std::same_as<void>;
    { ce.estimate() } -> std::same_as<EstimateResult>;
    { e.reset() } -> std::same_as<void>;
    { ce.name() } -> std::convertible_to<std::string_view>;
};

} // namespace cvm::core
