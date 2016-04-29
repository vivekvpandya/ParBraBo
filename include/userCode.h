#ifndef USERCODE_H
#define USERCODE_H

#include <stdio.h>
#include <mpi.h>
#include "dataStructures.h"

// Following definitions will be provided by the framework -------------
extern long globalBound;
extern vector <long> currentSol;
extern long **inputArray;
extern long limit;
extern set <Node *> liveNodes;
extern int processDepth;

// extern void insertLiveNode(Node *solution);
// extern void updateBestSolution(Node *solution);
extern void sendNodeMPI(Node *node, int dest, int tag, MPI_Comm comm);
// Framework definitions ends here -------------------------------------

void initialize(void * root);
void branch(void * SubPro);
void evaluate(void * SubPro);
void sendUpdates();


#endif