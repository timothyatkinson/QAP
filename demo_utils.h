#ifndef QAP_DEMO_UTILS_H
#define QAP_DEMO_UTILS_H
#include "time.h"
#include "quant.h"
#include "example.h"

enum target_gate {
    fredkin,
    margolus,
    toffoli,
};

int run_demo(enum target_gate g, char* qap_output_file, char* rqap_output_file);

#endif
