# TEST EVIDENCE: CVM-analysis

This document records test evidence for every case according to AGENTS.md Section 40.

| Test ID | Case ID | Test Type | Target / Command | Expectation | Result | Commit SHA |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| TEST-AUDIT-INTEGRITY | CASE-00 | Audit | Inspection of repository files against PLAN | 100% components accounted for | PASS | ecdc56f |
| TEST-TRACEABILITY-VERIFICATION | CASE-01 | Audit | Verification of C01-C28 and M01-M38 against S1/S2 | All locators verified, 0 blocked | PASS | ecdc56f |
| TEST-CASE02-SEMANTICS | CASE-02 | Unit / Invariant | ctest --test-dir build / ./build/test_case02 + ASan/UBSan | 72 assertions, contract violation throw, 0 leaks | PASS | ebeae2e |
| TEST-CASE03-RNG | CASE-03 | Unit / Invariant | ctest --test-dir build / ./build/test_case03 + ASan/UBSan | 427 assertions, determinism replay, bit accounting, 0 leaks | PASS | 61093ee |
| TEST-CASE04-RANK | CASE-04 | Unit / Invariant | ctest --test-dir build / ./build/test_case04 + ASan/UBSan | 402 assertions, pure integer cutoff, level semantics, 0 leaks | PASS | 05100c8 |
