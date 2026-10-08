#pragma once

#include <iostream>
#include <string_view>
#include <cstdlib>

namespace cvm::test {

inline int total_assertions = 0;
inline int failed_assertions = 0;

inline void check(bool condition, std::string_view expr, std::string_view file, int line) {
    ++total_assertions;
    if (!condition) {
        ++failed_assertions;
        std::cerr << "[FAIL] Assertion failed: (" << expr << ") at " << file << ":" << line << "\n";
    }
}

#define REQUIRE(cond) ::cvm::test::check((cond), #cond, __FILE__, __LINE__)

inline void report_and_exit() {
    std::cout << "Assertions run: " << total_assertions
              << ", Failures: " << failed_assertions << "\n";
    if (failed_assertions > 0) {
        std::cerr << "TEST SUITE FAILED\n";
        std::exit(1);
    }
    std::cout << "ALL TESTS PASSED\n";
}

} // namespace cvm::test
