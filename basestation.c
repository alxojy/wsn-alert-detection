#include "main.h"
#include "basestation.h"

#define NUM_THREADS 5

struct infrared { // struct to store infrared reports generated by threads
    int reading;
    int node;
    char timestamp[26];
};

void *ThreadFunc(void *pArg);
int n;
struct infrared inf_reports[50]; // store reports generated by infrared

// represents the base station in WSN
// params
// rank: rank of base station
// size: total number of nodes in 2D sensor grid
// report: struct when receive report from sensor nodes
// struct_type: struct type of report
int base_station(int rank, int size, struct report_struct report, MPI_Datatype struct_type, FILE* outputfile) {
    int i, j, msg = 0, flag, sensor_reading;
    n = size;

    srandom(time(NULL) | rank); // seed

    pthread_t tid[NUM_THREADS];
    int threadNum[NUM_THREADS];

    for (i = 0; i < NUM_THREADS; i++) {
        threadNum[i] = i;
        pthread_create(&tid[i], 0, ThreadFunc, &threadNum[i]); // fork
    }
    
    for (i = 0; i < NUM_THREADS; i++) { // join
        pthread_join(tid[i], NULL);
    }

    sleep(1); 
    int true_alerts = 0, false_alerts = 0, num_alerts = 0;
    while (msg < size-1) { // listen to sensor nodes to check for alert
        MPI_Status stat;
        for (i = 0; i < size; i++) {
            MPI_Iprobe(MPI_ANY_SOURCE, BASE_TAG, MPI_COMM_WORLD, &flag, &stat);
            if (flag) { // there exists a message
                num_alerts++;
                MPI_Recv(&report, 1, struct_type, stat.MPI_SOURCE, BASE_TAG, MPI_COMM_WORLD, &stat);
                fprintf(outputfile, "=============================================\n");
                // true alert
                if (inf_reports[stat.MPI_SOURCE].reading - TOLERANCE_RANGE < report.reading && report.reading < inf_reports[stat.MPI_SOURCE].reading + TOLERANCE_RANGE && strcmp(inf_reports[stat.MPI_SOURCE].timestamp, report.timestamp) == 0) { 
                    true_alerts++;
                    fprintf(outputfile, "ALERT STATUS: %s", "TRUE\n");
                    fprintf(outputfile, "Alert sent from sensor node %d\n", stat.MPI_SOURCE);
                    fprintf(outputfile, "Sensor node reading: %d\n", report.reading);
                    fprintf(outputfile, "Infrared reading: %d\n", inf_reports[stat.MPI_SOURCE].reading);
                    fprintf(outputfile, "Adjacent nodes\n");
                    for (j = 0; j < 4; j++) {
                        if (report.adj_reading[j] >= 0) {
                            fprintf(outputfile, "Node: %d reading: %d\n", report.adj_nodes[j], report.adj_reading[j]);
                        }
                    }
                    fprintf(outputfile, "Time taken: %f\n", report.time_taken);
                    fprintf(outputfile, "Number of messages exchanged: %d\n", report.num_msg);
                    fprintf(outputfile, "Timestamp: %s", report.timestamp);
                } 
                else { // false alert
                    false_alerts++;
                    fprintf(outputfile, "ALERT STATUS: %s", "FALSE\n");
                    fprintf(outputfile, "Alert sent from sensor node: %d\n", stat.MPI_SOURCE);
                    fprintf(outputfile, "Sensor node reading: %d\n", report.reading);
                    fprintf(outputfile, "Infrared reading: %d\n", inf_reports[stat.MPI_SOURCE].reading);
                    fprintf(outputfile, "Adjacent nodes\n");
                    for (j = 0; j < 4; j++) {
                        if (report.adj_reading[j] >= 0) {
                            fprintf(outputfile, "Node: %d reading: %d\n", report.adj_nodes[j], report.adj_reading[j]);
                        }
                    }
                    fprintf(outputfile, "Time taken: %f\n", report.time_taken);
                    fprintf(outputfile, "Number of messages exchanged: %d\n", report.num_msg);
                    fprintf(outputfile, "Timestamp: %s", report.timestamp);
                }
                break;
            }
        }
        msg++;
    }
    // summary per iteration
    fprintf(outputfile, "---------------------------------------------\nNumber of true alerts: %d\nNumber of false alerts: %d\nTotal number of alerts: %d\n---------------------------------------------\n\n", true_alerts, false_alerts, num_alerts);
    return num_alerts;
}

void *ThreadFunc(void *pArg) { // POSIX thread to generate random numbers
    int i;
    int rank = *((int *)pArg);

    int rpt = n / NUM_THREADS;
    int rptr = n % NUM_THREADS;

    int sp = rank * rpt;
    int ep = sp + rpt;
    if (rank == NUM_THREADS - 1) {
        ep += rptr;
    }

    for (i = sp; i < ep; i++) {
        int rand_num = (random() % (MAX_READING - MIN_READING + 1)) + MIN_READING;  // rand num
        inf_reports[i].reading = rand_num; inf_reports[i].node = i; 
        time_t t; struct tm *tm; char* tms; 
        t = time(NULL); tm = localtime(&t); tms = asctime(tm);
        strcpy(inf_reports[i].timestamp, tms);
    }
    return NULL;
}

