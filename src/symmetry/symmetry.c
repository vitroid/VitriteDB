/*
 *与えられたグラフの別ラベリング方法を求める。netmatch4.cを改造するとすぐできる。
 main以外はほぼそのまま利用する。Distanceは、同値解を採用するとともに距離0でも終了しないようにする。

置換後のラベルを列挙出力する
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "Mark2.h"

#define min(x,y) (((x)<(y))?(x):(y))
#define MAXNEI 10
#define MAXDIST 24

/*int weight[]={1024,512,256,128,64,32,16,8,4,2,1,};*/
/*int weight[]={10,9,8,7,6,5,4,3,2,1,};*/
/*int weight[]={0,1,2,3,4,5,6,7,8,9};*/
int weight[]={MAP};

typedef struct
{
  int n;
  int *a;
  int *nnei;
  int *nei;
}
sIntMatrix2;

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
int countonly = 0;
int count = 0;
void show(sMark2 *m,FILE *file)
{
  int i;
  fprintf(file,"@FIXA %d %d\n",m->nmax,Mark2_OStackSize(m));
  for(i=0;i<Mark2_OStackSize(m);i++){
    fprintf(file,"%d ",Mark2_OStack(m,i));
  }
  fprintf(file,"\n");
  /*  printf("@FIXA %d %d\n",m->nmax,Mark2_RStackSize(m));
      for(i=0;i<Mark2_RStackSize(m);i++){
      printf("%d ",Mark2_RStack(m,i));
      }
      printf("\n");*/
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
  aa=(int *)a;
  bb=(int *)b;
  return (*aa>*bb)?1:(*aa<*bb)?-1:0;
}

void Distance(sIntMatrix2 *a,
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
  /*次の候補をソートできるようにしてみよう。まず、a側の最後の要素(最終追加要素)のとなりのノードをさがす。となりのノードに対応するb側のノードを見付ける。次の候補を、b側のノードから近い順でソートしなおす。*/
  /*いろいろ試行錯誤したが遅くなるので除去する。*/
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
    /*    for(i=0;i<Mark2_OStackSize(ma);i++){
      printf("%d ",Mark2_OStack(ma,i));
    }
    printf("\n");
    printf("%d %d %d %d %d %d %d %d %d\n",Mark2_OStackSize(ma),j,Mark2_OStackSize(mb),Mark2_OStack(ma,j),Mark2_OStack(mb,j),bneib,alast,aneib,Mark2_QueryOccupied(ma,aneib));*/
    for(i=0;i<Mark2_RStackSize(mb);i++){
      /*printf("%d %d %d %d\n",i,candidate[i],bneib/b->n,j);*/
      candidate[i]+=b->a[candidate[i]+bneib]<<16;
      /*printf("%d\n",candidate[i]);*/
    }
    qsort(candidate,ncandidate,sizeof(int),compare);
    for(i=0;i<Mark2_RStackSize(mb);i++){
      /*printf("%d %d\n",i,candidate[i]);*/
      candidate[i]&=0xffff;
    }
  }
#endif
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
    if(sum+ds<=BestResult){
      if(Mark2_OStackSize(ma)==Mark2_OStackSize(mb)){
	int k;
	int *x=malloc(sizeof(int)*Mark2_OStackSize(ma));
	for(k=0;k<Mark2_OStackSize(ma);k++){
	  int l=Mark2_OStack(ma,k);
	  x[l]=k;
	}
        if ( countonly ){
          count++;
        }
        else{
          for(k=0;k<Mark2_OStackSize(mb);k++){
            printf("%d ",Mark2_OStack(mb,x[k]));
          }
          printf("\n");
        }
	/*
	for(k=0;k<Mark2_OStackSize(mb);k++){
	  printf("%d:%d ",Mark2_OStack(ma,k),Mark2_OStack(mb,k));
	}
	printf("\n");
	*/
	/*show(mb,stderr);*/
	
	BestResult=ds+sum;
      }else{
	Distance(a,ma,b,mb,ds+sum);
      }
    }
    /*↑でマークした以前の状態に戻さなければいけない。どうやればいい？
      markの使い方を工夫する。0で空き、正で周縁、256以上は占有とする。*/
    Mark2_Unoccupy(mb,j);
    UnmarkRim(mb,b,j);
  }
  free(candidate);
}



sIntMatrix2 *IntMatrix2_LoadNGPH(FILE *f)
{
  int i;
  /*!!!注意。*/
  int maxPathLen = 20;
  sIntMatrix2 *a=malloc(sizeof(sIntMatrix2));
  LoadNGPHasDistanceMatrix(f,&a->nnei,&a->nei,&a->a,&a->n,maxPathLen);
  for(i=0;i<(a->n)*(a->n);i++){
    if(a->a[i]>MAXDIST)
      a->a[i]=MAXDIST;
  }
  return a;
}




sIntMatrix2 *Load(FILE *f)
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
      /*必ずしも、距離行列の要素をそのまま使う必要はなく、加重するとより高速に捜せる可能性もある。中心付近の一致度を優先するか、末端部分を優先するかで、結果のラベル順序は変わってくる。*/
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

int main(int argc,char *argv[])
{
  int i;
  sMark2 *m,*mref;
  FILE *f1;
  sIntMatrix2 *a,*ref;
  char buf[1024];
  if(argc!=2 && argc!=3){
    fprintf(stderr,"usage: %s [-c] (@DMTX|@NGPH)\n",argv[0]);
    exit(1);
  }
  if ( argc==3 ){
    if ( strncmp( argv[1], "-c", 2 ) ){
      fprintf(stderr,"usage: %s [-c] (@DMTX|@NGPH)\n",argv[0]);
      exit(1);
    }
    countonly=1;
    f1=fopen(argv[2],"r");
  }
  else{
    f1=fopen(argv[1],"r");
  }
      
  while(NULL!=fgets(buf,sizeof(buf),f1)){
    if(0==strncmp(buf,"@DMTX",5)){
      a=Load(f1);
      break;
    }
    if(0==strncmp(buf,"@NGPH",5)){
      a=IntMatrix2_LoadNGPH(f1);
      break;
    }
  }
  fclose(f1);
  /*  f1=fopen(argv[1],"r");
      while(NULL!=fgets(buf,sizeof(buf),f1)){
      if(0==strncmp(buf,"@DMTX",5)){
      ref=Load(f1);
      break;
    }
  }
  fclose(f1);*/
  ref=a;

  m=Mark2_New(a->n);
  Mark2_Occupy(m,0);
  MarkRim(m,a,0);
  /*a側ははじめから順番を決めておく。*/
  while(Mark2_RStackSize(m)){
    i=Mark2_RStack(m,0);
    Mark2_Occupy(m,i);
    MarkRim(m,a,i);
  }
  mref=Mark2_New(ref->n);
  for(i=0;i<ref->n;i++){
    Mark2_Occupy(mref,i);
    MarkRim(mref,ref,i);
    BestResult=0;
    Distance(a,m,ref,mref,0);
    Mark2_Unoccupy(mref,i);
    UnmarkRim(mref,ref,i);
  }
  if ( countonly ){
    printf("%d\n", count );
  }
  exit(0);
}
