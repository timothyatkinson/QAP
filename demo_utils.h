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

typedef struct demo_params {
    enum target_gate gate;
    char* qap_output_file;
    char* rqap_output_file;
} demo_params;

int run_demo(demo_params ps);

#endif
