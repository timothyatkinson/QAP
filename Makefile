OBJECTS := QuantumCircuits-C/*c quant.c example.c $(main)
CC=gcc

CFLAGS = -O2 -lgsl -lgslcblas -lm -g

default:	$(OBJECTS)
		$(CC) $(OBJECTS) $(CFLAGS) -o $(output)

clean:
		rm $(output)
