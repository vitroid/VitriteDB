#ifndef _VECTOR_H_MATTO
#define _VECTOR_H_MATTO

#define vX r[0]
#define vY r[1]
#define vZ r[2]

#define vA r[0]
#define vB r[1]
#define vC r[2]
#define vD r[3]

typedef struct 
{
    int n;
    double *r;
}
sVector;

sVector *Vector_Init(int n);
/*for quaternions*/
void quaternion_add(sVector *ans,sVector *x,sVector *y);

#endif
