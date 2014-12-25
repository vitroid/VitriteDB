/*
 *referenceグラフに完全一致する断片を、大きなネットワークの中から数えあげる。もしかしたら時間がかかりすぎるかもしれない。

2つの入力は共に無向グラフ(距離行列だからあたりまえ)
これで抽出したフラグメントの結合の向きを割りだすのはまた別のプログラムで行う。

出力の方法は未定。
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "Mark2.h"
#include "dm.h"

#define min(x,y) (((x)<(y))?(x):(y))
#define MAXNEI 10
#define MAXDIST 12

/*int weight[]={1024,512,256,128,64,32,16,8,4,2,1,};*/
/*int weight[]={10,9,8,7,6,5,4,3,2,1,};*/
int weight[]={0,1,2,3,4,5,6,7,8,9,10,11,12};

/*int weight[]={2,1,0,0,0,0,0,0,0,0,};*/

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

int maxdiff;
/*netmatch4では最小値を求めるのが目的だったので、どんどんしぼりこんでいったが、このプログラムでは最大値以下であればすべて列挙する。*/
sMark2 *maxo=NULL;

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
    if(sum>maxdiff)break;
    Mark2_Occupy(mb,j);
    MarkRim(mb,b,j);
    for(k=0;k<Mark2_OStackSize(mb)-1;k++){
      int delta=weight[a->a[anode*a->n+Mark2_OStack(ma,k)]]-
	weight[b->a[j*b->n+Mark2_OStack(mb,k)]];
      ds+=delta*delta;
    }
    if(sum+ds<=maxdiff){
      if(Mark2_OStackSize(ma)==Mark2_OStackSize(mb)){
	int k;
	int *x=malloc(sizeof(int)*Mark2_OStackSize(ma));
	for(k=0;k<Mark2_OStackSize(ma);k++){
	  int l=Mark2_OStack(ma,k);
	  x[l]=k;
	}
	for(k=0;k<Mark2_OStackSize(mb);k++){
	  printf("%d ",Mark2_OStack(mb,x[k]));
	}
	printf("\n");
	/*
	for(k=0;k<Mark2_OStackSize(mb);k++){
	  printf("%d:%d ",Mark2_OStack(ma,k),Mark2_OStack(mb,k));
	}
	printf("\n");
	*/
	/*show(mb,stderr);*/
	
	/*BestResult=ds+sum;絞りこまないので更新しない。*/
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
  int maxPathLen = 10;
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
  char buf[100];
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
    for(j=0;j<n;j++){
      int v;
      fscanf(f,"%d",&v);
      if(v==1){
	a->nei[a->nnei[i]*a->n+i]=j;
	a->nnei[i]++;
      }
      a->a[i*a->n+j]=v;/*weight[v];*/
    }
  }
  return a;
}

void Done(sIntMatrix2 *a)
{
  free(a->a);
  free(a->nnei);
  free(a->nei);
  free(a);
}


int main(int argc,char *argv[])
{
  int i;
  int count=0;
  sMark2 *m,*mref;
  FILE *f1;
  sIntMatrix2 *a,*ref;
  char buf[20000];
  int gzip;
  if((argc!=3)&&(argc!=4)){
    fprintf(stderr,"usage: %s small big [MaxDiff]\n",argv[0]);
    fprintf(stderr,"structures must be in format @DMTX or @NGPH\n");
    exit(1);
  }
  if(argc==4){
    maxdiff=atoi(argv[3]);
  }else{
    /*extract the exact shape only*/
    maxdiff=0;
  }
  /*最初は参照ネットワークフラグメント*/
  {
      int len = strlen( argv[1] );
      /*
       * ファイル名が.gzで終わる場合は圧縮ファイルとみなす
       */
      if ( 0 == strcmp( &argv[1][len-3], ".gz" ) ){
          char cmd[1024];
          sprintf(cmd, "zcat %s", argv[1] );
          fputs(cmd,stderr);
          f1=popen(cmd,"r");
          gzip=1;
      }
      else{
          f1=fopen(argv[1],"r");
          gzip=0;
      }
  }
  while(NULL!=fgets(buf,sizeof(buf),f1)){
    if(0==strncmp(buf,"@DMTX",5)){
      ref=Load(f1);
      break;
    }
    if(0==strncmp(buf,"@NGPH",5)){
      ref=IntMatrix2_LoadNGPH(f1);
      break;
    }
  }
  if ( gzip )
      pclose( f1 );
  else
      fclose(f1);
  fprintf(stderr,"%d\tMaxDiff\n",maxdiff);
  
  mref=Mark2_New(ref->n);
  Mark2_Occupy(mref,0);
  MarkRim(mref,ref,0);
  /*ref側ははじめから順番を決めておく。*/
  while(Mark2_RStackSize(mref)){
    i=Mark2_RStack(mref,0);
    Mark2_Occupy(mref,i);
    MarkRim(mref,ref,i);
  }
  {
      int len = strlen( argv[2] );
      /*
       * ファイル名が.gzで終わる場合は圧縮ファイルとみなす
       */
      if ( 0 == strcmp( &argv[2][len-3], ".gz" ) ){
          char cmd[1024];
          sprintf(cmd, "zcat %s", argv[2] );
          fputs(cmd,stderr);
          f1=popen(cmd,"r");
          gzip=1;
      }
      else{
          f1=fopen(argv[2],"r");
          gzip=0;
      }
  }
  while(NULL!=fgets(buf,sizeof(buf),f1)){
    int dmtx=0,ngph=0;
    if(0==strncmp(buf,"@DMTX",5))dmtx++;
    if(0==strncmp(buf,"@NGPH",5))ngph++;
    if(dmtx||ngph){
      if(dmtx)a=Load(f1);
      if(ngph)a=IntMatrix2_LoadNGPH(f1);
      dmtx=ngph=0;
      m=Mark2_New(a->n);
      fprintf(stderr,"[%d]",count++);
      printf("@FRAG\n%d %d\n",ref->n,a->n);
      for(i=0;i<a->n;i++){
	Mark2_Occupy(m,i);
	MarkRim(m,a,i);
	Distance(ref,mref,a,m,0);
	Mark2_Unoccupy(m,i);
	UnmarkRim(m,a,i);
      }
      Done(a);
      Mark2_Done(m);
      for(i=0;i<ref->n;i++){
	printf("-1 ");
      }
      printf("\n");
    }
  }
  if ( gzip )
      pclose( f1 );
  else
      fclose(f1);
  exit(0);
}
