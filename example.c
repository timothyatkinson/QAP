#include "example.h"

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
    //printf("\n\n");
    q_state_print(X[i]);
    //printf("\n=>\n");
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

q_op* generate_gdo(int qubits){
  q_op* op = q_op_calloc(qubits);
  int n2 = pow(2, qubits);
  double entry = 2.0 / (double)n2;
  double identry = entry - 1.0;
  gsl_complex z;
  gsl_complex o;
  GSL_SET_COMPLEX(&z, entry, 0.0);
  GSL_SET_COMPLEX(&o, identry, 0.0);
  for(int i = 0; i < n2; i++){
    for(int j = 0; j < n2; j++){
      if(i == j){
        gsl_matrix_complex_set(op->matrix, i, j, o);
      }
      else{
        gsl_matrix_complex_set(op->matrix, i, j, z);
      }
    }
  }
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
      //printf("\n\n");
      q_state_print(X[i]);
      //printf("\n=>\n");
      q_state_print(Y[i]);
    }
  }

  return make_dataset(examples, X, Y);
}

dataset* gdo_dataset(int qubits, int examples){
  q_op* gdo = generate_gdo(qubits);
  q_op_print(gdo);
  q_state** X = malloc(examples * sizeof(q_state*));
  q_state** Y = malloc(examples * sizeof(q_state*));

  for(int i = 0; i < examples; i++){
    X[i] = r_q_qubits(qubits);
    Y[i] = apply_qop(gdo, X[i]);
    q_state_normalize(X[i]);
    q_state_normalize(Y[i]);
    if(i < 10){
      //printf("\n\n");
      q_state_print(X[i]);
      //printf("\n=>\n");
      q_state_print(Y[i]);
    }
  }

  return make_dataset(examples, X, Y);
}

dataset* toffoli_dataset(int examples) {
    return make_basic_dataset(q_toffoli(), TOFFOLI_QUBITS, examples);
}

dataset* fredkin_dataset(int examples) {
    return make_basic_dataset(q_fredkin(), FREDKIN_QUBITS, examples);
}

dataset* margolus_dataset(int examples) {
    return make_basic_dataset(q_margolus(), MARGOLUS_QUBITS, examples);
}

dataset* op_dataset(int qubits, q_op* op, int examples){
  q_state** X = malloc(examples * sizeof(q_state*));
  q_state** Y = malloc(examples * sizeof(q_state*));

  for(int i = 0; i < examples; i++){
    X[i] = r_q_qubits(qubits);
    Y[i] = apply_qop(op, X[i]);
    q_state_normalize(X[i]);
    q_state_normalize(Y[i]);
    if(i < 10){
      //printf("\n\n");
      q_state_print(X[i]);
      //printf("\n=>\n");
      q_state_print(Y[i]);
    }
  }

  return make_dataset(examples, X, Y);
}

error_dataset* bit_flip_dataset(int examples){
  q_state** X = malloc(examples * sizeof(q_state*));
  q_state*** Y = malloc(examples * sizeof(q_state**));
  q_op** error_functions = malloc(4 * sizeof(q_op*));

  q_op* x = q_pauli_X();
  q_op* W = q_identity(1);
  q_op* A = q_op_tensor(x, W);
  q_op* B = q_op_tensor(W, x);
  q_op* C = q_op_tensor(W, W);
  q_op* D = q_op_tensor(x, x);
  q_op* E = q_op_tensor(W, A);
  q_op* F = q_op_tensor(W, B);
  q_op* G = q_op_tensor(W, C);
  q_op* H = q_op_tensor(W, D);
  q_state* z = q_zero();
  for(int i = 0; i < examples; i++){
    q_state* xi = r_q_qubits(1);
    q_state* a1 = q_state_tensor(xi, z);
    q_state* a2 = q_state_tensor(a1, z);

    q_state_free(xi);
    q_state_free(a1);
    X[i] = a2;
    Y[i] = malloc(4 * sizeof(q_state*));
    Y[i][0] = apply_qop(E, a2);
    Y[i][1] = apply_qop(F, a2);
    Y[i][2] = apply_qop(G, a2);
    Y[i][3] = apply_qop(H, a2);
    if(i < 10){
      //printf("\n\n");
      q_state_print(X[i]);
      //printf("\n=>\n");
      q_state_print(Y[i][0]);
      q_state_print(Y[i][1]);
      q_state_print(Y[i][2]);
      q_state_print(Y[i][3]);
    }
  }
  q_state_free(z);
  q_op_free(E);
  q_op_free(F);
  q_op_free(G);
  q_op_free(H);
  error_functions[0] = q_op_tensor(A, W);
  //printf("Error 1\n");
  q_op_print(error_functions[0]);
  error_functions[1] = q_op_tensor(B, W);
  //printf("Error 2\n");
  q_op_print(error_functions[1]);
  error_functions[2] = q_op_tensor(W, B);
  //printf("Error 3\n");
  q_op_print(error_functions[2]);
  error_functions[3] = q_op_tensor(W, C);
  //printf("Error 4\n");
  q_op_print(error_functions[3]);

  q_op_free(x);
  q_op_free(W);
  q_op_free(A);
  q_op_free(B);
  q_op_free(C);

  return make_error_dataset(examples, X, Y, error_functions, 4);
}

error_dataset* phase_flip_dataset(int examples){
  q_state** X = malloc(examples * sizeof(q_state*));
  q_state*** Y = malloc(examples * sizeof(q_state**));
  q_op** error_functions = malloc(4 * sizeof(q_op*));

  q_op* x = q_pauli_Z();
  q_op* W = q_identity(1);
  q_op* A = q_op_tensor(x, W);
  q_op* B = q_op_tensor(W, x);
  q_op* C = q_op_tensor(W, W);
  q_op* D = q_op_tensor(x, x);
  q_op* E = q_op_tensor(W, A);
  q_op* F = q_op_tensor(W, B);
  q_op* G = q_op_tensor(W, C);
  q_op* H = q_op_tensor(W, D);
  q_state* z = q_zero();
  for(int i = 0; i < examples; i++){
    q_state* xi = r_q_qubits(1);
    q_state* a1 = q_state_tensor(xi, z);
    q_state* a2 = q_state_tensor(a1, z);

    q_state_free(xi);
    q_state_free(a1);
    X[i] = a2;
    Y[i] = malloc(4 * sizeof(q_state*));
    Y[i][0] = apply_qop(E, a2);
    Y[i][1] = apply_qop(F, a2);
    Y[i][2] = apply_qop(G, a2);
    Y[i][3] = apply_qop(H, a2);
    if(i < 10){
      //printf("\n\n");
      q_state_print(X[i]);
      //printf("\n=>\n");
      q_state_print(Y[i][0]);
      q_state_print(Y[i][1]);
      q_state_print(Y[i][2]);
      q_state_print(Y[i][3]);
    }
  }
  q_state_free(z);
  q_op_free(E);
  q_op_free(F);
  q_op_free(G);
  q_op_free(H);
  error_functions[0] = q_op_tensor(A, W);
  //printf("Error 1\n");
  q_op_print(error_functions[0]);
  error_functions[1] = q_op_tensor(B, W);
  //printf("Error 2\n");
  q_op_print(error_functions[1]);
  error_functions[2] = q_op_tensor(W, B);
  //printf("Error 3\n");
  q_op_print(error_functions[2]);
  error_functions[3] = q_op_tensor(W, C);
  //printf("Error 4\n");
  q_op_print(error_functions[3]);

  q_op_free(x);
  q_op_free(W);
  q_op_free(A);
  q_op_free(B);
  q_op_free(C);

  return make_error_dataset(examples, X, Y, error_functions, 4);
}
