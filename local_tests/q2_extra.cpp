// Q2 自己补充的边界测试（放在 local_tests/，不改 src/tests）
// 编译：g++ -std=c++17 -Wall -Wextra local_tests/q2_extra.cpp -o q2_extra
// 运行：./q2_extra        期望输出 Q2 EXTRA ALL PASS
#define main aim_embedded_public_main
#include "../src/main/A1_q2.cpp"
#undef main

#include <cstdio>
#include <string>

using aim::q2::LabelEdit;
using aim::q2::edit_labels;

static int failures = 0;

static void same(const char* what, const std::string& got, const std::string& want) {
    if (got != want) {
        std::printf("FAIL %s\n  got  [%s]\n  want [%s]\n", what, got.c_str(), want.c_str());
        ++failures;
    }
}

int main() {
    // 题面三组样例
    same("样例1", edit_labels("cam,imu,radar", {{"imu","gyro"},{"cam",""},{"","lidar"}}), "gyro,radar,lidar");
    same("样例2", edit_labels("cam,cam2,cam", {{"cam","camera"},{"cam",""}}), "camera,cam2");
    same("样例3", edit_labels("cam", {{"cam",""},{"","imu"}}), "imu");

    // 题面边界表
    same("只匹配完整标签", edit_labels("cam2,cam", {{"cam","imu"}}), "cam2,imu");
    same("只改首次命中", edit_labels("a,b,a", {{"a","x"}}), "x,b,a");
    same("删末尾", edit_labels("a,b", {{"b",""}}), "a");
    same("先清空再追加", edit_labels("a", {{"a",""},{"","b"}}), "b");

    // 自补边界
    same("空文本+空编辑", edit_labels("", {}), "");
    same("空文本+追加", edit_labels("", {{"","x"}}), "x");
    same("删首个", edit_labels("a,b,c", {{"a",""}}), "b,c");
    same("删中间", edit_labels("a,b,c", {{"b",""}}), "a,c");
    same("删唯一", edit_labels("only", {{"only",""}}), "");
    same("未命中不改", edit_labels("a,b", {{"zz","y"}}), "a,b");
    same("from==to 不变", edit_labels("a,a,b", {{"a","a"}}), "a,a,b");
    same("from/to 均空", edit_labels("a,b", {{"",""}}), "a,b");
    same("大小写敏感", edit_labels("Cam,cam", {{"cam","x"}}), "Cam,x");
    same("子串不命中", edit_labels("camera,camcorder", {{"cam","x"}}), "camera,camcorder");
    same("变长改名后继续", edit_labels("a,b", {{"a","aaaa"},{"b",""}}), "aaaa");
    same("追加重复不过滤", edit_labels("a", {{"","a"}}), "a,a");
    same("32 字符标签", edit_labels(std::string(32,'z'), {{std::string(32,'z'), "y"}}), "y");

    if (failures == 0) {
        std::printf("Q2 EXTRA ALL PASS\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
