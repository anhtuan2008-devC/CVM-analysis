# DECISION LOG: CVM-analysis

This log records every architectural, mathematical, and implementation decision in strict accordance with AGENTS.md Section 7.

---

### Decision ID: DEC-0001
- **Step:** Step 0
- **Case:** CASE-00
- **Change ID:** CHG-0001

#### Problem:
Need to establish a verified baseline of the repository state and trace system before any code modification.

#### Observation:
Repository is at initial commit with only `docs/` (`AGENTS.md`, `CVM-paper.md`, `PLAN.md`). No source, header, test, or build files exist.

#### Plan Requirement:
Section 1, Section 3, Section 16 of `AGENTS.md` mandate a full repository audit prior to any code modification (`GATE-00 = PASS`).

#### Idea:
Perform full scan, record baseline Git status, map existing files to PLAN requirements, and create `docs/agent-execution/repository-audit.md`.

#### Decision:
Document all files in `repository-audit.md` and set up the traceability logging directory structure in `docs/agent-execution/`.

#### Why:
Guarantees clean provenance, complete audit integrity, and strictly follows Rule 0.1 of `AGENTS.md`.

#### Alternative(s):
Start coding directly without an audit. Rejected because it violates Rule 0.1 and Section 4 of `AGENTS.md`.

#### Final Status:
APPROVED / PASS

---

### Decision ID: DEC-0002
- **Step:** Step 1
- **Case:** CASE-01
- **Change ID:** CHG-0002

#### Problem:
Need to verify all mathematical theorems [T], derivations [T-D], and methodologies [M] against primary sources S1 (Chakraborty et al. 2023) and S2 (Knuth 2023) to eliminate unverified or fabricated assertions.

#### Observation:
`PLAN.md` declares 28 source claims (C01–C28) and 38 project methodologies (M01–M38), referencing specific sections of S1 and S2.

#### Plan Requirement:
Section 8, Section 9, Section 10, Section 17 of `AGENTS.md` mandate establishing `docs/agent-execution/source-traceability.md` mapping every claim to an exact source locator before any implementation.

#### Idea:
Cross-reference each claim against the text of S1 (arXiv:2301.10191v2) and S2 (Knuth Stanford Note revised Dec 2023) and record verbatim statements, project interpretations, and implementation impacts.

#### Decision:
Created `source-traceability.md` containing all verified locators for C01–C28 and M01–M38. Confirmed that no secondary or LLM-generated texts are used as mathematical sources.

#### Why:
Prevents semantic drifts, ensures high source fidelity, and fulfills GATE-01 requirements.

#### Alternative(s):
Proceeding to coding without formal source mapping. Rejected because it violates Rule 0.1 and Section 8 of `AGENTS.md`.

#### Final Status:
APPROVED / PASS

---

### Decision ID: DEC-0003
- **Step:** Step 2
- **Case:** CASE-02
- **Change ID:** CHG-0003

#### Problem:
Need to implement foundational result and execution semantics (`FailureStatus`, `EstimateResult`, `is_execution_valid`, `is_approximation_valid`, and `CardinalityEstimator` concept) strictly enforcing invariants and forbidding sentinel failure encodings.

#### Observation:
PLAN Section 5, 81, and 82 define explicit contracts: `has_value() <=> status == Success`, `value()` on failure is a contract violation throwing `logic_error`, CSV failure output is `"null"`, and `NumericFailure`/`ResourceFailure` are never counted as approximation failures.

#### Plan Requirement:
Section 18 of `AGENTS.md` mandates implementing and testing only foundational semantics in CASE-02, barring algorithmic implementations until later cases.

#### Idea:
Create `include/cvm/core/types.hpp`, `include/cvm/core/concepts.hpp`, and standalone test suite in `tests/unit/test_case02.cpp`.

#### Decision:
Implemented `FailureStatus` enum, `EstimateResult` class with private constructor and static factories, status predicates, schema enums (`AlgorithmId`, `WorkloadId`, `ThresholdState`), provenance structure `RunProvenance`, and `CardinalityEstimator` concept.

#### Why:
Guarantees typesafe and mathematically sound error handling across all downstream CVM algorithm implementations.

#### Alternative(s):
Returning `double` with `-1` or `NaN` on failure. Strictly rejected per PLAN Section 5.2.

#### Final Status:
APPROVED / PASS

---

### Decision ID: DEC-0004
- **Step:** Step 3
- **Case:** CASE-03
- **Change ID:** CHG-0004

#### Problem:
Need to provide a high-performance, completely deterministic pseudo-randomness layer supporting both fair-bit extraction (`next_bit()`) for A1/W and 64-bit volatility deviates (`next_uint64()`) for D/D', along with a reproducible seed hierarchy and zero global state.

#### Observation:
PLAN Section 11.2, 19, and 86 require `SplitMix64` for seed mixing, `xoshiro256**` for generation, and an `RngInterface` preventing algorithms from constructing ad-hoc RNGs in hot paths.

#### Plan Requirement:
Section 19 of `AGENTS.md` mandates `SplitMix64`, `xoshiro256**`, master/experiment/algorithm/trial seed hierarchy, and verification of bit consumption accounting, seed replay, and zero global state.

#### Idea:
Implement `SplitMix64`, `Xoshiro256StarStar`, `SeedHierarchy`, and `OracleRng` in `include/cvm/core/rng.hpp`.

#### Decision:
Implemented all required PRNG classes, buffered bit extraction in `next_bit()`, accurate `bits_consumed()` and `uint64_consumed()` accounting, and verified determinism across 427 test assertions.

#### Why:
Guarantees identical replayability, reproducible experiments, and isolates algorithm logic from PRNG details.

#### Alternative(s):
Using `std::mt19937_64` or `rand()`. Rejected due to non-standard bit extraction, slower performance, and violation of PLAN Section 11.2.

#### Final Status:
APPROVED / PASS

---

### Decision ID: DEC-0005
- **Step:** Step 4
- **Case:** CASE-04
- **Change ID:** CHG-0005

#### Problem:
Need an exact finite representation for volatility ranks and cutoff thresholds without using floating-point comparisons in acceptance predicates, and implement exact dyadic integer level semantics for Algorithm D'.

#### Observation:
PLAN Sections 11, 14, 15, and 16 require `Cutoff64` (`ONE` or `RANK(c)`), exact predicate $u_r < p \iff r < c$, integer binary level $l$ for D' ($l=0 \to$ accept all, $1 \le l \le 64 \to r < 2^{64-l}$, $l \ge 65 \to$ reject all), and exact mathematical coupling level derivation $l = 65 - \lfloor\log_2(2c+1)\rfloor$.

#### Plan Requirement:
Section 20 of `AGENTS.md` mandates pure integer acceptance, barring floats in cutoff decisions and testing boundaries ($0, 2^{64}-1, l=0, 1, 63, 64, 65, >65$).

#### Idea:
Create `include/cvm/ds/cutoff64.hpp` and `include/cvm/core/rank.hpp` with exact integer logic, bit manipulation, and comprehensive boundary tests.

#### Decision:
Implemented `Cutoff64`, `dprime_accepts`, `dprime_cutoff_for_level`, and `dprime_expected_level_from_cutoff`. Verified 402 assertions with 0 failures and clean sanitizer runs.

#### Why:
Prevents IEEE 754 precision issues and ensures machine-exact implementation fidelity.

#### Alternative(s):
Using `double` cutoff with `(c + 0.5) / 18446744073709551616.0`. Strictly forbidden per PLAN Section 13.

#### Final Status:
APPROVED / PASS

---

### Decision ID: DEC-0006
- **Step:** Step 5
- **Case:** CASE-05
- **Change ID:** CHG-0006

#### Problem:
Need to implement CVM Algorithm 1 strictly according to verified S1 Algorithm 1 semantics (remove key, sample with $p$, insert, halving when $|X|=s$, and abort if $|X|=s$ after halving) without reordering or shortcutting.

#### Observation:
PLAN Section 7.1 and 51 define the exact sequence of steps A1.1 to A1.5. S2 Appendix provides an exact analytical reference for $s=2$ on 3 distinct elements (Fixture F1: $P(\mathrm{failure}) = 5/16$, $E[\hat F_0 \mid \neg\mathrm{failure}] = 20/11$).

#### Plan Requirement:
Section 21 of `AGENTS.md` mandates strict sequential order, no presumed equivalent algorithms, testing boundary conditions ($s=F_0, s=F_0+1, s>F_0$), and verifying Fixture F1.

#### Idea:
Create `include/cvm/core/cvm_a1.hpp` using swap-remove for deletion, bit extraction via `RngInterface`, and comprehensive unit tests in `tests/unit/test_case05.cpp`.

#### Decision:
Implemented `CvmA1<Key>`, verified empty/single/duplicate/distinct behavior, exact capacity threshold boundary ($s > F_0$ exact vs $s=F_0$ not guaranteed exact), abort on full post-halving, and confirmed Fixture F1.

#### Why:
Preserves 100% source fidelity with S1 Algorithm 1 and provides a verified baseline for CVM-A1.

#### Alternative(s):
Looping halving until buffer size decreases. Rejected because that is CVM-W, not S1 Algorithm 1.

#### Final Status:
APPROVED / PASS

---

### Decision ID: DEC-0007
- **Step:** Step 6
- **Case:** CASE-06
- **Change ID:** CHG-0007

#### Problem:
Need to implement Knuth's modified Algorithm X (CVM-W) from S2 Appendix with `while |X| == s`, no source-level abort, guaranteed $|X| \le s - 1$ post-update invariant, and explicit recording of same-step cascades with $\Delta t = 0$ and $\mathrm{delta\_t\_valid} = \mathrm{false}$ without converting $0$ to $1$.

#### Observation:
PLAN Section 7.2, 8, 37, and 38 mandate loop halving, post-update capacity bounded by $s-1$, $s \ge 2$, and precise logging of each reduction event.

#### Plan Requirement:
Section 22 of `AGENTS.md` mandates testing multi-halving cascades, explicit verification of $\Delta t = 0$, duplicate deletion, and parameter-shift invariant.

#### Idea:
Create `include/cvm/core/cvm_w.hpp` implementing `CvmW<Key>` and `ReductionEvent` tracking, tested in `tests/unit/test_case06.cpp`.

#### Decision:
Implemented `CvmW<Key>`, verified the post-update invariant across 1000 stream items, constructed controlled cascade fixtures confirming $\Delta t = 0$ with `delta_t_valid == false`, and verified 1032 test assertions.

#### Why:
Guarantees source fidelity with S2 Appendix Algorithm X and enables downstream RQ2 dynamics studies.

#### Alternative(s):
Converting $\Delta t = 0$ to $1$ to avoid zero division. Strictly prohibited per PLAN Section 37.

#### Final Status:
APPROVED / PASS

---

### Decision ID: DEC-0008
- **Step:** Step 7
- **Case:** CASE-07
- **Change ID:** CHG-0008

#### Problem:
Need high-performance, cache-friendly data structures for A1/W algorithms satisfying PLAN Section 83 and AGENTS.md Case 07: `dense vector<Key>` coupled with `key -> dense-index map`, $O(1)$ swap-remove, zero stale indices, and zero allocations on steady-state update path after `reserve()`, strictly prohibiting swap-delete inside hash table physical slots.

#### Observation:
Naive hash sets cannot be swap-deleted without destroying probe sequences. A contiguous vector storage `elements_` paired with an open-addressed index map `KeyIndexMap` (using Knuth Algorithm 6.4R backward-shift deletion) achieves tombstone-free $O(1)$ operations with zero general-purpose allocations after initial reservation.

#### Plan Requirement:
PLAN Section 83, 124 and AGENTS.md Case 23 mandate implementing `DenseKeySet` and `KeyIndexMap`, and passing 8 mandatory adversarial tests (remove first, remove middle, remove last, remove same key repeatedly, remove key after swap, insert after many swaps, duplicate after swap, full-capacity transition) and zero memory leaks under sanitizers.

#### Idea:
Implement `KeyIndexMap` in `include/cvm/ds/key_index_map.hpp` with power-of-two table size, load factor $\le 0.5$, SplitMix64 finalizer hashing, and backward-shift deletion. Implement `DenseKeySet` in `include/cvm/ds/dense_key_set.hpp` wrapping `vector<Key>` and `KeyIndexMap<Key, uint32_t>`, and verify all requirements in `tests/unit/test_case07.cpp`.

#### Decision:
Implemented both structures cleanly. Verified all 8 adversarial scenarios, backward-shift deletion with artificial collision clusters, zero vector reallocations post-reserve, batch in-place filtering with index rebuild, and ASan/UBSan clean execution across 147 assertions and thousands of runtime invariant checks.

#### Why:
Provides the canonical, optimal, and mathematically robust storage layer required for scalable A1 and W streaming cardinality estimators.

#### Alternative(s):
Using `std::unordered_map` with node-based allocations or open-addressing with tombstones. Rejected because node maps allocate on each insert, and tombstones degrade probe sequences under continuous streaming updates.

#### Final Status:
APPROVED / PASS

---

### Decision ID: DEC-0009
- **Step:** Step 8
- **Case:** CASE-08
- **Change ID:** CHG-0009

#### Problem:
Need to implement Knuth Algorithm D (`KnuthD<Key>`) strictly according to verified S2 semantics (D1–D6) and PLAN Section 7.3, 14, 15, 84, and 85, using an `IndexedHeap` with reverse index mapping and pure integer `Cutoff64` representation, enforcing strict cutoff invariants, tie event tracking, and purge of multiple equal maxima.

#### Observation:
Algorithm D maintains at most $s$ pairs $(a, r)$ ordered by volatility rank $r$. When buffer is full ($|B|=s$), maximum stored rank $M$ is compared against incoming $r$:
- $r > M$: cutoff becomes $r$, incoming not stored.
- $r < M$: cutoff becomes $M$, stored items with $r \ge M$ purged, incoming inserted.
- $r = M$: finite tie, cutoff becomes $M$, stored items with $r \ge M$ purged, incoming rejected (tie recorded).
Furthermore, $s=1$ on stream $(a, b, a)$ has theoretical mean $E[\hat F_0] = 2.0$, and on $(a, b, c)$ has $E[\hat F_0] = 3.0$ with $p \sim \operatorname{Beta}(2, 2)$.

#### Plan Requirement:
Section 24 of `AGENTS.md` and Section 84, 85, 124 of `PLAN.md` mandate indexed max-heap + reverse index map, preserving source semantics, mandatory invariants ($|B| \le s$, unique keys, heap order, key-index correctness, strict cutoff $r < c$, cutoff monotonicity), and testing all mandatory cases including boundaries and exact-capacity guarantees ($s \ge F_0$).

#### Idea:
Implement `IndexedHeap` in `include/cvm/ds/indexed_heap.hpp` with $O(1)$ key lookup and $O(\log s)$ arbitrary erasure/purge, and `KnuthD` in `include/cvm/core/knuth_d.hpp` with `Cutoff64`, full diagnostics counters, and comprehensive verification in `tests/unit/test_case08.cpp`.

#### Decision:
Implemented `IndexedHeap` and `KnuthD`. Verified all 101 test assertions, confirmed $E[\hat F_0] = 2.0$ on $(a, b, a)$ and $E[\hat F_0] = 3.0$ on $(a, b, c)$ across 50,000 trials each, verified boundary behaviors ($s=1$, $s=F_0$, $s=F_0-1$, cutoff rank 0, ties and equal maxima purge), and verified clean ASan/UBSan execution.

#### Why:
Guarantees mathematical correctness, high execution speed, zero memory leaks, and 100% fidelity to S2 Algorithm D.

#### Alternative(s):
Using treap like Knuth's note or `std::map`. Rejected because indexed binary heap is more cache-friendly and fulfills project methodology [M] with $O(1)$ lookup and $O(\log s)$ updates.

#### Final Status:
APPROVED / PASS

---

### Decision ID: DEC-0010
- **Step:** Step 9
- **Case:** CASE-09
- **Change ID:** CHG-0010

#### Problem:
Need to implement Knuth Algorithm D' ("A Coarser Method", `KnuthDprime<Key>`) strictly according to verified S2 semantics (p. 5, step D6') and PLAN Section 7.6, 16, 17.2, using binary cutoff $p = 2^{-l}$, integer level exponent $l \in [0, 65]$, machine-exact bit shift acceptance predicates, cascade purging, and zero floating-point operations in acceptance.

#### Observation:
Algorithm D' modifies step D6: when $|B|=s$ and incoming rank is accepted under level $l$, the algorithm increments $l \leftarrow l+1$ and purges all stored items with $r_{\text{stored}} \ge 2^{64-l}$. If buffer remains full ($|B|=s$) and incoming item is still accepted, the halving cascade repeats. If $l \ge 65$, finite resolution limit is reached, recording a resolution event. Furthermore, for stream $(a, b, c)$ with $s=1$, S2 Exercise 8 proves $\Pr[\hat F_0 = 0] = 2/7$ and estimates are powers of 2.

#### Plan Requirement:
Section 25 of `AGENTS.md` and Section 7.6, 16 of `PLAN.md` mandate binary cutoff, integer level $l$, pure integer acceptance (`dprime_accepts`), cascade purge, strict prohibition of `double p` or approximate logarithms, and testing $l=0, 1, 63, 64, 65$, cascades, duplicates, ties, multiple maxima, exact capacity, and sanitizers.

#### Idea:
Implement `KnuthDprime<Key>` in `include/cvm/core/knuth_dprime.hpp` using `IndexedHeap`, integer level $l$, pure integer bit shifts `1ULL << (64 - l)`, `dprime_accepts()`, resolution limit tracking, and comprehensive verification in `tests/unit/test_case09.cpp`.

#### Decision:
Implemented `KnuthDprime<Key>`. Verified all 49 assertions, confirmed multiple-level cascade (e.g. $l=0 \to 58$ in one update), resolution event at $l \ge 65$, verified Source Fixture F4 ($\Pr[\hat F_0 = 0] = 2/7$ and powers of 2 estimates over 50,000 trials), and confirmed clean ASan/UBSan runs.

#### Why:
Provides the canonical, machine-exact realization of Knuth's dyadic Algorithm D' without floating-point error drift.

#### Alternative(s):
Using floating-point `pow(2, -l)` or floating-point comparisons. Strictly prohibited per AGENTS.md Section 25.

#### Final Status:
APPROVED / PASS
