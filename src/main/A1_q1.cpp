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

// Assessment 01 Q1; see 题面.pdf.
namespace aim::q1 {


// BEGIN IMPLEMENTATION
// 获得HP的百分比
long long ratio(long long hp, long long max_hp) { 
    if ( max_hp <= 0 ) return -1;
    if ( hp < 0 ) return 0;
    if ( hp > max_hp ) return 100;
    return (hp * 100) / max_hp;
}
// 生成报告
std::string report(int id, long long hp, long long max_hp, int battery) { 
    std::string text = "ID=" + std::to_string(id);
    long long HP = ratio(hp,max_hp);
    if (HP == -1) {
        text += ";HP=INVALID";
    } else {
        text += ";HP=" + std::to_string(HP);
    }
    int BAT;
    if (battery < 0){
        BAT = 0;
    }
    else if (battery > 100){
        BAT = 100;
    }
    else {
        BAT = battery;
    }
    text += ";BAT=" + std::to_string(BAT);
    if (BAT < 20){
        text += ";STATE=LOW";
    }
    else if (BAT < 60){
        text += ";STATE=WARNING";
    }
    else {
        text += ";STATE=OK";
    }
    return text;
 }
// END IMPLEMENTATION
}

#define CHECK(...) do { if (!(__VA_ARGS__)) throw std::runtime_error("check failed"); } while(false)
#define EXPECT_INVALID(...) do { bool caught=false; try { (void)(__VA_ARGS__); } catch(const std::invalid_argument&) { caught=true; } CHECK(caught); } while(false)
#define EXPECT_RANGE(...) do { bool caught=false; try { (void)(__VA_ARGS__); } catch(const std::out_of_range&) { caught=true; } CHECK(caught); } while(false)

using namespace aim::q1;
int main() {
    try {
        { CHECK(ratio(2,3)==66); }
        { CHECK(report(7,2,3,20)=="ID=7;HP=66;BAT=20;STATE=WARNING"); }
        std::cout << "PUBLIC 2/2\n";
    } catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
