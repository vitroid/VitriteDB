/*multivalue mark set*/
#ifndef _MARK2_H
#define _MARK2_H
#define INDEX
typedef struct
{
  int nmax;
  int sp[2];
  int *mark;
  int *stack;
#ifdef INDEX
  /*平成13年2月10日(土)スタック内検索が遅いので、インデックスを作成する。*/
  /*インデックスは、RスタックとOスタックを区別しない。contextから判断できる。*/
  int *index;
#endif
}
sMark2;
#define RIM 0
#define OCCUPY 1
#define Mark2_QueryMarked(m,x) ((m)->mark[x])
#define Mark2_QueryOccupied(m,x) ((m)->mark[x]>=256)
#define Mark2_Stack(m,s,x) ((m)->stack[(x)*2+s])
#define Mark2_RStack(m,x) Mark2_Stack(m,RIM,x)
#define Mark2_OStack(m,x) Mark2_Stack(m,OCCUPY,x)
#define Mark2_StackSize(m,s) ((m)->sp[s])
#define Mark2_RStackSize(m) Mark2_StackSize(m,RIM)
#define Mark2_OStackSize(m) Mark2_StackSize(m,OCCUPY)
sMark2 *Mark2_New(int n);
sMark2 *Mark2_Dup(sMark2 *m);
void Mark2_Done(sMark2 *m);
void Mark2_Unrim(sMark2 *m,int i);
void Mark2_Unoccupy(sMark2 *m,int i);
void Mark2_Rim(sMark2 *m,int i);
void Mark2_Occupy(sMark2 *m,int i);
#endif
