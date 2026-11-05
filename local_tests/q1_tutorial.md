# Q1 定点遥测规范化 · 教程

> 依据：`PROBLEM.md` 的 Q1 小节（与 `题面.pdf` 同源）。本教程只讲 Q1，每条结论都能对应回题面的编号规范。
> 文件位置：本教程放在 `local_tests/q1_tutorial.md`。该目录在 CI 白名单内（`tools/ci_support.py` 的 `ALLOWED` 正则含 `local_tests/.+`），所以教程、笔记、测试代码都可以留在仓库里，不会让改动范围检查失败。
> 反例提醒：放到 `docs/`、仓库根目录或 `.workbuddy/` 都会让 CI 红——白名单只认 `src/main/A1_*.cpp`、`README.md`、`local_tests/`、`.agent-sessions/bundles/` 四个位置。

---

## 0 一句话结论

Q1 只需两个函数、不到 25 行代码：`ratio` 把血量换算成 0..100 的整数百分比，`report` 把四个字段拼成一条固定格式的字符串。**难点不在算法，而在三件事：先判合法、再夹取、只用整数**。任何一处顺序错了，隐藏用例都会挂。

---

## 1 先划边界：哪里有改动权

| 位置 | 能否修改 | 说明 |
| --- | --- | --- |
| `src/main/A1_q1.cpp` 中 `// BEGIN IMPLEMENTATION` 与 `// END IMPLEMENTATION` 之间 | 可以 | 唯一允许写实现的地方 |
| 同文件的 `#include`、`namespace aim::q1`、`main`、`CHECK` 宏 | 不可以 | 属于固定外壳 |
| `src/tests/`、`include/`、`tools/`、`CMakeLists.txt`、题面 PDF | 不可以 | 基础设施与固定类型 |
| `local_tests/` | 可以新增 | 实验测试、笔记、教程都放这里（该目录下任意文件都在 CI 白名单内） |
| `README.md` 末尾两节 | 需要填写 | 设计记录与 AI 使用说明 |

实现区内部还有额外的约束（题面第 9 段 + 规范 6）：

- 可以在区内增加普通辅助函数，但不能新增 `class`、构造函数、继承、多态、资源所有权设计。
- 不能在 `aim::q1` 里再套一层命名空间；接口已经在 `namespace aim::q1` 内。
- 不能自己读取 stdin、不能输出调试信息、不能修改函数签名、不能改标记行。
- 本题**不需要也不允许**抛异常：`max_hp<=0` 是正常返回值 `-1`，不是错误。

---

## 2 接口与数据含义

```cpp
long long ratio(long long hp, long long max_hp);
std::string report(int id, long long hp, long long max_hp, int battery);
```

| 参数 | 类型 | 题面保证的范围 | 含义 | 要不要夹取 |
| --- | --- | --- | --- | --- |
| `hp` | `long long` | 绝对值 ≤ 10^12 | 当前血量原始值 | 要，夹到 `[0, max_hp]` |
| `max_hp` | `long long` | 绝对值 ≤ 10^12 | 血量上限，**可能 ≤ 0** | 不夹取，≤0 走无效分支 |
| `id` | `int` | `[0,9999]` | 设备编号 | 不夹取，原样输出 |
| `battery` | `int` | 任意 `int` | 电量原始值 | 要，夹到 `[0,100]` |

返回值：`ratio` 返回 `-1`（无效）或 `0..100`（百分比整数）；`report` 返回格式化字符串。

---

## 3 规范逐条拆解

| 题面规范 | 翻译成代码动作 | 最容易踩的坑 |
| --- | --- | --- |
| 1. 数值范围 | 参数已是 `long long`，直接算即可 | 把 `hp` 转成 `int` 再乘 100 → 溢出 |
| 2. `max_hp<=0` 返回 `-1`，不得除法 | 第一句就写 `if (max_hp <= 0) return -1;` | 先算后判 → 除零崩溃 |
| 2. `report` 的 HP 写 `INVALID`，电量照常 | HP 分支只影响一个字段，不能提前 `return ""` | 整个 report 返回空串，电量信息丢失 |
| 3. 先夹到 `[0,max_hp]`，再 `hp*100/max_hp` 取整数部分 | 两句 `if` 夹取 + 一次整除 | 用 `double`、或用 `+max_hp/2` 做四舍五入 |
| 4. `battery` 夹到 `[0,100]`，`≥60 OK`、`20..59 WARNING`、`<20 LOW` | 夹取后按 60 / 20 两个阈值分支 | 忘记夹取，`BAT=101` 或 `BAT=-7` 直接进字符串 |
| 5. 严格格式 `ID=<id>;HP=<..>;BAT=<..>;STATE=<..>` | `std::to_string` 拼接，分隔符分号 | 多空格、末尾换行、`%02d` 补零 |
| 6. 不修改参数、每次调用独立 | 参数按值传递，全部用局部变量 | 用 `static` 缓存上一次结果 |

---

## 4 四个必须想清楚的点

### 4.1 为什么中间乘积必须是 `long long`

题面给的上界是 10^12，夹取后 `hp <= max_hp <= 10^12`，所以第一步乘积最大是：

```
hp × 100 = 10^12 × 100 = 10^14
```

| 类型 | 能装下的最大值 | 10^14 能不能装下 |
| --- | --- | --- |
| `int`（32 位） | 2,147,483,647 ≈ 2.1×10^9 | 不能，差约 4.6 万倍 |
| `long long`（64 位） | 9,223,372,036,854,775,807 ≈ 9.2×10^18 | 能，余量约 9 万倍 |

结论：**先乘后除，且乘积在 `long long` 里算**。`hp` 和 `max_hp` 本身就是 `long long`，所以 `hp * 100` 天然是 64 位运算，只要中途不把它塞进 `int` 就安全。

顺带说明为什么不能"先除后乘"：`ratio(2,3)` 若写成 `hp/max_hp*100`，先得 `2/3 = 0`，再乘 100 还是 0，正确结果是 66。整数运算里，顺序就是精度。

### 4.2 为什么要"先夹后算"

| 输入 | 不夹取的结果 | 正确结果 | 原因 |
| --- | --- | --- | --- |
| `ratio(200, 100)` | 200 | 100 | 血量超过上限时按满血处理 |
| `ratio(-5, 100)` | -5 | 0 | 负数血量归零，不是"负百分比" |

注意 `-5 * 100 / 100` 在 C++ 里等于 -5（整除向零取整），并不会自动变 0，所以必须显式夹取。

### 4.3 整除天然就是"截断"，不需要也不能四舍五入

C++ 的整数除法向零取整。因为夹取后 `hp >= 0`，向零取整就等于向下取整，正好是题面要求的"整数部分"。

- 允许：`hp * 100 / max_hp` → `ratio(2,3) = 66`
- 禁止：`(hp * 100 + max_hp / 2) / max_hp` → 四舍五入得 67，判错
- 禁止：`(double)hp * 100 / max_hp` → 浮点结果 66.666…，不符合"禁止浮点计算"

### 4.4 除零不是异常，是未定义行为

`max_hp == 0` 时执行 `hp / max_hp`，C++ 标准里是未定义行为；在 Windows 上通常直接以 `0xC0000094`（整数除零）终止进程，**不会**抛 `std::exception`，你的 `main` 里的 `try/catch` 也接不住。所以规范 2 的"不得进行除法"不是风格建议，而是崩溃防护。

---

## 5 分步实现

### Step 1：`ratio` 的三道闸门

顺序不能换：**先判合法 → 再夹下界 → 再夹上界 → 最后运算**。

```cpp
long long ratio(long long hp, long long max_hp) {
    if (max_hp <= 0) return -1;   // 闸门 1：规范 2，之后才允许除法
    if (hp < 0) hp = 0;           // 闸门 2：规范 3 的下界
    if (hp > max_hp) hp = max_hp; // 闸门 3：规范 3 的上界
    return hp * 100 / max_hp;     // 规范 3：整数截断，long long 足够宽
}
```

小细节：`hp` 是值传递的副本，直接改它不影响调用方，满足规范 6 的"输入参数不会被修改"。

### Step 2：`report` 的 HP 字段复用 `ratio`

不要在 `report` 里把百分比逻辑再写一遍——两份实现迟早会不一致。判 `max_hp` 只需要决定"写数字还是写 INVALID"：

```cpp
std::string text = "ID=" + std::to_string(id) + ";HP=";
if (max_hp <= 0) text += "INVALID";
else             text += std::to_string(ratio(hp, max_hp));
```

注意这里**没有提前 return**：规范 2 明确"电量仍须正常处理"。

### Step 3：电量夹取与三档

```cpp
if (battery < 0) battery = 0;
if (battery > 100) battery = 100;
text += ";BAT=" + std::to_string(battery) + ";STATE=";
if (battery >= 60)      text += "OK";
else if (battery >= 20) text += "WARNING";
else                    text += "LOW";
```

`battery` 同样是值传递，改它安全。三档的边界是**闭区间左端**：60 属于 OK，20 属于 WARNING，19 属于 LOW。

### Step 4：格式的三个"不要"

| 不要 | 反例 | 后果 |
| --- | --- | --- |
| 不要补零 | `printf("ID=%02d", id)` → `ID=07` | 字符串比较直接不等 |
| 不要空格 | `"; HP="` 或 `"; "` | 分隔符被污染 |
| 不要末尾换行 | `+ "\n"` | 隐藏用例按字节比较，判错 |

统一的写法是 `std::to_string`：它输出最短十进制表示，不补零、不带空格、不带换行，正好符合规范 5。

---

## 6 完整参考实现

把下面内容整体替换掉 `src/main/A1_q1.cpp` 里 `// BEGIN IMPLEMENTATION` 到 `// END IMPLEMENTATION` 之间的两行 TODO 即可，标记行原样保留。

```cpp
// BEGIN IMPLEMENTATION
// aim-cpp-cw
long long ratio(long long hp, long long max_hp) {
    if (max_hp <= 0) return -1;
    if (hp < 0) hp = 0;
    if (hp > max_hp) hp = max_hp;
    return hp * 100 / max_hp;
}

std::string report(int id, long long hp, long long max_hp, int battery) {
    std::string text = "ID=" + std::to_string(id) + ";HP=";
    if (max_hp <= 0) text += "INVALID";
    else text += std::to_string(ratio(hp, max_hp));
    if (battery < 0) battery = 0;
    if (battery > 100) battery = 100;
    text += ";BAT=" + std::to_string(battery) + ";STATE=";
    if (battery >= 60) text += "OK";
    else if (battery >= 20) text += "WARNING";
    else text += "LOW";
    return text;
}
// END IMPLEMENTATION
```

这段代码在本机 `g++ 16.2.0 -std=c++17 -Wall -Wextra -Wconversion` 下零警告通过，公开自检输出 `PUBLIC 2/2`，第 9 节的 24 条边界断言全部通过。

---

## 7 交卷前自检清单（对照规范 1–6）

| 规范 | 代码里的体现 | 怎么验证 |
| --- | --- | --- |
| 1 数值范围 | 参数类型原样保留，未做 `int` 强转 | `grep -n "int"` 检查是否出现多余的窄化转换 |
| 2 无效上限 | `if (max_hp <= 0) return -1;` 位于除法之前；`report` 只换字段不提前返回 | `ratio(10,0)`、`report(0,4,0,-7)` |
| 3 夹取 + 整除 | 两行 `if` 夹取，`hp*100/max_hp` 一步完成 | `ratio(-5,100)=0`、`ratio(200,100)=100`、`ratio(2,3)=66` |
| 4 电量分档 | 先夹 `[0,100]`，再用 60 / 20 两个阈值 | `battery = 19/20/59/60/101` 五连测 |
| 5 输出格式 | 四个分号、无空格、无补零、无换行 | 用 `==` 比较整串，而不是 `find` 或肉眼比对 |
| 6 参数与独立性 | 全部值传递，无 `static`、无全局可变状态 | 连续调用两次同一输入，结果必须完全一致 |

---

## 8 编译与运行

在仓库根目录：

```bash
# 单题编译（Windows 上把输出名写成 a1_q1.exe）
g++ -std=c++17 -Wall -Wextra src/main/A1_q1.cpp -o a1_q1
./a1_q1
```

期望输出：

```
PUBLIC 2/2
```

未实现时输出 `TODO` 并返回 1，属预期状态。补充两点：

- `PUBLIC 2/2` 只代表两组公开调用通过，不代表隐藏用例全过，也不代表拿到 8 分。
- 也可以用教师同引擎跑一遍：`python3 tools/check.py . --only q1`（本机 Windows 用 `python`）。

---

## 9 边界自测（本机实测结果）

题面要求自测覆盖 `hp<0`、`hp>max_hp`、`max_hp=0`、`battery` 为 19/20/59/60，这里再补大数与上限为负的情况。

| # | 调用 | 预期 | 实测 | 覆盖点 |
| --- | --- | --- | --- | --- |
| 1 | `ratio(2,3)` | `66` | 66 | 截断，公开用例 |
| 2 | `ratio(-5,100)` | `0` | 0 | 负血量归零 |
| 3 | `ratio(200,100)` | `100` | 100 | 超上限夹到满血 |
| 4 | `ratio(10,0)` | `-1` | -1 | 上限无效，不除法 |
| 5 | `ratio(999999999999, 1000000000000)` | `99` | 99 | 10^12 级乘法不溢出 |
| 6 | `ratio(1000000000000, 1)` | `100` | 100 | 夹取后必为 100 |
| 7 | `report(7,2,3,20)` | `ID=7;HP=66;BAT=20;STATE=WARNING` | 一致 | 公开用例 + 20 边界 |
| 8 | `report(0,4,0,-7)` | `ID=0;HP=INVALID;BAT=0;STATE=LOW` | 一致 | 无效上限 + 负电量 |
| 9 | `report(3,5,-5,50)` | `ID=3;HP=INVALID;BAT=50;STATE=WARNING` | 一致 | 上限为负也走 INVALID |
| 10 | `report(1,50,100,19)` | `...BAT=19;STATE=LOW` | 一致 | LOW 上边界 |
| 11 | `report(1,50,100,59)` | `...BAT=59;STATE=WARNING` | 一致 | WARNING 上边界 |
| 12 | `report(1,50,100,60)` | `...BAT=60;STATE=OK` | 一致 | OK 下边界 |
| 13 | `report(1,50,100,101)` | `...BAT=100;STATE=OK` | 一致 | 电量上限夹取 |
| 14 | `report(1,50,100,INT_MIN)` | `...BAT=0;STATE=LOW` | 一致 | 电量下界夹取，无溢出 |

可直接运行的自测程序见同目录 `q1_extra.cpp`（它靠相对路径 `../src/main/A1_q1.cpp` 引入被测实现，所以必须和实现文件待在同一个仓库里）：

```bash
g++ -std=c++17 -Wall -Wextra local_tests/q1_extra.cpp -o q1_extra
./q1_extra          # 期望输出 Q1 EXTRA ALL PASS
```

### 一次真实的期望值写错（缺陷定位记录）

写第 11 条用例时，我把预期写成了 `ID=1;HP=59;BAT=59;STATE=WARNING`，运行后报：

```
FAIL report(1,50,100,59) got=[ID=1;HP=50;BAT=59;STATE=WARNING] want=[ID=1;HP=59;BAT=59;STATE=WARNING]
```

定位方法：先看差异字段——只有 `HP` 不同，而 `BAT` 两边一致，说明分档逻辑没问题，问题在"预期值"这一侧。手算 `hp=50, max_hp=100` → `50*100/100 = 50`，所以 `HP=50` 才是对的，是**测试的期望值写错**（把电量 59 误抄进了 HP 字段）。

修复：改正预期字符串后重跑，全部通过。教训是——**失败时先确认是代码错还是期望错**，两条都用同一份手算草稿交叉验证；如果直接去改实现代码迁就错误的期望，就会把正确逻辑改坏。

---

## 10 常见错误对照表

| 错误写法 | 现象 | 正确做法 |
| --- | --- | --- |
| `return hp / max_hp * 100;` | `ratio(2,3)` 得 0 | 先乘后除 |
| `return (int)(hp * 100.0 / max_hp);` | 引入浮点，违反规范 3 | 全程整数 |
| `int hp100 = hp * 100;` | 10^12 级输入溢出成随机负数 | 保持 `long long` |
| 先算 `hp*100/max_hp` 再判 `max_hp<=0` | 除零，进程崩溃（非异常） | 第一句就判 |
| `if (max_hp <= 0) return "";` | 丢了 BAT 与 STATE | 只替换 HP 字段 |
| `text += "; "` 或 `"ID = "` | 字符串比较不等 | 严格按 `ID=`、`;HP=` 拼接 |
| `snprintf(..., "%02d", id)` | `ID=07` | `std::to_string` |
| `text += "\n";` | 末尾多一个字节 | 不加换行 |
| `static std::string last;` 缓存 | 第二次调用沿用旧值，违反规范 6 | 全部局部变量 |
| 结果为 `double` 再 `to_string` | 输出 `66.000000` | 整数类型 `to_string` |

---

## 11 复杂度与验收要点

- **时间复杂度**：O(1)，固定次数的比较、一次乘法、一次除法、若干次字符串拼接（字符串长度不超过几十字节）。
- **空间复杂度**：O(1) 额外空间，只用到返回的字符串本身。
- Q1 没有循环、没有容器，所以不存在性能陷阱；评分只看行为是否严格符合规范 1–6。
- 提交前最后确认三件事：只改了实现区、`-Wall -Wextra` 零警告、公开自检 2/2。
