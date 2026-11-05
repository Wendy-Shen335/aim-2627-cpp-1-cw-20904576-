# 项目长期记忆 · AIM 2627 C++ CW1

## 用户约定（必须遵守）

- **文档位置：`local_tests/` 内可以放**（已实测 CI 通过），教程/笔记就放 `local_tests/`；也可放仓库外 `D:\workbuddy工作空间\`。**绝不能放** `docs/`、仓库根目录、`.workbuddy/`（实测都会让 CI 红）。
- 仓库内只允许出现：`src/main/A1_*.cpp` 的实现、`README.md`、`local_tests/` 下任意文件、`.agent-sessions/bundles/`。
- 提交时用明确路径 `git add <file>`，**禁止** `git add -A` / `git add .`：`.workbuddy/` 未被忽略，会被扫进提交并让 CI 变红。

## 关键事实（已核实的机制）

- `tools/ci_support.py` 的 `ALLOWED` 正则：`^(src/main/A1_(q[1-8]|b[12])\.cpp|README\.md|local_tests/.+|\.agent-sessions/bundles/.+)$`。
- `check_changes()` 用 `git diff --name-only <初始commit> HEAD`，**只统计已提交的内容**；未提交/未跟踪文件不影响 CI。
- `.gitignore` 只忽略 `build/`、`scores/`、`local_tests/build/`、`*.exe`、`.DS_Store`、`.venv*/`、`.agent-sessions/source/`；**`.workbuddy/` 未被忽略**。
- 编译环境：MSYS2 `g++ 16.2.0`，用 `-std=c++17 -Wall -Wextra` 验证。
- `local_tests/q1_extra.cpp` 依赖相对路径 `../src/main/A1_q1.cpp`，必须留在仓库内才能编译。

## Q1 结论摘要

- `max_hp<=0` 先判再除（除零是 UB 非异常）；夹 `hp` 到 `[0,max_hp]` 后 `hp*100/max_hp`；`battery` 夹 `[0,100]` 后按 60/20 分档；格式 `ID=;HP=;BAT=;STATE=`，无空格无补零无换行。
- 教程：`local_tests/q1_tutorial.md`（296 行，随仓库提交，已实测范围检查通过）。
