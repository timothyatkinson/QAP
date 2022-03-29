#ifndef EX_H
#define EX_H

#include "QuantumCircuits-C/q_circuit.h"
#include "QuantumCircuits-C/predefined_q.h"
#include "quant.h"
#include <stdio.h>

dataset* epr_pair_dataset();

q_op* generate_qft(int qubits);
q_op* generate_gdo(int qubits);
dataset* qft_dataset(int qubits, int examples);
dataset* gdo_dataset(int qubits, int examples);

dataset* op_dataset(int qubits, q_op* op, int examples);
error_dataset* bit_flip_dataset(int examples);
error_dataset* phase_flip_dataset(int examples);
#endif
