#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-crosscor2"
#define TITLE_STRING thisprog" v.1: 7.May.2020 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>signal_processing stats</TAGS>

v.1: 7.May.2020 [JRH]

*/

/* external functions start */
long xf_interp3_d(double *data, long ndata);
double xf_correlate_simple_d(double *x, double *y, long nn, double *result_d);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *infile,*outfile,line[MAXLINELEN],templine[MAXLINELEN],word[256],*pline,*pcol;
	long int ii,jj,kk,nn=0,maxshift;
	int v,w,x,y,z,col,colmatch;
	int sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	double *xdat=NULL,*ydat=NULL,*tempdat=NULL,*xcor=NULL;
	/* arguments */
	int setverb=1;
	double setsfreq=1.0,setmax=-1.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate cross-correlation function on a time-series\n");
		fprintf(stderr," - this method uses the actual correlation of the time-series\n");
		fprintf(stderr," - non-numeric values will be ignored\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\", 2 columns per line ( x y )\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-sf: sample frequency (Hz) [%g]\n",setsfreq);
		fprintf(stderr,"	-max: max lag to calculate (seconds, -1 = auto) [%g]\n",setmax);
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -sf 1500\n",thisprog);
		fprintf(stderr,"	ccut -f 2,3 temp.txt | %s stdin -sf 24000 -max 60 \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	1st column: time-lag (seconds)\n");
		fprintf(stderr,"	2nd column: cross-correlation\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-sf")==0)  setsfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-max")==0) setmax=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb != 999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0,1, or 999\n\n",thisprog,setverb);exit(1);}


	/* STORE DATA METHOD 1 - stream of single numbers in column or row */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf %lf",&aa,&bb)!=2) continue;
		if((xdat=(double *)realloc(xdat,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((ydat=(double *)realloc(ydat,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		xdat[nn]= aa;
		ydat[nn]= bb;
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(setverb==999) for(ii=0;ii<nn;ii++) fprintf(stderr,"%g\t%g\n",xdat[ii],ydat[ii]);

	if(setmax<=0.0) setmax=(double)nn/setsfreq;
	maxshift=(long int)(setmax*setsfreq);
	if(maxshift>nn) {
		fprintf(stderr,"\n--- Warning[%s]: max lag (%g seconds) is longer than the input (%g seconds) - adjusting to input length\n\n",thisprog,setmax,(nn*setsfreq));
		maxshift=nn;
	}


	/* interpolate datasets to remove non-numerics */
	xf_interp3_d(xdat,nn);
	xf_interp3_d(ydat,nn);

	/* expand ydat to allow shifting */
	if((ydat= realloc(ydat,nn*2*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	/* make a copy of ydat at the end  */
	jj=nn; for(ii=0;ii<nn;ii++) { ydat[jj]= ydat[ii]; jj++; }

	for(kk=0;kk<maxshift;kk++) {
		aa= xf_correlate_simple_d(xdat,(ydat+kk),nn,result_d);
		printf("%g %g\n",(kk/setsfreq),aa);
	}

	free(xdat);
	free(ydat);
	free(xcor);
	exit(0);
	}
