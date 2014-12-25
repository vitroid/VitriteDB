/*初期配置がそれなりにリーズナブルであることを期待し、斥力を与えないで結合と角度を最適化する。graphquench.cは角度は最適化せず、斥力の作用にまかせていた。*/
/*In the final stage, make the force constant for bond stretching stronger, and output the potential component from bending only.*/
/*graphquench4bとの違いは、角度の個数で割っていることだけ。*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#define REPUL 0.1
extern double fabs(double);
extern double rint(double);

int pbc=0;

int  nmol;
int* nnei;
int* nei;

double diagonal;

void register_bond
(
    int x,
    int y
)
{
    int i;
    for( i=0; i< nnei[x]; i++ ){
        if ( nei[ x + i*nmol ] == y )
            return;
    }
    nei[ x + nnei[x]*nmol ] = y;
    nnei[x]++;
}


/* they should be global, sorry.*/
double boxx,boxy,boxz;
double bend;
int count=1,next=1;
double f(double *x)
{
    int i;
    int nangle = 0;
    double pot=0;
    bend=0;
    
    for(i=0;i<nmol;i++) {
        int k;
        double xi,yi,zi;
        xi = x[i*3+1];
        yi = x[i*3+2];
        zi = x[i*3+3];
        for( k=0; k<nnei[i]; k++ ){
            double r;
            double xj,yj,zj;
            int j;
            double dx,dy,dz;
            j = nei[ i + k*nmol ];
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
            r=sqrt(dx*dx+dy*dy+dz*dz)-1.0L;
            pot+=10000.0L * r*r;
        }
    }
    bend=0;
    for(i=0;i<nmol;i++) {
        int k1;
        double xi,yi,zi;
        xi = x[i*3+1];
        yi = x[i*3+2];
        zi = x[i*3+3];
        for( k1=0; k1<nnei[i]; k1++ ){
            double xj1,yj1,zj1;
            int j1;
            int k2;
	    double r;
	    
            j1 = nei[ i + k1*nmol ];
            xj1 = x[j1*3+1] - xi;
            yj1 = x[j1*3+2] - yi;
            zj1 = x[j1*3+3] - zi;
	    if ( pbc ){
	      xj1 -= rint(xj1/boxx)*boxx;
	      yj1 -= rint(yj1/boxy)*boxy;
	      zj1 -= rint(zj1/boxz)*boxz;
	    }
	    r = sqrt( xj1*xj1 + yj1*yj1 + zj1*zj1 );
	    xj1 /= r;
	    yj1 /= r;
	    zj1 /= r;
	    
            for( k2=k1+1; k2<nnei[i]; k2++ ){
                double xj2,yj2,zj2;
                int j2;
                j2 = nei[ i + k2*nmol ];
                xj2 = x[j2*3+1] - xi;
                yj2 = x[j2*3+2] - yi;
                zj2 = x[j2*3+3] - zi;
                if ( pbc ){
                    xj2 -= rint(xj2/boxx)*boxx;
                    yj2 -= rint(yj2/boxy)*boxy;
                    zj2 -= rint(zj2/boxz)*boxz;
                }
		r = sqrt( xj2*xj2 + yj2*yj2 + zj2*zj2 );
		xj2 /= r;
		yj2 /= r;
		zj2 /= r;
                r= xj1*xj2 + yj1*yj2 + zj1*zj2 + 1.0L/3.0L;
                bend+=r*r;
		nangle ++;
            }
        }
    }
    bend /= nangle;
    pot+=bend;
    if(--count <= 0 ){
      next+=next;
      count=next;
      fprintf(stderr,"%f %f %d\n",pot,bend,count);
    }
    return pot;
}

void df(double x[],double xd[])
{
    int i,j;
    double xi,yi,zi;
    double xj,yj,zj;
    double dx,dy,dz;
    double pot=0;
    
    for(i=1;i<=nmol*3;i++)
        xd[i]=0;
    for(i=0;i<nmol;i++) {
        int k;
        double xi,yi,zi;
        xi = x[i*3+1];
        yi = x[i*3+2];
        zi = x[i*3+3];
        for( k=0; k<nnei[i]; k++ ){
            double r;
            double xj,yj,zj;
            int j;
            double dx,dy,dz;
            j = nei[ i + k*nmol ];
            xj = x[j*3+1];
            yj = x[j*3+2];
            zj = x[j*3+3];
            dx = xi-xj;
            dy = yi-yj;
            dz = zi-zj;
            if ( pbc ){
                dx -= rint(dx/boxx)*boxx;
                dy -= rint(dy/boxy)*boxy;
                dz -= rint(dz/boxz)*boxz;
            }
            r=sqrt(dx*dx+dy*dy+dz*dz);
            xd[i*3+1]+=2.*dx/r*(r-1.0L) * 10000.0L;
            xd[i*3+2]+=2.*dy/r*(r-1.0L) * 10000.0L;
            xd[i*3+3]+=2.*dz/r*(r-1.0L) * 10000.0L;
            xd[j*3+1]-=2.*dx/r*(r-1.0L) * 10000.0L;
            xd[j*3+2]-=2.*dy/r*(r-1.0L) * 10000.0L;
            xd[j*3+3]-=2.*dz/r*(r-1.0L) * 10000.0L;
        }
    }
    for(i=0;i<nmol;i++) {
        int k1;
        double xi,yi,zi;
        xi = x[i*3+1];
        yi = x[i*3+2];
        zi = x[i*3+3];
        for( k1=0; k1<nnei[i]; k1++ ){
            double xj1,yj1,zj1;
            int j1;
            int k2;
	    double r1;
	    
            j1 = nei[ i + k1*nmol ];
            xj1 = x[j1*3+1] - xi;
            yj1 = x[j1*3+2] - yi;
            zj1 = x[j1*3+3] - zi;
	    if ( pbc ){
	      xj1 -= rint(xj1/boxx)*boxx;
	      yj1 -= rint(yj1/boxy)*boxy;
	      zj1 -= rint(zj1/boxz)*boxz;
	    }
	    r1 = sqrt( xj1*xj1 + yj1*yj1 + zj1*zj1 );
	    xj1 /= r1;
	    yj1 /= r1;
	    zj1 /= r1;

            for( k2=k1+1; k2<nnei[i]; k2++ ){
                double xj2,yj2,zj2;
                double fx1,fy1,fz1;
                double fx2,fy2,fz2;
                int j2;
                double r2, a;
                
                j2 = nei[ i + k2*nmol ];
                xj2 = x[j2*3+1] - xi;
                yj2 = x[j2*3+2] - yi;
                zj2 = x[j2*3+3] - zi;
                if ( pbc ){
                    xj2 -= rint(xj2/boxx)*boxx;
                    yj2 -= rint(yj2/boxy)*boxy;
                    zj2 -= rint(zj2/boxz)*boxz;
                }
		r2 = sqrt( xj2*xj2 + yj2*yj2 + zj2*zj2 );
		xj2 /= r2;
		yj2 /= r2;
		zj2 /= r2;

		a = xj1*xj2 + yj1*yj2 + zj1*zj2;
		
		fx1 = 2.0L * ( a + 1.0L/3.0L ) * ( xj2 - a * xj1 ) / r1;
		fy1 = 2.0L * ( a + 1.0L/3.0L ) * ( yj2 - a * yj1 ) / r1;
		fz1 = 2.0L * ( a + 1.0L/3.0L ) * ( zj2 - a * zj1 ) / r1;
		fx2 = 2.0L * ( a + 1.0L/3.0L ) * ( xj1 - a * xj2 ) / r2;
		fy2 = 2.0L * ( a + 1.0L/3.0L ) * ( yj1 - a * yj2 ) / r2;
		fz2 = 2.0L * ( a + 1.0L/3.0L ) * ( zj1 - a * zj2 ) / r2;

                xd[j1*3+1]+=fx1;
                xd[j1*3+2]+=fy1;
                xd[j1*3+3]+=fz1;
                xd[j2*3+1]+=fx2;
                xd[j2*3+2]+=fy2;
                xd[j2*3+3]+=fz2;
                xd[i*3+1]-=(fx1+fx2);
                xd[i*3+2]-=(fy1+fy2);
                xd[i*3+3]-=(fz1+fz2);
            }
        }
    }
}

double Graph_Quench(double *x)
{
    int iter;
    double fmin;
    x--;/*recipes require arrays beginning from [1..]*/
    iter=10000;
    frprmn(x,nmol*3,1e-6,&iter,&fmin,&f,&df);
    return fmin;
}


int main(int argc,char *argv[])
{
    char    buf[256],box[256];
    double* coord   = NULL;
    int     nocoord = 0;
    int     graph;
    
    
    diagonal = 2.0L*sqrt(2.0L)/sqrt(3.0L);
    
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
        if(strncmp(buf,"@NGPH",5)==0) {
            graph = 1;
            fgets( buf, sizeof(buf), stdin );
            nmol = atoi(buf);
            nnei = calloc( nmol, sizeof( int ));
            nei = malloc( nmol*10*sizeof( int ));
            while(NULL!=fgets(buf,sizeof(buf),stdin)) {
                int i,j;
                sscanf( buf, "%d %d", &i, &j );
                if ( i < 0 )
                    break;
                register_bond( i,j );
                register_bond( j,i );
            }
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
            nmol = atoi(buf);
            coord = malloc(sizeof(double)*(nmol*3+1));
            for(i=0;i<nmol;i++){
                fgets(buf,sizeof(buf),stdin);
                sscanf(buf,"%lf %lf %lf",&coord[i*3+0],&coord[i*3+1],&coord[i*3+2]);
            }
        }
        if( graph && ( coord || nocoord ))
        {
            int i;
	    double fmin;
            if ( nocoord ) {
                coord = malloc( sizeof( double ) * ( nmol * 3 + 1 ) );
                for ( i=0; i<nmol*3; i++ ){
                    coord[i] = drand48()*4-2;
                }
            }

            fmin = Graph_Quench( coord );
            if ( pbc ){
                printf("@BOX3\n%24.17e %24.17e %24.17e\n",boxx, boxy, boxz);
            }
            printf("@ARD0\n%f\n@AR3A\n%d\n",bend,nmol);
	    double comx,comy,comz;
	    comx = comy = comz = 0;
            for(i=0;i<nmol;i++) {
	      comx += coord[i*3+0];
	      comy += coord[i*3+1];
	      comz += coord[i*3+2];
            }
	    comx /= nmol;
	    comy /= nmol;
	    comz /= nmol;

            for(i=0;i<nmol;i++) {
                printf("%f %f %f\n",coord[i*3+0] - comx,coord[i*3+1]-comy,coord[i*3+2]-comz);
            }
            free( coord );
            coord = NULL;
            graph = 0;
        }
    }
    exit(0);
}
