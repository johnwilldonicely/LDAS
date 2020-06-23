#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define thisprog "xe-smoothgauss1"
#define TITLE_STRING thisprog" v 3: 28.March.2016 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>signal_processing filter</TAGS>

v 3: 28.March.2016 [JRH]
	- allow data passthrough if no smoothing is applied (smooth<3)

v 3: 30.July.2013 [JRH]
	- update - revised version of xf_smoothgauss1_d

v 2: 6.November.2012 [JRH]
	- update for revised use of xf_smoothgaussd

v 1: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/


/* external functions start */
int xf_smoothgauss1_d(double *original, size_t arraysize,int smooth);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],word[MAXLINELEN],message[256];
	long int i,j,k,n=0;
	int v,w,x,y,z,sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program specific variables */
	double *data=NULL;
	/* arguments */
	int winwidth=-1,smooth=-1;

	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"Apply Gaussian smoothing kernel to data\n");
		fprintf(stderr,"Assumes data has a fixed sample-rate\n");
		fprintf(stderr,"USAGE: \n");
		fprintf(stderr,"\t%s [infile][smooth]\n",thisprog);
		fprintf(stderr,"	[infile]: data file or stdin\n");
		fprintf(stderr,"		- non-numeric data is ignored\n");
		fprintf(stderr,"		- ignores newlines\n");
		fprintf(stderr,"	[smooth]: size of smoothing window (samples)\n");
		fprintf(stderr,"		- must be an odd number\n");
		fprintf(stderr,"		- use \"1\" for no smoothing\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(1);
	}

	/* READ COMMAND-LINE ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	winwidth=atoi(argv[2]);

	if(winwidth%2==0){fprintf(stderr,"Error[%s]: smoothing window (%d) must be an odd number\n",thisprog,winwidth);exit(1);}

	/* STORE DATA - FROM FILE OR STANDARD-INPUT IF FILENAME=STDIN */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"Error[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf",&aa)!=1) continue;
		data=(double *)realloc(data,(n+1)*sizeofdouble); /* otherwise, increase storage for data */
		if(data==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog);exit(1);}
		data[n++]=aa;	/* store the data */
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(winwidth>=3) {
		smooth=(winwidth-1)/2;	/* calculate smoothing half-window size */
		z=xf_smoothgauss1_d(data,(size_t)n,smooth); 	/* call function to appy smoothing kernel*/
		if(z!=0) {fprintf(stderr,"\n--- Error[%s]: insufficient memory for smoothing function\n\n",thisprog);exit(1);}

		for(i=0;i<n;i++) { printf("%g\n",data[i]);}	/* output the smoothed data */
	}
	else for(i=0;i<n;i++) { printf("%g\n",data[i]);}


	free(data);
	exit(0);
}
