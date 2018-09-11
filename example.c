#include "example.h"

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

dataset* epr_pair_dataset(){
  q_state** X = malloc(4 * sizeof(q_state));
  q_state** Y = malloc(4 * sizeof(q_state));

  q_state* z = q_zero();
  q_state* o = q_one();

  X[0] = q_state_tensor(z, z);
  X[1] = q_state_tensor(z, o);
  X[2] = q_state_tensor(o, z);
  X[3] = q_state_tensor(o, o);

  q_state_free(z);
  q_state_free(o);

  q_op* H = q_hadamard();
  q_op* W = q_identity(1);
  q_op* first = q_op_tensor(H, W);
  q_op* cnot = q_cX();

  q_op* bp = q_op_multiply(cnot, first);

  for(int i = 0; i < 4; i++){
    q_state_normalize(X[i]);
    Y[i] = apply_qop(bp, X[i]);
    q_state_normalize(Y[i]);
    printf("\n\n");
    q_state_print(X[i]);
    printf("\n=>\n");
    q_state_print(Y[i]);
  }
  q_op_free(H);
  q_op_free(W);
  q_op_free(first);
  q_op_free(cnot);
  q_op_free(bp);

  dataset* d = make_dataset(4, X, Y);
  return d;
}


q_op* generate_qft(int qubits){
  q_op* op = q_op_calloc(qubits);
  int n = pow(2, qubits);
  gsl_complex w;
  double x = 2 * M_PI / n;
  GSL_SET_COMPLEX(&w, cos(x), sin(x));
  for(int i = 0; i < n; i++){
    for(int j = 0; j < n; j++){
      double mul = i * j;
      gsl_matrix_complex_set(op->matrix, i, j, gsl_complex_div_real(gsl_complex_pow_real(w, mul), sqrt(n)));
    }
  }
  q_op_print(op);
  return op;
}

dataset* qft_dataset(int qubits, int examples){
  q_op* qft = generate_qft(qubits);
  q_state** X = malloc(examples * sizeof(q_state*));
  q_state** Y = malloc(examples * sizeof(q_state*));

  for(int i = 0; i < examples; i++){
    X[i] = r_q_qubits(qubits);
    Y[i] = apply_qop(qft, X[i]);
    q_state_normalize(X[i]);
    q_state_normalize(Y[i]);
    if(i < 10){
      printf("\n\n");
      q_state_print(X[i]);
      printf("\n=>\n");
      q_state_print(Y[i]);
    }
  }

  return make_dataset(examples, X, Y);
}
