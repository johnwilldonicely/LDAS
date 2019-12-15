#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-template1"
#define TITLE_STRING thisprog" v 1: 14.August.2012 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>signal_processing stats</TAGS>
*/

/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *infile,*outfile,line[MAXLINELEN],templine[MAXLINELEN],word[256],*pline,*pcol;
	long int ii,jj,kk,nn=0,mm;
	int v,w,x,y,z,col,colmatch;
	int sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	double *xdat=NULL,*xcor=NULL;
	/* arguments */
	double setsfreq=1.0,setmax=-1.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate autocorrelation function on a time-series\n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-sf: sample frequency (Hz) [%g]\n",setsfreq);
		fprintf(stderr,"	-max: max lag to calculate (seconds, -1 = auto) [%g]\n",setmax);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -sf 1500\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -sf 24000\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	1st column: time-lag (seconds)\n");
		fprintf(stderr,"	2nd column: autocorrelation\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-sf")==0)  setsfreq=atof(argv[ii++]);
			else if(strcmp(argv[ii],"-max")==0) setmax=atof(argv[ii++]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	/* STORE DATA METHOD 1 - stream of single numbers in column or row */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while(fscanf(fpin,"%s",line)==1) {
		if(sscanf(line,"%lf",&aa)!=1) aa=NAN;
		if((xdat=(double *)realloc(xdat,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		xdat[nn++]=aa;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(setmax<=0.0) setmax=(double)nn/setsfreq;
	mm=(long int)(setmax*setsfreq);
	if(mm>nn) {fprintf(stderr,"\n--- Warning[%s]: max lag (%g seconds) is longer than the input (%g seconds) - adjusting to input length\n\n",thisprog,setmax,(nn*setsfreq));mm=nn;}
	if((xcor=calloc((mm+1),sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	for(ii=0;ii<nn;ii++) {
		for(jj=ii,kk=0;jj<nn && kk++<mm;jj++) {
			if(xdat[ii]!=NAN&&xdat[jj]!=NAN) {
				xcor[(jj-ii)]+=xdat[ii]*xdat[jj];
			}
		}
	}

	for(ii=0;ii<mm;ii++) printf("%g %g\n",(ii/setsfreq),xcor[ii]);

	free(xdat);
	free(xcor);
	exit(0);
	}
