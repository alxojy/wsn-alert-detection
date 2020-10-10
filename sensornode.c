#include "main.h"
#include "sensornode.h"

// represents a sensor node in WSN network
// params
// rank: rank of node
// root: base station rank
// comm: communicator in WSN
// coord: coordinate of node
// report: struct to send report to base station
// struct_type: struct type for report
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

    if (right != MPI_PROC_NULL) 
        receive_msg(&right_reading, right, READING_TAG); // try to receive requested reading from right
    

    if (left != MPI_PROC_NULL) 
        receive_msg(&left_reading, left, READING_TAG); // try to receive requested reading from left
    

    if (up != MPI_PROC_NULL) 
        receive_msg(&up_reading, up, READING_TAG); // try to receive requested reading from up
    

    if (down != MPI_PROC_NULL) 
        receive_msg(&down_reading, down, READING_TAG); // try to receive requested reading from down
    

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
            
            printf("rank %d more than \n", rank);
            printf("rank %d alert time %s", rank, report.timestamp);
            printf("rank %d time %f\n", rank, report.time_taken);
            printf("rank %d num msg %d \n", rank, report.num_msg);
        }
    }

    MPI_Barrier(comm);
}
