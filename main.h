#ifndef MAIN_H
#define MAIN_H

#define SENSOR_THRESHOLD 5 // if value > SENSOR_THRESHOLD => event
#define MIN_READING 0 // min sensor node reading
#define MAX_READING 10 // max sensor node reading
#define TOLERANCE_RANGE 3 
#define NEIGHBOUR_TAG 0
#define BASE_TAG 1
#define READING_TAG 2
#define OUTPUTFILE "logs.txt"

struct report_struct { // struct for node reports sent to base station
    int reading;
    float time_taken;
    char timestamp[26];
    int num_msg; // num messages compared
    int adj_nodes[4]; // adjacent nodes rank
    int adj_reading[4]; // adjacent nodes reading
    int ip_address[15]; // store ip address of node
};

// import necessary libraries
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include "sensornode.h"
#include "basestation.h"

#endif // MAIN_H
