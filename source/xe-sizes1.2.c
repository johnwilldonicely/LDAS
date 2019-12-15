#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <stdint.h>

#define thisprog "xe-sizes1"
#define TITLE_STRING thisprog" v 2: 7.February.2014 [JRH]"

/*
<TAGS>programming</TAGS>

v 2: 7.February.2014 [JRH]
	- remove use of stderr for outputting some text
	- allows simple output filtering using grep
*/

/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],*line=NULL,*templine=NULL,word[256],*pline,*pcol;
	long int i,j,k,n,nchars=0,maxlinelen=10000;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d;
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin,*fpout;
	size_t ii,jj,kk,nn,mm;
	size_t sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);

	/* program-specific variables */
	/* arguments */

	printf("\n");
	printf("----------------------------------------------------------------------\n");
	printf("%s\n",TITLE_STRING);
	printf("----------------------------------------------------------------------\n");
	printf("Programming tool: report size of numerical types\n");
	printf("----------------------------------------------------------------------\n");

	printf("char:\t\t%ld bytes\t%ld bits\n",sizeof(char),8*sizeof(char));
	printf("short:\t\t%ld bytes\t%ld bits\n",sizeof(short),8*sizeof(short));
	printf("int:\t\t%ld bytes\t%ld bits\n",sizeof(int),8*sizeof(int));
	printf("long:\t\t%ld bytes\t%ld bits\n",sizeof(long),8*sizeof(long));
	printf("long long:\t%ld bytes\t%ld bits\n",sizeof(long long),8*sizeof(long long));
	printf("size_t:\t\t%ld bytes\t%ld bits\n",sizeof(size_t),8*sizeof(size_t));
	printf("off_t:\t\t%ld bytes\t%ld bits\n",sizeof(off_t),8*sizeof(size_t));
	printf("float:\t\t%ld bytes\t%ld bits\n",sizeof(float),8*sizeof(float));
	printf("double:\t\t%ld bytes\t%ld bits\n",sizeof(double),8*sizeof(double));
	printf("long double:\t%ld bytes\t%ld bits\n",sizeof(long double),8*sizeof(long double));
	printf("\n");
	printf("max sizes as defined in <limits.h> and <float.h> headers:\n");
	printf("NOTE: all are for unsigned values, except size_t:\n\n");
	printf("char:		%d to %d\n",CHAR_MIN,CHAR_MAX);
	printf("short:		%d to %d\n",SHRT_MIN,SHRT_MAX);
	printf("int:		%d to  %d (%.2lf GB)\n",INT_MIN,INT_MAX,((double)INT_MAX/1000000000L));
	printf("long:		%ld to %ld\n",LONG_MIN,LONG_MAX);
	printf("long long:	%lld to %lld\n",LLONG_MIN,LLONG_MAX);
	printf("size_t:		0 to %ld\n",SIZE_MAX);
	printf("float:		%g to %g\n",-FLT_MAX,FLT_MAX);
	printf("double: 	%g to %g\n",-DBL_MAX,DBL_MAX);
	printf("long double:	%Lg to %Lg\n",-LDBL_MAX,LDBL_MAX);

	printf("\n");
	printf("absolute numerical limits for given byte-sizes:\n\n");
	printf("2^8: %ld\n", (long)pow(2.0,8.0));
	printf("2^16: %ld\n",(long)pow(2.0,16.0));
	printf("2^32: %g\n",pow(2.0,32.0));
	printf("2^64: %g x 2\n",pow(2.0,63.0));
	printf("\n");
	exit(0);
	}
