OBJECTS := QuantumCircuits-C/*c quant.c example.c demo_utils.c demo_fredkin.c $(main)
CC=gcc
output := demo_fredkin

CFLAGS = -O2 -lgsl -lgslcblas -lm -g

default:	$(OBJECTS)
		$(CC) $(OBJECTS) $(CFLAGS) -o $(output)

clean:
		rm $(output)
