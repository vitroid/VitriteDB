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

/*char$B$@$H!"9TNs1i;;$,CY$$!#2DG=$J$i(Bint$B$r;H$(!#(B*/
#define bool unsigned char
/*$B%0%i%U$rJ,N`$9$k$?$a$N9=B$BN!#(B*/
/*$B%0%i%U$K!VCf?4@aE@!W$,$"$k>l9g$K$O!"$=$l$r%i%Y%k(B0$B$H$9$k!#$"$H$N@aE@(B */
/*$B$NHV9fIU$1$O<+M3!#(B2$B$D$N%0%i%U$r>H9g$9$k$?$a$N6/NO$JHf3S%k!<%A%s$rDs(B */
/*$B6!$9$k!#3F@aE@$,(BID$B$r$b$C$F$$$F!"0LAj4v2?E*$KF1$8$G$b(BID$B$,$3$H$J$k>l9g(B */
/*$B$b$"$k!#$3$N(BID$BIU$-%0%i%U$rHf3S$9$k%k!<%A%s$bDs6!$9$k!#(B*/

/*$B9=B$BN$N3FMWAG$O!"0MB84X78$,$"$k!#>e0L$N$b$N$,99?7$5$l$k:]$K$O!"2<0L(B */
/*$B$NMWAG$O%j%;%C%H$5$l$J$1$l$P$$$1$J$$(B*/
typedef struct _clustertopology_
{
/*next$B$O$3$N%i%$%V%i%j$N$J$+$G$O;H$o$J$$!#(B*/
/*  struct _clustertopology_ *next;*/
    int graphtype;
    int nnode;
    int nbond;
    /*$BNY@\9TNs!"(Bnnode*node$B8D$NMWAG$,$"$k!#(B*/
  /*$BJ?@.#1#3G/#17n#1#6F|(B($B2P(B)$B$G$-$l$P(BSparceMatrix$B$G=q$$$?J}$,$h$$!#NY@\MWAG$NG[Ns$,$J$$$H!"$$$m$s$J$H$3$m$G8zN($,0-$$!#(BGraph2.h$B$r:n@.$7$?J}$,$$$$$+$b$7$l$J$$!#(B*/
    bool *am;
    /*$BF~7k9g!"=P7k9g$NJ,I[(B*/
    int maxin,maxout,*nin,*nout;
    /*$B5wN%9TNs!"(Bnnode*nnode$B8D$NMWAG$,$"$k!#(B*/
      int *dm; /* depends on am */
    /*$B5wN%9TNs$N8GM-CM!"(Bnnode$B8D$NMWAG$,$"$k!#(B*/
        double *devaluer,*devaluei; /* depends on dm */
    /*$B5wN%9TNs$N8GM-%Y%/%H%k!#(Bnnode*nnode$B8D$NMWAG$,$"$k!#(B*/
    /*  double *devectorr,*devectori; /* depends on dm */
    /*$BNY@\9TNs0J30$O!"=`Hw$9$k5AL3$O$J$$!#(BNULL$B$K$7$F$*$1$P$h$$!#(B*/
    /*$B5wN%9TNs$r$I$3$^$G7W;;$7$?$+!#ESCf$G$&$A$-$k%1!<%9$,B?$$$?$a!#(B*/
    /*dmmax: max iteration; pmax: real maximum path*/
        int itemax,pmax; /* depends on dm */
    /*$B7PO)$N?t(B*/
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

/*$B$I$s$J%k!<%A%s$,I,MW$+!#(B*/
/*$BBg$-$JNY@\9TNs$+$i!"FCDj$N>r7o$r$_$?$9ItJ,%0%i%U$r@Z$j$@$9%k!<%A%s(B*/
/*$B%H%]%m%8!<$N>H9g(B*/
/*ID$B$N>H9g!"(BRenumber Table$B$NDs6!(B*/
/*$B5wN%9TNs$N7W;;(B*/
/*$B8GM-CMB?9`<0$N7W;;(B*/
/*$BEy2A@aE@$NCj=P!(Ey2A$G$"$k$+$I$&$+$O!"5wN%9TNs$+$iH=JL$G$-$k!#(B */
/*$B9TNs$O=LLs$7$F$7$^$C$F$+$^$o$J$$$O$:!#H]!#%Y!<%F3J;R$r=LLs$7$F$7$^$&(B */
/*$B$H!"(BID$BIU$1$,0l0U$K$G$-$J$/$J$k!#B?=E=LLs$,$^$:$$$N$+!#(B*/

/*ID$BIU$-%0%i%U$NHf3S$O(BID$BL5$7$h$j$b4JC1$+!)$=$s$J$3$H$O$J$$!#BP>N@-$N$;(B */
/*$B$$$GEy2A$J@aE@$,$"$j$&$k!#$=$l$i$NAH$_9g$o$;$r$9$Y$F$"$?$i$J$$$H!"F1(B */
/*$B0l$+$I$&$+H=Dj$G$-$J$$!#(B*/
/*$B?e$N%M%C%H%o!<%/$rM-8~%0%i%U$H$7$F07$&$,!"(B2$BK\$N?eAG7k9g$N8r49$OH=JL(B */
/*$B$G$-$J$$!#(B($B6u4V5wN%$N>pJs$HF1MM!"%M%C%H%o!<%/%H%]%m%8!<>pJs$+$i$O>C(B */
/*$B$($F$7$^$&(B)$B!(>pJsNL$r8:$i$99)IW$r$7$F$$$k0J>e$7$+$?$,$J$$$H$7$h$&!#(B*/

/*$BJ?@.#1#1G/#47n#9F|(B($B6b(B)$B9=B$BN$KMWAG$rF~$l$9$.!#$J$K$,2?$@$+$o$+$i$s>e$K!"4JC1$J$3$H$r$9$k$N$K$($i$/%a%b%j$r?)$C$F$7$^$&!#(B*/


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

/*$B2<0L%G!<%?$N%j%;%C%H(B*/
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
    /*$B3F@aE@$KNY@\$7$F$$$k@aE@$N%j%9%H(B*/
    int **in,**out;
    /*$BF~7k9g!"=P7k9g$NJ,I[(B*/
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
