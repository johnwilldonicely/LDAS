#define thisprog "xe-smoothbox1"
#define TITLE_STRING thisprog" v 2: 14.December.2013 [JRH]"
#define MAXLINELEN 1000

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*
<TAGS>signal_processing filter</TAGS>

v 2: 16.June.2016 [JRH]
	- change definition of window size to type size_t, to be safe
	- switch to new low-memory function smoothbox2_d
	- switch to new half-window definition to allow any number >=0 (0= no smoothing)
	- hake half-window a required argument - no optional arguments necessary

v 2: 14.December.2013 [JRH]
	- improvement: use function xf_nwin1box1_d - drop-1 add-1 method for high-speed processing

v 1: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/

/* external functions start */
int xf_smoothbox2_d(double *data, size_t nn, size_t halfwin, char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char line[MAXLINELEN],message[256];
	long int i,j,k,n=0;
	size_t ii,jj,kk,nn;
	int v,w,x,y,z,sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program specific variables */
	long max;
	double *data1=NULL,*data2=NULL;
	double sum,temp_n1,temp_n2;
	/* arguments */
	char infile[256];
	long sethalfwin;

	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Apply boxcar-averaging to data\n");
		fprintf(stderr,"Sliding window averages values to either side of central data-point\n");
		fprintf(stderr,"USAGE: %s [infile] [s]\n",thisprog);
		fprintf(stderr,"	[infile]: data file or stdin\n");
		fprintf(stderr,"	[s]: samples either side of data to use for averaging [%ld]\n");
		fprintf(stderr,"		- total size of averaging window = s+s+1\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt 5\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	smoothed data - non-numeric values converted to NaN\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(1);
	}

	/* READ COMMAND-LINE ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	sethalfwin=atol(argv[2]);

	/* STORE DATA - FROM FILE OR STANDARD-INPUT IF FILENAME=STDIN */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"Error[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf",&aa)!=1) aa=NAN;
		if((data1=(double *)realloc(data1,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		data1[nn++]=aa;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* APPLY THE BOXCAR AVERAGING */
	if(sethalfwin>0) {
		z= xf_smoothbox2_d(data1,(size_t)nn,(size_t)sethalfwin,message);
		if(z!=0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	}

	/* OUTPUT THE SMOOTHED DATA */
	for(ii=0;ii<nn;ii++) { printf("%g\n",data1[ii]);}

	free(data1);
	exit(0);
}
