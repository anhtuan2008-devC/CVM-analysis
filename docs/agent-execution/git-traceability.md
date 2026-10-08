# GIT TRACEABILITY: CVM-analysis

This document maps PLAN sections, cases, decisions, sources, and tests to Git commits according to AGENTS.md Section 42.

| Case ID | Change ID | Decision ID | Base Commit | Impl Commit | Files Changed | Tests Passed | Gate Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| CASE-00 | CHG-0001 | DEC-0001 | none (initial) | ecdc56f | docs/agent-execution/* | TEST-AUDIT-INTEGRITY | PASS |
| CASE-01 | CHG-0002 | DEC-0002 | none (initial) | ecdc56f | docs/agent-execution/source-traceability.md | TEST-TRACEABILITY-VERIFICATION | PASS |
| CASE-02 | CHG-0003 | DEC-0003 | ecdc56f | ebeae2e | include/cvm/core/*, tests/*, CMakeLists.txt | TEST-CASE02-SEMANTICS | PASS |
| CASE-03 | CHG-0004 | DEC-0004 | ebeae2e | 61093ee | include/cvm/core/rng.hpp, tests/unit/test_case03.cpp | TEST-CASE03-RNG | PASS |
| CASE-04 | CHG-0005 | DEC-0005 | 61093ee | 1aa7e09 | include/cvm/core/rank.hpp, include/cvm/ds/cutoff64.hpp, tests/unit/test_case04.cpp | TEST-CASE04-RANK | PASS |
| CASE-05 | CHG-0006 | DEC-0006 | 1aa7e09 | 1770233 | include/cvm/core/cvm_a1.hpp, tests/unit/test_case05.cpp | TEST-CASE05-A1 | PASS |
| CASE-06 | CHG-0007 | DEC-0007 | 1770233 | da371dd | include/cvm/core/cvm_w.hpp, tests/unit/test_case06.cpp, CMakeLists.txt | TEST-CASE06-W | PASS |
| CASE-07 | CHG-0008 | DEC-0008 | da371dd | 80bae88 | include/cvm/ds/key_index_map.hpp, include/cvm/ds/dense_key_set.hpp, tests/unit/test_case07.cpp, CMakeLists.txt | TEST-CASE07-DATASTRUCTURES | PASS |
| CASE-08 | CHG-0009 | DEC-0009 | 80bae88 | 5445ef0 | include/cvm/ds/indexed_heap.hpp, include/cvm/core/knuth_d.hpp, tests/unit/test_case08.cpp, CMakeLists.txt | TEST-CASE08-KNUTHD | PASS |
| CASE-09 | CHG-0010 | DEC-0010 | 5445ef0 | 770ccaf | include/cvm/core/knuth_dprime.hpp, tests/unit/test_case09.cpp, CMakeLists.txt | TEST-CASE09-DPRIME | PASS |
