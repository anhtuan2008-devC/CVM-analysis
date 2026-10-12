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
    int group_id;
    std::string group_name;
    std::uint64_t total_n;
    std::uint64_t distinct_f0;
    std::string dist_type;
    double eps;
    double delta;
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

// Pre-create shared string dictionary
std::vector<std::string> make_dictionary(std::size_t distinct_count) {
    std::vector<std::string> dict(distinct_count + 1);
    for (std::size_t i = 1; i <= distinct_count; ++i) {
        dict[i] = "item_" + std::to_string(i);
    }
    return dict;
}

// Generate data stream into RAM
std::vector<std::string> generate_stream(
    const TestCaseConfig& cfg,
    const std::vector<std::string>& dict,
    const std::vector<double>& zipf_cdf,
    std::uint64_t& out_actual_f0
) {
    std::vector<std::string> stream;
    stream.reserve(cfg.total_n);
    std::unordered_set<std::uint64_t> distinct_set;
    std::mt19937_64 rng(cfg.seed + 54321);

    // Ensure all distinct items appear at least once
    for (std::uint64_t i = 1; i <= cfg.distinct_f0; ++i) {
        stream.push_back(dict[i]);
        distinct_set.insert(i);
    }

    if (cfg.dist_type == "zipf") {
        std::uniform_real_distribution<double> unif_real(0.0, 1.0);
        for (std::uint64_t i = cfg.distinct_f0; i < cfg.total_n; ++i) {
            double r = unif_real(rng);
            auto it = std::lower_bound(zipf_cdf.begin(), zipf_cdf.end(), r);
            std::size_t id = 1 + std::distance(zipf_cdf.begin(), it);
            if (id > cfg.distinct_f0) id = cfg.distinct_f0;
            stream.push_back(dict[id]);
            distinct_set.insert(id);
        }
    } else { // uniform
        std::uniform_int_distribution<std::uint64_t> dist(1, cfg.distinct_f0);
        for (std::uint64_t i = cfg.distinct_f0; i < cfg.total_n; ++i) {
            std::uint64_t id = dist(rng);
            stream.push_back(dict[id]);
            distinct_set.insert(id);
        }
    }

    out_actual_f0 = distinct_set.size();
    return stream;
}

int main() {
    const double theoretical_eps = 0.20;
    const double theoretical_delta = 0.10;
    const std::uint64_t fixed_distinct_f0 = 50000;

    // Group 4: 10 tests with N from 5 million to 10 million items
    std::vector<std::uint64_t> group4_n = {
        5000000, 5500000, 6000000, 6500000, 7000000,
        7500000, 8000000, 8500000, 9000000, 10000000
    };

    std::vector<TestCaseConfig> configs;
    for (int i = 0; i < 10; ++i) {
        std::uint64_t n = group4_n[i];
        std::size_t s = cvm_standalone::CvmA1<std::string>::compute_theoretical_capacity(theoretical_eps, theoretical_delta, n);
        configs.push_back({
            i + 31,
            4,
            "Group 4: 5,000,000 <= N <= 10,000,000 (Ultra-Large Scale)",
            n,
            fixed_distinct_f0,
            (i % 2 == 0) ? "uniform" : "zipf",
            theoretical_eps,
            theoretical_delta,
            s,
            static_cast<std::uint64_t>(400 + i)
        });
    }

    std::cout << "Precomputing dictionary and distributions...\n";
    auto dict = make_dictionary(fixed_distinct_f0);

    const double alpha = 1.05;
    std::vector<double> zipf_cdf;
    zipf_cdf.reserve(fixed_distinct_f0);
    double sum = 0.0;
    for (std::uint64_t i = 1; i <= fixed_distinct_f0; ++i) {
        sum += 1.0 / std::pow(static_cast<double>(i), alpha);
        zipf_cdf.push_back(sum);
    }
    for (double& v : zipf_cdf) v /= sum;

    std::vector<TestResult> results;

    std::cout << "======================================================================\n";
    std::cout << "     RUNNING 10 ULTRA-LARGE TESTCASES (N = 5M - 10M ITEMS)\n";
    std::cout << "  - Theorem 2 Buffer Capacity formula: s = ceil( (12/eps^2) * log2(8N/delta) )\n";
    std::cout << "  - Buffer s ranges: " << configs.front().buffer_s << " -> " << configs.back().buffer_s << "\n";
    std::cout << "======================================================================\n\n";

    for (const auto& cfg : configs) {
        std::cout << "[Test " << cfg.test_id << "/40] Group 4 | N = " << std::setw(8) << cfg.total_n 
                  << " | s = " << std::setw(5) << cfg.buffer_s 
                  << " (" << cfg.dist_type << ")... " << std::flush;

        std::uint64_t actual_f0 = 0;
        auto stream = generate_stream(cfg, dict, zipf_cdf, actual_f0);

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

        std::cout << "DONE! Est: " << std::fixed << std::setprecision(1) << est 
                  << " | Err: " << std::setprecision(2) << err << "%"
                  << " | Time: " << std::setprecision(1) << elapsed_ms << " ms (" 
                  << std::setprecision(2) << throughput << " M/s)\n";
    }

    // Open in APPEND mode to preserve the original 30 test results!
    std::ofstream out_file("benchmark_results_30tests.txt", std::ios::out | std::ios::app);
    if (!out_file.is_open()) {
        std::cerr << "Cannot open benchmark_results_30tests.txt for appending!\n";
        return 1;
    }

    auto write_report = [&](std::ostream& os) {
        os << "\n=========================================================================================================\n";
        os << " GROUP 4: 5,000,000 <= N <= 10,000,000 (ULTRA-LARGE SCALE - EXTENDED)\n";
        os << "=========================================================================================================\n";
        os << "| Test |     N     | Ground Truth | Computed s | Distribution | Estimated F0 | Rel. Error | Time (ms) | Status  |\n";
        os << "|:----:|:---------:|:------------:|:----------:|:------------:|:------------:|:----------:|:---------:|:-------:|\n";

        double sum_err = 0.0;
        double sum_time = 0.0;
        double sum_mops = 0.0;
        int count = 0;

        for (const auto& r : results) {
            os << "|  " << std::setw(2) << r.config.test_id << "  | "
               << std::setw(9) << r.config.total_n << " | "
               << std::setw(12) << r.actual_f0 << " | "
               << std::setw(10) << r.config.buffer_s << " | "
               << std::setw(12) << r.config.dist_type << " | "
               << std::setw(12) << std::fixed << std::setprecision(1) << r.estimated_f0 << " | "
               << std::setw(9) << std::setprecision(2) << (std::to_string(r.relative_error_pct).substr(0, 4) + "%") << " | "
               << std::setw(9) << std::setprecision(1) << r.elapsed_ms << " | "
               << (r.aborted ? " ABORT " : " SUCCESS") << " |\n";
            sum_err += r.relative_error_pct;
            sum_time += r.elapsed_ms;
            sum_mops += r.throughput_mops;
            ++count;
        }

        os << "---------------------------------------------------------------------------------------------------------\n";
        os << " >> GROUP 4 AVERAGES:\n";
        os << "    - Mean Relative Error:   " << std::fixed << std::setprecision(3) << (sum_err / count) << " %\n";
        os << "    - Mean Execution Time:   " << std::setprecision(1) << (sum_time / count) << " ms (" 
           << std::setprecision(2) << (sum_time / count / 1000.0) << " s)\n";
        os << "    - Mean Throughput:       " << std::setprecision(3) << (sum_mops / count) << " million items/sec\n";
        os << "=========================================================================================================\n";
    };

    write_report(out_file);
    out_file.close();

    std::cout << "\n";
    write_report(std::cout);
    std::cout << "\n✅ Successfully APPENDED 10 ultra-large test results into: testcode/benchmark_results_30tests.txt\n";

    return 0;
}
