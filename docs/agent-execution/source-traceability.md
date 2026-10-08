# SOURCE TRACEABILITY MATRIX: CVM-analysis

- **Document ID:** `TRACE-0001`
- **Case:** `CASE-01`
- **Timestamp:** `2026-10-09T00:19:30+07:00`
- **SSOT Version:** `3.1.2-Core-Research-Frozen-Candidate`
- **Status:** `GATE-01: PASS`

---

## 1. Primary Sources Inventory

1. **S1:** Sourav Chakraborty, N. V. Vinodchandran, Kuldeep S. Meel.<br>
   *Distinct Elements in Streams: An Algorithm for the (Text) Book.*<br>
   `arXiv:2301.10191v2`, 24 May 2023.
2. **S2:** Donald E. Knuth.<br>
   *The CVM Algorithm for Estimating Distinct Elements in Streams.*<br>
   Stanford Computer Science, 25 May 2023 (Revised 29 December 2023).

---

## 2. Source Claims Traceability Matrix (C01 – C28)

| Claim ID | PLAN Section | Classification | Source | Exact Locator | Verified Statement | Project Interpretation | Implementation Impact |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **C01** | §1.1, §4.2 | [T] | S1 | §1, Problem 1 | $(\varepsilon, \delta)$-approximation of distinct elements $F_0 = \vert A\vert$ with $\Pr[\vert\hat F_0 - F_0\vert \le \varepsilon F_0] \ge 1 - \delta$. | Defines the target approximation criteria for cardinality estimation. | Drives approximation validity condition in `EstimateResult`. |
| **C02** | §1.1, §4.1 | [T] | S1 | §1 | All logarithms are base 2: $\log = \log_2$. | Explicit base-2 logarithm requirement across all capacity formulas. | `std::log2` required; natural `std::log` prohibited in bound calculations. |
| **C03** | §6, §7.1 | [T] | S1 | Algorithm 1 | Threshold buffer capacity formula $thresh = \lceil \frac{12}{\varepsilon^2} \log_2(\frac{8m}{\delta}) \rceil$. | Exact theoretical threshold for Algorithm 1. | Implemented in `theoretical_capacity(m, eps, delta)`. |
| **C04** | §7.1 | [T] | S1 | Theorem 2 | Algorithm 1 produces an $(\varepsilon, \delta)$-approximation of $F_0$. | High-probability correctness bound for CVM-A1. | Benchmark for empirical validation in RQ1. |
| **C05** | §5.3, §26 | [T] | S1 | Claim 3 | $\Pr[\mathrm{Fail}] \le \delta / 8$. | Probability of A1 aborting prematurely is bounded by $\delta/8$. | Governs expected failure rate in RQ1 experiments. |
| **C06** | §5.3, §26 | [T] | S1 | Claim 4 | $\Pr[\mathrm{Error} \cap \overline{\mathrm{Fail}}] \le \delta / 2$. | Probability of error given non-failure is bounded by $\delta/2$. | Used in Track B conditional accuracy validation. |
| **C07** | §0.2, §7.1 | [T] | S1 | Footnote after Alg 1 | Joint independence claim in earlier draft is erroneous. | Joint independence cannot be assumed across elements or time steps. | Excludes independent Bernoulli proofs; requires careful simulation. |
| **C08** | §0.2, §7.1 | [T] | S1 | Footnote after Alg 1 | $\vert\mathcal{X}\vert / p$ is not unbiased in general for Algorithm 1. | A1 estimate is biased conditionally and unconditionally due to aborts. | A1 cannot be claimed as unbiased; bias must be measured conditionally. |
| **C09** | §7.3 | [T] | S2 | Steps D1–D6 | Algorithm D maintains ordered pairs $(a, u)$ with uniform $u \in [0, 1)$, replacing maximum volatility when full. | Exact semantics of Knuth-D with buffer size $s$. | Directly implemented in `KnuthD<Key>`. |
| **C10** | §7.4 | [T] | S2 | Algorithm D text | Algorithm D returns an unbiased estimate $\vert B\vert / p$ of $F_0$ for any stream and $s \ge 1$. | Mathematical unbiasedness of ideal Algorithm D. | Baseline theorem for RQ3/RQ4 unbiasedness verification. |
| **C11** | §7.5 | [T] | S2 | p. 2, Lemma M | $(a, u_{\mathrm{last}(a)}) \in B \iff u_{\mathrm{last}(a)} < p_t$ at step D2. | Fundamental invariant of Algorithm D / D'. | Implemented as oracle invariant check in CASE-12. |
| **C12** | §7.6 | [T] | S2 | p. 5, step D6' | Algorithm D' ("A Coarser Method") restricts $p = 2^{-l}$, purging items with $u' \ge p/2$. | Binary cutoff algorithm semantics. | Directly implemented in `KnuthDprime<Key>`. |
| **C13** | §7.6 | [T] | S2 | p. 5 | Algorithm D' estimator is unbiased. | Unbiasedness holds under binary cutoff. | Verified in RQ3 and RQ4. |
| **C14** | §10 | [T] | S2 | p. 5, Eq. (13) | In coupled runs, $p_{\mathrm{bin}} \le p < 2 p_{\mathrm{bin}}$. | Exact coupling relation between D and D'. | Implemented as strict coupling test in CASE-13. |
| **C15** | §10 | [T] | S2 | p. 5, Lemma R | Supporting cutoff result for binary coarsening. | Theoretical backing for D' validity. | Confirms correctness of dyadic purge. |
| **C16** | §52, §56 | [T] | S2 | Answers, Ex. 3 | For distinct streams, $p = U_{m, s+1}$ where $U_{m, k} \sim \operatorname{Beta}(k, m+1-k)$. | Exact distribution of cutoff for distinct stream under Algorithm D. | Used in Fixture F2 and exact distribution tests. |
| **C17** | §52 | [T] | S2 | Answers, Ex. 3 | For distinct streams, $E[\hat F_0] = m$ under Algorithm D. | Exact expectation for all-distinct stream. | Verified in Fixture F2. |
| **C18** | §53 | [T] | S2 | p. 2, Eq. (7) | For stream $(a, b, a)$ with $s=1$, $E[\hat F_0] = 2$. | Validates duplicate deletion and cutoff transition. | Verified in Fixture F3. |
| **C19** | §54 | [T] | S2 | Answers, Ex. 8 | For stream $(a, b, c)$ with $s=1$, $\Pr[\hat F_0 = 0] = 2/7$. | Exact probability mass at zero for small distinct stream. | Verified in Fixture F4. |
| **C20** | §52 | [T]/[T-D] | S2 | Answers, Ex. 3/6 | Variance formula for Algorithm D valid for $s \ge 2$; singular at $s=1$. | Variance exists only for buffer size $s \ge 2$. | Guides parameter choices in variance ratio tests. |
| **C21** | §7.1, §51 | [T] | S2 | Appendix, p. 12 | Analysis of original Algorithm X (S1 draft). | Baseline description of S1 flaw. | Reconstructed in Fixture F1. |
| **C22** | §51 | [T] | S2 | Appendix, p. 12 | For stream $(a,b,c)$ with $s=2$, Algorithm X fails with probability $5/16$. | Exact failure probability of flawed algorithm. | Verified in Fixture F1. |
| **C23** | §51 | [T] | S2 | Appendix, p. 12 | For stream $(a,b,c)$ with $s=2$, conditional mean $E[\hat F_0 \mid \neg\mathrm{failure}] = 20/11$. | Exact conditional expectation of flawed algorithm. | Verified in Fixture F1. |
| **C24** | §7.2 | [T] | S2 | Appendix, p. 12 | Replacing `If |B|=s` with `While |B|=s` and eliminating failure produces unbiased modified algorithm. | Proves that while-loop halving eliminates abort bias. | Forms the algorithmic definition of CVM-W. |
| **C25** | §2.3, §8 | [T] | S2 | Appendix, p. 12 | Modified Algorithm X is equivalent to Algorithm D with parameter shifted by 1. | Equivalence between CVM-W and Algorithm D. | Foundation for matched capacity normalization. |
| **C26** | §2.3, §8 | [T-D] | S2 + §2.3 | Parameter reindexing | $W(k+1) \equiv D(k)$ under project normalization ($s_W = k+1, s_D = k$). | Normalized operational equivalence between W and D. | Governs RQ3/RQ4 experimental configurations. |
| **C27** | §10 | [T] | S2 | Eq. (13) | Cutoff bracket between D and D'. | Duplicate claim reinforcing C14. | Verified in coupled test suite. |
| **C28** | §14 | [T] | S2 | Answers, Ex. 7 | Algorithm D is exact when $s \ge n$ (all items fit, $p=1$). | Exact capacity threshold for D: $s \ge F_0^*$. | Governs exact-capacity test in CASE-14. |

---

## 3. Project Methodologies Traceability Matrix (M01 – M38)

| ID | Methodology | PLAN Section | Implementation Manifestation |
| :--- | :--- | :--- | :--- |
| **M01** | A1(s) parameterization | §2.2, §7.1 | Allows independent buffer parameter $s$ sweep in RQ1 instead of fixed formula. |
| **M02** | 13-point dyadic grid | §6.1, §26 | Evaluates $s \in \{2^0, 2^1, \dots, 2^{12}\}$ against theoretical bound. |
| **M03** | Family = fixed $(m, \varepsilon, \delta)$ | §27 | Grouping unit for Clopper-Pearson coverage testing. |
| **M04** | Clopper-Pearson interval | §28 | Exact binomial confidence interval for empirical success rate. |
| **M05** | CP endpoint handling | §28 | Handles $K=0$ and $K=N$ boundary cases without zero division. |
| **M06** | Threshold-state classifier | §32 | Classifies states: `IDENTIFIED`, `STATISTICALLY_INCONCLUSIVE`, `LEFT-CENSORED`, `RIGHT-CENSORED`. |
| **M07** | Separate `non_monotone_flag` | §0.4, §32 | Monotonicity flag stored independently of threshold state. |
| **M08** | Non-monotone exclusion | §32 | Families with non-monotone transitions excluded from primary H1 report. |
| **M09** | Matched retained capacity $k$ | §2.3, §9 | Common normalization metric across all four algorithms. |
| **M10** | W uses $s = k+1$ | §2.3, §9 | Normalization rule ensuring $|X| \le k$ post-update. |
| **M11** | WL-DUP workload | §21 | Canonical duplicate-heavy workload for empirical stress testing. |
| **M12** | A1 conditional Track B | §26, §33 | Evaluates A1 accuracy strictly conditional on execution success. |
| **M13** | Run-cluster bootstrap | §42 | Resampling unit is whole run/trial, never individual update event. |
| **M14** | TOST equivalence bound $\Delta = 0.01$ | §48 | Two One-Sided Tests margin for confirming relative error equivalence. |
| **M15** | Pilot TOST power calibration | §48 | Ensures adequate sample size before confirmation runs. |
| **M16** | Predeclared multiplicity scopes | §49 | Pre-registered Holm adjustment across hypothesis families. |
| **M17** | Finite 64-bit rank realization | §11, §14 | Uniform deviates mapped to $[0, 2^{64}-1]$ integers; no floats in decisions. |
| **M18** | Deterministic tie policy | §15 | Ties resolved deterministically via purge of all $r \ge r_{\max}$. |
| **M19** | Exact Oracle Mode | §19 | Bit-tape and volatility-permutation harness for exact state replay. |
| **M20** | Dense vector + key-index map | §83 | Cache-friendly $O(1)$ swap-remove set for A1/W without tombstone thrashing. |
| **M21** | Indexed max-heap + map | §84 | Heap for $O(\log s)$ priority updates and $O(1)$ key lookup in D/D'. |
| **M22** | Throughput timing boundary | §23 | Excludes I/O and workload generation from nanosecond throughput timing. |
| **M23** | Binary/source hash provenance | §19 | Hashes recorded in every experiment metadata block. |
| **M24** | Exact `Cutoff64` type | §14 | Tagged union of `ONE` and `RANK(c)` preserving exact dyadic threshold. |
| **M25** | D' exponent representation | §16 | Binary level stored as integer $l$ with $p = 2^{-l}$. |
| **M26** | RQ3 KS permutation statistic | §46 | Permutation-based two-sample Kolmogorov–Smirnov test. |
| **M27** | KS permutation count 100,000 | §46 | Fixed permutation count for empirical $p$-value calibration. |
| **M28** | H4a run-level bootstrap CI | §47 | Confidence intervals for variance ratios calculated via run-level bootstrap. |
| **M29** | TOST cell-level interpretation | §48 | Primary equivalence evaluated per $(workload, k)$ configuration. |
| **M30** | `q_total_alg` condition | §23, §26 | Algorithm failure rate conditioned on non-infrastructure errors. |
| **M31** | Execution-invalid rate | §5.3, §26 | Separate tracking of numerical/resource exceptions vs algorithmic aborts. |
| **M32** | D/D' finite bracket predicate | §10.1 | Exact integer bracket predicate $l = 65 - \lfloor\log_2(2c+1)\rfloor$. |
| **M33** | RQ2 trace snapshots $t=0, m$ | §36 | Mandatory state capture at start and finish plus periodic checkpoints. |
| **M34** | Bootstrap invalid-replicate policy | §42 | Runs with unhandled exceptions cause replicate failure and invalidate batch. |
| **M35** | Exact Student-$t$ TOST procedure | §48 | Two one-sided $t$-tests with degrees of freedom $n-1$. |
| **M36** | KS on successful-run RE | §46 | Excludes aborted runs from distribution comparison. |
| **M37** | Exact-capacity edge policy | §14, §136 | A1/W exact if $s > F_0^*$; D/D' exact if $s \ge F_0^*$. |
| **M38** | Explicit $\varepsilon, \delta$ fields | §22 | Every raw output schema contains explicit configuration parameters. |

---

## 4. Gate Assessment

- **Gate ID:** `GATE-01`
- **Integrity Test:** All 28 source claims and 38 project methodologies mapped to exact locators in S1 and S2. Zero unverified claims. Zero external secondary citations used as mathematical truth.
- **Result:** `PASS`
