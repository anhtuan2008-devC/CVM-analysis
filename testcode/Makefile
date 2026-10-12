CXX = g++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -Wpedantic

TARGETS = generate_testcase run_testcase benchmark_5_tests benchmark_30_tests benchmark_extra_10_tests

all: $(TARGETS)

generate_testcase: generate_testcase.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

run_testcase: run_testcase.cpp CVM_a1.hpp
	$(CXX) $(CXXFLAGS) -o $@ run_testcase.cpp

benchmark_5_tests: benchmark_5_tests.cpp CVM_a1.hpp
	$(CXX) $(CXXFLAGS) -o $@ benchmark_5_tests.cpp

benchmark_30_tests: benchmark_30_tests.cpp CVM_a1.hpp
	$(CXX) $(CXXFLAGS) -o $@ benchmark_30_tests.cpp

benchmark_extra_10_tests: benchmark_extra_10_tests.cpp CVM_a1.hpp
	$(CXX) $(CXXFLAGS) -o $@ benchmark_extra_10_tests.cpp

clean:
	rm -f $(TARGETS) testcase.txt my_test.txt benchmark_results_5tests.txt benchmark_results_30tests.txt

.PHONY: all clean
