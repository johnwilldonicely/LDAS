#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

#define thisprog "xe-timestamp2"
#define TITLE_STRING thisprog" v 1: 7.September.2015 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing synthetic_data</TAGS>

v 1: 7.September.2015[JRH]
	- based on xe-timestamp1.4, but no input required
*/

/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char line[MAXLINELEN],infile[MAXLINELEN],message[MAXLINELEN];
	long ii,jj,nn;
	double aa;
	FILE *fpin;
	/* program-specific variables */
	double samplerate,sampleint,start;
	/* arguments */
	long setn=0;
	int setp=-1,setformat=0,setround=0;
	double setsamp=0.0,seto=0.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Generate time-stamps given a sample-frequency or interval\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [n] [samp]\n",thisprog);
		fprintf(stderr,"	[n]: number of values to generate\n");
		fprintf(stderr,"	[samp]: sample-frequency or interval\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-f format (0=sample-frequency, 1=interval) [%d]\n",setformat);
		fprintf(stderr,"	-o start-sample offset [%g]\n",seto);
		fprintf(stderr,"	-p decimal precision (-1=auto (%%f), 0=none(integer), >0=precision) [%d]\n",setp);
		fprintf(stderr,"	-r round down (0) or to the nearest integer (1) [%d]\n",setround);
		fprintf(stderr,"		NOTE: only applies to integer output (-p 0) \n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s 100 25.0 -f 0 -p 7\n",thisprog);
		fprintf(stderr,"	%s 100 0.04 -f 1 -p 0 -r 1 \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	single column of timestamps\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	setn=atol(argv[1]);
	setsamp=atof(argv[2]);
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-f")==0) setformat=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-o")==0) seto=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-p")==0) setp=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-r")==0) setround=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setp!=-1&&setp<0) {fprintf(stderr,"\n--- Error[%s]: -p (%d) option must be positive or -1\n\n",thisprog,setp);exit(1);}
	if(setround!=0&&setround!=1) {fprintf(stderr,"\n--- Error[%s]: -r (%d) option must be 0 or 1\n\n",thisprog,setround);exit(1);}

	if(setformat==0) { samplerate=setsamp; sampleint=1.0/samplerate; }
	else if(setformat==1) { sampleint=setsamp; samplerate=1.0/sampleint; }
	else {fprintf(stderr,"\n--- Error[%s]: -f (%d) option must be 0 or 1\n\n",thisprog,setformat);exit(1);}

	start=(seto*samplerate);

	/* OUTPUT */
	if(setp==-1) {
		for(ii=0;ii<setn;ii++) printf("%f\n",(start++ * sampleint));
	}

	if(setp==0) {
		if(setround==0) for(ii=0;ii<setn;ii++) printf("%ld\n",(long)(start++ * sampleint));
		if(setround==1) for(ii=0;ii<setn;ii++) printf("%ld\n",(long)(0.5+(start++ * sampleint)));
	}

	if(setp>0) {
		for(ii=0;ii<setn;ii++) printf("%.*f\n",setp,(start++ * sampleint));
	}

	exit(0);
}
