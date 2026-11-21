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

// Assessment 01 B1; see 题面.pdf.
namespace aim::b1 {


// BEGIN IMPLEMENTATION
bool parse_i32(const std::string& text, int& output) {
    if (text.empty() || text.size() > 1000) return false;
    std::size_t i = 0;
    bool negative = false;
    if (text[i] == '+' || text[i] == '-') {
        negative = (text[i] == '-');
        i++;
    }
    // 只有符号，没有数字
    if (i == text.size()) return false;
    const long long limit = negative ? 2147483648LL : 2147483647LL;
    long long value = 0;
    for (; i < text.size(); ++i) {
        const char ch = text[i];
        // 非 ASCII 数字
        if (ch < '0' || ch > '9') return false;
        const int digit = ch - '0';
        // 溢出前拦截
        if (value > (limit - digit) / 10) return false;
        value = value * 10 + digit;
    }
    output = static_cast<int>(negative ? -value : value);
    return true;
}
// END IMPLEMENTATION
}

#define CHECK(...) do { if (!(__VA_ARGS__)) throw std::runtime_error("check failed"); } while(false)
#define EXPECT_INVALID(...) do { bool caught=false; try { (void)(__VA_ARGS__); } catch(const std::invalid_argument&) { caught=true; } CHECK(caught); } while(false)
#define EXPECT_RANGE(...) do { bool caught=false; try { (void)(__VA_ARGS__); } catch(const std::out_of_range&) { caught=true; } CHECK(caught); } while(false)

using namespace aim::b1;
int main() {
    try {
        { int x=9; CHECK(parse_i32("-12",x) && x==-12); }
        { int x=9; CHECK(!parse_i32("12x",x) && x==9); }
        std::cout << "PUBLIC 2/2\n";
    } catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
