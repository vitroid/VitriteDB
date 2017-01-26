#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <db.h>
#include <sys/file.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>

#include <stdio.h>
#include "IntMatrix2.h"
#include "NetMatch.h"
#include "Isomorph.h"
#include "SimpleGraph.h"

/*いよいよDBを構成する。dbopenをそのまま使用する。*/
/*keyにはスペクトルを、値には隣接情報を使用する。*/
/*異なるグラフが同じスペクトルを生む可能性もあるぞ。その場合は、valueを複数持てるようにして、順次照合するしかないのだろうか。*/

void
db_open(DB_ENV *dbenv, DB **dbp, char *name, int dups)
{
    DB *db;
    int ret;
    /* Create the database handle. */
    if ((ret = db_create(&db, dbenv, 0)) != 0) {
        dbenv->err(dbenv, ret, "db_create");
        exit (1);
    }
    /* Optionally, turn on duplicate data items. */
    if (dups && (ret = db->set_flags(db,DB_DUP)) != 0) {
        dbenv->err(dbenv, ret, "db set_flags: DB_DUP");
        exit (1);
    }
    /*
     * Open a database in the environment:
     * create if it doesn't exist
     * free-threaded handle
     * read/write owner only
     */
    if ((ret = db->open(db, NULL, name, NULL, 
                       DB_BTREE, DB_CREATE | DB_THREAD, S_IRUSR | S_IWUSR)) != 0) {
        dbenv->err(dbenv, ret, "db open: %s", name);
        exit (1);
    }
    *dbp = db;
}

/*グラフが無向でサイズが小さい(254ノード以下)と仮定してシリアライズ*/
/*NGPH類似のデータ形式に戻す。*/
int Serialize(int count, int size, int* nnei, int* nei, char* buf)
{
    char* p=buf;
    int i;
    int* q;
    
    q = (int *) p;
    *q = count;
    p += sizeof(int);
    
    *(p++) = size;
    for(i=0;i<size;i++){
        int j;
        for(j=0;j<nnei[i];j++){
            int k;
            k=nei[j*size+i];
            if(i<k){
                *(p++)=i;
                *(p++)=k;
            }
        }
    }
    *(p++)='\255';
    return p-buf;
}




char* Unserialize(char* ptr, int* count, int* size, int* nnei, int* nei)
{
    int i;
    int* p;
    p = (int *) ptr;
    *count = *p;
    ptr += sizeof(int);
    *size = *(ptr++);
    if(nnei==NULL)
        return NULL;
    for (i=0; i<*size; i++) {
        nnei[i] = 0;
    }
    while((*ptr)!='\255'){
        int j,k;
        j = *(ptr++);
        k = *(ptr++);
        PushUnique( nei, nnei, *size, j, k );
        PushUnique( nei, nnei, *size, k, j );
    }
    return ptr;
}
    

void
env_dir_create(char* envdir)
{
    struct stat sb;
/*
 * If the directory exists, we're done. We do not further check
 * the type of the file, DB will fail appropriately if it's the
 * wrong type.
 */
    if (stat(envdir, &sb) == 0)
        return;
/* Create the directory, read/write/access owner only. */
    if (mkdir(envdir, S_IRWXU) != 0) {
        fprintf(stderr,
                "txnapp: mkdir: %s: %s\n", envdir,
                strerror(errno));
        exit (1);
    }
}

void
env_open(DB_ENV **dbenvp, char* envdir, unsigned int cachesize)
{
    DB_ENV *dbenv;
    int ret;
/* Create the environment handle. */
    if ((ret = db_env_create(&dbenv, 0)) != 0) {
        fprintf(stderr,
                "txnapp: db_env_create: %s\n", db_strerror(ret));
        exit (1);
    }
/* Set up error handling. */
    dbenv->set_errpfx(dbenv, "txnapp");
    dbenv->set_cachesize(dbenv,0,cachesize,1);
/*
 * Open a transactional environment:
 * create if it doesn't exist
 * free-threaded handle
 * run recovery
 * read/write owner only
 */
/*
  if ((ret = dbenv->open(dbenv, envdir,
                          DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG |
                          DB_INIT_MPOOL | DB_INIT_TXN | DB_RECOVER
                          | DB_THREAD,
                          S_IRUSR | S_IWUSR)) != 0) {*/
    if ((ret = dbenv->open(dbenv, envdir,
                           DB_CREATE | DB_INIT_LOCK | DB_INIT_MPOOL | DB_THREAD| DB_PRIVATE ,
                          S_IRUSR | S_IWUSR)) != 0) {
        dbenv->err(dbenv, ret,
                  "dbenv open: %s", envdir);
        exit (1);
    }
    *dbenvp = dbenv;
}

/*algorithm is based on addRecord in dbtest.c.*/
#define MAXIMUM_RETRY 10
/*addRecord is removed and put in scratch*/


/*txnを使わなければどうなるか。log.*の排出をやめれば相当速くできるはず。*/
int addRecord2(DB_ENV *dbenv, DB* db, int* gkey, int size, int* nnei, int* nei, int gcount)
{
    DBT key, value, newvalue;
    int ret;
    DBC *dbc;
    int putflag=0;
    int getflag=0;
    int count=0;
    int fail=0;
    int retryCount=0;

    /*gcount==0の場合は、書きこまずに個数をreturnするのみ。*/
    /*initialization*/
    memset(&key, 0, sizeof(key));
    memset(&value, 0, sizeof(value));
    memset(&newvalue, 0, sizeof(newvalue));
    key.data = gkey;
    key.size = sizeofSpectrum(size);
    //showSpectrum(size,gkey);
    //key.data = "test";
    //key.size = strlen("test")+1;
    value.flags = DB_DBT_MALLOC;

retry:
    /*
     * Get the key. If it exists, we increment the value. If it
     * doesn't exist, we create it.
     */
    /*make cursor*/
    ret = db->cursor(db, NULL, &dbc, 0);
    getflag = DB_SET;
    for(;;){
        switch(        ret = dbc->c_get(dbc, &key, &value, getflag)){
        case 0:
            /*found*/
            /*lookup the dup records*/
            //printf("Found.");
            {
                int rsize;
                int rnei[size*10*sizeof(int)];
                int rnnei[size*sizeof(int)];
                Unserialize(value.data, &count, &rsize, rnnei, rnei);
                /*printf("%d %d:%d\n",count,size,rsize);*/
                if((rsize==size) && Isomorph(size,nnei,nei,rnnei,rnei)){
                    count+=gcount;
                    //printf("Isomorph.%d ",count);
                    if(value.data!=NULL)
                        free(value.data);
                    /*create the new data item.*/
                    //showSpectrum(size, key.data);
                    //newvalue.data="data";  //malloc(sizeof(int)*size*10);/*estimated*/
                    //newvalue.size=strlen("data")+1;//Serialize(1,size,nnei,nei,&newvalue.data);
                    putflag = DB_CURRENT;
                    /*exit loop and update*/
                    goto put;
                }else{
                    /*find next*/

                    retryCount ++;
                    /*same key=same spectrum, but different graph*/
                    //fprintf(stderr,"(%d:%d)",retryCount,size);
                    getflag = DB_NEXT_DUP;
                    /*scan next*/
                    break;
                }
            }
            break;
        case DB_NOTFOUND:
            /*not found*/
            //printf("Not found.");
            putflag = DB_KEYFIRST;
            count=gcount;
            goto put;
        default:
            /* Error: run recovery. */
            dbenv->err(
                dbenv, ret, "dbc get");
            exit (1);
        }/*switch*/
    }/*for*/
put:
    if(gcount){
        char data[sizeof(int)*size*10];
        newvalue.data=data;
        newvalue.size=Serialize(count,size,nnei,nei,newvalue.data);
        switch((ret=dbc->c_put(dbc, &key,&newvalue,putflag)))
        {
            int t_ret;
        case 0:
            /*close transaction*/
            break;
        case DB_LOCK_DEADLOCK:
        default:
            /* Retry the operation. */
            if ((t_ret = dbc->c_close(dbc)) != 0) {
                dbenv->err( dbenv, t_ret, "dbc->c_close");
                exit (1);
            }
            if (++fail == MAXIMUM_RETRY) 
                /*emergency stop*/
                exit(2);
            //goto retry;
            goto retry;
        }
    } 
    /* Success: commit the change. */
    if ((ret = dbc->c_close(dbc)) != 0) { 
        dbenv->err(dbenv, ret, "dbc->c_close");
        exit (1);
    }
    /* do not close db */
    /*更新後のcountを返す。*/
    return count;
}

void SaveNGPH(int size,int* nnei,int* nei)
{
    int i;
    printf("@NGPH\n%d\n",size);
    for(i=0;i<size;i++){
        int j;
        for(j=0;j<nnei[i];j++){
            printf("%d %d\n",i,nei[j*size+i]);
        }
    }
    printf("-1 -1\n");
}



int db_dump_old(DB_ENV *dbenv, DB* db, int size)
{
    DBT key, value;
    int ret;
    DBC *dbc;
    int getflag=0;

    /*initialization*/
    memset(&key, 0, sizeof(key));
    memset(&value, 0, sizeof(value));
    value.flags = DB_DBT_MALLOC;

retry:
    /*
     * Get the key. If it exists, we increment the value. If it
     * doesn't exist, we create it.
     */
    /*make cursor*/
    ret = db->cursor(db, NULL, &dbc, 0);
    getflag = DB_FIRST;
    for(;;){
        switch(        ret = dbc->c_get(dbc, &key, &value, getflag)){
        case 0:
            /*found*/
            /*lookup the dup records*/
            {
                int rsize;
                int* rnei;
                int* rnnei;
                int count;
                Unserialize(value.data, &count, &rsize, NULL, rnei);
                if(rsize>=size){
                    rnei = malloc(rsize*10*sizeof(int));
                    rnnei = malloc(rsize*sizeof(int));
                    Unserialize(value.data, &count, &rsize, rnnei, rnei);
                    printf("@CNT0\n%d\n",count);
                    SaveNGPH(rsize,rnnei,rnei);
                    free(rnnei);
                    free(rnei);
                }
                if(value.data!=NULL)
                    free(value.data);
                getflag=DB_NEXT;
            }
            break;
        case DB_NOTFOUND:
            /*not found*/
            goto exit;
        default:
            /* Error: run recovery. */
            dbenv->err(
                dbenv, ret, "dbc get");
            exit (1);
        }/*switch*/
    }/*for*/
exit:
    /* do not close db */
    return 0;
}

sSimpleGraph*
db_getnext(DB_ENV *dbenv, DB* db, DBC *dbc, int getflag, int minsize)
{
    DBT key, value;
    int ret;
    sSimpleGraph* graph = NULL;

    /*initialization*/
    memset(&key, 0, sizeof(key));
    memset(&value, 0, sizeof(value));
    value.flags = DB_DBT_MALLOC;

    for(;;){
        switch(        ret = dbc->c_get(dbc, &key, &value, getflag)){
        case 0:
            /*found*/
            /*lookup the dup records*/
            {
                int count;
                int size;
                Unserialize(value.data, &count, &size, NULL, NULL);
                if(size>=minsize){
                    graph = SimpleGraph_New( size );
                    Unserialize(value.data, &graph->count, &graph->size, graph->nnei, graph->nei);
                    if(value.data!=NULL)
                        free(value.data);
                    return graph;
                }
                if(value.data!=NULL)
                    free(value.data);
                getflag=DB_NEXT;
            }
            break;
        case DB_NOTFOUND:
            /*not found*/
            goto exit;
        default:
            /* Error: run recovery. */
            dbenv->err(
                dbenv, ret, "dbc get");
            exit (1);
        }/*switch*/
    }/*for*/
exit:
    /* do not close db */
    return graph;
}

/*rewritten with db_getnext*/
int db_dump(DB_ENV *dbenv, DB* db, int size)
{
    int ret;
    DBC *dbc;
    int getflag=DB_FIRST;

    /*
     * Get the key. If it exists, we increment the value. If it
     * doesn't exist, we create it.
     */
    /*make cursor*/
    ret = db->cursor(db, NULL, &dbc, 0);
    for(;;){
        sSimpleGraph* graph;
        
        graph = db_getnext( dbenv, db, dbc, getflag, size );
        if ( graph == NULL )
            break;
        getflag = DB_NEXT;

        SimpleGraph_SaveNGPH( graph, stdout );

        SimpleGraph_Done( graph );
    }
    /* do not close db */
    return 0;
}

int db_merge(DB_ENV *dbenv, DB* todb, char* from)
{
    DBT key, value;
    int ret;
    DBC *dbc;
    int getflag=0;
    DB *fromdb;

    db_open(dbenv, &fromdb, from, 1);
    /*initialization*/
    memset(&key, 0, sizeof(key));
    memset(&value, 0, sizeof(value));
    value.flags = DB_DBT_MALLOC;
retry:
    /*
     * Get the key. If it exists, we increment the value. If it
     * doesn't exist, we create it.
     */
    /*make cursor*/
    ret = fromdb->cursor(fromdb, NULL, &dbc, 0);
    getflag = DB_FIRST;
    for(;;){
        switch(        ret = dbc->c_get(dbc, &key, &value, getflag)){
        case 0:
            /*found*/
            /*lookup the dup records*/
            {
                int rsize;
                int* rnei;
                int* rnnei;
                int count;
                Unserialize(value.data, &count, &rsize, NULL, rnei);
                rnei = malloc(rsize*10*sizeof(int));
                rnnei = malloc(rsize*sizeof(int));
                Unserialize(value.data, &count, &rsize, rnnei, rnei);
                addRecord2(dbenv,todb,key.data,rsize, rnnei, rnei, count);
                free(rnnei);
                free(rnei);
                if(value.data!=NULL)
                    free(value.data);
                getflag=DB_NEXT;
            }
            break;
        case DB_NOTFOUND:
            /*not found*/
            goto exit;
        default:
            /* Error: run recovery. */
            dbenv->err(
                dbenv, ret, "dbc get");
            exit (1);
        }/*switch*/
    }/*for*/
exit:
    fromdb->close(fromdb, 0);
    return 0;
}

    
/* test program
#include "dm.h"

int main(int argc, char *argv[])
{
    DB* db;
    char buf[1000];
    DB_ENV *dbenv;
    
    env_dir_create();
    env_open(&dbenv);
    db_open(dbenv, &db, "test", 1);
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
            addRecord(dbenv,db,sp,size,nn,n,1);
            free(dm);
            free(sp);
        }
    }
    db->close(db, 0);
}

*/
