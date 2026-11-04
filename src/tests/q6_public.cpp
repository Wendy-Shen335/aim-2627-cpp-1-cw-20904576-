#define main aim_embedded_public_main
#include "../main/A1_q6.cpp"
#undef main

int main() {
    try {
        { CHECK(select_target({{7,3,4,20,true},{2,0,5,20,true}},5)==2); }
        { CHECK(select_target({},5)==-1); }
    } catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
