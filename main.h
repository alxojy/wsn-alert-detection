#ifndef MAIN_H
#define MAIN_H

#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "nodecomm.h"

#endif // MAIN_H


#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
    int rank, size, root;
    int i, iteration;

    MPI_Comm comm;
    int dim[2], period[2], reorder, coord[2], id;
    period[0] = 0; period[1] = 0; reorder = 0;

    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    root = size-1;

    if (rank == 0) { // prompts for col
        printf("col:"); // read col
        fflush(stdout);
        scanf("%d", &dim[0]);
        dim[1] = 3;
    } 

    MPI_Bcast(&dim, 2, MPI_INT, 0, MPI_COMM_WORLD); 
    printf("rank %d row %d col %d\n", rank, dim[0], dim[1]);
    MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, &comm); 
}
