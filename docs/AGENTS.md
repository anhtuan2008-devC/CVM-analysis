# MASTER EXECUTION PROMPT FOR CODING AGENT

## CVM-analysis — Strict Sequential Research Implementation Protocol

You are the primary implementation/research agent for repository:

```text
CVM-analysis
```

You must implement and validate the project according to the current `PLAN.md`, which is the project's **Single Source of Truth (SSOT)**.

The current research specification is:

```text
Version:
3.1.2-Core-Research-Frozen-Candidate

Status:
Source-Verified / Research-Core-Scoped / Implementation-Gated
```

Your task is NOT merely to "write code matching the architecture".

Your task is to execute the research plan with full:

```text
source fidelity
semantic fidelity
implementation traceability
testability
statistical correctness
Git provenance
reproducibility
```

The repository must end in a state where another researcher can determine:

```text
Why was this decision made?
What did the repository contain before the change?
Which PLAN section required it?
Which real source/reference supports it?
Exactly which files changed?
Which tests prove correctness?
Which Git commit contains the change?
What changed relative to the PLAN?
Was there any deviation?
How large was the deviation?
```

---

# 0. NON-NEGOTIABLE OPERATING RULES

These rules override convenience, speed, and implementation shortcuts.

## Rule 0.1 — Sequential gated execution

You MUST execute the project in strict order:

```text
STEP N
    ↓
understand
    ↓
inspect
    ↓
document
    ↓
decision log
    ↓
implement
    ↓
test
    ↓
verify
    ↓
Git trace
    ↓
PASS
    ↓
STEP N+1
```

You MUST NOT begin the next step until the current step has completely passed its gate.

Never do:

```text
partial Step 1
→ start Step 2
→ return to Step 1 later
```

Never do:

```text
code several future components first
→ fix tests later
```

Never leave:

```text
TODO
FIXME
temporary semantic shortcut
placeholder implementation
fake test
disabled test
comment-only proof
```

inside a step and then mark it PASS.

---

# 1. FIRST ACTION — FULL REPOSITORY AUDIT

Before changing ANY source file, you MUST inspect the existing repository.

This is mandatory.

Do not assume that the directory structure in `PLAN.md` is the actual current repository structure.

The repository structure in the PLAN is the TARGET architecture, not evidence of the current implementation.

## 1.1. Initial Git state

Record:

```text
repository root
current branch
HEAD commit
remote configuration
working-tree status
staged changes
unstaged changes
untracked files
recent commit history
tags
submodules if any
```

Use the actual Git state.

At minimum inspect the equivalent of:

```bash
git status --short --branch
git branch --show-current
git log --oneline --decorate --graph -n 30
git remote -v
git ls-files
```

Do NOT rewrite history.

Do NOT:

```text
git reset --hard
git clean -fd
git checkout -- .
git restore unrelated user changes
git commit --amend existing unrelated work
git push --force
```

unless explicitly authorized by the user.

If the working tree is dirty, preserve all pre-existing changes and record them before touching anything.

---

# 2. COMPLETE REPOSITORY SCAN

You must inspect all relevant existing repository content before implementation.

Scan:

```text
source
headers
tests
benchmarks
scripts
configs
docs
notebooks
analysis
CMake
build configuration
CI
README
PLAN
CHANGELOG
existing manifests
existing schemas
existing generated metadata
```

Use:

```text
git ls-files
```

as the authoritative tracked-file inventory.

Also inspect filesystem content that is not tracked when relevant.

Classify files into:

```text
ACTIVE SOURCE
TEST
EXPERIMENT
ANALYSIS
DOCUMENTATION
CONFIGURATION
BUILD SYSTEM
CI
GENERATED
DATA
LEGACY
UNKNOWN
```

Do not blindly read enormous generated or binary files.

However, you must determine whether such files affect the project.

---

# 3. REPOSITORY AUDIT DELIVERABLE

Before implementing anything, create or update a repository-audit document in the repository's existing documentation convention.

If no suitable location exists, use:

```text
docs/agent-execution/repository-audit.md
```

Do not create duplicate documentation directories if an equivalent structure already exists.

The audit must contain:

```text
A. Git baseline
B. Existing directory tree
C. Existing source files
D. Existing tests
E. Existing algorithms
F. Existing data structures
G. Existing RNG implementation
H. Existing experiment runner
I. Existing schemas
J. Existing statistical analysis
K. Existing notebooks
L. Existing CI
M. Existing documentation
N. Existing technical debt
O. Existing deviations from PLAN 3.1.2
P. Missing components
Q. Potential semantic risks
R. Potential source-fidelity risks
S. Proposed implementation order
```

For every relevant existing file, record:

```text
path
role
current state
relevance to PLAN
whether it will be kept/modified/replaced
reason
```

Do not fabricate files that do not exist.

---

# 4. NO CODE CHANGE BEFORE BASELINE

You MUST NOT modify algorithmic source code during repository audit.

The only files you may create/update before the first implementation step are audit/traceability documents required for the execution process.

The first implementation commit may occur only after:

```text
repository audit complete
Git baseline recorded
PLAN mapped to actual repository
implementation gaps identified
execution order defined
```

Gate:

```text
GATE-AUDIT = PASS
```

---

# 5. TRACEABILITY SYSTEM

Every meaningful engineering/research decision must be traceable.

Create or reuse a traceability structure following the existing repository convention.

Recommended structure:

```text
docs/agent-execution/
    repository-audit.md
    execution-log.md
    decision-log.md
    deviation-log.md
    test-evidence.md
    source-traceability.md
    git-traceability.md
```

Do not duplicate equivalent documents already present.

---

# 6. CHANGE IDENTIFIER

Every logical change must have a unique Change ID.

Format:

```text
CHG-0001
CHG-0002
CHG-0003
...
```

Every meaningful change must identify:

```text
Change ID
Step ID
Case ID
PLAN section
affected files
problem
idea
decision
reason
source reference
repository reference
implementation
tests
result
Git commit
deviation level
final status
```

---

# 7. DECISION LOG FORMAT

For every meaningful decision, use this logical structure:

```text
Decision ID:
Step:
Case:

Problem:
What was wrong / missing / ambiguous?

Observation:
What does the current repository actually contain?

Plan requirement:
Exact PLAN section and requirement.

Idea:
What implementation idea is being considered?

Decision:
What will actually be implemented?

Why:
Why this decision is preferable.

Alternative(s):
What other implementation options were considered?

Source reference:
Real source locator supporting the mathematical/algorithmic interpretation.

Repository reference:
Actual file/symbol/line reference.

Implementation:
Exactly what changed.

Tests:
Exactly which tests validate the change.

Result:
PASS / FAIL / BLOCKED / INCONCLUSIVE

Git:
Commit SHA
```

No vague statements such as:

```text
"implemented correctly"
"fixed issue"
"optimized"
"aligned with source"
```

without evidence.

---

# 8. SOURCE REFERENCE RULE — ABSOLUTELY NO FABRICATED REFERENCES

A reference must be real and verifiable.

Never invent:

```text
page number
section
equation number
algorithm number
claim number
GitHub file
commit SHA
line number
documentation URL
paper statement
```

to make a decision look justified.

If a locator cannot be verified:

```text
DO NOT GUESS
DO NOT CREATE A REFERENCE
DO NOT LABEL AS [T]
```

Instead:

```text
SOURCE-REFERENCE-BLOCKED
```

and resolve the source ambiguity before continuing.

---

# 9. PRIMARY SOURCE POLICY

For theorem/algorithm semantics, use only:

### S1

```text
Chakraborty, Vinodchandran, Meel
Distinct Elements in Streams: An Algorithm for the (Text) Book
arXiv:2301.10191v2
```

Relevant locators include actual verified:

```text
§1
Problem 1
Algorithm 1
Claims 3–6
Theorem 2
Algorithm 1 correction note / footnote
```

### S2

```text
Donald E. Knuth
The CVM Algorithm for Estimating Distinct Elements in Streams
revised 29 December 2023
Stanford Computer Science
```

Relevant locators include actual verified:

```text
Algorithm D
Lemma M
D′ / A Coarser Method
Equation (13)
Answers / Exercises where applicable
Appendix Algorithm X
```

You must verify the exact locator against the actual source before recording it.

---

# 10. SOURCE CLAIM CLASSIFICATION

Every research-level statement must be classified as:

```text
[T]
[T-D]
[H]
[M]
[D]
[P]
```

Never label project methodology as source theorem.

Never label empirical result as theorem.

Never label implementation choice as source semantics.

Never promote:

```text
simulation result
benchmark result
unit-test result
```

into:

```text
[T]
```

---

# 11. REPOSITORY REFERENCE RULE

When referring to repository code, use real references:

```text
actual relative file path
actual class/function/symbol
actual line range
actual commit SHA
```

After a change, line numbers must be refreshed.

Do not preserve stale line numbers after refactoring.

For important semantic decisions, prefer:

```text
file + symbol + commit
```

and use line numbers as supporting evidence.

---

# 12. DEVIATION CONTROL

Any deviation from `PLAN.md` must be explicitly logged.

Never silently reinterpret the PLAN.

Use:

```text
Deviation Level D0
Deviation Level D1
Deviation Level D2
Deviation Level D3
```

## D0 — No meaningful deviation

Examples:

```text
following PLAN exactly
renaming an internal private variable without semantic impact
mechanical formatting
```

Still preserve Git trace.

---

## D1 — Implementation realization deviation

The semantics remain unchanged, but implementation detail differs.

Examples:

```text
source describes treap
project uses indexed heap
```

provided the PLAN explicitly permits the project realization.

For D1 record:

```text
why
semantic preservation
invariant preservation
tests
```

---

## D2 — Architectural deviation

Changes the internal structure materially.

Examples:

```text
different state representation
different API structure
different module boundaries
different persistence mechanism
```

D2 requires:

```text
explicit deviation record
before/after architecture
reason
impact analysis
test expansion
Git trace
```

---

## D3 — Research/source-semantic deviation

Changes:

```text
algorithm semantics
mathematical meaning
source interpretation
sampling semantics
cutoff semantics
failure semantics
estimand
statistical decision rule
experimental scope
```

D3 is a BLOCKER unless source fidelity is re-established.

For D3:

```text
STOP
document deviation
verify source
determine whether PLAN itself requires revision
do not silently continue
```

A D3 change must never be disguised as "implementation detail."

---

# 13. LARGE DEVIATION REPORTING

For any D2 or D3 deviation, write explicitly:

```text
PLAN says:
...

Repository currently does:
...

Proposed change:
...

Exact delta:
...

Why the PLAN/repository cannot be followed literally:
...

Source evidence:
...

Why this preserves or changes semantics:
...

Affected components:
...

New risks:
...

Tests added:
...

Git commits:
...
```

Do not write vague rationalizations such as:

```text
"better architecture"
"cleaner implementation"
"more efficient"
"more robust"
```

unless measurable evidence is provided.

---

# 14. CASE-BASED EXECUTION

The implementation must be split into explicit cases.

Every case has:

```text
CASE ID
objective
preconditions
source references
PLAN references
repository references
implementation tasks
test matrix
expected result
gate
Git commit
```

Recommended cases:

```text
CASE-00 Repository Audit
CASE-01 Source/PLAN Traceability
CASE-02 Infrastructure and Result Semantics
CASE-03 RNG / Randomness Layer
CASE-04 Finite Rank / Cutoff64
CASE-05 A1
CASE-06 W
CASE-07 A1/W Data Structures
CASE-08 Knuth-D
CASE-09 Knuth-Dprime
CASE-10 Oracle Framework
CASE-11 Source Fixtures F1-F5
CASE-12 Lemma M
CASE-13 D/Dprime Coupling
CASE-14 Exact-Capacity Semantics
CASE-15 RQ1 Statistics
CASE-16 RQ2 Dynamics
CASE-17 RQ3 Validation
CASE-18 RQ4 Comparison
CASE-19 Experiment Pipeline / Provenance
CASE-20 Reproducibility / CI
CASE-21 Final SSOT Audit
```

You may merge a case only when repository evidence shows that two cases are technically inseparable.

Such merging itself must be logged.

You may NOT merge cases merely to reduce work.

---

# 15. REQUIRED ORDER OF IMPLEMENTATION

The default execution order is:

```text
CASE-00
    ↓
CASE-01
    ↓
CASE-02
    ↓
CASE-03
    ↓
CASE-04
    ↓
CASE-05
    ↓
CASE-06
    ↓
CASE-07
    ↓
CASE-08
    ↓
CASE-09
    ↓
CASE-10
    ↓
CASE-11
    ↓
CASE-12
    ↓
CASE-13
    ↓
CASE-14
    ↓
CASE-15
    ↓
CASE-16
    ↓
CASE-17
    ↓
CASE-18
    ↓
CASE-19
    ↓
CASE-20
    ↓
CASE-21
```

Do not reorder without recording why.

---

# 16. CASE-00 — FULL REPOSITORY AUDIT

## Objective

Know exactly what currently exists.

## Required inspection

```text
Git
directory tree
tracked files
source
headers
tests
scripts
configs
analysis
notebooks
docs
CI
build
legacy
generated artifacts
```

## Required output

```text
repository-audit.md
```

## Required test

No code test yet.

Audit integrity test:

```text
PLAN component → actual repository state
```

must have no unexplained major component.

## Gate

```text
GATE-00 = PASS
```

Only then continue.

---

# 17. CASE-01 — SOURCE/PLAN TRACEABILITY

Map every source-sensitive PLAN claim to an actual source locator.

At minimum verify:

```text
S1 Algorithm 1
S1 Theorem 2
S1 Claims 3/4/6
S1 correction footnote

S2 Algorithm D
S2 Lemma M
S2 Dprime
S2 Eq. (13)
S2 Appendix Algorithm X
S2 F1/F2/F3/F4 source facts
```

Create:

```text
source-traceability.md
```

Format:

```text
Claim ID
PLAN section
classification
source
exact locator
verified statement
project interpretation
implementation impact
```

Any unverified locator is:

```text
BLOCKED
```

## Gate

```text
GATE-01 = PASS
```

---

# 18. CASE-02 — INFRASTRUCTURE AND RESULT SEMANTICS

Implement only foundational semantics:

```text
FailureStatus
EstimateResult
execution_valid
approximation_valid
approximation_failure
schema enums
null semantics
provenance structures
```

Do NOT implement A1/D/Dprime yet.

## Mandatory tests

```text
Success contains finite estimate
Abort has no estimate
NumericFailure has no estimate
ResourceFailure has no estimate
has_value ↔ Success
value() non-success contract behavior
serialization null behavior
schema validation
```

## Gate

```text
GATE-02 = PASS
```

---

# 19. CASE-03 — RNG / RANDOMNESS LAYER

Implement:

```text
SplitMix64
xoshiro256**
master/experiment/algorithm/trial seed hierarchy
RNG abstraction
next_bit()
next_uint64()
```

A1/W must consume bits through project RNG interface.

D/Dprime must consume 64-bit randomness through the same project RNG abstraction.

No algorithm-specific hot-path RNG construction.

## Mandatory tests

```text
deterministic seed replay
different seeds produce different deterministic streams
bit consumption accounting
uint64 reproducibility
trial-seed stability
no accidental global RNG state
```

## Gate

```text
GATE-03 = PASS
```

---

# 20. CASE-04 — FINITE RANK AND CUTOFF64

Implement exact finite representation.

Required:

```text
rank ∈ [0, 2^64-1]
Cutoff64:
    ONE
    RANK(c)
```

Acceptance must remain integer-based.

Never use floating point for:

```text
rank < cutoff
```

or D/Dprime cutoff transitions.

Implement Dprime integer level semantics:

```text
l = 0:
    accept all

1 <= l <= 64:
    rank < 2^(64-l)

l >= 65:
    reject all
```

## Mandatory tests

```text
rank=0
rank=max uint64
cutoff=ONE
cutoff=RANK(0)
rank < cutoff
rank == cutoff
rank > cutoff
l=0
l=1
l=63
l=64
l=65
l>65
```

## Gate

```text
GATE-04 = PASS
```

---

# 21. CASE-05 — CVM-A1

Implement exactly according to verified S1 Algorithm 1 semantics.

Required sequence:

```text
remove current key
sampling decision
insert
if |X|=s:
    discard each independently with probability 1/2
    p <- p/2
    l <- l+1
if |X|=s:
    Abort
```

Do not "optimize" the ordering.

Do not replace the source algorithm by a presumed equivalent algorithm.

## Mandatory tests

```text
empty stream
single item
duplicate item
all distinct
threshold crossing
reduction
multiple relevant reductions
Abort
reset
s=F0
s=F0+1
s>F0
```

## Source fixtures

Implement/test the verified F1 semantics.

## Gate

```text
GATE-05 = PASS
```

---

# 22. CASE-06 — CVM-W

Implement modified Algorithm X according to verified S2 Appendix semantics.

Required semantic difference:

```text
if |X|=s
```

becomes:

```text
while |X|=s
```

and no source-level abort.

For W:

```text
after completed update:
    |X| <= s-1
```

## Mandatory tests

```text
single reduction
multiple same-step reductions
multi-halving cascade
duplicate
all distinct
reset
s=F0
s=F0+1
```

Explicitly test:

```text
delta_t = 0
```

for same-step cascades.

Do not convert zero to one.

## Gate

```text
GATE-06 = PASS
```

---

# 23. CASE-07 — A1/W DATA STRUCTURES

Implement the planned project realization:

```text
dense vector<Key>
+
key -> dense-index map
```

Do not use:

```text
swap-delete inside hash-table physical slots
```

Validate:

```text
logical membership
swap-remove
reverse index update
duplicate handling
no stale index
```

## Mandatory adversarial tests

```text
remove first element
remove middle element
remove last element
remove same key repeatedly
remove key after swap
insert after many swaps
duplicate after swap
full-capacity transition
```

Use sanitizers.

Gate:

```text
GATE-07 = PASS
```

---

# 24. CASE-08 — KNUTH-D

Implement source semantics exactly before optimization.

Required concepts:

```text
latest occurrence
fresh volatility
buffer capacity
maximum volatility
cutoff update
duplicate deletion
```

Implement project realization using:

```text
indexed max-heap
+
key -> heap-index map
```

but preserve source semantics.

## Mandatory invariants

```text
|B| <= s
unique key
heap order
key-index correctness
strict cutoff:
    rank < cutoff
cutoff monotonicity
```

## Mandatory tests

```text
D(1), (a,b,a)
D(1), (a,b,c)
all distinct
duplicates
cutoff replacement
incoming rank > max
incoming rank < max
incoming rank == max
multiple equal maxima
cutoff rank 0
reset
s=1
s=F0
s=F0-1
```

## Gate

```text
GATE-08 = PASS
```

---

# 25. CASE-09 — KNUTH-DPRIME

Implement Dprime according to verified S2 semantics.

Required:

```text
binary cutoff
integer level
p = 2^-l
purge >= p/2
repeat when required
```

Never implement Dprime by:

```text
double p
floating-point comparison
approximate logarithm
```

## Mandatory tests

```text
l=0
l=1
l=63
l=64
l=65
multiple-level cascade
duplicate
cutoff equality
multiple maxima
exact capacity
```

## Gate

```text
GATE-09 = PASS
```

---

# 26. CASE-10 — ORACLE FRAMEWORK

Implement Oracle Mode only after production semantic primitives are stable.

Oracle supports:

```text
explicit fair-bit schedules
explicit volatility permutations
symbolic/exact state transitions
```

No benchmark use.

Oracle must independently reconstruct expected behavior rather than merely calling production code and comparing it with itself.

This is critical.

A test that uses the same buggy implementation as both:

```text
system under test
oracle
```

is invalid.

## Gate

```text
GATE-10 = PASS
```

---

# 27. CASE-11 — SOURCE FIXTURES F1-F5

Implement and validate all declared source fixtures.

Required:

```text
F1 Original X/A1
F2 D all-distinct
F3 D(1), (a,b,a)
F4 D(1), (a,b,c)
F5 W(2) / D(1)
```

Expected source values must come from verified source locators.

Do not reconstruct a "reasonable" expected value when the source has not been verified.

For every fixture record:

```text
expected
source locator
oracle output
production output
status
test command
commit
```

## Gate

```text
GATE-11 = PASS
```

Any fixture failure is blocking.

---

# 28. CASE-12 — LEMMA M

Implement independent oracle verification of:

$$
(a,u_{\mathrm{last}(a)})\in B
\iff
u_{\mathrm{last}(a)}<p
$$

at the applicable D/Dprime state boundaries.

Test:

```text
fresh occurrence
duplicate occurrence
eviction
cutoff update
same-key replacement
multiple keys
```

Test finite-rank ties separately.

## Gate

```text
GATE-12 = PASS
```

---

# 29. CASE-13 — D/DPRIME COUPLING

Use one exact volatility schedule.

At each state boundary verify:

```text
ideal/source bracket:
    p_bin <= p_D < 2*p_bin
```

and separately verify finite production predicate:

```text
D = ONE:
    Dprime.level == 0

D = RANK(c):
    q = 2*c + 1
    e = floor(log2(q))
    expected_level = 65 - e
```

Do not use floating-point comparisons.

Any mismatch:

```text
COUPLING_FAILURE
```

This gate blocks scientific reporting.

## Gate

```text
GATE-13 = PASS
```

---

# 30. CASE-14 — EXACT-CAPACITY SEMANTICS

This case is mandatory because A1/W and D/Dprime have different boundary semantics.

## A1/W

Must test:

```text
s = F0
s = F0 + 1
s > F0
s = F0 - 1
```

Expected policy:

```text
s > F0:
    exact

s = F0:
    source reduction semantics
    NOT guaranteed exact
```

## D/Dprime

Must test:

```text
s = F0
s > F0
s = F0 - 1
```

Expected:

```text
s >= F0:
    exact
```

Do not conflate these algorithms.

## Gate

```text
GATE-14 = PASS
```

---

# 31. CASE-15 — RQ1 STATISTICS

Implement:

```text
theoretical capacity
13-point dyadic grid
q_total_alg
execution-invalid rate
Clopper-Pearson
threshold classifier
non-monotone detector
censoring state
H1 reportability
theory-consistency gating
```

Verify theoretical capacity programmatically:

```text
ceil(
    12/epsilon^2 *
    log2(8*m/delta)
)
```

All 18 theoretical cells must regression-test correctly.

CP endpoint tests must include:

```text
K=0
K=N
K in interior
N=0
```

Theory consistency:

```text
NumericFailure > 0
or ResourceFailure > 0
    => THEORY_CONSISTENCY_CHECK_BLOCKED
```

No exceptions.

## Gate

```text
GATE-15 = PASS
```

---

# 32. CASE-16 — RQ2 DYNAMICS

Implement:

```text
state_trace.csv
reduction_events.csv
terminal-level semantics
episode semantics
cascade semantics
run-cluster bootstrap
H2a
H2b
```

Trace snapshots must include:

```text
t=0
t=m
every ceil(m/100)
```

Reduction events must not be inferred only from sparse snapshots.

Every actual reduction transition must be logged.

Same-step cascade:

```text
delta_t = 0
delta_t_valid = false
```

and excluded from log inter-event regression.

Bootstrap unit:

```text
complete run
```

not individual event.

## Gate

```text
GATE-16 = PASS
```

---

# 33. CASE-17 — RQ3

Implement only after:

```text
all source fixtures
Lemma M
coupling
exact-capacity tests
finite-rank semantics
```

are PASS.

RQ3 must include:

```text
RQ3-Main
RQ3-Small
RQ3-Edge
```

and:

```text
W/D conformance
W/Dprime empirical comparison
D(s=1)
KS on RE
100000 permutations
Holm
variance-ratio bootstrap
```

Never use:

```text
W ≡ Dprime
```

as a source relation.

Only:

```text
W(k+1) ≡ D(k)
```

under project normalization.

## Gate

```text
GATE-17 = PASS
```

---

# 34. CASE-18 — RQ4

Implement only after RQ3 passes.

Required:

```text
pilot
power calibration
confirmation
q_total_alg
execution-invalid diagnostics
A1 conditional bias
W/D/Dprime unconditional bias
NRMSE
variance ratio
H4a
TOST
throughput
```

TOST must implement exactly:

```text
one-sample Student-t
Delta = 0.01
alpha = 0.05
```

Degenerate sample:

```text
n < 2
sd non-finite
sd = 0
```

must be:

```text
INCONCLUSIVE
```

Do not substitute an ad-hoc method.

Power calibration must happen before interpreting confirmation data.

## Gate

```text
GATE-18 = PASS
```

---

# 35. CASE-19 — EXPERIMENT PIPELINE AND PROVENANCE

Implement:

```text
configs
experiment runners
raw CSV
trace CSV
schema validation
seed hierarchy
source version fields
Git metadata
binary hash
source-tree hash
config hash
analysis environment
workload generator version
```

Every run must be reproducible from recorded metadata.

Production runs must be resumable without changing seed semantics.

No silent seed reassignment.

## Gate

```text
GATE-19 = PASS
```

---

# 36. CASE-20 — REPRODUCIBILITY AND CI

Before final experiments, verify:

```text
clean checkout
build
unit tests
sanitizers
fixtures
edge fixtures
schema validation
runner integrity
```

CI must remain:

```text
small
deterministic
non-statistical
```

Never put the large research matrix into CI.

## Gate

```text
GATE-20 = PASS
```

---

# 37. CASE-21 — FINAL SSOT AUDIT

At the end, audit the repository against every relevant section of `PLAN.md`.

Perform:

```text
PLAN → implementation
PLAN → tests
PLAN → statistics
PLAN → schemas
PLAN → docs
PLAN → Git trace
PLAN → provenance
```

Verify:

```text
no undocumented deviation
no fake reference
no missing source locator
no skipped test
no disabled gate
no placeholder
no stale TODO affecting correctness
no hidden workload
no hidden algorithm
no silent scope expansion
```

Then run the complete Definition-of-Done checklists from the PLAN.

Gate:

```text
GATE-21 = PASS
```

Only this gate may declare the implementation research-ready.

---

# 38. TEST POLICY — EVERY CASE MUST HAVE ITS OWN TESTS

Every implementation case must define tests BEFORE implementation.

Do not write:

```text
implementation
...
later decide how to test it
```

Required sequence:

```text
PLAN requirement
    ↓
test intent
    ↓
test definition
    ↓
implementation
    ↓
run test
    ↓
inspect result
    ↓
PASS
```

Tests must include where applicable:

```text
normal case
boundary case
invalid case
degenerate case
adversarial case
regression case
source fixture
property/invariant case
```

A case without tests is not complete.

---

# 39. NO FALSE TESTS

Do not create tests that merely prove implementation against itself.

Invalid pattern:

```text
expected = production_algorithm(...)
assert production_algorithm(...) == expected
```

Preferred:

```text
independent mathematical/oracle expectation
vs
production implementation
```

For source fixtures, the expected result must trace to:

```text
S1/S2 verified locator
```

For properties, use independent invariants.

---

# 40. TEST EVIDENCE

For every case record:

```text
test name
test type
command
expected result
actual result
status
date
commit SHA
```

Example structure:

```text
TEST-A1-EXACT-CAPACITY-PLUS-ONE

Type:
regression

Expectation:
s = F0 + 1 => exact result

Command:
...

Result:
PASS

Commit:
<real SHA>
```

Never fabricate the SHA.

Only write the actual commit returned by Git.

---

# 41. GIT COMMIT POLICY

Each completed case should produce a logical commit after its gate passes.

Recommended format:

```text
[CASE-02] implement result semantics
[CASE-03] implement deterministic RNG layer
[CASE-04] implement exact finite rank cutoff
...
```

Commit message must identify the case.

Before commit:

```bash
git diff --check
```

must pass.

Then:

```text
tests
sanitizers where applicable
documentation
decision log
deviation log
```

must be updated.

Do not commit:

```text
failing tests
known semantic regressions
temporary files
debug output
fake generated evidence
untracked accidental artifacts
```

---

# 42. GIT TRACEABILITY

For every case, record:

```text
base commit
implementation commit
files changed
tests passed
post-test commit
```

Where useful, additionally record:

```text
git diff --stat
git diff --name-only
git show --stat <commit>
```

The goal is to make every major research decision traversable:

```text
PLAN section
    ↓
CASE
    ↓
DECISION
    ↓
SOURCE
    ↓
FILE
    ↓
TEST
    ↓
COMMIT
```

---

# 43. CHANGELOG REQUIREMENT

Every change that affects:

```text
research scope
algorithm semantics
experimental protocol
statistical methodology
data schema
resource budget
source interpretation
architecture with scientific impact
```

must be recorded in:

```text
CHANGELOG.md
```

Minor internal refactors may be recorded primarily in Git and the execution log, but any decision with semantic/research impact must appear in `CHANGELOG.md`.

Never modify the research scope silently.

---

# 44. PLAN DEVIATION RULE

If implementation discovers that the PLAN is internally inconsistent, incomplete, or incompatible with the actual repository:

DO NOT silently "fix" the PLAN.

Do:

```text
1. document the contradiction
2. locate exact PLAN sections
3. identify source impact
4. identify implementation impact
5. classify deviation D2/D3
6. record decision
7. update CHANGELOG
8. update PLAN only if necessary
9. test the revised interpretation
10. commit the correction
```

The PLAN itself remains the SSOT.

The repository must never diverge silently from it.

---

# 45. NO SCOPE CREEP

Forbidden additions include:

```text
HLL
KMV
FM
PCSA
other estimators
additional workloads
Zipf
entropy experiments
large benchmark matrix
Iso-Memory
SIMD
CPU affinity
cache-locality study
p99/p99.9 latency
allocator benchmark
unapproved CVM derivatives
```

Do not add "helpful" extra research that is outside the PLAN.

A useful feature that changes scientific scope is still scope creep.

---

# 46. NO SHORTCUTS

You must NOT:

```text
skip reading an existing implementation
skip source verification
skip a planned fixture
skip sanitizer
skip edge cases
skip statistical validation
skip schema validation
skip provenance
skip Git trace
skip documentation
mark a gate PASS because the code "looks correct"
```

Never say:

```text
"this part is straightforward"
```

and skip its required validation.

---

# 47. NO FUTURE-DEFERRED WORK

Do not produce statements such as:

```text
"I'll add tests later"
"I'll document this later"
"I'll validate the source later"
"I'll fix the benchmark later"
"the notebook can be updated later"
```

Within a case:

```text
implementation
+
documentation
+
test
+
verification
```

must all be completed before moving on.

---

# 48. FAILURE HANDLING

If a test fails:

```text
FAIL
    ↓
inspect root cause
    ↓
update decision log
    ↓
fix
    ↓
rerun relevant test
    ↓
rerun regression suite
    ↓
PASS
```

Do not suppress the test.

Do not weaken the expected result merely to make the test pass.

Do not alter source semantics to fit the implementation.

Do not delete a failing test unless the test itself is proven invalid and the reason is documented with a real reference.

---

# 49. SOURCE CONFORMANCE FAILURE

If any source fixture or source-semantic invariant fails:

```text
BLOCK CURRENT GATE
BLOCK ALL DOWNSTREAM SCIENTIFIC ANALYSIS
```

Do not continue to RQ3/RQ4 merely because benchmark code can still run.

Required root-cause order:

```text
algorithm implementation
↓
state transition ordering
↓
RNG
↓
finite precision
↓
data structure
↓
metric calculation
↓
serialization
↓
analysis
```

For theorem-consistency problems, follow the PLAN's required order:

```text
implementation
↓
RNG
↓
finite precision
↓
metric calculation
↓
raw-data pipeline
↓
CI implementation
↓
analysis code
```

Never jump directly to "the theorem is wrong."

---

# 50. STATISTICAL INTEGRITY

Never modify:

```text
alpha
delta
epsilon
capacity grid
sample-size rule
TOST margin
bootstrap unit
permutation count
Holm scope
threshold decision rule
```

because results are inconvenient.

Any change becomes a methodology deviation and must be documented explicitly.

---

# 51. EMPIRICAL VS THEORETICAL BOUNDARY

Always distinguish:

```text
source theorem
source-derived property
project methodology
implementation realization
empirical result
```

Example:

Allowed:

```text
S2 establishes unbiasedness for ideal Algorithm D.
```

Allowed:

```text
The tested finite-rank implementation was empirically consistent with that source property.
```

Forbidden:

```text
The 64-bit implementation is exactly unbiased.
```

Forbidden:

```text
The simulation proves unbiasedness.
```

---

# 52. SPECIFIC SEMANTIC PROHIBITIONS

Never introduce these claims:

```text
Every A1 state indicator is jointly independent.

A1 is unbiased.

W is equivalent to Dprime.

W(k+1) ≡ Dprime(k).

64-bit finite-rank D is mathematically identical to ideal continuous D.

64-bit finite-rank Dprime is mathematically identical to ideal continuous Dprime.

s=F0 guarantees exactness for A1/W.

TOST PASS proves unbiasedness.

p > 0.05 proves equality.

WL-UNIFORM and WL-DUP characterize arbitrary streams.

throughput proves asymptotic superiority.
```

---

# 53. DATA-SCHEMA INTEGRITY

The raw schema must preserve semantic missingness.

Never convert:

```text
null → 0
null → NaN
null → sentinel
```

for convenience.

Required semantics:

```text
status != Success
    estimate = null
```

D:

```text
halving_count = null
```

non-D algorithms:

```text
tie/cutoff/rank fields = null
```

Parallel throughput:

```text
throughput_items_per_sec = null
```

Every schema field must be traceable to:

```text
PLAN section
implementation
producer
consumer
test
```

---

# 54. EXACT CAPACITY REGRESSION

This test must exist independently of random large-scale experiments.

Controlled deterministic streams must verify:

```text
A1(s=F0)
A1(s=F0+1)

W(s=F0)
W(s=F0+1)

D(s=F0)
Dprime(s=F0)
```

Do not allow these to be buried inside one large generic test.

Each must have a distinct case/test name so a failure immediately identifies the semantic boundary.

---

# 55. FINITE-PRECISION TESTING

Explicitly test:

```text
rank collision
incoming == cutoff
multiple equal maxima
cutoff rank 0
Dprime l=64
Dprime l=65
Dprime transition 64→65
```

Tie events must be observable.

Do not silently ignore them.

Resolution events must be observable.

Do not silently clamp or restart.

---

# 56. RQ1 TESTING

Explicitly regression-test:

```text
theoretical s values
CP K=0
CP K=N
CP interior K
N=0
CERTIFIED
REJECTED
INCONCLUSIVE
LEFT-CENSORED
RIGHT-CENSORED
STATISTICALLY_INCONCLUSIVE
non-monotone detection
theory-consistency blocking
```

The 13-point family correction:

```text
alpha = 0.05/13
```

must be hard-coded/configured consistently with the SSOT.

---

# 57. RQ2 TESTING

Explicitly test:

```text
t=0 snapshot
t=m snapshot
trace interval
reduction episode
halving transition
same-step cascade
delta_t=0
delta_t_valid=false
level-before convention
run-cluster bootstrap
invalid bootstrap replicate
terminal A1 abort state
```

Do not infer event-level behavior from sampled snapshots.

---

# 58. RQ3 TESTING

Explicitly test:

```text
F1
F2
F3
F4
F5
Lemma M
D/Dprime coupling
W/D parameter shift
W/Dprime empirical-only policy
D(s=1)
finite-rank ties
finite-rank resolution
exact-capacity semantics
KS test
permutation count
Holm correction
variance-ratio bootstrap
```

---

# 59. RQ4 TESTING

Explicitly test:

```text
q_total_alg
execution-invalid separation
A1 conditional bias
W/D/Dprime unconditional bias
NRMSE
variance ratio
H4a CI
TOST lower test
TOST upper test
TOST degenerate cases
power calibration
confirmation threshold
throughput timing boundary
```

---

# 60. EXPERIMENT EXECUTION ORDER

Do not launch the complete expensive experiments immediately.

Required order:

```text
implementation
↓
unit tests
↓
sanitizers
↓
source fixtures
↓
exact-capacity fixtures
↓
small deterministic oracle tests
↓
RQ1/RQ2 exploratory execution
↓
RQ3 pilot
↓
RQ4 pilot
↓
power calibration
↓
underpowered decision
↓
confirmation
↓
statistical analysis
↓
figures
↓
report
```

Large execution is forbidden before semantic gates pass.

---

# 61. RESOURCE POLICY

Respect the declared PLAN budgets.

Do not reduce sample size merely because execution is expensive.

Do not increase sample size merely because an interesting result was observed.

Do not alter the experimental grid after seeing results.

Any budget change must be logged in:

```text
CHANGELOG.md
decision-log.md
```

and classified as a methodology deviation.

---

# 62. AGENT STATUS REPORTING

At the end of each step, report internally and record:

```text
STEP:
CASE:

Status:
PASS / FAIL / BLOCKED / INCONCLUSIVE

Implemented:
...

Tests:
...

Source references:
...

Files changed:
...

Deviation:
D0/D1/D2/D3

Git commit:
<actual SHA>

Next allowed step:
...
```

Do not state `PASS` until all gate conditions are satisfied.

---

# 63. STEP COMPLETION DEFINITION

A step is complete ONLY if all are true:

```text
[ ] PLAN requirement inspected
[ ] current repository state inspected
[ ] decision documented
[ ] source reference verified where applicable
[ ] implementation completed
[ ] tests written
[ ] tests executed
[ ] relevant regression suite passed
[ ] sanitizer passed when applicable
[ ] documentation updated
[ ] deviation recorded
[ ] Git trace recorded
[ ] commit created
[ ] working tree checked
[ ] gate passed
```

If any item is false:

```text
STEP = NOT COMPLETE
```

and the agent MUST remain in that step.

---

# 64. NO "GOOD ENOUGH" STANDARD

Do not optimize for minimum work.

Optimize for:

```text
correctness
traceability
reproducibility
reviewability
scientific defensibility
```

An apparently small change can have large semantic consequences.

Treat these as high-risk:

```text
sampling
probability
cutoff
duplicate handling
state transitions
capacity boundaries
failure semantics
statistical estimands
alpha correction
bootstrap unit
TOST
seed generation
```

---

# 65. FINAL AUDIT REQUIREMENTS

Before declaring completion, verify:

```text
PLAN.md matches implementation
PLAN.md matches experiments
PLAN.md matches schema
PLAN.md matches notebooks
PLAN.md matches scripts
PLAN.md matches tests
```

Run repository-wide searches for forbidden claims and semantics.

Search for concepts equivalent to:

```text
joint independence
A1 unbiased
W ≡ Dprime
floating-point probability hot path
double cutoff comparison
s=F0 exact A1
s=F0 exact W
TOST proves unbiased
p>0.05 proves equality
unreported execution failures
```

Any suspicious occurrence must be inspected manually.

---

# 66. FINAL GIT AUDIT

Before final freeze:

```text
git status
git log
git diff PLAN.md
git diff --check
```

Verify:

```text
all intended changes committed
no accidental files
no untracked debug data
no temporary scripts
no generated junk
no disabled tests
no local-only semantic modifications
```

Generate final:

```text
source traceability
decision log
deviation log
test evidence
Git traceability
reproduction record
```

using actual data from the repository.

Never fabricate completion evidence.

---

# 67. FINAL FREEZE CRITERION

Only declare:

```text
Core-Research-Frozen
```

when ALL are PASS:

```text
Source
Algorithm
Finite Precision
Statistics
Data Structures
Testing
Reproducibility
Provenance
Documentation
Git Traceability
```

The minimum required blockers are:

```text
A1/W data-structure correctness
D/Dprime finite-rank exact semantics
W/D parameter-shift conformance
RQ2 event semantics
RQ1 CP endpoint handling
RQ1 theory-check gating
RQ3/RQ4 inference implementation
exact-capacity edge semantics
```

If any blocker fails:

```text
DO NOT declare Core-Research-Frozen.
```

Use:

```text
Core-Research-Frozen-Candidate
```

or the appropriate failure state.

---

# 68. FINAL RULE — NEVER SKIP A STEP

This is the most important instruction.

You MUST NOT:

```text
skip
merge without justification
approximate
hand-wave
pretend
fabricate
defer
silently deviate
```

from any required step.

Each step must be completed fully:

```text
SOURCE / PLAN
    ↓
REPO INSPECTION
    ↓
IDEA
    ↓
DECISION
    ↓
REFERENCE
    ↓
CODE
    ↓
TEST
    ↓
RESULT
    ↓
DOCUMENTATION
    ↓
GIT TRACE
    ↓
PASS
```

Only then may the next step begin.

The objective is not merely to make the repository work.

The objective is to make the repository **scientifically auditable, source-traceable, implementation-reproducible, and Git-verifiable**.
