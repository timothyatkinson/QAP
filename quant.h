#ifndef QUANT_H
#define QUANT_H

#include "QuantumCircuits-C/predefined_q.h"
#include "QuantumCircuits-C/q_circuit.h"
#include <string.h>

typedef struct function{
  q_op* op;
  q_op* control_op;
  int qubits;
  char* name;
  int controllable;
} function;

typedef struct qap_column{
  function** f_set;
  int functions;
  int* f_index;
  int*** track_index;
  int* q_index;
  int n;
  int f;
} qap_column;

typedef struct ant_map{
  double*** pheremone_map;
  int n;
  int columns;
} ant_map;

typedef struct qap_graph{
  qap_column* column;
  ant_map** ant_maps;
  int qubits;
  int columns;
} qap_graph;

typedef struct dataset{
  int entries;
  q_state** X;
  q_state** Y;
} dataset;

typedef struct params{
  double target_score;
  int ants;
  int max_runs;
  qap_graph* g;

  double p_min;
  double p_max;
  double l_rate;
  double el_rate;
  double p_evap;
  double p_diff;
  double elite_sel_p;
  double cooperate_bonus;
} params;

q_op* make_controllable(q_op* op);

q_op* one();

function* make_function(q_op* op, char* name, int controllable);
void free_function(function* f);
void free_fset(function** fset, int count);

qap_column* make_column(function** f_set, int functions, int qubits);
void print_column(qap_column* c);
void free_column(qap_column* c);

qap_graph* make_graph(qap_column* c, int columns, int qubits);
void free_graph(qap_graph* g);

ant_map* make_ant_map(int n, int columns);
void free_ant_map(ant_map* m);

void add_route(qap_graph* g, int** route, double fitness, double learning_rate, double diffusion, double p_max);
void update_pheremone(qap_graph* g, double min, double max, double evaporation);

void reset_pheremone(qap_graph* g, double min);

int pick_from(double* weights, int* blocked, int* cooperate, double cooperate_bonus, int n);

int* generate_route(qap_column* column, ant_map* m, int** blocked, int** cooperate, double cooperate_bonus, double elite_sel_p, int* elite_route);

int** generate_routes(qap_graph* g, double elite_sel_p, double cooperate_bonus, int** elite_routes);

void free_routes(int** routes, int qubits);

q_op* make_q_op(qap_graph* g, int** routes);
void print_op(qap_graph* g, int** routes);
int count_op(qap_graph* g, int** routes);
double mean_square_fidelity(q_op* op, dataset* dataset);

dataset* make_dataset(int entries, q_state** X, q_state** Y);
void free_dataset(dataset* d);

int run_qap(params* p, dataset* d);
#endif
