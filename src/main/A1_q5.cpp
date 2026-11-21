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

// Assessment 01 Q5; see 题面.pdf.
namespace aim::q5 {


// BEGIN IMPLEMENTATION
std::vector<std::vector<int>> risk(const std::vector<std::string>& grid) {
    if (grid.empty()) return {};
    const size_t rows = grid.size();
    const size_t cols = grid[0].size();
    if (cols == 0) throw std::invalid_argument("zero columns");
    //完整验证
    for (const std::string& row : grid) {
        if (row.size() != cols) throw std::invalid_argument("ragged row");
        for (char ch : row) {
            if (ch != '.' && ch != '#') throw std::invalid_argument("invalid cell");
        }
    }
    //计算风险
    static const int dr[4] = {-1, 1, 0, 0};
    static const int dc[4] = { 0, 0, -1, 1};
    std::vector<std::vector<int>> result;
    result.reserve(rows);
    for (std::size_t r = 0; r < rows; ++r) {
        std::vector<int> line;
        line.reserve(cols);
        for (std::size_t c = 0; c < cols; ++c) {
            if (grid[r][c] == '#') { line.push_back(-1); continue; }
            int count = 0;
            for (int d = 0; d < 4; ++d) {
                const long long nr = static_cast<long long>(r) + dr[d];
                const long long nc = static_cast<long long>(c) + dc[d];
                if (nr >= 0 && nr < static_cast<long long>(rows) && nc >= 0 && nc < static_cast<long long>(cols) && grid[static_cast<std::size_t>(nr)][static_cast<std::size_t>(nc)] == '#') {
                    count++;
                }
            }
            line.push_back(count);
        }
        result.push_back(std::move(line));
    }
    return result;
// END IMPLEMENTATION
}

#define CHECK(...) do { if (!(__VA_ARGS__)) throw std::runtime_error("check failed"); } while(false)
#define EXPECT_INVALID(...) do { bool caught=false; try { (void)(__VA_ARGS__); } catch(const std::invalid_argument&) { caught=true; } CHECK(caught); } while(false)
#define EXPECT_RANGE(...) do { bool caught=false; try { (void)(__VA_ARGS__); } catch(const std::out_of_range&) { caught=true; } CHECK(caught); } while(false)

using namespace aim::q5;
int main() {
    try {
        { CHECK((risk({".#",".."})==std::vector<std::vector<int>>{{1,-1},{0,1}})); }
        { CHECK(risk({}).empty()); }
        std::cout << "PUBLIC 2/2\n";
    } catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
