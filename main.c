// alxojy
// cartesian topology https://www.codingame.com/playgrounds/47058/have-fun-with-mpi-in-c/mpi-process-topologies
// unicast

#include "main.h"

#define NUM_ITERATIONS 1
#define SENSOR_THRESHOLD 5 // if value > SENSOR_THRESHOLD => event
#define MIN_READING 0 // min sensor node reading
#define MAX_READING 10 // max sensor node reading
#define TOLERANCE_RANGE 3 
#define NEIGHBOUR_TAG 0
#define BASE_TAG 1
#define READING_TAG 2

int main(int argc, char *argv[]) {
    int rank, size, root; // root = base station
    int i, iteration;
    int left = -1, right = -1, up = -1, down = -1; // neighbours
    int sensor_reading;
    enum boolean { false = 0, true = 1 } simulation; 
    float simul_duration = 0.01; // simulation duration: 4 milliseconds
    int left_req = -999, right_req = -999, up_req = -999, down_req = -999;
    int left_reading = -999, right_reading = -999, up_reading = -999, down_reading = -999;
    float start, time_taken;

    // params for cartesian topology
    MPI_Comm comm;
    int dim[2], period[2], reorder, coord[2], id; // dim = [col, row]
    period[0] = 0; period[1] = 0; reorder = 0;

    MPI_Init(&argc, &argv); 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	MPI_Comm_size(MPI_COMM_WORLD, &size);
    root = size-1;

    if (rank == 0) { // prompt for row & col input
        //printf("col:"); // read col
        //fflush(stdout);
        //scanf("%d", &dim[0]);
        //printf("row:"); // read row
        //fflush(stdout);
        //scanf("%d", &dim[1]);
        dim[0] = 3; dim[1] = 3;
    } 

    MPI_Bcast(&dim, 2, MPI_INT, 0, MPI_COMM_WORLD);  // broadcast dimensions to all processes
    
    if (size < (dim[0]*dim[1])+1) { // insufficient number of processes for 2d grid & base station
        printf("Insufficient number of processes. Try smaller values of row & col or larger values of processes");
    }   

    MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, &comm); // initialise cartesian topology 

    if (rank == root) {
        sleep(3);
        int msg = 0, flag;
        while (msg < size) {
            MPI_Status stat;
            for (i = 0; i < 5; i++) {
                MPI_Iprobe(MPI_ANY_SOURCE, BASE_TAG, MPI_COMM_WORLD, &flag, &stat);
                if (flag) { // there exists a message
                    MPI_Recv(&sensor_reading, 1, MPI_INT, stat.MPI_SOURCE, BASE_TAG, MPI_COMM_WORLD, &stat);
                    printf("base-- from %d read %d\n", stat.MPI_SOURCE, sensor_reading);
                    break;
                }
            }
            msg++;
        }
    }

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

            start = MPI_Wtime(); // get start time
            if(sensor_reading > SENSOR_THRESHOLD) { // over threshold. trigger an event
                // record timestamp when event is triggered
                time_t t = time(NULL);
                struct tm *tm = localtime(&t);
                char *s;
                s = asctime(tm);
                printf("rank %d alert time %s\n", rank, s);

                if (left != MPI_PROC_NULL) { // left exists 
                    printf("rank %d left\n", rank);
                    send_msg(&sensor_reading, left, NEIGHBOUR_TAG); // send left to request for reading
                }

                if (right != MPI_PROC_NULL) { // right exists
                    printf("rank %d right\n", rank);
                    send_msg(&sensor_reading, right, NEIGHBOUR_TAG); // send right to request for reading
                }

                if (up != MPI_PROC_NULL) { // up exists
                    printf("rank %d up\n", rank);
                    send_msg(&sensor_reading, up, NEIGHBOUR_TAG); // send up to request for reading
                }

                if (down != MPI_PROC_NULL) { // down exists
                    printf("rank %d down\n", rank);
                    send_msg(&sensor_reading, down, NEIGHBOUR_TAG); // send down to request for reading
                }
            }

            MPI_Barrier(comm);

            if (right != MPI_PROC_NULL) {
                receive_msg(&right_req, right, NEIGHBOUR_TAG); // try to receive request message from right
                if (right_req > -1)
                printf("rank %d neighbour r %d \n", rank, right_req);
                if (sensor_reading - SENSOR_THRESHOLD < right_req && right_req < sensor_reading + SENSOR_THRESHOLD) {
                    printf("over t. rank %d right\n", rank);
                    send_msg(&sensor_reading, right, READING_TAG); // send reading right
                }
            }

            if (left != MPI_PROC_NULL) {
                receive_msg(&left_req, left, NEIGHBOUR_TAG); // try to receive request message from left
                if (left_req > -1)
                printf("rank %d neighbour l %d \n", rank, left_req);
                if (sensor_reading - SENSOR_THRESHOLD < left_req && left_req < sensor_reading + SENSOR_THRESHOLD) {
                    printf("over t. rank %d left\n", rank);
                    send_msg(&sensor_reading, left, READING_TAG); // send reading left
                }
            }

            if (down != MPI_PROC_NULL) {
                receive_msg(&down_req, down, NEIGHBOUR_TAG); // try to receive request message from down
                if (down_req > -1)
                printf("rank %d neighbour d %d \n", rank, down_req);
                if (sensor_reading - SENSOR_THRESHOLD < down_req && down_req < sensor_reading + SENSOR_THRESHOLD) {
                    printf("over t. rank %d down\n", rank);
                    send_msg(&sensor_reading, down, READING_TAG); // send reading down
                }
            }

            if (up != MPI_PROC_NULL) {
                receive_msg(&up_req, up, NEIGHBOUR_TAG); // try to receive request message from up
                if (up_req > -1)
                printf("rank %d neighbour u %d \n", rank, up_req);
                if (sensor_reading - SENSOR_THRESHOLD < up_req && up_req < sensor_reading + SENSOR_THRESHOLD) {
                    printf("over t. rank %d up\n", rank);
                    send_msg(&sensor_reading, up, READING_TAG); // send reading up
                }
            }
            
            MPI_Barrier(comm);

            if (right != MPI_PROC_NULL) {
                receive_msg(&right_reading, right, READING_TAG); // try to receive requested reading from right
                if (right_reading > -1)
                printf("!! rank %d neighbour r %d \n", rank, right_reading);
            }

            if (left != MPI_PROC_NULL) {
                receive_msg(&left_reading, left, READING_TAG); // try to receive requested reading from left
                if (left_reading > -1)
                printf("!! rank %d neighbour l %d \n", rank, left_reading);
            }

            if (up != MPI_PROC_NULL) {
                receive_msg(&up_reading, up, READING_TAG); // try to receive requested reading from up
                if (up_reading > -1)
                printf("!! rank %d neighbour u %d \n", rank, up_reading);
            }

            if (down != MPI_PROC_NULL) {
                receive_msg(&down_reading, down, READING_TAG); // try to receive requested reading from down
                if (down_reading > -1)
                printf("!! rank %d neighbour d %d \n", rank, down_reading);
            }

            if (sensor_reading > SENSOR_THRESHOLD) {
                // at least 2 or more similar readings, report to base station
                if ((right_reading > -1 && left_reading > -1) || (right_reading > -1 && up_reading > -1) || 
                (right_reading > -1 && down_reading > -1) || (left_reading > -1 && up_reading > -1) || 
                (left_reading > -1 && down_reading > -1) || (up_reading > -1 && down_reading > -1)) 
                    send_msg(&sensor_reading, root, BASE_TAG);
            }

            time_taken = MPI_Wtime() - start;
            printf("rank %d start %f time %f\n", rank, start, time_taken);

            MPI_Barrier(comm);

        }

    }

    MPI_Finalize();
    return; // exit
}

