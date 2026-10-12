# CVM-A1 Standalone Benchmark & Analysis Module (`testcode/`)

Thư mục này chứa toàn bộ mã nguồn cài đặt chuẩn mực, tối ưu hóa hiệu năng cao cho **Thuật toán CVM Algorithm 1** (*Chakraborty, Vinodchandran, Meel – ESA 2023* / *Donald Knuth 2023*), kèm theo công thức tính bộ đệm lý thuyết (**Theorem 2**), bộ sinh testcase, công cụ đo đạc thực nghiệm 40 testcase và script vẽ đồ thị tự động.

---

## 1. Cấu trúc thư mục (Directory Structure)

- **`CVM_a1.hpp`**: Mã nguồn C++ cài đặt Algorithm 1 với cấu trúc Hash Index $O(1)$, hỗ trợ cả 2 chế độ khởi tạo: truyền dung lượng $s$ thực nghiệm hoặc tự động tính $s$ theo Theorem 2.
- **`CVM_a1.cpp`**: Chương trình demo kiểm tra hoạt động cơ bản.
- **`generate_testcase.cpp`**: Bộ sinh dữ liệu luồng hỗ trợ phân phối đều (`uniform`), phân phối lũy thừa lệch (`zipf`), và lặp tuần hoàn (`repeat`).
- **`run_testcase.cpp`**: Bộ đo hiệu năng đơn lẻ (thời gian chạy, throughput, giá trị ước lượng $\hat{F}_0$, sai số tương đối so với Ground Truth $F_0$).
- **`benchmark_5_tests.cpp`**: Bộ kiểm thử tự động 5 testcase lớn ($N = 700\text{k} - 1\text{M}$).
- **`benchmark_30_tests.cpp`**: Bộ kiểm thử tự động 30 testcase qua 3 nhóm quy mô với $s$ tính theo Theorem 2.
- **`benchmark_extra_10_tests.cpp`**: Bộ kiểm thử mở rộng 10 testcase siêu lớn ($N = 5\text{M} - 10\text{M}$ phần tử).
- **`benchmark_results_30tests.txt`**: File nhật ký lưu kết quả đầy đủ của toàn bộ 40 testcase.
- **`plot_benchmarks.py`**: Script Python tự động phân tích file kết quả và vẽ 6 đồ thị chuẩn học thuật (300 DPI).
- **`charts/`**: Thư mục chứa 6 hình ảnh đồ thị phân tích đã xuất ra.
- **`Makefile`**: Script biên dịch tự động cho toàn bộ dự án.

---

## 2. Chi tiết từng bước thuật toán hoạt động (Algorithm Walkthrough)

### 💡 Mô hình trực quan: Chiếc xô có sức chứa $s$ quả bóng
Hãy tưởng tượng bộ đệm $X$ như một **chiếc xô nước có sức chứa tối đa $s$ quả bóng**:
1. Mỗi khi một quả bóng mới $a_t$ đến, ta kiểm tra xem trong xô đã có quả bóng nào giống hệt chưa. Nếu có $\to$ **vứt quả bóng cũ đi**.
2. Ta tung đồng xu để quyết định có nhận quả bóng mới vào xô hay không với xác suất $p$.
3. Khi xô **đầy tràn** ($|X| = s$ quả bóng), ta **tung đồng xu công bằng cho từng quả bóng trong xô**: giữ lại nếu ra Ngửa ($1/2$), vứt đi nếu ra Sấp ($1/2$). Xô lập tức vơi đi khoảng $50\%$.
4. Đồng thời, ta **bóp hẹp miệng xô lại một nửa** ($p \leftarrow p / 2$): từ nay về sau, bóng mới muốn lọt vào xô cũng khó gấp đôi!
5. Khi dòng dữ liệu kết thúc, mỗi quả bóng còn lại trong xô đại diện cho đúng $2^l$ quả bóng ngoài đời thực.

---

### 🔄 Sơ đồ khối quy trình thực thi (Execution Flowchart)

```
                       [ Phần tử luồng đến: a_t ]
                                   │
                                   ▼
               ┌───────────────────────────────────────┐
               │ Bước 1: Xóa phần tử cũ nếu đã có     │
               │   X <- X \ {a_t}   (O(1) qua Map)     │
               └───────────────────┬───────────────────┘
                                   │
                                   ▼
               ┌───────────────────────────────────────┐
               │ Bước 2: Lấy mẫu Bernoulli             │
               │   Thêm a_t vào X với prob p = 2^-l    │
               └───────────────────┬───────────────────┘
                                   │
                                   ▼
                           [ Bộ đệm |X| == s? ]
                                 /   \
                               Có     Không ──> (Chờ phần tử tiếp theo)
                               /
                              ▼
           ┌───────────────────────────────────────────────┐
           │ Bước 3: Thanh lọc 50% khi đệm đầy             │
           │   Mỗi phần tử trong X: giữ lại với prob 1/2   │
           └───────────────────────┬───────────────────────┘
                                   │
                                   ▼
           ┌───────────────────────────────────────────────┐
           │ Bước 4: Giảm xác suất tương lai               │
           │   p <- p / 2   (tương đương l <- l + 1)       │
           └───────────────────────┬───────────────────────┘
                                   │
                                   ▼
                        [ Vẫn đầy? |X| == s ]
                                 /   \
                               Có     Không ──> (Chờ phần tử tiếp theo)
                               /
                              ▼
                    ┌──────────────────┐
                    │ Bước 5: ABORT!   │
                    └──────────────────┘
```

---

### Chi tiết 3 Giai đoạn thực hiện

#### Giai đoạn 1: Khởi tạo (Initialization)
- Bộ đệm $X \leftarrow \varnothing$ (rỗng).
- Số vòng giảm nửa $l \leftarrow 0$.
- Xác suất lấy mẫu $p \leftarrow 2^{-0} = 1$ ($100\%$ nhận vào ban đầu).
- Sức chứa tối đa của bộ đệm là $s$ (truyền vào trực tiếp hoặc tính theo Theorem 2).

#### Giai đoạn 2: Vòng lặp xử lý từng phần tử $a_t$ ($t = 1, 2, \dots, m$)

- **Bước 1 (Dòng 3 Paper): Xóa phần tử cũ nếu đã có ($X \leftarrow X \setminus \{a_t\}$)**
  - *Hành động*: Kiểm tra xem $a_t$ đã từng nằm trong bộ đệm $X$ hay chưa. Nếu có $\to$ xóa nó ngay lập tức.
  - *Triển khai C++*: Dùng bảng băm `pos_` ánh xạ `Key -> vị trí trong mảng`. Thao tác tìm và xóa (swap-pop với phần tử cuối) tốn thời gian **$\mathcal{O}(1)$**.
  - *Ý nghĩa*: **Xóa sạch dấu vết quá khứ**. Chỉ có lần xuất hiện cuối cùng của một phần tử mới có giá trị đại diện. Mọi lần xuất hiện trước đó đều bị loại bỏ, bảo toàn tính phân biệt.

- **Bước 2 (Dòng 4 Paper): Lấy mẫu ngẫu nhiên Bernoulli**
  - *Hành động*: Thực hiện phép thử Bernoulli với xác suất $p = 2^{-l}$.
  - *Triển khai C++*: Hàm `bernoulli_pow2(l)` tung $l$ đồng xu công bằng độc lập. Nếu cả $l$ đồng xu đều ra mặt $0$ (xác suất trúng $(1/2)^l$) $\to$ nạp $a_t$ vào $X$ và lưu vị trí vào `pos_`. Nếu trượt $\to$ bỏ qua $a_t$.
  - *Ý nghĩa*: Càng về sau, xác suất nhận một phần tử mới càng giảm theo cấp số nhân ($1 \to 1/2 \to 1/4 \to 1/8 \dots$).

- **Bước 3 (Dòng 5–6 Paper): Thanh lọc bộ đệm khi đầy tràn ($|X| == s$)**
  - *Hành động*: Khi số phần tử trong $X$ chạm ngưỡng dung lượng $s$, tung đồng xu công bằng cho từng phần tử trong $X$.
  - *Triển khai C++*: Duyệt qua $s$ phần tử, sinh bit ngẫu nhiên bằng `next_bit()`. Bit $0$ $\to$ giữ lại; Bit $1$ $\to$ loại bỏ.
  - *Ý nghĩa*: Giải phóng ngay lập tức $\approx 50\%$ dung lượng RAM, số phần tử trong đệm vơi từ $s$ xuống còn khoảng $\approx s/2$.

- **Bước 4 (Dòng 7 Paper): Cập nhật xác suất tương lai ($p \leftarrow p / 2 \iff l \leftarrow l + 1$)**
  - *Hành động*: Tăng biến đếm vòng halving: `++l_`.
  - *Ý nghĩa*: Vì các phần tử cũ vừa phải vượt qua đợt thanh lọc $50\%$, các phần tử mới đến sau này cũng phải chịu xác suất nhận giảm đi một nửa để **đảm bảo tính công bằng xác suất trên toàn bộ luồng dữ liệu**.

- **Bước 5 (Dòng 8 Paper): Kiểm tra điều kiện thất bại (Abort Check)**
  - *Hành động*: Nếu sau Bước 3 mà toàn bộ $s$ phần tử đều may mắn sống sót ($|X| == s$), thuật toán dừng và báo lỗi `Abort`.
  - *Ý nghĩa*: Xác suất xảy ra là $(1/2)^s$ (với $s \ge 1000$ thì con số này gần như bằng 0). Đảm bảo thuật toán không bị rơi vào vòng lặp vô hạn.

#### Giai đoạn 3: Tính toán kết quả ước lượng cuối cùng (End of Stream)
Khi dòng dữ liệu kết thúc ($t = m$):

$$\hat{F}_0 = \frac{|X|}{p} = |X| \times 2^l$$

- *Bản chất thống kê*: Mỗi phần tử sống sót trong bộ đệm $X$ đã phải vượt qua $l$ lần thử thách xác suất độc lập (xác suất sống là $2^{-l}$). Vì vậy, **mỗi 1 phần tử hiện diện trong $X$ đại diện cho kỳ vọng đúng $2^l$ phần tử phân biệt ngoài thực tế**.
- Bộ ước lượng này là **không chệch (Unbiased Estimator)**: $\mathbb{E}[\hat{F}_0] = F_0$.

---

### Bảng đối chiếu từng dòng Paper vs Mã nguồn C++ (`CVM_a1.hpp`)

| Dòng trong Paper | Ký hiệu Toán học | Thao tác trong mã C++ (`CVM_a1.hpp`) | Ý nghĩa hoạt động |
| :--- | :---: | :--- | :--- |
| **Khởi tạo** | $X \leftarrow \varnothing, p \leftarrow 1$ | `buffer_.clear(); pos_.clear(); l_ = 0;` | Dọn trống RAM, xác suất ban đầu 100%. |
| **Dòng 3 (A1.1)** | $X \leftarrow X \setminus \{a_t\}$ | `pos_.find(...) + pop_back()` trong $\mathcal{O}(1)$ | Xóa phần tử cũ nếu đã có sẵn trong đệm. |
| **Dòng 4 (A1.2)** | Sample with prob $p$ | `if (bernoulli_pow2(l_)) buffer_.push_back(...)` | Thêm vào với xác suất $p = 2^{-l}$. |
| **Dòng 5–6 (A1.3)** | Halving khi $|X| = s$ | Duyệt mảng với `next_bit()` | Xô đầy $\to$ loại bỏ ngẫu nhiên 50% phần tử. |
| **Dòng 7 (A1.4)** | $p \leftarrow p / 2$ | `++l_;` | Giảm xác suất lấy mẫu cho tương lai đi 1 nửa. |
| **Dòng 8 (A1.5)** | Abort check | `if (buffer_.size() == s_) aborted_ = true;` | Đề phòng trường hợp xô không vơi được quả nào. |
| **Dòng 9** | Return $|X| / p$ | `return buffer_.size() * std::ldexp(1.0, l_);` | Trả về $\hat{F}_0 = |X| \times 2^l$. |

---

## 3. Định lý lý thuyết Theorem 2 & Công thức tính bộ đệm

Theorem 2 trong bài báo của Chakraborty et al. (ESA 2023) chứng minh thuật toán bảo đảm xấp xỉ $(\varepsilon, \delta)$:
$$\Pr\left[|\hat{F}_0 - F_0| \le \varepsilon F_0\right] \ge 1 - \delta$$

### Công thức tính ngưỡng bộ đệm lý thuyết:
$$s = \left\lceil \frac{12}{\varepsilon^2} \log_2\left(\frac{8m}{\delta}\right) \right\rceil$$

Trong đó:
- $\varepsilon \in (0, 1)$: Sai số tương đối chấp nhận được (ví dụ: $0,20$ cho $20\%$).
- $\delta \in (0, 1)$: Xác suất thất bại (ví dụ: $0,10$ cho độ tin cậy $90\%$).
- $m$: Cận trên của độ dài luồng dữ liệu ($m = N$).

### Độ phức tạp Bộ nhớ vs Thời gian (Space vs Time Complexity)

| Tiêu chí | Chứng minh Lý thuyết (Theorem 2) | Triển khai trong `CVM_a1.hpp` |
| :--- | :--- | :--- |
| **Space Complexity** | $\mathcal{O}\left(\frac{\log n}{\varepsilon^2} \left(\log m + \log \frac{1}{\delta}\right)\right)$ bits | Chuẩn $\mathcal{O}(s)$ tuyệt đối; RAM không vượt quá $s$ phần tử. |
| **Time per Item** | Giả định thao tác trừ tập hợp $\mathcal{O}(1)$ | **$\mathcal{O}(1)$ trung bình** nhờ bảng băm `pos_`. |
| **Total Stream Time** | Không bị chặn bởi Theorem 2 | **$\mathcal{O}(m)$ tuyến tính tối ưu**, đạt **$>25\text{ triệu items / giây}$**. |

---

## 4. Mô hình "Tam giác vàng" ảnh hưởng đến độ tối ưu

Hiệu năng và độ chính xác của CVM-A1 được quyết định bởi 3 yếu tố tương tác chặt chẽ:

```
                       [ 1. Dung lượng bộ đệm (s) ]
                          (Độ phân giải thước đo)
                                /        \
                               /          \
                              /            \
 [ 2. Kích thước luồng (N) ]  --------------  [ 3. Tỷ lệ trùng lặp (N / F0) ]
     (Độ hội tụ số lớn)                           (Cơ chế dọn dẹp đệm A1.1)
```

1. **Dung lượng bộ đệm ($s$) — Độ phân giải**:
   - Sai số chuẩn tỉ lệ với $\sigma_{\text{rel}} \approx \frac{1}{\sqrt{s}}$.
   - Ngưỡng trần của vòng $l$: $\hat{F}_{\max} = s \cdot 2^l$.
   - **Vùng ranh giới Halving**: Khi $F_0$ vượt qua $s \cdot 2^{l-1}$, thuật toán bị ép nhảy sang vòng $l$, bước nhảy nhân đôi ($2^l$).
2. **Tần suất trùng lặp ($N / F_0$) — Cơ chế dọn dẹp đệm**:
   - Bước A1.1 liên tục xóa các phần tử cũ xuất hiện lại, giải phóng chỗ trống và hoãn việc phải thanh lọc giảm nửa non.
3. **Kích thước luồng ($N$) — Hội tụ theo Luật số lớn**:
   - Khi $N$ đủ lớn, các sai số ngẫu nhiên của các lần tung đồng xu tự triệt tiêu lẫn nhau, kéo ước lượng hội tụ sát về $F_0$.

---

## 5. Hướng dẫn Biên dịch và Sử dụng

### Biên dịch toàn bộ
```bash
cd testcode
make all
```

### Chạy kiểm thử tùy biến
```bash
# Cú pháp: ./run_testcase [file_testcase] [buffer_s] [seed]
./run_testcase my_test.txt 5000 42

# Sinh luồng dữ liệu tùy biến:
# Cú pháp: ./generate_testcase [file] [total_N] [distinct_F0] [distribution]
./generate_testcase testcase.txt 1000000 50000 uniform
```

---

## 6. Kết quả Thực nghiệm 40 Testcase (Theo công thức Paper)

Bộ kiểm thử gồm **40 testcase** với Ground Truth cố định $F_0 = 50.000$, buffer $s$ tính theo Theorem 2 ($\varepsilon = 0,20, \delta = 0,10$):

1. **Nhóm 1 (Test 1–10)**: Quy mô nhỏ ($N = 300\text{k} - 750\text{k}$, $s = 7355 - 7752$).
2. **Nhóm 2 (Test 11–20)**: Quy mô tối ưu ($N = 800\text{k} - 1.1\text{M}$, $s = 7780 - 7918$).
3. **Nhóm 3 (Test 21–30)**: Quy mô lớn ($N = 1.2\text{M} - 1.7\text{M}$, $s = 7955 - 8106$).
4. **Nhóm 4 (Test 31–40)**: Quy mô siêu lớn ($N = 5\text{M} - 10\text{M}$, $s = 8573 - 8873$).

Toàn bộ số liệu chi tiết được lưu trong file [`benchmark_results_30tests.txt`](benchmark_results_30tests.txt).

### Bảng tổng hợp so sánh 4 Nhóm

| Nhóm | Dải phần tử ($N$) | Buffer $s$ (Paper) | Ground Truth $F_0$ | Sai số TB | Thời gian TB | Tốc độ xử lý | Trạng thái |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **Nhóm 1 (Nhỏ)** | $300\text{k} - 750\text{k}$ | $7355 - 7752$ | $50.000$ | **$0,99\%$** | $21,9\text{ ms}$ | $23,78\text{ triệu items/s}$ | `SUCCESS` |
| **Nhóm 2 (Tối ưu)** | $800\text{k} - 1.1\text{M}$ | $7780 - 7918$ | $50.000$ | ⭐ **$0,55\%$** | $38,8\text{ ms}$ | $24,44\text{ triệu items/s}$ | `SUCCESS` |
| **Nhóm 3 (Lớn)** | $1.2\text{M} - 1.7\text{M}$ | $7955 - 8106$ | $50.000$ | **$0,92\%$** | $59,0\text{ ms}$ | $24,28\text{ triệu items/s}$ | `SUCCESS` |
| **Nhóm 4 (Siêu lớn)**| $5.0\text{M} - 10.0\text{M}$ | $8573 - 8873$ | $50.000$ | **$0,66\%$** | $291,9\text{ ms}$ | **$25,06\text{ triệu items/s}$** | `SUCCESS` |

---

## 7. Xuất Đồ thị Phân tích Tự động (Python Visualizer)

Để vẽ và xuất toàn bộ 6 đồ thị chuẩn học thuật phục vụ báo cáo:
```bash
python3 plot_benchmarks.py
```

Các hình ảnh xuất ra được lưu tại thư mục [`charts/`](charts/):
- **`chart1_cardinality_estimation.png`**: Giá trị ước lượng $\hat{F}_0$ so với Ground Truth $F_0 = 50.000$.
- **`chart2_relative_error_vs_N.png`**: Sai số tương đối theo kích thước luồng $N$ kèm đường chặn Theorem 2.
- **`chart3_execution_time_linearity.png`**: Thời gian thực thi tuyến tính $\mathcal{O}(N)$.
- **`chart4_group_error_comparison.png`**: So sánh sai số giữa 4 nhóm quy mô.
- **`chart5_distribution_impact.png`**: So sánh tác động của phân phối Uniform vs Zipf.
- **`chart_dashboard_all_in_one.png`**: Master Dashboard tổng hợp 4 biểu đồ trong 1 ảnh độ phân giải cao.
