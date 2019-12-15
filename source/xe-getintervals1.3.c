#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#define thisprog "xe-getintervals1"
#define TITLE_STRING thisprog" v 3: 9.November.2018 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>screen time</TAGS>

v 3: 9.November.2018 [JRH]
	- switch to long int for critical variables
	- use -DBL_MAX and DBL_MAX for default min & max values
	- update variable name conventions
	- resolve some "uninitialized variable" warnings

v 3: 11.February.2014 [JRH]
	- allow min and max to be the same (ie. detect intervals where an exact condition is met)

v 2: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/


/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char line[MAXLINELEN],*pline,*pcol;
	long ii,jj,kk,nn;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	long count=0;
	double ttime=0.,tstart=0.,tend=0.,ttot=0.,trialstart=0.,trialdur=0.,data;
	/* arguments */
	char *infile;
	int setwinmax=0;
	double datamin=-DBL_MAX,datamax=DBL_MAX,winmin=0,winmax=0.;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Get time intervals during which a data condition (min,max) is met\n");
		fprintf(stderr,"USAGE: %s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: input file or \"stdin\", of format <time> <value>\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-min: minimum value [%g]\n",datamin);
		fprintf(stderr,"	-max: maximum value [%g]\n",datamax);
		fprintf(stderr,"	-imin: min interval length for output [%g)\n",winmin);
		fprintf(stderr,"	-imax: max interval length [unset by default]\n");
		fprintf(stderr,"		: will cause longer intervals to be split\n");
		fprintf(stderr,"		: set the same as -imin to enforce uniform intervals\n");
		fprintf(stderr,"NOTE: relaxed criteria = fewer intervals, longer summed durations\n",thisprog);
		fprintf(stderr,"EXAMPLEs: \n");
		fprintf(stderr,"	cut -f 1,5 crunch_pos.txt | %s stdin 0 20 0\n",thisprog);
		fprintf(stderr,"	%s pos.txt 0 20 -imin .1 -max .1\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	start and end ttimes for valid intervals\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-min")==0)  datamin=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-max")==0)  datamax=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-imin")==0) winmin=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-imax")==0) { winmax=atof(argv[ii+1]); setwinmax=1; ii++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	//printf("%g %g    %g\n",datamin,datamax,winmin);
	if(datamax<datamin) {fprintf(stderr,"\n--- Error[%s]: max (%g) must be greater or equal to min (%g)\n\n",thisprog,datamax,datamin);exit(1);}
	if(setwinmax==1&&winmax<winmin) {fprintf(stderr,"\n--- Error[%s]: -imax (%g) must be equal to or greater than -imin (%g)\n\n",thisprog,winmax,winmin);exit(1);}

	/* READ DATA */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	z=0; nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {

		if(sscanf(line,"%lf %lf",&ttime,&data)!=2) continue;
		if(count==0) trialstart=ttime;

		// IF GOOD DATA FOUND, CHECK IF ITS PART OF A SERIES AND IF MAXIMUM DURATION WAS EXCEEDED
		if(data>=datamin && data<=datamax) {

			// if this follows bad data, set new start ttime
			if(z==0) { tstart=ttime; z=1; }
			// if this follows good data and max duration was set and exceeded...
			else if(setwinmax==1 && (ttime-tstart)>=winmax) {
				for(aa=tstart;(aa+winmax)<=ttime;aa+=winmax) {
					printf("%lf\t%lf\n",aa,(aa+winmax));
					ttot+=winmax;
					tstart+=winmax;
					nn++;
		}}}
		// IF OUT OF RANGE DATA FOUND, CHECK IF MINIMUM DURATION CRITERION WAS MET
		else {
			if(z==1 && (tend-tstart)>=winmin) {
				printf("%lf\t%lf\n",tstart,tend);
				ttot+=tend-tstart;
				nn++;
			}
			z=0;
		}
		tend=ttime;
		count++;
	}

	if(z==1 && (tend-tstart)>=winmin) { printf("%lf\t%lf\n",tstart,tend); nn++; ttot+=tend-tstart; }
	printf("n_intervals: %d\n",nn);
	printf("summed_intervals: %g\n",ttot);
	printf("trial_duration: %g\n",(tend-trialstart));

	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	exit(0);
}
