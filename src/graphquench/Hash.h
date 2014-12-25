#ifndef _HASH_H
#define _HASH_H

typedef struct _sEntry
{
    struct _sEntry *next;
    void *value;
    int count;
    void *optional;
}
sEntry;

typedef struct
{
    sEntry *last;
    sEntry **entry;
    int nentry;
    unsigned int mask;
    int shift;
}
sHash;

unsigned int Hash_Encode(sHash *s,unsigned int key);
sEntry *Hash_SearchFirst(sHash *s,unsigned int key);
sEntry *Hash_SearchNext(sHash *s);
sHash *Hash_Create(int nshift);
void Hash_Enter(sHash *s,int key,sEntry *e);
void Hash_Done(sHash *s,void Entry_Done(void *));

#endif
