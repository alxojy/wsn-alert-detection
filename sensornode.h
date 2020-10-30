#ifndef SENSORNODE_H
#define SENSORNODE_H

#include "main.h"
#include "nodecomm.h"
#include "get_ip_address.h"
#include "get_mac_address.h"

int sensor_node(int rank, int root, MPI_Comm comm, int coord[], struct report_struct report, MPI_Datatype struct_type);

#endif // SENSORNODE_H
