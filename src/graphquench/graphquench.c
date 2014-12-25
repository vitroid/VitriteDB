/*PBCで使用する場合、linmin.c(共役勾配法の初期手順)のステップ幅(変数xx */
/*の初期値)をうまく決めないと、極端に異なる配置空間を探索してしまう可 */
/*能性がある。*/
/*座標が読みこまれなかった場合はまったく適当に座標を与えることにした。*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include "Graph.h"
#define REPUL 3
extern double fabs(double);
extern double rint(double);

int nnode;
sGraph *graph = NULL;
int pbc=0;

/* they should be global, sorry.*/
double boxx,boxy,boxz;

double f(double *x)
{
    int i,j;
    double xi,yi,zi;
    double xj,yj,zj;
    double dx,dy,dz;
    double pot=0;
    
    for(i=0;i<nnode;i++)
      {
	  xi = x[i*3+1];
	  yi = x[i*3+2];
	  zi = x[i*3+3];
	  for(j=i+1;j<nnode;j++)
	    {
		xj = x[j*3+1];
		yj = x[j*3+2];
		zj = x[j*3+3];
		dx = xi-xj;
		dy = yi-yj;
		dz = zi-zj;
                if ( pbc ){
/*		fprintf(stderr,"d0:%f %f %f\n",dx,dy,dz);
		fprintf(stderr,"b:%f %f %f\n",boxx,boxy,boxz);*/
                    dx -= rint(dx/boxx)*boxx;
                    dy -= rint(dy/boxy)*boxy;
                    dz -= rint(dz/boxz)*boxz;
/*		fprintf(stderr,"d1:%f %f %f\n",dx,dy,dz);*/
                }
                
		if(graph->am[j*nnode+i]) {
                    double r=dx*dx+dy*dy+dz*dz-1;
                    pot+=r*r;
                }
		else {
                    pot += REPUL/(dx*dx+dy*dy+dz*dz);
                }
	    }
      }
    //fprintf(stderr,"%f\n",pot);
    return pot;
}

void df(double x[],double xd[])
{
    int i,j;
    double xi,yi,zi;
    double xj,yj,zj;
    double dx,dy,dz;
    double pot=0;
    
    for(i=1;i<=nnode*3;i++)
        xd[i]=0;
    for(i=0;i<nnode;i++) {
        xi = x[i*3+1];
        yi = x[i*3+2];
        zi = x[i*3+3];
        for(j=i+1;j<nnode;j++) {
            xj = x[j*3+1];
            yj = x[j*3+2];
            zj = x[j*3+3];
            dx = xi-xj;
            dy = yi-yj;
            dz = zi-zj;

            if ( pbc ) {
                dx -= rint(dx/boxx)*boxx;
                dy -= rint(dy/boxy)*boxy;
                dz -= rint(dz/boxz)*boxz;
            }
            
            if( graph->am[j*nnode+i] ) {
                double r;
                r=dx*dx+dy*dy+dz*dz-1;
                xd[i*3+1]+=4.*dx*r;
                xd[i*3+2]+=4.*dy*r;
                xd[i*3+3]+=4.*dz*r;
                xd[j*3+1]-=4.*dx*r;
                xd[j*3+2]-=4.*dy*r;
                xd[j*3+3]-=4.*dz*r;
            }
            else {
                pot = 1./(dx*dx+dy*dy+dz*dz);
                pot*=-2*REPUL*pot;
                xd[i*3+1]+=dx*pot;
                xd[i*3+2]+=dy*pot;
                xd[i*3+3]+=dz*pot;
                xd[j*3+1]-=dx*pot;
                xd[j*3+2]-=dy*pot;
                xd[j*3+3]-=dz*pot;
            }
        }
    }
}

void Graph_Quench(double *x)
{
    int iter;
    double fmin;
    x--;/*recipes require arrays beginning from [1..]*/
    frprmn(x,nnode*3,1e-4,&iter,&fmin,&f,&df);
}


int main(int argc,char *argv[])
{
    char    buf[256],box[256];
    double* coord   = NULL;
    int     nocoord = 0;
    

    srand48(getpid());

    while (1) {
        /*for getopt*/
        int        c;
        int option_index = 0;
        static struct option long_options[] = {
            {"pbc", 0, 0, 0},
            {"nocoord", 0, 0, 0},
            {0, 0, 0, 0}
        };
        
        c = getopt_long (argc, argv, "",
                         long_options, &option_index);
        if (c == -1)
            break;
        
        switch (c) {
        case 0:
            if ( strcmp( long_options[option_index].name, "pbc" ) == 0 ){
                pbc++;
            }
            else if ( strcmp( long_options[option_index].name, "nocoord" ) == 0 ){
                nocoord++;
            }
            else{
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                //usage( argc, argv );
                exit(1);
            }
            break;
            
        default:
            printf ("?? getopt returned character code 0%o ??\n", c);
            //usage( argc, argv );
        }
    }


    while(NULL!=fgets(buf,sizeof(buf),stdin)) {
        if(strncmp(buf,"@GRPH",5)==0) {
            sGraph* tmp;
            tmp = Graph_Load(stdin);
            Graph_Save_NGPH(tmp,stdout);
            graph = Digraph2Graph(tmp);
        }
        else if(strncmp(buf,"@NGPH",5)==0) {
            sGraph *tmp;
            tmp = Graph_Load_NGPH(stdin);
            Graph_Save_NGPH(tmp,stdout);
            graph = Digraph2Graph(tmp);
        }
        else if(0==strncmp(buf,"@BXLA",5)) {
            fgets(box,sizeof(box),stdin);
            boxx=boxy=boxz=atof(box);
            pbc++;
        }
        else if(0==strncmp(buf,"@BOX3",5)) {
            fgets(box,sizeof(box),stdin);
            sscanf(box,"%lf %lf %lf",&boxx,&boxy,&boxz);
            pbc++;
        }
        else if((0==strncmp(buf,"@AR3A",5))||(0==strncmp(buf,"@NX3A",5))) {
            int i;
            fgets(buf,sizeof(buf),stdin);
            nnode = atoi(buf);
            coord = malloc(sizeof(double)*(nnode*3+1));
            for(i=0;i<nnode;i++){
                fgets(buf,sizeof(buf),stdin);
                sscanf(buf,"%lf %lf %lf",&coord[i*3+0],&coord[i*3+1],&coord[i*3+2]);
            }
        }
        if( graph && ( coord || nocoord ))
        {
            int i;
            if ( nocoord ) {
              fprintf(stderr, "Generating coord...\n");
                nnode = graph->nnode;
                coord = malloc( sizeof( double ) * ( nnode * 3 + 1 ) );
                for ( i=0; i<nnode*3; i++ ){
                    coord[i] = drand48()*4-2;
                }
            }

            Graph_Quench( coord );
            if ( pbc ){
                printf("@BOX3\n%24.17e %24.17e %24.17e\n",boxx, boxy, boxz);
            }
            printf("@AR3A\n%d\n",nnode);
            for(i=0;i<nnode;i++) {
                printf("%f %f %f\n",coord[i*3+0],coord[i*3+1],coord[i*3+2]);
            }
            free( coord );
            Graph_Done( graph );
            graph = NULL;
            coord = NULL;
        }
    }
    exit(0);
}
