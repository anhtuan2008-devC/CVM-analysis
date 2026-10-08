# GIT TRACEABILITY: CVM-analysis

This document maps PLAN sections, cases, decisions, sources, and tests to Git commits according to AGENTS.md Section 42.

| Case ID | Change ID | Decision ID | Base Commit | Impl Commit | Files Changed | Tests Passed | Gate Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| CASE-00 | CHG-0001 | DEC-0001 | none (initial) | ecdc56f | docs/agent-execution/* | TEST-AUDIT-INTEGRITY | PASS |
| CASE-01 | CHG-0002 | DEC-0002 | none (initial) | ecdc56f | docs/agent-execution/source-traceability.md | TEST-TRACEABILITY-VERIFICATION | PASS |
| CASE-02 | CHG-0003 | DEC-0003 | ecdc56f | ebeae2e | include/cvm/core/*, tests/*, CMakeLists.txt | TEST-CASE02-SEMANTICS | PASS |
| CASE-03 | CHG-0004 | DEC-0004 | ebeae2e | 61093ee | include/cvm/core/rng.hpp, tests/unit/test_case03.cpp | TEST-CASE03-RNG | PASS |
| CASE-04 | CHG-0005 | DEC-0005 | 61093ee | 05100c8 | include/cvm/core/rank.hpp, include/cvm/ds/cutoff64.hpp, tests/unit/test_case04.cpp | TEST-CASE04-RANK | PASS |
