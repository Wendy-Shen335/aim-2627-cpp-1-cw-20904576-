#define main aim_embedded_public_main
#include "../src/main/A1_q6.cpp"
#undef main

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

using aim::q6::Target;
using aim::q6::select_target;

static int failures = 0;

static void same(const char* what, const std::vector<Target>& targets,
                 int range, int want) {
    const int got = select_target(targets, range);
    if (got == want) return;
    ++failures;
    std::printf("FAIL %s: got %d want %d\n", what, got, want);
}

int main() {
    // 题面样例
    same("样例1 三级平局", {{7,3,4,20,true},{2,0,5,20,true}}, 5, 2);
    same("样例2 血量优先", {{7,3,4,1,true},{2,0,0,2,true}}, 5, 7);
    same("样例3 血量零淘汰", {{1,0,0,0,true}}, 1, -1);

    // 题面隐藏边界速查表
    same("平局按编号 与顺序无关", {{9,0,0,3,true},{1,0,0,3,true}}, 0, 1);
    same("射程包含边界", {{1,3,4,1,true}}, 5, 1);
    same("无候选", {}, 5, -1);

    // 基础筛选
    same("空输入负射程", {}, -1, -1);
    same("负射程直接返回", {{1,0,0,5,true}}, -1, -1);
    same("不可见目标", {{9,0,0,1,false}}, 5, -1);
    same("负血量目标", {{9,0,0,-1,true}}, 5, -1);
    same("超射程目标", {{9,1000000,1000000,1,true}}, 5, -1);
    same("射程外刚好差1", {{9,3,4,1,true}}, 4, -1);

    // 三级比较
    same("血量第一优先", {{1,0,0,5,true},{2,0,0,1,true}}, 5, 2);
    same("同血量距离优先", {{9,3,4,5,true},{1,0,1,5,true}}, 5, 1);
    same("同血量同距离编号优先", {{5,-3,4,5,true},{2,3,-4,5,true}}, 5, 2);
    same("平局逆序输入", {{2,3,-4,5,true},{5,-3,4,5,true}}, 5, 2);

    // 整数宽度：百万级坐标平方和必须 long long
    same("百万坐标平方和", {{1,1000000,1000000,1,true}}, 1414214, 1);
    same("百万坐标超射程", {{9,1000000,1000000,1,true}}, 100000, -1);

    // O(n) 规模：10 万个目标，最后一个最优
    {
        std::vector<Target> many;
        many.reserve(100000);
        for (int i = 0; i < 99999; ++i) {
            many.push_back({i, 0, 0, 100, true});
        }
        many.push_back({99999, 0, 0, 1, true});
        same("十万目标", many, 5, 99999);
    }

    // 不修改输入
    {
        std::vector<Target> ts = {{7,3,4,20,true},{2,0,5,20,true}};
        const auto before = ts;
        (void)select_target(ts, 5);
        bool same_input = ts.size() == before.size();
        for (std::size_t i = 0; same_input && i < ts.size(); ++i) {
            if (ts[i].id != before[i].id || ts[i].x != before[i].x ||
                ts[i].y != before[i].y || ts[i].hp != before[i].hp ||
                ts[i].visible != before[i].visible) same_input = false;
        }
        if (!same_input) { ++failures; std::printf("FAIL 输入被修改\n"); }
    }

    if (failures == 0) { std::printf("Q6 EXTRA ALL PASS\n"); return 0; }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
