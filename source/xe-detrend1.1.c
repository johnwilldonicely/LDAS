#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <time.h>

#define thisprog "xe-detrend1"
#define TITLE_STRING thisprog" v 1: 19.March.2014 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing filter</TAGS>
v 1: 19.March.2014 [JRH]
*/

/* external functions start */
long xf_interp3_d(double *data, long ndata);
int xf_detrend1_d(double *y, size_t nn, double *result_d);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],*line=NULL,*templine=NULL,word[256],*pline,*pcol,message[MAXLINELEN];
	long int i,j,k,n,nchars=0,maxlinelen=0;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin,*fpout;
	size_t ii,jj,kk,nn,mm;
	size_t sizeofdouble=sizeof(double);

	/* program-specific variables */
	double *data=NULL;
	int foundbad=0;

	/* arguments */
	int setformat=1,setbintot=25,coldata=1;
	float setlow=0.0,sethigh=0.0,setbinwidth=0.0;

	if((line=(char *)realloc(line,MAXLINELEN))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Remove linear trend from a data series\n");
		fprintf(stderr,"Will interpolate invalid or non-finite data points\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt \n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	single column of de-trended data\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/******************************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item */
	/******************************************************************************/
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-t")==0)   setformat=atoi(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}


	/******************************************************************************/
	/* STORE DATA newline-delimited double-precision floating point numbers */
	/******************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf",&aa)==1) {
			if(!isfinite(aa)) { aa=NAN; foundbad=1;}
		}
		else { aa=NAN; foundbad=1;}
		if((data=(double *)realloc(data,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		data[nn++]=aa;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/******************************************************************************/
	/* INTERPOLATE AND DE-TREND THE DATA ******************************************/
	/******************************************************************************/
	if(foundbad==1) jj= xf_interp3_d(data,(long)nn);
	kk= xf_detrend1_d(data,nn,result_d);

	/******************************************************************************/
	/* OUTPUT THE DATA ************************************************************/
	/******************************************************************************/
	for(ii=0;ii<nn;ii++) printf("%g\n",data[ii]);

	free(data);
	exit(0);
}
