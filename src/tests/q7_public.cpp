#define main aim_embedded_public_main
#include "../main/A1_q7.cpp"
#undef main

int main() {
    try {
        { aim::q7::FireResult r=simulate(2,90,10,20,{1,1,1}); CHECK(r.shots==2 && r.rejected==1 && r.ammo==0 && r.heat==100); }
        { aim::q7::FireResult r=simulate(3,100,20,10,{0}); CHECK(r.heat==80 && r.rejected==0 && r.ammo==3); }
    } catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
