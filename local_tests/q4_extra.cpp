#define main aim_embedded_public_main
#include "../src/main/A1_q4.cpp"
#undef main

#include <cstdio>
#include <string>
#include <vector>

using aim::q4::CommandResult;
using aim::q4::execute;

static int failures = 0;

static void same(const char* what, const CommandResult& got,
                 long long value, int accepted, int rejected) {
    if (got.value == value && got.accepted == accepted && got.rejected == rejected) return;
    ++failures;
    std::printf("FAIL %s\n  got  {%lld,%d,%d}\n  want {%lld,%d,%d}\n",
                what, got.value, got.accepted, got.rejected, value, accepted, rejected);
}

int main() {
    // 题面样例
    same("样例1", execute({"ADD 5", "SET 8 x", "RESET", "ADD -2"}), -2, 3, 1);
    same("样例2 空输入", execute({}), 0, 0, 0);
    same("样例3 候选超界", execute({"SET 1000000", "ADD 1"}), 1000000, 1, 1);

    // 题面隐藏边界速查表
    same("非法 RESET 不清零", execute({"SET 5", "RESET x"}), 5, 1, 1);
    same("大小写 + 空行", execute({"add 5", ""}), 0, 0, 2);
    same("合法零值操作", execute({"ADD +2", "RESET"}), 0, 2, 0);

    // 单条失败命令：确认没有留下半套修改
    same("SET 带尾巴单独出现", execute({"SET 8 x"}), 0, 0, 1);
    same("ADD 带尾巴单独出现", execute({"ADD 12x"}), 0, 0, 1);

    // 解析细节
    same("空行与纯空白", execute({"", "   ", "\t"}), 0, 0, 3);
    same("大小写敏感", execute({"Add 5", "ADD 5", "add 5", "SET 3", "set 3", "RESET", "reset"}), 0, 3, 4);
    same("多余字段", execute({"ADD 5 6", "SET 3 x", "RESET y"}), 0, 0, 3);
    same("缺字段", execute({"ADD", "SET", "RESET"}), 0, 1, 2);
    same("小数尾缀", execute({"ADD 1.5"}), 0, 0, 1);
    same("十六进制样式尾缀", execute({"SET 0x10"}), 0, 0, 1);
    same("双负号", execute({"ADD --2"}), 0, 0, 1);
    same("正负号与零", execute({"ADD +2", "SET -3", "ADD +0"}), -3, 3, 0);
    same("未知操作", execute({"MUL 2", "PRINT", "add 5"}), 0, 0, 3);
    same("首尾与中间空白", execute({"  ADD   5  ", "\tSET\t3\t"}), 3, 2, 0);
    same("RESET 后只有空白", execute({"SET 9", "RESET   "}), 0, 2, 0);

    // 数值范围
    same("参数本身越界", execute({"ADD 1000001", "SET -1000001"}), 0, 0, 2);
    same("超长整数提取失败", execute({"ADD 99999999999999999999999", "SET 99999999999999999999999"}), 0, 0, 2);
    same("候选下界越界", execute({"SET -1000000", "ADD -1"}), -1000000, 1, 1);
    same("恰好等于上下界", execute({"SET 1000000", "SET -1000000"}), -1000000, 2, 0);
    same("前导零", execute({"ADD 0000005"}), 5, 1, 0);

    // 事务性与统计
    same("失败后继续处理", execute({"ADD 1", "BAD", "ADD 2", "SET 8 x", "RESET", "ADD 5"}), 5, 4, 2);
    same("调用之间独立 第1次", execute({"SET 7"}), 7, 1, 0);
    same("调用之间独立 第2次", execute({"SET 7"}), 7, 1, 0);

    // 规模上限：10000 行合法命令
    {
        std::vector<std::string> many(10000, "ADD 0");
        same("10000 行", execute(many), 0, 10000, 0);
    }

    // 计数守恒：混合输入
    {
        std::vector<std::string> mix = {"ADD 1", "BAD", "", "SET 2", "RESET x", "ADD -1000001"};
        CommandResult r = execute(mix);
        same("混合输入结果", r, 2, 2, 4);
        if (r.accepted + r.rejected != static_cast<int>(mix.size())) {
            ++failures;
            std::printf("FAIL 计数守恒: %d + %d != %zu\n",
                        r.accepted, r.rejected, mix.size());
        }
    }

    if (failures == 0) { std::printf("Q4 EXTRA ALL PASS\n"); return 0; }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
