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

// Assessment 01 Q6; see 题面.pdf.
namespace aim::q6 {
struct Target { int id; int x; int y; int hp; bool visible; };

// BEGIN IMPLEMENTATION
int select_target(const std::vector<Target>& targets, int max_range) {
    if (max_range < 0) return -1;
    const long long limit2 = static_cast<long long>(max_range) * max_range;
    bool found = false;
    int best_id = 0;
    int best_hp = 0;
    long long best_dist2 = 0;
    for (const Target& t : targets) {
    //筛选：可见/有血/在射程内
        if (!t.visible || t.hp <= 0) continue;
        const long long dx = static_cast<long long>(t.x);
        const long long dy = static_cast<long long>(t.y);
        const long long dist2 = dx * dx + dy * dy;
        if (dist2 > limit2) continue;
        if (!found || t.hp < best_hp || (t.hp == best_hp && (dist2 < best_dist2 || (dist2 == best_dist2 && t.id < best_id)))) {
            found = true;
            best_id = t.id;
            best_hp = t.hp;
            best_dist2 = dist2;
        }
    }
    return found ? best_id : -1;
}
// END IMPLEMENTATION
}

#define CHECK(...) do { if (!(__VA_ARGS__)) throw std::runtime_error("check failed"); } while(false)
#define EXPECT_INVALID(...) do { bool caught=false; try { (void)(__VA_ARGS__); } catch(const std::invalid_argument&) { caught=true; } CHECK(caught); } while(false)
#define EXPECT_RANGE(...) do { bool caught=false; try { (void)(__VA_ARGS__); } catch(const std::out_of_range&) { caught=true; } CHECK(caught); } while(false)

using namespace aim::q6;
int main() {
    try {
        { CHECK(select_target({{7,3,4,20,true},{2,0,5,20,true}},5)==2); }
        { CHECK(select_target({},5)==-1); }
        std::cout << "PUBLIC 2/2\n";
    } catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
