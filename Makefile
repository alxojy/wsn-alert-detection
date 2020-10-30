LDLIBS = -pthread

CC = mpicc
PROGRAM = WSN
DEPS = main.h basestation.h sensornode.h nodecomm.h get_ip_address.h get_mac_address.h
OBJ = main.o basestation.o sensornode.o nodecomm.o get_ip_address.o get_mac_address.o

%.o: %.c $(DEPS)
	$(CC) -c $< $(CFLAGS) $(INCLUDE)

$(PROGRAM): $(OBJ)
	$(CC) $(LDLIBS) -o $@ $^ 

clean:
	rm -f $(PROGRAM) *.o
