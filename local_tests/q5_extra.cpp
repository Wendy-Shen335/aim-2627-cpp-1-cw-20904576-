#define main aim_embedded_public_main
#include "../src/main/A1_q5.cpp"
#undef main

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

using Matrix = std::vector<std::vector<int>>;

static int failures = 0;

static void same(const char* what, const Matrix& got, const Matrix& want) {
    if (got == want) return;
    ++failures;
    std::printf("FAIL %s\n  got  =", what);
    for (const auto& row : got) { std::printf(" {"); for (int v : row) std::printf("%d,", v); std::printf("}"); }
    std::printf("\n  want =");
    for (const auto& row : want) { std::printf(" {"); for (int v : row) std::printf("%d,", v); std::printf("}"); }
    std::printf("\n");
}

static void invalid(const char* what, const std::vector<std::string>& grid) {
    try {
        (void)aim::q5::risk(grid);
    } catch (const std::invalid_argument&) {
        return;
    } catch (const std::exception& e) {
        ++failures;
        std::printf("FAIL %s: threw %s (want invalid_argument)\n", what, typeid(e).name());
        return;
    }
    ++failures;
    std::printf("FAIL %s: no exception thrown\n", what);
}

int main() {
    using aim::q5::risk;

    // 题面样例
    same("样例1 右上障碍", risk({".#", ".."}), {{1, -1}, {0, 1}});
    same("样例2 单格空地", risk({"."}), {{0}});
    same("样例3 中心四邻域", risk({".#.", "#.#", ".#."}), {{2, -1, 2}, {-1, 4, -1}, {2, -1, 2}});
    invalid("样例4 行长不一致", {"..", "."});

    // 题面隐藏边界速查表
    same("空地图", risk({}), {});
    invalid("有行零列", {""});
    same("单列", risk({".", "#"}), {{1}, {-1}});
    same("对角不计", risk({"#.", ".."}), {{-1, 1}, {1, 0}});

    // 自补：字符与验证顺序
    invalid("非法字符 首行", {"#x"});
    invalid("非法字符 最后一行最后一位", {".#", "#.", "..x"});
    invalid("行长不一致 最后一行", {"..", "..", "."});
    invalid("零列 第二行", {".", ""});
    invalid("数字字符", {"1"});
    invalid("空格字符", {". "});

    // 自补：规模与取值
    same("全障碍 2x2", risk({"##", "##"}), {{-1, -1}, {-1, -1}});
    same("全空地 3x3", risk({"...", "...", "..."}), {{0,0,0},{0,0,0},{0,0,0}});
    same("单行", risk({"...."}), {{0, 0, 0, 0}});
    same("边缘不当墙", risk({"..#", "..."}), {{0, 1, -1}, {0, 0, 1}});

    // 输入不被修改
    {
        std::vector<std::string> grid = {".#", ".."};
        const auto before = grid;
        (void)risk(grid);
        if (grid != before) { ++failures; std::printf("FAIL 输入被修改\n"); }
    }

    // 规模上限 300x300：全空地应全 0，且能正常返回
    {
        std::vector<std::string> big(300, std::string(300, '.'));
        Matrix r = risk(big);
        bool ok = (r.size() == 300);
        for (const auto& row : r) {
            if (row.size() != 300) { ok = false; break; }
            for (int v : row) if (v != 0) { ok = false; break; }
        }
        if (!ok) { ++failures; std::printf("FAIL 300x300 全空地\n"); }
    }

    if (failures == 0) { std::printf("Q5 EXTRA ALL PASS\n"); return 0; }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
