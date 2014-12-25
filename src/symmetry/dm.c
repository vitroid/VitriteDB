#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAXD 0x7fffffff
#define MAXN 10000

/*「アルゴリズムの道具箱」参照*/

int loc[MAXN];
int debug=0;

int pathlen[MAXN];
#define value(x) pathlen[x]

void _swap(int i,int j,int *A)
{
  int temp,ip,jp;
  ip=temp=A[i];
  jp=A[i]=A[j];
  A[j]=temp;
  loc[ip]=j;
  loc[jp]=i;
}

void _upmin(int i,int *A,int n)
{
  int j;
  if(i<0 || i>=n){
    printf("UPMIN::Illegal i=%d for n=%d\n",i,n);
    exit(1);
  }
  if(i==0)return;
  j=(i+1)/2-1;
  if(value(A[j]) > value(A[i])){
    _swap(i,j,A);
    _upmin(j,A,n);
  }
}

void _downmin(int i,int *A,int n)
{
  int j;
  if(i<0 || i>=n){
    printf("DOWNMIN::Illegal i=%d for n=%d\n",i,n);
    exit(1);
  }
  j=2*i+1;
  if(j>=n)return;
  if(j+1<n && value(A[j]) > value(A[j+1]))
    j=j+1;
  if(value(A[j]) < value(A[i])){
    _swap(i,j,A);
    _downmin(j,A,n);
  }
}


void HeapInsert(int i,int *A,int n)
{
  A[n]=i;
  _upmin(n,A,n+1);
}

int HeapDeleteMin(int *A,int n)
{
  int min;
  min=A[0];
  A[0]=A[n-1];
  if(n>1)
    _downmin(0,A,n-1);
  return(min);
}


/*nnei      number of neighbor node
  nei       neighbor node list
  size      number of nodes
  startNode     start node
  pathlen     distance list

  no need to maintain shortest path tree.
 */


/*startNodeから残りのノードまでの距離をすべて求める。*/
void Dijkstra(int *nnei,int *nei,int size,int startNode, int maxPathLen)
{
  int loop;
  int i;
  int heapcount=0;
  int A[MAXN];
  
  for(i=0;i<size;i++){
    loc[i]=-1;
    if(pathlen[i]<maxPathLen){
      loc[i]=heapcount;
      HeapInsert(i,A,heapcount++);
    }
  }
  pathlen[startNode]=0;
  loc[startNode]=0;/*dummy*/
  while(1){
    int j;
    for(i=0;i<nnei[startNode];i++){
      int nextNode=nei[i*size+startNode];
      if(pathlen[nextNode]>pathlen[startNode]+1){
	pathlen[nextNode]=pathlen[startNode]+1;
	if(loc[nextNode]<0){
	  loc[nextNode]=heapcount;
	  HeapInsert(nextNode,A,heapcount++);
	  if(debug){
	    for(j=0;j<heapcount;j++){
	      printf("%d %d %d\n",j,A[j],value(A[j]));
	    }
	    printf("\n");
	  }
	}else{
	  _upmin(loc[nextNode],A,heapcount);
	}
      }
    }
    while(1){
        if(heapcount==0)return;
        startNode=HeapDeleteMin(A,heapcount--);
        if ( pathlen[startNode] < maxPathLen )
            break;
    }
    if(debug){
      printf("Minimal pathlen[%d]=%d\n\n",startNode,value(startNode));
      for(j=0;j<heapcount;j++){
	printf("%d %d %d\n",j,A[j],value(A[j]));
      }
      printf("-----\n");
    }
  }
}

/*NGPHを読みこむ。*/
void LoadNGPH(FILE *file,int **nnei,int **nei,int *size)
{
  char buf[1024];
  int i;
  int row;
  int s  = *size =atoi(fgets(buf,sizeof(buf),file));
  int *nn= *nnei =calloc(s,sizeof(int));
  int *n = *nei  =calloc(s*10,sizeof(int));
  while(NULL!=fgets(buf,sizeof(buf),file)){
    int i,j;
    sscanf(buf,"%d %d\n",&i,&j);
    if(i<0)break;
    n[nn[i]*s+i]=j;
    nn[i]++;
    n[nn[j]*s+j]=i;
    nn[j]++;
  }
}

int* MakeDistanceMatrix(int s,int *nn,int *n, int maxPathLen)
{
  int i;
  int row;
  int* dm=calloc(s*s,sizeof(int));
  /*距離行列を求める際には、できるだけ既存のデータを再利用したい。*/
  for(row=0;row<s;row++){
    for(i=0;i<s;i++){
      if(dm[row*s+i]==0){
	pathlen[i]=maxPathLen;
      }else{
	pathlen[i]=dm[row*s+i];
      }
    }
    Dijkstra(nn,n,s,row, maxPathLen);
    for(i=row;i<s;i++){
	dm[row*s+i]=dm[i*s+row]=pathlen[i];
      /*printf("%d\n",pathlen[i]);*/
    }
  }
  return dm;
}

/*NGPHを読みこみ、距離行列を計算して、それを返す。返り値はNetMatchでの利用に適した形になっているようだ。*/
void LoadNGPHasDistanceMatrix(FILE *file,int **nnei,int **nei,int **dm,int *size, int maxPathLen)
{
    LoadNGPH(file,nnei,nei,size);
    *dm=MakeDistanceMatrix(*size,*nnei,*nei, maxPathLen);
}

      

/*距離行列の要素から、スペクトルを作る。スペクトルは、異なるグラフではほぼ確実に異なるが、運悪く同じスペクトルを持つ場合もありうる。完全な相同性チェックはIsomorph()で行う必要がある。*/
/*距離10以上はカウントしない。*/
#define MAXDIST 10
int comp(const void *a,const void *b)
{
    int* aa;
    int* bb;
    int i;
    aa=(int *)a;
    bb=(int *)b;
    for(i=0;i<MAXDIST;i++){
	if(*aa > *bb)
	    return 1;
	else if(*aa < *bb)
	    return -1;
	aa++;
	bb++;
    }
    return 0;
}

int* DistanceMatrix2Spectrum(int size,int* dm)
{
    int i;
    int* sp=(int *)calloc(size*MAXDIST,sizeof(int));
    int* ps=(int *)calloc(size*MAXDIST,sizeof(int));
    for(i=0;i<size;i++){
	int j;
	for(j=0;j<size;j++){
	    int k;
	    k=dm[i*size+j];
	    if(k<MAXDIST)
		sp[i*MAXDIST+k]++;
	}
    }
    /*it might be slow because element size is big...*/
    qsort(sp,size,MAXDIST*sizeof(int),comp);
    /*transpose*/
    for(i=0;i<size;i++){
	int j;
	for(j=0;j<MAXDIST;j++){
	    ps[j*size+i]=sp[i*MAXDIST+j];
	}
    }
    free(sp);
    return ps;
}


void showSpectrum(int size, int* sp)
{
    int i;
    for(i=0;i<size;i++){
	int j;
	for(j=0;j<MAXDIST;j++){
	    printf("%d ",sp[j*size+i]);
	}
	printf("\n");
    }
    printf("\n");
}

int sizeofSpectrum(int size)
{
    return size*sizeof(int)*MAXDIST;
}

void showDistanceMatrix(int size,int* dm)
{
    int i;
    printf("%d\n",size);
    for(i=0;i<size;i++){
	int j;
	for(j=0;j<size;j++){
	    printf("%d ",dm[j*size+i]);
	}
	printf("\n");
    }
}
