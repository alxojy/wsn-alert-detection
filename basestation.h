#ifndef BASESTATION_H
#define BASESTATION_H

#include <omp.h>

void base_station(int rank, int size, struct report_struct report, MPI_Datatype struct_type, FILE* outputfile);

#endif // BASESTATION_H
