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
	
	if (sol->yDone.size() == limit) {
		if(sol->bound < globalBound){
				Evaluation *Eval =  new Evaluation();
				Eval->sol = sol;
				Eval->y = y;
				Eval->x = 0;
				Eval->limit = limit;
				evaluate(Eval);
			} else {
				//printf("Branch pruned GlobalBound : %ld, SolutionBound : %ld \n",globalBound, sol->bound );
			}
	}

	for(long i = 0;  i < limit; i++) {
		if(inputArray[y][i] != -1 && sol->yDone.find(i) == sol->yDone.end()){
			// create new Node which has previous details
			Node * newSol = new Node();
			newSol->bound = sol->bound;
			newSol->globalBound = sol->globalBound;
			newSol->actualCost = sol->actualCost;
			newSol->assignment.resize(limit,-1); 

			for(auto y : sol->yDone) {
				newSol->yDone.insert(y);
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
				//printf("Branch pruned GlobalBound : %ld, SolutionBound : %ld \n",globalBound, newSol->bound );
			}
		}
	}
}

void recursionx(Node *sol, long y, long x, int limit) {
	
	
	//sol->xDone.insert(x);
	sol->actualCost = sol->actualCost + inputArray[y][x];
	sol->bound = sol->actualCost;
	sol->assignment[y] = x;

	for(long yIt = 0; yIt < limit ; yIt++) {
		if (sol->yDone.find(yIt) == sol->yDone.end() ) {
			long miny2x = INF;
			for(long xIt = 0; xIt < limit; xIt++ ) {
				if(sol->yDone.find(xIt) == sol->yDone.end()) {
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


	if (globalBound > sol->globalBound) {
		globalBound = sol->globalBound;
	}
	
	// for (long i = 0; i < sol->assignment.size(); i++) {
	// 	printf("Index %ld : assignment : %ld\n", i, sol->assignment[i]);
	// }

	if(sol->bound < sol->globalBound) {
		if(sol->yDone.size() != limit &&  processDepth != 0) {
			processDepth--;
			branch((void *)sol);
		} else { 
			if(sol->yDone.size() == limit) {
				
					//printf("A solution found on local machine\n");
					set <Node *> tempSet;
					for(auto sendItem : sendSet) {
						if(sol->bound < sendItem->bound) {
							tempSet.insert(sendItem);
						}
					}
					//printf("Number of removed solution : %ld\n",tempSet.size() );
					for(auto removeItem : tempSet) {
						sendSet.erase(removeItem);
					}				
			}
			sendSet.insert(sol); 
		}
	} else {
		//free(sol);
	} 
}

void initialize(void * root) {
	Node *RootSol = (Node *)root;
	RootSol->bound = 0;
	RootSol->globalBound = INF;
	RootSol->actualCost = 0;
	RootSol->assignment.resize(limit,-1);
}

void branch(void * SubPro) {
	Node *SubProNode = (Node *)SubPro;
	long nexty ;
	if(SubProNode->yDone.size() == 0) {
		nexty = 0; 
	}
	else {
		long index = 0;
		while(SubProNode->assignment[index] != -1) {
			index = SubProNode->assignment[index];
		}
		nexty = index;
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
	//printf("Sending %d nodes to the master\n", size); 
	MPI_Send(&size, 1, MPI_INT, 0, SIZEMSG, MPI_COMM_WORLD);
	for(auto sendItem : sendSet){
		sendNodeMPI(sendItem, 0, 0, MPI_COMM_WORLD);
		//free(sendItem);	
	}
	sendSet.clear();
}