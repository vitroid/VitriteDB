#ifndef _NETMATCH_H
#define _NETMATCH_H
#include "IntMatrix2.h"
#include "Mark2.h"

extern int BestResult;
void         IntMatrix2_Done(sIntMatrix2 *a);
sIntMatrix2* IntMatrix2_LoadNGPH(FILE *f);
sIntMatrix2* IntMatrix2_LoadDMTX(FILE *f);
sIntMatrix2* IntMatrix2_NewAdjMatrix(int size,int* nn, int* n);
int          IntMatrix2_NetMatch(sIntMatrix2 *small,
                                 sIntMatrix2 *big, int maxscore);
void Enumerate(sIntMatrix2 *a, sMark2 *ma, sIntMatrix2 *b, sMark2 *mb,
               int sum);
void Distance(sIntMatrix2 *a, sMark2 *ma, sIntMatrix2 *b, sMark2 *mb,
              int sum);
#endif

