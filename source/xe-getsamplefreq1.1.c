#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-getsamplefreq1"
#define TITLE_STRING thisprog" v 1: 15.February.2015 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>math signal_processing time</TAGS>
*/

/* external functions start */
int xf_percentile1_d(double *data, long n, double *result);
int xf_compare1_d(const void *a, const void *b);
/* external functions end */



int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],line[MAXLINELEN];
	long int ii,jj,kk,nn;
	double aa,bb,cc,dd, result_d[64];
	double *data=NULL;
	size_t sizeofdouble=sizeof(double);
	FILE *fpin,*fpout;

	/* arguments*/
	long set_n=1000;
	double set_mult=1.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Determine the sampling frequency from a series of timestamps\n");
		fprintf(stderr,"Uses the median value for the interval between values\n");
		fprintf(stderr,"Non-numerical values, INF, NAN, or blank lines will result in errors\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\", single column of numbers\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-n : maximum numbers to read [%ld]\n",set_n);
		fprintf(stderr,"	-mult : multiplier to convert timestamps to seconds [%g]\n",set_mult);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -n 100\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -n 1000 -m 60\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	A single value representing 1 / <median interval>\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-n")==0)   set_n=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-mult")==0)   set_mult=atof(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	/* STORE DATA */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf",&aa)!=1){
			fprintf(stderr,"\n--- Error[%s]: non-numerical data found on line %ld: %s\n",thisprog,(nn+1),line);
			exit(1);
		}
		else if(isfinite(aa)) {
			if((data=(double *)realloc(data,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			data[nn++]=aa*set_mult;
		}
		else {
			fprintf(stderr,"\n--- Error[%s]: non-finite number found on line %ld: %s\n",thisprog,(nn+1),line);
			exit(1);
		}
		if(set_n>0 && nn>=set_n) break;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* MAKE SURE THERE IS ENOUGH DATA */
	if(nn<2) {
		fprintf(stderr,"\n--- Error[%s]: file contains %ld timestamps - insufficient for calculating sampling frequency\n\n",thisprog,nn);
		exit(1);
	}

	/* DETERMINE THE INERTVALS */
	nn--;
	for(ii=0;ii<nn;ii++) {
		data[ii]=data[ii+1]-data[ii];
	}

	if(xf_percentile1_d(data,nn,result_d)!=0) {
		fprintf(stderr,"\n--- Error[%s]: insufficient memory for calculation of percentiles\n\n",thisprog);
		exit(1);
	}

	printf("%g\n",(1.0/result_d[5]));

	free(data);
	exit(0);
	}
