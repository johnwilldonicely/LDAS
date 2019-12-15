#define thisprog "xe-math_index1"
#define TITLE_STRING thisprog" 24.February.2019 [JRH]"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#define MAXLINELEN 1000

/*
<TAGS>math screen</TAGS>

24.February.2019 [JRH]
 */

/* external functions start */
long xf_getindex1_d(double min, double max, long n, double value, char *message);
/* external functions end */


int main(int argc, char *argv[]) {

	char message[256];
	long ii,jj,kk;
	double aa,bb,cc,dd;
	/* arguments */
	long setn;
	double setmin,setmax,setval;

	/******************************************************************************
	If only one argument (executable's name) print instructions
	******************************************************************************/
	if(argc<5) 	{
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Identify the index to an item in a list, given the list parameters\n");
		fprintf(stderr,"- use when there's no no index (e.g. time) associated with each value\n");
		fprintf(stderr,"USAGE: %s [min] [max] [n] [val]\n",thisprog);
		fprintf(stderr,"	[min] : minimum value in series\n");
		fprintf(stderr,"	[max] : maximum value in series\n");
		fprintf(stderr,"	[n]   : number of items in  series\n");
		fprintf(stderr,"	[val] : value for which index is required\n");
		fprintf(stderr,"EXAMPLES: in a 15-second time series sampled at 100Hz...\n");
		fprintf(stderr," - find index to value at 7 seconds\n");
		fprintf(stderr,"	%s 0 15 100 7\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	- index to the value, rounded down to the nearest integer\n");
		fprintf(stderr,"	- NOTE: index is zero-offset\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		exit(0);
		fprintf(stderr,"\n");
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	setmin= atof(argv[1]);
	setmax= atof(argv[2]);
	setn=   atol(argv[3]);
	setval= atof(argv[4]);

	if(setn<1) {fprintf(stderr,"\n--- Error[%s]: [n] %ld must be >0\n\n",thisprog,setn);exit(1);}
	if(setval<setmin) {fprintf(stderr,"\n--- Error[%s]: [val] %g is less than [min] %g\n\n",thisprog,setval,setmin);exit(1);}
	if(setval>setmax) {fprintf(stderr,"\n--- Error[%s]: [val] %g is greater than [max] %g\n\n",thisprog,setval,setmax);exit(1);}

	jj= xf_getindex1_d(setmin,setmax,setn,setval,message);
	if(jj<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message);exit(1);}

	printf("%ld\n",jj);
	exit(0);
}
