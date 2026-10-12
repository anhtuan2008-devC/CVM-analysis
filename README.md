# CVM-Analysis

Nghiên cứu và hiện thực hóa chuẩn xác các thuật toán ước lượng độ phong phú luồng dữ liệu (Streaming Cardinality Estimation) thuộc họ thuật toán CVM theo tài liệu gốc S1 (Chakraborty et al., arXiv:2301.10191v2) và S2 (Knuth, Stanford Note 2023).

---

## 1. Thuật toán cốt lõi

Dự án hiện thực hóa 4 thuật toán streaming cardinality CVM bằng C++23:

1. **CVM-A1 (`cvm::core::CvmA1`):**
   - Hiện thực hóa chuẩn xác 5 bước A1.1 đến A1.5 theo S1 Algorithm 1.
   - Cơ chế giảm một nửa (halving) với xác suất $1/2$ và ngắt sớm (abort) khi bộ đệm đầy sau bước giảm.

2. **CVM-W (`cvm::core::CvmW`):**
   - Hiện thực hóa Algorithm X cải tiến của Knuth (S2 Appendix).
   - Vòng lặp `while |X| == s`, loại bỏ hoàn toàn cơ chế abort, bảo đảm bất biến sau cập nhật $|X| \le s - 1$.
   - Ghi nhận cascade cùng bước với $\Delta t = 0$ và `delta_t_valid = false` (không chuyển thành 1).

3. **Knuth-D (`cvm::core::KnuthD`):**
   - Thuật toán D theo S2 với ngưỡng biến thiên liên tục (continuous volatility).
   - Biểu diễn rank hữu hạn 64-bit [`Cutoff64`](include/cvm/ds/cutoff64.hpp) với vị từ số nguyên thuần túy $r < c$.
   - Duy trì bất biến nghiêm ngặt $r < \text{cutoff}$, xóa khóa trùng lặp và loại bỏ đồng thời các rank cực đại bằng nhau.

4. **Knuth-D' (`cvm::core::KnuthDprime`):**
   - Thuật toán D' ("A Coarser Method", S2 p. 5) với ngưỡng nhị phân $p = 2^{-l}$.
   - Biểu diễn số nguyên thuần túy bằng cấp độ $l \in [0, 65]$ và vị từ bit shift nguyên thủy $r < 2^{64-l}$, không dùng số thực hay logarit xấp xỉ.
   - Cơ chế thanh lọc cascade đa cấp độ và xử lý giới hạn phân giải $l \ge 65$ trả về `NumericFailure`.

---

## 2. Cấu trúc dữ liệu

- **`KeyIndexMap` (`cvm::ds::KeyIndexMap`):** Bảng băm định địa chỉ mở (open addressing, linear probing), hệ số tải $\le 0.5$, băm SplitMix64, cơ chế xóa dồn ngược Knuth 6.4R (tombstone-free).
- **`DenseKeySet` (`cvm::ds::DenseKeySet`):** Mảng liên tục `std::vector<Key>` kết hợp ánh xạ ngược, hỗ trợ thao tác Swap-Remove trong thời gian $O(1)$ và bảo đảm 0 cấp phát bộ nhớ động sau `reserve()`.
- **`IndexedHeap` (`cvm::ds::IndexedHeap`):** Hàng đợi ưu tiên max-heap kết hợp ánh xạ khóa sang chỉ mục heap, hỗ trợ tra cứu khóa $O(1)$, xóa phần tử bất kỳ $O(\log s)$ và thanh lọc đa phần tử cực đại.
- **`Cutoff64` (`cvm::ds::Cutoff64`):** Biểu diễn ngưỡng cắt hữu hạn 64-bit với hai trạng thái `ONE` ($p=1$) và `RANK(c)` ($p = (c + 0.5)/2^{64}$).

---

## 3. Cấu trúc thư mục

```text
.
├── CMakeLists.txt              # Cấu hình biên dịch CMake C++23
├── CHANGELOG.md                # Lịch sử thay đổi theo chuẩn Keep a Changelog
├── README.md                   # Tài liệu tổng quan dự án
├── .gitignore                  # Cấu hình loại trừ file rác, build artifacts, cache
├── docs/                       # Tài liệu đặc tả và kiểm toán
│   ├── AGENTS.md               # Quy chuẩn thực thi có cổng kiểm soát
│   ├── PLAN.md                 # Nguồn chân lý duy nhất (SSOT v3.1.2)
│   ├── CVM-paper.md            # Tài liệu phân tích toán học lý thuyết
│   └── agent-execution/        # Nhật ký kiểm toán, quyết định và truy xuất Git
├── include/cvm/                # Mã nguồn header-only C++23
│   ├── core/                   # Các thuật toán CVM và hạ tầng lõi
│   └── ds/                     # Cấu trúc dữ liệu chuyên biệt (DenseKeySet, IndexedHeap)
└── tests/                      # Bộ kiểm thử đơn vị, kiểm thử đối kháng và fixture
    ├── test_utils.hpp
    └── unit/                   # Các bài test độc lập từ CASE-02 đến CASE-09
```

---

## 4. Hướng dẫn biên dịch và kiểm thử

### Yêu cầu môi trường
- Trình biên dịch C++ hỗ trợ C++23 (GCC 14+ hoặc Clang 18+)
- CMake 3.20+

### Biên dịch và chạy toàn bộ kiểm thử
```bash
# Cấu hình bản dựng Release
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Biên dịch song song
cmake --build build -j

# Chạy toàn bộ test suites qua CTest
ctest --test-dir build --output-on-failure
```

### Chạy kiểm tra an toàn bộ nhớ (ASan & UBSan)
```bash
cmake -B build_san -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -g"
cmake --build build_san -j
ctest --test-dir build_san --output-on-failure
```
