#include "qap.h"
#include "QuantumCircuits-C/q_circuit.h"


/**rand_double - RAND DOUBLE
  *Computes a random double between 0 and 1 using C's inbuilt RNG
*/
double rand_double();

/**rand_double - RAND double
  *Computes a random double between 0 and 1 using C's inbuilt RNG
*/
double rand_double(){
  return (double)rand() / (double)RAND_MAX;
}

int get_rows(op** function_set, int functions, int qubits);

//Works out how many total occurrences of each function is needed to give maximum coverage. E.g. if there are 4 qubits, we can use at most 1 of each 3-qubit gates but 2 of each 2-qubit gates
int get_rows(op** function_set, int functions, int qubits){
  int count = 0;
  for(int i = 0; i < functions; i++){
    int arity = function_set[i]->qop->qubits;
    int mul = qubits/arity;
    count += (mul * arity);
  }
  return count;
}

qap_result* quantum_ant_programming(params* ant_params){

  int qubits = ant_params->train_dataset->qubits;
  int ants = ant_params->ants;
  int depth = ant_params->depth;
  int rows = get_rows(ant_params->function_set, ant_params->functions, int qubits);
  bool size_matters = ant_params->size_matters;
  double target_score = ant_params->target_score;
  double target_size = ant_params->target_size;
  int max_steps = ant_params->max_steps;

  //scores are between 0.0 (best) and 1.0 (worst).
  double best_score = 1.0;
  //worst possible size is depth * rows
  int best_size = depth * rows;

  //Elite ant is stored by indexed decisions
  int** elite = malloc(qubits * sizeof(int*);
  for(int i = 0; i < qubits; i++){
    elite[i] = malloc(depth * sizeof(int));
  }

  //Construct a pheromone map for each qubit.
  double*** pheromone_map = malloc(qubits * sizeof(double**));
  for(int i = 0; i < qubits; i++){
    pheremone_map[i] = malloc(depth * sizeof(double*));
    for(int j = 0; j < rows; j++){
      pheremone_map[i][j] = calloc(rows * sizeof(double));
      for(int k = 0; k < rows; k++){
        pheremone_map[i][j][k] = ant_params->p_min;
      }
    }
  }

  int steps = 0;
  while(steps < max_steps && best_score > target_score && best_size > target_size){

    double*** update_map = malloc(qubits * sizeof(double**));
    for(int i = 0; i < qubits; i++){
      update_map[i] = malloc(depth * sizeof(double*));
      for(int j = 0; j < rows; j++){
        update_map[i][j] = calloc(rows * sizeof(double));
      }
    }

    //Each qubit is an ant so we have qubits * ants total ants

    for(int a = 0; a < ants; a++){

      //Let the #qubit ants cooperatively find a qcircuit
      int** ants_route = malloc(qubits * sizeof(int*));

      int** blocked = malloc(depth * sizeof(int*));

      for(int i  = 0; i < depth; i++){
        blocked[i] = calloc(rows * sizeof(int));
      }

      for(int i = 0; i < qubits; i++){

        ants_route[i] = malloc(depth * sizeof(int));

        //Go through each column.
        for(int j = 0; j< depth; j++){

          //Get total pheremone
          double total_pheremone = 0.0;
          for(int k = 0; k < rows; k++){
            if(blocked[j][k] != 1){
              total_pheremone += pheremone_map[i][j][k];
            }
          }

          double p = rand_double() * total_pheremone;
          double sum = 0.0;

          //Go through each row
          for(int k = 0; k < rows; k++){
            if(blocked[j][k] != 1){
              sum+= pheremone_map[i][j][k];
              if(sum >= p){

                //Pick that path
                ants_route[i][j] = k;

                //block that path for the next ant
                blocked[j][k] = 1;
                break;
              }
            }
          }
        }
      }
    }



    steps += 1;
  }


}

q_circuit* get_q_circuit(int** ant_map, int qubits, int rows, int depth, op* function_set, int functions){
  q_circuit* ret = malloc(sizeof(q_circuit));

  ret->circuit = malloc((1 + (depth * 2)) * sizeof(op*));
  int[qubits] loc;
  for(int i = 0; i < qubits; i++){
    loc[i] = i;
  }
  for(int i = 0; i < depth; i = i + 2){

    ret->circuit[i] = malloc(sizeof(op));
    ret->circuit[i + 1] = malloc(qubits * sizeof(op));
    int op_c = 0;

    int[qubits] new_loc;

    int c = 0;
    for(int j = 0; j < functions; j++){
      int num = function_set[j]->qop->qubits / qubits;

      int present = 0;

      for(int k = 0; k < function_set[j]->qop->qubits; k++){

        int present = 0;
        for(int q = 0; q < qubits; q++){
          if(ant_map[q][i] == c){
            new_loc[q] = c;
            present += 1;
            break;
          }
        }

        c++;
      }
      if (present == function_set[j]->qop->qubits){
        circuit[i][op_c] = function_set[j];
      }
      else{
        for(int k = 0; k < present; k++){
          //Wire should always be index 0
          ret->circuit[i][op_c] = function_set[0];
        }
      }
    }

    int[qubits] new_loc_index;
    for(int q = 0; q < qubits; q++){

      for(int j )

    }
  }

}

double fidelity_fitness(int** ant_map, op* function_set, int functions, int qubits, dataset* dataset){
  return 0.0;
}


qap_graph* construct_graph(op* function_set, int functions, int layers, int qubits){
  qap_graph* graph = malloc(sizeof(graph));
  graph->layers = malloc(layers * sizeof(qap_node**));
  for(int i = 0; i < layers; i++){
    int f = get_rows(function_set, functions, qubits);
    graph->layers[i] = malloc(f * sizeof(qap_node*));
    int nodes = 0;;
    for(int j = 0; j < functions; j++){
      int arity = function_set[j]->qop->qubits;
      int mul = qubits/arity;
      for(int k = 0; k < mul; k++){
        graph->layers[i][nodes] = malloc(sizeof(qap_node));
        graph->layers[i][nodes]->index = nodes;
        graph->layers[i][nodes]->op = function_set[j];
        if(i != layers - 1){
          graph->layers[i][nodes]->out_connections = malloc(arity * sizeof(qap_connection**));
          for(int q = 0; q < arity; q++){
            graph->layers[i][nodes]->out_connections[q] = malloc(f * sizeof(qap_connection*));
          }
        }
        if(layers != 0){
          graph->layers[i][nodes]->in_connections = malloc(arity * sizeof(qap_connection**));
          for(int q = 0; q < arity; q++){
            graph->layers[i][nodes]->in_connections[q] = malloc(f * sizeof(qap_connection*));
            for(int c = 0; c < functions; c++){
              graph->layers[i][nodes]->in_connections[q][c] = malloc(sizeof(qap_connection);
              int arity2 = function_set[j]->qop->qubits;
              int mul2 = qubits/arity;
              int nodes2 = 0;
              for(int j2 = 0; j2 < mul2; j++){
                
              }
              graph->layers[i][nodes]->in_connections[q][c]
            }
          }
        }
        nodes++;
      }
    }
  }
}
