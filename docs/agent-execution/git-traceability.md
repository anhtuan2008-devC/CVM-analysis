# GIT TRACEABILITY: CVM-analysis

This document maps PLAN sections, cases, decisions, sources, and tests to Git commits according to AGENTS.md Section 42.

| Case ID | Change ID | Decision ID | Base Commit | Impl Commit | Files Changed | Tests Passed | Gate Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| CASE-00 | CHG-0001 | DEC-0001 | none (initial) | ecdc56f | docs/agent-execution/* | TEST-AUDIT-INTEGRITY | PASS |
| CASE-01 | CHG-0002 | DEC-0002 | none (initial) | ecdc56f | docs/agent-execution/source-traceability.md | TEST-TRACEABILITY-VERIFICATION | PASS |
| CASE-02 | CHG-0003 | DEC-0003 | ecdc56f | c7d5c4e | include/cvm/core/*, tests/*, CMakeLists.txt | TEST-CASE02-SEMANTICS | PASS |
