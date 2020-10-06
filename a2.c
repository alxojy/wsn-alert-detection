// cartesian topology https://www.codingame.com/playgrounds/47058/have-fun-with-mpi-in-c/mpi-process-topologies
// allgatherv

#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_ITERATIONS 1
#define SENSOR_THRESHOLD 5
#define MIN_READING 0
#define MAX_READING 10
#define TOLERANCE_RANGE 3

int main(int argc, char *argv[]) {
    int rank, size, root; // root = base station
    int NEIGHBOUR_TAG = 0, BASE_TAG = 1, READING_TAG = 2;
    int i, iteration, row, col;
    int left = -1, right = -1, up = -1, down = -1; // neighbours
    int left_req = -1, right_req = -1, up_req = -1, down_req = -1;
    int sensor_reading;
    int neighbour_readings[4] = {-1, -1, -1, -1}; // index 0 = left, 1 = right, 2 = up, 3 = down
    enum boolean { false = 0, true = 1 } simulation; 
    float simul_duration = 0.01; // simulation duration: 4 milliseconds
    int probe_output[4] = {0, 0, 0, 0};

    // params for cartesian topology
    MPI_Comm comm;
    int dim[2], period[2], reorder, coord[2], id; // dim = [col, row]
    period[0] = 0; period[1] = 0; reorder = 0;

    MPI_Status status[4];
    MPI_Request request[4];

    MPI_Init(&argc, &argv); 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	MPI_Comm_size(MPI_COMM_WORLD, &size);
    root = size-1;

    if (rank == root) { // base station prompts for row & col input
        /*printf("row: "); // read row
        fflush(stdout);
        scanf("%d", dim[0]);
        printf("col: "); // read col
        fflush(stdout);
        scanf("%d", dim[1]);*/
        dim[0] = 3; dim[1] = 3;
    }

    MPI_Bcast(dim, 2, MPI_INT, root, MPI_COMM_WORLD); // broadcast number of rows & columns
    
    if (size < (dim[0]*dim[1])+1) { // insufficient number of processes for 2d grid & base station
        printf("Insufficient number of processes. Try smaller values of row & col and larger values of processes");
    }   
    //printf("%d %d", dim[0], dim[1]);
    
    MPI_Barrier(MPI_COMM_WORLD); // sync the nodes

    MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, &comm); // initialise cartesian topology 

    for (iteration = 0; iteration < NUM_ITERATIONS; iteration++) { // run NUM_ITERATIONS times

        if (rank != root) { // sensor in 2d grid
            int source;
            MPI_Cart_coords(comm, rank, 2, coord);
            MPI_Cart_shift(comm, 1, -1, &source, &left); 
            MPI_Cart_shift(comm, 1, 1, &source, &right); 
            
            srandom(time(NULL) | rank); // seed
            sensor_reading = (random() % (MAX_READING - MIN_READING + 1)) + MIN_READING; // random sensor reading
            printf("rank %d reading %d \n", rank, sensor_reading);

            int *arr = malloc(4 * sizeof(int)); // array to store 

            if (sensor_reading > SENSOR_THRESHOLD) {
                int ct[4] = {1,1,1,1};
                int disp[4] = {0,1,2,3};
                MPI_Neighbor_allgatherv(&sensor_reading, 1, MPI_INT, arr, ct, disp, MPI_INT, comm);
                for (i = 0; i < 4; i++) {
                    printf("rank %d more case %d\n", rank, arr[i]);
                }
            }

            else {
                int ct[4] = {1,1,1,1};
                int disp[4] = {0,0,0,0};
                MPI_Neighbor_allgatherv(&sensor_reading, 1, MPI_INT, arr, ct, disp, MPI_INT, comm);
                printf("rank %d %d \n", rank, arr[0]);
            }

            free(arr);

            MPI_Barrier(comm);

        }
        else if (rank == root) {
        }
    }

    MPI_Finalize();
    return; // exit
}
