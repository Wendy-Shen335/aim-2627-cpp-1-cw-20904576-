#define main aim_embedded_public_main
#include "../main/A1_b2.cpp"
#undef main

int main() {
    try {
        { int a[] = {-1,2,0,9}; std::size_t n=compact(a,4,0,2); CHECK(n==2 && a[0]==2 && a[1]==0); }
        { CHECK(compact(nullptr,0,0,1)==0); }
    } catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
