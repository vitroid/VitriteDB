/*GraphDBの、相同性チェック算程を移植する。(Graph.hをできるだけ使いたくないので。)*/
/*文献をいろいろ捜してみたが、Graphからhash keyを作る良い方法が見当らないので、とりあえず、距離行列の要素でhash keyをつくることにする。各ノードに関して、距離1のノード数、距離2のノード数・・・を数える。これを、ソートしたものをハッシュキーとする。*/

/*あらかじめ、距離行列と、ハッシュキーは作っておく。*/


#include <stdio.h>
#include "IntMatrix2.h"
#include "NetMatch.h"



/*Isomorphは同一かどうかだけをチェックし、類似度はしらべない。
距離行列、ハッシュキーは使用せず、隣接情報のみから照合を行う。*/
int Isomorph(int size,int* nn0,int* n0,int* nn1,int* n1)
{
    sIntMatrix2* a;
    sIntMatrix2* b;
    int result;
    
    if(size==1)
	return 1;
    a=IntMatrix2_NewAdjMatrix(size,nn0,n0);
    b=IntMatrix2_NewAdjMatrix(size,nn1,n1);
    
    result=IntMatrix2_NetMatch(a,b,1);
    IntMatrix2_Done(a);
    IntMatrix2_Done(b);
    return (result==0);
}

/*
int main(int argc,char *argv[])
{
    int* n0=NULL;
    int* nn0;
    int* n1=NULL;
    int* nn1;
    int size0,size1;
    
    char buf[1000];
    while(NULL!=fgets(buf,sizeof(buf),stdin)){
	if(0==strncmp("@NGPH",buf,5)){
	    if(n0==NULL){
		puts("First");
		LoadNGPH(stdin,&nn0,&n0,&size0);
	    }else{
		puts("Second");
		LoadNGPH(stdin,&nn1,&n1,&size1);
	    }
	    if(n1!=NULL)
		break;
	}
    }
    if(size0!=size1){
	printf("Nooo: %d %d\n",size0,size1);
	exit(1);
    }
    if(Isomorph(size0,nn0,n0,nn1,n1)){
	puts("Yes");
    }else{
	puts("No");
    }
}
*/



/*test*/


/*
int main(int argc,char *argv[])
{
    char buf[1000];
    while(NULL!=fgets(buf,sizeof(buf),stdin)){
	if(0==strncmp("@NGPH",buf,5)){
	    int* n=NULL;
	    int* nn;
	    int size;
	    int i;
	    int* sp;
	    int* dm;
	    LoadNGPH(stdin,&nn,&n,&size);
	    dm = MakeDistanceMatrix(size,nn,n);
	    sp = DistanceMatrix2Spectrum(size,dm);
	    for(i=0;i<size;i++){
		int j;
		for(j=0;j<MAXDIST;j++){
		    printf("%d ",sp[j*size+i]);
		}
		printf("\n");
	    }
	    printf("\n");
	}
    }
}
*/


