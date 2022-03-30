#include "demo_utils.h"

int main() {
    demo_params margolus_ps = {margolus, "./output/margolus/qap.csv", "./output/margolus/r-qap.csv"};
    if (run_demo(margolus_ps) < 0)
        return -1;
    return 0;
}
