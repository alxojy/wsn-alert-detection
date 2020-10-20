CC = mpicc
PROGRAM = WSN
DEPS = main.h basestation.h sensornode.h nodecomm.h
OBJ = main.o basestation.o sensornode.o nodecomm.o 

%.o: %.c $(DEPS)
	$(CC) -c $< $(CFLAGS) $(INCLUDE)

$(PROGRAM): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(PROGRAM) *.o