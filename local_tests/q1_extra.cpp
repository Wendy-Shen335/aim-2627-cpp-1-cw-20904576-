// Q1 自己补充的边界测试（不属于 src/tests，不会被 CI 当作公开测试改写）
// 编译：g++ -std=c++17 -Wall -Wextra local_tests/q1_extra.cpp -o q1_extra
// 运行：./q1_extra        期望输出 Q1 EXTRA ALL PASS
#define main aim_embedded_public_main
#include "../src/main/A1_q1.cpp"
#undef main

#include <climits>
#include <cstdio>
#include <string>

using aim::q1::ratio;
using aim::q1::report;

static int failures = 0;

static void same(const char* label, const std::string& got, const std::string& want) {
    if (got != want) {
        std::printf("FAIL %s: got [%s] want [%s]\n", label, got.c_str(), want.c_str());
        ++failures;
    }
}

static void same_ratio(const char* label, long long got, long long want) {
    if (got != want) {
        std::printf("FAIL %s: got %lld want %lld\n", label, got, want);
        ++failures;
    }
}

int main() {
    same_ratio("ratio(2,3)", ratio(2, 3), 66);
    same_ratio("ratio(-5,100)", ratio(-5, 100), 0);
    same_ratio("ratio(200,100)", ratio(200, 100), 100);
    same_ratio("ratio(10,0)", ratio(10, 0), -1);
    same_ratio("ratio(1,3)", ratio(1, 3), 33);
    same_ratio("ratio(199,200)", ratio(199, 200), 99);
    same_ratio("ratio(1e12,1e12)", ratio(1000000000000LL, 1000000000000LL), 100);
    same_ratio("ratio(1e12-1,1e12)", ratio(999999999999LL, 1000000000000LL), 99);
    same_ratio("ratio(-1e12,1e12)", ratio(-1000000000000LL, 1000000000000LL), 0);
    same_ratio("ratio(1e12,1)", ratio(1000000000000LL, 1), 100);

    same("report 公开", report(7, 2, 3, 20), "ID=7;HP=66;BAT=20;STATE=WARNING");
    same("report 无效上限", report(0, 4, 0, -7), "ID=0;HP=INVALID;BAT=0;STATE=LOW");
    same("report 上限为负", report(3, 5, -5, 50), "ID=3;HP=INVALID;BAT=50;STATE=WARNING");
    same("report bat=19", report(1, 50, 100, 19), "ID=1;HP=50;BAT=19;STATE=LOW");
    same("report bat=20", report(1, 50, 100, 20), "ID=1;HP=50;BAT=20;STATE=WARNING");
    same("report bat=59", report(1, 50, 100, 59), "ID=1;HP=50;BAT=59;STATE=WARNING");
    same("report bat=60", report(1, 50, 100, 60), "ID=1;HP=50;BAT=60;STATE=OK");
    same("report bat=101", report(1, 50, 100, 101), "ID=1;HP=50;BAT=100;STATE=OK");
    same("report bat=INT_MIN", report(1, 50, 100, INT_MIN), "ID=1;HP=50;BAT=0;STATE=LOW");
    same("report 上界", report(9999, 1000000000000LL, 1000000000000LL, 60),
         "ID=9999;HP=100;BAT=60;STATE=OK");

    if (failures == 0) {
        std::printf("Q1 EXTRA ALL PASS\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
