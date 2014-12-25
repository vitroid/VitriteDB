/*
 * SparseMatrix.c
 * 
 * Treating sparse matrices with hash table
 * $Id: SparseMatrix.c,v 1.2 2007/09/20 09:45:39 matto Exp $
 * $Log: SparseMatrix.c,v $
 * Revision 1.2  2007/09/20 09:45:39  matto
 * Version down. varray is removed because of miscounting the 8 membered rings in CountRings/test.ngph
 *
 * Revision 1.2  2004-01-27 08:07:09  matto
 * IntHash is replaced by Int64Hash.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SparseMatrix.h"

/*$BMWAG(B(i,j)$B$K(Bvalue$B$rEPO?$9$k!#(Bvalue$B$,(B0$B$N>l9g!"K\Mh$J$i(BHash$B$+$i%G!<%?$r>C$7!"NY(B
  $B@\>pJs$+$i$b=|$+$J$1$l$P$$$1$J$$$,!"<B:]E*$JI,MW$,@8$8$F$$$J$$$N$G!"(B
  $B$3$3$G$O(Bvalue$B$,(B0$B$N%1!<%9$O9M$($F$$$J$$!#(B*/
void SparseMatrix_RegisterValue(sSparseMatrix *path,int i,int j,int value)
{
  int k;
  int new=Int64Hash_RegisterValue(path->ih,(u_int64_t)i*(u_int64_t)path->ncolumn+(u_int64_t)j,value);
  if(new){
    if(path->e_line[i].nadj>=path->maxadj-1){
      fprintf(stderr,"Too many elements in line %d\n",i);
      exit(1);
    }
    path->e_line[i].adj[path->e_line[i].nadj++]=j;
    if(path->e_column[j].nadj>=path->maxadj-1){
      fprintf(stderr,"Too many elements in column %d\n",i);
      exit(1);
    }
    path->e_column[j].adj[path->e_column[j].nadj++]=i;
  }
}


/*$BMWAG(B(i,j)$B$NCM$rJV$9!#(B*/
int SparseMatrix_QueryValue(sSparseMatrix *path,int i,int j)
{
  return Int64Hash_QueryValue(path->ih,(u_int64_t)i*(u_int64_t)path->ncolumn+(u_int64_t)j);
}

void _sp_hasherror()
{
  fprintf(stderr,"Hash error. Halted.\n");
  exit(1);
}


/*$B6uABBP>N9TNs$N=i4|2=(B*/
sSparseMatrix *SparseMatrix_Init(int nline,int ncolumn,int hashbit,int maxadj)
{
  int i;
  sSparseMatrix *path=malloc(sizeof(sSparseMatrix));
  path->nline=nline;
  path->ncolumn=ncolumn;
  path->maxadj=maxadj;
  path->ih=Int64Hash_Init(hashbit);
  if(NULL==(path->e_line=calloc(nline,sizeof(sLineElement))))
    _sp_hasherror();
  if(NULL==(path->e_column=calloc(ncolumn,sizeof(sLineElement))))
    _sp_hasherror();
  for(i=0;i<nline;i++){
    if(NULL==(path->e_line[i].adj=malloc(sizeof(int)*maxadj)))
      _sp_hasherror();
  }
  for(i=0;i<ncolumn;i++){
    if(NULL==(path->e_column[i].adj=malloc(sizeof(int)*maxadj)))
      _sp_hasherror();
  }
  return path;
}

/*$B6uABBP>N9TNs$N(Bdestructor*/
void SparseMatrix_Done(sSparseMatrix *path)
{
  int i;
  for(i=0;i<path->nline;i++){
    free(path->e_line[i].adj);
  }
  for(i=0;i<path->ncolumn;i++){
    free(path->e_column[i].adj);
  }
  free(path->e_line);
  free(path->e_column);
  Int64Hash_Done(path->ih);
  free(path);
}

/* $B6uABBP>N9TNs$NFI$_9~$_(B */
sSparseMatrix *SparseMatrix_LoadSSMX(FILE *fp,int hashbit,int maxadj) {
  int n;
  char buf[256];
  sSparseMatrix *path;
  /*SSMX$B7A<0$rA[Dj!#(B1$B9TL\$O@aE@$N?t(B*/
  fgets(buf,sizeof(buf),fp);
  n=atoi(buf);
  /*$B=i4|2=(B*/
  path=SparseMatrix_Init(n,n,hashbit,maxadj);
  /*$BNY@\4X78$NFI$_$3$_!#(B*/
  while(NULL!=fgets(buf,sizeof(buf),fp)){
    int i,j,v;
    sscanf(buf,"%d %d %d",&i,&j,&v);
    /**/
    if(i<0)break;
    /*$BL58~%0%i%U$H$7$F$"$D$+$&!#(B*/
    SparseMatrix_RegisterValue(path,i,j,v);
    if(i!=j)
      SparseMatrix_RegisterValue(path,j,i,v);
  }
  return path;
}

/* $BFsCM6uABBP>N9TNs$NFI$_9~$_(B */
sSparseMatrix *SparseMatrix_LoadNGPH(FILE *fp,int hashbit,int maxadj) {
  int n;
  char buf[256];
  sSparseMatrix *path;
  /*NGPH$B7A<0$rA[Dj!#(B1$B9TL\$O@aE@$N?t(B*/
  fgets(buf,sizeof(buf),fp);
  n=atoi(buf);
  /*$B=i4|2=(B*/
  path=SparseMatrix_Init(n,n,hashbit,maxadj);
  /*$BNY@\4X78$NFI$_$3$_!#(B*/
  while(NULL!=fgets(buf,sizeof(buf),fp)){
    int i,j,v;
    sscanf(buf,"%d %d",&i,&j);
    /**/
    if(i<0)break;
    /*$BL58~%0%i%U$H$7$F$"$D$+$&!#(B*/
    SparseMatrix_RegisterValue(path,i,j,1);
    if(i!=j)
      SparseMatrix_RegisterValue(path,j,i,1);
  }
  return path;
}

/* $BFsCM6uABHsBP>N9TNs$NFI$_9~$_(B */
sSparseMatrix *SparseMatrix_LoadAsymNGPH(FILE *fp,int hashbit,int maxadj) {
  int n;
  char buf[256];
  sSparseMatrix *path;
  /*NGPH$B7A<0$rA[Dj!#(B1$B9TL\$O@aE@$N?t(B*/
  fgets(buf,sizeof(buf),fp);
  n=atoi(buf);
  /*$B=i4|2=(B*/
  path=SparseMatrix_Init(n,n,hashbit,maxadj);
  /*$BNY@\4X78$NFI$_$3$_!#(B*/
  while(NULL!=fgets(buf,sizeof(buf),fp)){
    int i,j,v;
    sscanf(buf,"%d %d",&i,&j);
    /**/
    if(i<0)break;
    /*$BL58~%0%i%U$H$7$F$"$D$+$&!#(B*/
    SparseMatrix_RegisterValue(path,i,j,1);
  }
  return path;
}

/* $BFsCMBP>N9TNs$NFI$_9~$_(B */
sSparseMatrix *SparseMatrix_LoadGRPH(FILE *fp,int hashbit,int maxadj) {
  int n,i,j;
  char buf[10000];
  sSparseMatrix *path;
  /*GRPH$B7A<0$rA[Dj!#(B1$B9TL\$O@aE@$N?t(B*/
  fgets(buf,sizeof(buf),fp);
  n=atoi(buf);
  /*$B=i4|2=(B*/
  path=SparseMatrix_Init(n,n,hashbit,maxadj);
  /*$BNY@\4X78$NFI$_$3$_!#(B*/
  for(i=0;i<n;i++){
    if(NULL==fgets(buf,sizeof(buf),fp)){
      SparseMatrix_Done(path);
      return NULL;
    }
    for(j=0;j<n;j++)
      if(buf[j]=='1'){
	SparseMatrix_RegisterValue(path,i,j,1);
	if(i!=j)
	  SparseMatrix_RegisterValue(path,j,i,1);
      }
  }
  return path;
}

/* $BFsCMHsBP>N9TNs$NFI$_9~$_(B */
sSparseMatrix *SparseMatrix_LoadAsymGRPH(FILE *fp,int hashbit,int maxadj) {
  int n,i,j;
  char buf[10000];
  sSparseMatrix *path;
  /*GRPH$B7A<0$rA[Dj!#(B1$B9TL\$O@aE@$N?t(B*/
  fgets(buf,sizeof(buf),fp);
  n=atoi(buf);
  /*$B=i4|2=(B*/
  path=SparseMatrix_Init(n,n,hashbit,maxadj);
  /*$BNY@\4X78$NFI$_$3$_!#(B*/
  for(i=0;i<n;i++){
    if(NULL==fgets(buf,sizeof(buf),fp)){
      SparseMatrix_Done(path);
      return NULL;
    }
    for(j=0;j<n;j++)
      if(buf[j]=='1'){
	SparseMatrix_RegisterValue(path,i,j,1);
      }
  }
  return path;
}

/* $B=PNO!#(B($B3NG'MQ(B) */
void SparseMatrix_SaveSMTX(FILE *fp,sSparseMatrix *path) {
  int n;
  char buf[256];
  int i,j;
  if(path->nline!=path->ncolumn){
    fprintf(stderr,"Warning: Not a square matrix.\n");
  }
  fprintf(fp,"@SMTX\n%d\n",path->nline);
  for(i=0;i<path->nline;i++){
    for(j=0;j<path->ncolumn;j++){
      fprintf(fp,"%d ",SparseMatrix_QueryValue(path,i,j));
    }
    fprintf(fp,"\n");
  }
}

void SparseMatrix_SaveNGPH(FILE *fp,sSparseMatrix *path) {
  int n;
  char buf[256];
  int i,j;
  if(path->nline!=path->ncolumn){
    fprintf(stderr,"Warning: Not a square matrix.\n");
  }
  fprintf(fp,"@NGPH\n%d\n",path->nline);
  for(i=0;i<path->nline;i++){
    for(j=0;j<path->e_line[i].nadj;j++){
      int k=path->e_line[i].adj[j];
      int v=SparseMatrix_QueryValue(path,i,k);
      if(v>1){
	fprintf(stderr,"Warning: not a binary value %d at element %d %d of NGPH.\n",v,i,k);
      }
      if(v){
	fprintf(fp,"%d %d\n",i,k);
      }
    }
  }
  fprintf(fp,"-1 -1\n");
}
