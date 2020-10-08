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

    float start, time_taken;
    int num_msg = 0;
    int left_req = -999, right_req = -999, up_req = -999, down_req = -999;
    int left_reading = -999, right_reading = -999, up_reading = -999, down_reading = -999;
    time_t t; struct tm *tm; char *s;


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
            start = time_taken = 0;
            num_msg = 0;
            left_req = right_req = up_req = down_req = -999;
            left_reading = right_reading = up_reading = down_reading = -999;

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
                t = time(NULL);
                tm = localtime(&t);
                s = asctime(tm);

                if (left != MPI_PROC_NULL) { // left exists 
                    send_msg(&sensor_reading, left, NEIGHBOUR_TAG); // send left to request for reading
                    num_msg++;
                }

                if (right != MPI_PROC_NULL) { // right exists
                    send_msg(&sensor_reading, right, NEIGHBOUR_TAG); // send right to request for reading
                    num_msg++;
                }

                if (up != MPI_PROC_NULL) { // up exists
                    send_msg(&sensor_reading, up, NEIGHBOUR_TAG); // send up to request for reading
                    num_msg++;
                }

                if (down != MPI_PROC_NULL) { // down exists
                    send_msg(&sensor_reading, down, NEIGHBOUR_TAG); // send down to request for reading
                    num_msg++;
                }
            }

            MPI_Barrier(comm);

            if (right != MPI_PROC_NULL) {
                receive_msg(&right_req, right, NEIGHBOUR_TAG); // try to receive request message from right
                if (sensor_reading - SENSOR_THRESHOLD < right_req && right_req < sensor_reading + SENSOR_THRESHOLD) {
                    send_msg(&sensor_reading, right, READING_TAG); // send reading right
                    num_msg++;
                }
            }

            if (left != MPI_PROC_NULL) {
                receive_msg(&left_req, left, NEIGHBOUR_TAG); // try to receive request message from left
                if (sensor_reading - SENSOR_THRESHOLD < left_req && left_req < sensor_reading + SENSOR_THRESHOLD) {
                    send_msg(&sensor_reading, left, READING_TAG); // send reading left
                    num_msg++;
                }
            }

            if (down != MPI_PROC_NULL) {
                receive_msg(&down_req, down, NEIGHBOUR_TAG); // try to receive request message from down
                if (sensor_reading - SENSOR_THRESHOLD < down_req && down_req < sensor_reading + SENSOR_THRESHOLD) {
                    send_msg(&sensor_reading, down, READING_TAG); // send reading down
                    num_msg++;
                }
            }

            if (up != MPI_PROC_NULL) {
                receive_msg(&up_req, up, NEIGHBOUR_TAG); // try to receive request message from up
                if (sensor_reading - SENSOR_THRESHOLD < up_req && up_req < sensor_reading + SENSOR_THRESHOLD) {
                    send_msg(&sensor_reading, up, READING_TAG); // send reading up
                    num_msg++;
                }
            }
            
            MPI_Barrier(comm);

            if (right != MPI_PROC_NULL) {
                receive_msg(&right_reading, right, READING_TAG); // try to receive requested reading from right
            }

            if (left != MPI_PROC_NULL) {
                receive_msg(&left_reading, left, READING_TAG); // try to receive requested reading from left
            }

            if (up != MPI_PROC_NULL) {
                receive_msg(&up_reading, up, READING_TAG); // try to receive requested reading from up
            }

            if (down != MPI_PROC_NULL) {
                receive_msg(&down_reading, down, READING_TAG); // try to receive requested reading from down
            }

            if (sensor_reading > SENSOR_THRESHOLD) {
                // at least 2 or more similar readings, report to base station
                if ((right_reading > -1 && left_reading > -1) || (right_reading > -1 && up_reading > -1) || 
                (right_reading > -1 && down_reading > -1) || (left_reading > -1 && up_reading > -1) || 
                (left_reading > -1 && down_reading > -1) || (up_reading > -1 && down_reading > -1))  {
                    send_msg(&sensor_reading, root, BASE_TAG);
                    printf("rank %d more than \n", rank);
                    time_taken = MPI_Wtime() - start;
                    printf("rank %d alert time %s", rank, s);
                    printf("rank %d start %f time %f\n", rank, start, time_taken);
                    printf("rank %d num msg %d \n", rank, num_msg);
                }
            }

            MPI_Barrier(comm);

        }

    }

    MPI_Finalize();
    return; // exit
}

