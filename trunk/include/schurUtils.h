
#ifndef __SCHUR_UTILS__
#define __SCHUR_UTILS__

#include "schur.h"

void rsdDirectSolve(RSDnode* root, double* sol, double* rhs);

void createRSDtree(RSDnode* & root, MPI_Comm rootComm);

void deleteRSDtree(RSDnode* & root);

#endif

