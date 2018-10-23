#ifndef EX_H
#define EX_H

#include "QuantumCircuits-C/q_circuit.h"
#include "QuantumCircuits-C/predefined_q.h"
#include "quant.h"
#include <stdio.h>

q_state* r_q();
q_state* r_q_qubits(int qubits);

dataset* epr_pair_dataset();

q_op* generate_qft(int qubits);
dataset* qft_dataset(int qubits, int examples);

dataset* op_dataset(int qubits, q_op* op, int examples);
#endif
