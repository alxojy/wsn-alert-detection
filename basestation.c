#include "main.h"
#include "basestation.h"

struct infrared { // struct to store infrared reports generated by threads
    int reading;
    int node;
    char timestamp[26];
};

// represents the base station in WSN
// params
// rank: rank of base station
// size: total number of nodes in 2D sensor grid
// report: struct when receive report from sensor nodes
// struct_type: struct type of report
void base_station(int rank, int size, struct report_struct report, MPI_Datatype struct_type, FILE* outputfile) {
    struct infrared inf_reports[size]; // store reports generated by infrared
    int i, msg = 0, flag, sensor_reading;

    srandom(time(NULL) | rank); // seed

    // thread for generating infrared readings
    #pragma omp parallel for private(i) shared(size, inf_reports) schedule(static, 2)
        for (i = 0; i < size; i++) {
            int rand_num = (random() % (MAX_READING - MIN_READING + 1)) + MIN_READING;  // rand num
            inf_reports[i].reading = rand_num; inf_reports[i].node = i; 
            time_t t; struct tm *tm; char* tms; 
            t = time(NULL); tm = localtime(&t); tms = asctime(tm);
            strcpy(inf_reports[i].timestamp, tms);
        }

    sleep(1); 
    while (msg < size-1) { // listen to sensor nodes to check for alert
        MPI_Status stat;
        for (i = 0; i < size; i++) {
            MPI_Iprobe(MPI_ANY_SOURCE, BASE_TAG, MPI_COMM_WORLD, &flag, &stat);
            if (flag) { // there exists a message
                MPI_Recv(&report, 1, struct_type, stat.MPI_SOURCE, BASE_TAG, MPI_COMM_WORLD, &stat);
                printf("base-- from %d read %d msg %d time %s", stat.MPI_SOURCE, report.reading, report.num_msg, report.timestamp);
                fprintf(outputfile, "=============================================\n");
                // true alert
                if (inf_reports[stat.MPI_SOURCE].reading - TOLERANCE_RANGE < report.reading && report.reading < inf_reports[stat.MPI_SOURCE].reading + TOLERANCE_RANGE && strcmp(inf_reports[stat.MPI_SOURCE].timestamp, report.timestamp) == 0) { 
                    fprintf(outputfile, "ALERT STATUS: %s", "TRUE\n");
                    fprintf(outputfile, "Alert sent from sensor node: %d\n", stat.MPI_SOURCE);
                    fprintf(outputfile, "Sensor node reading: %d\n", report.reading);
                    fprintf(outputfile, "Infrared reading: %d\n", inf_reports[stat.MPI_SOURCE].reading);
                    fprintf(outputfile, "Adjacent nodes reading: %d, %d, %d, %d\n", report.adj_nodes[0], report.adj_nodes[1], report.adj_nodes[2], report.adj_nodes[3]);
                    fprintf(outputfile, "*neighbour order: left, right, up, down\n");
                    fprintf(outputfile, "*reading: -999 if neighbour does not exist/\nneighbour reading is not in threshold range\n");
                    fprintf(outputfile, "Number of messages exchanged: %d\n", report.num_msg);
                    fprintf(outputfile, "Timestamp: %s", report.timestamp);
                } 
                else { // false alert
                    fprintf(outputfile, "ALERT STATUS: %s", "FALSE\n");
                    fprintf(outputfile, "Alert sent from sensor node: %d\n", stat.MPI_SOURCE);
                    fprintf(outputfile, "Sensor node reading: %d\n", report.reading);
                    fprintf(outputfile, "Infrared reading: %d\n", inf_reports[stat.MPI_SOURCE].reading);
                    fprintf(outputfile, "Adjacent nodes reading: %d, %d, %d, %d\n", report.adj_nodes[0], report.adj_nodes[1], report.adj_nodes[2], report.adj_nodes[3]);
                    fprintf(outputfile, "*neighbour order: left, right, up, down\n");
                    fprintf(outputfile, "*reading: -999 if neighbour does not exist/\nneighbour reading is not in threshold range\n");
                    fprintf(outputfile, "Number of messages exchanged: %d\n", report.num_msg);
                    fprintf(outputfile, "Timestamp: %s", report.timestamp);
                }
                break;
            }
        }
        msg++;
    }
}
