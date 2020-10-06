// cartesian topology https://www.codingame.com/playgrounds/47058/have-fun-with-mpi-in-c/mpi-process-topologies
// unicast
#include "main.h"

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
    int sensor_reading;
    enum boolean { false = 0, true = 1 } simulation; 
    float simul_duration = 0.01; // simulation duration: 4 milliseconds
    int left_reading = -999, right_reading = -999, up_reading = -999, down_reading = -999;

    // params for cartesian topology
    MPI_Comm comm;
    int dim[2], period[2], reorder, coord[2], id; // dim = [col, row]
    period[0] = 0; period[1] = 0; reorder = 0;

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
                    send_msg(&sensor_reading, left, NEIGHBOUR_TAG);
                }

                if (right != MPI_PROC_NULL) {// right exists
                    printf("rank %d right\n", rank);
                    // send to right to request for reading
                    send_msg(&sensor_reading, right, NEIGHBOUR_TAG);
                }

                if (up != MPI_PROC_NULL) {// up exists
                    printf("rank %d up\n", rank);
                    // send up to request for reading
                    send_msg(&sensor_reading, up, NEIGHBOUR_TAG);
                }

                if (down != MPI_PROC_NULL) {// down exists
                    printf("rank %d down\n", rank);
                    // send down to request for reading
                    send_msg(&sensor_reading, down, NEIGHBOUR_TAG);
                }
            }

            MPI_Barrier(comm);

            if (right != MPI_PROC_NULL) {
                receive_msg(&right_reading, right, NEIGHBOUR_TAG);
                if (right_reading > -1)
                printf("rank %d neighbour r %d \n", rank, right_reading);
                if (sensor_reading - SENSOR_THRESHOLD < right_reading && right_reading < sensor_reading + SENSOR_THRESHOLD) {
                    printf("over t. rank %d right\n", rank);
                    send_msg(&sensor_reading, right, READING_TAG); // send reading to right
                }
            }

            if (left != MPI_PROC_NULL) {
                receive_msg(&left_reading, left, NEIGHBOUR_TAG);
                if (left_reading > -1)
                printf("rank %d neighbour l %d \n", rank, left_reading);
                if (sensor_reading - SENSOR_THRESHOLD < left_reading && left_reading < sensor_reading + SENSOR_THRESHOLD) {
                    printf("over t. rank %d left\n", rank);
                    send_msg(&sensor_reading, left, READING_TAG); // send reading to left
                }
            }

            if (down != MPI_PROC_NULL) {
                receive_msg(&down_reading, down, NEIGHBOUR_TAG);
                if (down_reading > -1)
                printf("rank %d neighbour d %d \n", rank, down_reading);
                if (sensor_reading - SENSOR_THRESHOLD < down_reading && down_reading < sensor_reading + SENSOR_THRESHOLD) {
                    printf("over t. rank %d down\n", rank);
                    send_msg(&sensor_reading, down, READING_TAG); // send reading to down
                }
            }

            if (up != MPI_PROC_NULL) {
                receive_msg(&up_reading, up, NEIGHBOUR_TAG);
                if (up_reading > -1)
                printf("rank %d neighbour u %d \n", rank, up_reading);
                if (sensor_reading - SENSOR_THRESHOLD < up_reading && up_reading < sensor_reading + SENSOR_THRESHOLD) {
                    printf("over t. rank %d up\n", rank);
                    send_msg(&sensor_reading, up, READING_TAG); // send reading to right
                }
            }
            
            MPI_Barrier(comm);

            if (right != MPI_PROC_NULL) {
                receive_msg(&right_reading, right, READING_TAG);
                if (right_reading > -1)
                printf("!! rank %d neighbour r %d \n", rank, right_reading);
            }

            if (left != MPI_PROC_NULL) {
                receive_msg(&left_reading, left, READING_TAG);
                if (left_reading > -1)
                printf("!! rank %d neighbour l %d \n", rank, left_reading);
            }

            if (up != MPI_PROC_NULL) {
                receive_msg(&up_reading, up, READING_TAG);
                if (up_reading > -1)
                printf("!! rank %d neighbour u %d \n", rank, up_reading);
            }

            if (down != MPI_PROC_NULL) {
                receive_msg(&down_reading, down, READING_TAG);
                if (down_reading > -1)
                printf("!! rank %d neighbour d %d \n", rank, down_reading);
            }

            MPI_Barrier(comm);

        }
        else if (rank == root) {
        }
    }

    MPI_Finalize();
    return; // exit
}
