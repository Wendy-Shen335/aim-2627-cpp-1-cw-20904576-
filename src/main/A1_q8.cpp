#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// Assessment 01 Q8; see 题面.pdf.
namespace aim::q8 {
struct Telemetry { int id; int hp; };
struct Summary { int valid; int duplicate; int invalid; long long total_hp; std::vector<int> ids; };

// BEGIN IMPLEMENTATION
Summary summarize(const std::vector<Telemetry>& frames) {
    Summary out{0, 0, 0, 0, {}};
    std::vector<bool> seen(10000, false);
    for (const Telemetry& frame : frames) {
        // 范围验证（非法记录不占编号）
        if (frame.id < 0 || frame.id > 9999 ||
            frame.hp < -1000 || frame.hp > 1000) {
            out.invalid++;
            continue;
        }
        // 查重
        if (seen[static_cast<std::size_t>(frame.id)]) {
            out.duplicate++;
            continue;
        }
        // 首次合法：标记 + 统计 + 夹取 + 存编号
        seen[static_cast<std::size_t>(frame.id)] = true;
        out.valid++;
        int hp = frame.hp;
        if (hp < 0) hp = 0;
        if (hp > 100) hp = 100;
        out.total_hp += hp;
        out.ids.push_back(frame.id);
    }
    std::sort(out.ids.begin(), out.ids.end());
    return out;
}
// END IMPLEMENTATION
}

#define CHECK(...) do { if (!(__VA_ARGS__)) throw std::runtime_error("check failed"); } while(false)
#define EXPECT_INVALID(...) do { bool caught=false; try { (void)(__VA_ARGS__); } catch(const std::invalid_argument&) { caught=true; } CHECK(caught); } while(false)
#define EXPECT_RANGE(...) do { bool caught=false; try { (void)(__VA_ARGS__); } catch(const std::out_of_range&) { caught=true; } CHECK(caught); } while(false)

using namespace aim::q8;
int main() {
    try {
        { aim::q8::Summary r=summarize({{2,200},{1,-5},{2,10},{-1,30}}); CHECK(r.valid==2 && r.duplicate==1 && r.invalid==1 && r.total_hp==100 && (r.ids==std::vector<int>{1,2})); }
        { aim::q8::Summary r=summarize({{1,-1001},{1,30},{1,1001}}); CHECK(r.valid==1 && r.duplicate==0 && r.invalid==2 && r.total_hp==30 && (r.ids==std::vector<int>{1})); }
        std::cout << "PUBLIC 2/2\n";
    } catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
