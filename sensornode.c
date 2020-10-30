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
int sensor_node(int rank, int root, MPI_Comm comm, int coord[], struct report_struct report, MPI_Datatype struct_type) {
    int sensor_reading, i, j;
    int neighbours[4] = {-1, -1, -1, -1}; // left, right, up, down
    int neighbour_req[4] = {-999, -999, -999, -999}; // store request messages
    int neighbour_reading[4] = {-999, -999, -999, -999}; // store neighbour readings
    int alert_sent = 0;

    float start, time_taken;
    int num_msg = 0;
    time_t t; struct tm *tm; char* tms; 

    MPI_Cart_coords(comm, rank, 2, coord);
    MPI_Cart_shift(comm, 0, 1, &neighbours[2], &neighbours[3]);
    MPI_Cart_shift(comm, 1, 1, &neighbours[0], &neighbours[1]);

    srandom(time(NULL) | rank); // seed
    sensor_reading = (random() % (MAX_READING - MIN_READING + 1)) + MIN_READING; // random sensor reading
    
    start = MPI_Wtime(); // get start time
    if(sensor_reading > SENSOR_THRESHOLD) { // over threshold. trigger an event
        // record timestamp when event is triggered
        t = time(NULL);
        tm = localtime(&t);
        tms = asctime(tm);

        for (i = 0; i < 4; i++) { // loop through all 4 neighbours in left, right, up, down order
            if (neighbours[i] != MPI_PROC_NULL) { // neighbour exists
                send_msg(&sensor_reading, neighbours[i], NEIGHBOUR_TAG); // send neighbour a message to request for reading
                num_msg++;
            }
        }
    }

    MPI_Barrier(comm);

    for (i = 0; i < 4; i++) { // loop through all 4 neighbours in left, right, up, down order
        if (neighbours[i] != MPI_PROC_NULL) { // neighbour exists
            receive_msg(&neighbour_req[i], neighbours[i], NEIGHBOUR_TAG); // try to receive request message from neighbour
            if (sensor_reading - TOLERANCE_RANGE < neighbour_req[i] && neighbour_req[i] < sensor_reading + TOLERANCE_RANGE) {
                send_msg(&sensor_reading, neighbours[i], READING_TAG); // send reading to neighbour
            }
        }
    }

    MPI_Barrier(comm);

    for (i = 0; i < 4; i++) { // loop through all 4 neighbours in left, right, up, down order
        if (neighbours[i] != MPI_PROC_NULL)  { // neighbour exists 
            receive_msg(&neighbour_reading[i], neighbours[i], READING_TAG); // try to receive requested reading from neighbour
            if (neighbour_reading[i] > 0)
                num_msg++;
        }
    }

    if (sensor_reading > SENSOR_THRESHOLD) {
        // at least 2 or more similar readings, report to base station
        if ((neighbour_reading[0] > -1 && neighbour_reading[1] > -1) || (neighbour_reading[0] > -1 && neighbour_reading[2] > -1) || 
        (neighbour_reading[0] > -1 && neighbour_reading[3] > -1) || (neighbour_reading[1] > -1 && neighbour_reading[2] > -1) || 
        (neighbour_reading[1] > -1 && neighbour_reading[3] > -1) || (neighbour_reading[2] > -1 && neighbour_reading[3] > -1))  {
            alert_sent = 1;
            time_taken = MPI_Wtime() - start; // calculate total time taken by a sensor node
            report.reading = sensor_reading; // sensor node reading
            report.time_taken = time_taken; 
            strcpy(report.timestamp, tms); // timestamp of event
            report.num_msg = num_msg; // number of messages exchanged
            memcpy(report.ip_address, get_ip_address(), 15);

            for (j = 0; j < 4; j++) {
                report.adj_nodes[j] = neighbours[j]; // neighbour rank (left, right, up, down)
                report.adj_reading[j] = neighbour_reading[j]; // neighbour readings
            }

            MPI_Request req;
            MPI_Isend(&report, 1, struct_type, root, BASE_TAG, MPI_COMM_WORLD, &req);
        }
    }

    MPI_Barrier(comm);

    return alert_sent;
}
