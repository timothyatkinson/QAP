#include "hquant.h"

q_op* one();
q_op* one(){
  q_op* o = q_op_alloc(0);
  gsl_matrix_complex_set(o->matrix, 0, 0, GSL_COMPLEX_ONE);
  return o;
}

q_op* make_controllable(q_op* op);
q_op* make_controllable(q_op* op){
  q_op* new_op = q_op_calloc(op->qubits + 1);
  int m = pow(2, op->qubits + 1);
  for(int i = 0; i < m; i++){
    if(i < (m / 2)){
      gsl_matrix_complex_set(new_op->matrix, i, i, GSL_COMPLEX_ONE);
    }
    else{
      for(int j = m / 2; j < m; j++){
        gsl_matrix_complex_set(new_op->matrix, i, j, gsl_matrix_complex_get(op->matrix, i - (m/2), j - (m/2)));
      }
    }
  }
  q_op_free(op);
  return new_op;
}

q_op* swap(q_op* op);
q_op* swap(q_op* op){

  int* map = malloc(2 * sizeof(int));
  map[0] = 1;
  map[1] = 0;
  q_op* swap = q_swap(2, map);
  q_op* new_op = q_op_multiply(swap, op);
  q_op* new_op2 = q_op_multiply(new_op, swap);
  q_op_free(swap);
  q_op_free(op);
  q_op_free(new_op);
  free(map);
  return new_op2;
}

char* str_cpy(char* src);
char* str_cpy(char* src){
  int len = strlen(src) + 1;
  char* trg = malloc(len * sizeof(char));
  for(int i = 0; i < len; i++){
    trg[i] = src[i];
  }
  return trg;
}

char* str_cat(char* a, char* b);
char* str_cat(char* a, char* b){
  int lenA = strlen(a);
  int lenB = strlen(b);
  int new_len = lenA + lenB + 1;
  char* trg = malloc(new_len * sizeof(char));
  for(int i = 0; i < lenA; i++){
    trg[i] = a[i];
  }
  for(int i = 0; i < lenB + 1; i++){
    trg[i + lenA] = b[i];
  }
  return trg;
}

qap_oper* make_oper(function** funcs, int func_count){
  q_op* o = one();
  q_op* op = q_op_tensor(o, funcs[0]->op);
  q_op_free(o);
  char* name = str_cpy(funcs[0]->name);
  for(int i = 1; i < func_count; i++){

    q_op* new_op = q_op_tensor(op, funcs[i]->op);
    char* new_name = str_cat(name, funcs[i]->name);
    free(name);
    q_op_free(op);
    op = new_op;
    name = new_name;
  }

  qap_oper* oper = malloc(sizeof(qap_oper));
  oper->op = op;
  oper->name = name;

  return oper;
}

void free_oper(qap_oper* oper){
  q_op_free(oper->op);
  free(oper->name);
  free(oper);
}

qap_column* make_column(qap_oper** opers, int oper_count){
  qap_column* column = malloc(sizeof(qap_column));
  column->opers = opers;
  column->oper_count = oper_count;
  return column;
}

void free_column(qap_column* column){
  for(int i = 0; i < column->oper_count; i++){
    free_oper(column->opers[i]);
  }
  free(column->opers);
  free(column);
}

qap_column* generate_column(function** funcs, int func_count, int qubits){
  int count = 0;
  for(int i = 0; i < func_count; i++){
    count += qubits - (funcs[i]->qubits - 1);
  }
  function* W = make_function(q_identity(1), 1, " W ");
  qap_oper** opers = malloc(count * sizeof(qap_oper*));
  int c = 0;
  for(int i = 0; i < func_count; i++){
    for(int j = 0; j < qubits - (funcs[i]->qubits - 1); j++){
      function** fset = malloc((qubits - (funcs[i]->qubits - 1)) * sizeof(function*));
      for(int k = 0; k < j; k++){
        fset[k] = W;
      }
      fset[j] = funcs[i];
      for(int k = j + 1; k < qubits - (funcs[i]->qubits - 1); k++){
        fset[k] = W;
      }
      opers[c] = make_oper(fset, qubits - (funcs[i]->qubits - 1));
      free(fset);
      c++;
    }
  }
  free_function(W);
  return make_column(opers, count);
}

void print_column(qap_column* column){
  //printf("QAP Column with %d Rows\n", column->oper_count);
  for(int i = 0; i < column->oper_count; i++){
    //printf("%d: %s\n", i, column->opers[i]->name);
    q_op_print(column->opers[i]->op);
  }
}

qap_graph* make_graph(qap_column* column, int columns, double min_p){
  qap_graph* graph = malloc(sizeof(qap_graph));

  graph->column = column;
  graph->columns = columns;
  graph->ant_map = malloc(columns * sizeof(double**));
  graph->ant_map[0] = malloc(sizeof(double*));
  graph->ant_map[0][0] = malloc(column->oper_count * sizeof(double));
  for(int i = 0; i < column->oper_count; i++){
    graph->ant_map[0][0][i] = min_p;
  }
  for(int i = 1; i < columns; i++){
    graph->ant_map[i] = malloc(column->oper_count * sizeof(double*));
    for(int j = 0; j < column->oper_count; j++){
      graph->ant_map[i][j] = malloc(column->oper_count * sizeof(double));
      for(int k = 0; k < column->oper_count; k++){
        graph->ant_map[i][j][k] = min_p;
      }
    }
  }

  return graph;
}

void free_graph(qap_graph* graph){
  int columns = graph->columns;
  int rows = graph->column->oper_count;

  free(graph->ant_map[0][0]);
  free(graph->ant_map[0]);
  for(int i = 1; i < columns; i++){
    for(int j = 0; j < rows; j++){
      free(graph->ant_map[i][j]);
    }
    free(graph->ant_map[i]);
  }
  free(graph->ant_map);
  free_column(graph->column);
  free(graph);
}

void print_graph(qap_graph* graph){
  //printf("Graph with %d columns\n", graph->columns);
  print_column(graph->column);
}

function* make_function(q_op* op, int qubits, char* name){
  function* f = malloc(sizeof(function));
  f->op = op;
  f->name = name;
  f->qubits = qubits;
  return f;
}

void print_ant_map(qap_graph* graph){
  int rows = graph->column->oper_count;
  int columns = graph->columns;
  //printf("Ant Map with %d columns and %d rows\n", columns, rows);
  //printf("Column 0 (init choice)\n  ");
  for(int i = 0; i < rows; i++){
    if(i != 0){
      //printf(", ");
    }
    //printf("%lf", graph->ant_map[0][0][i]);
  }
  //printf("\n");
  for(int i = 1; i < columns; i++){
    //printf("Column %d:\n", i);
    for(int j = 0; j < rows; j++){
      //printf("  Row %d:\n   ", j);
      for(int k = 0; k < rows; k++){
        if(k != 0){
          //printf(", ");
        }
        //printf("%lf", graph->ant_map[i][j][k]);
      }
      //printf("\n");
    }
  }
}

void free_function(function* func){
  q_op_free(func->op);
  free(func);
}

function** core_fset(){
  function** fset = malloc(5 * sizeof(function*));
  fset[0] = make_function(q_identity(1), 1, " W ");
  fset[1] = make_function(q_hadamard(), 1, " H ");
  fset[2] = make_function(q_t(), 1, " T ");
  fset[3] = make_function(q_cX(), 2, " cX ");
  int* map = malloc(2 * sizeof(int));
  map[0] = 1;
  map[1] = 0;
  fset[4] = make_function(q_swap(2, map), 2, " Sw ");
  free(map);
  return fset;
}

function** ccore_fset(){
  function** fset = malloc(11 * sizeof(function*));
  fset[0] = make_function(q_identity(1), 1, " W ");
  fset[1] = make_function(q_hadamard(), 1, " H ");
  fset[2] = make_function(q_t(), 1, " T ");
  fset[3] = make_function(q_pauli_X(), 1, " X ");
  fset[4] = make_function(make_controllable(q_hadamard()), 2, " cH ");
  fset[5] = make_function(make_controllable(q_t()), 2, " cT ");
  fset[6] = make_function(q_cX(), 2, " cX ");
  fset[8] = make_function(swap(make_controllable(q_hadamard())), 2, " uH ");
  fset[9] = make_function(swap(make_controllable(q_t())), 2, " uT ");
  fset[10] = make_function(swap(q_cX()), 2, " uX ");
  int* map = malloc(2 * sizeof(int));
  map[0] = 1;
  map[1] = 0;
  fset[7] = make_function(q_swap(2, map), 2, " Sw ");
  free(map);
  return fset;
}

function** rot_fset(){
  function** fset = malloc(15 * sizeof(function*));
  fset[0] = make_function(q_identity(1), 1, " W ");
  fset[1] = make_function(q_hadamard(), 1, " H ");
  fset[2] = make_function(q_t(), 1, " T ");
  fset[3] = make_function(q_cX(), 2, " cX ");
  fset[4] = make_function(q_rot_z(1.0/4.0), 1, " r2 ");
  fset[5] = make_function(q_rot_z(1.0/8.0), 1, " r4 ");
  fset[6] = make_function(q_rot_z(1.0/16.0), 1, " r8 ");
  fset[7] = make_function(q_rot_z(1.0/32.0), 1, " r16 ");
  fset[8] = make_function(q_rot_z(1.0/64.0), 1, " r32 ");
  fset[9] = make_function(q_crot_z(1.0/4.0), 2, " cr2 ");
  fset[10] = make_function(q_crot_z(1.0/8.0), 2, " cr4 ");
  fset[11] = make_function(q_crot_z(1.0/16.0), 2, " cr8 ");
  fset[12] = make_function(q_crot_z(1.0/32.0), 2, " cr16 ");
  fset[13] = make_function(q_crot_z(1.0/64.0), 2, " cr32 ");
  int* map = malloc(2 * sizeof(int));
  map[0] = 1;
  map[1] = 0;
  fset[14] = make_function(q_swap(2, map), 2, " Sw ");
  free(map);
  return fset;
}

void free_fset(function** fset, int f_count){
  for(int i = 0; i < f_count; i++){
    free_function(fset[i]);
  }
  free(fset);
}

int pick_from(double* pheremones, int choices);
int pick_from(double* pheremones, int choices){
  double total_pheremone = 0.0;
  for(int i = 0; i < choices; i++){
    total_pheremone += pheremones[i];
  }
  double r = rand_double() * total_pheremone;
  total_pheremone = 0.0;
  for(int i = 0; i < choices; i++){
    total_pheremone += pheremones[i];
    if(total_pheremone + 0.000001 > r){
      return i;
    }
  }
  //printf("Error: Could not pick Pheremone!\n");
  exit(0);
}

int* generate_route(qap_graph* graph){
  int position = 0;
  int* route = malloc(graph->columns * sizeof(int));
  for(int i = 0; i < graph->columns; i++){
    int new_position = pick_from(graph->ant_map[i][position], graph->column->oper_count);
    route[i] = new_position;
    position = new_position;
  }
  return route;
}

int routes_equal(int* a, int* b, int route_length){
  for(int i = 0; i < route_length; i++){
    if(a[i] != b[i]){
      return 0;
    }
  }
  return 1;
}

void lay_pheremone(qap_graph* graph, int* route, double pheremone, double learning_rate, double diffusion_rate){
  int current = 0;
  double mL = 1.0 - learning_rate;
  double diff = learning_rate * diffusion_rate;
  double mDiff = 1.0 - diff;
  for(int i = 0; i < graph->columns; i++){
    int next = route[i];
    graph->ant_map[i][current][next] = graph->ant_map[i][current][next] + pheremone;
    if(i != 0){
      for(int j = 0; j < graph->column->oper_count; j++){
        if(current != j){
          graph->ant_map[i][j][next] = (mDiff * graph->ant_map[i][j][next]) + (diff * pheremone);
        }
      }
    }
    current = next;
  }
}


void update_pheremone(qap_graph* graph, double p_min, double p_max, double evap_rate){
  for(int i = 0; i < graph->column->oper_count; i++){
    graph->ant_map[0][0][i] = graph->ant_map[0][0][i] * (1.0 - evap_rate);
    if(graph->ant_map[0][0][i] < p_min){
      graph->ant_map[0][0][i] = p_min;
    }
    if(graph->ant_map[0][0][i] > p_max){
      graph->ant_map[0][0][i] = p_max;
    }
  }
  for(int i = 1; i < graph->columns; i++){
    for(int j = 0; j < graph->column->oper_count; j++){
      for(int k = 0; k < graph->column->oper_count; k++){
        graph->ant_map[i][j][k] = graph->ant_map[i][j][k] * (1.0 - evap_rate);
        if(graph->ant_map[i][j][k] < p_min){
          graph->ant_map[i][j][k] = p_min;
        }
        if(graph->ant_map[i][j][k] > p_max){
          graph->ant_map[i][j][k] = p_max;
        }
      }
    }
  }
}

void reset_pheremone(qap_graph* graph, double p_init){
  for(int i = 0; i < graph->column->oper_count; i++){
    graph->ant_map[0][0][i] = p_init;
  }
  for(int i = 1; i < graph->columns; i++){
    for(int j = 0; j < graph->column->oper_count; j++){
      for(int k = 0; k < graph->column->oper_count; k++){
        graph->ant_map[i][j][k] = p_init;
      }
    }
  }
}

q_op* route_to_op(int* route, qap_graph* graph){
  q_op* o = one();
  q_op* op = q_op_tensor(o, graph->column->opers[route[0]]->op);
  q_op_free(o);
  for(int i = 1; i < graph->columns; i++){
    q_op* new_op = q_op_multiply(graph->column->opers[route[i]]->op, op);
    q_op_free(op);
    op = new_op;
  }
  return op;
}

double mean_square_fidelity(q_op* op, dataset* dataset){
  double sum = 0.0;
  for(int i = 0; i < dataset->entries; i++){
    q_state* res = apply_qop(op, dataset->X[i]);
    q_state_normalize(res);
    double fid = fidelity(res, dataset->Y[i]);
    sum += (fid * fid);
    q_state_free(res);
  }
  return sum / (double)dataset->entries;
}

dataset* make_dataset(int entries, q_state** X, q_state** Y){
  dataset* d = malloc(sizeof(dataset));
  d->entries = entries;
  d->X = X;
  d->Y = Y;
  return d;
}

void free_dataset(dataset* d){
  for(int i = 0; i < d->entries; i++){
    q_state_free(d->X[i]);
    q_state_free(d->Y[i]);
  }
  free(d->X);
  free(d->Y);
  free(d);
}

params* make_params(qap_graph* graph, dataset* dataset, double target_score, int ants, int max_iter, double p_init, double p_min, double p_max, double l_rate, double evap_rate, double diffusion){
  params* par = malloc(sizeof(params));
  par->graph = graph;
  par->dataset = dataset;
  par->target_score = target_score;
  par->ants = ants;
  par->max_iter = max_iter;
  par->p_init = p_init;
  par->p_min = p_min;
  par->p_max = p_max;
  par->l_rate = l_rate;
  par->evap_rate = evap_rate;
  par->diffusion = diffusion;
  return par;
}

void free_params(params* par){
  free_graph(par->graph);
  free_dataset(par->dataset);
  free(par);
}

params* default_params(qap_graph* graph, dataset* dataset){
  return make_params(graph, dataset, 0.98, 10, 1000000, 1.0, 0.01, 1.0, 0.05, 0.03, 0.5);
}

result* run_qap(params* params){
  reset_pheremone(params->graph, params->p_init);
  int* elite_route = malloc(params->graph->columns * sizeof(int));
  double elite_score = 0.0;
  int iter = 0;
  int stag = 0;
  while(iter < params->max_iter && elite_score < params->target_score){

    int** ant_routes = malloc(params->ants * sizeof(int*));
    double* ant_scores = malloc(params->ants * sizeof(double));
    int best_ant = -1;
    double best_score = 0.0;
    for(int a = 0; a < params->ants; a++){
      ant_routes[a] = generate_route(params->graph);
      while(iter > 0 && routes_equal(ant_routes[a], elite_route, params->graph->columns) == 1){
        free(ant_routes[a]);
        ant_routes[a] = generate_route(params->graph);
      }
      q_op* op = route_to_op(ant_routes[a], params->graph);
      ant_scores[a] = mean_square_fidelity(op, params->dataset);
      if(ant_scores[a] > best_score){
        best_ant = a;
        best_score = ant_scores[a];
      }
      q_op_free(op);
    }
    //printf("\rIteration %d (%d). Ants have found solution with MSF %lf", iter, stag, elite_score);
    //printf(" Scores [");
    for(int a = 0; a < params->ants; a++){
      if(a == best_ant){
        lay_pheremone(params->graph, ant_routes[a], ant_scores[a], 0.0 , 0.0);
      }
      if( a < 10){
        if(a != 0){
          //printf(", ");
        }
        //printf("%lf (%d)", ant_scores[a], routes_equal(ant_routes[a], elite_route, params->graph->columns));
      }
      if(ant_scores[a] >= elite_score + 0.0001 && (iter == 0 || routes_equal(ant_routes[a], elite_route, params->graph->columns) != 1)){
        free(elite_route);
        elite_route = ant_routes[a];
        elite_score = ant_scores[a];
        stag = 0;
      }
      else{
        free(ant_routes[a]);
      }
    }
    //printf("]           ");
    //lay_pheremone(params->graph, elite_route, elite_score, params->l_rate, params->diffusion);
    if(stag < 0){
      reset_pheremone(params->graph, params->p_init);
      elite_score = 0.0;
      stag = 0;
    }
    else{
      update_pheremone(params->graph, params->p_min, params->p_max, params->evap_rate);
    }
    free(ant_routes);
    free(ant_scores);
    iter++;
    stag++;
  }

  result* res = malloc(sizeof(result));
  res->iter = iter;
  res->route_score = elite_score;
  res->route = elite_route;
  res->route_op = route_to_op(elite_route, params->graph);
  //printf("\n Finished on iteration %d with score %lf and with Route:\n", res->iter, res->route_score);
  for(int i = 0; i < params->graph->columns; i++){
    if(i != 0){
      //printf(", ");
    }
    //printf("%d", res->route[i]);
  }

  //printf("\nOp: \n");
  q_op_print(res->route_op);
  //printf("\n");

  return res;
}
