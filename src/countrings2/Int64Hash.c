/*$B@0?t(B(0$B$r4^$^$J$$(B)$B$rMWAG$H$9$k(BHash$B!#(B*/

#include <stdio.h>
#include <stdlib.h>
#include "Int64Hash.h"
#define EMPTY -1

/*shift$B$H(Bxor$B$K$h$k4JC1$J(Bhash key$B$N@8@.(B*/
u_int64_t _Int64Hash_Encode(sInt64Hash *s, u_int64_t key)
{
    u_int64_t mod=0;
    while(key)
      {
          mod ^= (key & (s->hashsize-1));
          key >>= s->shift;
      }
    return mod;
}

/*hash$BMWAG$NHV9f$rJV$9!#$b$7B8:_$7$J$$MWAG$J$i!"6u$-MWAG$rJV$9!#MWAG$ODI2C$5$l$J$$!#(B*/
u_int64_t _Int64Hash_QueryElement(sInt64Hash *ih, u_int64_t key)
{
  u_int64_t e=_Int64Hash_Encode(ih,key);
  while(1){
    if(ih->key[e]==EMPTY){
      ih->value[e]=0;
      return e;
    }
    if(ih->key[e]==key){
      return e;
    }
    e+=13;
    if(e>=ih->hashsize)
      e-=ih->hashsize;
  }
}

/*$BCM$rEPO?$9$k!#99?7$J$i(B0$B!"DI2C$J$i(B1$B$rJV$9!#(B*/
int Int64Hash_RegisterValue(sInt64Hash *ih, u_int64_t key,int value)
{
    u_int64_t e=_Int64Hash_QueryElement(ih,key);
    int v=ih->value[e];
    if(v==0){
        ih->key[e]=key;
        ih->nentry++;
        if(ih->nentry > ih->hashsize/2){
            fprintf(stderr,"Warning: hash size seems too small.\n");
        }
        if(ih->nentry >= ih->hashsize){
            fprintf(stderr,"Error: hash overflow.\n");
            exit(1);
        }
    }
    ih->value[e]=value;
    return (v==0);
}

/*$BCM$r;2>H$9$k!#(B*/
int Int64Hash_QueryValue(sInt64Hash *ih,u_int64_t key)
{
  return ih->value[_Int64Hash_QueryElement(ih,key)];
}

/*$BCM$rKu>C$9$k!#(B*/
void Int64Hash_EraseOne(sInt64Hash *ih,u_int64_t key)
{
  ih->key[_Int64Hash_QueryElement(ih,key)]=EMPTY;
}

sInt64Hash *Int64Hash_Init(int size)
{
    sInt64Hash *ih=malloc(sizeof(sInt64Hash));
    int i;
    u_int64_t m;
    
    m=1<<size;
    ih->shift=size;
    ih->hashsize=m;
    ih->key=malloc(m*sizeof(u_int64_t));
    ih->value=calloc(m,sizeof(int));
    ih->nentry=0;
    for(i=0;i<m;i++)
        ih->key[i]=EMPTY;
    return ih;
}

void Int64Hash_Done(sInt64Hash *ih)
{
    free(ih->key);
    free(ih->value);
    free(ih);
}
