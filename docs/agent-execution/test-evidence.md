# TEST EVIDENCE: CVM-analysis

This document records test evidence for every case according to AGENTS.md Section 40.

| Test ID | Case ID | Test Type | Target / Command | Expectation | Result | Commit SHA |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| TEST-AUDIT-INTEGRITY | CASE-00 | Audit | Inspection of repository files against PLAN | 100% components accounted for | PASS | ecdc56f |
| TEST-TRACEABILITY-VERIFICATION | CASE-01 | Audit | Verification of C01-C28 and M01-M38 against S1/S2 | All locators verified, 0 blocked | PASS | ecdc56f |
| TEST-CASE02-SEMANTICS | CASE-02 | Unit / Invariant | ctest --test-dir build / ./build/test_case02 + ASan/UBSan | 72 assertions, contract violation throw, 0 leaks | PASS | ebeae2e |
| TEST-CASE03-RNG | CASE-03 | Unit / Invariant | ctest --test-dir build / ./build/test_case03 + ASan/UBSan | 427 assertions, determinism replay, bit accounting, 0 leaks | PASS | 61093ee |
| TEST-CASE04-RANK | CASE-04 | Unit / Invariant | ctest --test-dir build / ./build/test_case04 + ASan/UBSan | 402 assertions, pure integer cutoff, level semantics, 0 leaks | PASS | 1aa7e09 |
| TEST-CASE05-A1 | CASE-05 | Unit / Invariant / Fixture | ctest --test-dir build / ./build/test_case05 + ASan/UBSan | 36 assertions, F1 fixture P(fail)=5/16, E=20/11, 0 leaks | PASS | 1770233 |
| TEST-CASE06-W | CASE-06 | Unit / Invariant / Cascade | ctest --test-dir build / ./build/test_case06 + ASan/UBSan | 1032 assertions, post-update |X|<=s-1, cascade delta_t=0 uncoerced, 0 leaks | PASS | da371dd |
| TEST-CASE07-DATASTRUCTURES | CASE-07 | Unit / Adversarial / Invariant | ctest --test-dir build / ./build/test_case07 + ASan/UBSan | 147 assertions, 8 adversarial tests, zero allocations post-reserve, 0 leaks | PASS | 0b427a3 |
