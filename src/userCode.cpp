#include "userCode.h"

typedef struct {
	Node *sol;
	long y;
	long x;
	int limit;
} Evaluation;

set <Node *> sendSet;
long updatedBound = INF;

void recursionx(Node *sol, long y, long x, int limit);
void evaluate(void * EvaluationNode);

void recursiony(Node *sol, long y, int limit) {
	sol->yDone.insert(y);

	for(long i = 0;  i < limit; i++) {
		if(sol->xDone.find(i) == sol->xDone.end()){
			// create new Node which has previous details
			Node * newSol = new Node();
			newSol->bound = sol->bound;
			newSol->globalBound = sol->globalBound;
			newSol->actualCost = sol->actualCost;
			newSol->assignment.resize(limit); 

			for(auto y : sol->yDone) {
				newSol->yDone.insert(y);
			}

			for(auto x : sol->xDone) {
				newSol->xDone.insert(x);
			}

			for (int it = 0; it < limit ; it++) {
				newSol->assignment[it] = sol->assignment[it];
			}

			if (globalBound > newSol->globalBound) {
				globalBound = newSol->globalBound;
			}

			

			if(newSol->bound < globalBound){
				Evaluation *Eval =  new Evaluation();
				Eval->sol = newSol;
				Eval->y = y;
				Eval->x = i;
				Eval->limit = limit;
				evaluate(Eval);
			} else {
				printf("Branch pruned GlobalBound : %ld, SolutionBound : %ld \n",globalBound, newSol->bound );
			}
		}
	}
}

void recursionx(Node *sol, long y, long x, int limit) {
	
	sol->xDone.insert(x);
	sol->actualCost = sol->actualCost + inputArray[y][x];
	sol->bound = sol->actualCost;
	sol->assignment[y] = x;

	for(long yIt = 0; yIt < limit ; yIt++) {
		if (sol->yDone.find(yIt) == sol->yDone.end() ) {
			long miny2x = INF;
			for(long xIt = 0; xIt < limit; xIt++ ) {
				if(sol->xDone.find(xIt) == sol->xDone.end()) {
					if(miny2x > inputArray[yIt][xIt]) {
						miny2x = inputArray[yIt][xIt];
					}
				}
			}
			if(miny2x != INF) {
				sol->bound += miny2x;
			}
		}
	}

	// if( sol->yDone.size() == limit){
	// 	updateBestSolution(sol);
	// } else {
	// 	if(sol->bound < globalBound) {
	// 		insertLiveNode(sol);
	// 	} else {
	// 		printf("Branch pruned GlobalBound : %ld, SolutionBound : %ld \n",globalBound, sol->bound );
	// 	}
	// }

	// if(globalBound > updatedBound) {
	// 	globalBound = updatedBound;
	// }

	if (globalBound > sol->globalBound) {
		globalBound = sol->globalBound;
	}

	if(sol->bound < sol->globalBound) {
		if(sol->yDone.size() != limit &&  processDepth != 0) {
			processDepth--;
			branch((void *)sol);
		} else { 
			// send node back to master
			//sendNodeMPI(sol, 0, 0, MPI_COMM_WORLD);
			if(sol->yDone.size() == limit) {
				printf("A solution found on local machine\n");
				set <Node *> tempSet;
				for(auto sendItem : sendSet) {
					if(sol->bound < sendItem->bound) {
						tempSet.insert(sendItem);
					}
				}
				printf("Number of removed solution : %ld\n",tempSet.size() );
				for(auto removeItem : tempSet) {
					sendSet.erase(removeItem);
				}
			}	
				sendSet.insert(sol);
		}
	}
}


void initialize(void * root) {
	Node *RootSol = (Node *)root;
	RootSol->bound = 0;
	RootSol->globalBound = INF;
	RootSol->actualCost = 0;
	RootSol->assignment.resize(limit);
}

void branch(void * SubPro) {
	Node *SubProNode = (Node *)SubPro;
	long nexty = 0;
	for (long i = 0; i < limit ; i++) {
		if(SubProNode->yDone.find(i) == SubProNode->yDone.end()) {
			nexty = i;
			break;
		}
	}
	recursiony(SubProNode,nexty,limit);
}

void evaluate(void * SubPro) {
	Evaluation *Eval = (Evaluation *)SubPro;
	Node *newSol = Eval->sol;
	long y = Eval->y;
	long x = Eval->x;
	long limit = Eval->limit;
	recursionx(newSol,y,x,limit);
}

void sendUpdates() { 
	
	int size = sendSet.size();
	printf("Sending %d nodes to the master\n", size); 
	MPI_Send(&size, 1, MPI_INT, 0, SIZEMSG, MPI_COMM_WORLD);
	for(auto sendItem : sendSet){
		sendNodeMPI(sendItem, 0, 0, MPI_COMM_WORLD);	
	}
	sendSet.clear();
}