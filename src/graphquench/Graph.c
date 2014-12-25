/*
 * Graph.c: Functions for graph, digraph or network.
 *
 * $Id: Graph.c,v 1.1 2007/09/09 14:47:41 matto Exp $
 * $Log: Graph.c,v $
 * Revision 1.1  2007/09/09 14:47:41  matto
 * *** empty log message ***
 *
 * Revision 1.8  2001/09/10  08:42:20  matto
 * o Dijkstra's algorithm is implemented in Graph.c to make distance matrix.
 * o Mcmc.c is modified.
 *
 * Revision 1.7  1999/09/08 09:05:09  matto
 * NGPH is available for reading.
 *
 * Revision 1.6  1999/04/09 03:09:39  matto
 * IntHash is added.
 *
 * Revision 1.5  1998/10/30 01:45:11  matto
 * spell was wrong.
 *
 * Revision 1.4  1998/10/30 01:39:53  matto
 * USE_F2C macro is added for followed _ symbol.
 *
 * Revision 1.3  1998/10/30 01:29:22  matto
 * F2C adds _ after function names.
 *
 * Revision 1.2  1998/10/30 01:09:41  matto
 * some files are older than those in Analysis.ORG. Why?
 *
 * Revision 1.9  1997/05/06 07:25:33  matto
 * Only a trivial change.
 *
 * Revision 1.8  1997/02/21 08:28:50  matto
 * Graph_Distance is added.
 *
 * Revision 1.7  1997/02/14 11:47:10  matto
 * Add comments.
 *
 */
#define _XPG4
#include <stdio.h>
#include <stdlib.h>
#define _INCLUDE_XOPEN_SOURCE
#include <math.h>
#include "Graph.h"
#include "Vector.h"
#include "Ring.h"
/* The direction of the connection is:: y--->x ; y*n+x ; a_{yx}*/
/* e.g. in case of (0 0 1)
                   (0 0 0)
                   (0 0 0),
   the "1" bit is: a_{13}
   direction is  : 1 ==> 3
                 : 0*3+2nd element of one dimensional array
   and the matrix is descripted as
@GRPH
3
001
000
000

The order of IN and OUT is always IN OUT
That's all, folks. */

		   

void debug_alloc(void *g,void *h)
{
    fprintf(stderr,"ALLOC:%x :%x\n",g,h);
}

void debug_free(void *g,void *h)
{
    fprintf(stderr,"FREE :%x :%x\n",g,h);
}

void Graph_Free_npath(sGraph *g)
{
#ifdef DEBUG
    debug_free(g,g->npath);
#endif
    free(g->npath);
    g->npath=NULL;
    g->pmax=-1;
    g->hashkey.d=0;
}

void Graph_Free_nin(sGraph *g)
{
#ifdef DEBUG
    debug_free(g,g->nin);
#endif
    free(g->nin);
    g->nin=NULL;
#ifdef DEBUG
    debug_free(g,g->nout);
#endif
    free(g->nout);
    g->nout=NULL;
    g->maxin=g->maxout=0;
    g->hashkey.d=0;
}

void Graph_Free_dev(sGraph *g)
{
/*
    free(g->devectorr);
    free(g->devectori);
*/
#ifdef DEBUG
    debug_free(g,g->devaluer);
#endif
    free(g->devaluer);
#ifdef DEBUG
    debug_free(g,g->devaluei);
#endif
    free(g->devaluei);
/*  g->devectorr=g->devectori=*/
    g->devaluer=g->devaluei=NULL;
}

void Graph_Init_itemax(sGraph *g)
{    
    g->itemax = -1;
    Graph_Free_npath(g);
}

void Graph_Free_dm(sGraph *g)
{
#ifdef DEBUG
    debug_free(g,g->dm);
#endif
    free(g->dm);
    g->dm=NULL;
    Graph_Init_itemax(g);
    Graph_Free_dev(g);
}

void Graph_Free_am(sGraph *g)
{
#ifdef DEBUG
    debug_free(g,g->am);
#endif
    free(g->am);
    g->am=NULL;
    Graph_Free_dm(g);
    Graph_Free_nin(g);
}

/*Count how many incoming and outgoing bonds are attached to each */
/*node. Result is allocated in the given sGraph structure.*/
/*This routine should be called after making the am[]*/
void Graph_ConnectionDistrib(sGraph *g)
{
/*どっちをinと呼ぶかはここで決めた。横ならびの00100100の1が、出結合を */
/*表す*/
    int i,j,*in,*out,n;
    n = g->nnode;
    in = calloc(n,sizeof(int));
#ifdef DEBUG
    debug_alloc(NULL,in);
#endif
    out = calloc(n,sizeof(int));
#ifdef DEBUG
    debug_alloc(NULL,out);
#endif
    for(i=0;i<n;i++)
      {
	  int sum=0;
	  for(j=0;j<n;j++)
	    {
		out[i] += g->am[i*n+j];
		in[j] += g->am[i*n+j];
	    }
      }
    Graph_Free_nin(g);
    g->maxin = g->maxout = 0;
    for(i=0;i<n;i++)
      {
	  if(out[i]>g->maxout)
	    g->maxout = out[i];
	  if(in[i]>g->maxin)
	    g->maxin = in[i];
      }
    g->nin = calloc(g->maxin+1,sizeof(int));
#ifdef DEBUG
    debug_alloc(g,g->nin);
#endif
    g->nout= calloc(g->maxout+1,sizeof(int));
#ifdef DEBUG
    debug_alloc(g,g->nout);
#endif
    for(i=0;i<n;i++)
      {
	  g->nout[out[i]]++;
	  g->nin[in[i]]++;
      }
#ifdef DEBUG
    debug_free(NULL,in);
#endif
    free(in);
#ifdef DEBUG
    debug_free(NULL,out);
#endif
    free(out);
}    

/*Path length distribution of a given sGraph structure is calculated */
/*and attached in it.*/
/*This routine should be called after calculating the dm[]*/
void Graph_PathLenDistrib(sGraph *g,int maxpath)
{
    int i;
    int n=g->nnode;
    Graph_Free_npath(g);
    /*余計目に領域を確保しておく。*/
    g->npath = calloc(maxpath+1,sizeof(int));
#ifdef DEBUG
    debug_alloc(g,g->npath);
#endif
    for(i=0;i<n*n;i++)
      {
	  g->npath[g->dm[i]]++;
      }
    for(i=maxpath;i>=0;i--)
      {
	  if(g->npath[i])
	    {
		g->pmax = i;
		break;
	    }
      }
}

/*boolean matrix multiplier*/
void bmul(int n,bool *a,bool *b,bool *answer)
{
    int i,j,k,ij,jk;
    for(i=0;i<n;i++)
      {
	  for(j=0;j<n;j++)
	    {
		bool flag=0;
		for(k=0;k<n;k++)
		  if(a[i*n+k]&&b[k*n+j])
		    {
			flag=1;
			break;
		    }
		answer[i*n+j]=flag;
	    }
      }
}

/*distance matrix between the nodes of the given sGraph structure is */
/*calculated and attached in it. */
int DistanceMatrix(sGraph *g,int maxpath)
{
    bool *current,*next,*tmp;
    int i,loop;
    int count;
    int n;
    
    n = g->nnode;
    current = malloc(sizeof(bool)*n*n);
#ifdef DEBUG
    debug_alloc(NULL,current);
#endif
    next    = malloc(sizeof(bool)*n*n);
#ifdef DEBUG
    debug_alloc(NULL,next);
#endif
    Graph_Free_dm(g);
    g->dm = calloc(n*n,sizeof(int));
#ifdef DEBUG
    debug_alloc(g,g->dm);
#endif
    for(i=0;i<n*n;i++)
      {
	  current[i]=g->am[i];
          g->dm[i]=0;
      }
    g->pmax=-1;
    for(loop=1;loop<=maxpath;loop++)
      {
	  int x,y;
	  count=0;
	  for(x=0;x<n;x++)
	    for(y=0;y<n;y++)
              if(x!=y)
                {
                    int p=x*n+y;
                    if(g->dm[p]==0)
                      {
                          if(current[p])
			    {
				g->dm[p]=loop;
				count++;
			    }
		      }
                }
/*	  fprintf(stderr,"%d[%d]",loop,count);*/
	  if(count==0)
	    break;
	  bmul(n,g->am,current,next);
	  tmp = next;
	  next = current;
	  current = tmp;
      }
#ifdef DEBUG
    debug_free(NULL,current);
    debug_free(NULL,next);
#endif
    free(current);
    free(next);
    g->itemax = loop;
    return count;
}

/*
 * FastDistanceMatrix
 *
 * Distance matrix is calculated more quickly by squaring the adjacent 
 * matrix again and again. It is faster when the adjacent matrix is
 dense and the searching path is long. See the benchmark file.
 */

void isq(int n,int *a,int *answer,int *max)
{
    int i,j,k;
    for(i=0;i<n;i++)
      for(j=0;j<n;j++)
	if(i!=j)
	  {
	      if(a[i*n+j]==0)
		{
		    int min=65535;
		    for(k=0;k<n;k++)
		      {
			  int x,y,z;
			  x = a[i*n+k];
			  y = a[k*n+j];
			  if(x&&y)
			    {
				z = x+y;
				if(z<min)
				  min=z;
			    }
		      }
		    if(min!=65535)
		      {
			  answer[i*n+j] = min;
			  if(*max<min)
			    *max=min;
		      }
		    else answer[i*n+j]=0;
		}
	      else
		answer[i*n+j]=a[i*n+j];
	  }
	else
	  answer[i*n+j]=0;
}

void FastDistanceMatrix(sGraph *g,int maxpath)
{
    int i,loop;
    int n;
    int max;
    int *newdm,*tmp;
    
    n = g->nnode;
    Graph_Free_dm(g);
    g->dm = calloc(n*n,sizeof(int));
#ifdef DEBUG
    debug_alloc(g,g->dm);
#endif
    newdm = malloc(sizeof(int)*n*n);
#ifdef DEBUG
    debug_alloc(NULL,newdm);
#endif
    for(i=0;i<n*n;i++)
      g->dm[i]=g->am[i];
    
    max=0;
    for(loop=1;loop<maxpath;loop+=loop)
      {
	  isq(n,g->dm,newdm,&max);
	  tmp=g->dm;
	  g->dm=newdm;
	  newdm = tmp;
/*	  Graph_Print(g,stdout);*/
	  if(max<loop)
	    break;
      }
#ifdef DEBUG
    debug_free(NULL,newdm);
#endif
    free(newdm);
    g->itemax = loop;
}

/*
 * FasterDistanceMatrix
 *
 * When the adjacent matrix is very sparse, it is better to search
 paths directly than to multiply the adjacent matrix again and
 again. See the benchmark file.*/

/*OBSOLETE. Use DistanceMatrix/dm.c which uses Dijkstra's Algorithm*/
void FasterDistanceMatrix(sGraph *g,int maxpath)
{
    int *newdm;
    int *nnei;
    int *nei;
    int n;
    int i,j,k,l,m;
    int dk,dm;
    int nj,nl;
    int path;
    int loop;
    int count;
    
    n = g->nnode;
    Graph_Free_dm(g);
    g->dm = calloc(n*n,sizeof(int));
#ifdef DEBUG
    debug_alloc(g,g->dm);
#endif
    newdm = malloc(sizeof(int)*n*n);
#ifdef DEBUG
    debug_alloc(NULL,newdm);
#endif
    for(i=0;i<n*n;i++)
      g->dm[i] = newdm[i] = g->am[i];
    nei = malloc(sizeof(int)*n*n);
#ifdef DEBUG
    debug_alloc(NULL,nei);
#endif
    nnei = malloc(sizeof(int)*n);
#ifdef DEBUG
    debug_alloc(NULL,nnei);
#endif
    
/*  for(loop=1;loop<maxpath;loop+=loop)どうも変なので以下に変更平成８年１１月２７日(水)*/
    for(loop=2;;loop+=loop)
      {
	  /*make neighbor list*/
	  count=0;
	  for(i=0;i<n;i++)
	    {
		nnei[i]=0;
		for(j=0;j<n;j++)
		  if(g->dm[i*n+j])
		    {
			nei[i*n+nnei[i]]=j;
			nnei[i]++;
		    }
	    }
	  for(i=0;i<n;i++)
	    {
		nj=nnei[i];
		for(j=0;j<nj;j++)
		  {
		      k = nei[i*n+j];
		      dk = g->dm[i*n+k];
		      nl = nnei[k];
		      for(l=0;l<nl;l++)
			{
			    m = nei[k*n+l];
			    dm = g->dm[k*n+m];
			    path=dk+dm;
			    if(newdm[i*n+m]==0)
			      {
				  if(i!=m)
				    {
					newdm[i*n+m]=path;
					count++;
				    }
			      }
			    else
			      if(newdm[i*n+m]>path)
				newdm[i*n+m]=path;
			}
		  }
	    }
	  memcpy(g->dm,newdm,sizeof(int)*n*n);
/*	  Graph_Print(g,stdout);*/
	  if(loop>maxpath)break;
	  if(count==0)break;
      }
#ifdef DEBUG
    debug_free(NULL,nnei);
#endif
    free(nnei);
#ifdef DEBUG
    debug_free(NULL,nei);
#endif
    free(nei);
#ifdef DEBUG
    debug_free(NULL,newdm);
#endif
    free(newdm);
    g->itemax = loop;
}



void *memdup(void *a,size_t s)
{
    void *m;
    m = malloc(s);
#ifdef DEBUG
    debug_alloc(NULL,m);
#endif
    memcpy(m,a,s);
    return m;
}

/* allocate the memory and duplicate the graph*/
sGraph *Graph_Duplicate(sGraph *g)
{
    sGraph *newg;
    int n;
    newg = calloc(1,sizeof(sGraph));
#ifdef DEBUG
    debug_alloc(NULL,newg);
#endif
/*  newg->next = NULL;*/
    newg->graphtype= g->graphtype;
    n = newg->nnode    = g->nnode;
    newg->nbond    = g->nbond;
    newg->itemax  = g->itemax;
    newg->pmax   = g->pmax;
    newg->am = memdup(g->am,sizeof(bool)*n*n);
    if(g->dm!=NULL)
      newg->dm = memdup(g->dm,sizeof(int)*n*n);
    newg->npath = NULL;
    if(g->devaluer!=NULL)
      {
	  newg->devaluer = memdup(g->devaluer,sizeof(double)*n);
	  newg->devaluei = memdup(g->devaluei,sizeof(double)*n);
      }
/*
    if(g->devectorr!=NULL)
      {
	  newg->devectorr = memdup(g->devectorr,sizeof(double)*n*(n+1));
	  newg->devectori = memdup(g->devectori,sizeof(double)*n*(n+1));
      }
*/
    return newg;
}

/*convert the digraph to undirected graph*/
sGraph *Digraph2Graph(sGraph *g)
{
    sGraph *newg;
    int x,y,n,xy,yx;
    newg = calloc(1,sizeof(sGraph));
#ifdef DEBUG
    debug_alloc(NULL,newg);
#endif
/*  newg->next = NULL;*/
    newg->graphtype=GRAPH;
    n=newg->nnode = g->nnode;
/*  Graph_Free_am(newg);*/
    newg->am = memdup(g->am,sizeof(bool)*n*n);
    newg->nbond = 0;
    for(x=0;x<n-1;x++)
      for(y=x+1;y<n;y++)
	{
	    xy=x*n+y;
	    yx=x+y*n;
	    newg->nbond += newg->am[xy] = newg->am[yx] = 
	      (g->am[xy]||g->am[yx]);
	}
    newg->nbond *= 2;
    return newg;
}

/*convert the directed graph(digraph) to "Go-back" undirected */
/*graph. This routine is developped specifically for water network.*/
sGraph *Digraph2Goback(sGraph *g)
{
    sGraph *newg;
    int x,y,n,xy,yx,n2,x2,y2;
    newg = calloc(1,sizeof(sGraph));
#ifdef DEBUG
    debug_alloc(NULL,newg);
#endif
/*  newg->next = NULL;*/
    newg->graphtype=GOBACK;
    n = g->nnode;
    n2=newg->nnode = n*2;
    /*0:out 1:in 2:out 3:in ... */
/*  Graph_Free_am(newg);*/
    newg->am = memdup(g->am,sizeof(bool)*n2*n2);
    newg->nbond = g->nbond*2;
    for(x=x2=0;x<n;x++,x2+=2)
      for(y=y2=0;y<n;y++,y2+=2)
	{
	    /* y--->x ; y*n+x */
	    newg->am[y2*n2+x2]=newg->am[x2*n2+y2]=0;
	    newg->am[(y2+1)*n2+x2+1]=newg->am[(x2+1)*n2+y2+1]=0;
	    newg->am[y2*n2+x2+1]=newg->am[(x2+1)*n2+y2]=g->am[y*n+x];
	    newg->am[(y2+1)*n2+x2]=newg->am[x2*n2+y2+1]=g->am[x*n+y];
	}
    return newg;
}

/*free the sGraph structure*/
void Graph_Done(sGraph *g)
{
    Graph_Free_am(g);
#ifdef DEBUG
    debug_free(NULL,g);
#endif
    free(g);
}
    
/*give mark list and make subset of the given g*/
sGraph *Graph_MarkedSubset(sGraph *g,int nmark,int *marked)
{
  int x,y,xx,yy;
  
  sGraph *newg = calloc(1,sizeof(sGraph));
  newg->graphtype = g->graphtype;
  newg->nnode     = nmark;
  newg->nbond     = 0;
/*  Graph_Free_am(newg);*/
  newg->am        = (bool *)malloc(sizeof(bool)*nmark*nmark);
  for(xx=0;xx<nmark;xx++){
    x=marked[xx];
    for(yy=0;yy<nmark;yy++){
      y=marked[yy];
      newg->am[xx*nmark+yy] = g->am[x*g->nnode+y];
      newg->nbond += g->am[x*g->nnode+y];
    }
  }
  return newg;
}

/* Extract a subset graph from the graph "g", by referring the distance */
/* matrix of the graph "ref". "center" is the central node of the */
/* subset graph, and "path" is the path length from the central node.*/
sGraph *Graph_Subset(sGraph *g,sGraph *ref,int center,int path)
{
    bool *mark;
    int x,y,xx,yy;
    
    sGraph *newg;
    int n=g->nnode,nmark;
    mark = calloc(g->nnode,sizeof(bool));
#ifdef DEBUG
    debug_alloc(NULL,mark);
#endif
    if(ref->itemax<path)
      FasterDistanceMatrix(ref,path);
    nmark=0;
    for(x=0;x<n;x++)
      {
	  int distance=ref->dm[x*n+center];
	  if((distance>0)&&(distance<=path))
	    {
		nmark ++;
		mark[x] = 1;
	    }
      }
    mark[center]=1;
    nmark++;
    
    newg = calloc(1,sizeof(sGraph));
#ifdef DEBUG
    debug_alloc(NULL,newg);
#endif
    newg->graphtype = g->graphtype;
    newg->nnode     = nmark;
    newg->nbond     = 0;
/*  Graph_Free_am(newg);*/
    /*平成13年1月25日(木)距離行列には、元の行列から切りだしたものをそのまま入れておく。つまり、Subset自身の距離行列ではない。*/
    fprintf(stderr,"nmark:%d\n",nmark);
    
    newg->am        = (bool *)malloc(sizeof(bool)*nmark*nmark);
    newg->dm        = (int *)malloc(sizeof(int)*nmark*nmark);
#ifdef DEBUG
    debug_alloc(newg,newg->am);
#endif
    xx=0;
    for(x=0;x<n;x++)
      if(mark[x])
	{
            yy=0;
            for(y=0;y<n;y++)
	      if(mark[y])
		{
		    newg->am[xx*nmark+yy] = g->am[x*n+y];
		    newg->dm[xx*nmark+yy] = g->dm[x*n+y];
		    newg->nbond += g->am[x*n+y];
		    yy++;
		}
	    xx++;
	}
#ifdef DEBUG
    debug_free(NULL,mark);
#endif
    free(mark);
    return newg;
}

/*Calculate the "Eigenvalue" of the given graph "g" to identify the */
/*graph topology. "Eigenvalue" is not the eigenvalue in mathematical */
/*sense. Calculated "Eigenvalue"s are attached in sGraph structure.*/
/*int Graph_EigenValue(sGraph *g) is separated into GraphDB.c 平成13年1月25日(木)*/

/*allocate and load a graph from file*/
sGraph *Graph_Load(FILE *input)
{
    char buf[65536];
    int n,id;
    sGraph *g;
    int x,y;
    
    g = calloc(1,sizeof(sGraph));
#ifdef DEBUG
    debug_alloc(NULL,g);
#endif
    fgets(buf,sizeof(buf),input);
    sscanf(buf,"%d",&n);
    g->nnode = n;

    g->am = calloc(n*n,sizeof(bool));
#ifdef DEBUG
    debug_alloc(g,g->am);
#endif

    g->nbond=0;
    for(y=0;y<n;y++)
      {
          fgets(buf,sizeof(buf),input);
          for(x=0;x<n;x++)
            {
                if(buf[x]=='\0')break;
                g->am[y*n+x] = (buf[x]=='1');
                g->nbond += (buf[x]=='1');
            }
      }

    g->graphtype = GRAPH;
    for(y=0;y<n-1;y++)
      for(x=y+1;x<n;x++)
        if(g->am[x*n+y]!=g->am[y*n+x])
          {
              g->graphtype = DIGRAPH;
              return g;
          }
    return g;
}

/*allocate and load a graph from file*/
sGraph *Graph_Load_NGPH(FILE *input)
{
    char buf[65536];
    int n,id;
    sGraph *g;
    int x,y;
    
    g = calloc(1,sizeof(sGraph));
#ifdef DEBUG
    debug_alloc(NULL,g);
#endif
    fgets(buf,sizeof(buf),input);
    sscanf(buf,"%d",&n);
    g->nnode = n;

    g->am = calloc(n*n,sizeof(bool));
#ifdef DEBUG
    debug_alloc(g,g->am);
#endif

    g->nbond=0;
    while(1){
      fgets(buf,sizeof(buf),input);
      sscanf(buf,"%d %d\n",&y,&x);
      if(y<0)break;
      g->am[y*n+x] = 1;
      g->nbond ++;
    }

    g->graphtype = GRAPH;
    for(y=0;y<n-1;y++)
      for(x=y+1;x<n;x++)
        if(g->am[x*n+y]!=g->am[y*n+x])
          {
              g->graphtype = DIGRAPH;
              return g;
          }
    return g;
}

/*allocate an initialized graph*/
sGraph *Graph_Init(int n)
{
    int id;
    sGraph *g;
    int x,y;
    
    g = calloc(1,sizeof(sGraph));
    g->nnode = n;
    g->am = calloc(n*n,sizeof(bool));
    g->nbond=0;
    g->graphtype = GRAPH;
    return g;
}

/*print the sGraph structure*/
void Graph_Print(sGraph *g,FILE *file)
{
    int n,x,y;
    n = g->nnode;
    fprintf(file,"%d graphtype\n",g->graphtype);
    fprintf(file,"%d nnode\n",n);
    fprintf(file,"%d nbond\n",g->nbond);
    fprintf(file,"%d max iteration\n",g->itemax);
    fprintf(file,"%d max path\n",g->pmax);
    fprintf(file,"%d hashkey\n",g->hashkey.ui);
/*  fprintf(file,"%d count\n",g->count);*/
    fprintf(file,"# Adjacency Matrix\n@GRPH\n%d\n",g->nnode);
    for(y=0;y<n;y++)
      {
          for(x=0;x<n;x++)
            fprintf(file,"%d",g->am[y*n+x]);
          fputc('\n',file);
      }
    if(g->dm!=NULL)
      {
          fprintf(file,"# Distance Matrix in @DMTX\n");
	  fprintf(file,"@DMTX\n%d\n",n);
          for(y=0;y<n;y++)
            {
                for(x=0;x<n;x++)
                  fprintf(file,"%d ",g->dm[y*n+x]);
                fputc('\n',file);
            }
      }
    if(g->npath!=NULL)
      {
          fprintf(file,"# Path Number\n");
          for(x=0;x<=g->pmax;x++)
            fprintf(file,"%d %d\n",x,g->npath[x]);
      }
    if(g->nin!=NULL)
      {
          fprintf(file,"# Incoming Bond\n");
          for(x=0;x<=g->maxin;x++)
            fprintf(file,"%d %d\n",x,g->nin[x]);
      }
    if(g->nout!=NULL)
      {
          fprintf(file,"# Outgoing Bond\n");
          for(x=0;x<=g->maxout;x++)
            fprintf(file,"%d %d\n",x,g->nout[x]);
      }
    if(g->devaluer!=NULL)
      {
          fprintf(file,"# Distance Matrix Eigen Values\n");
          for(x=0;x<n;x++)
            fprintf(file,"%24.17e %24.17e\n",g->devaluer[x],g->devaluei[x]);
      }
/*    if(g->devectorr!=NULL)
      {
          fprintf(file,"# Distance Matrix Eigen Vectors\n");
          for(y=0;y<n;y++)
            {
                for(x=0;x<n;x++)
                  fprintf(file,"(%f,%f) ",g->devectorr[y*n+x],g->devectori[y*n+x]);
                fputc('\n',file);
            }
      }
*/
}

/*print minimal info of a graph*/
void Graph_miniPrint(sGraph *g,FILE *file)
{
    int n,x,y;
    n = g->nnode;
    if(g->dm!=NULL)
      {
          fprintf(file,"# Distance Matrix\n");
          for(y=0;y<n;y++)
            {
                for(x=0;x<n;x++)
                  fprintf(file,"%d ",g->dm[y*n+x]);
                fputc('\n',file);
            }
      }
}

/*save a graph in the file*/
void Graph_Save(sGraph *g,FILE *file)
{
    int n,x,y;
    n = g->nnode;
    fprintf(file,"@GRPH\n");
    fprintf(file,"%d\n",n);
    for(y=0;y<n;y++)
      {
          for(x=0;x<n;x++)
            fprintf(file,"%d",g->am[y*n+x]);
          fputc('\n',file);
      }
}

/*save a graph in the file*/
void Graph_Save_NGPH(sGraph *g,FILE *file)
{
    int n,x,y;
    n = g->nnode;
    fprintf(file,"@NGPH\n");
    fprintf(file,"%d\n",n);
    for(y=0;y<n;y++)
        for(x=0;x<n;x++)
            if ( g->am[y*n+x] )
                fprintf( file, "%d %d\n", y, x );
    fprintf( file, "%d %d\n", -1, -1 );
}

/*save a @NNBR(number of neighbor node) information in a file*/
void Graph_Save_NNBR(sGraph *g,FILE *file)
{
    int n,x,y;
    n = g->nnode;
    fprintf(file,"@NNBR\n");
    fprintf(file,"%d\n",n);
    for(y=0;y<n;y++)
      {
	  int ci,co;
	  ci=co=0;
	  for(x=0;x<n;x++)
            {
		co += g->am[y*n+x];
		ci += g->am[x*n+y];
	    }
	  fprintf(file,"%d %d\n",ci,co);
      }
}

/*quicksort array of double*/
int double_array_compare(int top,int bottom,double *key1,double *key2)
{
    if(fabs(key1[top]-key1[bottom])<1e-11)
      {
	  if(key2[top]>key2[bottom])
	    return 1;
	  else
	    return -1;
      }
    else
      if(key1[top]>key1[bottom])
	return 1;
      else
	return -1;
}

void quicksort(int top,int bottom,double *key1,double *key2)
{
    int t,b,dt,db,i;
    if(top>=bottom)
      return;
    t=top;
    b=bottom;
    dt=1;
    db=0;
    while(t!=b)
      {
	  if(double_array_compare(t,b,key1,key2)>0)
	    {
		double tmp;
		tmp = key1[t];
		key1[t]=key1[b];
		key1[b]=tmp;
		tmp = key2[t];
		key2[t]=key2[b];
		key2[b]=tmp;
		dt=1-dt;
		db=1-db;
	    }
	  t+=dt;
	  b-=db;
      }
    quicksort(top,t-1,key1,key2);
    quicksort(t+1,bottom,key1,key2);
}

/*are the two graphs are isomorphic?*/


/*2つのグラフのハミング距離を計算する。henkan01には、グラフ0の各節点を、 */
/*グラフ1のどの節点と対応させるか、変換表を与える*/
int Graph_Distance(sGraph *g0,sGraph *g1,int *henkan01)
{
    int n=g0->nnode;
    int i,j;
    int sum=0;
    for(i=0;i<n;i++)
      for(j=0;j<n;j++)
	sum+=(g0->am[i*n+j]!=g1->am[henkan01[i]*n+henkan01[j]]);
    return sum;
}

