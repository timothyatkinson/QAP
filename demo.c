#include "quant.h"
#include "time.h"
#include "example.h"

int main (void)
{
  srand(time(NULL));

  int rc = 1;
  function* S = make_function(q_s(), "S", 0);
  function* T = make_function(q_t(), "T", 1);
  function* H = make_function(q_hadamard(), "Ha", 1);
  function* X = make_function(q_pauli_X(), "X", 1);
  function* Y = make_function(q_pauli_Y(), "Y", 1);
  function* Z = make_function(q_pauli_Z(), "Z", 1);
  function* cX = make_function(q_cX(), "cX", 0);
  function* cT = make_function(q_ct(), "cT", 0);
  function* r2 = make_function(q_rot_z(1.0/2.0), "r1/2", rc);
  function* r4 = make_function(q_rot_z(1.0/4.0), "r1/4", rc);
  function* r8 = make_function(q_rot_z(1.0/8.0), "r1/8", rc);
  function* r16 = make_function(q_rot_z(1.0/16.0), "r1/16", rc);
  function* r32 = make_function(q_rot_z(1.0/32.0), "r1/32", rc);
  function* r64 = make_function(q_rot_z(1.0/64.0), "r1/64", rc);
  function* rx4 = make_function(r_x(M_PI/4.0), "rx1/4", rc);
  function* rx8 = make_function(r_x(M_PI/8.0), "rx1/8", rc);
  function* rx16 = make_function(r_x(M_PI/16.0), "rx1/16", rc);
  function* rx32 = make_function(r_x(M_PI/32.0), "rx1/32", rc);
  function* rx64 = make_function(r_x(M_PI/64.0), "rx1/64", rc);
  function* rx128 = make_function(r_x(M_PI/128.0), "rx1/128", rc);
  function* ry4 = make_function(r_y(M_PI/4.0), "ry1/4", rc);
  function* ry8 = make_function(r_y(M_PI/8.0), "ry1/8", rc);
  function* ry16 = make_function(r_y(M_PI/16.0), "ry1/16", rc);
  function* ry32 = make_function(r_y(M_PI/32.0), "ry1/32", rc);
  function* ry64 = make_function(r_y(M_PI/64.0), "ry1/64", rc);
  function* ry128 = make_function(r_y(M_PI/128.0), "ry1/128", rc);
  function* rz4 = make_function(r_z(M_PI/4.0), "rz1/4", rc);
  function* rz8 = make_function(r_z(M_PI/8.0), "rz1/8", rc);
  function* rz16 = make_function(r_z(M_PI/16.0), "rz1/16", rc);
  function* rz32 = make_function(r_z(M_PI/32.0), "rz1/32", rc);
  function* rz64 = make_function(r_z(M_PI/64.0), "rz1/64", rc);
  function* rz128 = make_function(r_z(M_PI/128.0), "rz1/128", rc);
  function* W = make_function(q_identity(1), "W", 0);
  function* Td = make_function(q_td(), "Td", 1);

  int* map = malloc(2 * sizeof(int));
  map[0] = 1;
  map[1] = 0;
  function* Swap = make_function(q_swap(2,map), "Swap", 0);
  free(map);

  function* f1 = make_function(q_s(), "S", 1);
  function* f2 = make_function(q_t(), "T", 1);
  function* f3 = make_function(q_hadamard(), "Ha", 0);
  function* f4 = make_function(q_cX(), "cX", 0);
  function* f5 = make_function(q_identity(1), "W", 0);
  function* f6 = make_function(q_pauli_X(), "X", 0);
  function* f7 = make_function(q_pauli_Y(), "Y", 0);
  function* f8 = make_function(q_pauli_Z(), "Z", 0);
  function* f9 = make_function(q_rot_z(1.0/4.0), "r1/2", 0);
  function* f10 = make_function(q_rot_z(1.0/8.0), "r1/4", 0);
  function* f11 = make_function(q_rot_z(1.0/16.0), "r1/8", 0);
  function* f12 = make_function(q_crot_z(1.0/4.0), "cr1/2", 0);
  function* f13 = make_function(q_crot_z(1.0/8.0), "cr1/4", 0);
  function* f14 = make_function(q_crot_z(1.0/16.0), "cr1/8", 0);
  function* f26 = make_function(q_crot_z(1.0/32.0), "cr1/16", 0);
  function* f27 = make_function(q_crot_z(1.0/32.0), "r1/16", 0);
  function* f15 = make_function(r_x(M_PI / 4.0), "rx/4", 0);
  function* f16 = make_function(r_x(M_PI / 8.0), "rx/8", 0);
  function* f17 = make_function(r_y(M_PI / 4.0), "ry/4", 0);
  function* f18 = make_function(r_y(M_PI / 8.0), "ry/8", 0);
  function* f19 = make_function(r_z(M_PI / 4.0), "rz/4", 0);
  function* f20 = make_function(r_z(M_PI / 8.0), "rz/8", 0);
  function* f21 = make_function(r_x(M_PI / 32.0), "rx/32", 0);
  function* f22 = make_function(r_y(M_PI / 32.0), "ry/32", 0);
  function* f23 = make_function(r_z(M_PI / 64.0), "rz/32", 0);
  function* f24 = make_function(q_identity(1), "W", 0);
  function* f25 = make_function(q_identity(1), "W", 0);


  int qubits = 3;

  function** f_set = malloc(22 * sizeof(function));
  f_set[0] = H;
  f_set[1] = T;
  f_set[2] = X;
  f_set[3] = W;
  f_set[4] = Swap;
  f_set[5] = Td;
  f_set[6] = Y;
  f_set[7] = r64;
  f_set[8] = Swap;
  f_set[9] = W;
  f_set[10] = ry8;
  f_set[11] = ry16;
  f_set[12] = ry32;
  f_set[13] = ry64;
  f_set[14] = ry128;
  f_set[15] = rz4;
  f_set[16] = rz8;
  f_set[17] = rz16;
  f_set[18] = rz32;
  f_set[19] = rz64;
  f_set[20] = rz128;
  f_set[21] = T;

  qap_column* c = make_column(f_set, 6, qubits);
  print_column(c);
  //q_op_print(f3->control_op);
  //printf("%s\n", f->name);

  qap_graph* graph = make_graph(c, 15, qubits);

  //dataset* d = qft_dataset(qubits, 20);
  dataset* d = gdo_dataset(qubits, 20);
  //error_dataset* e = bit_flip_dataset(20);
  //free_error_dataset(e);
  //dataset* d = epr_pair_dataset();
  //dataset* d = op_dataset(qubits, q_toffoli(), 100);
  params* p = malloc(sizeof(params));
  p->target_score = 0.98;
  p->ants = 50;
  p->max_runs = 20000;
  p->g = graph;
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
  int columns = 20 * (qubits - 2);
  if (columns == 0){
	columns = 10;
  }
  printf("Learning\n");

  for(int i = 0; i < n; i++){
    printf("Run %d \n", i);
    if(i != 0){
      free_graph(p->g);
    }
    p->g = make_graph(c, 10 * (qubits - 1), qubits);
    qap[i] = run_qap(p, d);
  }

  printf("Random\n");

  p->l_rate = 0.0;
  p->el_rate = 0.0;
  p->elite_sel_p = 0.0;

  for(int i = 0; i < n; i++){
    printf("Run %d \n", i);
    free_graph(p->g);
    p->g = make_graph(c, 10 * (qubits - 1), qubits);
    r[i] = run_qap(p, d);
  }

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
}
