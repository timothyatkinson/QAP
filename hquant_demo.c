#include "hquant.h"
#include "time.h"
#include "example.h"

int main (void)
{
  srand(time(NULL));
  printf("Hello World!\n");
  function** core = core_fset();
  qap_column* col = generate_column(core, 5, 2);
  qap_graph* gr = make_graph(col, 3, 0.01);
  print_graph(gr);
  print_ant_map(gr);
  int* route = generate_route(gr);
  printf("Route: \n");
  for(int i = 0; i < 3; i++){
    printf("%d\n", route[i]);
  }
  lay_pheremone(gr, route, 0.8, 1.0, 0.5);
  print_ant_map(gr);
  free(route);
  route = generate_route(gr);
  printf("Route 2: \n");
  for(int i = 0; i < 3; i++){
    printf("%d\n", route[i]);
  }
  q_op* op = route_to_op(route, gr);
  q_op_print(op);
  q_op_free(op);
  free(route);
  free_graph(gr);
  col = generate_column(ccore_fset(), 11, 3);
  gr = make_graph(col, 20, 0.01);
  print_graph(gr);
  dataset* d = qft_dataset(3, 100);
  params* par = default_params(gr, d);
  par->p_min = 0.01;
  par->p_max = 5.0;
  par->diffusion = 0.0;
  par->p_init = 10.0;
  par->ants = 50;
  par->evap_rate = 0.1;
  result** res = malloc(100 * sizeof(result*));
  for(int i = 0; i < 100; i++){
    res[i] = run_qap(par);
  }
  for(int i = 0; i < 100; i++){
    printf("%d, %lf, \n", res[i]->iter, res[i]->route_score);
  }
  //print_ant_map(gr);
  free_params(par);
  free_fset(core, 5);
}
