#include "quant.h"

q_state* r_q(){
    double r_1 = rand_double() * 2.0 * M_PI;
    double r_2 = rand_double() * 2.0 * M_PI;
    gsl_complex z;
    gsl_complex o;
    GSL_SET_COMPLEX(&z, cos(r_1 / 2.0), 0.0);
    GSL_SET_COMPLEX(&o, cos(r_2) * sin(r_1 / 2.0), sin(r_2) * sin(r_1 / 2.0));
    q_state* r = q_state_calloc(1);
    gsl_matrix_complex_set(r->vector, 0, 0, z);
    gsl_matrix_complex_set(r->vector, 1, 0, o);
    return r;
}

q_state* r_q_qubits(int qubits){
    q_state* q = r_q();
    for(int i = 1; i < qubits; i++){
        q_state* qi = r_q();
        q_state* q2 = q_state_tensor(qi, q);
        q_state_free(qi);
        q_state_free(q);
        q = q2;
    }
    return q;
}

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
  return new_op;
}

q_op* one(){
  q_op* o = q_op_alloc(0);
  gsl_matrix_complex_set(o->matrix, 0, 0, GSL_COMPLEX_ONE);
  return o;
}
void randomize ( int* arr, int n );
// A function to generate a random permutation of arr[] SOURCE: https://www.geeksforgeeks.org/shuffle-a-given-array/
void randomize ( int* arr, int n )
{
    // Start from the last element and swap one by one. We don't
    // need to run for the first element that's why i > 0
    for (int i = n-1; i > 0; i--)
    {
        // Pick a random index from 0 to i
        int j = rand() % (i+1);

        // Swap arr[i] with the element at random index
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}


int get_rows(function** function_set, int functions, int qubits);

//Works out how many total occurrences of each function is needed to give maximum coverage. E.g. if there are 4 qubits, we can use at most 1 of each 3-qubit gates but 2 of each 2-qubit gates
int get_rows(function** function_set, int functions, int qubits){
  int count = 0;
  for(int i = 0; i < functions; i++){
    int arity = function_set[i]->qubits;
    int mul = qubits/arity;
    count += (mul * (arity + function_set[i]->controllable));
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

function* make_function(q_op* op, char* name, int controllable){
  function* f = malloc(sizeof(function));
  f->op = op;
  f->name = name;
  f->qubits = op->qubits;
  f->controllable = controllable;
  if(controllable == 1){
    f->control_op = make_controllable(op);
  }
  return f;
}


void free_function(function* f){
  q_op_free(f->op);
  if(f->controllable == 1){
    q_op_free(f->control_op);
  }
  free(f);
}

void free_fset(function** fset, int count){
  for(int i = 0; i < count; i++){
    free_function(fset[i]);
  }
  free(fset);
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
    arity +=  f_set[i]->controllable;
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
    for(int j = 0; j < c->f_set[func]->qubits + c->f_set[func]->controllable; j++){
      printf(" %d,", c->track_index[i][1][j]);
    }
    q_op_print(c->f_set[func]->op);
    if(c->f_set[func]->controllable == 1){
      q_op_print(c->f_set[func]->control_op);
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
    g->ant_maps[i] = make_ant_map(c->n, columns, qubits);
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

ant_map* make_ant_map(int n, int columns, int qubits){
  ant_map* m = malloc(sizeof(ant_map));
  m->n = n;
  m->columns = columns;
  m->qubits = qubits;
  m->pheremone_map = malloc((columns + 1) * sizeof(double**));
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
  m->pheremone_map[columns] = malloc(n * sizeof(double*));
  for(int i = 0; i < n; i++){

      m->pheremone_map[columns][i] = malloc(qubits * sizeof(double));
      for(int q = 0; q < qubits; q++){
      	m->pheremone_map[columns][i][q] = 0.0;
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

  for(int q = 0; q < m->n; q++){
	free(m->pheremone_map[m->columns][q]);
  }
  free(m->pheremone_map[m->columns]);

  free(m->pheremone_map);

  free(m);
}

void add_route(qap_graph* g, int** route, double fitness, double learning_rate, double diffusion, double p_max){
  if (learning_rate != 0.0){
	  for(int i = 0; i < g->qubits; i++){
	    //1st case
	    ant_map* m = g->ant_maps[i];
	    m->pheremone_map[0][0][route[i][0]] = m->pheremone_map[0][0][route[i][0]] + fitness;
	    //m->pheremone_map[0][0][route[i][0]] = (learning_rate * fitness);
	    if(m->pheremone_map[0][0][route[i][0]] > p_max){
	      m->pheremone_map[0][0][route[i][0]] = p_max;
	    }
	    int current = route[i][0];
	    for(int j = 1; j < m->columns + 1; j++){
	      //printf("\nUpdate %lf -> %lf \n", m->pheremone_map[j][current][route[i][j]], (o * m->pheremone_map[j][current][route[i][j]]) + (learning_rate * fitness));
	      m->pheremone_map[j][current][route[i][j]] = m->pheremone_map[j][current][route[i][j]] + fitness;
	      //m->pheremone_map[j][current][route[i][j]] += (learning_rate * fitness);
	      if(m->pheremone_map[j][current][route[i][j]] > p_max){
		m->pheremone_map[j][current][route[i][j]] = p_max;
	      }
	      current = route[i][j];
	    }
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
      for(int k = 0; k < n; k++){
        for(int l = 0; l < m->qubits; l++){
          m->pheremone_map[m->columns][k][l] = m->pheremone_map[m->columns][k][l] * (1.0 - evaporation);
          if(m->pheremone_map[m->columns][k][l] > max){
            m->pheremone_map[m->columns][k][l] = max;
          }
          else if(m->pheremone_map[j][k][l] < min){
            m->pheremone_map[m->columns][k][l] = min;
          }
        }
      }
    }
  }
}



void reset_pheremone(qap_graph* g, double min){
  for(int i = 0; i < g->qubits; i++){
    //1st case
    ant_map* m = g->ant_maps[i];
    int n = m->n;
    for(int j = 0; j < n; j++){
      m->pheremone_map[0][0][j] = min;
    }
    for(int j = 1; j < m->columns; j++){
      for(int k = 0; k < n; k++){
        for(int l = 0; l < n; l++){
          m->pheremone_map[j][k][l] = min;
        }
      }
    }
    for(int k = 0; k < n; k++){
      for(int l = 0; l < m->qubits; l++){
        m->pheremone_map[m->columns][k][l] = min;
      }
    }
  }
}

int pick_from(double* weights, int* blocked, int* cooperate, double cooperate_bonus, int n){
  double total_weight = 0.0;
  int last = 0;
  for(int i = 0; i < n; i++){
    if(blocked[i] == 0){
      double weight = weights[i];
      if(cooperate[i] == 1){
        weight = weight * cooperate_bonus;
      }
      total_weight += weight;
      last = i;
    }
  }
  double in = rand_double() * total_weight;
  total_weight = 0.0;
  for(int i = 0; i < n; i++){
    if(blocked[i] == 0){
      double weight = weights[i];
      if(cooperate[i] == 1){
        weight = weight * cooperate_bonus;
      }
      total_weight += weight;
      if(in <= total_weight || i == last){
        return i;
      }
    }
  }
  printf("Route selection failed\n");
    printf("\n [");
    total_weight = 0.0;
    for(int i = 0; i < n; i++){
      if(blocked[i] == 0){
        double weight = weights[i];
        if(cooperate[i] == 1){
          weight = weight * cooperate_bonus;
        }
        total_weight += weight;
          printf("(%d-%lf = %lf) ", i, weights[i], total_weight);
      }
    }
    printf("], %lf chosen, ", in);
    total_weight = 0.0;
    for(int i = 0; i < n; i++){
      if(blocked[i] == 0){
        double weight = weights[i];
        if(cooperate[i] == 1){
          weight = weight * cooperate_bonus;
        }
        total_weight += weight;
      }
      if(in <= total_weight){
        printf("%d chosen \n", i);
        return i;
      }
    }
  exit(0);
}

int* generate_route(qap_column* c, ant_map* m, int** blocked, int** cooperate, double cooperate_bonus, double elite_sel_p, int* elite_route){
  int* route = malloc((m->columns + 1) * sizeof(int));
  if(rand_double() <= elite_sel_p && !blocked[0][elite_route[0]]){
    route[0] = elite_route[0];
  }
  else{
    route[0] = pick_from(m->pheremone_map[0][0], blocked[0], cooperate[0], cooperate_bonus, m->n);
  }
  int current = route[0];
  blocked[0][current] = 1;

  int** track_function = c->track_index[c->f_index[current]];
  function* f = c->f_set[track_function[0][0]];
  for(int k = 0; k < f->qubits + f->controllable; k++){
    int index = track_function[1][k];
    cooperate[0][index] = 1;
  }

  for(int i = 1; i < m->columns; i++){
    if(rand_double() <= elite_sel_p && !blocked[i][elite_route[i]]){
      route[i] = elite_route[i];
    }
    else{
      route[i] = pick_from(m->pheremone_map[i][current], blocked[i], cooperate[i], cooperate_bonus, m->n);
    }
    current = route[i];
    blocked[i][current] = 1;
    int** track_function = c->track_index[c->f_index[current]];
    function* f = c->f_set[track_function[0][0]];
    for(int k = 0; k < f->qubits + f->controllable; k++){
      int index = track_function[1][k];
      cooperate[i][index] = 1;
    }


  }

  route[m->columns] = pick_from(m->pheremone_map[m->columns][current], blocked[m->columns], cooperate[m->columns], cooperate_bonus, m->qubits);
  blocked[m->columns][route[m->columns]] = 1;

  return route;
}

int** generate_routes(qap_graph* g, double elite_sel_p, double cooperate_bonus, int ** elite_routes){
  int** blocked = malloc((g->columns + 1) * sizeof(int*));
  int** cooperate = malloc((g->columns + 1) * sizeof(int*));
  for(int i = 0; i < g->columns; i++){
    blocked[i] = malloc(g->column->n * sizeof(int));
    cooperate[i] = malloc(g->column->n * sizeof(int));
    for(int j = 0; j < g->column->n; j++){
      blocked[i][j] = 0;
      cooperate[i][j] = 0;
    }
  }

  blocked[g->columns] = malloc(g->qubits * sizeof(int));
  cooperate[g->columns] = malloc(g->qubits * sizeof(int));
  for(int j = 0; j < g->qubits; j++){
    blocked[g->columns][j] = 0;
    cooperate[g->columns][j] = 0;
  }


  int** routes = malloc(g->qubits * sizeof(int*));
  int* order = malloc(g->qubits * sizeof(int));
  for(int i = 0; i < g->qubits; i++){
    order[i] = i;
  }
  randomize(order, g->qubits);
  for(int i = 0; i < g->qubits; i++){
    if(elite_sel_p != -1.0){
      routes[order[i]] = generate_route(g->column, g->ant_maps[order[i]], blocked, cooperate, cooperate_bonus, elite_sel_p, elite_routes[order[i]]);
    }
    else{
      routes[order[i]] = generate_route(g->column, g->ant_maps[order[i]], blocked, cooperate, cooperate_bonus, elite_sel_p, NULL);
    }
  }
  for(int i = 0; i < g->columns + 1; i++){
    free(blocked[i]);
    free(cooperate[i]);
  }
  free(blocked);
  free(cooperate);
  return routes;
}

void free_routes(int** routes, int qubits){
  for(int i = 0; i < qubits; i++){
    free(routes[i]);
  }
  free(routes);
}

q_op* make_q_op(qap_graph* g, int** routes){
  int* maps_to = malloc(g->qubits * sizeof(int));
  q_op* op;
  q_op* W = q_identity(1);
  int* location = malloc(g->qubits * sizeof(int));
  for(int i = 0; i < g->qubits; i++){
    location[i] = i;
  }
  for(int i = 0; i < g->columns; i++){
    q_op* this_op = one();
    int up_to_q = 0;
    for(int j = 0; j < g->column->f; j++){
      int** track_function = g->column->track_index[j];
      function* f = g->column->f_set[track_function[0][0]];
      int present = 0;
      int first_present = 0;
      for(int k = 0; k < f->qubits + f->controllable; k++){
        int index = track_function[1][k];
        for(int q = 0; q < g->qubits; q++){
          if(routes[q][i] == index){
            if(k == 0){
              first_present = 1;
            }
            present++;
            maps_to[location[q]] = up_to_q;
            location[q] = up_to_q;
            up_to_q++;
          }
        }
      }
      if(present == f->qubits && (f->controllable == 0 || first_present == 0)){
        q_op* this_new_op = q_op_tensor(this_op, f->op);
        q_op_free(this_op);
        this_op = this_new_op;
      }
      else if(present == f->qubits + f->controllable){
        q_op* this_new_op = q_op_tensor(this_op, f->control_op);
        q_op_free(this_op);
        this_op = this_new_op;
      }
      else{
        for(int q = 0; q < present; q++){
          q_op* this_new_op = q_op_tensor(this_op, W);
          q_op_free(this_op);
          this_op = this_new_op;
        }
      }
    }
    q_op* map = q_swap(g->qubits, maps_to);
    q_op* this_new_op = q_op_multiply(this_op, map);
    q_op_free(map);
    q_op_free(this_op);
    if(i == 0){
      op = this_new_op;
    }
    else{
      q_op* new_op = q_op_multiply(this_new_op, op);
      q_op_free(op);
      op = new_op;
      q_op_free(this_new_op);
    }
    free(maps_to);
    maps_to = malloc(g->qubits * sizeof(int));
  }
  for(int i = 0; i < g->qubits; i++){
    maps_to[location[i]] = routes[i][g->columns];
  }
  q_op* map = q_swap(g->qubits, maps_to);
  q_op* new_op = q_op_multiply(map, op);
  q_op_free(op);
  q_op_free(map);
  op = new_op;

  q_op_free(W);
  free(maps_to);
  free(location);
  return op;
}


int count_op(qap_graph* g, int** routes){
  int count = 0;
  for(int i = 0; i < g->columns; i++){
    int up_to_q = 0;
    for(int j = 0; j < g->column->f; j++){
      int** track_function = g->column->track_index[j];
      function* f = g->column->f_set[track_function[0][0]];
      int present = 0;
      int first_present = 0;
      for(int k = 0; k < f->qubits + f->controllable; k++){
        int index = track_function[1][k];
        for(int q = 0; q < g->qubits; q++){
          if(routes[q][i] == index){
            if(k == 0){
              first_present = 1;
            }
            present++;
            up_to_q++;
          }
        }
      }
      if(((present == f->qubits && (first_present == 0 || f->controllable == 0)) || present == f->qubits + f->controllable) && strcmp(f->name, "W") != 0){
        count++;
      }
    }
  }
  return count;
}


void print_op(qap_graph* g, int** routes){
  int* maps_to = malloc(g->qubits * sizeof(int));
  int* location = malloc(g->qubits * sizeof(int));
  for(int i = 0; i < g->qubits; i++){
    location[i] = i;
    printf("Route %d: ", i);
    for(int j = 0; j < g->columns; j++){
      printf("%d, ", routes[i][j]);
    }
    printf("\n");
  }
  for(int i = 0; i < g->columns; i++){
    int up_to_q = 0;
    for(int j = 0; j < g->column->f; j++){
      int** track_function = g->column->track_index[j];
      function* f = g->column->f_set[track_function[0][0]];
      int present = 0;
      int first_present = 0;
      for(int k = 0; k < f->qubits + f->controllable; k++){
        int index = track_function[1][k];
        for(int q = 0; q < g->qubits; q++){
          if(routes[q][i] == index){
            if(k == 0){
              first_present = 1;
            }
            present++;
            maps_to[location[q]] = up_to_q;
            location[q] = up_to_q;
            printf(" loc%d=%d ", q, up_to_q);
            up_to_q++;
          }
        }
      }
      if(present == f->qubits && (f->controllable == 0 || first_present == 0)){
        printf(" %s ", f->name);
      }
      else if(present == f->qubits + f->controllable){
        printf(" c%s ", f->name);
      }
      else{
        for(int q = 0; q < present; q++){
          printf(" W ");
        }
      }
    }
    printf(" (");
    for(int i = 0; i < g->qubits; i++){
      printf(", %d", maps_to[i]);
    }
    printf(")\n");
    free(maps_to);
    maps_to = malloc(g->qubits * sizeof(int));
  }
  for(int i = 0; i < g->qubits; i++){
    printf("Location %d: %d\n", i, location[i]);
    maps_to[location[i]] = i;
  }
  printf(" \n(");
  for(int i = 0; i < g->qubits; i++){
    printf(", %d", maps_to[i]);
  }
  printf(")\n");
  free(maps_to);
  free(location);
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


double mean_square_fidelity_error(q_op* opA, q_op* opB, error_dataset* dataset){
  double sum = 0.0;
  for(int i = 0; i < dataset->entries; i++){
    q_state* res = apply_qop(opA, dataset->X[i]);
    for(int j = 0; j < dataset->error_count; j++){
        q_state* post_error = apply_qop(dataset->error_functions[j], res);
        q_state* fix = apply_qop(opB, post_error);
        q_state_normalize(fix);
        double max_fid = 0.0;
        for(int k = 0; k < dataset->error_count; k++){
          double fid = fidelity(fix, dataset->Y[i][k]);
          if(fid > max_fid){
            max_fid = fid;
          }
        }
        sum += (max_fid * max_fid);
        q_state_free(post_error);
        q_state_free(fix);
    }
    q_state_free(res);
  }
  return sum / ((double)dataset->entries * (double)dataset->error_count);
}

double print_mean_square_fidelity(q_op* op, dataset* dataset);
double print_mean_square_fidelity(q_op* op, dataset* dataset){
  double sum = 0.0;
  for(int i = 0; i < dataset->entries; i++){
    printf("Input \n");
    q_state_print(dataset->X[i]);
    q_state* res = apply_qop(op, dataset->X[i]);
    q_state_normalize(res);
    printf("Produces \n");
    q_state_print(res);
    printf("Expects \n");
    q_state_print(dataset->Y[i]);
    double fid = fidelity(res, dataset->Y[i]);
    printf("Fidelity %lf\n", fid);
    sum += (fid * fid);
    q_state_free(res);
  }
  printf("Mean %lf => %lf\n", sum, sum / (double)dataset->entries);
  return sum / (double)dataset->entries;
}

dataset* make_dataset(int entries, q_state** X, q_state** Y){
  dataset* d = malloc(sizeof(dataset));
  d->entries = entries;
  d->X = X;
  d->Y = Y;
  return d;
}

dataset* make_basic_dataset(q_op* op, int qubits, int examples) {
    q_state** X = malloc(examples * sizeof(q_state*));
    q_state** Y = malloc(examples * sizeof(q_state*));
    for (int i = 0; i < examples; i++) {
        X[i] = r_q_qubits(qubits);
        Y[i] = apply_qop(op, X[i]);
        q_state_normalize(X[i]);
        q_state_normalize(Y[i]);
    }
    return make_dataset(examples, X, Y);
}

error_dataset* make_error_dataset(int entries, q_state** X, q_state*** Y, q_op** error_functions, int error_count){
  error_dataset* d = malloc(sizeof(error_dataset));
  d->entries = entries;
  d->X = X;
  d->Y = Y;
  d->error_functions = error_functions;
  d->error_count = error_count;
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

void free_error_dataset(error_dataset* d){
  for(int i = 0; i < d->entries; i++){
    q_state_free(d->X[i]);
    for(int j = 0; j < d->error_count; j++){
      q_state_free(d->Y[i][j]);
    }
    free(d->Y[i]);
  }
  free(d->X);
  free(d->Y);
  for(int i = 0; i < d->error_count; i++){
    q_op_free(d->error_functions[i]);
  }
  free(d->error_functions);
  free(d);
}

double get_p_min(double p_mid, double p_var, int gens, int l, int q);
double get_p_min(double p_mid, double p_var, int gens, int l, int q){
	double cycle_p = ((double)(gens % l))/((double)l) * 2 * 3.14159;

	double q_p = 1.0 / ((double)q * (double)q * (double)q);
	//double cycle_p = 2.0 * ((double)(gens % l))/((double)l);
	//if(cycle_p < 1.0){
	//	return (p_mid + p_var) * cycle_p * q_p;
	//}
	//else{
	//	return (1.0 + (1.0 - cycle_p)) * (p_mid + p_var) * q_p;
	//}

	//double q_p = 1.0 / ((double)q * (double)q * (double)q);
	return (p_mid + (p_var * cos(cycle_p))) * q_p;
}

int** make_wire_route(qap_graph* g);
int* generate_wire(qap_column* c, int** blocked, int columns);
int pick_wire(qap_column* c, int* blocked);

int** make_wire_route(qap_graph* g){
  int** blocked = malloc(g->columns * sizeof(int*));
  for(int i = 0; i < g->columns; i++){
    blocked[i] = malloc(g->column->n * sizeof(int));
    for(int j = 0; j < g->column->n; j++){
      blocked[i][j] = 0;
    }
  }
  int** routes = malloc(g->qubits * sizeof(int*));
  int* order = malloc(g->qubits * sizeof(int));
  for(int i = 0; i < g->qubits; i++){
    order[i] = i;
  }
  randomize(order, g->qubits);
  for(int i = 0; i < g->qubits; i++){
    routes[order[i]] = generate_wire(g->column, blocked, g->columns);
  }
  for(int i = 0; i < g->columns; i++){
    free(blocked[i]);
  }
  free(blocked);
  return routes;
}

int* generate_wire(qap_column* c, int** blocked, int columns){
  int* route = malloc(columns * sizeof(int));
  route[0] = pick_wire(c, blocked[0]);
  int current = route[0];
  blocked[0][current] = 1;

  for(int i = 1; i < columns; i++){
    route[i] = pick_wire(c, blocked[i]);
    current = route[i];
    blocked[i][current] = 1;
  }
  return route;
}

int pick_wire(qap_column* c, int* blocked){
  double total_weight = 0.0;
  int last = 0;
  int n = c->n;
  for(int i = 0; i < n; i++){
	if(blocked[i] == 0 && strcmp(c->f_set[c->f_index[i]]->name, "W") == 0){
		return i;
	}
  }
}

result* run_qap(params* p, dataset* d){

  int** elite_routes;
  q_op* o = q_identity(p->g->qubits);
  double elite_score = 0.0;

  printf("Init score %lf\n", elite_score);
  int iter = 0;
  int first = 0;
  double stag_score = 0.0;
  int stagnation = 0;
  int elite_count = (p->g->columns * p->g->qubits);

  update_pheremone(p->g, p->p_min, p->p_max, p->p_evap);

  while(elite_score < p->target_score && iter < p->max_runs){
   printf("%d: %f (%d) :[", iter, elite_score, elite_count);
    int*** cand_routes = malloc(p->ants * sizeof(int**));
    double* scores = malloc(p->ants * sizeof(double));
    int* cand_size = malloc(p->ants * sizeof(int));
    int best_ant = 0;
    double best_score = 0.0;
    for(int a = 0; a < p->ants; a++){
      if(first != 0){
        cand_routes[a] = generate_routes(p->g, p->elite_sel_p, p->cooperate_bonus, elite_routes);
      }
      else{
        cand_routes[a] = generate_routes(p->g, -1.0, p->cooperate_bonus, NULL);
      }
      o = make_q_op(p->g, cand_routes[a]);
      scores[a] = mean_square_fidelity(o, d);
      if(scores[a] > best_score){
        best_ant = a;
        best_score = scores[a];
      }
      cand_size[a] = count_op(p->g, cand_routes[a]);
      q_op_free(o);
      if(a < 10){
        printf("%lf, ", scores[a]);
      }
    }
    double sum = 0.0;
    int update = 0;
    double p_min = get_p_min(0.3, 0.3, iter, 500, p->g->qubits);
    update_pheremone(p->g, p_min, p->p_max, p->p_evap);
    for(int a = 0; a < p->ants; a++){
      sum += scores[a];
      if(scores[a] > elite_score - 0.05){
        add_route(p->g, cand_routes[a], scores[a], p->el_rate, 0.0, p->p_max);
      }
      if(scores[a] > elite_score - 0.00001 || (scores[a] > elite_score - 0.00001 && count_op(p->g, cand_routes[a]) < elite_count)){
      //if(mod_score > mod_el_score - 0.0001){
        update = 1;
        if(first != 0){
          free_routes(elite_routes, p->g->qubits);
          elite_routes = cand_routes[a];
          elite_score = scores[a];
          elite_count = count_op(p->g, cand_routes[a]);
        }
        else{
          first = 1;
          elite_routes = cand_routes[a];
          elite_score = scores[a];
          elite_count = count_op(p->g, cand_routes[a]);
        }
      }
      else{
        free_routes(cand_routes[a], p->g->qubits);
      }
    }
    if(first != 0){
      add_route(p->g, elite_routes, elite_score, p->el_rate, p->p_diff, p->p_max);
    }
    free(cand_routes);
    free(scores);
    free(cand_size);
    printf("] - %lf average, %lf best (%lf p_min)\r", sum / (double)p->ants, best_score, p_min);
    q_op* op = make_q_op(p->g, elite_routes);
    elite_score = mean_square_fidelity(op, d);
    q_op_free(op);
    //print_op(p->g, elite_routes);
    if(stagnation < 0){
      stagnation = 0;
      elite_score = 0;
      elite_count = (p->g->columns * p->g->qubits);
      free_routes(elite_routes, p->g->qubits);
      first = 0;
      stag_score = 0.0;
      reset_pheremone(p->g, p->p_min);
    }
    iter++;
  }
  printf("\n");
  //print_op(p->g, elite_routes);
  q_op* op = make_q_op(p->g, elite_routes);
  q_op_print(op);
  q_op_free(op);
  //print_mean_square_fidelity(make_q_op(p->g, elite_routes), d);
  free_routes(elite_routes, p->g->qubits);
  return make_result(iter, elite_score);
}


result* run_e_qap(e_params* p, error_dataset* d){

  int** elite_routesA;
  int** elite_routesB;
  double elite_score = 0.0;

  printf("Init score %lf\n", elite_score);
  int iter = 0;
  int first = 0;

  update_pheremone(p->g1, p->p_min, p->p_max, p->p_evap);
  update_pheremone(p->g2, p->p_min, p->p_max, p->p_evap);

  while(elite_score < p->target_score && iter < p->max_runs){
   printf("%d: %f :[", iter, elite_score);
    int*** cand_routesA = malloc(p->ants * sizeof(int**));
    int*** cand_routesB = malloc(p->ants * sizeof(int**));
    double* scores = malloc(p->ants * sizeof(double));
    int best_ant = 0;
    double best_score = 0.0;
    for(int a = 0; a < p->ants; a++){
      if(first != 0){
        cand_routesA[a] = generate_routes(p->g1, p->elite_sel_p, p->cooperate_bonus, elite_routesA);
        cand_routesB[a] = generate_routes(p->g2, p->elite_sel_p, p->cooperate_bonus, elite_routesB);
      }
      else{
        cand_routesA[a] = generate_routes(p->g1, -1.0, p->cooperate_bonus, NULL);
        cand_routesB[a] = generate_routes(p->g2, -1.0, p->cooperate_bonus, NULL);
      }
      q_op* o1 = make_q_op(p->g1, cand_routesA[a]);
      q_op* o2 = make_q_op(p->g2, cand_routesB[a]);
      scores[a] = mean_square_fidelity_error(o1, o2, d);
      if(scores[a] > best_score){
        best_ant = a;
        best_score = scores[a];
      }
      q_op_free(o1);
      q_op_free(o2);
      if(a < 10){
        printf("%lf, ", scores[a]);
      }
    }
    double sum = 0.0;
    int update = 0;
    double p_min = get_p_min(0.3, 0.3, iter, 500, p->g1->qubits);
    update_pheremone(p->g1, p_min, p->p_max, p->p_evap);
    update_pheremone(p->g2, p_min, p->p_max, p->p_evap);
    for(int a = 0; a < p->ants; a++){
      sum += scores[a];
      if(scores[a] > elite_score - 0.05){
        add_route(p->g1, cand_routesA[a], scores[a], p->el_rate, 0.0, p->p_max);
        add_route(p->g2, cand_routesB[a], scores[a], p->el_rate, 0.0, p->p_max);
      }
      if(scores[a] > elite_score - 0.00001){
      //if(mod_score > mod_el_score - 0.0001){
        update = 1;
        if(first != 0){
          free_routes(elite_routesA, p->g1->qubits);
          free_routes(elite_routesB, p->g1->qubits);
          elite_routesA = cand_routesA[a];
          elite_routesB = cand_routesB[a];
          elite_score = scores[a];
        }
        else{
          first = 1;
          elite_routesA = cand_routesA[a];
          elite_routesB = cand_routesB[a];
          elite_score = scores[a];
        }
      }
      else{
        free_routes(cand_routesA[a], p->g1->qubits);
        free_routes(cand_routesB[a], p->g1->qubits);
      }
    }
    if(first != 0){
      add_route(p->g1, elite_routesA, elite_score, p->el_rate, p->p_diff, p->p_max);
      add_route(p->g2, elite_routesB, elite_score, p->el_rate, p->p_diff, p->p_max);
    }
    free(cand_routesA);
    free(cand_routesB);
    free(scores);
    printf("] - %lf average, %lf best (%lf p_min)\r", sum / (double)p->ants, best_score, p_min);
    q_op* op1 = make_q_op(p->g1, elite_routesA);
    q_op* op2 = make_q_op(p->g2, elite_routesB);
    elite_score = mean_square_fidelity_error(op1, op2, d);
    q_op_free(op1);
    q_op_free(op2);
    iter++;
  }
  printf("\n");
  //print_op(p->g, elite_routes);
  q_op* op1 = make_q_op(p->g1, elite_routesA);
  print_op(p->g1, elite_routesA);
  q_op_print(op1);
  q_op_free(op1);
  q_op* op2 = make_q_op(p->g2, elite_routesB);
  print_op(p->g2, elite_routesB);
  q_op_print(op2);
  q_op_free(op2);
  //print_mean_square_fidelity(make_q_op(p->g, elite_routes), d);
  free_routes(elite_routesA, p->g1->qubits);
  free_routes(elite_routesB, p->g2->qubits);
  return make_result(iter, elite_score);
}

result* make_result(int gens, double best_score){
  result* res = malloc(sizeof(result));
  res->gens = gens;
  res->best_score = best_score;
  return res;
}

void free_result(result* res){
  free(res);
}
