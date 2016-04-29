#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
//#include <omp.h>
#include <mpi.h>

#include "userCode.h"

using namespace std;

// Message passing code 
void sendDataSet(long **inputArray, long size, int dest, int tag, MPI_Comm comm) {
	int totalCount = (size * size);
	long *buffer = (long *) malloc(sizeof(long) * (totalCount));
	//buffer[0] = size;
	int count = 0;
	for(int i = 0 ; i < size; i++) {
		for (int j = 0 ; j < size; j++) {
			buffer[count++] = inputArray[i][j];
		}
	}
	long *sizeBuff = &size;
	MPI_Send(sizeBuff, 1 , MPI_INT, dest, tag, comm);
	MPI_Send(buffer,totalCount, MPI_LONG, dest, tag, comm);
	free(buffer);
}

void recvDataSet(long ***inputArray, long *size, int src, int tag, MPI_Comm comm) {
	MPI_Status status;

	MPI_Recv(size, 1, MPI_LONG, src, tag, comm, &status);
	
	int totalSize = (*size) * (*size);

	//printf("Total size : %d\n", totalSize );

	long *buffer = (long *)malloc(sizeof(long) * (totalSize));

	int count = 0;

	MPI_Recv(buffer, totalSize, MPI_LONG, src, tag, comm, &status);

	*inputArray = (long **) malloc(sizeof(long *) * (*size));
	
	for(int i = 0; i < (*size) ; i++) {
			(*inputArray)[i] = (long *)malloc(sizeof(long) * (*size));
			for(int j = 0 ; j < (*size) ;j++) {
				(*inputArray)[i][j] = buffer[count++];
				//printf("%ld\n", (*inputArray)[i][j] );
			}
	}
	free(buffer);
}

void sendNodeMPI(Node *node, int dest, int tag, MPI_Comm comm) {
	long *buffer =  (long *) malloc(sizeof(long) * 6);
	buffer[0] = node->bound;
	buffer[1] = node->globalBound;
	buffer[2] = node->actualCost;
	buffer[3] = node->yDone.size();
	buffer[4] = node->xDone.size();
	buffer[5] = node->assignment.size();
	//printf("Sending values : %ld, %ld, %ld ,%ld , %ld\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4] );
	MPI_Send(buffer,6, MPI_LONG, dest, tag, comm);
	int totalDataSize = buffer[3] + buffer[4] + buffer[5];
	long *buffer2 =  (long *) malloc(sizeof(long) * totalDataSize);
	int count = 0;
	for(auto data : node->yDone) {
		buffer2[count++] = data;
	}
	for(auto data : node->xDone) {
		buffer2[count++] = data;
	}

	for (auto data : node->assignment) {
		buffer2[count++] = data;
 	}

 	MPI_Send(buffer2, totalDataSize, MPI_LONG, dest, tag, comm);

	free(buffer);
	free(buffer2);
}

void recvNodeMPI(Node *node, int src, int tag, MPI_Comm comm, MPI_Status *status) {
	long *buffer = (long *)malloc(sizeof(long) * 6);
	//MPI_Status status;
	MPI_Recv(buffer, 6 , MPI_LONG, src , tag, comm, status);
	// printf("Bound value received : %ld\n", buffer[0]);
	// printf("ActualCost value received : %ld\n", buffer[1]);
	// printf("yDone size value received : %ld\n", buffer[2]);
	// printf("xDone size value received : %ld\n", buffer[3]);
	// printf("Assignment value received : %ld\n", buffer[4]);

	node->bound = buffer[0];
	node->globalBound = buffer[1];
	node->actualCost = buffer[2];
	int receiveCount = buffer[3] + buffer[4] + buffer[5];
	long *buffer2 =  (long *) malloc(sizeof(long) * receiveCount);
	MPI_Recv(buffer2, receiveCount , MPI_LONG, src , tag, comm, status);
	int count = 0;
	for(int i = 0; i < buffer[3]; i++) {
		node->yDone.insert(buffer2[count++]);
	}

	for(int i = 0; i < buffer[4]; i++) {
		node->xDone.insert(buffer2[count++]);
	}

	for(int i = 0; i < buffer[5]; i++) {
		node->assignment.push_back(buffer2[count++]);
	}

	free(buffer);
	free(buffer2);
}

void printMatrix(long **inputArray, long size) {
	for (long i = 0; i < size; i++) {
		for (long j = 0;  j < size; j++) {
			printf("%ld ", inputArray[i][j] );
		}
		printf("\n");
	}
}

long globalBound = INF;
vector <long> currentSol;
long **inputArray;
long limit;
int processDepth;
//Framework Code starts

set <Node *> liveNodes;

void insertLiveNode(Node *solution) {
	// #pragma omp critical (insert)
	// {
		if(solution->bound < globalBound) {
			solution->globalBound = globalBound;
			liveNodes.insert(solution);
		} //else { printf("Branch pruned due to bound value\n");
	//	}
	//}
}

void updateBestSolution(Node *solution) {
	// #pragma omp critical (update)
	// {
		if(globalBound >  solution->bound) {
			printf("Solution found : %ld \n", solution->bound);
			globalBound = solution->bound;
			currentSol = solution->assignment;
		}
	//}
}
	
void * chooseBestLiveNode() {

	/* Following code implements best first
	int minBound = INF;
	Node * liveNode = NULL;
	for (auto sol : liveNodes) {
		if(minBound > sol->bound) {
			minBound = sol->bound;
			liveNode = sol;
		}
	}
	liveNodes.erase(liveNode);
	return (void *)liveNode;
	*/

	// Following code implements depth first 
	// depth first results into more prunning of branches
	Node *liveNode = NULL;
	if(!liveNodes.empty()) {
		for (auto sol : liveNodes) {
			if(liveNode == NULL) {
				liveNode = sol;
			} else {
				if(liveNode->yDone.size() < sol->yDone.size()) {
					liveNode = sol;
				}
			}
		}
		liveNodes.erase(liveNode);
	}
	return (void *)liveNode;
}	

int main(int argc, char **argv) {

	int rank, size;

    MPI_Status status;
 
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (size < 2)
    {
        printf("Please run with two processes.\n");fflush(stdout);
        MPI_Finalize();
        return 0;
    }
    if (rank == 0) {

		FILE *inputFILE = fopen(argv[1],"r");
		char *line = NULL;
		size_t len ;
		ssize_t read;
		long noOfJob = 0;
		
		if( inputFILE ==  NULL)
			exit(-2);

		(void) fscanf(inputFILE, "%ld", &noOfJob);
		limit = noOfJob;
		inputArray = (long **) malloc(noOfJob * sizeof(long *));
		
		int temp = 0;
		int i = 0;
		int j = 0;

		for(i = 0; i < noOfJob ; i++) {
			inputArray[i] = (long *)malloc(noOfJob * sizeof(long));
			for(j = 0 ; j < noOfJob ;j++) {
				(void) fscanf(inputFILE, "%ld ", &inputArray[i][j]);
				printf("%ld ", inputArray[i][j] );
			}
			printf("\n"); 
		}

		for (int i = 1; i < size; i++) {
			// send problem matrix to all slave process
			printf("Sending the dataset to the slave %d\n", i); 
			sendDataSet(inputArray, noOfJob, i, 0, MPI_COMM_WORLD);			
		}

		Node *sol = new Node();
		initialize((void *)sol);
		liveNodes.insert(sol);
		bool solutionFound = false;
		// omp_set_num_threads(8);	
		// int numOfThreads;
		int numOfCurPath = 0; // this is to track how many slaves are currently exploring search tree
		int curSlave = 1; // this is used to implement round robin task assignment among slave nodes
		while(solutionFound != true) {
			while(!liveNodes.empty()) { 
				printf("liveNodes size : %ld\n", liveNodes.size());
				// #pragma omp parallel
				// {	numOfThreads = omp_get_num_threads();
					void *n;
					// #pragma omp critical
					// {
						n = chooseBestLiveNode();
					// }
					if(n != NULL) {
						Node *node = (Node *)n;
						if(node->bound < globalBound) {
							// send the node to a slave process
							numOfCurPath++;
							printf("Sending the task to the slave %d\n", curSlave); 
							sendNodeMPI((Node *)n, curSlave, 0, MPI_COMM_WORLD);
							curSlave++;
							if(curSlave >= size){
								curSlave = 1;
								break;
							}
							//branch(n);
						} else {
							printf("+++++++++++++++++++++++++++++++ Node not sent to slaves +++++++++++++++++++++++++++++++\n");
						}
					}
				//}
			}
			if(liveNodes.size() == 0)
				solutionFound = true;
		
			while(numOfCurPath != 0) {

				numOfCurPath--;
				// wait for slave to return the node details
				int dataToRecv;
				MPI_Status st;
				MPI_Recv(&dataToRecv, 1, MPI_INT, MPI_ANY_SOURCE , SIZEMSG, MPI_COMM_WORLD, &status);

				for(int i = 0 ; i < dataToRecv ; i++){
					Node *  sol = new Node();
    				recvNodeMPI(sol,status.MPI_SOURCE,0,MPI_COMM_WORLD,&st);
	    			if( sol->yDone.size() == limit) {
						updateBestSolution(sol);
						if(sol->bound <= globalBound) {
							int bound = sol->bound;
							 MPI_Request request;
							for(int i=1; i < size; i++) {
								MPI_Isend(&bound, 1, MPI_LONG, i, BOUNDUPDATE, MPI_COMM_WORLD, &request);
							}
						}
					} else {
						if(sol->bound < globalBound) {
							solutionFound = false;
							insertLiveNode(sol);
						} else {
							printf("Branch pruned GlobalBound : %ld, SolutionBound : %ld \n",globalBound, sol->bound );
						}
					}
				}
			}
		}

		printf("solution : %ld\n",globalBound );
		long size = currentSol.size();
		for (long it = 0; it < size ; it++) {
			printf("JOB : %ld assigned to PERSON : %ld\n", it, currentSol[it]);
		}

	} else {
		// every slave process receives inputMatrix from master process 
		bool solutionFound = false;
		processDepth = 20;
		recvDataSet(&inputArray, &limit, 0, 0, MPI_COMM_WORLD);
		printf("Receiving the dataset for the slave %d\n", rank); 
		
		// printMatrix(inputArray,limit); // verifying recieved data
		while (true) {
			Node *  node = new Node();
    		recvNodeMPI(node, 0,0,MPI_COMM_WORLD, &status);
    		printf("received task for the slave %d\n", rank); 
    		void *n = (void *)node;
    		if(processDepth <= 0)
    			processDepth = 20;
    		branch(n);
    		sendUpdates();
    		free(node);
		}
		
	}
	
	return 0;
}