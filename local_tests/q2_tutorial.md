# Q2 设备标签维护 · 教程

> 依据：`PROBLEM.md` 的 Q2 小节（与 `题面.pdf` 同源）。本教程只讲 Q2，每条结论都能对应回题面的编号规范。
> 文件位置：放在 `local_tests/`。该目录在 CI 白名单内（`tools/ci_support.py` 的 `ALLOWED` 正则含 `local_tests/.+`），文档和测试都可以留在这里。

---

## 0 一句话结论

`cam,imu,radar` 这种文本，本质是**一个字符串列表的序列化**。最省事的做法不是对着整串做字符串手术，而是：

**先还原成列表 → 按顺序逐条改 → 再用单个逗号拼回去。**

三步做完，规范里那堆"不能有首尾逗号、不能有连续逗号"的要求自动成立，因为你从来没有手工拼过逗号。

---

## 1 先划边界：哪里有改动权

| 位置 | 能否修改 | 说明 |
| --- | --- | --- |
| `src/main/A1_q2.cpp` 中 `// BEGIN IMPLEMENTATION` 与 `// END IMPLEMENTATION` 之间 | 可以 | 唯一允许写实现的地方 |
| 同文件的 `namespace aim::q2`、`struct LabelEdit`、`main`、`CHECK` 宏 | 不可以 | 固定外壳；`LabelEdit` 已由骨架提供，不要重复定义 |
| `src/tests/`、`include/`、`tools/`、`CMakeLists.txt`、题面 PDF | 不可以 | 基础设施与固定类型 |
| `local_tests/` | 可以新增 | 自己的实验测试与笔记 |
| `README.md` 末尾两节 | 需要填写 | 设计记录与 AI 使用说明 |

实现区内部的额外约束：

- 可以增加普通辅助函数（例如边界判定），但**不能新增 `class`**、构造函数、继承、多态。
- 不能在 `aim::q2` 里再套一层命名空间。
- 不能读 stdin、不能输出调试信息、不能改函数签名。
- 本题不涉及异常：输入保证语法合法（规范 2），**不需要**报告格式错误。

---

## 2 接口与数据含义

```cpp
struct LabelEdit { std::string from; std::string to; };
std::string edit_labels(std::string labels, const std::vector<LabelEdit>& edits);
```

| 参数 | 类型 | 题面保证 | 含义 |
| --- | --- | --- | --- |
| `labels` | `std::string`（**按值传入**） | 空串，或逗号连接的非空标签；总长 ≤ 1000；无首尾逗号、无连续逗号；每个标签 1..32 字符，只含字母/数字/下划线，区分大小写 | 当前设备名单 |
| `edits` | `const std::vector<LabelEdit>&` | ≤ 200 项；`from`、`to` 为空串或符合上述规则的标签 | 编辑指令序列，**按顺序**执行 |
| 返回值 | `std::string` | 与 `labels` 同格式 | 编辑后的名单 |

两个设计细节值得注意：

1. **`labels` 是按值传递**。这正好免费满足规范 7 的"不修改调用方的文本"——函数拿到的是副本，随便改。
2. **`edits` 是 `const` 引用**。既不能也不该改，规范 7 的"不修改 `edits`"自然成立。

---

## 3 规范逐条拆解

| 题面规范 | 翻译成代码动作 | 最容易踩的坑 |
| --- | --- | --- |
| 1 名单格式 | 按 `,` 切分即得标签列表 | 切完还去过滤空串——本题输入保证没有空标签，过滤反而掩盖真实问题 |
| 2 编辑项合法 | 不做校验，直接使用 | 写一堆正则/长度校验，属于无效工作 |
| 3 `from` 空、`to` 非空 → 追加 | 只在**当前列表非空**时补一个逗号 | `labels += "," + to` → 空名单变成 `,lidar` |
| 4 `from` 非空 → 只找第一个**完整**标签 | 比较"切分后的元素"，命中即停 | 用 `find` 直接搜整串，把 `cam2`、`camera` 也改掉 |
| 5 命中且 `to` 空 → 删除，并处理相邻逗号 | 删掉该元素，拼接时统一生成逗号 | 手工删字符时漏掉首/尾逗号的特殊情况 |
| 6 允许重复、顺序不变、每次重新定位 | 只改第一个匹配元素，其余不动 | 缓存下标，改名变长后位置失效 |
| 7 空 `edits` 原样返回、不改入参 | 无编辑时列表原样拼回 | 用 `static` 变量"记住"上次结果 |

---

## 4 四个必须想清楚的难点

### 4.1 "完整标签"不等于"子串"

题面里第一个坑就是这个：搜索 `cam` 时，`cam2`、`camera`、`camcorder` 里都含 `cam`，但它们**都不是** `cam` 这个标签。

| 当前名单 | 编辑 `cam → imu` | 正确结果 |
| --- | --- | --- |
| `cam,cam2` | 命中第 1 个 | `imu,cam2` |
| `cam2,cam` | 只命中第 3 个 | `cam2,imu` |
| `camera,camcorder` | 一个都不命中 | `camera,camcorder`（原样） |

按列表比较时这个问题自动消失：`tags[i] == "cam"` 是整串相等比较，`"cam2" != "cam"`。

### 4.2 只改"第一次命中"，不是"全部命中"

规范 4 写的是"只查找第一个与 `from` 完全相同的完整标签"，规范 6 又允许重复标签。两者合起来意味着：

```cpp
edit_labels("a,b,a", {{"a","x"}});   // "x,b,a"，不是 "x,b,x"
```

常见错误是写成循环替换（`while (find != npos) replace(...)`），那会把所有 `a` 都改掉。正确做法是**找到一个就 `break`**。

### 4.3 删除时的"逗号账"

如果走原地字符串操作（方案 B），删除标签时必须同时删掉**一个**逗号，否则会留下畸形文本。四种位置，四种范围：

| 命中位置 | 需要删除的字符区间 | 例子 → 结果 |
| --- | --- | --- |
| 唯一标签 | `[pos, pos+len)` | `cam` → `""` |
| 首个（后面还有） | `[pos, pos+len+1)`，吃掉**后面**的逗号 | `cam,b` → `b` |
| 末尾（前面有） | `[pos-1, pos+len)`，吃掉**前面**的逗号 | `a,cam` → `a` |
| 中间 | `[pos-1, pos+len)`，吃掉**前面**的逗号 | `a,cam,b` → `a,b` |

我在本机用"只删标签、不删逗号"的写法实测过，结果是：

```
["a,b,c" 删 b] → [a,,c]     连续逗号
["a,b"   删 b] → [a,]       尾部逗号
["a,b"   删 a] → [,b]       首部逗号
```

三种畸形全部违反规范 5。**方案 A 从根上没有这个问题**，因为它从不手工删逗号。

### 4.4 每次编辑都要重新定位

规范 6 后半句："每次编辑使用上一次编辑后的文本，不能沿用修改前的字符位置。"

```cpp
// "a,b" 先 a→aaaa，再删 b
// 若第一步缓存了 b 的下标 2，第二步去删下标 2 的字符，
// 实际删掉的是第 5 个字符附近的内容 —— 错误
```

改名会改变标签长度，删除会移动后续所有下标。所以每一轮都必须从头搜索，这也是"列表化"方案顺手解决的问题：你操作的是元素，不是偏移量。

---

## 5 两种实现路线

### 5.1 方案 A（推荐）：列表化

三段式，逻辑和规范一一对应：

```cpp
// 第一段：切分（空串 → 空列表，不是含一个空串的列表）
std::vector<std::string> tags;
if (!labels.empty()) {
    std::string current;
    for (char ch : labels) {
        if (ch == ',') { tags.push_back(current); current.clear(); }
        else           { current += ch; }
    }
    tags.push_back(current);
}

// 第二段：按顺序执行编辑
for (const LabelEdit& edit : edits) {
    if (edit.from.empty()) {                 // 规范 3：追加
        if (!edit.to.empty()) tags.push_back(edit.to);
        continue;
    }
    for (std::size_t i = 0; i < tags.size(); ++i) {
        if (tags[i] != edit.from) continue;  // 规范 4：整标签相等
        if (edit.to.empty()) tags.erase(tags.begin() + static_cast<std::ptrdiff_t>(i));
        else                 tags[i] = edit.to;
        break;                                // 规范 4：只处理首次命中
    }
}

// 第三段：拼接
std::string result;
for (std::size_t i = 0; i < tags.size(); ++i) {
    if (i != 0) result += ',';
    result += tags[i];
}
return result;
```

**为什么它天然满足格式要求**：逗号只在"非首个元素之前"生成，所以不可能出现首逗号、尾逗号、连续逗号；列表为空时循环一次都不执行，返回空串，正好对应"删除唯一标签得到空字符串"。

**为什么 `labels.empty()` 要单独判断**：如果不判断，切分会得到一个"含一个空串"的列表，最后拼回来是空串——结果碰巧也对，但中间状态多了一个非法标签，后续若有追加操作就会拼出 `,x`。显式判断更稳。

### 5.2 方案 B（理解边界）：原地字符串操作

题面允许用 `find`/`substr`/`replace`/`erase`，所以这条路是合法的，只是更容易出错。核心是先写一个"这个位置是不是完整标签"的判定：

```cpp
static bool complete_tag_at(const std::string& text, std::size_t pos, std::size_t len) {
    const bool left_ok  = (pos == 0) || (text[pos - 1] == ',');
    const std::size_t end = pos + len;
    const bool right_ok = (end == text.size()) || (text[end] == ',');
    return left_ok && right_ok;
}
```

然后循环找第一个"完整"命中：

```cpp
std::size_t pos = labels.find(edit.from);
while (pos != std::string::npos && !complete_tag_at(labels, pos, edit.from.size()))
    pos = labels.find(edit.from, pos + 1);   // 注意 pos+1，否则死循环
```

命中后：`to` 非空就 `replace(pos, from.size(), to)`；`to` 为空就按 4.3 的表格分四种情况 `erase`。

它比方案 A 多三处风险：逗号分支容易漏、删除后下标全部前移（必须重新 `find`）、`from` 与 `to` 相同时若用循环会陷入死循环。**如果要选一条写，选 A。**

---

## 6 完整参考实现

把下面内容整体替换掉 `src/main/A1_q2.cpp` 里 `// BEGIN IMPLEMENTATION` 到 `// END IMPLEMENTATION` 之间的那一行 TODO，标记行原样保留。

```cpp
// BEGIN IMPLEMENTATION
// aim-cpp-cw
std::string edit_labels(std::string labels, const std::vector<LabelEdit>& edits) {
    std::vector<std::string> tags;
    if (!labels.empty()) {
        std::string current;
        for (char ch : labels) {
            if (ch == ',') {
                tags.push_back(current);
                current.clear();
            } else {
                current += ch;
            }
        }
        tags.push_back(current);
    }
    for (const LabelEdit& edit : edits) {
        if (edit.from.empty()) {
            if (!edit.to.empty()) tags.push_back(edit.to);
            continue;
        }
        for (std::size_t i = 0; i < tags.size(); ++i) {
            if (tags[i] != edit.from) continue;
            if (edit.to.empty()) tags.erase(tags.begin() + static_cast<std::ptrdiff_t>(i));
            else tags[i] = edit.to;
            break;
        }
    }
    std::string result;
    for (std::size_t i = 0; i < tags.size(); ++i) {
        if (i != 0) result += ',';
        result += tags[i];
    }
    return result;
}
// END IMPLEMENTATION
```

本机验证（`g++ 16.2.0 -std=c++17 -Wall -Wextra -Wconversion`）：零警告，`PUBLIC 2/2`，20 条边界用例通过，4001 组随机差分无差异。

---

## 7 交卷前自检清单

| 规范 | 代码里的体现 | 怎么验证 |
| --- | --- | --- |
| 1 名单格式 | 切分逻辑不认识空标签 | `edit_labels("", {})` 返回 `""` |
| 2 编辑项合法 | 无校验分支 | 代码里没有多余的合法性判断 |
| 3 追加不加前导逗号 | `tags.push_back` + 拼接时补逗号 | `edit_labels("", {{"","x"}})` → `x` |
| 4 只匹配完整标签、只处理首次 | `tags[i] != edit.from` + `break` | `cam2,cam` 与 `a,b,a` 两例 |
| 5 删除不留畸形逗号 | 删除元素，逗号由拼接生成 | 删首/中/尾/唯一四例 |
| 6 顺序与重复 | 只改一个元素，其余不动 | `a,a` 改第一个后仍是 `x,a` |
| 7 空编辑原样返回 | 无编辑时列表原样拼回 | `edit_labels("cam", {})` → `cam` |

---

## 8 编译与运行

```bash
g++ -std=c++17 -Wall -Wextra src/main/A1_q2.cpp -o a1_q2
./a1_q2
```

期望输出 `PUBLIC 2/2`。未实现时输出 `TODO` 并返回 1，属预期状态。`PUBLIC 2/2` 只说明两组公开调用通过，隐藏用例还会覆盖删除位置、大小写、重复标签等。

---

## 9 边界实测

本机 `g++ 16.2.0` 实测，配套程序 `local_tests/q2_extra.cpp`（20 条断言，运行输出 `Q2 EXTRA ALL PASS`）。

| # | 调用 | 预期 | 实测 |
| --- | --- | --- | --- |
| 1 | `("cam,imu,radar", {imu→gyro, cam→空, 空→lidar})` | `gyro,radar,lidar` | 一致 |
| 2 | `("cam,cam2,cam", {cam→camera, cam→空})` | `camera,cam2` | 一致 |
| 3 | `("cam", {cam→空, 空→imu})` | `imu` | 一致 |
| 4 | `("cam2,cam", {cam→imu})` | `cam2,imu` | 一致 |
| 5 | `("a,b,a", {a→x})` | `x,b,a` | 一致 |
| 6 | `("a,b", {b→空})` | `a` | 一致 |
| 7 | `("a", {a→空, 空→b})` | `b` | 一致 |
| 8 | `("", {})` | `""` | 一致 |
| 9 | `("", {空→x})` | `x` | 一致 |
| 10 | `("a,b,c", {a→空})` | `b,c` | 一致 |
| 11 | `("a,b,c", {b→空})` | `a,c` | 一致 |
| 12 | `("only", {only→空})` | `""` | 一致 |
| 13 | `("a,b", {zz→y})` | `a,b` | 一致 |
| 14 | `("a,a,b", {a→a})` | `a,a,b` | 一致 |
| 15 | `("a,b", {空→空})` | `a,b` | 一致 |
| 16 | `("Cam,cam", {cam→x})` | `Cam,x` | 一致 |
| 17 | `("camera,camcorder", {cam→x})` | 原样 | 一致 |
| 18 | `("a,b", {a→aaaa, b→空})` | `aaaa` | 一致 |
| 19 | `("a", {空→a})` | `a,a` | 一致 |
| 20 | 32 个 `z` 改名为 `y` | `y` | 一致 |

另外做了随机差分：以题面规范为定义，在 Python 侧独立实现一份期望值，生成 4000 组随机名单与编辑序列（含 `from`/`to` 为空、不存在、互为子串、大小写不同等情形），外加 1 组"满 200 次编辑 + 长名单"，共 4001 组，逐字节比对，**0 处不一致**。

### 一处实测出来的缺陷（可写进 README 的缺陷分析）

先写了一版"原地字符串操作"的删除逻辑，只 `erase(pos, from.size())`，忘记处理逗号。运行后得到：

```
["a,b,c" 删 b] → [a,,c]
["a,b"   删 b] → [a,]
["a,b"   删 a] → [,b]
```

**定位方法**：结果长度只差 1 而内容多了一个逗号，说明删掉了标签但没删分隔符；再对照规范 5 的"不应有首尾逗号或连续逗号"确认这是必错项。

**修复**：按 4.3 的表格分四种情况，首/中/尾都要多吃掉一个相邻逗号。改完后我判断这种手工账太容易出错，最终改用列表化方案——**删掉"逗号"这个概念，问题就不存在了**。这也是最后提交版本的选择理由。

---

## 10 常见错误对照表

| 错误写法 | 现象 | 正确做法 |
| --- | --- | --- |
| `labels.find(edit.from)` 直接 replace | `cam` 改动 `cam2`/`camera` | 按元素整串比较 |
| `while (find != npos) replace(...)` | 所有重复标签都被改 | 命中一次即 `break` |
| 追加时 `labels += "," + to` | 空名单得到 `,x` | 交给拼接阶段补逗号 |
| 删除只 `erase(pos, len)` | `a,,c`、`a,`、`,b` | 列表化，或按四种位置处理逗号 |
| 缓存上一步的下标 | 改名变长后删错标签 | 每轮重新查找 |
| `labels.split` 后过滤空串 | 掩盖非法输入，逻辑与规范脱节 | 输入已保证合法，直接切 |
| 用 `static` 保存上次结果 | 第二次调用结果错乱 | 全部局部变量 |
| `from == to` 时 `continue` | 本轮不倒错，但若与"只改一次"混写成循环会出错 | 正常走替换流程即可 |

---

## 11 复杂度

- **时间**：设标签数 `k`（≤ 名单长度），编辑数 `m`（≤ 200）。每次编辑扫描列表 `O(k)`，删除时搬移元素 `O(k)`，拼接 `O(k)`；总计 `O(m·k)`，最坏约 200 × 数百个字符，完全在限制内。
- **空间**：`O(k)` 存放切分后的标签，属于"实现可用"的辅助空间；题面对本题没有额外空间限制（只有 Q3、B2 明确要求 `O(1)`）。
- 名单初始总长 ≤ 1000，但改名可能变长、追加也会增长；`std::vector`/`std::string` 自动扩容，不需要预先算容量。
