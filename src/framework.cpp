#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
//#include <omp.h>

#include "userCode.h"

using namespace std;


long globalBound = INF;
vector <long> currentSol;
long **inputArray;
int limit;

//Framework Code starts

set <Node *> liveNodes;

void insertLiveNode(Node *solution) {
	// #pragma omp critical (insert)
	// {
		if(solution->bound < globalBound) {
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

	FILE *inputFILE = fopen(argv[1],"r");
	char *line = NULL;
	size_t len ;
	ssize_t read;
	int noOfJob = 0;
	
	if( inputFILE ==  NULL)
		exit(-2);

	fscanf(inputFILE, "%d", &noOfJob);
	limit = noOfJob;
	inputArray = (long **) malloc(noOfJob * sizeof(long *));
	
	int temp = 0;
	int i = 0;
	int j = 0;

	for(i = 0; i < noOfJob ; i++) {
		inputArray[i] = (long *)malloc(noOfJob * sizeof(long));
		for(j = 0 ; j < noOfJob ;j++) {
			fscanf(inputFILE, "%ld ", &inputArray[i][j]);
			printf("%ld ", inputArray[i][j] );
		}
		printf("\n"); 
	}

	Node *sol = new Node();

	initialize((void *)sol);
	liveNodes.insert(sol);

	// omp_set_num_threads(8);	
	// int numOfThreads;

	while(!liveNodes.empty()) { 
		printf("liveNodes size : %ld\n", liveNodes.size());
		// #pragma omp parallel
		// {	numOfThreads = omp_get_num_threads();
			void *n;
			// #pragma omp critical
			// {
				n = chooseBestLiveNode();
			// }
			if(n != NULL)
				branch(n);
		//}
	}
	

	printf("solution : %ld\n",globalBound );
	long size = currentSol.size();
	for (long it = 0; it < size ; it++) {
		printf("JOB : %ld assigned to PERSON : %ld\n", it, currentSol[it]);
	}

	//printf("%d number of threads \n", numOfThreads );

	return 0;
}