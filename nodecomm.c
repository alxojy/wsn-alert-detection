#include "nodecomm.h"
#include "main.h"

// functions are used by sensor nodes for intercommunication between neighbouring nodes

void send_msg(int *msg, int to, int tag) { // send message to 'to' process
    MPI_Request req;
    MPI_Isend(msg, 1, MPI_INT, to, tag, MPI_COMM_WORLD, &req);
}

void receive_msg(int *holder, int from, int tag) { // receive message from 'from' process
    int flag, i;
    for (i = 0; i < 5; i++) { // probe more than once because flag might report false reading
        MPI_Iprobe(from, tag, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE); // check for incoming msg
        if (flag) { // there exists a message
            MPI_Recv(holder, 1, MPI_INT, from, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            break;
        }
    }
}
