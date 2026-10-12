#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <unordered_set>
#include <chrono>
#include <iomanip>
#include <cmath>

/**
 * @file generate_testcase.cpp
 * @brief C++ utility to generate data streams for benchmarking CVM-A1.
 *
 * Usage:
 *   ./generate_testcase [output_file] [total_N] [distinct_F0] [distribution]
 *
 * Defaults:
 *   output_file: testcase.txt
 *   total_N: 1000000 (1 million elements)
 *   distinct_F0: 50000 (50k distinct elements)
 *   distribution: uniform (options: uniform, zipf, repeat)
 */

void print_usage(const char* prog_name) {
    std::cout << "Usage:\n"
              << "  " << prog_name << " [output_file] [total_N] [distinct_F0] [distribution: uniform|zipf|repeat]\n\n"
              << "Examples:\n"
              << "  " << prog_name << " testcase.txt 1000000 50000 uniform\n"
              << "  " << prog_name << " testcase.txt 2000000 100000 zipf\n";
}

int main(int argc, char* argv[]) {
    std::string output_file = "testcase.txt";
    std::uint64_t total_n = 1000000;
    std::uint64_t distinct_f0 = 50000;
    std::string dist_type = "uniform";

    if (argc > 1) output_file = argv[1];
    if (argc > 2) total_n = std::stoull(argv[2]);
    if (argc > 3) distinct_f0 = std::stoull(argv[3]);
    if (argc > 4) dist_type = argv[4];

    if (distinct_f0 > total_n) {
        std::cerr << "Error: Target distinct count F0 (" << distinct_f0 
                  << ") cannot exceed total stream length N (" << total_n << ")!\n";
        return 1;
    }

    std::cout << "========================================================\n";
    std::cout << "                  TESTCASE GENERATOR\n";
    std::cout << "========================================================\n";
    std::cout << " - Output file:            " << output_file << "\n";
    std::cout << " - Total stream items (N): " << total_n << "\n";
    std::cout << " - Target distinct (F0):   " << distinct_f0 << "\n";
    std::cout << " - Distribution type:      " << dist_type << "\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << "Generating stream data, please wait...\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    std::ofstream out(output_file, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "Error: Unable to open file '" << output_file << "' for writing!\n";
        return 1;
    }

    // Fast I/O
    std::ios_base::sync_with_stdio(false);
    out.tie(nullptr);
    std::vector<char> buffer(1024 * 1024); // 1MB buffer
    out.rdbuf()->pubsetbuf(buffer.data(), buffer.size());

    std::mt19937_64 rng(std::random_device{}());
    std::unordered_set<std::uint64_t> actual_distinct_set;

    if (dist_type == "zipf") {
        // Approximate Zipfian distribution (Skewed / Heavy-tailed)
        const double alpha = 1.05;
        std::vector<double> cdf;
        cdf.reserve(distinct_f0);
        double sum = 0.0;
        for (std::uint64_t i = 1; i <= distinct_f0; ++i) {
            sum += 1.0 / std::pow(static_cast<double>(i), alpha);
            cdf.push_back(sum);
        }
        for (double& v : cdf) {
            v /= sum;
        }

        std::uniform_real_distribution<double> unif_real(0.0, 1.0);

        // Ensure every distinct element appears at least once
        for (std::uint64_t i = 1; i <= distinct_f0; ++i) {
            out << "item_" << i << "\n";
            actual_distinct_set.insert(i);
        }

        // Generate the remaining items using Zipf distribution
        for (std::uint64_t i = distinct_f0; i < total_n; ++i) {
            double r = unif_real(rng);
            auto it = std::lower_bound(cdf.begin(), cdf.end(), r);
            std::uint64_t item_id = 1 + std::distance(cdf.begin(), it);
            if (item_id > distinct_f0) item_id = distinct_f0;
            out << "item_" << item_id << "\n";
            actual_distinct_set.insert(item_id);
        }
    } else if (dist_type == "repeat") {
        // Uniform repetitions
        std::uint64_t repeats = total_n / distinct_f0;
        std::uint64_t remainder = total_n % distinct_f0;

        for (std::uint64_t rep = 0; rep < repeats; ++rep) {
            for (std::uint64_t i = 1; i <= distinct_f0; ++i) {
                out << "item_" << i << "\n";
                actual_distinct_set.insert(i);
            }
        }
        for (std::uint64_t i = 1; i <= remainder; ++i) {
            out << "item_" << i << "\n";
            actual_distinct_set.insert(i);
        }
    } else {
        // Default: Uniform Distribution
        for (std::uint64_t i = 1; i <= distinct_f0; ++i) {
            out << "item_" << i << "\n";
            actual_distinct_set.insert(i);
        }

        std::uniform_int_distribution<std::uint64_t> unif_dist(1, distinct_f0);
        for (std::uint64_t i = distinct_f0; i < total_n; ++i) {
            std::uint64_t item_id = unif_dist(rng);
            out << "item_" << item_id << "\n";
            actual_distinct_set.insert(item_id);
        }
    }

    out.flush();
    out.close();

    auto end_time = std::chrono::high_resolution_clock::now();
    double elapsed_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    std::cout << "\n✅ TESTCASE GENERATION SUCCESSFUL!\n";
    std::cout << " - Total lines generated (N): " << total_n << "\n";
    std::cout << " - Actual distinct count (F0): " << actual_distinct_set.size() << "\n";
    std::cout << " - Elapsed generation time:   " << std::fixed << std::setprecision(2) 
              << elapsed_ms << " ms (" << elapsed_ms / 1000.0 << " s)\n";
    std::cout << " - Output file:               " << output_file << "\n";
    std::cout << "========================================================\n";

    return 0;
}
