#ifndef _GRAPH_H
#define _GRAPH_H
#include "Hash.h"
#include "Group.h"
#define GRAPH 0
#define DIGRAPH 1
/* GOBACK is special network type for HBN */
#define GOBACK 2

#define ASIS
#define UNDIRECTIONIZE 1

#define GRAPH_LINE_WIDTH 1
#define GRAPH_LINE_STYLE 0
#define GRAPH_TEXT_SKIP 1
#define GRAPH_TEXT_PT 18

/*charだと、行列演算が遅い。可能ならintを使え。*/
#define bool unsigned char
/*グラフを分類するための構造体。*/
/*グラフに「中心節点」がある場合には、それをラベル0とする。あとの節点 */
/*の番号付けは自由。2つのグラフを照合するための強力な比較ルーチンを提 */
/*供する。各節点がIDをもっていて、位相幾何的に同じでもIDがことなる場合 */
/*もある。このID付きグラフを比較するルーチンも提供する。*/

/*構造体の各要素は、依存関係がある。上位のものが更新される際には、下位 */
/*の要素はリセットされなければいけない*/
typedef struct _clustertopology_
{
/*nextはこのライブラリのなかでは使わない。*/
/*  struct _clustertopology_ *next;*/
    int graphtype;
    int nnode;
    int nbond;
    /*隣接行列、nnode*node個の要素がある。*/
  /*平成１３年１月１６日(火)できればSparceMatrixで書いた方がよい。隣接要素の配列がないと、いろんなところで効率が悪い。Graph2.hを作成した方がいいかもしれない。*/
    bool *am;
    /*入結合、出結合の分布*/
    int maxin,maxout,*nin,*nout;
    /*距離行列、nnode*nnode個の要素がある。*/
      int *dm; /* depends on am */
    /*距離行列の固有値、nnode個の要素がある。*/
        double *devaluer,*devaluei; /* depends on dm */
    /*距離行列の固有ベクトル。nnode*nnode個の要素がある。*/
    /*  double *devectorr,*devectori; /* depends on dm */
    /*隣接行列以外は、準備する義務はない。NULLにしておけばよい。*/
    /*距離行列をどこまで計算したか。途中でうちきるケースが多いため。*/
    /*dmmax: max iteration; pmax: real maximum path*/
        int itemax,pmax; /* depends on dm */
    /*経路の数*/
          int *npath; /* depends on itemax */
    /*hash key for quick search*/
            union 
              {
		  float d;
		  unsigned int ui;
	      }
            hashkey; /* depends on npath */
/*  int count;*/
}
sGraph;

/*どんなルーチンが必要か。*/
/*大きな隣接行列から、特定の条件をみたす部分グラフを切りだすルーチン*/
/*トポロジーの照合*/
/*IDの照合、Renumber Tableの提供*/
/*距離行列の計算*/
/*固有値多項式の計算*/
/*等価節点の抽出；等価であるかどうかは、距離行列から判別できる。 */
/*行列は縮約してしまってかまわないはず。否。ベーテ格子を縮約してしまう */
/*と、ID付けが一意にできなくなる。多重縮約がまずいのか。*/

/*ID付きグラフの比較はID無しよりも簡単か？そんなことはない。対称性のせ */
/*いで等価な節点がありうる。それらの組み合わせをすべてあたらないと、同 */
/*一かどうか判定できない。*/
/*水のネットワークを有向グラフとして扱うが、2本の水素結合の交換は判別 */
/*できない。(空間距離の情報と同様、ネットワークトポロジー情報からは消 */
/*えてしまう)；情報量を減らす工夫をしている以上しかたがないとしよう。*/

/*平成１１年４月９日(金)構造体に要素を入れすぎ。なにが何だかわからん上に、簡単なことをするのにえらくメモリを食ってしまう。*/


void Graph_ConnectionDistrib(sGraph *g);
void Graph_PathLenDistrib(sGraph *g,int maxpath);
int DistanceMatrix(sGraph *g,int maxpath);
void FastDistanceMatrix(sGraph *g,int maxpath);
void FasterDistanceMatrix(sGraph *g,int maxpath);
void *memdup(void *a,size_t s);
sGraph *Graph_Duplicate(sGraph *g);
sGraph *Digraph2Graph(sGraph *g);
void Graph_Done(sGraph *g);
sGraph *Graph_Subset(sGraph *g,sGraph *ref,int center,int path);
int Graph_EigenValue(sGraph *g);
sGraph *Graph_Load(FILE *input);
sGraph *Graph_Load_NGPH(FILE *input);
void Graph_Print(sGraph *g,FILE *file);
void Graph_miniPrint(sGraph *g,FILE *file);
void Graph_Save(sGraph *g,FILE *file);
void Graph_Save_NGPH(sGraph *g,FILE *file);
int Graph_Isomorph(sGraph *g0,sGraph *g1);
int Graph_Distance(sGraph *g0,sGraph *g1,int *henkan01);

/*下位データのリセット*/
void Graph_Free_npath(sGraph *g);
void Graph_Free_dev(sGraph *g);
void Graph_Init_itemax(sGraph *g);
void Graph_Free_dm(sGraph *g);
void Graph_Free_am(sGraph *g);
void Graph_Free_nin(sGraph *g);
void Graph_Save_NNBR(sGraph *g,FILE *file);
sGraph *Digraph2Goback(sGraph *g);
sGraph *Graph_Init(int size);

typedef struct
{
    int graphtype;
    int nnode;
    /*各節点に隣接している節点のリスト*/
    int **in,**out;
    /*入結合、出結合の分布*/
    int *nin,*nout;
}
sRGraph;

sRGraph *RGraph_Load(FILE *file);
sGroup *RGraph_ConnectedClusters(sRGraph *);
sRGraph *RGraph_Duplicate(sRGraph *r);
sRGraph *RDigraph2Graph(sRGraph *r);
sGraph *RGraph_Subset(sRGraph *r,sRGraph *ref,int center,int path);
sRGraph *Graph2RGraph(sGraph *g);
void RGraph_Done(sRGraph *r);
void RGraph_Save(sRGraph *r,FILE *file,char *hdr);
unsigned char *GeneralUseDistanceMatrix(sRGraph *r,int maxpath);
sHash *RGraph_Ring4(sRGraph *r,const int size);

sGraph *Graph_MarkedSubset(sGraph *g,int nmark,int *marked);
#endif
