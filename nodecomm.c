#include "nodecomm.h"
#include "main.h"

void send_msg(int *msg, int to, int tag) {
    MPI_Request req;
    MPI_Isend(msg, 1, MPI_INT, to, tag, MPI_COMM_WORLD, &req);
}

void receive_msg(int *holder, int from, int tag) {
    int flag;
    MPI_Iprobe(from, tag, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
    if (flag) { // there exists a message
        MPI_Recv(holder, 1, MPI_INT, from, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        break;
    }
}