# FIT3143 Alert Detection in Wireless Sensor Networks

The purpose of this project is to simulate a WSN network where sensor nodes are arranged in a 2D grid with a common base station. The sensor nodes and base station are represented by MPI processes.  

```
WSN 2D grid

1 -- 2 -- 3
|    |    |
4 -- 5 -- 6
|    |    |
7 -- 8 -- 9
```

Sensor nodes can only communicate with its adjacent (left, right, up, down) neighbours and with the base station.  
  
When the sensor node's reading is above a threshold ```5```, the sensor node requests its neighbours for their reading to compare. I proposed a solution which uses point-to-point communication between nodes.   
  
#### How does it work?
Example:
1. Process 5's reading > threshold.
2. Process 5 does an ```MPI_Isend``` call to all its neighbours (2, 4, 6, 8).
3. Neighbours ```MPI_Recv``` the reading and compare it with its own. Let's use neighbour 2 in this example.
4. If neighbour 2's reading is in process 5's threshold range, ```MPI_Isend``` its own reading.
5. Process 5 ```MPI_Recv``` readings from all its neighbours where neighbour's reading is in its threshold range.
6. If process 5 receives >= 2 readings from its neighbour, send an alert to the base station. 
  
At the same time, the base station generates readings for each node using a POSIX thread. When the base station receives readings from the sensor nodes, it checks if the reading and time received are similar to its own and logs it in ```logs.txt```.
  
#### Dependencies
```
pthread
openmpi
```

#### To run 
```make``` <br>
```mpirun -np [num processes] WSN [rows] [cols] [num iterations]```

#### Results
Refer to ```logs.txt```
