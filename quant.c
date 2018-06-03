#include "quant.h"

int get_rows(function** function_set, int functions, int qubits);

//Works out how many total occurrences of each function is needed to give maximum coverage. E.g. if there are 4 qubits, we can use at most 1 of each 3-qubit gates but 2 of each 2-qubit gates
int get_rows(function** function_set, int functions, int qubits){
  int count = 0;
  for(int i = 0; i < functions; i++){
    int arity = function_set[i]->qubits;
    int mul = qubits/arity;
    count += (mul * arity);
  }
  return count;
}

int get_functions(function** function_set, int functions, int qubits);

//Works out how many total occurrences of each function is needed to give maximum coverage. E.g. if there are 4 qubits, we can use at most 1 of each 3-qubit gates but 2 of each 2-qubit gates
int get_functions(function** function_set, int functions, int qubits){
  int count = 0;
  for(int i = 0; i < functions; i++){
    int arity = function_set[i]->qubits;
    int mul = qubits/arity;
    count += mul;
  }
  return count;
}

function* make_function(q_op* op, char* name){
  function* f = malloc(sizeof(function));
  f->op = op;
  f->name = name;
  f->qubits = op->qubits;
  return f;
}


void free_function(function* f){
  q_op_free(f->op);
  free(f);
}


qap_column* make_column(function** f_set, int functions, int qubits){
  qap_column* c = malloc(sizeof(qap_column));
  int n = get_rows(f_set, functions, qubits);
  int f = get_functions(f_set, functions, qubits);
  c->f_set = f_set;
  c->functions = functions;
  c->f_index = malloc(n * sizeof(int));
  c->q_index = malloc(n * sizeof(int));
  c->n = n;
  c->f = f;
  c->track_index = malloc(f * sizeof(int**));
  int n_count = 0;
  int c_count = 0;
  for(int i = 0; i < functions; i++){
    int arity = f_set[i]->qubits;
    int mul = qubits/arity;
    for(int j = 0; j < mul; j++){

      c->track_index[c_count] = malloc(2 * sizeof(int*));
      c->track_index[c_count][0] = malloc(sizeof(int));
      c->track_index[c_count][0][0] = i;
      c->track_index[c_count][1] = malloc(sizeof(int) * arity);

      for(int a = 0; a < arity; a++){
        c->track_index[c_count][1][a] = n_count;
        c->f_index[n_count] = i;
        c->q_index[n_count] = a;
        n_count++;
      }
      c_count++;
    }
  }
  return c;
}

void print_column(qap_column* c){
  printf("%d, %d over %d fs\n", c->n, c->f, c->functions);
  printf("\nTrack Index\n");
  for(int i = 0; i < c->f; i++){
    int func = c->track_index[i][0][0];
    printf("%d (%s): ", func, c->f_set[func]->name);
    for(int j = 0; j < c->f_set[func]->qubits; j++){
      printf(" %d,", c->track_index[i][1][j]);
    }
    printf("\n");
  }
  printf("\nVertical Column\n");
  for(int i = 0; i < c->n; i++){
    printf("%d, %d\n", c->f_index[i], c->q_index[i]);
  }
}

void free_column(qap_column* c){

  for(int i = 0; i < c->functions; i++){
    free_function(c->f_set[i]);
  }
  free(c->f_set);
  for(int i = 0; i < c->f; i++){
    free(c->track_index[i][0]);
    free(c->track_index[i][1]);
    free(c->track_index[i]);
  }
  free(c->track_index);

  free(c->f_index);
  free(c->q_index);

  free(c);
}

qap_graph* make_graph(qap_column* c, int columns, int qubits){
  qap_graph* g = malloc(sizeof(qap_graph));
  g->column = c;
  g->columns = columns;
  g->qubits = qubits;
  g->ant_maps = malloc(qubits * sizeof(ant_map*));
  for(int i = 0; i < qubits; i++){
    g->ant_maps[i] = make_ant_map(c->n, columns);
  }
  return g;
}

void free_graph(qap_graph* g){
  for(int i = 0; i < g->qubits; i++){
    free_ant_map(g->ant_maps[i]);
  }
  free(g->ant_maps);
  free(g);
}

ant_map* make_ant_map(int n, int columns){
  ant_map* m = malloc(sizeof(ant_map));
  m->n = n;
  m->columns = columns;
  m->pheremone_map = malloc(columns * sizeof(double**));
  m->pheremone_map[0] = malloc(sizeof(double*));
  m->pheremone_map[0][0] = malloc(n * sizeof(double));
  for(int i = 0; i < n; i++){
      m->pheremone_map[0][0][i] = 0.0;
  }
  for(int i = 1; i < columns; i++){
    m->pheremone_map[i] = malloc(n * sizeof(double*));
    for(int j = 0; j < n; j++){
      m->pheremone_map[i][j] = malloc(n * sizeof(double));
      for(int k = 0; k < n; k++){
        m->pheremone_map[i][j][k] = 0.0;
      }
    }
  }
  return m;
}

void free_ant_map(ant_map* m){
  free(m->pheremone_map[0][0]);
  free(m->pheremone_map[0]);


  for(int i = 1; i < m->columns; i++){
    for(int k = 0; k < m->n; k++){
      free(m->pheremone_map[i][k]);
    }
    free(m->pheremone_map[i]);
  }

  free(m->pheremone_map);

  free(m);
}

void add_route(qap_graph* g, int** route, double fitness, double diffusion){
  for(int i = 0; i < g->qubits; i++){
    //1st case
    ant_map* m = g->ant_maps[i];
    m->pheremone_map[0][0][route[i][0]] += fitness;
    int current = route[i][0];
    for(int j = 1; j < m->columns; j++){
      m->pheremone_map[j][current][route[i][j]] += fitness;
      for(int k = 0; k < m->n; k++){
        if(k != current){
          m->pheremone_map[j][k][route[i][j]] += fitness * diffusion;
        }
      }
      current = route[i][j];
    }
  }
}

void update_pheremone(qap_graph* g, double min, double max, double evaporation){
  for(int i = 0; i < g->qubits; i++){
    //1st case
    ant_map* m = g->ant_maps[i];
    int n = m->n;
    for(int j = 0; j < n; j++){
      m->pheremone_map[0][0][j] = m->pheremone_map[0][0][j] * (1.0 - evaporation);
      if(m->pheremone_map[0][0][j] > max){
        m->pheremone_map[0][0][j] = max;
      }
      else if(m->pheremone_map[0][0][j] < min){
        m->pheremone_map[0][0][j] = min;
      }
    }
    for(int j = 1; j < m->columns; j++){
      for(int k = 0; k < n; k++){
        for(int l = 0; l < n; l++){
          m->pheremone_map[j][k][l] = m->pheremone_map[j][k][l] * (1.0 - evaporation);
          if(m->pheremone_map[j][k][l] > max){
            m->pheremone_map[j][k][l] = max;
          }
          else if(m->pheremone_map[j][k][l] < min){
            m->pheremone_map[j][k][l] = min;
          }
        }
      }
    }
  }
}

int pick_from(double* weights, int* blocked, int n){
  double total_weight = 0.0;
  for(int i = 0; i < n; i++){
    if(blocked[i] == 0){
      total_weight += weights[i];
    }
  }
  double in = rand_double() * total_weight;
  total_weight = 0.0;
  for(int i = 0; i < n; i++){
    if(blocked[i] == 0){
      total_weight += weights[i];
    }
    if(in <= total_weight){
      return i;
    }
  }
  printf("Route selection failed\n");
  exit(0);
}

int* generate_route(ant_map* m, int** blocked){
  int* route = malloc(m->columns * sizeof(int));
  route[0] = pick_from(m->pheremone_map[0][0], blocked[0], m->n);
  int current = route[0];
  blocked[0][current] = 1;
  for(int i = 1; i < m->columns; i++){
    route[i] = pick_from(m->pheremone_map[i][current], blocked[i], m->n);
    current = route[i];
    blocked[i][current] = 1;
  }
  return route;
}

int** generate_routes(qap_graph* g){
  int** blocked = malloc(g->columns * sizeof(int*));
  for(int i = 0; i < g->columns; i++){
    blocked[i] = malloc(g->column->n * sizeof(int));
    for(int j = 0; j < g->column->n; j++){
      blocked[i][j] = 0;
    }
  }
  int** routes = malloc(g->qubits * sizeof(int*));
  for(int i = 0; i < g->qubits; i++){
    routes[i] = generate_route(g->ant_maps[i], blocked);
  }
}

void free_routes(int** routes, int qubits){
  for(int i = 0; i < qubits; i++){
    free(routes[i]);
  }
  free(routes);
}

q_op* make_q_op(qap_graph* g, int** routes){
  int* maps_from = malloc(g->qubits * sizeof(int));
  for(int i = 0; i < g->qubits; i++){
    maps_from[i] = i;
  }
  int* maps_to = malloc(g->qubits * sizeof(int));
  for(int i = 0; i < g->columns; i++){

    for(int j = 0; j < g->column->f; j++){
      int** track_function = g->column->track_index[f];
      function f = g->f_set[track_function[0][0]];
      
    }
  }

}
