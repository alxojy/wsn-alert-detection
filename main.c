// alxojy
// cartesian topology https://www.codingame.com/playgrounds/47058/have-fun-with-mpi-in-c/mpi-process-topologies
// unicast
#include "main.h"

struct report_struct { // struct for node reports sent to base station
    int reading;
    float time_taken;
    char timestamp[26];
    int num_msg; // num messages compared
    int adj_nodes[4];
};

void base_station(int rank, int size, struct report_struct report, MPI_Datatype struct_type);
void sensor_node(int rank, int root, MPI_Comm comm, int coord[], struct report_struct report, MPI_Datatype struct_type);

int main(int argc, char *argv[]) {
    int rank, size, root; // root = base station
    int iteration, i, num_iterations;
    int sensor_reading;
    enum boolean { false = 0, true = 1 } simulation; 
    float simul_duration, sim_end; // simulation duration: 4 milliseconds

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
        printf("number of intervals:"); // read row
        fflush(stdout);
        scanf("%d", &num_iterations);
        dim[0] = 3; dim[1] = 3;
    } 

    MPI_Bcast(&dim, 2, MPI_INT, 0, MPI_COMM_WORLD);  // broadcast dimensions to all processes
    MPI_Bcast(&num_iterations, 1, MPI_INT, 0, MPI_COMM_WORLD); // broadcast number of iterations to all processes
    
    if (size < (dim[0]*dim[1])+1) { // insufficient number of processes for 2d grid & base station
        printf("Insufficient number of processes. Try smaller values of row & col or larger values of processes");
    }   

    MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, &comm); // initialise cartesian topology 

    struct report_struct report;
    MPI_Datatype report_type;
	MPI_Datatype type[5] = { MPI_INT, MPI_FLOAT, MPI_CHAR, MPI_INT, MPI_INT };
	int blocklen[5] = { 1,1,26,1,4 };
	MPI_Aint disp[5];

	MPI_Get_address(&report.reading, &disp[0]);
	MPI_Get_address(&report.time_taken, &disp[1]);
    MPI_Get_address(&report.timestamp, &disp[2]);
    MPI_Get_address(&report.num_msg, &disp[3]);
    MPI_Get_address(&report.adj_nodes, &disp[4]);

    for(i = 1; i < 5; i++) {
        disp[i] -= disp[0];
    }
    disp[0] = 0;

	// Create MPI struct
	MPI_Type_create_struct(5, blocklen, disp, type, &report_type);
	MPI_Type_commit(&report_type);

    for (iteration = 0; iteration < num_iterations; iteration++) { // run num_iterations times
        printf("iteration %d\n", iteration);
        
        if (rank != root) { // sensor in 2d grid
            sensor_node(rank, root, comm, coord, report, report_type);
            sleep(1);
            } 
        else {
            base_station(root, size, report, report_type);
        }
        
    }

    MPI_Type_free(&report_type);
    MPI_Finalize();
    return; // exit
}

void base_station(int rank, int size, struct report_struct report, MPI_Datatype struct_type) {
    sleep(1);
    int i;
    int msg = 0, flag, sensor_reading;
    while (msg < size-1) {
        MPI_Status stat;
        for (i = 0; i < size; i++) {
            MPI_Iprobe(MPI_ANY_SOURCE, BASE_TAG, MPI_COMM_WORLD, &flag, &stat);
            if (flag) { // there exists a message
                MPI_Recv(&report, 1, struct_type, stat.MPI_SOURCE, BASE_TAG, MPI_COMM_WORLD, &stat);
                printf("base-- from %d read %d msg %d time %s", stat.MPI_SOURCE, report.reading, report.num_msg, report.timestamp);
                break;
            }
        }
        msg++;
    }
}

void sensor_node(int rank, int root, MPI_Comm comm, int coord[], struct report_struct report, MPI_Datatype struct_type) {
    int sensor_reading;
    int left = -1, right = -1, up = -1, down = -1; // neighbours
    int left_req = -999, right_req = -999, up_req = -999, down_req = -999;
    int left_reading = -999, right_reading = -999, up_reading = -999, down_reading = -999;

    float start, time_taken;
    int num_msg = 0;
    time_t t; struct tm *tm; char* tms; 

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
        tms = asctime(tm);

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
        if (sensor_reading - TOLERANCE_RANGE < right_req && right_req < sensor_reading + TOLERANCE_RANGE) {
            send_msg(&sensor_reading, right, READING_TAG); // send reading right
            num_msg++;
        }
    }

    if (left != MPI_PROC_NULL) {
        receive_msg(&left_req, left, NEIGHBOUR_TAG); // try to receive request message from left
        if (sensor_reading - TOLERANCE_RANGE < left_req && left_req < sensor_reading + TOLERANCE_RANGE) {
            send_msg(&sensor_reading, left, READING_TAG); // send reading left
            num_msg++;
        }
    }

    if (down != MPI_PROC_NULL) {
        receive_msg(&down_req, down, NEIGHBOUR_TAG); // try to receive request message from down
        if (sensor_reading - TOLERANCE_RANGE < down_req && down_req < sensor_reading + TOLERANCE_RANGE) {
            send_msg(&sensor_reading, down, READING_TAG); // send reading down
            num_msg++;
        }
    }

    if (up != MPI_PROC_NULL) {
        receive_msg(&up_req, up, NEIGHBOUR_TAG); // try to receive request message from up
        if (sensor_reading - TOLERANCE_RANGE < up_req && up_req < sensor_reading + TOLERANCE_RANGE) {
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

            time_taken = MPI_Wtime() - start;
            report.reading = sensor_reading;
            report.time_taken = time_taken;
            strcpy(report.timestamp, tms);
            report.num_msg = num_msg;
            
            report.adj_nodes[0] = left_reading;
            report.adj_nodes[1] = right_reading;
            report.adj_nodes[2] = up_reading;
            report.adj_nodes[3] = down_reading;

            MPI_Request req;
            MPI_Isend(&report, 1, struct_type, root, BASE_TAG, MPI_COMM_WORLD, &req);
            
            //send_msg(&sensor_reading, root, BASE_TAG);
            printf("rank %d more than \n", rank);
            printf("rank %d alert time %s", rank, report.timestamp);
            printf("rank %d time %f\n", rank, report.time_taken);
            printf("rank %d num msg %d \n", rank, report.num_msg);
        }
    }

    MPI_Barrier(comm);
}
