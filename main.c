// alxojy
// to run: mpicc main.c basestation.c sensornode.c nodecomm.c
#include "main.h"

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

    // initialise MPI
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
        //printf("number of intervals:"); // read row
        //fflush(stdout);
        //scanf("%d", &num_iterations);
        num_iterations = 2;
        dim[0] = 3; dim[1] = 3;
    } 

    MPI_Bcast(&dim, 2, MPI_INT, 0, MPI_COMM_WORLD);  // broadcast dimensions to all processes
    MPI_Bcast(&num_iterations, 1, MPI_INT, 0, MPI_COMM_WORLD); // broadcast number of iterations to all processes
    
    if (size < (dim[0]*dim[1])+1) { // insufficient number of processes for 2D grid & base station
        printf("Insufficient number of processes. Try smaller values of row & col or larger values of processes");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }   

    FILE *outputfile; // create output file
    outputfile = fopen(OUTPUTFILE, "w");
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
        if (rank != root) { // sensor in 2d grid
            sensor_node(rank, root, comm, coord, report, report_type);
            sleep(1);
            } 
        else {
            base_station(root, size, report, report_type, outputfile);
        }
    }

    fclose(outputfile);
    MPI_Type_free(&report_type);
    MPI_Finalize();
    return 0; // exit
}
