/*NetMatch.c: netmatch6$B$r%Y!<%9$K!"HFMQ%i%$%V%i%j2=$r;n$_$k!#:G=*E*$K$O(BAnalysis$B%i%$%V%i%j$N0lIt$H$9$k!#(B*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "Mark2.h"
#include "IntMatrix2.h"

#define min(x,y) (((x)<(y))?(x):(y))

/*int weight[]={1024,512,256,128,64,32,16,8,4,2,1,};*/
/*int weight[]={10,9,8,7,6,5,4,3,2,1,};*/
/*int weight[]={0,1,2,3,4,5,6,7,8,9};*/
/*int weight[]={2,1,0,0,0,0,0,0,0,0,};*/
#ifndef MAP
#define MAP 2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
#endif
int weight[]={MAP};


void IntMatrix2_Done(sIntMatrix2 *a)
{
  free(a->a);
  free(a->nnei);
  free(a->nei);
  free(a);
}

void MarkRim(sMark2 *m,sIntMatrix2 *a,int minj)
{
  int i;
  for(i=0;i<a->nnei[minj];i++){
    int j=a->nei[i*a->n+minj];
    Mark2_Rim(m,j);
  }
}

void UnmarkRim(sMark2 *m,sIntMatrix2 *a,int minj)
{
  int i;
  for(i=0;i<a->nnei[minj];i++){
    int j=a->nei[i*a->n+minj];
    Mark2_Unrim(m,j);
  }
}

int BestResult;
sMark2 *maxo=NULL;

void show(sMark2 *m,FILE *file)
{
  int i;
  fprintf(file,"@FIXA %d %d\n",m->nmax,Mark2_OStackSize(m));
  for(i=0;i<Mark2_OStackSize(m);i++){
    fprintf(file,"%d ",Mark2_OStack(m,i));
  }
  fprintf(file,"\n");
}

void show2(sIntMatrix2 *r,sMark2 *oa,FILE *file)
{
  int i,j;
  for(i=0;i<Mark2_OStackSize(oa);i++){
    for(j=0;j<Mark2_OStackSize(oa);j++){
      fprintf(file,"%d ",r->a[Mark2_OStack(oa,i)*r->n+Mark2_OStack(oa,j)]);
    }
    fprintf(file,"\n");
  }
  fprintf(file,"\n");
}

int checksum(sIntMatrix2 *a,sMark2 *oa,sIntMatrix2 *b,sMark2 *ob)
{
  int i,j;
  int sum=0;
  for(i=0;i<Mark2_OStackSize(oa);i++){
    for(j=i+1;j<Mark2_OStackSize(oa);j++){
      int aa=a->a[Mark2_OStack(oa,i)*a->n+Mark2_OStack(oa,j)];
      int bb=b->a[Mark2_OStack(ob,i)*b->n+Mark2_OStack(ob,j)];
      int d=weight[aa]-weight[bb];
      sum+=d*d;
    }
  }
  return sum;
}

int compare(const void *a, const void *b)
{
  int *aa,*bb;
  aa=a;
  bb=b;
  return (*aa>*bb)?1:(*aa<*bb)?-1:0;
}

/*a,b,ma$B$O;vA0$KM?$($F$*$-!"$=$l$KBP1~$9$k:GNI$N(Bmb$B$r5a$a$k!#(B*/
void Distance(sIntMatrix2 *a,
	      sMark2 *ma,
	      sIntMatrix2 *b,
	      sMark2 *mb,
	      int sum
)
{
  int i;
  int ncandidate,*candidate;
  if(sum>=BestResult){
      int j;
      j = Mark2_OStackSize(mb);
#ifdef DEBUG
      fprintf( stderr, "%d/", j );
#endif
      return;
  }
  ncandidate=Mark2_RStackSize(mb);
  candidate=malloc(sizeof(int)*ncandidate);
  for(i=0;i<Mark2_RStackSize(mb);i++){
    candidate[i]=Mark2_RStack(mb,i);
  }
  /*$B<!$N8uJd$r%=!<%H$G$-$k$h$&$K$7$F$_$h$&!#$^$:!"(Ba$BB&$N:G8e$NMWAG(B($B:G=*DI2CMWAG(B)$B$N$H$J$j$N%N!<%I$r$5$,$9!#$H$J$j$N%N!<%I$KBP1~$9$k(Bb$BB&$N%N!<%I$r8+IU$1$k!#<!$N8uJd$r!"(Bb$BB&$N%N!<%I$+$i6a$$=g$G%=!<%H$7$J$*$9!#(B*/
  /*$B$$$m$$$m;n9T:x8m$7$?$,CY$/$J$k$N$G=|5n$9$k!#(B*/
#undef SORT
#ifdef SORT
  /*if(ncandidate>10)*/
  {
    int alast=Mark2_OStack(ma,Mark2_OStackSize(mb));
    int j,aneib,bneib,base;
    j=0;
    while(!Mark2_QueryOccupied(ma,aneib=a->nei[j*a->n+alast])){
      j++;
    }
#ifdef INDEX
    j=ma->index[aneib];
#else
    j=0;
    while(aneib!=Mark2_OStack(ma,j)){
      j++;
    }
#endif
    bneib=Mark2_OStack(mb,j)*b->n;
    for(i=0;i<Mark2_RStackSize(mb);i++){
      candidate[i]+=b->a[candidate[i]+bneib]<<16;
    }
    qsort(candidate,ncandidate,sizeof(int),compare);
    for(i=0;i<Mark2_RStackSize(mb);i++){
      candidate[i]&=0xffff;
    }
  }
#endif
  for(i=0;i<ncandidate;i++){
    int j=candidate[i];
    int k;
    int ds=0;
    int anode;
    //if(sum>=BestResult)break;
#ifdef DEBUG
    if ( Mark2_OStackSize(mb) < 8 ){
        int j;
        for( j=0; j<Mark2_OStackSize(mb); j++)
            fputc( ' ', stderr );
        fprintf( stderr, "(%d/%d@%d)\n", i+1, ncandidate, BestResult );
    }
#endif
    anode = Mark2_OStack(ma,Mark2_OStackSize(mb));
    Mark2_Occupy(mb,j);
    MarkRim(mb,b,j);
    for(k=0;k<Mark2_OStackSize(mb)-1;k++){
      int delta=weight[a->a[anode*a->n+Mark2_OStack(ma,k)]]-
	weight[b->a[j*b->n+Mark2_OStack(mb,k)]];
      ds+=delta*delta;
    }
    if(sum+ds<BestResult){
      if(Mark2_OStackSize(ma)==Mark2_OStackSize(mb)){
	if(maxo)
	  Mark2_Done(maxo);
	maxo=Mark2_Dup(mb);
	BestResult=ds+sum;
#ifdef DEBUG
	fprintf(stderr,"%d BestResult\n",BestResult);
#endif
      }else{
	Distance(a,ma,b,mb,ds+sum);
      }
    }
    /*
    else{
      int j;
      j = Mark2_OStackSize(mb);
      fprintf( stderr, "%d/", j );
      }*/

    /*$B",$G%^!<%/$7$?0JA0$N>uBV$KLa$5$J$1$l$P$$$1$J$$!#$I$&$d$l$P$$$$!)(B
      mark$B$N;H$$J}$r9)IW$9$k!#(B0$B$G6u$-!"@5$G<~1o!"(B256$B0J>e$O@jM-$H$9$k!#(B*/
    Mark2_Unoccupy(mb,j);
    UnmarkRim(mb,b,j);
  }
  free(candidate);
}

/*Distance$B$O:GNI0lCW$rDI5a$9$k$,!"$3$A$i$OM?$($i$l$?>r7o$K9gCW$9$k$b$N$r$9$Y$FNs5s$9$k!#%"%k%4%j%:%`$O$[$H$s$I0l=o(B*/
void Enumerate(sIntMatrix2 *a,
	      sMark2 *ma,
	      sIntMatrix2 *b,
	      sMark2 *mb,
	      int sum)
{
  int i;
  int ncandidate,*candidate;
  ncandidate=Mark2_RStackSize(mb);
  candidate=malloc(sizeof(int)*ncandidate);
  for(i=0;i<Mark2_RStackSize(mb);i++){
    candidate[i]=Mark2_RStack(mb,i);
  }
  for(i=0;i<ncandidate;i++){
    int j=candidate[i];
    int k;
    int ds=0;
    int anode=Mark2_OStack(ma,Mark2_OStackSize(mb));
    if(sum>BestResult)break;
    Mark2_Occupy(mb,j);
    MarkRim(mb,b,j);
    for(k=0;k<Mark2_OStackSize(mb)-1;k++){
      int delta=weight[a->a[anode*a->n+Mark2_OStack(ma,k)]]-
	weight[b->a[j*b->n+Mark2_OStack(mb,k)]];
      ds+=delta*delta;
    }
    /*
    {
	int k;
	int x[Mark2_OStackSize(ma)];
	for(k=0;k<Mark2_OStackSize(ma);k++){
            int l=Mark2_OStack(ma,k);
            x[l]=k;
	}
	for(k=0;k<Mark2_OStackSize(mb);k++){
            printf( "(%d)%d ", Mark2_OStack(ma,k), Mark2_OStack(mb,k));
	}
	printf( " = %d\n", sum+ds );
    }
    */
    if(sum+ds<=BestResult){
      if(Mark2_OStackSize(ma)==Mark2_OStackSize(mb)){
	int k;
	int x[Mark2_OStackSize(ma)];
        
	for(k=0;k<Mark2_OStackSize(ma);k++){
	  int l=Mark2_OStack(ma,k);
	  x[l]=k;
	}
	for(k=0;k<Mark2_OStackSize(mb);k++){
            printf("%d ",Mark2_OStack(mb,x[k]));
	}
	printf("\n");
      }else{
	Enumerate(a,ma,b,mb,ds+sum);
      }
    }
    /*mark$B$N;H$$J}$r9)IW$9$k!#(B0$B$G6u$-!"@5$G<~1o!"(B256$B0J>e$O@jM-$H$9$k!#(B*/
    Mark2_Unoccupy(mb,j);
    UnmarkRim(mb,b,j);
  }
  free(candidate);
}


sIntMatrix2 *IntMatrix2_LoadNGPH(FILE *f)
{
  int i;
  /*!!!$BCm0U!#(B*/
  int maxPathLen = 10;
  sIntMatrix2 *a=malloc(sizeof(sIntMatrix2));
  LoadNGPHasDistanceMatrix(f,&a->nnei,&a->nei,&a->a,&a->n,maxPathLen);
  for(i=0;i<(a->n)*(a->n);i++){
    if(a->a[i]>maxPathLen)
      a->a[i]=maxPathLen;
  }
  return a;
}



sIntMatrix2 *IntMatrix2_LoadDMTX(FILE *f)
{
  int i,j,n;
  char buf[1024];
  sIntMatrix2 *a;
  fgets(buf,sizeof(buf),f);
  n=atoi(buf);
  a=malloc(sizeof(sIntMatrix2));
  a->a=malloc(sizeof(int)*n*n);
  a->nnei=malloc(sizeof(int)*n);
  a->nei=malloc(sizeof(int)*n*MAXNEI);
  a->n=n;
  for(i=0;i<n;i++){
    a->nnei[i]=0;
  }
  
  for(i=0;i<n;i++){
    char *b=buf;
    fgets(buf,sizeof(buf),f);
    for(j=0;j<n;j++){
      /*$BI,$:$7$b!"5wN%9TNs$NMWAG$r$=$N$^$^;H$&I,MW$O$J$/!"2C=E$9$k$H$h$j9bB.$KA\$;$k2DG=@-$b$"$k!#Cf?4IU6a$N0lCWEY$rM%@h$9$k$+!"KvC<ItJ,$rM%@h$9$k$+$G!"7k2L$N%i%Y%k=g=x$OJQ$o$C$F$/$k!#(B*/
      int v;
      v=atoi(strtok(b," "));
      if(v==1){
	a->nei[a->nnei[i]*a->n+i]=j;
	a->nnei[i]++;
	/*assume symmetric matrix. reasonable
	  a->nei[a->nnei[j]*a->n+j]=i;
	  a->nnei[j]++;*/
      }
      a->a[i*a->n+j]=v;/*weight[v];*/
      b=NULL;
    }
  }
  return a;
}

sIntMatrix2* IntMatrix2_NewAdjMatrix(int size,int* nn, int* n)
{
    sIntMatrix2 *a;
    int i;
    a=malloc(sizeof(sIntMatrix2));
    a->a=calloc(size*size,sizeof(int));
    a->nnei=malloc(sizeof(int)*size);
    a->nei=malloc(sizeof(int)*size*MAXNEI);
    a->n=size;
    for(i=0;i<size;i++){
	int j;
	a->nnei[i]=nn[i];
	for(j=0;j<nn[i];j++){
	    int k;
	    k=a->nei[j*size+i]=n[j*size+i];
	    a->a[i*size+k]=a->a[k*size+i]=1;
	}
    }
    return a;
}


/*2$B$D$N9TNs$r0z?t$H$7!"$=$N=E$J$j$,:GBg$H$J$k%i%Y%kIU$1$G$N%9%3%"$rJV$9(B.*/
/*big$B$KBP$7$F!"(Bsmall$B$r$:$i$7$J$,$i=E$M$F$f$/!#(B*/

int IntMatrix2_NetMatch(sIntMatrix2 *small, sIntMatrix2 *big, int maxscore)
{
    int i;
    sMark2 *msmall,*mbig;
  
    mbig=Mark2_New(big->n);
    Mark2_Occupy(mbig,0);
    MarkRim(mbig,big,0);
    /*big$BB&$O$O$8$a$+$i=gHV$r7h$a$F$*$/!#(B*/
    while(Mark2_RStackSize(mbig)){
	i=Mark2_RStack(mbig,0);
	Mark2_Occupy(mbig,i);
	MarkRim(mbig,big,i);
    }
    msmall=Mark2_New(small->n);
    BestResult=maxscore/*0x7fffffff*/;
    for(i=0;i<small->n;i++){
	Mark2_Occupy(msmall,i);
	MarkRim(msmall,small,i);
	Distance(big,mbig,small,msmall,0);
	Mark2_Unoccupy(msmall,i);
	UnmarkRim(msmall,small,i);
    }
    Mark2_Done(mbig);
    Mark2_Done(msmall);
    return BestResult;
}


