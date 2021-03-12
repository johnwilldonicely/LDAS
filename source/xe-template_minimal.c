#define thisprog "xe-template"
#define TITLE_STRING thisprog" DAY.MONTH.2020 [JRH]"
#define MAXLINELEN 1000
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define N 20
/*
<TAGS> LDAS </TAGS>

v 1: DAY.MONTH.YEAR [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/



/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_scale1_l(long data, long min, long max);
int xf_bin3_d(double *data, short *flag, long *setn, long *setz, double setbinsize, char *message);
int xf_bin1b_f(float *data, long *setn, long *setz, double setbinsize, char *message);
int xf_bin1b_d(double *data, long *setn, long *setz, double setbinsize, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL,message[MAXLINELEN];
	int x,y,z,vector[] = {1,2,3,4,5,6,7};
	long ii,jj,kk,nn,maxlinelen=0;
	float a,b,c;
	double aa,bb,cc;
	FILE *fpin,*fpout;
	/* program-specific variables */
	int sizeofdata;
	long *iword=NULL,nwords;
	float *data1=NULL;
	double *data2=NULL;
	/* arguments */
	char *infile=NULL;
	int setverb=0;

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Template program source-code\n");
		fprintf(stderr,"USAGE: %s [in] [options]\n",thisprog);
		fprintf(stderr,"	[in]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	- \n");
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
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}

	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/

	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	sizeofdata= sizeof(*data2);
	nn=0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		if(nwords<1) continue;
		if(sscanf(line+iword[0],"%lf",&aa)!=1 || !isfinite(a)) continue;
		data2= realloc(data2,(nn+1)*sizeofdata);
		if(data2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		data2[nn]= aa;
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	//TEST

for(ii=0;ii<nn;ii++) printf("data2[%ld]= %g\n",ii,data2[ii]);

short *flag1=NULL;
flag1= realloc(flag1,(nn*sizeof(*flag1)));

for(ii=0;ii<nn;ii++) if(data2[ii]==0.0) jj=ii;
fprintf(stderr,"zero-sample=%ld\n",jj);

z= xf_bin3_d(data2,flag1,&nn,&jj,2.0,message);
if(z<0) {fprintf(stderr,"*** %s\n",message); exit(1);}

fprintf(stderr,"new-zero=%ld\n",jj);

goto END;

	/********************************************************************************/
	/* CLEANUP AND EXIT */
	/********************************************************************************/
END:
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(data1!=NULL) free(data1);
	if(data2!=NULL) free(data2);
	exit(0);
}
