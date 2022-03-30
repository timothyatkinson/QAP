#include "demo_utils.h"

int run_demo(demo_params ps) {
    srand(time(NULL));

    // Prepare function set.
    int no_of_functions = 13;
    int* map = malloc(2 * sizeof(int));
    map[0] = 1;
    map[1] = 0;
    dataset* d;
    function** f_set = malloc(no_of_functions * sizeof(function));
    f_set[0]  = make_function(q_identity(1), "Wire (I)", 0);
    f_set[1]  = make_function(q_hadamard(), "H", 0);
    f_set[2]  = make_function(q_pauli_X(), "X", 0);
    f_set[3]  = make_function(q_pauli_Y(), "Y", 0);
    f_set[4]  = make_function(q_pauli_Z(), "Z", 0);
    f_set[5]  = make_function(q_s(), "Phase (S)", 0);
    f_set[6]  = make_function(q_v(), "V", 0);
    f_set[7]  = make_function(q_cX(), "Feynman (CNot)", 0);
    f_set[8]  = make_function(q_swap(2, map), "Swap", 0);
    f_set[9]  = make_function(q_cv(), "Controlled-V", 0);
    f_set[10] = make_function(q_cvd(), "Controlled-V-Hermitian (C-V-dagger)", 0);
    switch (ps.gate) {
        case fredkin:
            d = fredkin_dataset(20);
            f_set[11] = make_function(q_toffoli(), "Toffoli", 0);
            f_set[12] = make_function(q_margolus(), "Margolus", 0);
            break;
        case margolus:
            d = margolus_dataset(20);
            f_set[11] = make_function(q_fredkin(), "Fredkin", 0);
            f_set[12] = make_function(q_toffoli(), "Toffoli", 0);
            break;
        case toffoli:
            d = toffoli_dataset(20);
            f_set[11] = make_function(q_fredkin(), "Fredkin", 0);
            f_set[12] = make_function(q_margolus(), "Margolus", 0);
            break;
        default:
            return -1;
    }
    free(map);

    // Set up QAP parameters.
    int qubits = 3,
        columns = 10 * (qubits - 1);
    qap_column* c = make_column(f_set, no_of_functions, qubits);
    qap_graph* graph = make_graph(c, columns, qubits);
    params* p = malloc(sizeof(params));
    p->target_score = 0.98;
    p->ants = 50;
    p->max_runs = 100000;
    p->g = graph;
    p->p_min = 0.1;
    p->p_max = 10.0;
    p->l_rate = 1.0;
    p->el_rate = 1.0;
    p->p_diff = 0.1;
    p->p_evap = 0.1;
    p->elite_sel_p = 0.0;
    p->cooperate_bonus = 0.0;
    int n = 5;
    result** qap = malloc(n * sizeof(result*));
    result** r = malloc(n * sizeof(result*));
    clock_t start_time, end_time;
    double execution_time;

    // QAP Training.
    printf("QAP in progress...\n");
    start_time = clock();
    for (int i = 0; i < n; i++){
        if (i != 0)
            free_graph(p->g);
        p->g = make_graph(c, columns, qubits);
        qap[i] = run_qap(p, d);
    }
    export_results(qap, n, ps.qap_output_file);
    end_time = clock();
    execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("QAP finished in %lfs!\n\n", execution_time);

    // R-QAP Training.
    printf("R-QAP in progress...\n");
    start_time = clock();
    p->l_rate = 0.0;
    p->el_rate = 0.0;
    p->elite_sel_p = 0.0;
    for (int i = 0; i < n; i++) {
        free_graph(p->g);
        p->g = make_graph(c, columns, qubits);
        r[i] = run_qap(p, d);
    }
    export_results(r, n, ps.rqap_output_file);
    end_time = clock();
    execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("R-QAP finished in %lfs!\n", execution_time);

    free(qap);
    free(r);
    free_dataset(d);
    free_graph(p->g);
    free_column(c);
    free(p);

    return 0;
}
