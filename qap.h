#ifndef QAP_H
#define QAP_H

typedef struct params{
  int ants;
  dataset* train_dataset;
  dataset* test_dataset;
  double p_init;
  double p_min;
  double p_max;
  double evap_rate;
  double elitist_rate;
  int depth;
  int max_steps;
  int target_size;
  double target_score;
  bool size_matters;
  double size_ratio;
  op** function_set;
  int functions;
}

typedef struct dataset{
  q_state** in;
  q_state** out;
  int entries;
} dataset;

typedef struct op{
  q_op* qop;
  char* name;
} op;

typedef struct q_circuit{
  op** circuit;
} q_circuit;

typedef struct qap_result{
  int evaluations;
  double winning_score;
  int winning_size;
  q_circuit* winning_circuit;
} qap_result;

typedef struct qap_node{
  op* op;
  int index;
  qap_connection*** in_connections;
  qap_connection*** out_connections;
} qap_node;

typedef struct qap_connection{
  qap_node* source;
  int source_conn_index;
  qap_node* target;
  int target_conn_index;
  double pheromone;
  int blocked;
} qap_connection;

typedef struct qap_graph{
  qap_node*** layers;
  qap_node** nodes;
  qap_connection** connections;
} qap_graph;

typedef struct qap_route{
  qap_connection** route;
}

qap_result* quantum_ant_programming(params* ant_params);

#endif
