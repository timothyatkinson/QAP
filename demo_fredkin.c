#include "demo_utils.h"

int main() {
    char* fredkin_qap_f  = "./output/fredkin/qap.csv",
        * fredkin_rqap_f = "./output/fredkin/r-qap.csv";
    if (run_demo(fredkin, fredkin_qap_f, fredkin_rqap_f) < 0)
        return -1;
    return 0;
}
