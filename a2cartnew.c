// cartesian topology https://www.codingame.com/playgrounds/47058/have-fun-with-mpi-in-c/mpi-process-topologies
// https://janth.home.xs4all.nl/MPIcourse/PDF/07_MPI_Topology.pdf
// https://www.asc.ohio-state.edu/physics/ntg/780/osc/mpi_day4.pdf

// finally it works!! 

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
    int flag[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int num_req = 0, num_probe = 0;

    // params for cartesian topology
    MPI_Comm comm;
    int dim[2], period[2], reorder, coord[2], id; // dim = [col, row]
    period[0] = 0; period[1] = 0; reorder = 0;

    MPI_Status status[8];
    MPI_Request request[8];

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
            MPI_Cart_shift(comm, 0, -1, &source, &up);
            MPI_Cart_shift(comm, 0, 1, &source, &down);
            
            srandom(time(NULL) | rank); // seed
            sensor_reading = (random() % (MAX_READING - MIN_READING + 1)) + MIN_READING; // random sensor reading
            printf("rank %d reading %d \n", rank, sensor_reading);

            if(sensor_reading > SENSOR_THRESHOLD) {

                if (left != MPI_PROC_NULL) { // left exists 
                    printf("rank %d left\n", rank);
                    // send to left to request for reading
                    MPI_Isend(&sensor_reading, 1, MPI_INT, left, NEIGHBOUR_TAG, MPI_COMM_WORLD, &request[num_req++]);
                }

                if (right != MPI_PROC_NULL) {// right exists
                    printf("rank %d right\n", rank);
                    // send to right to request for reading
                    MPI_Isend(&sensor_reading, 1, MPI_INT, right, NEIGHBOUR_TAG, MPI_COMM_WORLD, &request[num_req++]);
                }

                if (up != MPI_PROC_NULL) {// right exists
                    printf("rank %d up\n", rank);
                    // send up to request for reading
                    MPI_Isend(&sensor_reading, 1, MPI_INT, up, NEIGHBOUR_TAG, MPI_COMM_WORLD, &request[num_req++]);
                }

                if (down != MPI_PROC_NULL) {// right exists
                    printf("rank %d down\n", rank);
                    // send down to request for reading
                    MPI_Isend(&sensor_reading, 1, MPI_INT, down, NEIGHBOUR_TAG, MPI_COMM_WORLD, &request[num_req++]);
                }
            }

            MPI_Barrier(comm);

            if (right != MPI_PROC_NULL) {
                // iprobe to check for incoming messages
                MPI_Iprobe(right, NEIGHBOUR_TAG, MPI_COMM_WORLD, &flag[num_probe], MPI_STATUS_IGNORE);
                if (flag[num_probe++]) { // there exists a message
                    // receive message from right requesting for reading
                    MPI_Recv(&right_req, 1, MPI_INT, right, NEIGHBOUR_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    printf("rank %d neighbour r %d \n", rank, right_req);

                    // within range so send sensor reading to node that requested it
                    if (sensor_reading - SENSOR_THRESHOLD < right_req && right_req < sensor_reading + SENSOR_THRESHOLD) {
                        printf("over t. rank %d right\n", rank);
                        MPI_Isend(&sensor_reading, 1, MPI_INT, right, READING_TAG, MPI_COMM_WORLD, &request[num_req++]); // send reading to right
                    }
                }
            }

            if (left != MPI_PROC_NULL) {
                // iprobe to check for incoming messages
                MPI_Iprobe(left, NEIGHBOUR_TAG, MPI_COMM_WORLD, &flag[num_probe], MPI_STATUS_IGNORE);
                if (flag[num_probe++]) { // there exists a message
                    // receive message from left requesting for reading
                    MPI_Recv(&left_req, 1, MPI_INT, left, NEIGHBOUR_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    printf("rank % d neighbour l %d \n", rank, left_req);

                    // within range so send sensor reading to node that requested it
                    if (sensor_reading - SENSOR_THRESHOLD < left_req && left_req < sensor_reading + SENSOR_THRESHOLD) {
                        printf("over t. rank %d left\n", rank);
                        MPI_Isend(&sensor_reading, 1, MPI_INT, left, READING_TAG, MPI_COMM_WORLD, &request[num_req++]); // send reading to left
                    }
                }
            }

            if (down != MPI_PROC_NULL) {
                // iprobe to check for incoming messages
                MPI_Iprobe(down, NEIGHBOUR_TAG, MPI_COMM_WORLD, &flag[num_probe], MPI_STATUS_IGNORE);
                if (flag[num_probe++]) { // there exists a message
                    // receive message from down requesting for reading
                    MPI_Recv(&down_req, 1, MPI_INT, down, NEIGHBOUR_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    printf("rank % d neighbour d %d \n", rank, down);

                    // within range so send sensor reading to node that requested it
                    if (sensor_reading - SENSOR_THRESHOLD < down_req && down_req < sensor_reading + SENSOR_THRESHOLD) {
                        printf("over t. rank %d d\n", rank);
                        MPI_Isend(&sensor_reading, 1, MPI_INT, down, READING_TAG, MPI_COMM_WORLD, &request[num_req++]); // send reading to down
                    }
                }
            }

            if (up != MPI_PROC_NULL) {
                // iprobe to check for incoming messages
                MPI_Iprobe(up, NEIGHBOUR_TAG, MPI_COMM_WORLD, &flag[num_probe], MPI_STATUS_IGNORE);
                if (flag[num_probe++]) { // there exists a message
                    // receive message from up requesting for reading
                    MPI_Recv(&up_req, 1, MPI_INT, up, NEIGHBOUR_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    printf("rank % d neighbour u %d \n", rank, up_req);

                    // within range so send sensor reading to node that requested it
                    if (sensor_reading - SENSOR_THRESHOLD < up_req && up_req < sensor_reading + SENSOR_THRESHOLD) {
                        printf("over t. rank %d u\n", rank);
                        MPI_Isend(&sensor_reading, 1, MPI_INT, up, READING_TAG, MPI_COMM_WORLD, &request[num_req++]); // send reading to up
                    }
                }
            }
            
            MPI_Barrier(comm);

            if (right != MPI_PROC_NULL) {
                for (i = 0; i < 5; i++) { // single call iprobe might not suffice
                    // https://stackoverflow.com/questions/20999299/why-does-mpi-iprobe-return-false-when-message-has-definitely-been-sent
                    MPI_Iprobe(right, READING_TAG, MPI_COMM_WORLD, &flag[num_probe], MPI_STATUS_IGNORE); 
                    if (flag[num_probe++]) { // there exists a message
                        MPI_Recv(&neighbour_readings[0], 1, MPI_INT, right, READING_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive requested reading
                        printf("!! rank %d neighbour r %d \n", rank, neighbour_readings[0]);
                    }
                }
            }

            if (left != MPI_PROC_NULL) {
                for (i = 0; i < 5; i++) { // single call iprobe might not suffice 
                    // iprobe to check for incoming messages
                    MPI_Iprobe(left, READING_TAG, MPI_COMM_WORLD, &flag[num_probe], MPI_STATUS_IGNORE);
                    if (flag[num_probe++]) { // there exists a message
                        MPI_Recv(&neighbour_readings[1], 1, MPI_INT, left, READING_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive requested reading
                        printf("!! rank % d neighbour l %d \n", rank, neighbour_readings[1]);
                    }
                }
            }

            if (up != MPI_PROC_NULL) {
                for (i = 0; i < 5; i++) { // single call iprobe might not suffice 
                    // iprobe to check for incoming messages
                    MPI_Iprobe(up, READING_TAG, MPI_COMM_WORLD, &flag[num_probe], MPI_STATUS_IGNORE);
                    if (flag[num_probe++]) { // there exists a message
                        MPI_Recv(&neighbour_readings[2], 1, MPI_INT, up, READING_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive requested reading
                        printf("!! rank % d neighbour u %d \n", rank, neighbour_readings[2]);
                    }
                }
            }

            if (down != MPI_PROC_NULL) {
                for (i = 0; i < 5; i++) { // single call iprobe might not suffice 
                    // iprobe to check for incoming messages
                    MPI_Iprobe(down, READING_TAG, MPI_COMM_WORLD, &flag[num_probe], MPI_STATUS_IGNORE);
                    if (flag[num_probe++]) { // there exists a message
                        MPI_Recv(&neighbour_readings[3], 1, MPI_INT, down, READING_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive requested reading
                        printf("!! rank % d neighbour d %d \n", rank, neighbour_readings[3]);
                    }
                }
            }

            MPI_Barrier(comm);

        }
        else if (rank == root) {
        }
    }

    MPI_Finalize();
    return; // exit
}
