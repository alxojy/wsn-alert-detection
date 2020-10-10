#ifndef SENSORNODE_H
#define SENSORNODE_H

#include "main.h"

void sensor_node(int rank, int root, MPI_Comm comm, int coord[], struct report_struct report, MPI_Datatype struct_type);

#endif // NODECOMM_H