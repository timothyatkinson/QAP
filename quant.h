#ifndef QUANT_H
#define QUANT_H

#include "QuantumCircuits-C/predefined_q.h"
#include "QuantumCircuits-C/q_circuit.h"

typedef struct function{
  q_op* op;
  int qubits;
  char* name;
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
} dataset;

function* make_function(q_op* op, char* name);
void free_function(function* f);

qap_column* make_column(function** f_set, int functions, int qubits);
void print_column(qap_column* c);
void free_column(qap_column* c);

qap_graph* make_graph(qap_column* c, int columns, int qubits);
void free_graph(qap_graph* g);

ant_map* make_ant_map(int n, int columns);
void free_ant_map(ant_map* m);

void add_route(qap_graph* g, int** route, double fitness, double diffusion);
void update_pheremone(qap_graph* g, double min, double max, double evaporation);



int pick_from(double* weights, int* blocked, int n);

int* generate_route(ant_map* m, int** blocked);

int** generate_routes(qap_graph* g);

void free_routes(int** routes, int qubits);

q_op* make_q_op(qap_graph* g, int** routes);

#endif
