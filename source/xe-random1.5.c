
#define thisprog "xe-random1"
#define TITLE_STRING thisprog" v 5: 1.December.2015"
#define MAXLINE 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>   /* needed for time() function */
#include <unistd.h>	/* needed for getpid() function */


/*
<TAGS>math synthetic_data</TAGS>

v 5: 1.December.2015
	- actually added -min option
	- update counter variables

v 1.3: 8.October.2013
	- improved seed, using time + process-id
	- renamed from xe-random1 to xe-randomint1
*/

/* external functions start */
double xf_rand1_d(double setmax);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],line[256],word[256],*matchstring=NULL;
	int w,x,y,z;
	long ii,jj,kk,mm,nn;
	int sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofchar=sizeof(char);
	float a,b,c;
	double aa,bb,cc;
	FILE *fpin,*fpout;

	/* program-specific variables */
	int goodsample,goodtest;
	long *countrand=NULL;
	unsigned long seed;
	double range,twopi=2.0*M_PI;

	/* arguments */
	int setgaussian=1;
	long setn=-1,setfloat=0;
	double setmin=0.0,setmax=1.0,setsd=1.0;

	/* PRINT INSTRUCTIONS IF NO ARGUMENTS ARE SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Produce a series of random integers from [min] to [max]\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [n] [optional arguments]\n",thisprog);
		fprintf(stderr,"		[n]: number of random integers to generate\n");
		fprintf(stderr,"OPTIONAL ARGUMENTS:\n");
		fprintf(stderr,"	-g: uniform (0) or Gaussian (1) distribution [%d]\n",setgaussian);
		fprintf(stderr,"    if uniform:\n");
		fprintf(stderr,"	-min: lowest number [%g]\n",setmin);
		fprintf(stderr,"	-max: highest number [%g]\n",setmax);
		fprintf(stderr,"    if Gaussian, mean is zero:\n");
		fprintf(stderr,"	-sd: standard deviation of distribution [%g]\n",setsd);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s 60\n",thisprog);
		fprintf(stderr,"	%s 25 -min -10 -max 10 -r 2\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	setn=atol(argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-g")==0)   setgaussian=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-min")==0) setmin=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-max")==0) setmax=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-sd")==0)   setsd=atof(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setgaussian!=0 && setgaussian!=1) {fprintf(stderr,"\n--- Error[%s]: invalid gaussian switch (-g %d) - must be 0 or 1\n\n",thisprog,setgaussian); exit(1);}

	range=setmax-setmin;

	/* SET THE SEED VALUE FOR THE RAND FUNCTION */
	seed= (unsigned long) (time(NULL) + getpid());
	srand( seed );


	/* GENERATE THE RANDOM NUMBERS IF NO LIMIT ON NUMBER OF REPEATS */
	for(ii=0;ii<setn;ii++) {
		if(setgaussian==1) {
			aa= xf_rand1_d(1.0);
			bb= xf_rand1_d(1.0);
			cc= setsd * sqrt(-2.0*log(aa)) * cos(twopi*bb) ; // Box-Muler method to convert from uniform to normal distribution
			}
		else {
			cc= setmin + xf_rand1_d(range);
		}
		printf("%.12g\n",cc);
	}

	exit(0);
	}
