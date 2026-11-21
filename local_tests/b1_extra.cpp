#define main aim_embedded_public_main
#include "../src/main/A1_b1.cpp"
#undef main

#include <cstdio>
#include <string>
#include <vector>

using aim::b1::parse_i32;

static int failures = 0;

static void ok(const char* what, const std::string& text, int want) {
    int out = 12345;
    const bool got = parse_i32(text, out);
    if (got && out == want) return;
    ++failures;
    std::printf("FAIL %s [%s]: got bool=%d out=%d want true/%d\n",
                what, text.c_str(), got ? 1 : 0, out, want);
}

static void bad(const char* what, const std::string& text) {
    int out = 12345;
    const bool got = parse_i32(text, out);
    if (!got && out == 12345) return;
    ++failures;
    std::printf("FAIL %s [%s]: got bool=%d out=%d want false/12345\n",
                what, text.c_str(), got ? 1 : 0, out);
}

int main() {
    // 题面样例
    ok("样例1 最小负数", "-2147483648", -2147483648);
    bad("样例2 超最大正数", "2147483648");
    ok("样例3 负零", "-0000", 0);
    ok("公开用例 负12", "-12", -12);
    bad("公开用例 尾缀", "12x");

    // 端点
    ok("最大正数", "2147483647", 2147483647);
    ok("最小负数", "-2147483648", -2147483648);
    bad("最大正数加一", "2147483648");
    bad("最小负数减一", "-2147483649");
    ok("正数端点内一位", "2147483646", 2147483646);
    ok("负数端点内一位", "-2147483647", -2147483647);

    // 语法
    ok("正号", "+42", 42);
    ok("正零", "+0", 0);
    ok("负零单", "-0", 0);
    ok("前导零", "0000000123", 123);
    bad("空串", "");
    bad("单独正号", "+");
    bad("单独负号", "-");
    bad("双正号", "++1");
    bad("双负号", "--1");
    bad("符号在中间", "1-2");
    bad("前导空白", " 1");
    bad("尾随空白", "1 ");
    bad("小数点", "1.0");
    bad("字母开头", "x1");
    bad("非ASCII数字", "１２３");
    bad("十六进制", "0x10");

    // 长度上限
    ok("1000位前导零", std::string(999, '0') + "7", 7);
    bad("1001位数字", std::string(1001, '1'));

    // 失败后 output 不变（用非默认值再测一次）
    {
        int out = -77;
        if (parse_i32("999999999999", out) || out != -77) {
            ++failures;
            std::printf("FAIL 超长数失败后 output 被改: %d\n", out);
        }
    }

    if (failures == 0) { std::printf("B1 EXTRA ALL PASS\n"); return 0; }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
