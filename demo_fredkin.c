#include "time.h"
#include "quant.h"
#include "example.h"

int main() {
    srand(time(NULL));

    // Prepare function set.
    int* map = malloc(2 * sizeof(int));
    map[0] = 1;
    map[1] = 0;
    function** f_set = malloc(5 * sizeof(function));
    f_set[0] = make_function(q_hadamard(), "H", 1);
    f_set[1] = make_function(q_pauli_X(), "X", 1);
    f_set[2] = make_function(q_t(), "T", 1);
    f_set[3] = make_function(q_td(), "Td", 1);
    f_set[4] = make_function(q_swap(2,map), "Swap", 0);
    free(map);

    // Set up QAP parameters.
    int qubits = 3,
        columns = 10 * (qubits - 1);
    qap_column* c = make_column(f_set, 5, qubits);
    qap_graph* graph = make_graph(c, columns, qubits);
    dataset* d = fredkin_dataset(20);
    params* p = malloc(sizeof(params));
    p->target_score = 1.0;
    p->ants = 50;
    p->max_runs = 20000;
    p->g = graph;
    p->p_min = 0.1;
    p->p_max = 10.0;
    p->l_rate = 1.0;
    p->el_rate = 1.0;
    p->p_diff = 0.1;
    p->p_evap = 0.1;
    p->elite_sel_p = 0.0;
    p->cooperate_bonus = 0.0;
    int n = 100;
    result** qap = malloc(n * sizeof(result*));
    result** r = malloc(n * sizeof(result*));

    // QAP Training.
    printf("Learning\n");
    for (int i = 0; i < n; i++){
        if (i != 0)
            free_graph(p->g);
        p->g = make_graph(c, columns, qubits);
        qap[i] = run_qap(p, d);
    }

    // R-QAP Training.
    printf("Random\n");
    p->l_rate = 0.0;
    p->el_rate = 0.0;
    p->elite_sel_p = 0.0;
    for (int i = 0; i < n; i++) {
        free_graph(p->g);
        p->g = make_graph(c, columns, qubits);
        r[i] = run_qap(p, d);
    }

    // Print statistics of QAP and R-QAP.
    printf("Learning vs. Random\n");
    for(int i = 0; i < n; i++){
        printf("%d, %lf, %d, %lf\n", qap[i]->gens, qap[i]->best_score, r[i]->gens, r[i]->best_score);
        free_result(qap[i]);
        free_result(r[i]);
    }

    free(qap);
    free(r);
    free_dataset(d);
    free_graph(p->g);
    free_column(c);
    free(p);
    return 0;
}
