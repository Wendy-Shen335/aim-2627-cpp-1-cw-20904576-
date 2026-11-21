# Q5 网格邻域风险图 · 教程

> 依据：`PROBLEM.md` 的 Q5 小节（与 `题面.pdf` 同源）。本教程只讲 Q5，每条结论都能对应回题面的编号规范。
> 文件位置：放在 `local_tests/`。该目录在 CI 白名单内（`tools/ci_support.py` 的 `ALLOWED` 正则含 `local_tests/.+`）。

---

## 0 一句话结论

题目要的是：**把字符地图转成同尺寸整数矩阵——障碍格填 -1，空地格填它上/下/左/右四个方向中障碍格的个数**。

两个考点：

1. **先验证完整张地图，再计算**。任何一处非法（零列、行长不一致、非法字符）都抛 `std::invalid_argument`，且不能先返回部分结果。
2. **只数四邻域，越界不算**。斜对角不算障碍，地图边缘也不会因为"外面是墙"而增加风险——单格空地的结果是 0，不是 4。

---

## 1 先划边界：哪里有改动权

| 位置 | 能否修改 | 说明 |
| --- | --- | --- |
| `src/main/A1_q5.cpp` 中 `// BEGIN IMPLEMENTATION` 与 `// END IMPLEMENTATION` 之间 | 可以 | 唯一允许写实现的地方 |
| 同文件的 `namespace aim::q5`、`main`、`CHECK` 宏 | 不可以 | 固定外壳 |
| `src/tests/`、`include/`、`tools/`、`CMakeLists.txt`、题面 PDF | 不可以 | 基础设施 |
| `local_tests/` | 可以新增 | 自己的实验测试与笔记 |
| `README.md` 末尾两节 | 需要填写 | 设计记录与 AI 使用说明 |

约束提醒：本题**必须抛异常**（非法地图抛 `std::invalid_argument`，不是返回空矩阵）；不能修改输入 `grid`；不能读 stdin；不要在 `aim::q5` 里再套命名空间。

---

## 2 接口与数据含义

```cpp
std::vector<std::vector<int>> risk(const std::vector<std::string>& grid);
```

| 参数 | 类型 | 题面保证 | 含义 |
| --- | --- | --- | --- |
| `grid` | `const std::vector<std::string>&` | 最多 300 行 × 300 列 | 字符地图，`grid[row][column]` 访问 |
| 返回值 | `std::vector<std::vector<int>>` | 与输入同尺寸 | 风险矩阵，相同的行列顺序 |

字符只有两种：

| 字符 | 含义 | 输出 |
| --- | --- | --- |
| `.` | 可通行 | 四邻域中障碍的个数，0..4 |
| `#` | 障碍 | 恒为 -1，**不再数邻居** |

容易踩的一点：**返回的是新矩阵，不是原地修改**。`grid` 是 `const&`，本来也不能改；不要试图返回输入或复用它的行。

---

## 3 规范逐条拆解

| 题面规范 | 翻译成代码动作 | 最容易踩的坑 |
| --- | --- | --- |
| 1 空行数组合法 | `if (grid.empty()) return {};` | 把空地图也判成非法 |
| 1 有行必须至少一列 | `if (grid.front().empty()) throw ...` | 漏判，导致后面访问 `grid[0][0]` 越界 |
| 1 所有行等长 | 逐行 `row.size() != cols` → 抛异常 | 只检查第一行 |
| 2 只含 `.` `#` | 逐字符检查 | 只查部分字符，放过最后一行最后一位 |
| 3 **先完整验证再计算** | 两个独立循环：先验证、后计算 | 边验证边填充 → 非法时已改动结果/提前返回部分矩阵 |
| 4 障碍 -1，空地数四邻域 | `#` 分支直接填 -1；`.` 分支四方向计数 | 把障碍格也拿去数邻居 |
| 5 斜对角不算、越界不算 | 四个方向偏移量 `{-1,1,0,0}` / `{0,0,-1,1}` | 写成八方向；把负数下标当越界墙 |
| 6 不改输入、O(R×C) | `const&` + 一次嵌套循环 | 每行复制、或对每个格子重扫全图 |

---

## 4 核心一：验证与计算必须分成两个循环

这是本题最重要的结构约束（规范 3）：

```cpp
// 第一遍：只验证，不产生任何输出
if (grid.empty()) return {};
const std::size_t rows = grid.size();
const std::size_t cols = grid.front().size();
if (cols == 0) throw std::invalid_argument("zero columns");
for (const std::string& row : grid) {
    if (row.size() != cols) throw std::invalid_argument("ragged row");
    for (char ch : row) {
        if (ch != '.' && ch != '#') throw std::invalid_argument("invalid cell");
    }
}

// 第二遍：此时地图一定合法，放心计算
std::vector<std::vector<int>> result;
// ...
```

**为什么不能合并成一个循环**：如果边验证边填 `result`，当第三行出现非法字符时，`result` 已经有两行数据。此时若返回 `result`，就是"部分地图"（违反规范 6）；若抛异常，`result` 的构造工作是白做的，而且代码里很容易不小心在抛之前 `return`。分成两遍后，"合法"成为一个**前提条件**，第二遍不需要任何防御性判断。

实测可得：对 `{".#", "#.", "..x"}`，验证循环会在最后一行最后一位抛异常；实现若边算边验，前面两行的结果已经写好了。这也是自测里专门放"最后一行最后一位非法"用例的原因。

---

## 5 核心二：四邻域的坐标偏移

用一个方向表代替四段重复代码：

```cpp
static const int dr[4] = {-1, 1,  0, 0};   // 行偏移：上、下、左、右
static const int dc[4] = { 0, 0, -1, 1};   // 列偏移
```

对格子 `(r, c)`，邻格坐标是 `(r + dr[d], c + dc[d])`。判定"是障碍"要同时满足三点：

1. 行下标在 `[0, rows)` 内；
2. 列下标在 `[0, cols)` 内；
3. 该格字符是 `#`。

三者缺一不可，**顺序也不能乱**：必须先判越界再访问 `grid[nr][nc]`，否则越界读是未定义行为。

### 为什么用 `long long` 存邻格坐标

`r`、`c` 是 `std::size_t`（无符号）。如果先算 `r - 1` 再判 `>= 0`，当 `r == 0` 时会**下溢成一个巨大的正数**，`r >= 0` 恒真，越界访问就发生了。正确做法是先转成有符号再加减：

```cpp
const long long nr = static_cast<long long>(r) + dr[d];
if (nr >= 0 && nr < static_cast<long long>(rows)) { ... }
```

这是本题最容易出的隐蔽 bug——编译无警告、小地图可能碰巧不崩，但行为是未定义的。

### 四邻域 vs 八邻域

| 方向 | 偏移 | 是否计入 |
| --- | --- | --- |
| 上 / 下 / 左 / 右 | `(-1,0) (1,0) (0,-1) (0,1)` | ✅ 计入 |
| 四个斜对角 | `(-1,-1) (-1,1) (1,-1) (1,1)` | ❌ 不计入 |

题面背景里"把斜对角也算进去"是明确的反例。自测用例 `risk({"#.", ".."})` 期望 `{{-1,1},{1,0}}`：右下角 `(1,1)` 虽然斜对着左上角的 `#`，但不算邻居，所以是 0。

---

## 6 完整参考实现

```cpp
std::vector<std::vector<int>> risk(const std::vector<std::string>& grid) {
    if (grid.empty()) return {};

    const std::size_t rows = grid.size();
    const std::size_t cols = grid.front().size();
    if (cols == 0) throw std::invalid_argument("zero columns");

    // 第一遍：完整验证
    for (const std::string& row : grid) {
        if (row.size() != cols) throw std::invalid_argument("ragged row");
        for (char ch : row) {
            if (ch != '.' && ch != '#') throw std::invalid_argument("invalid cell");
        }
    }

    // 第二遍：计算风险
    static const int dr[4] = {-1, 1, 0, 0};
    static const int dc[4] = { 0, 0, -1, 1};
    std::vector<std::vector<int>> result;
    result.reserve(rows);
    for (std::size_t r = 0; r < rows; ++r) {
        std::vector<int> line;
        line.reserve(cols);
        for (std::size_t c = 0; c < cols; ++c) {
            if (grid[r][c] == '#') { line.push_back(-1); continue; }
            int count = 0;
            for (int d = 0; d < 4; ++d) {
                const long long nr = static_cast<long long>(r) + dr[d];
                const long long nc = static_cast<long long>(c) + dc[d];
                if (nr >= 0 && nr < static_cast<long long>(rows) &&
                    nc >= 0 && nc < static_cast<long long>(cols) &&
                    grid[static_cast<std::size_t>(nr)][static_cast<std::size_t>(nc)] == '#') {
                    ++count;
                }
            }
            line.push_back(count);
        }
        result.push_back(std::move(line));
    }
    return result;
}
```

逐关键点说明：

- `grid.front().size()` 取第一行的列数作为基准；`grid[0]` 也可以，但 `front()` 语义更清楚。
- `result.reserve(rows)` 和 `line.reserve(cols)` 是性能优化，避免反复扩容；不写也对。
- 障碍格 `continue` 跳过邻居计数——既符合规范 4，也避免"障碍格周围有几个障碍"这种无意义计算。
- `&&` 短路保证越界时不会访问 `grid[nr][nc]`。

---

## 7 交卷前自检清单

- [ ] 实现区之外一个字没改，`// END IMPLEMENTATION` 在函数 `}` 之后
- [ ] 空地图返回 `{}`，不是抛异常
- [ ] 有行零列、行长不一致、非法字符都抛 `std::invalid_argument`
- [ ] 验证在独立的循环里，**先验证完整张地图再计算**
- [ ] 邻格坐标先转 `long long` 再加减，先判越界再访问
- [ ] 只用四个正交方向，没有斜对角
- [ ] 障碍格输出 -1，空地输出 0..4
- [ ] 没有修改 `grid`，没有读 stdin
- [ ] 实现区内没有调试输出
- [ ] 编译加 `-Wall -Wextra -Wconversion` 零警告

---

## 8 编译与运行

```bash
g++ -std=c++17 -Wall -Wextra -Wconversion -O2 src/main/A1_q5.cpp -o q5
./q5                                   # PUBLIC 2/2

g++ -std=c++17 -Wall -Wextra -Wconversion -O2 local_tests/q5_extra.cpp -o q5_extra
./q5_extra                             # Q5 EXTRA ALL PASS
```

`q5_extra.cpp` 用 `#include "../src/main/A1_q5.cpp"` 引入被测实现，必须留在 `local_tests/`。实现区仍是 `TODO` 时它会编译通过、运行抛 `TODO`，填入实现后才会全过。

---

## 9 边界实测

参考实现在本机 `g++ 16.2.0 -std=c++17 -Wall -Wextra -Wconversion` 下零警告，`PUBLIC 2/2`，`q5_extra.cpp` **全过**。摘录关键项：

| 输入 | 期望 | 考查点 |
| --- | --- | --- |
| `{}` | `{}` | 空地图合法 |
| `{""}` | 抛异常 | 有行零列非法 |
| `{"."}` | `{{0}}` | 单格空地输出 0，不是 4 |
| `{".#", ".."}` | `{{1,-1},{0,1}}` | 题面样例 |
| `{".#.", "#.#", ".#."}` | `{{2,-1,2},{-1,4,-1},{2,-1,2}}` | 中心格四邻域满 4 |
| `{"..", "."}` | 抛异常 | 行长不一致 |
| `{".","#"}` | `{{1},{-1}}` | 单列照常算上下邻居 |
| `{"#.", ".."}` | `{{-1,1},{1,0}}` | 右下角的对角障碍不计 |
| `{"#x"}` | 抛异常 | 非法字符 |
| `{".#", "#.", "..x"}` | 抛异常 | **最后一行最后一位**非法也必须发现 |
| `{"..", "..", "."}` | 抛异常 | 行长不一致在最后一行 |
| `{"1"}` / `{". "}` | 抛异常 | 数字、空格都不是合法字符 |
| `{"##", "##"}` | 全 -1 | 全障碍 |
| `{"..#", "..."}` | `{{0,1,-1},{0,0,1}}` | 边缘不自动增加风险 |
| 300×300 全空地 | 全 0 | 规模上限 |

另外做了 **2500 组随机差分**：按题面规范用 Python 独立重写期望值，随机生成 0..12 行、1..12 列的 `.`/`#` 地图，结果 **0 处不一致**。

---

## 10 缺陷定位与修复（README 素材）

| 阶段 | 现象 | 定位 | 修复 |
| --- | --- | --- | --- |
| 写自测时 | 样例3 与"边缘不当墙"两条断言失败 | 逐格手算四邻域：`{".#.", "#.#", ".#."}` 的 `(0,0)` 邻居是下 `#` 和右 `#`，应为 2；我误写成 -1（把障碍格自身的输出套到了空地格上） | 修正**期望值**为 `{{2,-1,2},{-1,4,-1},{2,-1,2}}`，实现无需改动 |

这段的价值在于说明：**测试失败不等于实现有错**。定位方法是"按规范逐格手算期望值，与程序输出比对差异"——若差异只出现在期望值那一侧，就是测试写错了。本例里程序输出与四邻域定义完全吻合，错的是我把"障碍格填 -1"误套到了空地格上。

另一个值得写进 README 的防御性设计：**先验证完整地图再计算**。对 `{".#", "#.", "..x"}` 这类"最后一行最后一位非法"的输入，边算边验的实现会在抛异常前已产出两行结果；两遍法则保证"要么完整合法、要么抛异常"，不存在中间态。

---

## 11 常见错误对照表

| 错误写法 | 后果 | 正确做法 |
| --- | --- | --- |
| 空地图抛异常 | 违反规范 1 | `if (grid.empty()) return {};` |
| 漏判 `grid[0].empty()` | 后续 `grid[0][0]` 越界 | 先检查 `cols == 0` |
| 只检查第一行长度 | 行长不一致漏判 | 逐行检查 `row.size() != cols` |
| 边验证边填结果 | 可能返回部分地图 | 两个独立循环 |
| `size_t r` 直接算 `r - 1` | 无符号下溢，越界访问 | 先转 `long long` 再加减 |
| 先访问 `grid[nr][nc]` 再判越界 | 未定义行为 | `&&` 短路，先判越界 |
| 写八方向 | 斜对角被误计 | 方向表只有四个正交方向 |
| 障碍格也数邻居 | 语义错误（虽不影响输出 -1） | 障碍格直接 `push_back(-1); continue;` |
| 用 `int` 存 rows/cols 又强转 | 无谓的类型噪音 | 统一 `std::size_t`，比较时转 `long long` |
| 修改 `grid` 或复用其行 | 违反规范 6 | 新建 `result` |

---

## 12 复杂度

- **时间**：`O(rows × cols)`。验证阶段扫描每个字符一次；计算阶段对每个空地格做 4 次邻居检查，常数次操作。
- **空间**：`O(rows × cols)` 用于返回结果（题面允许，返回值不计入额外空间）；除结果外只用两个 4 元素方向表和几个下标。
- **规模**：`rows, cols ≤ 300`，最多 9 万格、36 万次邻居检查，远低于任何时限。实测 300×300 全空地正常返回。

注意：题面规范 6 的 `O(行数×列数)` 指**有效地图**的计算时间。非法地图允许在验证阶段提前抛异常，不必扫完剩余行。
