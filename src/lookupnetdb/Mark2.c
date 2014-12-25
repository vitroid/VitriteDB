#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Mark2.h"

/*2状態をとるmark。クラスタの周縁の処理に特化する。下8ビットは、周縁かどうかを表し、第8ビット(256)がセットされていれば、マークされているものとして扱う。2状態の番号付けは、0が周縁、1が占有。*/

sMark2 *Mark2_New(int n)
{
  int i;
  sMark2 *m=malloc(sizeof(sMark2));
  m->nmax=n;
  m->mark=calloc(n,sizeof(int));
  m->stack=malloc(sizeof(int)*n*2);
#ifdef INDEX
  m->index=malloc(sizeof(int)*n);
#endif
  m->sp[0]=m->sp[1]=0;
  return m;
}

sMark2 *Mark2_Dup(sMark2 *m)
{
  int i;
  sMark2 *a=Mark2_New(m->nmax);
  a->sp[0]=m->sp[0];
  a->sp[1]=m->sp[1];
  for(i=0;i<m->nmax;i++){
    a->mark[i]=m->mark[i];
  }
  for(i=0;i<Mark2_OStackSize(m);i++){
    Mark2_OStack(a,i)=Mark2_OStack(m,i);
#ifdef INDEX
    a->index[Mark2_OStack(m,i)]=i;
#endif
  }
  for(i=0;i<Mark2_RStackSize(m);i++){
    Mark2_RStack(a,i)=Mark2_RStack(m,i);
#ifdef INDEX
    a->index[Mark2_RStack(m,i)]=i;
#endif
  }
  return a;
}

  

void Mark2_Done(sMark2 *m)
{
  free(m->mark);
  free(m->stack);
#ifdef INDEX
  free(m->index);
#endif
  free(m);
}

/*指定されたステータスの最後の要素をスタックから除去して返す。*/
int Mark2_Pop(sMark2 *m,int state)
{
  Mark2_StackSize(m,state)--;
  return Mark2_Stack(m,state,Mark2_StackSize(m,state));
}

void Mark2_Push(sMark2 *m,int state,int i)
{
    Mark2_Stack(m,state,Mark2_StackSize(m,state))=i;
#ifdef INDEX
    m->index[i]=Mark2_StackSize(m,state);
#endif
    Mark2_StackSize(m,state)++;
}

/*指定されたステータスのスタックのi要素をさがして除去する。*/
void Mark2_EraseStack(sMark2 *m,int state,int i)
{
#ifdef INDEX
  int j;
  j=Mark2_Pop(m,state);
  Mark2_Stack(m,state,m->index[i])=j;
  m->index[j]=m->index[i];
#else
  int j;
  for(j=0;j<Mark2_StackSize(m,state);j++){
    if(Mark2_Stack(m,state,j)==i){
      Mark2_Stack(m,state,j)=Mark2_Pop(m,state);
      return;
    }
  }
#endif
}

/*ノードiの周縁マークを1減らす。0になったら周縁スタックから抹消する。*/
void Mark2_Unrim(sMark2 *m,int i)
{
  int j;
  int state=m->mark[i];
  m->mark[i]--;
  if(m->mark[i]==0){
    Mark2_EraseStack(m,RIM,i);
  }
}


/*ノードiの占有マークを消し、占有スタックから抹消する。もし周縁マークがついていれば周縁スタックに追加する。*/
void Mark2_Unoccupy(sMark2 *m,int i)
{
  int j;
  int state=m->mark[i];
  m->mark[i]-=256;
  Mark2_EraseStack(m,OCCUPY,i);
  if(m->mark[i]>0){
    Mark2_Push(m,RIM,i);
  }
}

/*ノードiの周縁マークを1増やす。もしもともと周縁でなかったら周縁スタックに追加*/
void Mark2_Rim(sMark2 *m,int i)
{
  if(m->mark[i]==0){
    Mark2_Push(m,RIM,i);
  }
  m->mark[i]++;
}

/*ノードiを占有する。もしもともと周縁であれば、周縁マークの値は維持するが周縁スタックからは除去する。、。*/
void Mark2_Occupy(sMark2 *m,int i)
{
  if(m->mark[i]>=256){
    return;
  }
  if(m->mark[i]>0){
    Mark2_EraseStack(m,RIM,i);
  }
  Mark2_Push(m,OCCUPY,i);
  m->mark[i]+=256;
}
