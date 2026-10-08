# MASTER RESEARCH & IMPLEMENTATION SPECIFICATION: CVM-ANALYSIS

## Distinct Elements in Streams: An Algorithm for the Book — Core Research Specification

> **Version:** `3.1.2-Core-Research-Frozen-Candidate`
> **Status:** `Source-Verified / Research-Core-Scoped / Implementation-Gated`
> **Repository:** `CVM-analysis`
> **Problem:** Streaming Distinct Elements / \(F_0\) Cardinality Estimation
> **Primary sources:** S1, S2 only
> **Document role:** Single Source of Truth (SSOT)

---

# 0. DOCUMENT CONTROL

## 0.1. Purpose

Tài liệu này là **Single Source of Truth (SSOT)** cho toàn bộ research và implementation của repository `CVM-analysis`.

Mọi thành phần sau phải tuân thủ SSOT:

```text
C++ implementation
experiment runner
configuration
test suite
statistical analysis
notebooks
figures
tables
CI/CD
provenance
resource planning
documentation
```

Không được mở rộng hoặc thay đổi research scope trong code/notebook mà không cập nhật SSOT.

---

## 0.2. Source-First Principle

Khi có xung đột, thứ tự ưu tiên là:

```text
Primary Source
    ↓
Source-verified mathematical interpretation
    ↓
Project methodology [M]
    ↓
Implementation realization [M]
    ↓
Experiment result
```

Không được đảo chiều quan hệ này.

Một empirical result không thể thay thế theorem.

Một implementation choice không thể được viết như source semantics.

---

## 0.3. Claim Labels

| Label     | Meaning                                                                         |
| --------- | ------------------------------------------------------------------------------- |
| **[T]**   | Theorem / lemma / exact source fact được thiết lập trong S1 hoặc S2             |
| **[T-D]** | Claim được suy ra trực tiếp từ [T] bằng lập luận toán học ngắn và kiểm tra được |
| **[H]**   | Research hypothesis / empirical expectation                                     |
| **[M]**   | Project methodology / implementation convention                                 |
| **[D]**   | Pure definition / notation                                                      |
| **[P]**   | Planning / resource / engineering estimate                                      |

Mọi [T] và [T-D] phải có source locator cụ thể.

---

## 0.4. Status Vocabulary

### Research / execution statuses

```text
PASS
FAIL
BLOCKED
INCONCLUSIVE
INCONCLUSIVE_UNDERPOWERED
THEORY_CONSISTENCY_ALERT
THEORY_CONSISTENCY_CHECK_BLOCKED
COUPLING_FAILURE
FINITE_RANK_RESOLUTION_EVENT
```

### Threshold state của RQ1

`threshold_state` là một biến phân loại độc lập:

```text
IDENTIFIED
STATISTICALLY_INCONCLUSIVE
LEFT-CENSORED
RIGHT-CENSORED
```

`non_monotone_flag` là một biến Boolean độc lập:

```text
false
true
```

Không dùng `IDENTIFIED_NON_MONOTONE` làm threshold state.

---

# 1. PRIMARY SOURCES

## 1.1. S1 — Chakraborty–Vinodchandran–Meel

**S1**

Sourav Chakraborty, N. V. Vinodchandran, Kuldeep S. Meel.

*Distinct Elements in Streams: An Algorithm for the (Text) Book.*

`arXiv:2301.10191v2`

**Source date:** 24 May 2023.

S1 định nghĩa \(F_0\), Problem 1, quy ước \(\log=\log_2\), Algorithm 1, threshold, Error/Fail, Claims 3–6 và Theorem 2. Footnote của Algorithm 1 cũng xác nhận claim cũ về joint independence là sai và \(|\mathcal X|/p\) không phải estimator unbiased nói chung. ([arXiv][1])

### S1 source boundary

Project sử dụng:

```text
Problem 1
Section 1
Algorithm 1
Algorithm 2
Claim 3
Claim 4
Claim 6
Theorem 2
Algorithm 1 correction note / footnote
```

Không sử dụng bibliographic remarks của S1 làm methodology.

---

## 1.2. S2 — Knuth

**S2**

Donald E. Knuth.

*The CVM Algorithm for Estimating Distinct Elements in Streams.*

Stanford Computer Science.

**Original:** 25 May 2023
**Revised:** 29 December 2023.

S2 định nghĩa Algorithm D, Lemma M, Algorithm D′ / A Coarser Method, Equation (13), các theorem/bounds liên quan và Appendix Algorithm X. Appendix chỉ ra bug của Algorithm X; thay `If |B|=s` bằng `While |B|=s` và loại failure tạo ra modified algorithm unbiased và equivalent to Algorithm D với tham số dịch một đơn vị. ([Computer Science][2])

---

## 1.3. Source restriction

Không source nào ngoài S1/S2 được phép thiết lập [T].

Các nguồn sau bị loại khỏi source-of-truth:

```text
Wikipedia
StackOverflow
blogs
tutorials
benchmark articles
secondary papers
LLM-generated explanations
```

External sources chỉ được sử dụng cho implementation engineering nếu cần và không được biến thành [T] trong research document.

---

# 2. RESEARCH IDENTITY AND SCOPE

## 2.1. Research objective

Repository nghiên cứu một họ thuật toán distinct-element streaming dựa trên CVM:

```text
Original CVM / Algorithm 1
        ↓
Theoretical capacity requirement
        ↓
Sampling/reduction dynamics
        ↓
Knuth corrected formulation
        ↓
Empirical source-conformance
        ↓
Matched-capacity comparison
```

Research không nhằm xây dựng survey toàn diện về cardinality estimation.

---

## 2.2. Core algorithms

| Project ID     | Source                               | Role                                  |
| -------------- | ------------------------------------ | ------------------------------------- |
| `CVM-A1`       | S1 Algorithm 1                       | Original CVM                          |
| `CVM-A1(s)`    | S1 + project parameterization        | Controlled threshold sweep            |
| `CVM-W`        | S2 Appendix Algorithm X + `If→While` | Non-aborting modified CVM             |
| `Knuth-D`      | S2 Algorithm D                       | Continuous-cutoff corrected algorithm |
| `Knuth-Dprime` | S2 A Coarser Method                  | Binary-cutoff corrected algorithm     |

`CVM-W` là **project alias**, không phải tên mà S2 dùng.

---

## 2.3. Critical parameter normalization

Để loại ambiguity giữa notation của Appendix và notation của project:

```text
W-threshold parameter:
    s_W

Matched retained capacity:
    k

Knuth-D buffer capacity:
    s_D
```

Project cố định:

$$
\boxed{
s_W=k+1,
\qquad
s_D=k.
}
$$

Vì W giữ invariant:

$$
|X|\le s_W-1=k
$$

sau mỗi update hoàn chỉnh.

S2 Appendix xác nhận modified Algorithm X equivalent to Algorithm D với tham số D tăng một đơn vị. Do đó project chuẩn hóa:

$$
\boxed{
W(k+1)\equiv D(k)
}
$$

Đây là **[T-D]**, không phải source theorem độc lập. ([Computer Science][2])

---

## 2.4. Absolute exclusions

Không nghiên cứu:

```text
HyperLogLog
KMV
Flajolet–Martin
PCSA
CVM-U
Nandi–Pal
Karayel
other CVM derivatives
Zipf workload
Shannon entropy
Spearman correlation
large benchmark matrices
cache locality
CPU affinity
SIMD/vectorization
p99/p99.9 latency
allocator benchmark
container benchmark
FlatBuffer vs HashSet
Iso-Memory experiment
```

Không có `Optional Extension` trong core plan.

---

# 3. CORE RESEARCH QUESTIONS

# RQ1 — Theoretical Capacity Bound vs Observed Threshold

## Research question

How conservative is the S1 theoretical threshold when compared with the smallest experimentally certified threshold under the controlled all-distinct workload?

### Objects

```text
Algorithm:
    CVM-A1(s)

Workload:
    WL-UNIFORM

m:
    {10^5, 10^6}

epsilon:
    {0.20, 0.10, 0.05}

delta:
    {0.10, 0.05, 0.01}

capacity:
    dyadic s-grid
```

### H1

For reportable configurations:

$$
\boxed{
\frac{s_{\mathrm{theo}}}
{s_{\mathrm{emp}}^{\mathrm{obs}}}
\ge 3
}
$$

H1 là empirical hypothesis / pre-specified effect criterion [H], không phải theorem.

---

# RQ2 — Internal CVM State Dynamics

Nghiên cứu evolution của:

$$
|X_t|,
\qquad
p_t,
\qquad
l_t,
\qquad
\Delta t.
$$

Workloads:

```text
WL-UNIFORM
WL-DUP
```

## H2a — Empirical scaling hypothesis

$$
E[\Delta t_l]\propto s2^l.
$$

Operational expectation:

$$
\boxed{
\beta_1\approx1,
\qquad
\beta_2\approx1,
\qquad
\beta_3\approx0
}
$$

Đây là **empirical scaling hypothesis** được motivated bởi reduction mechanism; S1/S2 không thiết lập trực tiếp \(\beta_1=1,\beta_2=1,\beta_3=0\).

## H2b

$$
\boxed{
l_{\mathrm{terminal}}
\approx
a+
b\log_2\left(\frac{F_0^*}{s}\right)
}
$$

Fit trên `WL-UNIFORM`, đánh giá out-of-sample trên `WL-DUP`.

Không có universality claim.

---

# RQ3 — Exact Semantics and Knuth Corrections

Kiểm tra implementation consistency đối với:

```text
CVM-W
Knuth-D
Knuth-Dprime
```

và:

```text
duplicate removal
cutoff transitions
buffer invariant
Lemma M
W–D relation
D–D′ cutoff bracket
source fixtures
finite-rank policy
```

Các source-level properties là [T]/[T-D].

---

# RQ4 — Matched Retained Capacity Comparison

So sánh:

```text
CVM-A1
CVM-W
Knuth-D
Knuth-Dprime
```

với:

$$
\boxed{
k=
\text{maximum retained buffer size at an update boundary}
}
$$

Metrics:

```text
total algorithmic approximation failure probability
conditional/unconditional bias
variance
NRMSE
throughput
```

---

# 4. SCIENTIFIC MODEL AND NOTATION

## 4.1. Stream

$$
\mathcal A
=
\langle a_1,\ldots,a_m\rangle,
\qquad
a_t\in[n].
$$

Distinct cardinality:

$$
F_0^*
=
\left|
\{a_1,\ldots,a_m\}
\right|.
$$

S1 định nghĩa \(F_0\) là số phần tử distinct của stream. ([arXiv][1])

---

## 4.2. Output

$$
\hat F_0\in[0,\infty)\cup\{\bot\}.
$$

\(\bot\) là control-flow outcome, không phải numerical observation.

---

## 4.3. A1/W state

$$
X_t\subseteq[n].
$$

Project invariant:

$$
|X_t|\le s
$$

ở state boundary.

For W after completed update:

$$
\boxed{
|X_t|\le s-1.
}
$$

---

## 4.4. D/D′ state

$$
B_t\subseteq[n]\times[0,1).
$$

Mỗi key xuất hiện tối đa một lần trong buffer.

---

## 4.5. Sampling rate

For A1/W:

$$
p_t=2^{-l_t},
\qquad
l_t\in\mathbb N_0.
$$

For D:

$$
p_t\in(0,1].
$$

For D′:

$$
p_t=2^{-l_t}.
$$

---

## 4.6. Relative error

For successful numeric output and \(F_0^*>0\):

$$
\mathrm{RE}
=
\frac{\hat F_0-F_0^*}{F_0^*}.
$$

For non-successful runs:

$$
\mathrm{RE}=\mathrm{NA}.
$$

---

# 5. OUTPUT STATUS MODEL

## 5.1. Status enum

```text
Success
Abort
NumericFailure
ResourceFailure
```

---

## 5.2. Status invariant

| Status          | Estimate |
| --------------- | -------- |
| Success         | present  |
| Abort           | absent   |
| NumericFailure  | absent   |
| ResourceFailure | absent   |

CSV:

```text
status=Success
    estimate=<finite number>
```

```text
status!=Success
    estimate=null
```

Không sentinel:

```text
-1
NaN
0
INF
```

được sử dụng để encode failure.

---

## 5.3. Scientific validity of failed runs

```text
Abort:
    algorithmic/control-flow failure

NumericFailure:
    implementation produced an invalid numerical state

ResourceFailure:
    execution could not complete due to external/system resource limits
```

### Execution-valid definition

$$
\boxed{
execution\_valid
\iff
status\in\{Success,Abort\}
}
$$

### Approximation-valid definition

Với một tolerance \(\varepsilon\):

```text
approximation_valid = true
    iff status=Success and RE <= epsilon

approximation_valid = false
    iff status=Abort
       or status=Success and RE > epsilon
```

`NumericFailure` và `ResourceFailure` không được gán là approximation failure.

---

## 5.4. Abort semantics

Chỉ `CVM-A1` có source-level `Abort`.

A1 abort khi sau reduction:

$$
|X|=s.
$$

S1 Algorithm 1 ghi rõ sequence này: threshold reached → halving → nếu vẫn đầy thì output \(\bot\). ([arXiv][1])

W/D/D′ không có source-level abort.

---

# 6. SOURCE-CORRECT THEORETICAL BOUND

S1 Algorithm 1 sử dụng:

$$
\boxed{
s_{\mathrm{theo}}
=
\left\lceil
\frac{12}{\varepsilon^2}
\log_2\left(\frac{8m}{\delta}\right)
\right\rceil
}
$$

S1 quy ước \(\log=\log_2\). ([arXiv][1])

---

## 6.1. Grid

$$
m\in\{10^5,10^6\}
$$

$$
\varepsilon\in\{0.2,0.1,0.05\}
$$

$$
\delta\in\{0.1,0.05,0.01\}.
$$

---

## 6.2. Exact theoretical table

| \(\varepsilon\) | \(\delta\) |    \(m\) | \(s_{\mathrm{theo}}\) |
| --------------: | ---------: | -------: | --------------------: |
|            0.20 |       0.10 | \(10^5\) |                 6,880 |
|            0.20 |       0.10 | \(10^6\) |                 7,877 |
|            0.20 |       0.05 | \(10^5\) |                 7,180 |
|            0.20 |       0.05 | \(10^6\) |                 8,177 |
|            0.20 |       0.01 | \(10^5\) |                 7,877 |
|            0.20 |       0.01 | \(10^6\) |                 8,873 |
|            0.10 |       0.10 | \(10^5\) |                27,518 |
|            0.10 |       0.10 | \(10^6\) |                31,505 |
|            0.10 |       0.05 | \(10^5\) |                28,718 |
|            0.10 |       0.05 | \(10^6\) |                32,705 |
|            0.10 |       0.01 | \(10^5\) |                31,505 |
|            0.10 |       0.01 | \(10^6\) |                35,491 |
|            0.05 |       0.10 | \(10^5\) |               110,072 |
|            0.05 |       0.10 | \(10^6\) |               126,017 |
|            0.05 |       0.05 | \(10^5\) |               114,872 |
|            0.05 |       0.05 | \(10^6\) |               130,817 |
|            0.05 |       0.01 | \(10^5\) |               126,017 |
|            0.05 |       0.01 | \(10^6\) |           **141,963** |

Therefore:

$$
\boxed{
\max s_{\mathrm{theo}}=141,963
}
$$

Bảng phải được sinh programmatically và regression-tested.

---

# 7. EXACT ALGORITHM SEMANTICS

## 7.1. CVM-A1

`CVM-A1(s)` là project parameterization của S1 Algorithm 1.

State:

```text
X = empty
p = 1
l = 0
```

For each \(a_t\):

### Step A1.1

$$
X\leftarrow X\setminus\{a_t\}.
$$

### Step A1.2

Insert with probability \(p\).

### Step A1.3

If:

$$
|X|=s,
$$

independently discard each element with probability \(1/2\).

### Step A1.4

$$
p\leftarrow p/2,
\qquad
l\leftarrow l+1.
$$

### Step A1.5

If:

$$
|X|=s,
$$

return:

$$
\bot.
$$

### Final

$$
\boxed{
\hat F_0=\frac{|X|}{p}
}
$$

S1 Algorithm 1 xác nhận trực tiếp transition structure này. ([arXiv][1])

---

# 7.2. CVM-W

`CVM-W` là modified Algorithm X từ Appendix của S2.

Control structure:

```text
if |X| = s
```

được thay bằng:

```text
while |X| = s
```

với:

```text
independently discard each retained element with probability 1/2
p <- p/2
l <- l+1
```

Cho:

$$
s\ge2
$$

sau mỗi update:

$$
\boxed{
|X|\le s-1
}
$$

Không có abort.

S2 Appendix nói modified Algorithm X trở thành unbiased và equivalent to Algorithm D khi đổi tham số \(s\) sang \(s+1\). ([Computer Science][2])

Do đó:

$$
\boxed{
W(k+1)\equiv D(k)
}
$$

theo project parameter normalization.

---

# 7.3. Knuth-D

S2 Algorithm D dùng buffer tối đa \(s\) ordered pairs \((a,u)\), \(u\in[0,1)\). Lemma M mô tả current latest occurrence và cutoff; D dùng dictionary + priority-queue semantics cho buffer. ([Computer Science][2])

### D1

$$
t\leftarrow0,
\qquad
p\leftarrow1,
\qquad
B\leftarrow\emptyset.
$$

### D2

Nếu:

$$
t=m,
$$

trả:

$$
\boxed{
\hat F_0=\frac{|B|}{p}
}
$$

### D3

Đọc next stream key \(a_t\).

### D4

Nếu current key tồn tại trong \(B\), xóa pair cũ.

### D5

Sinh fresh uniform deviate \(u\).

Nếu:

$$
u\ge p,
$$

không insert.

Nếu:

$$
u<p
\quad\land\quad
|B|<s,
$$

insert \((a,u)\).

### D6

Nếu:

$$
u<p
\quad\land\quad
|B|=s,
$$

xác định maximum stored volatility \(u_{\max}\).

Nếu:

$$
u>u_{\max},
$$

set:

$$
p\leftarrow u.
$$

Incoming key không được lưu.

Ngược lại:

$$
B
\leftarrow
(B\setminus\{(a',u_{\max})\})
\cup\{(a,u)\}
$$

và:

$$
p\leftarrow u_{\max}.
$$

---

# 7.4. Knuth-D source theorem

S2 phát biểu Algorithm D trả về unbiased estimate cho arbitrary stream với positive buffer size \(s\). Đây là:

```text
[T]
```

cho **ideal mathematical Algorithm D**.

Không được chuyển thành:

```text
64-bit implementation is exactly unbiased
```

---

# 7.5. Lemma M

Với key \(a\), gọi \(j\) là latest occurrence index của \(a\) tới state boundary đang xét.

S2 Lemma M:

$$
\boxed{
(a,u_j)\in B
\iff
u_j<p_t
}
$$

ở D2. ([Computer Science][2])

Đây là source-level invariant của D/D′.

Không áp dụng Lemma M cho A1/W.

---

# 7.6. Knuth-Dprime

D′ giữ D1–D5 và thay D6 bởi binary cutoff.

Tại D6′:

```text
u < p
|B| = s
```

thực hiện:

1. Remove mọi retained pair thỏa:

   $$
   u'\ge p/2.
   $$

2. Set:

   $$
   p\leftarrow p/2.
   $$

3. Nếu buffer không thay đổi và incoming vẫn thỏa:

   $$
   u<p_{\rm new},
   $$

   repeat.

4. Nếu cần, insert iff:

   $$
   u<p_{\rm new}.
   $$

D′ duy trì:

$$
\boxed{
p=2^{-l}
}
$$

và S2 nói Lemma M vẫn đúng, nên estimator vẫn unbiased trong source model. ([Computer Science][2])

---

# 8. W–D SOURCE RELATIONSHIP

S2 Appendix ghi rõ modified Algorithm X với `While |B| = s` và no failure equivalent to Algorithm D với tham số D tăng một đơn vị. ([Computer Science][2])

SSOT normalization:

$$
\boxed{
W(k+1)\equiv D(k)
}
$$

hay:

$$
\boxed{
W(s_W)\equiv D(s_W-1)
}
\qquad
s_W\ge2.
$$

Label:

```text
[T-D]
```

**Không được viết:**

$$
W(k+1)\equiv D'(k).
$$

D′ là binary/coarser version của D, không phải source-equivalent version của W.

---

# 9. MATCHED RETAINED CAPACITY

Định nghĩa:

$$
\boxed{
k=
\text{maximum retained buffer size at an update boundary}
}
$$

là [M].

Mapping:

| Algorithm      | Parameter |
| -------------- | --------: |
| `CVM-A1`       |   \(s=k\) |
| `CVM-W`        | \(s=k+1\) |
| `Knuth-D`      |   \(s=k\) |
| `Knuth-Dprime` |   \(s=k\) |

Sau completed update:

$$
|W|\le k.
$$

Nhưng W có thể có transient occupancy:

$$
|X|=k+1
$$

trước reduction.

Do đó report separately:

```text
retained_capacity
peak/transient_occupancy
```

Không gọi đây là byte-equivalent memory.

---

# 10. D ↔ D′ CUTOFF RELATION

S2 Equation (13):

$$
\boxed{
p_{\mathrm{bin}}\le p<2p_{\mathrm{bin}}
}
$$

trong coupled computation. ([Computer Science][2])

Project distinction:

```text
source mathematical bracket = [T]
finite implementation bracket = [M] exact conformance predicate
```

Pass implementation test không biến [M] thành [T].

## 10.1. Exact finite-model bracket representation

For D:

```text
Cutoff64 = ONE | RANK(c)
```

For D′:

```text
level = l
p_bin = 2^(-l)
```

### D = ONE

$$
p_D=1.
$$

Exact bracket condition:

$$
\boxed{l=0}
$$

### D = RANK(c)

Define:

$$
q=2c+1.
$$

Then:

$$
p_D=\frac{q}{2^{65}}.
$$

The exact bracket condition is:

$$
2^{-l}\le\frac{q}{2^{65}}<2^{1-l}.
$$

Because \(q\) is odd, define:

$$
e=\lfloor\log_2 q\rfloor.
$$

Then the unique matching binary level is:

$$
\boxed{
l=65-e.
}
$$

Therefore production conformance predicate is:

```text
D = ONE:
    Dprime.level == 0

D = RANK(c):
    q = 2*c + 1
    e = floor(log2(q))
    expected_level = 65 - e
    Dprime.level == expected_level
```

Không convert `p_D` hoặc \(p_{\rm bin}\) sang `double`.

---

# 11. FINITE-PRECISION RANDOMNESS POLICY

## 11.1. Separation

### Mathematical source model

```text
ideal Bernoulli sampling
ideal continuous uniform volatility
```

### Production realization

```text
deterministic PRNG
finite-width rank
deterministic tie semantics
```

Therefore:

$$
\boxed{
\text{ideal algorithm}
\neq
\text{finite production realization}
}
$$

trừ khi có proof độc lập.

---

## 11.2. PRNG

Production:

```text
seed mixer:
    SplitMix64

generator:
    xoshiro256**
```

Mọi randomness-consuming code phải đi qua project RNG interface.

Không algorithm nào được tự tạo RNG riêng trong hot path.

---

# 12. A1/W RANDOM SAMPLING

## 12.1. Bernoulli \(2^{-l}\)

Không dùng:

```cpp
double u;
u < std::pow(2.0, -l);
```

Use:

```text
BernoulliPow2(l)
```

### Semantics

Nếu:

$$
l=0,
$$

return true.

Nếu:

$$
l>0,
$$

consume \(l\) fair pseudo-random bits và accept iff mọi bit đều là zero.

Do đó xác suất lý tưởng:

$$
\boxed{
2^{-l}
}
$$

Không có:

```text
l <= 52
```

limitation.

---

# 13. D/D′ FINITE VOLATILITY REPRESENTATION

## 13.1. Rank

Production generates:

$$
r\in\{0,\ldots,2^{64}-1\}.
$$

Conceptually:

$$
\boxed{
u_r=
\frac{r+1/2}{2^{64}}
}
$$

Algorithm hot path chỉ sử dụng integer comparison.

Không convert sang `double` cho acceptance/cutoff decisions.

---

# 14. EXACT FINITE CUT-OFF MODEL

Đây là phần **machine-exact**.

## 14.1. D cutoff representation

Define:

```text
Cutoff64
```

với hai trạng thái:

```text
ONE
RANK(c)
```

### `ONE`

Đại diện:

$$
p=1.
$$

Mọi rank được accept.

### `RANK(c)`

Đại diện:

$$
p=
\frac{c+1/2}{2^{64}}.
$$

Acceptance predicate:

$$
\boxed{
u_r<p
\iff
r<c
}
$$

Hot path:

```text
incoming_rank < cutoff_rank
```

---

## 14.2. D finite cutoff transition

Khi buffer full và incoming rank \(r\) được accept:

Gọi \(M\) là maximum stored rank.

### Case 1

$$
r>M.
$$

Set:

```text
cutoff = RANK(r)
```

Incoming key không được lưu.

### Case 2

$$
r<M.
$$

Set:

```text
cutoff = RANK(M)
```

Purge every stored rank:

$$
r_{\rm stored}\ge M.
$$

Sau đó insert incoming rank vì:

$$
r<M.
$$

### Case 3 — finite tie

$$
r=M.
$$

Set:

```text
cutoff = RANK(M)
```

Purge every stored rank:

$$
r_{\rm stored}\ge M.
$$

Incoming rank bằng cutoff nên bị reject.

Đây là **[M] finite-rank realization**, không phải assertion rằng source continuous-volatility Algorithm D có tie semantics này.

---

# 15. FINITE-RANK TIE SEMANTICS

## 15.1. Tie events

Record:

```text
rank_collision_events
cutoff_tie_events
equal_max_rank_events
```

Không silent drop.

---

## 15.2. Strict cutoff invariant

Production D/D′:

$$
\boxed{
r_{\mathrm{stored}}<r_{\mathrm{cutoff}}
}
$$

sau mỗi state normalization.

---

## 15.3. Multiple equal maxima

Nếu:

```text
r1 = r2 = ... = rmax
```

thì purge tất cả:

$$
r\ge r_{\max}.
$$

Không chỉ remove một entry.

---

## 15.4. Incoming equals cutoff

Nếu:

$$
r_{\rm incoming}=r_{\rm cutoff},
$$

reject.

Record:

```text
cutoff_tie_events += 1
```

---

## 15.5. Deterministic ordering

Bookkeeping order:

```text
(rank, insertion_serial)
```

Serial number chỉ phục vụ deterministic container behavior.

Scientific acceptance vẫn là:

$$
r<r_{\rm cutoff}.
$$

Serial không được thay đổi acceptance.

---

# 16. D′ EXACT FINITE-CUTOFF SEMANTICS

Với:

$$
p=2^{-l},
$$

production acceptance:

$$
\frac{r+1/2}{2^{64}}<2^{-l}.
$$

Do đó:

### \(l=0\)

mọi rank được accept.

### \(1\le l\le64\)

$$
\boxed{
r<2^{64-l}
}
$$

### \(l\ge65\)

Không rank nào được accept.

Project representation:

```text
Dprime cutoff = integer exponent l
```

không reconstruct \(p\) bằng floating point.

---

# 17. FINITE-RANK RESOLUTION POLICY

## 17.1. D

`FINITE_RANK_RESOLUTION_EVENT` được record khi:

$$
\boxed{
\mathrm{Cutoff}=RANK(0)
}
$$

vì không còn rank nào thỏa:

$$
r<0.
$$

---

## 17.2. D′

Event xảy ra khi transition would move:

$$
l=64\rightarrow65.
$$

Khi:

$$
l\ge65,
$$

mọi future rank bị reject.

---

## 17.3. Scientific treatment

Resolution event:

```text
does not silently fail
does not clamp
does not restart RNG
does not replace estimate with zero
```

Nó được record và đưa vào sensitivity analysis.

Nếu event xuất hiện trong một primary configuration ở mức không bỏ qua được, configuration phải được đánh dấu:

```text
FINITE_RANK_RESOLUTION_EVENT
```

và không được diễn giải như clean implementation evidence.

---

# 18. FINITE-PRECISION SEMANTIC BOUNDARY

### Source mathematical property

$$
E[\hat F_0]=F_0^*
$$

cho ideal D/D′.

### Production realization

```text
64-bit finite-rank implementation
```

là [M].

Do đó không được viết:

```text
64-bit D/Dprime is exactly unbiased
```

mà chỉ:

```text
empirically consistent with source unbiasedness
```

nếu dữ liệu hỗ trợ.

---

# 19. EXACT ORACLE MODE

Repository có:

```text
PRODUCTION MODE
ORACLE MODE
```

---

## 19.1. Production Mode

```text
SplitMix64
xoshiro256**
64-bit finite ranks
real benchmark execution
```

---

## 19.2. Oracle Mode

Supports:

```text
explicit fair-bit trees
explicit volatility permutations
symbolic order
exact state transitions
```

Dùng cho:

```text
F1
F2
F3
F4
F5
Lemma M
D/Dprime bracket
small-state conformance
```

Không benchmark throughput ở Oracle Mode.

---

# 20. VALID PARAMETER DOMAINS

| Algorithm         | Valid project domain |
| ----------------- | -------------------: |
| `CVM-A1(s)`       |            \(s\ge2\) |
| `CVM-W(s)`        |            \(s\ge2\) |
| `Knuth-D(s)`      |            \(s\ge1\) |
| `Knuth-Dprime(s)` |            \(s\ge1\) |

A1/W \(s=1\) excluded from main scientific matrices.

D/D′ \(s=1\) retained as source/edge analysis.

---

# 21. CONTROLLED WORKLOADS

Exactly two workloads.

## 21.1. WL-UNIFORM

$$
a_t=t,
\qquad
1\le t\le m.
$$

Therefore:

$$
F_0^*=m,
\qquad
n=m.
$$

---

## 21.2. WL-DUP

Require even \(m\).

First half:

$$
a_t=t,
\qquad
1\le t\le m/2.
$$

Second half:

$$
a_t=t-m/2,
\qquad
m/2<t\le m.
$$

Therefore:

$$
\boxed{
F_0^*=m/2
}
$$

and:

$$
n=m/2.
$$

No stream randomness.

Không `stream_seed`.

---

# 22. WORKLOAD METADATA

Every run records:

```text
workload
m
universe_n
f0_true
```

Controlled values:

```text
WL-UNIFORM:
    universe_n = m
    f0_true = m

WL-DUP:
    universe_n = m/2
    f0_true = m/2
```

---

# 23. METRIC DEFINITIONS

## 23.1. Track A — Total algorithmic approximation failure

For `Success`:

$$
E_i(\varepsilon)
=
\mathbf 1
\left[
|\hat F_{0,i}-F_0^*|
>
\varepsilon F_0^*
\right].
$$

For `Abort`:

$$
E_i(\varepsilon)=1.
$$

For `NumericFailure` hoặc `ResourceFailure`:

```text
E_i = NA
```

Define:

$$
N_{\mathrm{exec}}
=
N_{\mathrm{Success}}
+
N_{\mathrm{Abort}}.
$$

Primary algorithmic failure estimand:

$$
\boxed{
q_{\mathrm{total,alg}}
=
\frac{
N_{\mathrm{Abort}}
+
N_{\mathrm{Success,out\_of\_tolerance}}
}{
N_{\mathrm{exec}}
}
}
$$

Đây là failure probability **conditional on execution-valid trials**.

Không dùng `q_total_alg` để đánh đồng execution failure với algorithmic approximation error.

---

## 23.2. Execution invalid rate

$$
\boxed{
q_{\mathrm{execution\_invalid}}
=
\frac{
N_{\mathrm{NumericFailure}}
+
N_{\mathrm{ResourceFailure}}
}{R}
}
$$

Report:

```text
NumericFailure count
ResourceFailure count
execution_invalid_rate
```

---

## 23.3. Full intended-run diagnostic

Không sử dụng một metric duy nhất gộp:

```text
Abort
NumericFailure
ResourceFailure
approximation failure
```

thành “error probability”.

Mỗi loại được report separately.

---

## 23.4. Abort rate

$$
\boxed{
q_{\mathrm{abort}}
=
\frac{N_{\mathrm{Abort}}}{R}
}
$$

với full intended trial count \(R\).

---

## 23.5. A1 conditional estimand

$$
\mathcal I_{A1}
=
\{i:\mathrm{status}_i=\mathrm{Success}\}.
$$

$$
R_{\mathrm{succ}}=|\mathcal I_{A1}|.
$$

Conditional bias:

$$
\widehat{\mathrm{Bias}}_{\mathrm{rel},A1}
=
\frac1{R_{\mathrm{succ}}}
\sum_{i\in\mathcal I_{A1}}
\mathrm{RE}_i.
$$

Nếu:

$$
R_{\mathrm{succ}}=0,
$$

estimand là:

```text
undefined
```

không được thay bằng zero.

---

## 23.6. W/D/D′ total estimands

Với:

$$
\mathcal I_{\mathrm{Success}}
=
\{i:\mathrm{status}_i=\mathrm{Success}\},
$$

mọi estimator metric sau đây được tính trên \(\mathcal I_{\mathrm{Success}}\):

$$
\widehat{\mathrm{Bias}}_{\mathrm{rel}}
=
\frac1{N_{\mathrm{succ}}}
\sum_i\mathrm{RE}_i.
$$

$$
\widehat{\operatorname{Var}}_{\mathrm{rel}}
=
\frac1{N_{\mathrm{succ}}-1}
\sum_i
(\mathrm{RE}_i-\overline{\mathrm{RE}})^2.
$$

$$
\boxed{
\mathrm{NRMSE}
=
\sqrt{
\frac1{N_{\mathrm{succ}}}
\sum_i \mathrm{RE}_i^2
}
}
$$

Nếu \(N_{\mathrm{succ}}<2\), variance/TOST-related estimands được đánh dấu `undefined`.

---

# 24. CROSS-ESTIMAND PROHIBITION

Không rank:

```text
A1 conditional bias
```

trực tiếp với:

```text
W unconditional bias
D unconditional bias
Dprime unconditional bias
```

Total algorithmic failure `q_total_alg` là estimand chung hợp lệ.

---

# 25. OBSERVABLE VARIABLES

Every run:

```text
status
execution_valid
approximation_valid
approximation_failure
estimate
estimate_log2

final_buffer_size
peak_buffer_size

sampling_rate_final
sampling_rate_repr
halving_count
reduction_rounds

dup_removals

tie_events
cutoff_tie_events
equal_max_rank_events
rank_collision_events
resolution_floor_hits

throughput_items_per_sec
allocated_state_bytes
```

Experimental metadata:

```text
run_id
experiment_id
phase
algorithm
workload

m
universe_n
f0_true

epsilon
delta

algorithm_param_s
matched_capacity_k

trial_index
trial_seed
master_seed

coupling_mode
coupling_level

theoretical_capacity
```

Provenance:

```text
git_commit
source_s1_version
source_s2_version

compiler_version
build_flags
os_name
cpu_model

binary_hash
source_tree_hash
config_hash
analysis_environment

workload_generator_version
schema_version
```

---

# 26. RQ1 EXPERIMENTAL DESIGN

## 26.1. Stream sizes

$$
m\in\{10^5,10^6\}.
$$

## 26.2. Error parameters

$$
\varepsilon\in\{0.20,0.10,0.05\}.
$$

$$
\delta\in\{0.10,0.05,0.01\}.
$$

## 26.3. Capacity grid

$$
\mathcal S=\{2^6,\ldots,2^{18}\}.
$$

Explicit:

```text
64
128
256
512
1024
2048
4096
8192
16384
32768
65536
131072
262144
```

13 points.

## 26.4. Replicates

$$
\boxed{
R=1000
}
$$

per fixed:

$$
(m,\varepsilon,\delta,s).
$$

---

# 27. RQ1 INFERENCE FAMILY

One inference family:

$$
\boxed{
(m,\varepsilon,\delta)
}
$$

with 13 \(s\)-points.

Set:

$$
\alpha_{\mathrm{grid}}
=
\frac{0.05}{13}.
$$

Multiplicity correction chỉ áp dụng trong 13-point family.

Không có global Bonferroni over all 18 families.

---

# 28. RQ1 CLOPPER–PEARSON

For \(K\) algorithmic failures among \(N_{\mathrm{exec}}\):

$$
\hat q=\frac{K}{N_{\mathrm{exec}}}.
$$

For:

$$
0<K<N_{\mathrm{exec}},
$$

$$
q_U
=
\operatorname{Beta}^{-1}
\left(
1-\alpha_{\mathrm{grid}};
K+1,N_{\mathrm{exec}}-K
\right).
$$

$$
q_L
=
\operatorname{Beta}^{-1}
\left(
\alpha_{\mathrm{grid}};
K,N_{\mathrm{exec}}-K+1
\right).
$$

---

## 28.1. Endpoint conventions

If:

$$
K=0,
$$

$$
q_L=0.
$$

If:

$$
K=N_{\mathrm{exec}},
$$

$$
q_U=1.
$$

For \(K=0\):

$$
\boxed{
q_U=
1-\alpha_{\mathrm{grid}}^{1/N_{\mathrm{exec}}}
}
$$

For \(K=N_{\mathrm{exec}}\):

$$
\boxed{
q_L=
\alpha_{\mathrm{grid}}^{1/N_{\mathrm{exec}}}
}
$$

Không gọi Beta inverse với zero shape parameter.

Nếu:

$$
N_{\mathrm{exec}}=0,
$$

RQ1 cell:

```text
INCONCLUSIVE
```

---

# 29. RQ1 CP DECISION RULE

For each \(s\):

```text
CERTIFIED
    q_U <= delta

REJECTED
    q_L > delta

INCONCLUSIVE
    otherwise
```

Các quyết định này chỉ được thực hiện trên `execution_valid` trials.

---

# 30. RQ1 SAMPLE-SIZE REFERENCE

Với:

$$
R=1000,
\qquad
\alpha_{\mathrm{grid}}=0.05/13,
$$

nếu không có execution failure:

| \(K\) |  \(q_U\) |
| ----: | -------: |
|     0 | 0.005545 |
|     1 | 0.007701 |
|     2 | 0.009561 |
|     3 | 0.011277 |

Do đó:

$$
K\le2
$$

có thể certify:

$$
q\le0.01.
$$

Đây là demonstration của certification resolution, không phải power analysis.

---

# 31. EMPIRICAL THRESHOLD

$$
\boxed{
s_{\mathrm{emp}}^{\mathrm{obs}}
=
\min
\{
s\in\mathcal S:
s\text{ is CERTIFIED}
\}
}
$$

Đây là:

```text
grid-specific
protocol-specific
workload-specific
observed
```

Không phải universal minimum.

---

# 32. RQ1 THRESHOLD CLASSIFICATION

`threshold_state` và `non_monotone_flag` được xác định riêng.

## 32.1. Non-monotone flag

$$
\boxed{
non\_monotone\_flag=true
}
$$

iff tồn tại:

$$
s_i<s_j
$$

sao cho:

```text
decision(s_i) = CERTIFIED
decision(s_j) = REJECTED
```

`INCONCLUSIVE` không tự tạo non-monotonicity.

---

## 32.2. `IDENTIFIED`

First certified point:

```text
previous grid point = REJECTED
```

và:

```text
no later grid point = REJECTED
```

---

## 32.3. `STATISTICALLY_INCONCLUSIVE`

First certified point tồn tại nhưng previous grid point:

```text
INCONCLUSIVE
```

hoặc không đủ evidence để xác định threshold bằng protocol.

---

## 32.4. `LEFT-CENSORED`

$$
s=64
$$

đã `CERTIFIED`.

---

## 32.5. `RIGHT-CENSORED`

Không grid point nào `CERTIFIED`.

---

# 33. H1 REPORTABILITY

H1 chỉ reportable nếu đồng thời:

```text
threshold_state = IDENTIFIED
non_monotone_flag = false
q_execution_invalid = 0
```

Khi đó:

$$
R_s
=
\frac{s_{\mathrm{theo}}}
{s_{\mathrm{emp}}^{\mathrm{obs}}}.
$$

Primary criterion:

$$
\boxed{
R_s\ge3
}
$$

H1 conclusion không được đưa ra cho:

```text
LEFT-CENSORED
RIGHT-CENSORED
STATISTICALLY_INCONCLUSIVE
non_monotone_flag=true
execution_invalid > 0
```

\(R_s\) được report như **observed conservatism ratio**, không xem là parameter có universal statistical distribution.

---

# 34. THEORY-CONSISTENCY ALERT

Với mỗi family, locate:

$$
s\ge s_{\mathrm{theo}}.
$$

Chỉ thực hiện theory-consistency decision nếu:

```text
NumericFailure = 0
ResourceFailure = 0
```

Nếu một grid point thỏa:

```text
s >= s_theo
decision = REJECTED
```

raise:

```text
THEORY_CONSISTENCY_ALERT
```

Alert không được mô tả là:

```text
refutation of Theorem 2
```

Nếu execution invalid > 0:

```text
THEORY_CONSISTENCY_CHECK_BLOCKED
```

thay vì tạo substantive theory alert.

Debug order:

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

S1 Theorem 2 là guarantee của source algorithm, không phải guarantee của production finite-rank realization. ([arXiv][1])

---

# 35. RQ2 EXPERIMENTAL DESIGN

Algorithms:

```text
CVM-A1
CVM-W
```

Capacities:

$$
k\in\{64,256,1024,4096\}.
$$

Mapping:

```text
A1: s=k
W : s=k+1
```

Workloads:

```text
WL-UNIFORM
WL-DUP
```

Stream lengths:

$$
m\in\{10^5,10^6\}.
$$

Replicates:

$$
R=20.
$$

---

# 36. RQ2 STATE TRACE

`state_trace.csv`:

```text
run_id
step_t

buffer_occupancy

sampling_rate_p
sampling_rate_repr
halving_count

status
```

Exact trace requirement:

```text
always record t=0
always record t=m
and additionally every ceil(m/100) steps
```

$$
\mathrm{trace\_interval}
=
\left\lceil\frac m{100}\right\rceil.
$$

`sampling_rate_repr` là source-of-truth exact representation.

`sampling_rate_p` là optional numeric rendering phục vụ plotting/inspection.

Trace sampling không thay thế exact reduction-event logging.

---

# 37. REDUCTION EPISODE SEMANTICS

Một `reduction_episode` là một stream input step \(t\) trong đó ít nhất một halving được thực hiện.

Episode:

```text
step_t
p_before
p_after
level_before
level_after
halvings_in_episode
```

---

## 37.1. Halving transition

Mỗi actual halving là một:

```text
halving_transition
```

Nếu một input item gây 3 halvings:

```text
one episode
three transitions
```

---

## 37.2. Same-step cascade

Các transitions có cùng:

$$
t_j=t_{j-1}
$$

được gọi là:

```text
same-step cascade
```

Do đó:

$$
\Delta t=0
$$

và:

```text
delta_t_valid=false
```

Không biến \(0\) thành \(1\).

---

# 38. REDUCTION EVENT SCHEMA

`reduction_events.csv`:

```text
run_id
episode_index
transition_index
step_t

level_before
level_after

p_before
p_after

halvings_in_episode
cascade_index

delta_t_since_previous_transition
delta_t_valid
```

---

# 39. H2a LEVEL CONVENTION

$$
\boxed{
l_j=
\text{halving level immediately before transition }j
}
$$

Do đó:

$$
p_{\mathrm{before}}
=
2^{-l_j}.
$$

Sau transition:

$$
l_j+1.
$$

Convention này bắt buộc nhất quán trong:

```text
CSV
regression
plots
README
notebook
```

---

# 40. H2a REGRESSION

Primary model:

$$
\boxed{
\log_2\Delta t
=
\beta_0
+\beta_1l
+\beta_2\log_2s
+\beta_3(l\log_2s)
+e
}
$$

Target expectations:

$$
\boxed{\beta_1=1}
$$

$$
\boxed{\beta_2=1}
$$

$$
\boxed{\beta_3=0}
$$

Interpretation:

```text
beta1:
    level scaling

beta2:
    capacity scaling

beta3:
    interaction/superlinear departure
```

Không dùng:

```text
CI contains target
```

như proof.

Report:

```text
point estimate
95% bootstrap CI
deviation from target
```

---

# 41. H2a STRATIFICATION

Primary fits:

```text
WL-UNIFORM, m=10^5
WL-UNIFORM, m=10^6
WL-DUP,     m=10^5
WL-DUP,     m=10^6
```

Sensitivity pooled model:

$$
\log_2\Delta t
=
\beta_0
+\beta_1l
+\beta_2\log_2s
+\beta_3l\log_2s
+\gamma_m
+\gamma_w.
$$

Pooled model không phải primary.

---

# 42. H2a RESAMPLING

Events within a run are clustered.

Primary bootstrap unit:

$$
\boxed{
\text{complete run}
}
$$

Procedure:

```text
sample runs with replacement
rebuild all event records from sampled runs
refit model
store coefficients
```

Use:

```text
95% bootstrap percentile CI
```

cho:

```text
beta1
beta2
beta3
```

Bootstrap target:

$$
\boxed{
B_{\mathrm{boot}}=10,000
}
$$

**valid bootstrap replicates** per primary coefficient fit.

### Bootstrap validity policy

A bootstrap replicate là invalid nếu:

```text
regression statistic undefined
insufficient sampled runs
non-finite fitted coefficient
```

Không impute.

Invalid bootstrap replicate được loại khỏi statistic set.

Target:

```text
B_boot valid replicates
```

Maximum resampling attempts:

```text
2 * B_boot
```

Nếu không đạt đủ valid replicates:

```text
bootstrap_status = INCONCLUSIVE
```

và CI không được sinh.

Report:

```text
B_valid
B_invalid
resampling_attempts
```

---

# 43. H2a CASCADE ANALYSIS

Report:

```text
number of reduction episodes
number of halving transitions
number of multi-halving episodes
fraction of episodes with halvings_in_episode > 1
distribution of cascade depth
```

Cascade events không đưa vào log-inter-event regression.

---

# 44. H2b TERMINAL LEVEL

$$
l_{\mathrm{terminal}}
=
\text{value of }l\text{ after the last completed halving}.
$$

### W

Always a valid final state.

### A1 Success

Final halving count.

### A1 Abort

$$
l_{\mathrm{terminal}}
=
\text{halving count after abort-triggering halving}.
$$

Record:

```text
terminal_state_type=Abort
```

Do not treat it as successful-estimator state.

---

# 45. H2b PRIMARY ANALYSIS

Primary:

```text
CVM-W
WL-UNIFORM
```

Fit:

$$
\boxed{
l_{\mathrm{terminal}}
=
a+
b\log_2
\left(
\frac{F_0^*}{s}
\right)
+e
}
$$

A1:

```text
success
abort
```

reported separately as diagnostic sensitivity analysis.

---

# 46. H2b OUT-OF-SAMPLE

Train:

```text
WL-UNIFORM
```

Evaluate:

```text
WL-DUP
```

Report:

```text
MAE
RMSE
mean residual
median residual
bootstrap CI
residual distribution
```

Bootstrap unit remains complete run.

No:

```text
universal law
distribution-free law
all-stream law
```

claim.

---

# 47. DUPLICATE SEMANTICS

For A1/W:

```text
remove current key
then process new occurrence
```

For D/D′, latest-occurrence volatility is central to Lemma M. ([Computer Science][2])

Forbidden:

```text
all actual A1 membership indicators are jointly independent
```

S1 explicitly says the old claim was erroneous. ([arXiv][1])

---

# 48. RQ3-MAIN

$$
k\in\{64,512,4096\}.
$$

Algorithms:

```text
A1(k)
W(k+1)
D(k)
Dprime(k)
```

Workloads:

```text
WL-UNIFORM
WL-DUP
```

$$
m=10^5.
$$

Configurations:

$$
4\times3\times2=24.
$$

Nominal:

$$
R=1000.
$$

---

# 49. RQ3-SMALL

Purpose:

```text
A1 small-buffer behavior
source edge behavior
conditional bias
failure behavior
```

$$
k\in\{2,4,8,16\}.
$$

Workload:

```text
WL-UNIFORM
```

Algorithms:

```text
A1(k)
W(k+1)
D(k)
Dprime(k)
```

Configurations:

$$
4\times4=16.
$$

Nominal:

$$
R=500.
$$

---

# 50. RQ3-EDGE

Only:

```text
Knuth-D
```

with:

$$
s=1.
$$

Workloads:

```text
WL-UNIFORM
WL-DUP
```

$$
R=1000.
$$

Source-validation/edge analysis only.

Không nằm trong main performance ranking.

---

# 51. SOURCE FIXTURE F1 — ORIGINAL X / A1

Configuration:

```text
s=2
stream=(a,b,c)
all distinct
```

S2 Appendix:

$$
\boxed{
P(\mathrm{failure})=\frac5{16}
}
$$

and:

$$
\boxed{
E[\hat F_0\mid\neg\mathrm{failure}]
=
\frac{20}{11}.
}
$$

Exact source fixture under matching algorithm semantics.

---

# 52. SOURCE FIXTURE F2 — D ALL-DISTINCT

For:

```text
D(s)
m distinct
1 <= s < m
```

S2 gives:

$$
|B|=s
$$

at termination and:

$$
p=U_{m,s+1}.
$$

Hence:

$$
\boxed{
\hat F_0=\frac{s}{U_{m,s+1}}
}
$$

and:

$$
\boxed{
E[\hat F_0]=m.
}
$$

S2's exercise answers also explicitly derive this and the corresponding variance for \(s\ge2\). ([Computer Science][2])

---

# 53. SOURCE FIXTURE F3 — D(1), REPEATED KEY

Stream:

$$
(a,b,a).
$$

S2 enumerates the six volatility-order cases and obtains:

$$
\boxed{
E[\hat F_0]=2.
}
$$

([Computer Science][2])

Validates:

```text
duplicate deletion
fresh volatility
cutoff interaction
```

---

# 54. SOURCE FIXTURE F4 — D(1), THREE DISTINCT

Stream:

$$
(a,b,c).
$$

S2 Exercise 8 gives:

$$
\boxed{
P(\hat F_0=0)=\frac27.
}
$$

and exact distribution over powers of two. ([Computer Science][2])

---

# 55. SOURCE-DERIVED FIXTURE F5 — W(2) / D(1)

Project source-derived normalization:

$$
\boxed{
W(k+1)\equiv D(k)
}
$$

so for \(k=1\):

$$
\boxed{
W(2)\equiv D(1).
}
$$

Therefore from F4:

$$
\boxed{
P_{W(2)}(\hat F_0=0)=\frac27
}
$$

under ideal/oracle semantics.

Label:

```text
[T-D]
```

Không phải direct standalone theorem statement.

Quan trọng:

$$
\boxed{
W(2)\not\equiv D'(1)
}
$$

---

# 56. D(1) DISTRIBUTION

For all-distinct stream:

$$
p=U_{m,2}.
$$

More generally:

$$
p=U_{m,s+1}.
$$

$$
U_{m,k}\sim\operatorname{Beta}(k,m+1-k).
$$

Therefore:

$$
\boxed{
p\sim\operatorname{Beta}(2,m-1)
}
$$

for \(s=1\). S2 defines \(U_{m,k}\) as the \(k\)-th smallest of \(m\) independent uniforms and gives its beta distribution. ([Computer Science][2])

---

# 57. D(1) VARIANCE SINGULARITY

For the distinct-stream formula, S2 derives:

$$
\boxed{
\operatorname{Var}(\hat F_0)
=
\frac{m(m-s)}{s-1},
\qquad s\ge2.
}
$$

At \(s=1\), the source edge case has infinite variance. ([Computer Science][2])

Therefore:

$$
\boxed{
\operatorname{Var}(\hat F_0)=\infty
}
$$

for the \(s=1\) source mathematical edge case.

This is [T-D].

---

# 58. D(1) STATISTICAL POLICY

Do not use:

```text
sample mean ± Student-t CI
```

to establish unbiasedness at \(s=1\).

Use:

```text
exact state-space fixtures
Beta/order-statistic distribution
distributional checks
median
quantiles
P(estimate=0)
tail summaries
sample mean as descriptive only
```

No CLT-based unbiasedness conclusion.

---

# 59. LEMMA M ORACLE

For short streams maintain independently:

```text
latest occurrence index
latest volatility
```

At each state boundary verify:

$$
\boxed{
(a,u_{\mathrm{last}(a)})\in B
\iff
u_{\mathrm{last}(a)}<p
}
$$

for D/D′.

Oracle inputs use strict volatility permutations.

Finite-rank ties are tested separately.

---

# 60. D ↔ D′ COUPLING TEST

Generate one exact volatility schedule:

$$
u_1,\ldots,u_m.
$$

Feed same schedule into D and D′.

At every state boundary verify source bracket:

$$
\boxed{
p_{\mathrm{bin},t}\le p_{\mathrm D,t}<2p_{\mathrm{bin},t}
}
$$

for ideal-order/reference mode. S2 states this bracket as a throughout-process property. ([Computer Science][2])

### Exact production finite-model predicate

```text
if D cutoff = ONE:
    Dprime level must equal 0

if D cutoff = RANK(c):
    q = 2*c + 1
    e = floor(log2(q))
    expected_level = 65 - e
    Dprime level must equal expected_level
```

Violation:

```text
COUPLING_FAILURE
```

is Gate-blocking.

Three layers must remain distinct:

```text
ideal source coupling
finite-model coupling
production randomized empirical behavior
```

---

# 61. W ↔ D CONFORMANCE

Primary exact source pair:

$$
\boxed{
W(k+1)
\quad\text{vs}\quad
D(k)
}
$$

for:

$$
k\in\{64,512,4096\}.
$$

This is the source-derived matched-capacity pair [T-D].

---

# 62. W ↔ D′ RELATION POLICY

No theorem of equivalence is claimed.

`W` and `D′` may still be compared empirically under matched retained capacity:

```text
W(k+1)
Dprime(k)
```

but such comparison is:

```text
empirical distributional comparison
```

not source equivalence.

---

# 63. W/D CONFORMANCE LEVELS

## Level 1 — Exact small-state

Oracle mode:

```text
controlled bit schedules
controlled volatility order
state transitions
occupancy
cutoff/level
output distribution
```

## Level 2 — Production

Independent production runs.

Compare:

```text
mean
median
variance
CDF
final occupancy
terminal level
```

This is empirical evidence only.

---

# 64. D/D′ DISTRIBUTIONAL TEST

Six predeclared comparisons:

$$
3k\times2\text{ workloads}.
$$

Primary variable:

$$
\boxed{
\mathrm{RE}
}
$$

using successful execution-valid runs only.

Primary statistic:

$$
\boxed{
D_{\mathrm{KS}}
=
\sup_x
|F_D(x)-F_{D'}(x)|
}
$$

Use two-sample permutation test.

Permutation count:

$$
\boxed{
B_{\mathrm{perm}}=100,000
}
$$

Permutation procedure:

```text
pool valid RE observations
retain original group sizes
randomly permute group labels
recompute KS statistic
```

P-value:

$$
\boxed{
p=
\frac{1+\#\{\text{permuted }T\ge T_{\rm obs}\}}
{B_{\mathrm{perm}}+1}
}
$$

Two-sided permutation interpretation.

Holm correction over six predeclared comparisons.

Always report:

```text
raw p
Holm-adjusted p
KS effect size
N_D
N_Dprime
invalid execution counts
```

If either group has fewer than two successful observations:

```text
INCONCLUSIVE
```

Không impute missing RE.

---

# 65. RQ3 VARIANCE COMPARISONS

For each:

```text
capacity × workload
```

compare:

```text
W
D
Dprime
```

Pairwise:

$$
\boxed{
VR_{A/B}
=
\frac{
\widehat{\operatorname{Var}}(RE_A)
}{
\widehat{\operatorname{Var}}(RE_B)
}
}
$$

Three pairwise comparisons per cell.

Use run-level bootstrap percentile CI.

Primary:

```text
effect estimation
```

No classical F-test.

### Bootstrap validity

A bootstrap replicate is invalid if:

```text
N_success < 2
variance undefined
denominator variance = 0 for VR
non-finite statistic
```

Do not substitute:

```text
0
INF
NA-imputation
```

For variance-ratio inference:

```text
valid replicate set only
```

is used.

If valid fraction is:

$$
<99\%
$$

or target \(B_{\mathrm{boot}}=10,000\) valid replicates cannot be obtained within:

```text
2 * B_boot
```

attempts:

```text
INCONCLUSIVE
```

If formal pairwise inference is invoked, Holm is applied over the full predeclared set:

$$
3\times3\times2=\boxed{18}
$$

comparisons.

---

# 66. RQ4 DESIGN

$$
k\in\{256,1024,4096\}.
$$

Algorithms:

```text
A1(k)
W(k+1)
D(k)
Dprime(k)
```

Workloads:

```text
WL-UNIFORM
WL-DUP
```

$$
m=10^6.
$$

Configurations:

$$
4\times3\times2=24.
$$

---

# 67. RQ4 PRIMARY ACCURACY

All four algorithms compared by:

$$
\boxed{
q_{\mathrm{total,alg}}(\varepsilon)
}
$$

for:

$$
\varepsilon\in\{0.05,0.10,0.20\}.
$$

For each algorithm/configuration report:

```text
q_hat
95% two-sided Clopper–Pearson CI
N_exec
N_success
N_abort
NumericFailure count
ResourceFailure count
execution_invalid_rate
```

No global “best algorithm” theorem is inferred.

---

# 68. RQ4 BIAS TRACKS

Primary bias group:

```text
CVM-W
Knuth-D
Knuth-Dprime
```

using unconditional bias over successful execution-valid runs.

A1:

```text
conditional bias
```

reported separately.

---

# 69. H4a — NRMSE

$$
\boxed{
H4a:
NRMSE_D\le NRMSE_{D'}
}
$$

at matched retained capacity.

Define:

$$
\Delta_{\mathrm{NRMSE}}
=
NRMSE_D-NRMSE_{D'}.
$$

For each of the six predeclared:

```text
k × workload
```

cells, use run-level percentile bootstrap.

Primary evidence:

$$
\boxed{
95\%\ \text{one-sided upper CI for }
\Delta_{\mathrm{NRMSE}}
}
$$

Interpretation:

```text
upper CI <= 0:
    supports H4a in that cell

CI overlaps 0:
    inconclusive

lower CI > 0:
    evidence against H4a in that cell
```

Không gộp sáu cells thành universal H4a claim.

---

# 70. H4b — A1 CONDITIONAL BIAS

$$
\boxed{
|\mathrm{Bias}_{A1,\mathrm{cond}}|
}
$$

được kỳ vọng giảm khi \(k\) tăng.

Đây là descriptive/hypothesis-driven analysis.

Không rank trực tiếp với W/D/D′ unconditional bias.

---

# 71. A1 TOTAL FAILURE

A1 trực tiếp comparable qua:

$$
q_{\mathrm{total,alg}}.
$$

Operational algorithmic failure:

```text
Abort
+
successful output outside tolerance
```

`NumericFailure` và `ResourceFailure` là execution failures và report separately.

---

# 72. TOST METHODOLOGY

TOST áp dụng cho:

```text
CVM-W
Knuth-D
Knuth-Dprime
```

ở configurations đủ điều kiện finite-variance.

Margin:

$$
\boxed{
\Delta=0.01
}
$$

Target practical equivalence:

$$
-0.01<\mu_{\rm RE}<0.01.
$$

Pass TOST không đồng nghĩa với theorem-level unbiasedness.

Required wording:

> The mean relative error is empirically equivalent to zero within the pre-specified practical margin \(\Delta=0.01\).

---

# 73. TOST EXACT PROCEDURE

TOST là **one-sample Student-t TOST** trên successful execution-valid `RE`.

Define:

$$
n=N_{\mathrm{succ}}
$$

$$
\bar x=\frac1n\sum_i RE_i
$$

$$
s_x=
\sqrt{
\frac1{n-1}
\sum_i(RE_i-\bar x)^2
}.
$$

Lower-bound null:

$$
H_{0L}:\mu_{\mathrm{RE}}\le-\Delta
$$

against:

$$
H_{1L}:\mu_{\mathrm{RE}}>-\Delta.
$$

Statistic:

$$
\boxed{
t_L=
\frac{\bar x+\Delta}
{s_x/\sqrt n}
}
$$

Upper-bound null:

$$
H_{0U}:\mu_{\mathrm{RE}}\ge\Delta
$$

against:

$$
H_{1U}:\mu_{\mathrm{RE}}<\Delta.
$$

Statistic:

$$
\boxed{
t_U=
\frac{\bar x-\Delta}
{s_x/\sqrt n}
}
$$

with:

$$
df=n-1.
$$

Decision:

```text
reject H0L iff
    t_L > t_(1-alpha, df)

reject H0U iff
    t_U < -t_(1-alpha, df)

TOST PASS iff both reject
```

với:

$$
\alpha=0.05.
$$

Equivalent CI formulation:

$$
\boxed{
CI_{90\%}(\mu_{\mathrm{RE}})
\subset
(-\Delta,\Delta)
}
$$

under the Student-t implementation.

### Degenerate cases

Nếu:

```text
n < 2
```

hoặc:

```text
s_x is non-finite
```

hoặc:

```text
s_x = 0
```

thì:

```text
TOST_STATUS = INCONCLUSIVE
```

Không thay bằng zero hoặc một test ad-hoc khác.

---

# 74. TOST MULTIPLICITY SCOPE

Có:

$$
3\ algorithms
\times3\ k
\times2\ workloads
=
\boxed{18}
$$

predeclared TOST cells.

Primary interpretation là **cell-specific**.

Không có global statement:

> all 18 configurations are unbiased

chỉ dựa trên 18 independent TOST passes.

Nếu muốn đưa ra global equivalence claim across all 18 cells, phải:

```text
apply Holm correction over 18 TOST cells
```

và explicitly label it as a separate aggregate claim.

---

# 75. POWER PILOT

Pilot:

$$
R_{\mathrm{pilot}}=100.
$$

Estimate:

$$
\hat\sigma_{\mathrm{RE}}.
$$

Required sample:

$$
\boxed{
R_{\mathrm{req}}
=
\left\lceil
\left(
\frac{
(z_{1-\alpha}+z_{1-\mathrm{power}})
\hat\sigma_{\mathrm{RE}}
}{
\Delta
}
\right)^2
\right\rceil
}
$$

với:

$$
\alpha=0.05,
\qquad
\mathrm{power}=0.80,
\qquad
\Delta=0.01.
$$

Round upward to nearest 100.

### Pilot degeneracy

Nếu:

```text
pilot successful n < 2
```

hoặc:

```text
sigma_hat is non-finite
```

hoặc:

```text
sigma_hat = 0
```

thì:

```text
POWER_CALIBRATION = DEGENERATE
```

Không dùng \(\hat\sigma=0\) để giả tạo sample size bằng 0.

Trong trường hợp này:

```text
R_conf = R_nom
```

và TOST confirmation vẫn phải tuân theo eligibility rule của Section 73.

---

# 76. CONFIRMATION SAMPLE POLICY

$$
\boxed{
R_{\mathrm{conf}}
=
\max(R_{\mathrm{nom}},R_{\mathrm{req}})
}
$$

Maximum:

```text
RQ3-Main:
    2000

RQ4:
    1000
```

If:

$$
R_{\mathrm{req}}>R_{\max},
$$

mark:

```text
INCONCLUSIVE_UNDERPOWERED
```

**before** interpreting confirmation data.

Pilot data never pooled into final confirmatory estimands.

---

# 77. MULTIPLE TESTING MASTER POLICY

## RQ1

Within each:

$$
(m,\varepsilon,\delta)
$$

family:

$$
\alpha=0.05/13.
$$

## RQ3 D/D′ distribution tests

Holm over:

$$
6
$$

predeclared comparisons.

## RQ3 variance formal comparisons

Holm over:

$$
18
$$

if formal pairwise hypothesis testing is reported.

## RQ4 TOST

Cell-specific primary interpretation.

Holm over 18 only for an explicit global equivalence claim.

## H4a

Confidence-interval estimation per cell.

No pooled multiplicity claim.

---

# 78. NO ISO-MEMORY STUDY

Không cố định:

```text
same bytes
same heap allocation
same object layout
same allocator footprint
```

Có thể report:

```text
peak_buffer_size
allocated_state_bytes
```

nhưng chỉ diagnostics.

Không phải primary RQ4 variables.

---

# 79. THROUGHPUT POLICY

$$
\boxed{
\mathrm{throughput}
=
\frac{\text{processed items}}
{\text{algorithm-only wall time}}
}
$$

Timed region excludes:

```text
stream generation
CSV serialization
console logging
figure generation
analysis
```

Report:

```text
mean
median
standard deviation
run-level distribution
```

where appropriate.

Throughput is:

> implementation-level throughput under the declared execution environment.

Không suy ra:

```text
asymptotic superiority
algorithmic optimality
microarchitectural superiority
```

Parallel execution:

```text
throughput_items_per_sec=null
```

---

# 80. C++20 ARCHITECTURE

```text
include/cvm/core/
    types.hpp
    concepts.hpp
    rng.hpp
    rank.hpp
    stream.hpp
    cvm_a1.hpp
    cvm_w.hpp
    knuth_d.hpp
    knuth_dprime.hpp
    diagnostics.hpp

include/cvm/ds/
    dense_key_set.hpp
    key_index_map.hpp
    indexed_heap.hpp
    cutoff64.hpp

src/
tests/
benchmarks/
scripts/

analysis/
notebooks/
docs/
configs/
```

---

# 81. API DESIGN

```cpp
enum class FailureStatus : std::uint8_t {
    Success,
    Abort,
    NumericFailure,
    ResourceFailure
};

class EstimateResult {
private:
    std::optional<double> estimate_;
    FailureStatus status_;

public:
    static constexpr EstimateResult success(double v) noexcept;
    static constexpr EstimateResult abort() noexcept;
    static constexpr EstimateResult numeric_failure() noexcept;
    static constexpr EstimateResult resource_failure() noexcept;

    [[nodiscard]]
    constexpr bool has_value() const noexcept;

    [[nodiscard]]
    constexpr FailureStatus status() const noexcept;

    [[nodiscard]]
    double value() const;

    [[nodiscard]]
    constexpr std::optional<double>
    as_optional() const noexcept;
};
```

Invariant:

$$
\boxed{
\mathrm{has\_value()}
\iff
\mathrm{status}=Success
}
$$

`value()` on non-success is a contract violation and must not be called.

---

# 82. CARDINALITY ESTIMATOR CONCEPT

```cpp
template <typename E, typename T>
concept CardinalityEstimator =
requires(E e, const E ce, const T& item) {
    { e.update(item) } -> std::same_as<void>;
    { ce.estimate() } -> std::same_as<EstimateResult>;
    { e.reset() } -> std::same_as<void>;
    { ce.name() } -> std::convertible_to<std::string_view>;
};
```

Diagnostics là separate interface.

Concepts constrain API.

Templates provide static dispatch.

No zero-overhead claim.

---

# 83. A1/W DATA STRUCTURE

## 83.1. Forbidden

Không:

```text
open-addressed hash set
+
swap-delete physical hash table slots
```

---

## 83.2. Required

```text
dense vector<Key>
+
open-addressed key -> dense-index map
```

Conceptually:

```text
elements:
    vector<Key>

index:
    OpenAddressMap<Key, uint32_t>
```

---

## 83.3. Removal

```text
find key
remove logical mapping
if dense vector uses swap-remove:
    move last_key into removed slot
    update index[last_key]
    pop_back()
```

---

## 83.4. Allocation policy

After reserve:

```text
ordinary update path:
    no general-purpose allocation expected
```

Exact behavior must be instrumented and verified.

---

# 84. D/D′ DATA STRUCTURE

Use:

```text
indexed max-heap
+
key -> heap-index map
```

Required operations:

```text
lookup(key)
delete(key)
insert(key, volatility)
get_max_volatility()
replace_max(...)
remove_all_above_cutoff(...)
```

S2 uses a treap. Project indexed heap is [M], not source semantics. S2 describes the underlying requirement as a dictionary plus priority queue/heap supporting lookup, insertion, deletion and maximum-volatility operations. ([Computer Science][2])

---

# 85. HEAP INVARIANTS

At all times:

```text
heap-order:
    parent.rank >= child.rank

index-map correctness:
    key -> exact heap slot

unique keys:
    <= 1 pair per key

strict cutoff:
    rank < cutoff
```

On equal maximum ranks:

```text
purge all rank >= cutoff
```

---

# 86. RANDOMNESS API

Expose:

```cpp
next_bit()
next_uint64()
```

A1/W:

```text
next_bit()
```

for:

```text
BernoulliPow2
halving
```

D/D′:

```text
next_uint64()
```

for volatility.

Oracle mode:

```text
deterministic bit tape
deterministic volatility permutation
```

---

# 87. NUMERICAL POLICY — A1/W

Store:

```text
halving_count l
```

instead of repeatedly storing \(p\).

$$
p=2^{-l}.
$$

Estimate conceptually:

$$
\hat F_0=|X|2^l.
$$

Use checked scaling.

For:

$$
|X|>0:
$$

$$
\log_2\hat F_0
=
\log_2|X|+l.
$$

Nếu direct conversion overflow:

```text
NumericFailure
```

only when final numerical output itself cannot be represented according to API contract.

---

# 88. NUMERICAL POLICY — D

Store cutoff as exact project state:

```text
ONE
or
RANK(c)
```

No floating-point acceptance.

Final estimate:

```text
long double
```

with:

```text
finite check
non-negative check
overflow/range check
conversion check
```

---

# 89. NUMERICAL POLICY — D′

Store:

```text
uint32_t l
```

or larger checked integer.

Acceptance threshold:

```text
l=0:
    all ranks

1<=l<=64:
    rank < 2^(64-l)

l>=65:
    no rank accepted
```

Do not materialize \(2^{64}\) inside `uint64_t`.

Implementation may use:

```text
special case
or
wider internal integer
```

but semantics must remain exact.

---

# 90. TESTING FRAMEWORK

## Unit tests

Each algorithm:

```text
initialization
insertion
duplicate
threshold crossing
reduction
termination
estimate
reset
```

## Invariants

A1/W:

$$
|X|\le s.
$$

W after update:

$$
|X|\le s-1.
$$

$$
p=2^{-l}.
$$

D/D′:

$$
|B|\le s.
$$

$$
r_{\rm stored}<r_{\rm cutoff}.
$$

D:

$$
p_{t+1}\le p_t.
$$

D′:

$$
p=2^{-l}.
$$

---

# 91. ORACLE TEST SUITE

Mandatory:

```text
F1
F2
F3
F4
F5
Lemma M
D/Dprime cutoff bracket
W/D parameter-shift conformance
finite tie semantics
finite resolution semantics
exact-capacity edge semantics
```

Every fixture:

```text
mathematical expectation
oracle implementation
production implementation
tolerance/status rule
```

---

# 92. W/D/D′ SMALL-STATE ORACLE

Small streams:

```text
enumerate controlled fair-bit schedules
enumerate volatility permutations
```

Compare:

```text
state transitions
occupancy
cutoff
estimate distribution
```

Oracle mode is reference semantics, not benchmark mode.

---

# 93. EDGE CASES

Mandatory:

```text
m=0
m=1
one unique key repeated
all distinct
WL-DUP

s=1 for D/Dprime

A1/W:
    s = F0*
    s = F0* + 1
    s = F0* - 1

D/Dprime:
    s = F0*
    s = F0* - 1

duplicate immediately after insertion
duplicate at cutoff
multiple equal finite ranks
incoming rank == cutoff
multiple maximum ranks
multi-halving W event
A1 abort
D cutoff rank 0
Dprime l=64 -> 65
```

## Exact-capacity semantics

For controlled finite streams:

### A1/W

$$
\boxed{
s>F_0^*
}
$$

must return exact distinct count:

$$
\boxed{
\hat F_0=F_0^*
}
$$

provided no `NumericFailure` or `ResourceFailure`.

### D/D′

$$
\boxed{
s\ge F_0^*
}
$$

must return exact distinct count:

$$
\boxed{
\hat F_0=F_0^*
}
$$

provided no `NumericFailure` or `ResourceFailure`.

Reason: A1 checks `if |X|=thresh` after insertion, so \(s=F_0^*\) still triggers reduction; D instead returns with full buffer when \(s\ge n\). S1 Algorithm 1 gives the A1 trigger sequence, while S2's exercise answer explicitly states Algorithm D is exact when \(s\ge n\). ([arXiv][1])

Mandatory regression fixtures:

```text
A1(s=F0)
W(s=F0)
A1(s=F0+1)
W(s=F0+1)

D(s=F0)
Dprime(s=F0)
```

Expected outcomes:

```text
A1(s=F0):
    must follow source reduction semantics
    NOT guaranteed exact

W(s=F0):
    must follow while-reduction semantics
    NOT guaranteed exact

A1(s=F0+1):
    exact

W(s=F0+1):
    exact

D(s=F0):
    exact

Dprime(s=F0):
    exact
```

---

# 94. SANITIZER POLICY

All C++ tests:

```text
AddressSanitizer
UndefinedBehaviorSanitizer
```

Build flags:

```text
-Wall
-Wextra
-Wpedantic
```

where supported.

Release benchmark executable separate from sanitizer build.

---

# 95. RESOURCE BUDGET — RQ1

Configurations:

$$
2\times3\times3\times13
=
\boxed{234}.
$$

Runs:

$$
234\times1000
=
\boxed{234,000}.
$$

Items:

$$
3\times3\times13\times1000\times(10^5+10^6)
=
\boxed{1.287\times10^{11}}.
$$

---

# 96. RESOURCE BUDGET — RQ2

Configurations:

$$
2\times2\times2\times4
=
32.
$$

Runs:

$$
32\times20
=
\boxed{640}.
$$

Items:

$$
2\times2\times4\times20\times(10^5+10^6)
=
\boxed{3.52\times10^8}.
$$

---

# 97. RESOURCE BUDGET — RQ3-MAIN

Configurations:

$$
4\times3\times2=24.
$$

Runs:

$$
24\times1000
=
\boxed{24,000}.
$$

Items:

$$
\boxed{2.4\times10^9}.
$$

---

# 98. RESOURCE BUDGET — RQ3-SMALL

Configurations:

$$
4\times4=16.
$$

Runs:

$$
16\times500
=
\boxed{8,000}.
$$

Items:

$$
\boxed{8\times10^8}.
$$

---

# 99. RESOURCE BUDGET — RQ3-EDGE

Configurations:

$$
2.
$$

Runs:

$$
2\times1000
=
\boxed{2,000}.
$$

Items:

$$
\boxed{2\times10^8}.
$$

---

# 100. RESOURCE BUDGET — RQ4

Configurations:

$$
4\times3\times2=24.
$$

Nominal:

$$
R=500.
$$

Runs:

$$
24\times500
=
\boxed{12,000}.
$$

Items:

$$
\boxed{1.2\times10^{10}}.
$$

---

# 101. TOTAL NOMINAL BUDGET

$$
234,000
+640
+24,000
+8,000
+2,000
+12,000
=
\boxed{280,640\ runs}.
$$

Items:

$$
1.287\times10^{11}
+
3.52\times10^8
+
2.4\times10^9
+
8\times10^8
+
2\times10^8
+
1.2\times10^{10}
=
\boxed{
1.44452\times10^{11}
}
$$

items.

---

# 102. PILOT BUDGET

RQ3:

$$
24\times100=2,400
$$

runs.

RQ4:

$$
24\times100=2,400
$$

runs.

Total:

$$
\boxed{
4,800\ runs
}
$$

Items:

$$
24\times100\times10^5
+
24\times100\times10^6
=
\boxed{
2.64\times10^9
}
$$

items.

---

# 103. MAXIMUM CONFIRMATION BUDGET

RQ3-Main extra:

$$
24(2000-1000)=24,000.
$$

RQ4 extra:

$$
24(1000-500)=12,000.
$$

Extra:

$$
\boxed{36,000\ runs}.
$$

Including pilot:

$$
280,640+4,800+36,000
=
\boxed{
321,440\ runs
}
$$

Maximum approximate items:

$$
1.44452\times10^{11}
+
2.64\times10^9
+
14.4\times10^9
=
\boxed{
1.61492\times10^{11}
}
$$

items.

Đây là planning ceiling, không phải target execution time.

---

# 104. EXECUTION-BUDGET POLICY

Pilot đo:

```text
items/sec
wall time/configuration
memory
failure rates
finite-rank events
```

Priority khi resource pressure:

```text
1. RQ3-Small
2. non-primary diagnostics
```

Không tự động giảm:

```text
RQ1 R
RQ1 inference family
RQ3/RQ4 pilot logic
```

Mọi budget alteration phải ghi trong:

```text
CHANGELOG.md
```

---

# 105. RAW DATA SCHEMA

## `raw_results.csv`

```text
run_id
experiment_id
phase
algorithm
workload

m
universe_n
f0_true

epsilon
delta

algorithm_param_s
matched_capacity_k
theoretical_capacity

trial_index
trial_seed
master_seed

status
execution_valid
approximation_valid
approximation_failure

estimate
estimate_log2

final_buffer_size
peak_buffer_size

sampling_rate_final
sampling_rate_repr
halving_count
reduction_rounds

dup_removals

tie_events
cutoff_tie_events
equal_max_rank_events
rank_collision_events
resolution_floor_hits

throughput_items_per_sec
allocated_state_bytes

coupling_mode
coupling_level

git_commit
source_s1_version
source_s2_version

compiler_version
build_flags
os_name
cpu_model

binary_hash
source_tree_hash
config_hash
analysis_environment

workload_generator_version
schema_version
```

---

# 106. NULL RULES

```text
status != Success
    estimate = null
    estimate_log2 = null
```

```text
algorithm = Knuth-D
    halving_count = null
```

```text
algorithm != Knuth-D and != Knuth-Dprime
    tie_events = null
    cutoff_tie_events = null
    equal_max_rank_events = null
    rank_collision_events = null
```

```text
parallel run
    throughput_items_per_sec = null
```

```text
RQ1/RQ4:
    epsilon = declared value

RQ2/RQ3 source fixture where not applicable:
    epsilon = null
    delta = null
```

For D′:

```text
sampling_rate_repr = dyadic exponent
```

For D:

```text
sampling_rate_repr = exact cutoff representation
```

`null` means semantic missingness.

Không convert:

```text
null -> 0
```

---

# 107. RQ2 TRACE SCHEMA

## `state_trace.csv`

```text
run_id
step_t

buffer_occupancy

sampling_rate_p
sampling_rate_repr
halving_count

status
```

Required snapshots:

```text
t=0
t=m
every ceil(m/100) steps
```

## `reduction_events.csv`

```text
run_id
episode_index
transition_index
step_t

level_before
level_after

p_before
p_after

halvings_in_episode
cascade_index

delta_t_since_previous_transition
delta_t_valid
```

---

# 108. PROVENANCE

Every production experiment:

```text
git commit
source versions
compiler
build flags
OS
CPU model

PRNG algorithm
PRNG implementation version
master seed

experiment config hash
binary hash
source tree hash

analysis environment
workload generator version
schema version
```

---

# 109. SEED HIERARCHY

$$
\mathrm{master\_seed}
\rightarrow
\mathrm{experiment\_seed}
\rightarrow
\mathrm{algorithm\_seed}
\rightarrow
\mathrm{trial\_seed}.
$$

$$
\boxed{
\mathrm{trial\_seed}
=
H(
\mathrm{master\_seed},
\mathrm{experiment\_id},
\mathrm{algorithm},
\mathrm{matched\_capacity},
\mathrm{workload},
\mathrm{trial\_index},
\mathrm{phase}
)
}
$$

Hash construction is implementation detail but must be stable.

---

# 110. COMMON-RANDOM-NUMBER POLICY

Ordinary production:

```text
independent trial seeds
```

Special coupling:

```text
shared exact volatility schedule
shared oracle schedule
```

carry:

```text
coupling_mode=true
```

Coupled runs are not mixed silently with ordinary confirmatory estimates.

---

# 111. NOTEBOOKS

Exactly four research notebooks.

## `01_cvm_theoretical_bounds.ipynb`

```text
threshold table
CP intervals
algorithmic failure rates
execution invalid rates
certification decisions
threshold state
non-monotone flag
censoring
conservatism ratio
theory-consistency checks
```

## `02_cvm_internal_dynamics.ipynb`

```text
|X_t|
p_t
l_t
reduction episodes
H2a
cascade analysis
H2b
WL-DUP validation
residual analysis
```

## `03_knuth_variants_validation.ipynb`

```text
F1-F5
D s=1
Beta/order-statistic analysis
Lemma M
D/Dprime coupling
W/D conformance
W/Dprime empirical comparison
finite-rank diagnostics
exact-capacity edge tests
```

## `04_final_core_comparison.ipynb`

```text
RQ4
Track A
A1 conditional Track B
W/D/Dprime unconditional Track B
NRMSE
variance ratios
bootstrap CIs
TOST
throughput
execution-invalid diagnostics
```

No Iso-Memory plot.

---

# 112. RESEARCH SCRIPTS

```text
scripts/
    setup_env.sh
    build.sh
    run_unit_tests.sh
    run_sanitizers.sh
    run_fixtures.sh

    run_rq1.sh
    run_rq2.sh

    run_rq3_pilot.sh
    run_rq4_pilot.sh

    calibrate_power.sh

    run_rq3_confirmation.sh
    run_rq4_confirmation.sh

    validate_schema.sh

    generate_figures.py
    generate_report.py
```

---

# 113. REPRODUCIBILITY COMMAND

```bash
git clone <repository>
cd CVM-analysis

bash scripts/setup_env.sh
bash scripts/build.sh --release

bash scripts/run_unit_tests.sh
bash scripts/run_sanitizers.sh
bash scripts/run_fixtures.sh

bash scripts/run_rq1.sh
bash scripts/run_rq2.sh

bash scripts/run_rq3_pilot.sh
bash scripts/run_rq4_pilot.sh

bash scripts/calibrate_power.sh

bash scripts/run_rq3_confirmation.sh
bash scripts/run_rq4_confirmation.sh

bash scripts/validate_schema.sh

python3 analysis/generate_figures.py
python3 analysis/generate_report.py
```

Production execution phải resumable mà không thay đổi seed semantics.

---

# 114. CANONICAL PIPELINE ORDER

```text
validate config
    ↓
build
    ↓
unit tests
    ↓
sanitizer tests
    ↓
source fixtures
    ↓
edge-capacity fixtures
    ↓
RQ1 / RQ2 exploratory-production runs
    ↓
RQ3 pilot
RQ4 pilot
    ↓
power calibration
    ↓
underpowered decision
    ↓
RQ3 confirmation
RQ4 confirmation
    ↓
schema validation
    ↓
statistical analysis
    ↓
figure generation
    ↓
report generation
```

Any source-conformance Gate failure blocks scientific reporting.

---

# 115. CI POLICY

CI seed:

```text
42
```

only for:

```text
compile
unit tests
fixture regression
sanitizer
schema validation
runner integrity
```

CI output is not statistical evidence.

Large experiments never silently execute inside CI.

---

# 116. FAILURE MODES

| Code | Situation                                     | Action                           |
| ---- | --------------------------------------------- | -------------------------------- |
| F01  | Invalid config                                | Reject before run                |
| F02  | Non-finite numerical result                   | NumericFailure                   |
| F03  | \(m=0\)                                       | exact zero                       |
| F04  | Schema mismatch                               | Pipeline failure                 |
| F05  | Rank tie                                      | Record + report                  |
| F06  | TOST underpowered                             | INCONCLUSIVE_UNDERPOWERED        |
| F07  | Source conformance violation                  | Block Gate                       |
| F08  | RQ1 non-monotone curve                        | Set `non_monotone_flag=true`     |
| F09  | Resource failure                              | ResourceFailure                  |
| F10  | finite-rank resolution                        | Record + sensitivity flag        |
| F11  | coupling violation                            | COUPLING_FAILURE                 |
| F12  | provenance mismatch                           | Pipeline failure                 |
| F13  | Theory check attempted with execution invalid | THEORY_CONSISTENCY_CHECK_BLOCKED |
| F14  | Bootstrap target not reached                  | INCONCLUSIVE                     |
| F15  | TOST degenerate sample                        | INCONCLUSIVE                     |
| F16  | Exact-capacity fixture violation              | Block Gate                       |

Không source algorithmic failure nào được encode thành numerical zero.

---

# 117. SOURCE CLAIM LEDGER

| ID  | Claim                                                               | Locator                            | Label                                 |                               |     |
| --- | ------------------------------------------------------------------- | ---------------------------------- | ------------------------------------- | ----------------------------- | --- |
| C01 | Problem 1 \((\varepsilon,\delta)\)-approximation                    | S1 §1, Problem 1                   | [T]                                   |                               |     |
| C02 | \(\log=\log_2\)                                                     | S1 §1                              | [T]                                   |                               |     |
| C03 | A1 threshold formula                                                | S1 Algorithm 1                     | [T]                                   |                               |     |
| C04 | A1 approximation guarantee                                          | S1 Theorem 2                       | [T]                                   |                               |     |
| C05 | \(\Pr[\mathrm{Fail}]\le\delta/8\)                                   | S1 Claim 3                         | [T]                                   |                               |     |
| C06 | \(\Pr[\mathrm{Error}\cap\overline{\mathrm{Fail}}]\le\delta/2\)      | S1 Claim 4                         | [T]                                   |                               |     |
| C07 | Old joint-independence claim is erroneous                           | S1 footnote after Algorithm 1      | [T]                                   |                               |     |
| C08 | (                                                                   | X                                  | /p) is not unbiased in general for A1 | S1 footnote after Algorithm 1 | [T] |
| C09 | Algorithm D semantics                                               | S2 D1–D6                           | [T]                                   |                               |     |
| C10 | Algorithm D unbiased                                                | S2 Algorithm D statement / theorem | [T]                                   |                               |     |
| C11 | Lemma M                                                             | S2 p.2                             | [T]                                   |                               |     |
| C12 | D′ semantics                                                        | S2 p.5, D6′                        | [T]                                   |                               |     |
| C13 | D′ unbiased                                                         | S2 p.5                             | [T]                                   |                               |     |
| C14 | \(p_{\mathrm{bin}}\le p<2p_{\mathrm{bin}}\)                         | S2 Eq. (13)                        | [T]                                   |                               |     |
| C15 | Lemma R / supporting cutoff result                                  | S2 p.5                             | [T]                                   |                               |     |
| C16 | Distinct-stream D: \(p=U_{m,s+1}\)                                  | S2 Answers, Ex. 3                  | [T]                                   |                               |     |
| C17 | Distinct D expectation \(E[X]=m\)                                   | S2 Answers, Ex. 3                  | [T]                                   |                               |     |
| C18 | D(1), \((a,b,a)\), expectation 2                                    | S2 Eq. (7)                         | [T]                                   |                               |     |
| C19 | D(1), \((a,b,c)\), \(P(X=0)=2/7\)                                   | S2 Answers, Ex. 8                  | [T]                                   |                               |     |
| C20 | D variance formula valid for \(s\ge2\); \(s=1\) singular            | S2 Answers, Ex. 3/6                | [T]/[T-D]                             |                               |     |
| C21 | Original Algorithm X semantics                                      | S2 Appendix p.12                   | [T]                                   |                               |     |
| C22 | Original X \(s=2,(a,b,c)\), failure \(5/16\)                        | S2 Appendix p.12                   | [T]                                   |                               |     |
| C23 | Original X conditional mean \(20/11\)                               | S2 Appendix p.12                   | [T]                                   |                               |     |
| C24 | If→While + no failure yields unbiased modified algorithm            | S2 Appendix p.12                   | [T]                                   |                               |     |
| C25 | Modified X equivalent to Algorithm D under one-unit parameter shift | S2 Appendix p.12                   | [T]                                   |                               |     |
| C26 | \(W(k+1)\equiv D(k)\) under project normalization                   | C25 + parameter reindexing         | [T-D]                                 |                               |     |
| C27 | D/D′ cutoff bracket                                                 | S2 Eq. (13)                        | [T]                                   |                               |     |
| C28 | Algorithm D exact when \(s\ge n\)                                   | S2 Exercise 7                      | [T]                                   |                               |     |

S1 source facts trực tiếp trong Algorithm 1/Theorem 2/Claims/footnote. ([arXiv][1])

S2 source facts trực tiếp trong revised note, Appendix và Answers. ([Computer Science][2])

---

# 118. PROJECT METHODOLOGY LEDGER

| ID  | Methodology                                       | Label |
| --- | ------------------------------------------------- | ----- |
| M01 | A1(s) threshold parameterization                  | [M]   |
| M02 | 13-point dyadic RQ1 grid                          | [M]   |
| M03 | Family = fixed \((m,\varepsilon,\delta)\)         | [M]   |
| M04 | CP interval for RQ1                               | [M]   |
| M05 | CP endpoint handling                              | [M]   |
| M06 | Threshold-state classifier                        | [M]   |
| M07 | Separate `non_monotone_flag`                      | [M]   |
| M08 | Non-monotone exclusion from primary H1            | [M]   |
| M09 | Matched retained capacity \(k\)                   | [M]   |
| M10 | W uses \(s=k+1\)                                  | [M]   |
| M11 | WL-DUP canonical workload                         | [M]   |
| M12 | A1 conditional Track B                            | [M]   |
| M13 | Run-cluster bootstrap                             | [M]   |
| M14 | TOST \(\Delta=0.01\)                              | [M]   |
| M15 | Pilot-based TOST power calibration                | [M]   |
| M16 | Predeclared multiplicity scopes                   | [M]   |
| M17 | Finite 64-bit rank realization                    | [M]   |
| M18 | Deterministic finite-rank tie policy              | [M]   |
| M19 | Exact Oracle Mode                                 | [M]   |
| M20 | Dense vector + key→index map                      | [M]   |
| M21 | Indexed max-heap + key index                      | [M]   |
| M22 | Throughput timing boundary                        | [M]   |
| M23 | Binary/source-tree hash provenance                | [M]   |
| M24 | Exact `Cutoff64` representation                   | [M]   |
| M25 | D′ exponent representation                        | [M]   |
| M26 | RQ3 KS permutation statistic                      | [M]   |
| M27 | RQ3 permutation count \(100,000\)                 | [M]   |
| M28 | H4a run-level bootstrap CI                        | [M]   |
| M29 | TOST cell-level primary interpretation            | [M]   |
| M30 | `q_total_alg` conditional on execution-valid runs | [M]   |
| M31 | Separate execution-invalid rate                   | [M]   |
| M32 | Exact D/D′ finite bracket predicate               | [M]   |
| M33 | Mandatory RQ2 trace snapshots \(t=0,m\)           | [M]   |
| M34 | Bootstrap invalid-replicate policy                | [M]   |
| M35 | Exact Student-t TOST procedure                    | [M]   |
| M36 | KS on successful-run RE                           | [M]   |
| M37 | Exact-capacity edge policy                        | [M]   |
| M38 | `epsilon`/`delta` explicit raw schema fields      | [M]   |

---

# 119. FORBIDDEN CLAIMS

Repository-wide forbidden:

```text
Every element seen so far is independently in A1's X with probability p.

Every CVM state indicator is jointly independent.

A1 is unbiased.

s_emp is the universal required buffer.

W(s) and Dprime(s) are source-equivalent.

W(k+1) and Dprime(k) are mathematically equivalent.

W(k+1) ≡ D(k) is a direct standalone source theorem.

Experiment proves unbiasedness.

p > 0.05 proves equivalence.

Tie probability is exactly zero in implementation.

l > 52 is a mathematical limitation.

64-bit finite-rank D is exactly the ideal continuous D.

64-bit finite-rank Dprime is exactly the ideal continuous Dprime.

WL-UNIFORM and WL-DUP characterize all possible streams.

Throughput proves asymptotic superiority.

TOST passing proves unbiasedness.

An isolated empirical certification point proves a universal threshold.

NumericFailure is approximation failure.

ResourceFailure is approximation failure.

Execution-invalid runs may be silently deleted without reporting.

Exactness at s=F0 applies to A1/W.

Iso-Memory is part of RQ4.
```

---

# 120. REQUIRED INFERENCE WORDING

Allowed:

```text
consistent with
supported by the observed data
empirical evidence indicates
no statistically detectable difference was observed
source theorem predicts
implementation conforms to the tested source semantics
observed under the declared workload
equivalent within the pre-specified practical margin
execution-invalid rate was
theory-consistency check was blocked by execution failures
```

Forbidden:

```text
proved experimentally
proved unbiased
proved mathematically by simulation
p > 0.05 proves equality
universally better
universally sufficient
works for all streams
```

---

# 121. SOURCE-SEMANTICS VS EMPIRICAL-SEMANTICS BOUNDARY

Every scientific statement belongs to exactly one category.

## Type 1 — Source theorem

Example:

$$
E[\hat F_0]=F_0^*
$$

for ideal Knuth-D/D′.

---

## Type 2 — Source-derived mathematical property

Examples:

$$
P(\hat F_0=0)=\frac27
$$

for the specific D(1) fixture, or:

$$
p_{\mathrm{bin}}\le p<2p_{\mathrm{bin}}
$$

for the D/D′ source bracket.

---

## Type 3 — Source-derived relationship

Example:

$$
\boxed{
W(k+1)\equiv D(k)
}
$$

under project parameter normalization.

Label:

```text
[T-D]
```

---

## Type 4 — Project methodology

Example:

$$
\boxed{
s_{\mathrm{emp}}^{\mathrm{obs}}
}
$$

under:

```text
fixed grid
fixed workload
fixed m
fixed epsilon/delta
fixed CP protocol
```

---

## Type 5 — Empirical result

Example:

$$
\boxed{
\frac{s_{\mathrm{theo}}}
{s_{\mathrm{emp}}^{\mathrm{obs}}}
=
4.7
}
$$

only under the declared experiment.

No type may silently transform into another.

---

# 122. DEFINITION OF DONE — SOURCE FIDELITY

```text
[ ] S1 v2 metadata verified
[ ] S2 revised 29/12/2023 metadata verified
[ ] every [T] has locator
[ ] every [T-D] has derivation note
[ ] S1 log base = 2 everywhere
[ ] old A1 independence claim not repeated
[ ] no A1 unbiasedness claim
[ ] D semantics exact
[ ] Dprime semantics exact
[ ] Appendix If->While exact
[ ] W/D parameter shift exact
[ ] W/D relation labelled [T-D], not [T]
[ ] no W/Dprime equivalence claim
[ ] D/Dprime bracket exact
[ ] exact-capacity distinction A1/W vs D/Dprime fixed
```

---

# 123. DEFINITION OF DONE — FINITE PRECISION

```text
[ ] BernoulliPow2 uses bits
[ ] no floating-point probability hot path
[ ] no l>52 limitation
[ ] 64-bit rank representation implemented
[ ] Cutoff64 exact
[ ] D strict cutoff invariant tested
[ ] Dprime integer threshold tested
[ ] incoming-rank=cutoff tested
[ ] multiple-max-rank purge tested
[ ] tie events recorded
[ ] finite resolution events recorded
[ ] continuous source vs finite realization separated
[ ] exact D/Dprime finite bracket predicate implemented
[ ] no floating-point bracket comparison
```

---

# 124. DEFINITION OF DONE — DATA STRUCTURES

```text
[ ] A1/W does NOT swap-delete inside hash table slots
[ ] dense vector + key->index map implemented
[ ] swap-remove reverse mapping correct
[ ] D/Dprime indexed max-heap correct
[ ] key->heap-index map correct
[ ] duplicate removal tested
[ ] heap max invariant tested
[ ] cutoff invariant tested
[ ] sanitizer clean
```

---

# 125. DEFINITION OF DONE — STATISTICS

```text
[ ] RQ1 CP endpoint handling implemented
[ ] RQ1 alpha=0.05/13 fixed
[ ] q_total_alg semantics fixed
[ ] execution-invalid rate separated
[ ] theory-consistency check blocked when execution-invalid > 0
[ ] threshold-state classifier deterministic
[ ] non_monotone_flag deterministic
[ ] H1 reportability fixed
[ ] RQ2 level convention fixed
[ ] t=0 trace snapshot
[ ] t=m trace snapshot
[ ] same-step cascade excluded from log model
[ ] run-cluster bootstrap implemented
[ ] bootstrap invalid-replicate policy fixed
[ ] H2a beta targets fixed
[ ] H2b terminal-state semantics fixed
[ ] W/D source relation tested
[ ] W/Dprime only empirical comparison
[ ] D(s=1) CLT prohibition
[ ] exact-capacity policy tested
[ ] TOST power evaluated before confirmation
[ ] TOST exact Student-t procedure fixed
[ ] TOST multiplicity scope fixed
[ ] variance ratio bootstrap fixed
[ ] KS statistic applied to RE
[ ] KS invalid-run policy fixed
[ ] permutation count fixed
[ ] Holm scopes fixed
```

---

# 126. DEFINITION OF DONE — EXPERIMENTAL DESIGN

```text
[ ] exactly two workloads
[ ] exactly four RQ
[ ] no hidden workload matrix
[ ] RQ1 grid fixed
[ ] RQ3/RQ4 matched capacity fixed
[ ] no Iso-Memory
[ ] no external benchmark family
[ ] raw schema fixed
[ ] epsilon/delta recorded explicitly
[ ] provenance fixed
[ ] seed hierarchy fixed
[ ] corrected RQ1 budget recorded
```

---

# 127. DEFINITION OF DONE — REPRODUCIBILITY

```text
[ ] clean checkout builds
[ ] tests pass
[ ] sanitizers pass
[ ] source fixtures pass
[ ] exact-capacity fixtures pass
[ ] configuration hashes recorded
[ ] binary hash recorded
[ ] source tree hash recorded
[ ] PRNG metadata recorded
[ ] workload metadata recorded
[ ] analysis environment recorded
[ ] raw schema validates
[ ] notebooks reproduce final figures
[ ] report regenerates from raw data
[ ] trace snapshots reproduce exactly
```

---

# 128. MILESTONES

# Phase 0 — Infrastructure

Deliver:

```text
C++20 core
RNG
rank representation
Cutoff64
data structures
schemas
unit tests
CI
oracle framework
```

### Gate 0

```text
build PASS
unit tests PASS
sanitizers PASS
F1-F5 PASS
exact-capacity fixtures PASS
schema PASS
```

---

# Phase 1 — A1 + RQ1

Deliver:

```text
A1
BernoulliPow2
theoretical bound generator
CP implementation
threshold classifier
non-monotone detector
RQ1 runner
```

### Gate 1

```text
all 18 theoretical values PASS
CP endpoint tests PASS
F1 PASS
RQ1 budget arithmetic PASS
no log-base inconsistency
A1 s=F0 edge fixture PASS
A1 s=F0+1 exact fixture PASS
```

---

# Phase 2 — W + Dynamics

Deliver:

```text
CVM-W
state tracing
t=0/m snapshots
reduction episodes
cascade recording
H2a
H2b
```

### Gate 2

```text
A1 semantics PASS
W semantics PASS
trace schema PASS
cluster bootstrap PASS
W exact-capacity edge semantics PASS
```

---

# Phase 3 — Knuth Algorithms

Deliver:

```text
D
Dprime
rank layer
Cutoff64
finite cutoff semantics
indexed heap
Lemma M oracle
D/Dprime coupling
```

### Gate 3

```text
D fixture PASS
Dprime semantics PASS
Lemma M PASS
coupling PASS
finite tie tests PASS
finite-resolution tests PASS
D/Dprime exact-capacity fixtures PASS
```

---

# Phase 4 — RQ3

Deliver:

```text
RQ3-Main
RQ3-Small
RQ3-Edge
W/D conformance
W/Dprime empirical comparison
D(s=1)
source fixtures
KS permutation analysis
variance bootstrap
```

### Gate 4

```text
no unexplained source-conformance failure
all inference labels correct
finite-resolution events accounted for
W/D relation indexed correctly
W/Dprime not treated as source equivalence
```

---

# Phase 5 — RQ4

Deliver:

```text
pilot
power calibration
confirmation
matched-capacity comparison
Track A
Track B
NRMSE analysis
variance analysis
TOST
throughput
```

### Gate 5

Every inferential result has:

```text
estimand
sample size
execution-valid count
CI/test
correction
effect size
interpretation
```

---

# Phase 6 — Final Freeze

Deliver:

```text
four notebooks
final figures
final tables
raw result manifest
seed manifest
reproduction log
SSOT audit
```

### Gate 6

```text
SSOT consistency audit = PASS
```

---

# 129. FINAL RQ1 CHECKLIST

```text
[ ] 2 m-values
[ ] 3 epsilon-values
[ ] 3 delta-values
[ ] 13 s-values
[ ] 234 configurations
[ ] R=1000
[ ] CP primary
[ ] alpha=0.05/13
[ ] K=0 endpoint
[ ] K=R endpoint
[ ] q_total_alg semantics
[ ] execution-invalid separation
[ ] first-certified semantics
[ ] threshold_state semantics
[ ] non_monotone_flag semantics
[ ] H1 stable-reportability
[ ] theory-consistency check blocking
[ ] theoretical table exact
[ ] corrected 234,000-run budget
[ ] A1 s=F0 edge fixture
[ ] A1 s>F0 exact fixture
```

---

# 130. FINAL RQ2 CHECKLIST

```text
[ ] A1/W only
[ ] k={64,256,1024,4096}
[ ] W s=k+1
[ ] both workloads
[ ] both m-values
[ ] R=20
[ ] state trace
[ ] t=0 snapshot
[ ] t=m snapshot
[ ] reduction episode
[ ] halving transition
[ ] level-before convention
[ ] same-step cascade excluded from log
[ ] run-cluster bootstrap
[ ] bootstrap invalid policy
[ ] workload stratification
[ ] H2b terminal-A1 abort definition
[ ] out-of-sample WL-DUP
```

---

# 131. FINAL RQ3 CHECKLIST

```text
[ ] RQ3-Main
[ ] RQ3-Small
[ ] RQ3-Edge
[ ] F1
[ ] F2
[ ] F3
[ ] F4
[ ] F5
[ ] Lemma M
[ ] D/Dprime coupling
[ ] exact finite D/Dprime bracket
[ ] W/D relation
[ ] W/Dprime only empirical comparison
[ ] finite-rank tie policy
[ ] finite-rank resolution policy
[ ] exact-capacity policy
[ ] variance bootstrap
[ ] D(s=1) CLT prohibition
[ ] KS on RE
[ ] 100,000 permutations
[ ] Holm scope fixed
```

---

# 132. FINAL RQ4 CHECKLIST

```text
[ ] k={256,1024,4096}
[ ] A1(k)
[ ] W(k+1)
[ ] D(k)
[ ] Dprime(k)
[ ] both workloads
[ ] m=10^6
[ ] q_total_alg primary
[ ] execution-invalid diagnostics
[ ] A1 conditional bias separate
[ ] W/D/Dprime unconditional bias
[ ] NRMSE
[ ] H4a definition
[ ] variance ratio
[ ] bootstrap CI
[ ] TOST exact procedure
[ ] TOST margin=0.01
[ ] pilot power
[ ] TOST multiplicity
[ ] throughput
[ ] no Iso-Memory
```

---

# 133. FINAL SCIENTIFIC POSITION

The project may make five classes of statement.

## Class A — Source theorem

Example:

$$
\boxed{
E[\hat F_0]=F_0^*
}
$$

for ideal Knuth-D/D′.

---

## Class B — Source mathematical property

Examples:

$$
\boxed{
P(\hat F_0=0)=\frac27
}
$$

for the specific D(1) fixture, or:

$$
\boxed{
p_{\mathrm{bin}}\le p<2p_{\mathrm{bin}}
}
$$

for the D/D′ source bracket.

---

## Class C — Source-derived relationship

Example:

$$
\boxed{
W(k+1)\equiv D(k)
}
$$

under project parameter normalization.

Label:

```text
[T-D]
```

---

## Class D — Project methodology

Example:

$$
\boxed{
s_{\mathrm{emp}}^{\mathrm{obs}}
}
$$

under:

```text
fixed grid
fixed workload
fixed m
fixed epsilon/delta
fixed CP protocol
```

---

## Class E — Empirical result

Example:

$$
\boxed{
\frac{s_{\mathrm{theo}}}
{s_{\mathrm{emp}}^{\mathrm{obs}}}
=
4.7
}
$$

only under the declared experiment.

No type may silently transform into another.

---

# 134. CORE SCIENTIFIC CLAIM BOUNDARY

Strongest permissible conclusion:

> Under the two controlled workloads, the declared implementation and finite-precision realization of the CVM family exhibit the empirical behavior reported by RQ1–RQ4 and show implementation-level consistency or inconsistency with the corresponding source properties of S1/S2.

Not permissible:

> The empirically observed threshold is universally sufficient.

Not permissible:

> Simulation proves the theorem.

Not permissible:

> Results on WL-UNIFORM and WL-DUP characterize arbitrary streams.

Not permissible:

> TOST passing proves unbiasedness.

Not permissible:

> Finite 64-bit rank implementation is mathematically identical to the ideal continuous-volatility algorithm.

Not permissible:

> W and D′ are source-equivalent algorithms.

Not permissible:

> \(s=F_0^*\) guarantees exactness for A1/W.

---

# 135. FINAL REPOSITORY ARCHITECTURE

```text
CVM-analysis/
│
├── include/
│   └── cvm/
│       ├── core/
│       │   ├── types.hpp
│       │   ├── concepts.hpp
│       │   ├── rng.hpp
│       │   ├── rank.hpp
│       │   ├── cutoff64.hpp
│       │   ├── stream.hpp
│       │   ├── cvm_a1.hpp
│       │   ├── cvm_w.hpp
│       │   ├── knuth_d.hpp
│       │   ├── knuth_dprime.hpp
│       │   └── diagnostics.hpp
│       │
│       └── ds/
│           ├── dense_key_set.hpp
│           ├── key_index_map.hpp
│           └── indexed_heap.hpp
│
├── src/
│
├── tests/
│   ├── unit/
│   ├── invariant/
│   ├── oracle/
│   ├── fixtures/
│   └── coupling/
│
├── benchmarks/
│
├── configs/
│
├── scripts/
│
├── analysis/
│
├── notebooks/
│   ├── 01_cvm_theoretical_bounds.ipynb
│   ├── 02_cvm_internal_dynamics.ipynb
│   ├── 03_knuth_variants_validation.ipynb
│   └── 04_final_core_comparison.ipynb
│
├── data/
│   ├── raw/
│   ├── processed/
│   └── manifests/
│
├── docs/
│
├── CMakeLists.txt
├── README.md
├── PLAN.md
└── CHANGELOG.md
```

---

# 136. VERSION HISTORY

## 3.1.2-Core-Research-Frozen-Candidate

Major corrections from 3.1.1:

1. Corrected A1/W exact-capacity condition:

   $$
   \boxed{
   s>F_0^*
   }
   $$

2. Corrected D/D′ exact-capacity condition:

   $$
   \boxed{
   s\ge F_0^*
   }
   $$

3. Added dedicated exact-capacity regression fixtures for all four algorithms.

4. Replaced ambiguous `q_total` with:

   $$
   \boxed{
   q_{\mathrm{total,alg}}
   }
   $$

   defined only over execution-valid runs.

5. Added:

   ```text
   execution_valid
   approximation_valid
   approximation_failure
   q_execution_invalid
   ```

6. Blocked theory-consistency checks whenever NumericFailure or ResourceFailure exists.

7. Separated:

   ```text
   threshold_state
   non_monotone_flag
   ```

8. Removed `IDENTIFIED_NON_MONOTONE` as a threshold state.

9. Formalized exact non-monotonicity criterion.

10. Added exact Student-t TOST specification.

11. Added TOST degenerate-sample policy.

12. Formalized bootstrap invalid-replicate policy.

13. Fixed KS variable to:

    $$
    \boxed{RE}
    $$

14. Fixed KS invalid-run handling.

15. Formalized finite D/D′ exact bracket predicate.

16. Added mandatory RQ2 trace snapshots at:

    ```text
    t=0
    t=m
    ```

17. Added exact trace representation alongside numeric \(p\).

18. Added explicit `epsilon` and `delta` to raw schema.

19. Added theoretical capacity to raw provenance.

20. Corrected source-semantics classification of:

    $$
    W(k+1)\equiv D(k)
    $$

    from [T] to [T-D].

21. Clarified D variance formula domain:

    $$
    s\ge2.
    $$

22. Distinguished ideal source semantics from the finite-rank model more explicitly.

23. Added failure modes F13–F16.

24. Updated all Definition-of-Done checklists.

25. Preserved exactly:

    ```text
    4 research questions
    2 workloads
    matched-capacity design
    core scope
    no Iso-Memory
    ```

26. Preserved all corrected resource budgets:

    $$
    \boxed{280,640\ nominal\ runs}
    $$

    and:

    $$
    \boxed{321,440\ maximum\ runs}
    $$

---

# 137. FINAL FREEZE CRITERION

`3.1.2` chỉ được chuyển từ:

```text
Core-Research-Frozen-Candidate
```

sang:

```text
Core-Research-Frozen
```

khi toàn bộ:

$$
\boxed{
\text{Source}
+
\text{Algorithm}
+
\text{Finite Precision}
+
\text{Statistics}
+
\text{Data Structures}
+
\text{Reproducibility}
}
$$

đạt PASS.

Các blocker cuối cùng:

$$
\boxed{
\begin{array}{c}
\text{A1/W data-structure correctness}\\
+\ \text{D/D′ finite-rank exact semantics}\\
+\ \text{W/D parameter-shift conformance}\\
+\ \text{RQ2 event/statistical semantics}\\
+\ \text{RQ1 CP endpoint handling}\\
+\ \text{RQ1 theory-check gating}\\
+\ \text{RQ3/RQ4 inference implementation}\\
+\ \text{exact-capacity edge semantics}
\end{array}
}
$$

Sau khi các Gate tương ứng PASS, PLAN được xem là **SSOT research specification frozen**, và mọi thay đổi research scope từ thời điểm đó phải tạo version mới.

---

# 138. PRIMARY SOURCE RECORD

**S1 — Chakraborty, Vinodchandran, Meel, arXiv:2301.10191v2.**

S1 v2 được ghi nhận ngày 24 May 2023 và chứa Problem 1, Algorithm 1, threshold, Claims 3–6, Theorem 2 và correction note về independence/unbiasedness. ([arXiv][1])

**S2 — Donald E. Knuth, *The CVM Algorithm for Estimating Distinct Elements in Streams*, revised 29 Dec 2023.**

S2 revised chứa Algorithm D, Lemma M, D′, Equation (13), các bounds/theorems liên quan, Appendix Algorithm X, và modified `If→While` construction. ([Computer Science][2])

---

[1]: https://arxiv.org/html/2301.10191v2 "Distinct Elements in Streams: An Algorithm for the (Text) Book"
[2]: https://cs.stanford.edu/~knuth/papers/cvm-note.pdf "cvm-note.dvi"
