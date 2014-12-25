/* �ͥåȥ���ΰ��Ĺ�¤�����ץ���� Ver. 0.70 */
/*$Id: countrings2.c,v 1.2 2007/09/20 09:45:39 matto Exp $
  $Log: countrings2.c,v $
  Revision 1.2  2007/09/20 09:45:39  matto
  Version down. varray is removed because of miscounting the 8 membered rings in CountRings/test.ngph

  Revision 1.5  2005-11-09 16:01:05  matto
  *** empty log message ***

  Revision 1.4  2004/01/27 08:07:09  matto
  IntHash is replaced by Int64Hash.

  Revision 1.3  2003/12/02 02:43:29  matto

 * Revision 1.2  2000/07/19  04:38:30  matto
 * countrings2 is modified to count rings at a time.(@RNGS format)
 *
  Revision 1.1.1.1  2000/02/25 03:01:57  matto
  Count rings in the network and find out order parameters.

  
  ����ʥ����ƥ���б��Ǥ���褦�˾��ܤ��ѹ���Ԥä���
  ʿ������ǯ�����(��)IntHash��SparseMatrix����Ѥ��ƴ��ǲ����褦��

  ʿ������ǯ�������(��)�դˡ������ʷϤ�6���Ĥ�������Ƥ��ޤ����꤬
  �ĤäƤ��롣�ĤϺ�ɸ�˴ط��ʤ��ȥݥ������������������ΤǤ��ꡢ
  ��ɸ�ˤ������ϳ����ץ����ǹԤä��ۤ������ä��ꤹ��Ȼפ���*/
#include<stdio.h>
#include<stdlib.h>
#include "SparseMatrix_CountRings.h"

void usage(char *cmd)
{
  fprintf(stderr,"$Id: countrings2.c,v 1.2 2007/09/20 09:45:39 matto Exp $\n");
  fprintf(stderr,"%s: List the rings in graph.\n",cmd);
  fprintf(stderr,"usage: %s -c maxsize < inputfile\n",cmd);
  fprintf(stderr,"usage: %s -C maxsize < inputfile\n",cmd);
  fprintf(stderr,"usage: %s size < inputfile\n",cmd);
  fprintf(stderr,"  inputfile must be in @NGPH or @GRPH format\n");
  fprintf(stderr,"Option:\n");
  fprintf(stderr,"  -c\tOutput the number of rings only.\n");
  fprintf(stderr,"Example:\n");
  fprintf(stderr,"  %s -c 6\tPrint the number of rings upto 6.\n",cmd);
  fprintf(stderr,"  %s -C 6\tPrint the number of rings upto 6 in @RNGS format.\n",cmd);
  fprintf(stderr,"  %s 3 5 7\tPrint the list of 3-, 5-, and 7-membered rings in @RNGL format\n",cmd);
  exit(1);
}

int main(int argc,char *argv[]) {
    RingType *ring;
    int ring_n;
    int i,j;
    char buf[255];
    int maxsize,output[MAXRINGSIZE],count=0,rngs=0;
    
    /*command line parser*/
    for(i=0;i<MAXRINGSIZE;i++)
      output[i]=0;
    if(argc<2)
      usage(argv[0]);
    if(strcmp("-c",argv[1])==0){
      if(argc!=3)
	usage(argv[0]);
      maxsize=atoi(argv[2]);
      count++;
    }else if(strcmp("-C",argv[1])==0){
      if(argc!=3)
	usage(argv[0]);
      maxsize=atoi(argv[2]);
      rngs++;
    }else{
      maxsize=3;
      i=1;
      while(i<argc){
	/*minimal check*/
	if(*argv[0]=='-')
	  usage(argv[0]);
	j=atoi(argv[i++]);
	if(j>=MAXRINGSIZE)
	  usage(argv[0]);
	output[j]++;
	if(j>maxsize)
	  maxsize=j;
      }
    }

    /*memory allocation*/
    if ((ring = (RingType *) malloc(sizeof(RingType)*RINGMAX)) == NULL)
        HeapError();

    while(NULL!=fgets(buf,sizeof(buf),stdin)){
      sSparseMatrix *path=NULL;
      if(0==strncmp(buf,"@NGPH",5)){
	/*maxadj�Ͽ�ξ�磱�����ǽ�ʬ��hashbit��log2(100*γ�ҿ�*2) ��
	  ���20���餤��ɬ�פ��⡣*/
	path=SparseMatrix_LoadNGPH(stdin,24,1000);
      }else if(0==strncmp(buf,"@GRPH",5)){
	/*maxadj�Ͽ�ξ�磱�����ǽ�ʬ��hashbit��log2(100*γ�ҿ�*2) ��
	  ���20���餤��ɬ�פ��⡣*/
	path=SparseMatrix_LoadGRPH(stdin,24,1000);
      }
      if(path!=NULL){
	int bond_n;
	BondType *bond;
	/*BondType����ϡ�SparseMatrix������Ф��롣*/
	if ((bond = (BondType *) malloc(sizeof(BondType)*path->nline)) == NULL)
	  HeapError();
	bond_n=SetBonds(path,bond);
	
        if (!CheckBonds(bond,bond_n))
            {fprintf(stderr,"Data error!\n"); exit(1);}

	/* ���Ĺ�¤��õ��(�������ꡢ��ʣ����) */
	ring_n = CountRings2(ring,RINGMAX,bond,bond_n,path,maxsize);
	
	/* ��ʣ�ν��� */
	ring_n = SimplifyRings(ring,ring_n);
	
	/* ���� */
	if(count){
	  for(i=3;i<=maxsize;i++){
	    output[i]=0;
	  }
	  for (i = 0; i < ring_n; i++){
	    if(ring[i].n<=maxsize)
	      output[ring[i].n]++;
	  }
	  for(i=3;i<=maxsize;i++){
	    printf("%d ",output[i]);
	  }
	  printf("\n");
	}else if(rngs){
	  int s;
	  printf("@RNGS\n%d\n",path->nline);
	  for (i = 0; i < ring_n; i++){
	    int j;
	    if(ring[i].n<=maxsize){
	      printf("%d ",ring[i].n);
	      for(j=0;j<ring[i].n;j++){
		printf("%d ",ring[i].list[j]);
	      }
	      printf("\n");
	    }
	  }
	  printf("0\n");
	}else{
	  int s;
	  for(s=3;s<=maxsize;s++)
	    if(output[s]){
	      printf("@RNGL\n%d\n%d\n",s,path->nline);
	      for (i = 0; i < ring_n; i++){
		if(ring[i].n==s){
		  int j;
		  for(j=0;j<s;j++){
		    printf("%d ",ring[i].list[j]);
		  }
		  printf("\n");
		}
	      }
	      for(i=0;i<s;i++)
		printf("-1 ");
	      printf("\n");
	    }
	}
	
	/* ��λ */
	free(bond);
	SparseMatrix_Done(path);
      }
    }
    free(ring);
    return 0;
}
