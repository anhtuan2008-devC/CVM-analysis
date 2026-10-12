#include "CVM_a1.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_set>
#include <chrono>
#include <iomanip>
#include <cmath>

/**
 * @file run_testcase.cpp
 * @brief Benchmark runner for the CVM-A1 algorithm on stream testcases.
 *        Measures elapsed time, calculates estimated vs ground truth F0,
 *        and reports accuracy percentages.
 *
 * Usage:
 *   ./run_testcase [testcase_file] [buffer_capacity_s] [random_seed]
 *
 * Defaults:
 *   testcase_file: testcase.txt
 *   buffer_capacity_s: 5000
 *   random_seed: 42
 */

// ANSI Color codes for clean console output
namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
    const std::string GREEN   = "\033[32m";
    const std::string CYAN    = "\033[36m";
    const std::string YELLOW  = "\033[33m";
    const std::string RED     = "\033[31m";
    const std::string MAGENTA = "\033[35m";
}

int main(int argc, char* argv[]) {
    std::string testcase_file = "testcase.txt";
    std::size_t buffer_size_s = 5000;
    std::uint64_t seed = 42;

    if (argc > 1) testcase_file = argv[1];
    if (argc > 2) buffer_size_s = std::stoull(argv[2]);
    if (argc > 3) seed = std::stoull(argv[3]);

    std::cout << Color::CYAN << Color::BOLD
              << "======================================================================\n"
              << "              CVM-A1 PERFORMANCE BENCHMARK & EVALUATION\n"
              << "======================================================================\n"
              << Color::RESET;
    std::cout << " - Testcase file:           " << testcase_file << "\n";
    std::cout << " - Buffer capacity (s):     " << buffer_size_s << "\n";
    std::cout << " - Random seed:             " << seed << "\n";
    std::cout << "----------------------------------------------------------------------\n";

    // 1. Read testcase stream into memory to isolate algorithm execution from disk I/O
    std::cout << "Loading stream data into RAM...\n";
    std::ifstream in(testcase_file);
    if (!in.is_open()) {
        std::cerr << Color::RED << "Error: Cannot open testcase file '" << testcase_file << "'!\n"
                  << "Please run './generate_testcase' first to generate test data.\n" << Color::RESET;
        return 1;
    }

    std::vector<std::string> stream_data;
    std::unordered_set<std::string> exact_set;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments
        stream_data.push_back(line);
        exact_set.insert(line);
    }
    in.close();

    const std::uint64_t total_n = stream_data.size();
    const std::uint64_t ground_truth_f0 = exact_set.size();

    std::cout << " -> Loaded: " << Color::BOLD << total_n << Color::RESET 
              << " stream items into RAM.\n";
    std::cout << " -> Ground Truth distinct count (F0): " 
              << Color::GREEN << Color::BOLD << ground_truth_f0 << Color::RESET << "\n";
    std::cout << "----------------------------------------------------------------------\n";
    std::cout << "Running CVM-A1 algorithm...\n";

    // 2. Initialize standalone CVM-A1 instance
    cvm_standalone::CvmA1<std::string> cvm(buffer_size_s, seed);

    // 3. Measure CVM-A1 execution time using high-resolution clock
    auto start_time = std::chrono::high_resolution_clock::now();

    for (const auto& item : stream_data) {
        cvm.update(item);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double elapsed_ns = std::chrono::duration<double, std::nano>(end_time - start_time).count();
    double elapsed_ms = elapsed_ns / 1'000'000.0;
    double elapsed_s  = elapsed_ns / 1'000'000'000.0;
    double throughput_mops = (total_n > 0 && elapsed_s > 0) ? (static_cast<double>(total_n) / elapsed_s / 1'000'000.0) : 0.0;

    // 4. Retrieve estimate result
    double estimate_f0 = cvm.estimate();
    bool aborted = cvm.is_aborted();

    // 5. Calculate comparison metrics
    double percentage_ratio = 0.0;
    double relative_error_pct = 0.0;
    if (!aborted && ground_truth_f0 > 0) {
        percentage_ratio = (estimate_f0 / static_cast<double>(ground_truth_f0)) * 100.0;
        relative_error_pct = (std::abs(estimate_f0 - static_cast<double>(ground_truth_f0)) / static_cast<double>(ground_truth_f0)) * 100.0;
    }

    // 6. Print formatted results table
    std::cout << Color::CYAN << Color::BOLD
              << "\n======================================================================\n"
              << "                     BENCHMARK & EVALUATION RESULTS\n"
              << "======================================================================\n"
              << Color::RESET;

    std::cout << std::fixed << std::setprecision(3);

    // Execution time
    std::cout << Color::BOLD << "⏱️  ALGORITHM EXECUTION TIME:\n" << Color::RESET;
    std::cout << "   - Total elapsed time:       " << Color::YELLOW << Color::BOLD
              << elapsed_ms << " ms" << Color::RESET << " (" << elapsed_s << " s, "
              << static_cast<std::uint64_t>(elapsed_ns / 1000.0) << " µs)\n";
    std::cout << "   - Processing throughput:    " << throughput_mops << " million items / sec\n\n";

    // Cardinality comparison
    std::cout << Color::BOLD << "📊 DISTINCT ELEMENT COUNT (CARDINALITY):\n" << Color::RESET;
    std::cout << "   - ACTUAL count (F0 Ground Truth):   " 
              << Color::GREEN << Color::BOLD << ground_truth_f0 << Color::RESET << "\n";
    
    if (aborted) {
        std::cout << "   - ESTIMATED count (F0 CVM-A1):      " 
                  << Color::RED << Color::BOLD << "FAILED (ABORT - Buffer capacity s too small)" << Color::RESET << "\n";
    } else {
        std::cout << "   - ESTIMATED count (F0 CVM-A1):      " 
                  << Color::CYAN << Color::BOLD << estimate_f0 << Color::RESET << "\n";
        
        std::cout << "\n" << Color::BOLD << "📈 ACCURACY & RELATIVE RATIO:\n" << Color::RESET;
        std::cout << "   - Estimate ratio vs Actual:         " 
                  << Color::MAGENTA << Color::BOLD << percentage_ratio << " %" << Color::RESET << "\n";
        std::cout << "   - Relative Error:                   " 
                  << Color::YELLOW << Color::BOLD << relative_error_pct << " %" << Color::RESET << "\n";
    }

    // Internal metrics
    std::cout << "\n" << Color::BOLD << "⚙️  INTERNAL CVM-A1 METRICS:\n" << Color::RESET;
    std::cout << "   - Termination status:               " 
              << (aborted ? (Color::RED + "ABORT") : (Color::GREEN + "SUCCESS")) << Color::RESET << "\n";
    std::cout << "   - Final buffer size (|X|):          " << cvm.buffer_size() << " / " << cvm.capacity() << "\n";
    std::cout << "   - Halving rounds (l):               " << cvm.halving_rounds() << " (Sampling prob p = 2^-" << cvm.halving_rounds() << ")\n";
    std::cout << "   - Halving activations:              " << cvm.reduction_rounds() << " times\n";
    std::cout << "   - Duplicate removals:               " << cvm.dup_removals() << " times\n";

    std::cout << Color::CYAN << Color::BOLD
              << "======================================================================\n"
              << Color::RESET;

    return aborted ? 2 : 0;
}
