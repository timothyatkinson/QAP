#include "quant.h"
#include "time.h"

int main (void)
{
  srand(time(NULL));
  function* f1 = make_function(q_cZ(), "cZ");
  function* f2 = make_function(q_pauli_X(), "X");
  function* f3 = make_function(q_hadamard(), "Ha");
  function* f4 = make_function(q_cX(), "cX");
  function** f_set = malloc(4 * sizeof(function));
  f_set[0] = f1;
  f_set[1] = f2;
  f_set[2] = f3;
  f_set[3] = f4;
  qap_column* c = make_column(f_set, 4, 4);
  print_column(c);
  //printf("%s\n", f->name);

  qap_graph* graph = make_graph(c, 10, 4);
  free_graph(graph);

  free_column(c);

  ant_map* m = make_ant_map(5, 10);
  free_ant_map(m);
}
