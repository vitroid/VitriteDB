#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Mark2.h"

/*2$B>uBV$r$H$k(Bmark$B!#%/%i%9%?$N<~1o$N=hM}$KFC2=$9$k!#2<(B8$B%S%C%H$O!"<~1o$+$I$&$+$rI=$7!"Bh(B8$B%S%C%H(B(256)$B$,%;%C%H$5$l$F$$$l$P!"%^!<%/$5$l$F$$$k$b$N$H$7$F07$&!#(B2$B>uBV$NHV9fIU$1$O!"(B0$B$,<~1o!"(B1$B$,@jM-!#(B*/

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

/*$B;XDj$5$l$?%9%F!<%?%9$N:G8e$NMWAG$r%9%?%C%/$+$i=|5n$7$FJV$9!#(B*/
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

/*$B;XDj$5$l$?%9%F!<%?%9$N%9%?%C%/$N(Bi$BMWAG$r$5$,$7$F=|5n$9$k!#(B*/
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

/*$B%N!<%I(Bi$B$N<~1o%^!<%/$r(B1$B8:$i$9!#(B0$B$K$J$C$?$i<~1o%9%?%C%/$+$iKu>C$9$k!#(B*/
void Mark2_Unrim(sMark2 *m,int i)
{
  int j;
  int state=m->mark[i];
  m->mark[i]--;
  if(m->mark[i]==0){
    Mark2_EraseStack(m,RIM,i);
  }
}


/*$B%N!<%I(Bi$B$N@jM-%^!<%/$r>C$7!"@jM-%9%?%C%/$+$iKu>C$9$k!#$b$7<~1o%^!<%/$,$D$$$F$$$l$P<~1o%9%?%C%/$KDI2C$9$k!#(B*/
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

/*$B%N!<%I(Bi$B$N<~1o%^!<%/$r(B1$BA}$d$9!#$b$7$b$H$b$H<~1o$G$J$+$C$?$i<~1o%9%?%C%/$KDI2C(B*/
void Mark2_Rim(sMark2 *m,int i)
{
  if(m->mark[i]==0){
    Mark2_Push(m,RIM,i);
  }
  m->mark[i]++;
}

/*$B%N!<%I(Bi$B$r@jM-$9$k!#$b$7$b$H$b$H<~1o$G$"$l$P!"<~1o%^!<%/$NCM$O0];}$9$k$,<~1o%9%?%C%/$+$i$O=|5n$9$k!#!"!#(B*/
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
