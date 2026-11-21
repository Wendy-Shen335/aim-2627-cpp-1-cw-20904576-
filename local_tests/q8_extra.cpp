#define main aim_embedded_public_main
#include "../src/main/A1_q8.cpp"
#undef main

#include <algorithm>
#include <cstdio>
#include <vector>

using aim::q8::Summary;
using aim::q8::Telemetry;
using aim::q8::summarize;

static int failures = 0;

static void same(const char* what, const Summary& got,
                 int valid, int duplicate, int invalid, long long total_hp,
                 const std::vector<int>& ids) {
    const std::vector<int> want_ids = ids;
    if (got.valid == valid && got.duplicate == duplicate && got.invalid == invalid &&
        got.total_hp == total_hp && got.ids == want_ids) return;
    ++failures;
    std::printf("FAIL %s\n  got  {%d,%d,%d,%lld,[", what,
                got.valid, got.duplicate, got.invalid, got.total_hp);
    for (int v : got.ids) std::printf("%d,", v);
    std::printf("]}\n  want {%d,%d,%d,%lld,[", valid, duplicate, invalid, total_hp);
    for (int v : want_ids) std::printf("%d,", v);
    std::printf("]}\n");
}

static void conservation(const char* what, const Summary& s, std::size_t n) {
    if (s.valid + s.duplicate + s.invalid == static_cast<int>(n) &&
        s.valid == static_cast<int>(s.ids.size())) return;
    ++failures;
    std::printf("FAIL %s: 守恒破坏 valid=%d dup=%d inv=%d ids=%zu n=%zu\n",
                what, s.valid, s.duplicate, s.invalid, s.ids.size(), n);
}

int main() {
    // 题面样例
    {
        std::vector<Telemetry> f = {{2,200},{1,-5},{2,10},{-1,30}};
        Summary r = summarize(f);
        same("样例1", r, 2,1,1,100, {1,2});
        conservation("样例1 守恒", r, f.size());
    }
    {
        std::vector<Telemetry> f = {{1,-1001},{1,30},{1,1001}};
        Summary r = summarize(f);
        same("样例2", r, 1,0,2,30, {1});
        conservation("样例2 守恒", r, f.size());
    }

    // 题面隐藏边界速查表
    same("非法不占编号", summarize({{1,-1001},{1,30}}), 1,0,1,30, {1});
    same("先验合法再判重复", summarize({{1,20},{1,-1001}}), 1,0,1,20, {1});
    same("编号排序非到达顺序", summarize({{2,5},{1,9}}), 2,0,0,14, {1,2});
    same("空批次", summarize({}), 0,0,0,0, {});

    // id 边界
    same("id 下界合法", summarize({{0,50}}), 1,0,0,50, {0});
    same("id 上界合法", summarize({{9999,50}}), 1,0,0,50, {9999});
    same("id 负一非法", summarize({{-1,50}}), 0,0,1,0, {});
    same("id 上万非法", summarize({{10000,50}}), 0,0,1,0, {});

    // hp 边界与夹取
    same("hp 下界合法夹零", summarize({{1,-1000}}), 1,0,0,0, {1});
    same("hp 上界合法夹百", summarize({{1,1000}}), 1,0,0,100, {1});
    same("hp 负一非法", summarize({{1,-1001}}), 0,0,1,0, {});
    same("hp 超千非法", summarize({{1,1001}}), 0,0,1,0, {});
    same("hp 零", summarize({{1,0}}), 1,0,0,0, {1});
    same("hp 一百", summarize({{1,100}}), 1,0,0,100, {1});

    // 重复与非法混合
    same("重复不累加", summarize({{1,10},{1,90}}), 1,1,0,10, {1});
    same("已见编号的非法仍计非法", summarize({{1,20},{1,2000}}), 1,0,1,20, {1});
    same("非法后补发", summarize({{1,-5000},{1,7}}), 1,0,1,7, {1});
    same("三个同编号", summarize({{5,1},{5,2},{5,3}}), 1,2,0,1, {5});
    same("乱序多编号", summarize({{9,10},{3,20},{7,30}}), 3,0,0,60, {3,7,9});

    // 调用独立
    {
        Summary a = summarize({{1,50}});
        Summary b = summarize({{1,50}});
        same("独立调用1", a, 1,0,0,50, {1});
        same("独立调用2", b, 1,0,0,50, {1});
    }

    // 规模上限 20000 条，全部不同编号不可能（只有 10000 个），用 20000 条含重复
    {
        std::vector<Telemetry> many;
        many.reserve(20000);
        for (int i = 0; i < 10000; ++i) many.push_back({i, 50});
        for (int i = 0; i < 10000; ++i) many.push_back({i, 99});
        Summary r = summarize(many);
        if (r.valid != 10000 || r.duplicate != 10000 || r.invalid != 0 ||
            r.total_hp != 500000 || r.ids.size() != 10000) {
            ++failures;
            std::printf("FAIL 20000 条规模: valid=%d dup=%d inv=%d total=%lld ids=%zu\n",
                        r.valid, r.duplicate, r.invalid, r.total_hp, r.ids.size());
        }
    }

    // 输入不被修改
    {
        std::vector<Telemetry> f = {{2,200},{1,-5}};
        const auto before = f;
        (void)summarize(f);
        bool same_input = f.size() == before.size();
        for (std::size_t i = 0; same_input && i < f.size(); ++i)
            if (f[i].id != before[i].id || f[i].hp != before[i].hp) same_input = false;
        if (!same_input) { ++failures; std::printf("FAIL 输入被修改\n"); }
    }

    if (failures == 0) { std::printf("Q8 EXTRA ALL PASS\n"); return 0; }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
