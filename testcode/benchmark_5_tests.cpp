#include "CVM_a1.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_set>
#include <chrono>
#include <iomanip>
#include <random>
#include <cmath>

struct TestCaseConfig {
    int test_id;
    std::uint64_t total_n;
    std::uint64_t distinct_f0;
    std::string dist_type;
    std::size_t buffer_s;
    std::uint64_t seed;
};

struct TestResult {
    TestCaseConfig config;
    std::uint64_t actual_f0;
    double estimated_f0;
    double relative_error_pct;
    double ratio_pct;
    double elapsed_ms;
    double throughput_mops;
    std::size_t final_buffer_size;
    std::uint32_t halving_rounds;
    std::uint64_t dup_removals;
    bool aborted;
};

// Generate data stream into RAM
std::vector<std::string> generate_stream(const TestCaseConfig& cfg, std::uint64_t& out_actual_f0) {
    std::vector<std::string> stream;
    stream.reserve(cfg.total_n);
    std::unordered_set<std::uint64_t> distinct_set;
    std::mt19937_64 rng(cfg.seed + 9999);

    if (cfg.dist_type == "zipf") {
        const double alpha = 1.05;
        std::vector<double> cdf;
        cdf.reserve(cfg.distinct_f0);
        double sum = 0.0;
        for (std::uint64_t i = 1; i <= cfg.distinct_f0; ++i) {
            sum += 1.0 / std::pow(static_cast<double>(i), alpha);
            cdf.push_back(sum);
        }
        for (double& v : cdf) v /= sum;

        std::uniform_real_distribution<double> unif_real(0.0, 1.0);

        for (std::uint64_t i = 1; i <= cfg.distinct_f0; ++i) {
            stream.push_back("item_" + std::to_string(i));
            distinct_set.insert(i);
        }
        for (std::uint64_t i = cfg.distinct_f0; i < cfg.total_n; ++i) {
            double r = unif_real(rng);
            auto it = std::lower_bound(cdf.begin(), cdf.end(), r);
            std::uint64_t id = 1 + std::distance(cdf.begin(), it);
            if (id > cfg.distinct_f0) id = cfg.distinct_f0;
            stream.push_back("item_" + std::to_string(id));
            distinct_set.insert(id);
        }
    } else if (cfg.dist_type == "repeat") {
        std::uint64_t repeats = cfg.total_n / cfg.distinct_f0;
        std::uint64_t rem = cfg.total_n % cfg.distinct_f0;
        for (std::uint64_t r = 0; r < repeats; ++r) {
            for (std::uint64_t i = 1; i <= cfg.distinct_f0; ++i) {
                stream.push_back("item_" + std::to_string(i));
                distinct_set.insert(i);
            }
        }
        for (std::uint64_t i = 1; i <= rem; ++i) {
            stream.push_back("item_" + std::to_string(i));
            distinct_set.insert(i);
        }
    } else { // uniform
        for (std::uint64_t i = 1; i <= cfg.distinct_f0; ++i) {
            stream.push_back("item_" + std::to_string(i));
            distinct_set.insert(i);
        }
        std::uniform_int_distribution<std::uint64_t> dist(1, cfg.distinct_f0);
        for (std::uint64_t i = cfg.distinct_f0; i < cfg.total_n; ++i) {
            std::uint64_t id = dist(rng);
            stream.push_back("item_" + std::to_string(id));
            distinct_set.insert(id);
        }
    }

    out_actual_f0 = distinct_set.size();
    return stream;
}

int main() {
    std::vector<TestCaseConfig> configs = {
        {1,  700000, 45000, "uniform", 3000, 101},
        {2,  800000, 50000, "zipf",    3500, 202},
        {3,  850000, 50000, "repeat",  4000, 303},
        {4,  900000, 55000, "uniform", 4500, 404},
        {5, 1000000, 60000, "zipf",    5000, 505}
    };

    std::vector<TestResult> results;
    std::ofstream out_file("benchmark_results_5tests.txt", std::ios::out | std::ios::trunc);

    if (!out_file.is_open()) {
        std::cerr << "Cannot open benchmark_results_5tests.txt for writing!\n";
        return 1;
    }

    std::cout << "======================================================================\n";
    std::cout << "         AUTOMATED CVM-A1 BENCHMARK SUITE (5 LARGE TESTCASES)\n";
    std::cout << "======================================================================\n\n";

    for (const auto& cfg : configs) {
        std::cout << "--> Running Test #" << cfg.test_id << " (N = " << cfg.total_n 
                  << ", F0 = " << cfg.distinct_f0 << ", Dist = " << cfg.dist_type 
                  << ", Buffer s = " << cfg.buffer_s << ")..." << std::flush;

        std::uint64_t actual_f0 = 0;
        auto stream = generate_stream(cfg, actual_f0);

        cvm_standalone::CvmA1<std::string> cvm(cfg.buffer_s, cfg.seed);

        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& item : stream) {
            cvm.update(item);
        }
        auto end = std::chrono::high_resolution_clock::now();

        double elapsed_ns = std::chrono::duration<double, std::nano>(end - start).count();
        double elapsed_ms = elapsed_ns / 1'000'000.0;
        double elapsed_s = elapsed_ns / 1'000'000'000.0;
        double throughput = (cfg.total_n > 0 && elapsed_s > 0) ? (cfg.total_n / elapsed_s / 1'000'000.0) : 0.0;

        double est = cvm.estimate();
        bool aborted = cvm.is_aborted();
        double ratio = (actual_f0 > 0 && !aborted) ? (est / actual_f0 * 100.0) : 0.0;
        double err = (actual_f0 > 0 && !aborted) ? (std::abs(est - actual_f0) / actual_f0 * 100.0) : 0.0;

        TestResult res{
            cfg,
            actual_f0,
            est,
            err,
            ratio,
            elapsed_ms,
            throughput,
            cvm.buffer_size(),
            cvm.halving_rounds(),
            cvm.dup_removals(),
            aborted
        };
        results.push_back(res);

        std::cout << " DONE! [Est: " << std::fixed << std::setprecision(1) << est 
                  << ", Error: " << std::setprecision(2) << err << "%, Time: " 
                  << std::setprecision(1) << elapsed_ms << " ms]\n";
    }

    // Write full report to benchmark_results_5tests.txt and console
    auto write_report = [&](std::ostream& os) {
        os << "=========================================================================================================\n";
        os << "                                 CVM-A1 BENCHMARK REPORT (5 EXTENDED TESTS)\n";
        os << "=========================================================================================================\n";
        os << "Test Specification:\n";
        os << " - Stream size N:        700,000 to 1,000,000 items\n";
        os << " - Distinct items F0:    45,000 to 60,000 items\n";
        os << " - Buffer capacities s:  3,000 to 5,000 items\n";
        os << " - Distributions:        Uniform, Zipf (skewed), Repeat\n";
        os << "---------------------------------------------------------------------------------------------------------\n\n";

        for (const auto& r : results) {
            os << "---------------------------------------------------------------------------------------------------------\n";
            os << "TEST CASE #" << r.config.test_id << " DETAILS:\n";
            os << "  * Configuration:\n";
            os << "      - Total Elements (N):         " << r.config.total_n << "\n";
            os << "      - Target Distinct (F0):       " << r.config.distinct_f0 << "\n";
            os << "      - Distribution:               " << r.config.dist_type << "\n";
            os << "      - Buffer Capacity (s):        " << r.config.buffer_s << "\n";
            os << "      - Random Seed:                " << r.config.seed << "\n";
            os << "  * Accuracy & Estimation:\n";
            os << "      - Ground Truth Distinct (F0): " << r.actual_f0 << "\n";
            os << "      - CVM-A1 Estimated (F0_hat):  " << std::fixed << std::setprecision(2) << r.estimated_f0 << "\n";
            os << "      - Accuracy Ratio:             " << std::setprecision(3) << r.ratio_pct << " %\n";
            os << "      - Relative Error:             " << std::setprecision(3) << r.relative_error_pct << " %\n";
            os << "  * Performance:\n";
            os << "      - Elapsed Time:               " << std::setprecision(2) << r.elapsed_ms << " ms (" 
               << (r.elapsed_ms / 1000.0) << " s)\n";
            os << "      - Throughput:                 " << std::setprecision(3) << r.throughput_mops << " million items/sec\n";
            os << "  * Internal Metrics:\n";
            os << "      - Final Buffer Size (|X|):    " << r.final_buffer_size << " / " << r.config.buffer_s << "\n";
            os << "      - Halving Rounds (l):         " << r.halving_rounds << " (Sampling probability p = 2^-" << r.halving_rounds << ")\n";
            os << "      - Duplicate Removals:         " << r.dup_removals << "\n";
            os << "      - Status:                     " << (r.aborted ? "ABORT" : "SUCCESS") << "\n\n";
        }

        os << "=========================================================================================================\n";
        os << "                                         SUMMARY COMPARISON TABLE\n";
        os << "=========================================================================================================\n";
        os << "| Test |     N     | Ground Truth | Buffer s | Distribution | Estimated F0 | Rel. Error | Time (ms) | Status  |\n";
        os << "|:----:|:---------:|:------------:|:--------:|:------------:|:------------:|:----------:|:---------:|:-------:|\n";
        for (const auto& r : results) {
            os << "|   " << r.config.test_id << "  | "
               << std::setw(9) << r.config.total_n << " | "
               << std::setw(12) << r.actual_f0 << " | "
               << std::setw(8) << r.config.buffer_s << " | "
               << std::setw(12) << r.config.dist_type << " | "
               << std::setw(12) << std::fixed << std::setprecision(1) << r.estimated_f0 << " | "
               << std::setw(9) << std::setprecision(2) << (std::to_string(r.relative_error_pct).substr(0, 4) + "%") << " | "
               << std::setw(9) << std::setprecision(1) << r.elapsed_ms << " | "
               << (r.aborted ? " ABORT " : " SUCCESS") << " |\n";
        }
        os << "=========================================================================================================\n";
    };

    write_report(out_file);
    out_file.close();

    std::cout << "\n";
    write_report(std::cout);
    std::cout << "\n✅ Results successfully written to file: testcode/benchmark_results_5tests.txt\n";

    return 0;
}
