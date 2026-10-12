/**
 * @file CVM_a1.cpp
 * @brief Standalone implementation and verification file for the CVM-A1 algorithm
 *        conforming strictly to S1 (Chakraborty et al. 2023).
 */

#include "CVM_a1.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

// Explicit template instantiations for common types
template class cvm_standalone::CvmA1<std::string>;
template class cvm_standalone::CvmA1<std::uint64_t>;

#ifdef CVM_A1_STANDALONE_DEMO
int main() {
    std::cout << "=== STANDALONE CVM-A1 ALGORITHM DEMO ===\n\n";

    const std::size_t buffer_capacity = 100;
    const std::uint64_t seed = 42;
    cvm_standalone::CvmA1<std::string> cvm(buffer_capacity, seed);

    std::cout << "[1] Initialized CVM-A1 with buffer capacity s = " << buffer_capacity << "\n";
    std::cout << "[2] Feeding a stream of 10,000 items (1,000 distinct items)...\n";

    // 1000 distinct items, each repeated 10 times
    for (int repeat = 0; repeat < 10; ++repeat) {
        for (int i = 0; i < 1000; ++i) {
            cvm.update("item_" + std::to_string(i));
        }
    }

    double est = cvm.estimate();

    std::cout << "\n--- QUICK VERIFICATION RESULTS ---\n";
    std::cout << "Status: " << (cvm.is_aborted() ? "ABORT" : "SUCCESS") << "\n";
    std::cout << "Actual distinct count (F0): 1000\n";
    std::cout << "Estimated distinct count (F0_hat): " << std::fixed << std::setprecision(2) << est << "\n";
    std::cout << "Estimate / Actual ratio: " << (est / 1000.0) * 100.0 << "%\n";
    std::cout << "Final buffer size: " << cvm.buffer_size() << " / " << cvm.capacity() << "\n";
    std::cout << "Halving rounds (l): " << cvm.halving_rounds() << "\n";
    std::cout << "Duplicate removals: " << cvm.dup_removals() << "\n";

    return 0;
}
#endif
