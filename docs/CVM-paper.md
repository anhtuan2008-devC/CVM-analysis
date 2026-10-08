# I. Nguồn

https://cs.stanford.edu/~knuth/papers/cvm-note.pdf

# II. Nội dung của Thuật toán CVM để ước lượng các phần tử phân biệt trong luồng dữ liệu

Sourav Chakraborty, N. V. Vinodchandran và Kuldeep S. Meel [2] gần đây đã đề xuất một thuật toán thú vị cho bài toán sau: Một luồng các phần tử ($a_1, a_2, \dots, a_m$) được đưa vào, từng phần tử một tại mỗi thời điểm, và chúng ta muốn biết có bao nhiêu phần tử trong số đó là phân biệt. Nói cách khác, nếu $A = \{a_1, a_2, \dots, a_m\}$ là tập hợp các phần tử trong luồng dữ liệu, bỏ qua số lần lặp lại, chúng ta muốn biết $\vert{}A\vert{}$, tức kích thước của tập hợp đó. Nhưng chúng ta không có nhiều bộ nhớ; trên thực tế, $\vert{}A\vert{}$ có thể lớn hơn rất nhiều so với số lượng phần tử mà chúng ta có thể lưu trữ trong bộ nhớ tại bất kỳ thời điểm nào. Đâu là một chiến lược tốt để tính toán một ước lượng không chệch của $\vert{}A\vert{}$?

Thuật toán của họ không chỉ thú vị mà còn cực kỳ đơn giản. Hơn nữa, nó rất thích hợp để giảng dạy cho các sinh viên đang học những kiến thức cơ bản về khoa học máy tính. (Thực vậy, kể từ khi tôi nhìn thấy nó vài ngày trước, tôi đã không thể cưỡng lại việc cố gắng giải thích những ý tưởng này cho hầu như bất kỳ ai mà tôi gặp.) Vì vậy, tôi khá chắc chắn rằng một điều tương tự như thế này rốt cuộc sẽ trở thành một chủ đề chuẩn mực trong sách giáo khoa. Ghi chú này là một phác thảo ban đầu cho những gì tôi có thể viết về nó nếu tôi đang chuẩn bị một cuốn sách giáo khoa về luồng dữ liệu.

Tất nhiên, mọi thuật toán quan trọng đều cần một cái tên. Tiếp nối một tiền lệ đã có từ trước đối với các quy trình có ba tác giả khác, chẳng hạn như thuật toán AKS, thuật toán KMP và thuật toán LLL, tôi sẽ gọi nó là thuật toán CVM trong ghi chú này.

Gọi $A_t = \{a_1, \dots, a_t\}$ là tập hợp $t$ phần tử đầu tiên của luồng dữ liệu, bỏ qua số lần lặp lại. Thuật toán CVM duy trì một bộ đệm có kích thước giới hạn nhằm lưu giữ một tập hợp con được chọn ngẫu nhiên từ các phần tử đó; gọi $B_t$ là nội dung của bộ đệm đó tại thời điểm $t$. Thuật toán cũng duy trì một số $p_t$ sao cho tính chất sau luôn được thỏa mãn:

$$
\Pr(a_j \in B_t) = p_t, \quad \text{với } 1 \le j \le t. \tag{1}
$$

Từ đó suy ra rằng giá trị kỳ vọng của $\vert{}B_t\vert{}$ là $p_t \vert{}A_t\vert{}$; nói cách khác,

$$
\vert{}B_t\vert{} / p_t \text{ là một ước lượng không chệch của } \vert{}A_t\vert{}. \tag{2}
$$

(Tình huống trên thực tế tinh tế hơn so với những gì phương trình không chính thức (1) gợi ý, bởi vì vế trái của (1) là một xác suất trong khi vế phải là một biến ngẫu nhiên! Tuy nhiên, chúng ta sẽ thấy rằng các ý tưởng này có thể được chứng minh hợp lý và (2) là hoàn toàn chính xác 100%.)

Thuật toán trong [2] được thiết kế sao cho tính chất (1) sẽ là một bất biến. Nhưng công thức ban đầu chứa một lỗi tinh vi, vốn được thảo luận trong phần phụ lục bên dưới. May mắn thay, có một cách đơn giản để tránh vấn đề đó, và tôi sẽ trình bày một thuật toán đã được sửa đổi tại đây. Thuật toán D là thuật toán mà tôi đề xuất gọi là thuật toán CVM, và tôi tin rằng nó thực sự là một ứng viên sáng giá để đưa vào các sách giáo khoa trong tương lai.

Có một tham số, $s$, biểu thị kích thước bộ đệm tối đa. Nó có thể là bất kỳ số nguyên dương nào. Nếu bộ đệm bị tràn ở bước thứ $t$—nghĩa là nếu $\vert{}B_t\vert{} = s$ và chúng ta muốn chèn thêm một phần tử khác—chúng ta sẽ xóa ngẫu nhiên một trong số $s + 1$ phần tử đó, và điều chỉnh $p_t$ sao cho (1) vẫn đúng. Việc lựa chọn $p_t$ một cách thích hợp đòi hỏi sự cẩn trọng; chúng ta sử dụng các giá trị ngẫu nhiên đều (các phân số thực ngẫu nhiên đều) cho mục đích này.

**Thuật toán D (Ước lượng phần tử phân biệt).** Cho một luồng dữ liệu tùy ý $A = (a_1, a_2, \dots, a_m)$ và một kích thước bộ đệm $s \ge 1$ như đã mô tả ở trên, thuật toán này trả về một ước lượng không chệch của $\vert{}A\vert{} = \vert{}\{a_1, a_2, \dots, a_m\}\vert{}$. Thuật toán sử dụng một bộ đệm $B$ có khả năng chứa tối đa $s$ cặp có thứ tự $(a, u)$, trong đó $a$ là một phần tử của luồng và $u$ là một số thực, $0 \le u < 1$.

- **D1. [Khởi tạo.]** Đặt $t \leftarrow 0$, $p \leftarrow 1$, và $B \leftarrow \emptyset$.
- **D2. [Hoàn tất?]** Kết thúc nếu $t = m$, trả về ước lượng $\vert{}B\vert{}/p$.
- **D3. [Nhập $a$.]** Đặt $t \leftarrow t + 1$ và $a \leftarrow a_t$, phần tử tiếp theo của luồng dữ liệu.
- **D4. [Xóa $a$ khỏi $B$.]** Nếu $B$ chứa cặp $(a, u)$, với bất kỳ giá trị $u$ nào, hãy xóa nó.
- **D5. [Có thể đưa $a$ vào $B$.]** Gọi $u$ là một giá trị ngẫu nhiên đều, độc lập với tất cả các giá trị khác (cụ thể là một số thực ngẫu nhiên trong khoảng $0 \le u < 1$). Nếu $u \ge p$, quay lại bước D2. Ngược lại, nếu $\vert{}B\vert{} < s$, chèn $(a, u)$ vào $B$ và quay lại bước D2. Ngược lại, gọi $(a', u')$ là một phần tử trong $B$ có $u'$ lớn nhất. Nếu $u > u'$, đặt $p \leftarrow u$. Ngược lại, thay thế $(a', u')$ trong $B$ bằng $(a, u)$ và đặt $p \leftarrow u'$. Sau đó quay lại bước D2. **|**

**Một ví dụ.** Để làm quen với thuật toán này, chúng ta hãy quan sát nó ở tốc độ chậm trong một trong những trường hợp đơn giản nhất có thể. Giả sử rằng $s = 1$, và luồng dữ liệu đầu vào chỉ bao gồm $m = 3$ phần tử phân biệt ($a_1, a_2, a_3$). Vì ban đầu $p$ bằng 1 và $B$ rỗng, bước D5 sẽ chèn $(a_1, u_1)$ vào $B$, trong đó $u_1$ là một giá trị ngẫu nhiên đều.

Tuy nhiên, lần tiếp theo khi chúng ta đến bước D5, chúng ta sẽ có $(a, u) = (a_2, u_2)$, trong đó $u_2 < p = 1$ nhưng $\vert{}B\vert{} = 1 = s$. Do đó bộ đệm đã đầy, và chúng ta sẽ chuyển sang bước D6 lần đầu tiên. Nếu $u_1 < u_2$, bước D6 sẽ đặt $p \leftarrow u_2$; ngược lại, nó sẽ đặt $p \leftarrow u_1$ và $B \leftarrow \{(a_2, u_2)\}$.

Và khi chúng ta đến bước D5 với $t = 3$ và $(a, u) = (a_3, u_3)$, các khả năng sẽ nhân lên. Sáu kịch bản có thể xảy ra trước khi kết thúc, tùy thuộc vào thứ tự tương đối giữa $u_1, u_2$ và $u_3$: Hoặc là

- $u_1 < u_2 < u_3$ và $p = u_2$ cùng $B = \{(a_1, u_1)\}$; hoặc
- $u_1 < u_3 < u_2$ và $p = u_3$ cùng $B = \{(a_1, u_1)\}$; hoặc
- $u_2 < u_1 < u_3$ và $p = u_1$ cùng $B = \{(a_2, u_2)\}$; hoặc
- $u_2 < u_3 < u_1$ và $p = u_3$ cùng $B = \{(a_2, u_2)\}$; hoặc
- $u_3 < u_1 < u_2$ và $p = u_1$ cùng $B = \{(a_3, u_3)\}$; hoặc
- $u_3 < u_2 < u_1$ và $p = u_2$ cùng $B = \{(a_3, u_3)\}$.

Mỗi trường hợp trong số sáu trường hợp này đều có khả năng xảy ra như nhau. Và trong mỗi trường hợp, $B$ lưu giữ duy nhất một cặp $(a_j, u_j)$, trong đó $u_j$ là giá trị nhỏ nhất của $\{u_1, u_2, u_3\}$. Hơn nữa, $p$ luôn là giá trị nhỏ *thứ hai* của $\{u_1, u_2, u_3\}$.

Chúng ta sẽ thấy ngay sau đây rằng giá trị trung bình của $1/p$ là 3 trong mỗi trường hợp (mặc dù giá trị trung bình của chính $p$ là $1/2$). Vì vậy kết quả được trả về bởi thuật toán, cụ thể là $\vert{}B\vert{}/p$, là không chệch: Nó có giá trị kỳ vọng chính xác, cụ thể là $\vert{}A\vert{} = 3$.

Bây giờ chúng ta hãy tạo một thay đổi nhỏ. Giả sử $a_2 \ne a_1$, như trước, nhưng $a_3 = a_1$. Do đó $\vert{}A\vert{}$ bây giờ bằng 2. Một lần nữa thuật toán sẽ dựa tính toán của nó trên ba giá trị ngẫu nhiên đều, $u_1, u_2$ và $u_3$; và không khó để thấy rằng sáu kết quả đồng khả năng bây giờ sẽ là hoặc

- $u_1 < u_2 < u_3$ và $p = u_2$ cùng $B = \emptyset$; hoặc
- $u_1 < u_3 < u_2$ và $p = u_2$ cùng $B = \{(a_3, u_3)\}$; hoặc
- $u_2 < u_1 < u_3$ và $p = u_1$ cùng $B = \{(a_2, u_2)\}$; hoặc
- $u_2 < u_3 < u_1$ và $p = u_3$ cùng $B = \{(a_2, u_2)\}$; hoặc
- $u_3 < u_1 < u_2$ và $p = u_2$ cùng $B = \{(a_3, u_3)\}$; hoặc
- $u_3 < u_2 < u_1$ và $p = u_2$ cùng $B = \{(a_3, u_3)\}$.

Lần này có sự đa dạng hơn nhiều so với trước. Giá trị sẽ được trả về ở bước D2 hóa ra là

$$
0/U_{3,2} \text{ hoặc } 1/U_{3,3} \text{ hoặc } 1/U_{3,2} \text{ hoặc } 1/U_{3,2} \text{ hoặc } 1/U_{3,3} \text{ hoặc } 1/U_{3,2}, \tag{3}
$$

mỗi giá trị với xác suất $1/6$, trong đó $U_{m,k}$ là một biến ngẫu nhiên:

$$
U_{m,k} = \text{giá trị nhỏ thứ } k \text{ trong số } m \text{ giá trị ngẫu nhiên đều độc lập.} \tag{4}
$$

Phân phối xác suất của $U_{m,k}$ được biết đến rộng rãi là “phân phối beta với các tham số $k$ và $m+1-k$”, cụ thể là

$$
\Pr(U_{m,k} \le x) = k \binom{m}{k} \int_0^x t^{k-1}(1-t)^{m-k} \, dt \quad \text{với } 0 \le x \le 1; \tag{5}
$$

điều này có thể được chứng minh, chẳng hạn, bằng nguyên lý bù trừ (inclusion and exclusion). Do đó

$$
E(1/U_{m,k}) = k \binom{m}{k} \int_0^1 t^{k-2}(1-t)^{m-k} \, dt = \frac{m}{k-1}, \quad \text{với } 1 < k \le m. \tag{6}
$$

(Một lập luận tương tự chỉ ra rằng $E[U_{m,k}] = k/(m+1)$.) Do đó, giá trị kỳ vọng của (3) trở thành

$$
\frac{1}{6} \left( 0 \cdot \frac{3}{1} + 1 \cdot \frac{3}{2} + 1 \cdot \frac{3}{1} + 1 \cdot \frac{3}{1} + 1 \cdot \frac{3}{2} + 1 \cdot \frac{3}{1} \right) = 2, \tag{7}
$$

đúng như kỳ vọng. Thuật toán D hoạt động tốt trong trường hợp này!