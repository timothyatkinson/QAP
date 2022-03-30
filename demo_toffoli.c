#include "demo_utils.h"

int main() {
    demo_params toffoli_ps = {toffoli, "./output/toffoli/qap.csv", "./output/toffoli/r-qap.csv"};
    if (run_demo(toffoli_ps) < 0)
        return -1;
    return 0;
}
