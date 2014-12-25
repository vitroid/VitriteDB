/*GraphDB$B$N!"AjF1@-%A%'%C%/;;Dx$r0\?"$9$k!#(B(Graph.h$B$r$G$-$k$@$1;H$$$?$/$J$$$N$G!#(B)*/
/*$BJ88%$r$$$m$$$mA\$7$F$_$?$,!"(BGraph$B$+$i(Bhash key$B$r:n$kNI$$J}K!$,8+Ev$i$J$$$N$G!"$H$j$"$($:!"5wN%9TNs$NMWAG$G(Bhash key$B$r$D$/$k$3$H$K$9$k!#3F%N!<%I$K4X$7$F!"5wN%(B1$B$N%N!<%I?t!"5wN%(B2$B$N%N!<%I?t!&!&!&$r?t$($k!#$3$l$r!"%=!<%H$7$?$b$N$r%O%C%7%e%-!<$H$9$k!#(B*/

/*$B$"$i$+$8$a!"5wN%9TNs$H!"%O%C%7%e%-!<$O:n$C$F$*$/!#(B*/


#include <stdio.h>
#include "IntMatrix2.h"
#include "NetMatch.h"



/*Isomorph$B$OF10l$+$I$&$+$@$1$r%A%'%C%/$7!"N`;wEY$O$7$i$Y$J$$!#(B
$B5wN%9TNs!"%O%C%7%e%-!<$O;HMQ$;$:!"NY@\>pJs$N$_$+$i>H9g$r9T$&!#(B*/
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


