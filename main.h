#ifndef MAIN_H
#define MAIN_H

#define NUM_ITERATIONS 1
#define SENSOR_THRESHOLD 5 // if value > SENSOR_THRESHOLD => event
#define MIN_READING 0 // min sensor node reading
#define MAX_READING 10 // max sensor node reading
#define TOLERANCE_RANGE 3 
#define NEIGHBOUR_TAG 0
#define BASE_TAG 1
#define READING_TAG 2

#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "nodecomm.c"

extern void sensor_node(int rank, int root, MPI_Comm comm, int coord[]);

#endif // MAIN_H