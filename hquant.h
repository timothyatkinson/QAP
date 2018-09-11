#ifndef HQUANT_H
#define HQUANT_H

#include "QuantumCircuits-C/predefined_q.h"
#include "QuantumCircuits-C/q_circuit.h"
#include <string.h>
#include <stdio.h>

typedef struct qap_oper {
  q_op* op;
  char* name;
} qap_oper;

typedef struct qap_column {
  qap_oper** opers;
  int oper_count;
} qap_column;

typedef struct qap_graph {
  qap_column* column;
  int columns;
  double*** ant_map;
} qap_graph;

typedef struct function {
  q_op* op;
  int qubits;
  char* name;
} function;

typedef struct dataset{
  int entries;
  q_state** X;
  q_state** Y;
} dataset;

typedef struct params {
  qap_graph* graph;
  dataset* dataset;
  double target_score;
  int ants;
  int max_iter;
  double p_init;
  double p_min;
  double p_max;
  double l_rate;
  double evap_rate;
  double diffusion;
} params;

typedef struct result{
  int iter;
  int* route;
  q_op* route_op;
  double route_score;
} result;

qap_oper* make_oper(function** funcs, int func_count);
void free_oper(qap_oper* oper);

qap_column* make_column(qap_oper** opers, int oper_count);
void free_column(qap_column* column);
qap_column* generate_column(function** funcs, int func_count, int qubits);
void print_column(qap_column* column);

qap_graph* make_graph(qap_column* column, int columns, double min_p);
void free_graph(qap_graph* graph);
void print_graph(qap_graph* graph);
void print_ant_map(qap_graph* graph);

function* make_function(q_op* op, int qubits, char* name);
void free_function(function* func);
function** core_fset();
function** rot_fset();
void free_fset(function** fset, int f_count);

int* generate_route(qap_graph* graph);
int routes_equal(int* a, int* b, int route_length);
void lay_pheremone(qap_graph* graph, int* route, double pheremone, double learning_rate, double diffusion_rate);
void update_pheremone(qap_graph* graph, double p_min, double p_max, double evap_rate);
void reset_pheremone(qap_graph* graph, double p_init);
q_op* route_to_op(int* route, qap_graph* graph);

double mean_square_fidelity(q_op* op, dataset* dataset);

dataset* make_dataset(int entries, q_state** X, q_state** Y);
void free_dataset(dataset* d);

params* make_params(qap_graph* graph, dataset* dataset, double target_score, int ants, int max_iter, double p_init, double p_min, double p_max, double l_rate, double evap_rate, double diffusion);
void free_params(params* par);
params* default_params(qap_graph* graph, dataset* dataset);

result* run_qap(params* params);
function** ccore_fset();
#endif
