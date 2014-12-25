#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
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
    if(argc!=0)
	usage(argv[0]);
    argv += optind;

    while ( NULL!=fgets(buf, sizeof(buf), stdin) ){
      if ( strncmp( buf, "@NGPH", 5 ) == 0 ){
        sIntMatrix2 *ref;
        int* sp;
        int count;
        int i,j;
        //md5
        MD4_CTX state;
        unsigned char digest[MD4_DIGEST_LENGTH];

        ref = IntMatrix2_LoadNGPH( stdin );
        sp = DistanceMatrix2Spectrum( ref->n, ref->a );
	//MD4 is fastest
        MD4_Init(&state);
        MD4_Update(&state, (const unsigned char *)sp, sizeof(int)*ref->n*10);
        MD4_Final(digest, &state);

        for (i = 0; i < MD4_DIGEST_LENGTH; ++i) {
          printf("%02x", digest[i]);
        }
        //putchar('\n');
        free(sp);
        IntMatrix2_Done( ref );
      }
    }
    
    /*close db*/
    exit(0);
}
