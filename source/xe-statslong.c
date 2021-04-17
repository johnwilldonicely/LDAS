#define thisprog "xe-statslong"
#define TITLE_STRING thisprog"v 1  17.April.2021 [JRH]"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>math stats</TAGS>

v 1  17.April.2021 [JRH]
	- simple statistics on a long-integer dataset
*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
void xf_stats2_d(double *data, long n, int varcalc, double *result_d);
int xf_compare1_d(const void *a, const void *b);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *line=NULL;
	long int ii,jj,kk,mm,nn,maxlinelen=0;
	double aa,bb,cc,dd;
	FILE *fpin;

	/* program-specific variables */
	long nwords=0,*iword=NULL,colmatch;
	long *data=NULL,newcoldata,min,max,sum,mean;
	int sizeofdata=sizeof(*data);

	/* arguments */
	char *infile=NULL;
	int setverb=0;
	long setcoldata=1;

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate stats on an input containing long-integers\n");
		fprintf(stderr,"- input must be tab-delimited\n");
		fprintf(stderr,"- non-numeric data will be ignored for stats calculations\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE: %s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-cy:   column containing the data [%ld]\n",setcoldata);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt",thisprog);
		fprintf(stderr,"OUTPUT: Statistics preceded by keyword (N,SUM,MEAN,MIN,MAX etc)\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-cy")==0)    setcoldata= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)  setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setcoldata<1) {fprintf(stderr,"\n--- Error[%s]: invalid data column (-cy %ld) - must be >0\n",thisprog,setcoldata); exit(1);}
	if(setverb!=0 && setverb!=1 && setverb!=999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}

	/* DECREMENT COLUMN-NUMBERS SO THEY'RE ZERO-OFFSET */
	newcoldata= setcoldata-1;

	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(line[0]=='#') continue;
		/* parse the line & make sure all required columns are present */
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		/* make sure the crequired column is present  */
		if(nwords<setcoldata) continue;
		/* make sure the data is a number */
		if(sscanf(line+iword[newcoldata],"%ld",&kk)!=1) continue;
		/* reallocate memory */
		data= realloc(data,(nn+1)*sizeofdata);
		if(data==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		/* assign values */
		data[nn]= kk;
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(setverb==999) { for(ii=0;ii<nn;ii++) fprintf(stderr,"data[%ld]= %ld\n",ii,data[ii]);	}

	min=max= data[0];
	sum=mean= 0L;

	for(ii=0;ii<nn;ii++) {
		sum+= data[ii];
		if(data[ii]<min) min= data[ii];
		if(data[ii]>max) max= data[ii];
	}


	printf("N %ld\n",nn);
	printf("SUM %ld\n",sum);
	printf("MEAN %ld\n",(sum/nn));
	printf("MIN %ld\n",min);
	printf("MAX %ld\n",max);
	printf("RANGE %ld\n",(max-min));

	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(data!=NULL) free(data);
	exit(0);
}
