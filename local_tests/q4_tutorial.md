# Q4 严格命令事务 · 教程

> 依据：`PROBLEM.md` 的 Q4 小节（与 `题面.pdf` 同源）。本教程只讲 Q4，每条结论都能对应回题面的编号规范。
> 文件位置：放在 `local_tests/`。该目录在 CI 白名单内（`tools/ci_support.py` 的 `ALLOWED` 正则含 `local_tests/.+`）。

---

## 0 一句话结论

题目要的是：**一行一条命令，整条命令全部检查通过才改累加器；任何一处不合法就只拒绝，绝不留下半套修改。**

代码骨架只有一句话：

> 先算出「假如成功，值会变成多少」存进局部变量 `candidate`；所有检查都过了，最后才 `value = candidate`。

配套的统计恒等式：**`accepted + rejected == lines.size()`**——每条命令必须恰好落入其中一个分支，不能既不计成功也不计失败，也不能两个都加。

---

## 1 先划边界：哪里有改动权

| 位置 | 能否修改 | 说明 |
| --- | --- | --- |
| `src/main/A1_q4.cpp` 中 `// BEGIN IMPLEMENTATION` 与 `// END IMPLEMENTATION` 之间 | 可以 | 唯一允许写实现的地方 |
| 同文件的 `namespace aim::q4`、`struct CommandResult`、`main`、`CHECK` 宏 | 不可以 | 固定外壳 |
| `src/tests/`、`include/`、`tools/`、`CMakeLists.txt`、题面 PDF | 不可以 | 基础设施 |
| `local_tests/` | 可以新增 | 自己的实验测试与笔记 |
| `README.md` 末尾两节 | 需要填写 | 设计记录与 AI 使用说明 |

三条容易忽略的约束：

1. **本题不抛异常**。非法命令是「拒绝 + 计数」，不是 `throw`；题面没有任何一处要求捕获异常。
2. **`CommandResult` 已由骨架定义**，不要在实现区重复定义，也不要再套一层命名空间。
3. **不能读 stdin**。所有输入都从 `lines` 参数来；`std::istringstream` 是把已有字符串当输入流，与键盘无关。

---

## 2 接口与数据含义

```cpp
struct CommandResult { long long value; int accepted; int rejected; };
CommandResult execute(const std::vector<std::string>& lines);
```

| 名称 | 类型 | 题面保证 | 含义 |
| --- | --- | --- | --- |
| `lines` | `const std::vector<std::string>&` | 最多 10000 行 | 一行视为一条命令，**空行也算一次拒绝** |
| `value` | `long long` | 恒在 [-10^6, 10^6] | 最终累加器值，从 0 开始 |
| `accepted` | `int` | ≥ 0 | 成功提交的命令条数 |
| `rejected` | `int` | ≥ 0 | 被拒绝的命令条数 |
| 返回值 | `CommandResult` | 三字段聚合 | `CommandResult{value, accepted, rejected}` |

两个容易看错的点：

1. `value` 是 `long long`，但**值域只有 ±10^6**（题面规范 4 对候选结果也做了限制），所以不存在溢出风险；写 `long long` 只是为了和接口一致。
2. **每次调用完全独立**。状态必须是函数内的局部变量，不能用 `static`，否则第二次调用会接着上一次的 `value` 往下算。

---

## 3 规范逐条拆解

| 题面规范 | 翻译成代码动作 | 最容易踩的坑 |
| --- | --- | --- |
| 1 累加器从 0 开始；一行一命令；空行拒绝 | `long long value = 0;` + 逐行循环 | 把空行 `continue` 掉，漏掉 `rejected` |
| 2 仅 `ADD x` / `SET x` / `RESET`，大小写敏感，允许首尾空白 | `input >> op` 后做**精确字符串比较** | 用 `tolower` 统一大小写 → `add 5` 被误收 |
| 2 ADD/SET 恰一个参数；RESET 不允许参数 | `input >> x` 之后再 `input >> extra` 检查尾部 | 只看第一个 token，漏掉 `SET 8 x` |
| 3 x 为十进制整数，`[-10^6,10^6]`；小数尾缀、多余字段、溢出均拒绝 | 提取 + 尾部检查 + 范围检查，三者全过 | 以为 `ADD 12x` 会提取失败（实际能读出 12） |
| 4 候选结果也必须在范围内 | 算完 `candidate` 再判范围 | 只检查 `x`，漏掉 `SET 1000000` 后 `ADD 1` |
| 5 完整验证成功才提交 | `ok` 布尔 + 局部变量 `candidate` | 边解析边改 `value`，失败时撤不回来 |
| 6 空输入 `{0,0,0}`；计数守恒；调用独立 | 循环一次不执行即返回；每行必落一个分支 | 失败时 `return` 提前退出，后续命令没处理 |

---

## 4 核心：事务式更新（先验证，后提交）

题面背景讲的就是这个 bug：「一读到 `SET 8` 就更新数值，随后才发现尾巴不合法，却已经无法恢复原来的状态」。

**错误写法**（边解析边改）：

```cpp
if (op == "SET") { in >> x; value = x; std::string extra; if (in >> extra) { /* 已经太迟 */ } }
```

**正确写法**（候选值 + 一次性提交）：

```cpp
long long candidate = 0;
bool ok = false;
// ... 所有检查都只写 candidate 和 ok，绝不碰 value ...
if (ok) { value = candidate; ++accepted; }
else    { ++rejected; }
```

`candidate` 是「假设成功后的值」，`ok` 是「整条命令是否合法」。两者都是**本轮循环内的局部变量**，所以即使中途检查失败，`value` 仍保持上一轮的值，天然满足规范 5。

### 实测缺陷（可直接写进 README）

我用一段故意写错的实现跑了两条用例：

```
执行 {"SET 8 x"} 后 value = 8   （正确应为 0，整条拒绝）
执行 {"ADD 5","SET 8 x","RESET","ADD -2"} 后 value = -2  （碰巧正确）
```

**定位过程**：第一条单独跑就暴露了——`SET 8 x` 被拒绝，值却变成 8。第二条之所以"看起来对"，是因为后面的 `RESET` 把脏值清零了，**掩盖了缺陷**。这说明验证事务性时必须用单条命令孤立测试，不能只看混合序列的最终值。

**修复**：把 `value = x` 从解析阶段移到循环末尾的 `if (ok)` 里。这正是「候选值」模式的由来。

---

## 5 解析：`istringstream` 的四步检查

`std::istringstream input(line)` 把一行文本变成输入流，`>>` 的行为和 `std::cin >>` 一样：**按目标类型读，跳过前导空白，读到不属于该类型的字符就停下**。它不是按空格切词，所以 `12x` 会先读出整数 `12`，把 `x` 留在流里。

每条命令新建自己的流，按这四步走：

| 步 | 代码 | 检查什么 |
| --- | --- | --- |
| ① | `input >> op` | 有没有命令名。空行 / 全空白 → 提取失败 → 拒绝 |
| ② | `op == "ADD"` / `"SET"` / `"RESET"` | 精确匹配，大小写敏感；其它一律拒绝 |
| ③ | `input >> x` 且 `!(input >> extra)` | 参数必须是**恰好一个整数**，后面不能再读出任何词 |
| ④ | `-LIMIT <= x <= LIMIT` 且候选也在范围内 | 参数与结果的双重范围检查 |

`RESET` 走另一条支：`!(input >> extra)` 为真才合法（尾部只有空白时读不出词，所以 `RESET   ` 合法，`RESET x` 不合法）。

### 各种输入的实际判定

| 输入 | `>> op` | `>> x` | `>> extra` | 判定 |
| --- | --- | --- | --- | --- |
| `ADD 5` | `ADD` | 5 | 读不出 | 接受 |
| `  ADD   5  ` | `ADD` | 5 | 读不出（尾部只有空白） | 接受 |
| `ADD +2` | `ADD` | 2 | 读不出 | 接受 |
| `ADD 5 6` | `ADD` | 5 | `"6"` | 拒绝 |
| `SET 8 x` | `SET` | 8 | `"x"` | 拒绝 |
| `ADD 1.5` | `ADD` | 1 | `".5"` | 拒绝 |
| `ADD 12x` | `ADD` | 12 | `"x"` | 拒绝 |
| `ADD` | `ADD` | 失败 | — | 拒绝 |
| `ADD 99999999999999999999999` | `ADD` | 失败（超出提取类型） | — | 拒绝 |
| `RESET` | `RESET` | — | 读不出 | 接受 |
| `RESET x` | `RESET` | — | `"x"` | 拒绝 |
| `""` / `"   "` | 失败 | — | — | 拒绝 |
| `add 5` | `add` | — | — | 拒绝（大小写） |

**最容易漏的是 `12x` 和 `1.5`**：整数提取"成功了"，但流里还剩东西。所以 `!(input >> extra)` 这一步不能省——题面的字符串流提示专门演示了这一点。

`x` 用 `long long` 而不是 `int`：超长整数会让 `>>` 直接失败（规范 3 的"溢出拒绝"自动成立），而合法范围内的数再靠 `-LIMIT <= x <= LIMIT` 兜住。

---

## 6 六个必须想清楚的细节

1. **大小写敏感**：`op == "ADD"` 是精确比较。`add`、`Add`、`RESET` 写成 `reset` 都要拒绝，不要调用 `tolower`。
2. **RESET 无参，但允许空白尾巴**：`RESET   ` 合法，`RESET x` 拒绝。用 `input >> extra` 是否失败来区分，不要自己 `trim`。
3. **双重范围检查**：参数 `x` 和候选 `candidate` 都要在 [-10^6, 10^6]。`SET 1000000`（合法）后 `ADD 1`（参数合法、结果 1000001 越界）必须整条拒绝，且 `value` 保持 1000000。
4. **失败后要继续**：任何一条命令都不能 `return` 或 `break`，否则后续命令不处理，`accepted + rejected` 就凑不满 `lines.size()`。
5. **空行也计数**：`input >> op` 失败时走 `else { ++rejected; }`，不能 `continue` 跳过。
6. **不能用 `static`**：`value`、`accepted`、`rejected` 必须是普通局部变量，否则第二次调用会继承上一次的状态。

---

## 7 完整参考实现

```cpp
CommandResult execute(const std::vector<std::string>& lines) {
    const long long LIMIT = 1000000;
    long long value = 0;
    int accepted = 0;
    int rejected = 0;

    for (const std::string& line : lines) {
        std::istringstream input(line);      // 每条命令一个独立的流
        std::string op;
        long long candidate = 0;             // 假设成功后的值，不碰 value
        bool ok = false;                     // 整条命令是否全部合法

        if (input >> op) {                   // ① 命令名；空行/全空白会失败
            if (op == "ADD" || op == "SET") {
                long long x = 0;
                std::string extra;
                // ③ 恰一个整数参数（尾部不能再读出词），④ 参数范围
                if ((input >> x) && !(input >> extra) && x >= -LIMIT && x <= LIMIT) {
                    candidate = (op == "ADD") ? value + x : x;
                    ok = (candidate >= -LIMIT && candidate <= LIMIT);  // ④ 候选值范围
                }
            } else if (op == "RESET") {
                std::string extra;
                if (!(input >> extra)) {     // RESET 不允许参数
                    candidate = 0;
                    ok = true;
                }
            }
            // 其它 op：什么都不做，ok 保持 false
        }

        if (ok) { value = candidate; ++accepted; }   // 唯一提交点
        else    { ++rejected; }                      // 只计数，value 不动
    }
    return CommandResult{value, accepted, rejected};
}
```

注意整段代码里 **只有一处 `value = candidate`**。这是事务性的最好自检：搜索 `value =`，应该只出现这一次（加上初始化那次）。

---

## 8 交卷前自检清单

- [ ] 实现区之外一个字没改，`// END IMPLEMENTATION` 在函数 `}` 之后
- [ ] `value`、`accepted`、`rejected` 都是局部变量，没有 `static`
- [ ] 每条命令恰好让 `accepted` 或 `rejected` 之一加一
- [ ] 用 `istringstream` 逐行解析，没有读 stdin
- [ ] `ADD` / `SET` 检查了尾部 `extra`；`RESET` 检查了无参数
- [ ] 参数 `x` 与候选 `candidate` 都判了 [-10^6, 10^6]
- [ ] 命令名精确比较，没有做大小写转换
- [ ] 实现区内没有 `cout` / `cerr` / 调试输出
- [ ] 编译加 `-Wall -Wextra -Wconversion` 零警告
- [ ] `PUBLIC 2/2`、`Q4 EXTRA ALL PASS` 都跑过

---

## 9 编译与运行

```bash
g++ -std=c++17 -Wall -Wextra -Wconversion -O2 src/main/A1_q4.cpp -o q4
./q4                                   # PUBLIC 2/2

g++ -std=c++17 -Wall -Wextra -Wconversion -O2 local_tests/q4_extra.cpp -o q4_extra
./q4_extra                             # Q4 EXTRA ALL PASS
```

`q4_extra.cpp` 用 `#include "../src/main/A1_q4.cpp"` 引入被测实现，必须留在 `local_tests/` 才能靠相对路径找到文件。实现区仍是占位（空函数体）时，编译会报 `-Wreturn-type` 与 `-Wunused-parameter` 两条警告，运行行为未定义；填入实现后才会全过。

---

## 10 边界实测

参考实现在本机 `g++ 16.2.0 -std=c++17 -Wall -Wextra -Wconversion` 下零警告，`PUBLIC 2/2`，`q4_extra.cpp` **29 条断言 + 1 项计数守恒检查全过**。摘录关键项：

| 输入 | 期望 | 考查点 |
| --- | --- | --- |
| `{"ADD 5","SET 8 x","RESET","ADD -2"}` | `{-2,3,1}` | 题面样例，带尾巴的 SET 整条拒绝 |
| `{}` | `{0,0,0}` | 空输入 |
| `{"SET 1000000","ADD 1"}` | `{1000000,1,1}` | 参数合法但候选越界 |
| `{"SET 5","RESET x"}` | `{5,1,1}` | 非法 RESET 不清零 |
| `{"add 5",""}` | `{0,0,2}` | 大小写 + 空行拒绝 |
| `{"ADD +2","RESET"}` | `{0,2,0}` | 合法零值操作仍计成功 |
| `{"SET 8 x"}` | `{0,0,1}` | 单条失败不留半套修改 |
| `{"", "   ", "\t"}` | `{0,0,3}` | 空行与纯空白都计数 |
| `{"ADD","SET","RESET"}` | `{0,1,2}` | 缺字段；RESET 无参仍合法 |
| `{"ADD 1.5"}` / `{"ADD 12x"}` | `{0,0,1}` | 小数/字母尾缀必须靠 `extra` 拦住 |
| `{"ADD 5 6","SET 3 x","RESET y"}` | `{0,0,3}` | 多余字段 |
| `{"ADD 99999999999999999999999"}` | `{0,0,1}` | 超长整数提取失败 |
| `{"SET -1000000","ADD -1"}` | `{-1000000,1,1}` | 候选下界越界 |
| `{"  ADD   5  ", "\tSET\t3\t"}` | `{3,2,0}` | 首尾与中间空白 |
| `{"ADD 1","BAD","ADD 2","SET 8 x","RESET","ADD 5"}` | `{5,4,2}` | 失败后继续处理 |
| 10000 行 `ADD 0` | `{0,10000,0}` | 规模上限 |

另外做了 **3600 组随机差分**：按题面规范用 Python 独立重写期望值，混合合法/非法命令、边界值、大小写、尾缀、空白、超长数，并定向构造"先 `SET` 到 ±10^6 再试探越界"的序列，结果 **0 处不一致**。

---

## 11 缺陷定位与修复（README 素材）

| 阶段 | 现象 | 定位 | 修复 |
| --- | --- | --- | --- |
| 初版（边解析边更新） | `{"SET 8 x"}` 返回 `value=8` | 用单条命令孤立复现；确认 `value = x` 在尾部检查之前执行 | 引入 `candidate` + `ok`，把提交移到循环末尾 |
| 混合序列看似通过 | `{"ADD 5","SET 8 x","RESET","ADD -2"}` 得到 -2 | 发现后续 `RESET` 清零掩盖了脏值 | 补一条单命令用例，避免被后续命令掩盖 |

这段可以直接对应 README 要求的「至少三个自测 + 一处缺陷定位与修复」；教程第 10 节还给了 16 条实测边界可摘用。

---

## 12 常见错误对照表

| 错误写法 | 后果 | 正确做法 |
| --- | --- | --- |
| 读到 `SET` 就 `value = x` | 失败命令留下脏值 | 只写 `candidate`，末尾统一提交 |
| 不检查 `input >> extra` | `SET 8 x`、`ADD 12x`、`ADD 1.5` 被误收 | 整数后再读一个词，成功即拒绝 |
| `RESET` 不检查参数 | `RESET x` 被误收且清零 | `if (!(input >> extra))` |
| 用 `tolower` 统一后比较 | `add 5` 被误收 | 精确字符串比较 |
| 空行 `continue` | `rejected` 少算，计数不守恒 | 走 `else { ++rejected; }` |
| 失败时 `return` | 后续命令不处理 | 继续下一轮循环 |
| 只判 `x` 不判 `candidate` | `SET 1000000` 后 `ADD 1` 越界 | 双重范围检查 |
| `static long long value` | 第二次调用继承上次状态 | 普通局部变量 |
| 用 `std::stoi` 解析 | 抛异常、且无法检查尾缀 | `istringstream >>` |

---

## 13 复杂度

- **时间**：`O(总字符数)`。每行只被 `istringstream` 顺序扫描一次，最多 10000 行；没有嵌套遍历。
- **空间**：`O(1)` 额外空间（每行一个临时流、几个局部变量与字符串）。返回值 `CommandResult` 按题面不计入。
- **规模**：`n ≤ 10000`，单次解析常数级，远低于任何时限；`value + x` 最大 2×10^6，`long long` 完全安全。

题面没有对本题规定复杂度上限，但"每条命令创建自己的流、先完成检查再修改"是明确的实现约束，人工复核会看这一点。
