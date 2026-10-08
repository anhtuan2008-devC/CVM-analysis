# CHANGELOG: CVM-analysis

All notable changes, architectural decisions, and research milestones for `CVM-analysis` will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), adhering strictly to the Master Execution Protocol (`AGENTS.md`) and SSOT (`PLAN.md` v3.1.2).

## [Unreleased] - 2026-10-09

### Added
- **[CASE-00] Full Repository Audit (`AUDIT-0001` / `CHG-0001`):**
  - Documented baseline Git state, repository structure, missing target components, and risk factors in `docs/agent-execution/repository-audit.md`.
  - Initialized execution logging system (`execution-log.md`, `decision-log.md`, `deviation-log.md`, `test-evidence.md`, `git-traceability.md`).
  - Passed `GATE-00`.
- **[CASE-01] Source / PLAN Traceability (`TRACE-0001` / `CHG-0002`):**
  - Mapped all 28 source claims (C01–C28) and 38 project methodologies (M01–M38) to explicit, verifiable locators in S1 (Chakraborty et al. 2023) and S2 (Knuth 2023).
  - Established `docs/agent-execution/source-traceability.md`.
  - Passed `GATE-01`.
- **[CASE-02] Infrastructure and Result Semantics (`CHG-0003` / `DEC-0003`):**
  - Implemented `FailureStatus`, `EstimateResult`, status invariants, execution validity predicates, schema enums (`AlgorithmId`, `WorkloadId`, `ThresholdState`), `RunProvenance`, and `CardinalityEstimator` concept.
  - Added unit test suite `tests/unit/test_case02.cpp` verifying 72 assertions and contract enforcement.
  - Passed `GATE-02`.
- **[CASE-03] RNG / Randomness Layer (`CHG-0004` / `DEC-0004`):**
  - Implemented `SplitMix64`, canonical `xoshiro256** 1.0` (`Xoshiro256StarStar`), `SeedHierarchy` (master/experiment/algorithm/trial derivation), `RngInterface`, and `OracleRng`.
  - Added unit test suite `tests/unit/test_case03.cpp` verifying 427 assertions across seed replay, determinism, bit consumption accounting, and independent instances.
  - Passed `GATE-03`.
- **[CASE-04] Finite Rank and Cutoff64 (`CHG-0005` / `DEC-0005`):**
  - Implemented exact finite representation `Cutoff64` (`ONE` and `RANK(c)`), exact integer predicate $r < c$, Dprime integer level semantics ($l=0 \to$ accept all, $1 \le l \le 64 \to r < 2^{64-l}$, $l \ge 65 \to$ reject all), and exact mathematical coupling level derivation $l = 65 - \lfloor\log_2(2c+1)\rfloor$.
  - Added unit test suite `tests/unit/test_case04.cpp` verifying 402 assertions across boundary conditions and coupling consistency.
  - Passed `GATE-04`.
- **[CASE-05] CVM-A1 Algorithm Implementation (`CHG-0006` / `DEC-0006`):**
  - Implemented canonical CVM Algorithm 1 (`CvmA1<Key>`) per S1 Algorithm 1 semantics (Step A1.1 to A1.5) with halving and abort trigger.
  - Added unit test suite `tests/unit/test_case05.cpp` verifying 36 assertions including capacity boundaries ($s > F_0$ exact guarantee) and Source Fixture F1 ($P(\mathrm{fail}) = 5/16$, $E[\hat F_0 \mid \neg\mathrm{fail}] = 20/11$).
  - Passed `GATE-05`.
- **[CASE-06] CVM-W Algorithm Implementation (`CHG-0007` / `DEC-0007`):**
  - Implemented Knuth's modified Algorithm X (`CvmW<Key>`) with `while |X| == s`, no algorithmic abort, post-update $|X| \le s - 1$ invariant, and detailed `ReductionEvent` cascade tracking.
  - Added unit test suite `tests/unit/test_case06.cpp` verifying 1032 assertions, including multi-halving cascade semantics with uncoerced $\Delta t = 0$ (`delta_t_valid == false`).
  - Passed `GATE-06`.
- **[CASE-07] A1/W Data Structures (`CHG-0008` / `DEC-0008`):**
  - Implemented `KeyIndexMap` (`include/cvm/ds/key_index_map.hpp`) open-addressing table with SplitMix64 mixing, load factor $\le 0.5$, and Knuth Algorithm 6.4R backward-shift deletion (tombstone-free).
  - Implemented `DenseKeySet` (`include/cvm/ds/dense_key_set.hpp`) pairing dense `vector<Key>` with reverse index map for $O(1)$ swap-remove without stale indices.
  - Added unit test suite `tests/unit/test_case07.cpp` verifying 147 assertions across all 8 mandatory adversarial scenarios, zero allocations post-reserve, batch in-place filtering, and clean ASan/UBSan runs.
  - Passed `GATE-07`.
