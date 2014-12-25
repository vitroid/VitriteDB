#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <db.h>
#include "GraphDB2.h"
#include "NetMatch.h"
#include "dm.h"

#define DIE_IF(cond,...) if(cond){printf(__VA_ARGS__);exit(2);}

void usage(char* arg)
{
    printf("usage: %s [-s n] envdir dbname < infile\n",arg);
    printf("Lookup subgraphs given in infile from the db and show values.\n");
    printf("\t-s n\tShow clusters with size not less than n.\n");
    exit(1);
}



int main(int argc,char *argv[])
{
    DB* db;
    DB_ENV *dbenv;
    extern char *optarg;
    extern int optind;
    char ch;
    int size=0;
    char buf[1000];
    

    while ((ch = getopt(argc, argv, ":s:")) != EOF)
	switch (ch) {
	case 's':
	    /*minimal size of the cluster*/
	    size=atoi(optarg);
	    break;
	case '?':
	case ':':
	default:
	    usage(argv[0]);
	}
    argc -= optind;
    if(argc!=2)
	usage(argv[0]);
    argv += optind;

    /*prepare db*/
    env_dir_create(argv[0]);
    env_open(&dbenv,argv[0],50000000);
    db_open(dbenv, &db, argv[1], 1);
    while ( NULL!=fgets(buf, sizeof(buf), stdin) ){
        if ( strncmp( buf, "@NGPH", 5 ) == 0 ){
            sIntMatrix2 *ref;
            int* sp;
            int count;
            
            ref = IntMatrix2_LoadNGPH( stdin );
            sp = DistanceMatrix2Spectrum( ref->n, ref->a );
            count = addRecord2( dbenv, db, sp, ref->n, ref->nnei, ref->nei, 0 );
            printf("%d\n", count);
            free(sp);
            IntMatrix2_Done( ref );
        }
    }
    
    /*close db*/
    db->close(db, 0);
    exit(0);
}
