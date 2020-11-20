// MA_Lab-01_Team_05
// to run: 
// 1. open terminal
// 2. make
// 3. mpirun -np [number of processes] WSN [rows] [cols] [num of iterations]
#include "main.h"

int main(int argc, char *argv[]) {
    int rank, size, root; // root = base station
    int iteration, i, num_iterations, num_alerts = 0, alert_sent = 0;
    int sensor_reading;
    enum boolean { false = 0, true = 1 } simulation; 
    float simul_duration, start; 
    fd_set rfds; struct timeval tv; int stop; // variables for sentinel value

    // params for cartesian topology
    MPI_Comm comm;
    int dim[2], period[2], reorder, coord[2], id; // dim = [col, row]
    period[0] = 0; period[1] = 0; reorder = 0;

    // initialise MPI
    MPI_Init(&argc, &argv); 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	MPI_Comm_size(MPI_COMM_WORLD, &size);
    root = size-1;

    if (argc == 4) { 
		dim[1] = atoi (argv[1]); // number of row
		dim[0] = atoi (argv[2]); // number of column
        num_iterations = atoi (argv[3]); // number of iterations
		if((dim[0]*dim[1])+1 != size) {
			if(rank == 0) 
            printf("ERROR: Enter row & col into command line\nPlease ensure that there are sufficient number of processes (row x col) + 1\n");
			MPI_Finalize(); 
			return 0;
		}

        if (num_iterations <= 0) {
            printf("ERROR: Number of iterations must be > 0\n");
			MPI_Finalize(); 
			return 0;
        }
    }

    FILE *outputfile; // create output file
    outputfile = fopen(OUTPUTFILE, "w");
    MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, &comm); // initialise cartesian topology 

    // initialise struct to store sensor node reports
    struct report_struct report;
    MPI_Datatype report_type;
	MPI_Datatype type[8] = { MPI_INT, MPI_FLOAT, MPI_CHAR, MPI_INT, MPI_INT, MPI_INT, MPI_CHAR, MPI_CHAR };
	int blocklen[8] = { 1,1,26,1,4,4,15,18 };
	MPI_Aint disp[8];

	MPI_Get_address(&report.reading, &disp[0]); 
	MPI_Get_address(&report.time_taken, &disp[1]); 
    MPI_Get_address(&report.timestamp, &disp[2]);
    MPI_Get_address(&report.num_msg, &disp[3]);
    MPI_Get_address(&report.adj_nodes, &disp[4]);
    MPI_Get_address(&report.adj_reading, &disp[5]);
    MPI_Get_address(&report.ip_address, &disp[6]);
    MPI_Get_address(&report.mac_address, &disp[7]);

    for(i = 1; i < 8; i++) {
        disp[i] -= disp[0];
    }
    disp[0] = 0;

	// Create MPI struct
	MPI_Type_create_struct(8, blocklen, disp, type, &report_type);
	MPI_Type_commit(&report_type);

    if (rank == root) 
        printf("Press enter to stop program:\n"); // print prompt for sentinel value

    start = MPI_Wtime();
    for (iteration = 0; iteration < num_iterations; iteration++) { // run num_iterations times
        FD_ZERO(&rfds); 
        FD_SET(0, &rfds);
        tv.tv_sec = 1;
        if (rank != root) { // sensor in 2d grid
            alert_sent += sensor_node(rank, root, comm, coord, report, report_type); // number of alerts sent per node
            sleep(1); // sleep while base station is collecting reports sent by sensor nodes
        } 
        else {
            fprintf(outputfile, "ITERATION: %d\n", iteration);
            num_alerts += base_station(root, size, report, report_type, outputfile); // number of alerts received by base station
        }

        if (rank == 0) {
            stop = select(1, &rfds, NULL, NULL, &tv); // get sentinel value
        }
        MPI_Bcast(&stop, 1, MPI_INT, 0, MPI_COMM_WORLD); // send termination message to all processes

        if (stop) { // sentinel value received
            break;
        }
    }

    fclose(outputfile);

    // overall summary
    FILE *fp; // create output file
    fp = fopen(OUTPUTFILE, "a+");
    
    if (rank == size-1) { // base station logs
        simul_duration =  MPI_Wtime() - start;
        fprintf(fp, "Total simulation time: %f\n", simul_duration);
        fprintf(fp, "Total number of alerts sent to the base station: %d\n", num_alerts);
    }
    else {
        fprintf(fp, "Total number of alerts sent by node %d: %d\n", rank, alert_sent);
    }
    
    fclose(fp);

    MPI_Type_free(&report_type);
    MPI_Finalize();
    return 0; // exit
}
