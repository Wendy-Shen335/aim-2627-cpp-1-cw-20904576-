#define main aim_embedded_public_main
#include "../src/main/A1_q7.cpp"
#undef main

#include <cstdio>
#include <vector>

using aim::q7::FireResult;
using aim::q7::simulate;

static int failures = 0;

static void same(const char* what, const FireResult& got,
                 int shots, int rejected, int ammo, int heat) {
    if (got.shots == shots && got.rejected == rejected &&
        got.ammo == ammo && got.heat == heat) return;
    ++failures;
    std::printf("FAIL %s: got {%d,%d,%d,%d} want {%d,%d,%d,%d}\n",
                what, got.shots, got.rejected, got.ammo, got.heat,
                shots, rejected, ammo, heat);
}

int main() {
    // 题面样例
    same("样例1 逐帧顺序", simulate(2,90,10,20,{1,1,1}), 2,1,0,100);
    same("样例2 只冷却", simulate(3,100,20,10,{0}), 0,0,3,80);
    same("样例3 零射击热量", simulate(1,100,0,0,{1}), 1,0,0,100);

    // 题面隐藏边界速查表
    same("空请求不冷却", simulate(4,80,10,30,{}), 0,0,4,80);
    same("无弹也先冷却", simulate(0,50,10,20,{1}), 0,1,0,40);
    same("零射击热量合法", simulate(1,100,0,0,{1}), 1,0,0,100);
    same("拒绝不扣弹", simulate(1,100,0,1,{1}), 0,1,1,100);

    // 冷却顺序陷阱：先判后冷会得到不同结果
    same("先冷却才允许", simulate(1,100,20,20,{1}), 1,0,0,100);
    same("冷却不足仍拒绝", simulate(1,100,19,20,{1}), 0,1,1,81);

    // 冷却夹零
    same("冷却夹到零", simulate(2,5,20,10,{1}), 1,0,1,10);
    same("请求0也夹零", simulate(2,5,20,10,{0}), 0,0,2,0);

    // 热量上限恰好等于允许
    same("上界等于100允许", simulate(1,100,10,10,{1}), 1,0,0,100);
    same("上界超1拒绝", simulate(1,100,10,11,{1}), 0,1,1,90);

    // 零冷却、零热量、空弹组合
    same("零冷却全发", simulate(2,0,0,0,{1,1,1}), 2,1,0,0);
    same("零射击热量无上限问题", simulate(3,100,0,0,{1,1,1}), 3,0,0,100);
    same("空弹多请求", simulate(0,0,0,0,{1,1,1}), 0,3,0,0);

    // 混合序列：失败后可继续
    same("混合请求", simulate(2,90,10,20,{1,1,1}), 2,1,0,100);
    same("失败后冷却再成功", simulate(2,100,20,20,{1,1,1}), 2,1,0,80);

    // 规模上限：10000 个请求，全 0 只冷却
    {
        std::vector<int> many(10000, 0);
        same("10000帧全0", simulate(100,100,1,10,many), 0,0,100,0);
    }
    // 10000 帧全 1，弹量不足后全部拒绝
    {
        std::vector<int> many(10000, 1);
        same("10000帧全1", simulate(3,0,0,0,many), 3,9997,0,0);
    }

    // 调用独立
    {
        FireResult a = simulate(1,50,10,10,{1});
        FireResult b = simulate(1,50,10,10,{1});
        same("独立调用 第1次", a, 1,0,0,50);
        same("独立调用 第2次", b, 1,0,0,50);
    }

    if (failures == 0) { std::printf("Q7 EXTRA ALL PASS\n"); return 0; }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
