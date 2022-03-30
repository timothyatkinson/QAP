#include "quant.h"
#include "time.h"
#include "example.h"

int main (void)
{
  srand(time(NULL));

  function* T = make_function(q_t(), "T", 1);
  function* H = make_function(q_hadamard(), "Ha", 1);
  function* X = make_function(q_pauli_X(), "X", 1);
  function* W = make_function(q_identity(1), "W", 0);
  function* cX = make_function(q_cX(), "cX", 1);
  function* toffoli = make_function(q_toffoli(), "Tof", 0);
  function* Td = make_function(q_td(), "Td", 1);

  int* map = malloc(2 * sizeof(int));
  map[0] = 1;
  map[1] = 0;
  function* Swap = make_function(q_swap(2,map), "Swap", 0);
  free(map);


  int qubits = 3;

  function** f_set = malloc(6 * sizeof(function));
  f_set[0] = H;
  f_set[1] = T;
  f_set[2] = X;
  f_set[3] = W;
  f_set[4] = Swap;
  f_set[5] = Td;

  qap_column* c = make_column(f_set, 6, qubits);
  print_column(c);
  qap_graph* graph = make_graph(c, 15, qubits);
  qap_graph* graph2 = make_graph(c, 15, qubits);

  error_dataset* e = phase_flip_dataset(20);

  q_op* cnot = q_cX();
  q_op* A = q_op_tensor(cX->op, W->op);
  q_op* B = q_op_tensor(Swap->op, W->op);
  q_op* C = q_op_tensor(W->op, cX->op);
  q_op* op1 = q_op_multiply(B, A);
  q_op* op2 = q_op_multiply(C, op1);
  q_op* op3 = q_op_multiply(B, op2);
  //printf("\n\nOP 3\n\n");
  q_op* D = q_op_tensor(W->op, Swap->op);
  q_op* op4 = q_op_multiply(D, B);
  q_op* op5 = q_op_multiply(B, D);
  q_op* op6 = q_op_multiply(toffoli->op, op4);
  q_op* op7 = q_op_multiply(op5, op6);
  q_op* op8 = q_op_multiply(op7, op3);
  q_op_print(op3);
  //printf("\n\nOP 8\n\n");
  q_op_print(op8);
  //printf("MSFE = %lf\n", mean_square_fidelity_error(op3, op8, e));
  e_params* p = malloc(sizeof(e_params));
  p->target_score = 0.98;
  p->ants = 50;
  p->max_runs = 20000;
  p->g1 = graph;
  p->g2 = graph2;
  p->p_min = 0.1;
  p->p_max =  10.0;
  p->l_rate = 1.0;
  p->el_rate = 1.0;
  p->p_diff = 0.1;
  p->p_evap = 0.1;
  p->elite_sel_p = 0.0;
  p->cooperate_bonus = 0.0;


  int n = 100;

  result** qap = malloc(n * sizeof(result*));
  result** r = malloc(n * sizeof(result*));

  //printf("Learning\n");

  for(int i = 0; i < n; i++){
    //printf("Run %d \n", i);
    if(i != 0){
      free_graph(p->g1);
      free_graph(p->g2);
    }
    p->g1 = make_graph(c, 10 * (qubits-1), qubits);
    p->g2 = make_graph(c,  10 * (qubits-1), qubits);
    qap[i] = run_e_qap(p, e);
  }

  //printf("Random\n");

  p->l_rate = 0.0;
  p->el_rate = 0.0;
  p->elite_sel_p = 0.0;

  for(int i = 0; i < n; i++){
    //printf("Run %d \n", i);
    free_graph(p->g1);
    free_graph(p->g2);
    p->g1 = make_graph(c, 10 * (qubits-1), qubits);
    p->g2 = make_graph(c, 10 * (qubits-1), qubits);
    r[i] = run_e_qap(p, e);
  }

  //printf("Learning vs. Random\n");
  for(int i = 0; i < n; i++){
    //printf("%d, %lf, %d, %lf\n", qap[i]->gens, qap[i]->best_score, r[i]->gens, r[i]->best_score);
    free_result(qap[i]);
    free_result(r[i]);
  }

  free(qap);
  free(r);

  free_error_dataset(e);
  free_graph(p->g1);
  free_graph(p->g2);
  free_column(c);
  free(p);
}
