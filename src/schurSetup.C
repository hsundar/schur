
#include "schur.h"
#include <iostream>

extern int DOFS_PER_NODE;

void createOuterContext(OuterContext* & ctx) {
  int rank;
  int npes;
  MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
  MPI_Comm_size(PETSC_COMM_WORLD, &npes);

  ctx = new OuterContext;

  ctx->data = NULL;
  ctx->root = NULL;
  ctx->outerMat = PETSC_NULL;
  ctx->outerKsp = PETSC_NULL;
  ctx->outerPC = PETSC_NULL;
  ctx->outerSol = PETSC_NULL;
  ctx->outerRhs = PETSC_NULL;

  createLocalData(ctx->data);

  createRSDtree(ctx->root, rank, npes);

  createOuterMat(ctx);

  createOuterPC(ctx);

  createOuterKsp(ctx);

  MatGetVecs(ctx->outerMat, &(ctx->outerSol), &(ctx->outerRhs));
}

void destroyOuterContext(OuterContext* ctx) {
  if(ctx->outerSol) {
    VecDestroy(ctx->outerSol);
  }
  if(ctx->outerRhs) {
    VecDestroy(ctx->outerRhs);
  }
  if(ctx->outerKsp) {
    KSPDestroy(ctx->outerKsp);
  }
  if(ctx->outerPC) {
    PCDestroy(ctx->outerPC);
  }
  if(ctx->outerMat) {
    MatDestroy(ctx->outerMat);
  }
  if(ctx->root) {
    destroyRSDtree(ctx->root);
  }
  if(ctx->data) {
    destroyLocalData(ctx->data);
  }
  delete ctx;
}

void createLocalData(LocalData* & data) {
  data = new LocalData;

  data->commAll = MPI_COMM_NULL;
  data->commLow = MPI_COMM_NULL;
  data->commHigh = MPI_COMM_NULL;
  data->Kssl = PETSC_NULL;
  data->Kssh = PETSC_NULL;
  data->Ksl = PETSC_NULL;
  data->Ksh = PETSC_NULL;
  data->Kls = PETSC_NULL;
  data->Khs = PETSC_NULL;
  data->Kll = PETSC_NULL;
  data->Khh = PETSC_NULL;
  data->lowSchurMat = PETSC_NULL;
  data->highSchurMat = PETSC_NULL;
  data->lowSchurKsp = PETSC_NULL;
  data->highSchurKsp = PETSC_NULL;
  data->mgObj = PETSC_NULL;

  data->buf1 = new VecBufType1;
  (data->buf1)->inSeq  = PETSC_NULL;
  (data->buf1)->outSeq = PETSC_NULL;

  data->buf2 = new VecBufType1;
  (data->buf2)->inSeq  = PETSC_NULL;
  (data->buf2)->outSeq = PETSC_NULL;

  data->buf3 = new VecBufType1;
  (data->buf3)->inSeq  = PETSC_NULL;
  (data->buf3)->outSeq = PETSC_NULL;

  data->buf4 = new VecBufType2;
  (data->buf4)->rhsKspL  = PETSC_NULL;
  (data->buf4)->rhsKspH = PETSC_NULL;
  (data->buf4)->solKspL  = PETSC_NULL;
  (data->buf4)->solKspH = PETSC_NULL;

  data->buf5 = new VecBufType3;
  (data->buf5)->uL = PETSC_NULL;
  (data->buf5)->uH = PETSC_NULL;
  (data->buf5)->vL = PETSC_NULL;
  (data->buf5)->vH = PETSC_NULL;
  (data->buf5)->wL = PETSC_NULL;
  (data->buf5)->wH = PETSC_NULL;
  (data->buf5)->wSl    = PETSC_NULL;
  (data->buf5)->wSh   = PETSC_NULL;
  (data->buf5)->uStarL   = PETSC_NULL;
  (data->buf5)->uStarH   = PETSC_NULL;
  (data->buf5)->uSinCopy = PETSC_NULL;

  data->buf6 = new VecBufType4;
  (data->buf6)->uSout = PETSC_NULL;
  (data->buf6)->uSl = PETSC_NULL;
  (data->buf6)->uSh = PETSC_NULL;
  (data->buf6)->wSl  = PETSC_NULL;
  (data->buf6)->wSh = PETSC_NULL;
  (data->buf6)->uL     = PETSC_NULL;
  (data->buf6)->uH     = PETSC_NULL;
  (data->buf6)->bSl  = PETSC_NULL;
  (data->buf6)->bSh = PETSC_NULL;
  (data->buf6)->ySl  = PETSC_NULL;
  (data->buf6)->ySh = PETSC_NULL;
  (data->buf6)->cL     = PETSC_NULL;
  (data->buf6)->cH     = PETSC_NULL;
  (data->buf6)->cOl  = PETSC_NULL;
  (data->buf6)->cOh = PETSC_NULL;

  data->buf7 = new VecBufType5;
  (data->buf7)->fStarHcopy = PETSC_NULL;
  (data->buf7)->gS = PETSC_NULL;
  (data->buf7)->fTmpL = PETSC_NULL;
  (data->buf7)->fTmpH = PETSC_NULL;
  (data->buf7)->fL = PETSC_NULL;
  (data->buf7)->fH = PETSC_NULL;
  (data->buf7)->fStarL = PETSC_NULL;
  (data->buf7)->fStarH = PETSC_NULL;
  (data->buf7)->uSl = PETSC_NULL;
  (data->buf7)->uSh = PETSC_NULL;
  (data->buf7)->gL = PETSC_NULL;
  (data->buf7)->gH = PETSC_NULL;

  data->dofsPerNode = DOFS_PER_NODE;
  data->N = 9;
  PetscOptionsGetInt(PETSC_NULL, "-N", &(data->N), PETSC_NULL);
  assert((data->N) >= 9);
  assert((data->dofsPerNode) >= 1);

  data->commAll = PETSC_COMM_WORLD;

  createLowAndHighComms(data);

  createMG(data);

  createLocalMatrices(data);

  createSchurMat(data);

  createInnerKsp(data);
}

void destroyLocalData(LocalData* data) {
  if((data->buf1)->inSeq) {
    VecDestroy((data->buf1)->inSeq);
  }
  if((data->buf1)->outSeq) {
    VecDestroy((data->buf1)->outSeq);
  }
  delete (data->buf1);

  if((data->buf2)->inSeq) {
    VecDestroy((data->buf2)->inSeq);
  }
  if((data->buf2)->outSeq) {
    VecDestroy((data->buf2)->outSeq);
  }
  delete (data->buf2);

  if((data->buf3)->inSeq) {
    VecDestroy((data->buf3)->inSeq);
  }
  if((data->buf3)->outSeq) {
    VecDestroy((data->buf3)->outSeq);
  }
  delete (data->buf3);

  if((data->buf4)->rhsKspL) {
    VecDestroy((data->buf4)->rhsKspL);
  }
  if((data->buf4)->rhsKspH) {
    VecDestroy((data->buf4)->rhsKspH);
  }
  if((data->buf4)->solKspL) {
    VecDestroy((data->buf4)->solKspL);
  }
  if((data->buf4)->solKspH) {
    VecDestroy((data->buf4)->solKspH);
  }
  delete (data->buf4);

  if((data->buf5)->uL) {
    VecDestroy((data->buf5)->uL);
  }
  if((data->buf5)->uH) {
    VecDestroy((data->buf5)->uH);
  }
  if((data->buf5)->vL) {
    VecDestroy((data->buf5)->vL);
  }
  if((data->buf5)->vH) {
    VecDestroy((data->buf5)->vH);
  }
  if((data->buf5)->wL) {
    VecDestroy((data->buf5)->wL);
  }
  if((data->buf5)->wH) {
    VecDestroy((data->buf5)->wH);
  }
  if((data->buf5)->wSl) {
    VecDestroy((data->buf5)->wSl);
  }
  if((data->buf5)->wSh) {
    VecDestroy((data->buf5)->wSh);
  }
  if((data->buf5)->uStarL) {
    VecDestroy((data->buf5)->uStarL);
  }
  if((data->buf5)->uStarH) {
    VecDestroy((data->buf5)->uStarH);
  }
  if((data->buf5)->uSinCopy) {
    VecDestroy((data->buf5)->uSinCopy);
  }
  delete (data->buf5);

  if((data->buf6)->uSout) {
    VecDestroy((data->buf6)->uSout);
  }
  if((data->buf6)->uSl) {
    VecDestroy((data->buf6)->uSl);
  }
  if((data->buf6)->uSh) {
    VecDestroy((data->buf6)->uSh);
  }
  if((data->buf6)->wSl) {
    VecDestroy((data->buf6)->wSl);
  }
  if((data->buf6)->wSh) {
    VecDestroy((data->buf6)->wSh);
  }
  if((data->buf6)->uL) {
    VecDestroy((data->buf6)->uL);
  }
  if((data->buf6)->uH) {
    VecDestroy((data->buf6)->uH);
  }
  if((data->buf6)->bSl) {
    VecDestroy((data->buf6)->bSl);
  }
  if((data->buf6)->bSh) {
    VecDestroy((data->buf6)->bSh);
  }
  if((data->buf6)->ySl) {
    VecDestroy((data->buf6)->ySl);
  }
  if((data->buf6)->ySh) {
    VecDestroy((data->buf6)->ySh);
  }
  if((data->buf6)->cL) {
    VecDestroy((data->buf6)->cL);
  }
  if((data->buf6)->cH) {
    VecDestroy((data->buf6)->cH);
  }
  if((data->buf6)->cOl) {
    VecDestroy((data->buf6)->cOl);
  }
  if((data->buf6)->cOh) {
    VecDestroy((data->buf6)->cOh);
  }
  delete (data->buf6);

  if((data->buf7)->fStarHcopy) {
    VecDestroy((data->buf7)->fStarHcopy);
  }
  if((data->buf7)->gS) {
    VecDestroy((data->buf7)->gS);
  }
  if((data->buf7)->fTmpL) {
    VecDestroy((data->buf7)->fTmpL);
  }
  if((data->buf7)->fTmpH) {
    VecDestroy((data->buf7)->fTmpH);
  }
  if((data->buf7)->fL) {
    VecDestroy((data->buf7)->fL);
  }
  if((data->buf7)->fH) {
    VecDestroy((data->buf7)->fH);
  }
  if((data->buf7)->fStarL) {
    VecDestroy((data->buf7)->fStarL);
  }
  if((data->buf7)->fStarH) {
    VecDestroy((data->buf7)->fStarH);
  }
  if((data->buf7)->uSl) {
    VecDestroy((data->buf7)->uSl);
  }
  if((data->buf7)->uSh) {
    VecDestroy((data->buf7)->uSh);
  }
  if((data->buf7)->gL) {
    VecDestroy((data->buf7)->gL);
  }
  if((data->buf7)->gH) {
    VecDestroy((data->buf7)->gH);
  }
  delete (data->buf7);

  if(data->lowSchurKsp) {
    KSPDestroy(data->lowSchurKsp);
  }
  if(data->highSchurKsp) {
    KSPDestroy(data->highSchurKsp);
  }
  if(data->lowSchurMat) {
    MatDestroy(data->lowSchurMat);
  }
  if(data->highSchurMat) {
    MatDestroy(data->highSchurMat);
  }
  if(data->Kssl) {
    MatDestroy(data->Kssl);
  }
  if(data->Kssh) {
    MatDestroy(data->Kssh);
  }
  if(data->Ksl) {
    MatDestroy(data->Ksl);
  }
  if(data->Ksh) {
    MatDestroy(data->Ksh);
  }
  if(data->Kls) {
    MatDestroy(data->Kls);
  }
  if(data->Khs) {
    MatDestroy(data->Khs);
  }
  if(data->Kll) {
    MatDestroy(data->Kll);
  }
  if(data->Khh) {
    MatDestroy(data->Khh);
  }
  if(data->mgObj) {
    DMMGDestroy(data->mgObj);
  }
  if(data->commLow != MPI_COMM_NULL) {
    MPI_Comm_free(&(data->commLow));
  }
  if(data->commHigh != MPI_COMM_NULL) {
    MPI_Comm_free(&(data->commHigh));
  }

  delete data;
}

void createRSDtree(RSDnode *& root, int rank, int npes) {
  root = new RSDnode;
  root->rankForCurrLevel = rank;
  root->npesForCurrLevel = npes;
  if(npes > 1) {
    if(rank < (npes/2)) {
      createRSDtree(root->child, rank, (npes/2));
    } else {
      createRSDtree(root->child, (rank - (npes/2)), (npes/2));
    }
  } else {
    root->child = NULL;
  }
}

void destroyRSDtree(RSDnode *root) {
  if(root->child) {
    destroyRSDtree(root->child);
  }
  delete root;  
}

void createLowAndHighComms(LocalData* data) {
  int rank, npes;
  MPI_Comm_rank(data->commAll, &rank);
  MPI_Comm_size(data->commAll, &npes);

  MPI_Group groupAll;
  MPI_Comm_group(data->commAll, &groupAll);

  if((rank%2) == 0) {
    if(rank < (npes - 1)) {
      int lowRanks[2];
      lowRanks[0] = rank;
      lowRanks[1] = rank + 1;
      MPI_Group lowGroup;
      MPI_Group_incl(groupAll, 2, lowRanks, &lowGroup);
      MPI_Comm_create(data->commAll, lowGroup, &(data->commLow));
      MPI_Group_free(&lowGroup);
    } else {
      MPI_Comm_create(data->commAll, MPI_GROUP_EMPTY, &(data->commLow));
      assert((data->commLow) == MPI_COMM_NULL);
    }
  } else {
    int highRanks[2];
    highRanks[0] = rank - 1;
    highRanks[1] = rank;
    MPI_Group highGroup;
    MPI_Group_incl(groupAll, 2, highRanks, &highGroup);
    MPI_Comm_create(data->commAll, highGroup, &(data->commHigh));
    MPI_Group_free(&highGroup);
  }

  if((rank%2) == 0) {
    if(rank > 0) {
      int highRanks[2];
      highRanks[0] = rank - 1;
      highRanks[1] = rank;
      MPI_Group highGroup;
      MPI_Group_incl(groupAll, 2, highRanks, &highGroup);
      MPI_Comm_create(data->commAll, highGroup, &(data->commHigh));
      MPI_Group_free(&highGroup);
    } else {
      MPI_Comm_create(data->commAll, MPI_GROUP_EMPTY, &(data->commHigh));
      assert((data->commHigh) == MPI_COMM_NULL);
    }
  } else {
    if(rank < (npes - 1)) {
      int lowRanks[2];
      lowRanks[0] = rank;
      lowRanks[1] = rank + 1;
      MPI_Group lowGroup;
      MPI_Group_incl(groupAll, 2, lowRanks, &lowGroup);
      MPI_Comm_create(data->commAll, lowGroup, &(data->commLow));
      MPI_Group_free(&lowGroup);
    } else {
      MPI_Comm_create(data->commAll, MPI_GROUP_EMPTY, &(data->commLow));
      assert((data->commLow) == MPI_COMM_NULL);
    }
  }

  MPI_Group_free(&groupAll);
}

void createOuterKsp(OuterContext* ctx) {
  KSPCreate((ctx->data)->commAll, &(ctx->outerKsp));
  PetscObjectIncrementTabLevel((PetscObject)(ctx->outerKsp), PETSC_NULL, 0);
  KSPSetType(ctx->outerKsp, KSPFGMRES);
  KSPSetTolerances(ctx->outerKsp, 1.0e-12, 1.0e-12, PETSC_DEFAULT, 50);
  KSPSetPC(ctx->outerKsp, ctx->outerPC);
  KSPSetOptionsPrefix(ctx->outerKsp, "outer_");
  KSPSetFromOptions(ctx->outerKsp);
  KSPSetOperators(ctx->outerKsp, ctx->outerMat,
      ctx->outerMat, SAME_NONZERO_PATTERN);
  KSPSetUp(ctx->outerKsp);
}

void createOuterMat(OuterContext* ctx) {
  int rank;
  MPI_Comm_rank(((ctx->data)->commAll), &rank);

  int onx;
  if(rank == 0) {
    onx = (ctx->data)->N;
  } else {
    onx = ((ctx->data)->N) - 1;
  }

  int locSize = onx*((ctx->data)->N)*((ctx->data)->dofsPerNode);

  Mat mat;
  MatCreateShell(((ctx->data)->commAll), locSize, locSize,
      PETSC_DETERMINE, PETSC_DETERMINE, ctx, &mat);
  MatShellSetOperation(mat, MATOP_MULT, (void(*)(void))(&outerMatMult));

  ctx->outerMat = mat;
}

void createSchurMat(LocalData* data) {
  int rank, npes;
  MPI_Comm_rank((data->commAll), &rank);
  MPI_Comm_size((data->commAll), &npes);

  Mat lowMat = PETSC_NULL;
  Mat highMat = PETSC_NULL;

  int locSize = (data->N)*(data->dofsPerNode);

  if((rank%2) == 0) {
    if(rank < (npes - 1)) {
      MatCreateShell((data->commLow), locSize, locSize,
          PETSC_DETERMINE, PETSC_DETERMINE, data, &lowMat);
    }
  } else {
    MatCreateShell((data->commHigh), 0, 0,
        PETSC_DETERMINE, PETSC_DETERMINE, data, &highMat);
  }

  if((rank%2) == 0) {
    if(rank > 0) {
      MatCreateShell((data->commHigh), 0, 0,
          PETSC_DETERMINE, PETSC_DETERMINE, data, &highMat);
    }
  } else {
    if(rank < (npes - 1)) {
      MatCreateShell((data->commLow), locSize, locSize,
          PETSC_DETERMINE, PETSC_DETERMINE, data, &lowMat);
    }
  }

  if(lowMat) {
    MatShellSetOperation(lowMat, MATOP_MULT, (void(*)(void))(&lowSchurMatMult));
  }

  if(highMat) {
    MatShellSetOperation(highMat, MATOP_MULT, (void(*)(void))(&highSchurMatMult));
  }

  data->lowSchurMat = lowMat;
  data->highSchurMat = highMat;
}

void createMG(LocalData* data) {
  assert(data != NULL);
  int nlevels = 1;
  PetscOptionsGetInt(PETSC_NULL, "-nlevels", &nlevels, PETSC_NULL);
  int coarseSize = 1 + (((data->N) - 1)>>(nlevels - 1));

  int rank;
  MPI_Comm_rank((data->commAll), &rank);
  if(!rank) {
    std::cout<<"nlevels = "<<nlevels<<std::endl;
    std::cout<<"coarseSize = "<<coarseSize<<std::endl;
  }

  DMMGCreate(PETSC_COMM_SELF, -nlevels, PETSC_NULL, &(data->mgObj));
  DMMGSetOptionsPrefix(data->mgObj, "loc_");

  DA da;
  DACreate2d(PETSC_COMM_SELF, DA_NONPERIODIC, DA_STENCIL_BOX, coarseSize, coarseSize,
      PETSC_DECIDE, PETSC_DECIDE, (data->dofsPerNode), 1, PETSC_NULL, PETSC_NULL, &da);
  DMMGSetDM((data->mgObj), (DM)da);
  DADestroy(da);

  DMMGSetKSP((data->mgObj), PETSC_NULL, &computeMGmatrix);
  PetscObjectIncrementTabLevel((PetscObject)(DMMGGetKSP(data->mgObj)), PETSC_NULL, 2);
}

void createOuterPC(OuterContext* ctx) {
  PCCreate(((ctx->data)->commAll), &(ctx->outerPC));
  PCSetType(ctx->outerPC, PCSHELL);
  PCShellSetName(ctx->outerPC, "RSD");
  PCShellSetContext(ctx->outerPC, ctx);
  PCShellSetApply(ctx->outerPC, &outerPCapply);
}

void createInnerKsp(LocalData* data) {
  int rank, npes;
  MPI_Comm_rank(data->commAll, &rank);
  MPI_Comm_size(data->commAll, &npes);

  if((rank%2) == 0) {
    if(rank < (npes - 1)) {
      KSPCreate(data->commLow, &(data->lowSchurKsp));
      PetscObjectIncrementTabLevel((PetscObject)(data->lowSchurKsp), PETSC_NULL, 1);
      KSPSetType(data->lowSchurKsp, KSPFGMRES);
      KSPSetTolerances(data->lowSchurKsp, 1.0e-12, 1.0e-12, PETSC_DEFAULT, 2);
      KSPSetOptionsPrefix(data->lowSchurKsp, "inner_");
      PC pc;
      KSPGetPC(data->lowSchurKsp, &pc);
      PCSetType(pc, PCNONE);
      KSPSetFromOptions(data->lowSchurKsp);
      KSPSetOperators(data->lowSchurKsp, data->lowSchurMat,
          data->lowSchurMat, SAME_NONZERO_PATTERN);
      KSPSetUp(data->lowSchurKsp);
    } else {
      data->lowSchurKsp = PETSC_NULL;
    }
  } else {
    KSPCreate(data->commHigh, &(data->highSchurKsp));
    PetscObjectIncrementTabLevel((PetscObject)(data->highSchurKsp), PETSC_NULL, 1);
    KSPSetType(data->highSchurKsp, KSPFGMRES);
    KSPSetTolerances(data->highSchurKsp, 1.0e-12, 1.0e-12, PETSC_DEFAULT, 2);
    KSPSetOptionsPrefix(data->highSchurKsp, "inner_");
    PC pc;
    KSPGetPC(data->highSchurKsp, &pc);
    PCSetType(pc, PCNONE);
    KSPSetFromOptions(data->highSchurKsp);
    KSPSetOperators(data->highSchurKsp, data->highSchurMat,
        data->highSchurMat, SAME_NONZERO_PATTERN);
    KSPSetUp(data->highSchurKsp);
  }

  if((rank%2) == 0) {
    if(rank > 0) {
      KSPCreate(data->commHigh, &(data->highSchurKsp));
      PetscObjectIncrementTabLevel((PetscObject)(data->highSchurKsp), PETSC_NULL, 1);
      KSPSetType(data->highSchurKsp, KSPFGMRES);
      KSPSetTolerances(data->highSchurKsp, 1.0e-12, 1.0e-12, PETSC_DEFAULT, 2);
      KSPSetOptionsPrefix(data->highSchurKsp, "inner_");
      PC pc;
      KSPGetPC(data->highSchurKsp, &pc);
      PCSetType(pc, PCNONE);
      KSPSetFromOptions(data->highSchurKsp);
      KSPSetOperators(data->highSchurKsp, data->highSchurMat,
          data->highSchurMat, SAME_NONZERO_PATTERN);
      KSPSetUp(data->highSchurKsp);
    } else {
      data->highSchurKsp = PETSC_NULL;
    }
  } else {
    if(rank < (npes - 1)) {
      KSPCreate(data->commLow, &(data->lowSchurKsp));
      PetscObjectIncrementTabLevel((PetscObject)(data->lowSchurKsp), PETSC_NULL, 1);
      KSPSetType(data->lowSchurKsp, KSPFGMRES);
      KSPSetTolerances(data->lowSchurKsp, 1.0e-12, 1.0e-12, PETSC_DEFAULT, 2);
      KSPSetOptionsPrefix(data->lowSchurKsp, "inner_");
      PC pc;
      KSPGetPC(data->lowSchurKsp, &pc);
      PCSetType(pc, PCNONE);
      KSPSetFromOptions(data->lowSchurKsp);
      KSPSetOperators(data->lowSchurKsp, data->lowSchurMat,
          data->lowSchurMat, SAME_NONZERO_PATTERN);
      KSPSetUp(data->lowSchurKsp);
    } else {
      data->lowSchurKsp = PETSC_NULL;
    }
  }
}



