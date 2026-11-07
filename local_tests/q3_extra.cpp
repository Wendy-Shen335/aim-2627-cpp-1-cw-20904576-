#define main aim_embedded_public_main
#include "../src/main/A1_q3.cpp"
#undef main

#include <cstdio>
#include <vector>

using aim::q3::alarms;

static int failures = 0;
static void same(const char* what, const std::vector<int>& got, const std::vector<int>& want) {
    if (got == want) return;
    ++failures;
    std::printf("FAIL %s\n  got  [", what);
    for (int v : got) std::printf("%d ", v);
    std::printf("]\n  want [");
    for (int v : want) std::printf("%d ", v);
    std::printf("]\n");
}

int main() {
    // 题面样例
    same("样例1", alarms({1,2,3,4}, 2, 4), {2,3});
    same("样例2", alarms({2,2}, 2, 4), {});
    same("样例3", alarms({-4,-3,-2}, 2, -6), {2});
    // 题面边界表
    same("相等不报警", alarms({2,2}, 2, 4), {});
    same("负数窗口", alarms({-4,-3,-2}, 2, -6), {2});
    same("k=0 非法", alarms({1}, 0, 0), {});
    same("k=1", alarms({1,2}, 1, 1), {1});
    // 自补
    same("空输入", alarms({}, 1, 0), {});
    same("k>n", alarms({1,2}, 3, 0), {});
    same("k=n", alarms({1,2,3}, 3, 5), {2});
    same("k=n 不超", alarms({1,2,3}, 3, 6), {});
    same("全相等恰等于阈值", alarms({1,1,1}, 3, 3), {});
    same("全负数都超", alarms({-1,-2,-3}, 2, -100), {1,2});
    same("单元素超阈值", alarms({5}, 1, 4), {0});
    same("大数防溢出", alarms({1000000000,1000000000,1000000000,1000000000,1000000000}, 5, 4999999999LL), {4});
    same("大数恰好等于", alarms({1000000000,1000000000,1000000000,1000000000,1000000000}, 5, 5000000000LL), {});
    same("负阈值", alarms({-1000000000,-1000000000}, 2, -2000000001LL), {1});
    if (failures == 0) { std::printf("Q3 EXTRA ALL PASS\n"); return 0; }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
