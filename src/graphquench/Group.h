#ifndef _GROUP_H
#define _GROUP_H
#include <stdio.h>
typedef struct
{
    int n;
    int *groupid;
    int *members;
}
sGroup;

/*count sizes of connected clusters.*/
sGroup *Group_Init(int n);
sGroup *Group_Init2(int n);
void Group_Register(sGroup *gr,int i,int j);
void Group_Done(sGroup *g);
void Group_Write(sGroup *g,FILE *file);
sGroup *Group_Read(FILE *file);
#endif
