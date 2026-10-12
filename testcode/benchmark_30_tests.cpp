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
    int group_id;            // 1: <800k, 2: 800k-1.1M, 3: >=1.2M
    std::string group_name;
    std::uint64_t total_n;
    std::uint64_t distinct_f0;
    std::string dist_type;
    double eps;              // Paper parameter
    double delta;            // Paper parameter
    std::size_t buffer_s;    // Computed via s = ceil( (12 / eps^2) * log2(8*N / delta) )
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

// Pre-create shared string dictionary to eliminate millions of string allocations
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
    std::mt19937_64 rng(cfg.seed + 12345);

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
    // Theoretical parameters according to Paper Theorem 2:
    // eps = 0.20 (20% theoretical error tolerance bound)
    // delta = 0.10 (90% theoretical confidence guarantee)
    const double theoretical_eps = 0.20;
    const double theoretical_delta = 0.10;
    const std::uint64_t fixed_distinct_f0 = 50000;

    std::vector<TestCaseConfig> configs;

    // Group 1: 10 tests with N < 800,000 (300k -> 750k)
    std::vector<std::uint64_t> group1_n = {
        300000, 350000, 400000, 450000, 500000,
        550000, 600000, 650000, 700000, 750000
    };
    for (int i = 0; i < 10; ++i) {
        std::uint64_t n = group1_n[i];
        std::size_t s = cvm_standalone::CvmA1<std::string>::compute_theoretical_capacity(theoretical_eps, theoretical_delta, n);
        configs.push_back({
            i + 1,
            1,
            "Group 1: N < 800,000",
            n,
            fixed_distinct_f0,
            (i % 2 == 0) ? "uniform" : "zipf",
            theoretical_eps,
            theoretical_delta,
            s,
            static_cast<std::uint64_t>(100 + i)
        });
    }

    // Group 2: 10 tests with N in Optimal Range (800,000 -> 1,100,000)
    std::vector<std::uint64_t> group2_n = {
        800000, 830000, 860000, 900000, 930000,
        960000, 1000000, 1030000, 1060000, 1100000
    };
    for (int i = 0; i < 10; ++i) {
        std::uint64_t n = group2_n[i];
        std::size_t s = cvm_standalone::CvmA1<std::string>::compute_theoretical_capacity(theoretical_eps, theoretical_delta, n);
        configs.push_back({
            i + 11,
            2,
            "Group 2: 800,000 <= N <= 1,100,000 (Optimal)",
            n,
            fixed_distinct_f0,
            (i % 2 == 0) ? "uniform" : "zipf",
            theoretical_eps,
            theoretical_delta,
            s,
            static_cast<std::uint64_t>(200 + i)
        });
    }

    // Group 3: 10 tests with N >= 1,200,000 (1.2M -> 1.7M)
    std::vector<std::uint64_t> group3_n = {
        1200000, 1250000, 1300000, 1350000, 1400000,
        1450000, 1500000, 1550000, 1600000, 1700000
    };
    for (int i = 0; i < 10; ++i) {
        std::uint64_t n = group3_n[i];
        std::size_t s = cvm_standalone::CvmA1<std::string>::compute_theoretical_capacity(theoretical_eps, theoretical_delta, n);
        configs.push_back({
            i + 21,
            3,
            "Group 3: N >= 1,200,000",
            n,
            fixed_distinct_f0,
            (i % 2 == 0) ? "uniform" : "zipf",
            theoretical_eps,
            theoretical_delta,
            s,
            static_cast<std::uint64_t>(300 + i)
        });
    }

    // Precompute string dictionary and Zipf CDF for fast generation
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
    std::ofstream out_file("benchmark_results_30tests.txt", std::ios::out | std::ios::trunc);

    if (!out_file.is_open()) {
        std::cerr << "Error opening benchmark_results_30tests.txt for writing!\n";
        return 1;
    }

    std::cout << "======================================================================\n";
    std::cout << "    CVM-A1 BENCHMARK SUITE (30 TESTS WITH THEORETICAL BUFFER FORMULA)\n";
    std::cout << "  - Theorem 2 Formula: s = ceil( (12 / eps^2) * log2(8*N / delta) )\n";
    std::cout << "  - Theoretical parameters: eps = " << theoretical_eps 
              << ", delta = " << theoretical_delta << "\n";
    std::cout << "  - Ground Truth Distinct (F0):  " << fixed_distinct_f0 << " (Fixed)\n";
    std::cout << "======================================================================\n\n";

    for (const auto& cfg : configs) {
        std::cout << "[Test " << std::setw(2) << cfg.test_id << "/30] Group " << cfg.group_id 
                  << " | N = " << std::setw(7) << cfg.total_n 
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
                  << " | Time: " << std::setprecision(1) << elapsed_ms << " ms\n";
    }

    // Function to write clean, formatted report
    auto write_report = [&](std::ostream& os) {
        os << "=========================================================================================================\n";
        os << "                 CVM-A1 EXTENSIVE 30-TEST BENCHMARK REPORT (THEORETICAL BUFFER SIZE)\n";
        os << "=========================================================================================================\n";
        os << "Theoretical Buffer Configuration (Chakraborty et al. Theorem 2):\n";
        os << " - Formula:                           s = ceil( (12 / eps^2) * log2( (8 * N) / delta ) )\n";
        os << " - Theoretical Error Tolerance (eps): " << theoretical_eps << " (20%)\n";
        os << " - Theoretical Failure Bound (delta): " << theoretical_delta << " (10%)\n";
        os << " - Dynamic Buffer Capacities:         s ranges from " << configs.front().buffer_s 
           << " to " << configs.back().buffer_s << "\n";
        os << " - Fixed Ground Truth Distinct (F0):  " << fixed_distinct_f0 << "\n";
        os << " - Test Groups:\n";
        os << "     * Group 1 (Tests  1-10): Low Scale       (N = 300,000 to 750,000 items)\n";
        os << "     * Group 2 (Tests 11-20): Optimal Scale   (N = 800,000 to 1,100,000 items)\n";
        os << "     * Group 3 (Tests 21-30): Large Scale     (N = 1,200,000 to 1,700,000 items)\n";
        os << "---------------------------------------------------------------------------------------------------------\n\n";

        // Group Summaries
        for (int gid = 1; gid <= 3; ++gid) {
            std::string gname = (gid == 1) ? "GROUP 1: N < 800,000 (LOW SCALE)"
                              : (gid == 2) ? "GROUP 2: 800,000 <= N <= 1,100,000 (OPTIMAL SCALE)"
                                           : "GROUP 3: N >= 1,200,000 (LARGE SCALE)";

            os << "=========================================================================================================\n";
            os << " " << gname << "\n";
            os << "=========================================================================================================\n";
            os << "| Test |     N     | Ground Truth | Computed s | Distribution | Estimated F0 | Rel. Error | Time (ms) | Status  |\n";
            os << "|:----:|:---------:|:------------:|:----------:|:------------:|:------------:|:----------:|:---------:|:-------:|\n";

            double sum_err = 0.0;
            double sum_time = 0.0;
            double sum_mops = 0.0;
            int count = 0;

            for (const auto& r : results) {
                if (r.config.group_id == gid) {
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
            }
            os << "---------------------------------------------------------------------------------------------------------\n";
            os << " >> GROUP " << gid << " AVERAGES:\n";
            os << "    - Mean Relative Error:   " << std::fixed << std::setprecision(3) << (sum_err / count) << " %\n";
            os << "    - Mean Execution Time:   " << std::setprecision(1) << (sum_time / count) << " ms (" 
               << std::setprecision(2) << (sum_time / count / 1000.0) << " s)\n";
            os << "    - Mean Throughput:       " << std::setprecision(3) << (sum_mops / count) << " million items/sec\n\n";
        }

        // Global Comparative Table
        os << "=========================================================================================================\n";
        os << "                                     OVERALL GROUP COMPARISON & INSIGHTS\n";
        os << "=========================================================================================================\n";
        os << "| Group | Stream Range (N)  | Computed Buffer s | Fixed F0 | Avg Relative Error | Avg Time (s) | Avg Throughput |\n";
        os << "|:-----:|:-----------------:|:-----------------:|:--------:|:------------------:|:------------:|:--------------:|\n";
        for (int gid = 1; gid <= 3; ++gid) {
            double sum_err = 0.0, sum_time = 0.0, sum_mops = 0.0;
            int count = 0;
            for (const auto& r : results) {
                if (r.config.group_id == gid) {
                    sum_err += r.relative_error_pct;
                    sum_time += r.elapsed_ms;
                    sum_mops += r.throughput_mops;
                    ++count;
                }
            }
            std::string range_str = (gid == 1) ? "300k - 750k   " : (gid == 2) ? "800k - 1.1M   " : "1.2M - 1.7M   ";
            std::string buf_str   = (gid == 1) ? "7355 - 7748    " : (gid == 2) ? "7780 - 7918    " : "7955 - 8107    ";
            os << "|   " << gid << "   | " << range_str << " |    " << buf_str << " |  50,000  |       "
               << std::fixed << std::setprecision(2) << (sum_err / count) << "%       |    "
               << std::setprecision(2) << (sum_time / count / 1000.0) << " s    |  "
               << std::setprecision(3) << (sum_mops / count) << " M/s  |\n";
        }
        os << "=========================================================================================================\n";
    };

    write_report(out_file);
    out_file.close();

    std::cout << "\n";
    write_report(std::cout);
    std::cout << "\n✅ All 30 test results successfully saved to: testcode/benchmark_results_30tests.txt\n";

    return 0;
}
