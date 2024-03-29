
#include "mpi.h"
#include <iostream>
#include <cstdlib>
#include "petsc.h"
#include "schur.h"

double** stencil;
int DOFS_PER_NODE;

PetscLogEvent outerKspEvent;
PetscLogEvent rhsEvent;
PetscLogEvent setUpEvent;
PetscCookie rsdCookie;

int main(int argc, char** argv) {
  PetscInitialize(&argc, &argv, "options", PETSC_NULL);

  PetscCookieRegister("RSD", &rsdCookie);
  PetscLogEventRegister("OuterKsp", rsdCookie, &outerKspEvent);
  PetscLogEventRegister("RHS", rsdCookie, &rhsEvent);
  PetscLogEventRegister("RsdSetUp", rsdCookie, &setUpEvent);

  int rank, npes;
  MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
  MPI_Comm_size(PETSC_COMM_WORLD, &npes);

  int N = 9;
  int G = 1;
  int problem = 1;
  PetscOptionsGetInt(PETSC_NULL, "-N", &N, PETSC_NULL);
  PetscOptionsGetInt(PETSC_NULL, "-inner_ksp_max_it", &G, PETSC_NULL);
  PetscOptionsGetInt(PETSC_NULL, "-problem", &problem, PETSC_NULL);
  if(!rank) {
    std::cout<<"N = "<<N<<std::endl;
    std::cout<<"P = "<<npes<<std::endl;
    std::cout<<"G = "<<G<<std::endl;
    std::cout<<"Problem = "<<problem<<std::endl;
  }

  if(problem == 1) {
    createPoissonStencil();
  } else if(problem == 2) {
    createHardStencilType1();
  } else if(problem == 3) {
    createHardStencilType2();
  } else if(problem == 4) {
    createConvectionDiffusionStencil();
  } else if(problem == 5) {
    createLinearElasticMechanicsStencil();
  } else if(problem == 6) {
    createConvectionDiffusionStencil2();
  } else {
    assert(false);
  }

  PetscLogEventBegin(setUpEvent, 0, 0, 0, 0);

  OuterContext* ctx;
  createOuterContext(ctx);

  PetscLogEventEnd(setUpEvent, 0, 0, 0, 0);

  const unsigned int seed = (0x3456782  + (54763*rank));

  PetscRandom rndCtx;
  PetscRandomCreate(PETSC_COMM_WORLD, &rndCtx);
  PetscRandomSetType(rndCtx, PETSCRAND48);
  PetscRandomSetSeed(rndCtx, seed);
  PetscRandomSeed(rndCtx);

  VecSetRandom(ctx->outerSol, rndCtx);

  PetscRandomDestroy(rndCtx);

  zeroBoundary(ctx->data, ctx->outerSol);

  MPI_Barrier(PETSC_COMM_WORLD);
  if(!rank) {
    std::cout<<"Outer MatVec for RHS ..."<<std::endl<<std::endl;
  }
  MPI_Barrier(PETSC_COMM_WORLD);

  PetscLogEventBegin(rhsEvent, 0, 0, 0, 0);

  MatMult(ctx->outerMat, ctx->outerSol, ctx->outerRhs);

  PetscLogEventEnd(rhsEvent, 0, 0, 0, 0);

  VecZeroEntries(ctx->outerSol);

  MPI_Barrier(PETSC_COMM_WORLD);
  if(!rank) {
    std::cout<<"Starting Solve ..."<<std::endl<<std::endl;
  }
  MPI_Barrier(PETSC_COMM_WORLD);

  PetscLogEventBegin(outerKspEvent, 0, 0, 0, 0);

  KSPSolve(ctx->outerKsp, ctx->outerRhs, ctx->outerSol);

  PetscLogEventEnd(outerKspEvent, 0, 0, 0, 0);

  MPI_Barrier(PETSC_COMM_WORLD);
  if(!rank) {
    std::cout<<"Done Solve"<<std::endl<<std::endl;
  }
  MPI_Barrier(PETSC_COMM_WORLD);

  destroyOuterContext(ctx);

  MPI_Barrier(PETSC_COMM_WORLD);
  if(!rank) {
    std::cout<<"Destroyed Outer Context"<<std::endl<<std::endl;
  }
  MPI_Barrier(PETSC_COMM_WORLD);

  destroyStencil();

  MPI_Barrier(PETSC_COMM_WORLD);
  if(!rank) {
    std::cout<<"Done!"<<std::endl<<std::endl;
  }
  MPI_Barrier(PETSC_COMM_WORLD);

  PetscFinalize();

  return 0;
}




