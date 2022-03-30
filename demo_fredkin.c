#include "demo_utils.h"

int main() {
    demo_params fredkin_ps = {fredkin, "./output/fredkin/qap.csv", "./output/fredkin/r-qap.csv"};
    if (run_demo(fredkin_ps) < 0)
        return -1;
    return 0;
}
