#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <time.h>

#define thisprog "xe-interpspectrum1"
#define TITLE_STRING thisprog" v 1: 02.February.2016 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing filter</TAGS>

v 1: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_interp3_d(double *data, long ndata);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],*line=NULL,message[MAXLINELEN];
	long int ii,jj,kk,ll,mm,nn,nbad;
	size_t sizeofdouble=sizeof(double);
	double aa,bb,cc,dd,ee;
	FILE *fpin,*fpout;

	/* program-specific variables */
	long nindices,*pindex=NULL;
	double *xdat=NULL,*ydat=NULL,*index1=NULL,min,max;

	/* arguments */
	char *setindices=NULL;
	double setwidth=1.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Interpolate problem-frequencies in a spectrum\n");
		fprintf(stderr,"Typically used to smooth over mains-noise (50 or 60 Hz) artefacts\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\" in format <frequency> <value>\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-freq:  CSV list of frequencies to interpolate [none by default]\n");
		fprintf(stderr,"	-width: width of band around each exclusion-frequency 1 [%g]\n",setwidth);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s spectrum.txt -freq 50,100,150 -w 2\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	interpolated spectrum\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	if((line=(char *)realloc(line,MAXLINELEN))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-freq")==0)  setindices=argv[++ii];
			else if(strcmp(argv[ii],"-width")==0) setwidth=atof(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}

	/* STORE DATA - newline-delimited pairs of data */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf %lf",&aa,&bb)!=2) continue;
		if(isfinite(aa) && isfinite(bb)) {
			if((xdat=(double *)realloc(xdat,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			if((ydat=(double *)realloc(ydat,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			xdat[nn]=aa;
			ydat[nn]=bb;
			nn++;
	}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* DETERMINE MINIMUM AND MAXIMUM FREQUENCY IN SPECTRUM */
	min=max=xdat[ii];
	for(ii=0;ii<nn;ii++) {
		if(xdat[ii]<min) min=xdat[ii];
		if(xdat[ii]>max) max=xdat[ii];
	}

	/* BUILD THE LIST OF FREQUENCY-INDICES */
	nindices=0;
	if(setindices!=NULL) {
		if(strlen(setindices)>0) {
			pindex= xf_lineparse2(setindices,",",&nindices);
	}}
	/* convert from text to array of numbers  */
	if((index1=(double *)realloc(index1,(nindices*sizeof(double))))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	for(ii=0;ii<nindices;ii++) {
		/* convert index text to number */
		if(sscanf((setindices+pindex[ii]),"%lf",&aa)!=1 || !isfinite(aa))  {fprintf(stderr,"\n--- Error[%s]: non-numeric index (%s) specified\n\n",thisprog,(setindices+pindex[ii]));exit(1);};
		/* make sure this number fall within appropriate bounds */
		if(aa<min||aa>max) {fprintf(stderr,"\n--- Error[%s]: index (%g) falls outside frequency range (%g-%g)\n\n",thisprog,aa,min,max);exit(1);}
		/* create an index */
		index1[ii]= aa;
	}

	/* ASSIGN DATA FOR PROBLEM FREQUENCIES TO NAN */
	for(ii=0;ii<nindices;ii++) {
		aa=index1[ii]-0.5*setwidth;
		bb=index1[ii]+0.5*setwidth;
		for(jj=0;jj<nn;jj++) if(xdat[jj]>=aa && xdat[jj]<=bb) ydat[jj]=NAN;
	}

	/* INTERPOLATE THE NANS */
	xf_interp3_d(ydat,nn);

	/* OUTPUT THE MODIFIED SPECTRUM */
	for(ii=0;ii<nn;ii++) printf("%g\t%g\n",xdat[ii],ydat[ii]);


	if(pindex!=NULL) free(pindex);
	if(index1!=NULL) free(index1);
	if(xdat!=NULL) free(xdat);
	if(ydat!=NULL) free(ydat);
	if(line!=NULL) free(line);
	exit(0);
}
