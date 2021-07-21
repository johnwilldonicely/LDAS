#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-trimoutliers1"
#define TITLE_STRING thisprog" v 2: 14.August.2012 [JRH]"
#define MAXLINELEN 1000
#define MAXGROUP 8000

/*
<TAGS>filter signal_processing</TAGS>

v 2: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/


/* external functions start */
int xf_compare1_d(const void *a, const void *b);
long xf_outlier1_d(double *data, long nn, float setlow, float sethigh, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char line[MAXLINELEN],message[256];
	long int ii,jj,kk,nn=0;
	int sizeofdata;
	float a,b,c,result_f[64];
	double aa,bb,cc,result_d[64];
	FILE *fpin;
	/* program-specific variables */
	double *data=NULL;
	/* arguments */
	char *infile=NULL;
	int groupcol=-1,varcalc=2;
	float setlow=0.0,sethigh=100.0;

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO INPUT SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Trim a single-column of data to remove outliers\n");
		fprintf(stderr,"- the following values will be set to NAN:\n");
		fprintf(stderr,"    - values exceeding the percentile limits (-low and -high)\n");
		fprintf(stderr,"    - values equal to the actual limits\n");
		fprintf(stderr,"    - non-numeric values (e.g. words)\n");
		fprintf(stderr,"USAGE: %s [in] [options]\n",thisprog);
		fprintf(stderr,"    [in]: input file-name or \"stdin\", single column\n");
		fprintf(stderr,"VALID OPTIONS ( defaults in [] ):\n");
		fprintf(stderr,"    -low: low percentile cutoff (-1 to skip) [%g]\n",setlow);
		fprintf(stderr,"    -high: high percentile cutoff (-1 to skip) [%g]\n",sethigh);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"    %s data.txt -low 5 -high 95\n",thisprog);
		fprintf(stderr,"    %s data.txt -low -1 -high 99.5\n",thisprog);
		fprintf(stderr,"OUTPUT: data with outliers set to NAN\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-low")==0) setlow= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-high")==0) sethigh= atof(argv[++ii]);
			else {fprintf(stderr,"\t\aError[%s]: invalid command line argument \"%s\"\n",thisprog,argv[ii]); exit(1);}
	}}
	/* check validity of arguments */
	/* NOTE: low and high passed straight to outlier function - this will check validity */

	/* straight pass-through option */
	if(sethigh==-1.0 && setlow==-1.0) {
		if(strcmp(infile,"stdin")==0) fpin=stdin;
		else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\t\aError[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}
		while(fgets(line,MAXLINELEN,fpin)!=NULL) printf("%s",line);
		if(strcmp(infile,"stdin")!=0) fclose(fpin);
		exit(0);
	}

	/********************************************************************************
	STORE DATA - TAKE FIRST COLUMN ONLY
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\t\aError[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}

	sizeofdata= sizeof(*data);
	nn= 0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf",&aa)!=1) aa= NAN;
		data=(double *)realloc(data,(nn+1)*sizeofdata);
		if(data==NULL) {fprintf(stderr,"\t\aError[%s]: insufficient memory\n",thisprog);exit(1);}
		data[nn++]= aa;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* TRIM: SET OUTLIERS TO NAN */
	kk= xf_outlier1_d(data,nn,setlow,sethigh,message);
	if(kk<0) { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); exit(1); }

	/* OUTPUT THE TRIMMED DATASET */
	for(ii=0;ii<nn;ii++)  printf("%g\n",data[ii]);

	/* CLEANUP AND EXIT */
	if(data!=NULL) free(data);
	exit(0);

}
