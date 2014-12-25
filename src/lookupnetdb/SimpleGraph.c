#include <stdio.h>
#include <stdlib.h>
#include "SimpleGraph.h"

sSimpleGraph* SimpleGraph_New( int size )
{
    sSimpleGraph* graph;

    graph = malloc( sizeof( sSimpleGraph ) );
    graph->nei  = calloc(size, SIMPLEGRAPH_MAXNEIGHBOR*sizeof(int));
    graph->nnei = calloc(size, sizeof(int));
    graph->count = 0;
    graph->size  = size;
    return graph;
}



void SimpleGraph_Done( sSimpleGraph* graph )
{
    free( graph->nei );
    free( graph->nnei );
    free( graph );
}



void SimpleGraph_Undirect( sSimpleGraph* graph )
{
    int nnei[ graph->size ];
    int i;
    for( i=0; i<graph->size; i++ )
        nnei[i] = graph->nnei[i];
    for( i=0; i<graph->size; i++ ){
        int j;
        for( j=0; j< graph->nnei[i]; j++ ){
            int k = Nei( graph, i, j );
            Nei( graph, k, nnei[k] ) = i;
            nnei[k] ++;
        }
    }
    for( i=0; i<graph->size; i++ )
        graph->nnei[i] = nnei[i];
}



void SimpleGraph_SaveNGPH( sSimpleGraph* graph, FILE* file )
{
    int i;
    fprintf( file, "@CNT0\n%d\n", graph->count );
    fprintf( file, "@NGPH\n%d\n", graph->size );
    for( i = 0 ; i < graph->size; i++ ){
	int j;
	for( j = 0; j < graph->nnei[i]; j++ ){
	    fprintf( file, "%d %d\n", i, Nei( graph, i, j ) );
	}
    }
    fprintf( file, "-1 -1\n" );
}



/*  挿入ソート  */
void __insertsort(int n,int *a) {
    int i,j;
    int x;

    for (i = 1; i < n; i++) {
        x = a[i];
        for (j = i-1; j >= 0 && a[j] > x; j--)
            a[j+1] = a[j];
        a[j+1] = x;
    }
}



BondType* SimpleGraph_ToBondType( sSimpleGraph* graph )
{
    BondType* bond;
    int i;
    
    //if ((bond = (BondType *) malloc(sizeof(BondType) * graph->size)) == NULL)
    //    HeapError();
    bond = (BondType *) malloc(sizeof(BondType) * graph->size);
    for( i=0; i< graph->size; i++ ){
        int j;
        
        bond[i].n = graph->nnei[i];
        //bond[i].to = malloc( sizeof(int) * graph->nnei[i] );
        for( j=0; j < graph->nnei[i]; j++ ){
            bond[i].to[j] = Nei( graph, i, j );
        }
    }

    /* ソート */
    for (i = 0; i < graph->size; i++)
        __insertsort(bond[i].n, bond[i].to);

    return bond;
}



sSimpleGraph* SimpleGraph_LoadNGPH( FILE* file )
{
    sSimpleGraph* graph;
    int size;
    char buf[256];
    
    fgets( buf, sizeof(buf), file );
    size = atoi( buf );
    graph = SimpleGraph_New( size );
    
    while( NULL != fgets( buf, sizeof( buf ), file ) ){
        int x, y;
        
        sscanf( buf, "%d %d", &x, &y );
        if ( x < 0 ) break;

	PushUnique( graph->nei, graph->nnei, graph->size, x, y );
	PushUnique( graph->nei, graph->nnei, graph->size, y, x );
    }
    return graph;
}



void PushUnique( int* nei, int* nnei, int size, int from, int to )
{
  int i;
  for(i=0;i<nnei[from]; i++){
    if ( nei[i*size+from] == to )
      return;
  }
  nei[nnei[from]*size+from] = to;
  nnei[from]++;
}
