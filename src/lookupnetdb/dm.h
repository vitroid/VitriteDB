#ifndef DM_H
#define DM_H
int* MakeDistanceMatrix(int s,int *nn,int *n, int maxPathLen);
void LoadNGPHasDistanceMatrix(FILE *file,int **nnei,int **nei,int **dm,int *size, int maxPathLen);
int* DistanceMatrix2Spectrum(int size,int* dm);
void showSpectrum(int size, int* sp);
int sizeofSpectrum(int size);
void showDistanceMatrix(int size,int* dm);
#endif

