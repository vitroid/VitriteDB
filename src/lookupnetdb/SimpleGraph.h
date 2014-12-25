#ifndef _SIMPLEGRAPH_H
#define _SIMPLEGRAPH_H

#include <stdio.h>
#include "Bond.h"

/*used in db_getnext. Should better be used in other routines, too. */

#define SIMPLEGRAPH_MAXNEIGHBOR 10
#define Nei( graph, node, order ) \
     (graph)->nei[ (order) * (graph)->size + (node) ]

typedef struct 
{
    int  size;  /* number of nodes */
    int  count; /* also used as ID */
    int* nnei;
    int* nei;
}
sSimpleGraph;

sSimpleGraph* SimpleGraph_New( int size );
void SimpleGraph_Done( sSimpleGraph* graph );
void SimpleGraph_SaveNGPH( sSimpleGraph* graph, FILE* file );
BondType* SimpleGraph_ToBondType( sSimpleGraph* graph );
sSimpleGraph* SimpleGraph_LoadNGPH( FILE* file );
void SimpleGraph_Undirect( sSimpleGraph* graph );
void PushUnique( int* nei, int* nnei, int size, int from, int to );




#endif
