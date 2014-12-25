#ifndef ANALYSIS_INTMATRIX_H
#define ANALYSIS_INTMATRIX_H
typedef struct
{
  int n;
  int *a;
  int *nnei;
  int *nei;
}
sIntMatrix2;
#define MAXNEI 10
#endif
