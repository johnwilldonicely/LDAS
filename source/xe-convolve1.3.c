#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-conv1"
#define TITLE_STRING thisprog" v 3: 6.October.2014 [JRH]"

/*
<TAGS>math signal_processing</TAGS>

v 3: 6.October.2014 [JRH]
	- put proper instructions in place

v 2: 1.April.2014 [JRH]
	- don't pre-allocate memory for line - unnecessary (let xf_lineread function handle it)
	- adopt versioned source-naming convention

v 1: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/


/* external functions start */
float *xf_conv1_f(float *sig1, size_t n1, float *sig2, size_t n2,char message[] );
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile1[1000],infile2[1000],*line=NULL,*templine=NULL,word[256],*pline,*pcol,message[1000];
	long int i,j,k,n,m,nchars=0,maxlinelen=0;
	size_t ii,jj,kk,ll,n2,n1;
	size_t sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d;
	FILE *fpin,*fpout;
	/* program-specific variables */
	float *sig1=NULL,*sig2=NULL,*result=NULL;
	/* arguments */
	int setformat=0; //not used at present


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Convolve input1 with input2\n");
		fprintf(stderr,"NOTE: no correction for non-numeric or non-finite numbers (NAN,INF)\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input1] [input2]\n",thisprog);
		fprintf(stderr,"	[input1]: file name of first input, or \"stdin\"\n");
		fprintf(stderr,"	[input2]: file name of second input, or \"stdin\"\n");
		fprintf(stderr,"	NOTE: if one input is \"stdin\", the other cannot be!\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	none\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data1.txt wavelet.txt\n",thisprog);
		fprintf(stderr,"result: single column of convolved results\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile1,"%s",argv[1]);
	sprintf(infile2,"%s",argv[2]);
	for(i=3;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"t")==0) 	{ setformat=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	/* STORE SIGNAL1 - stream of single numbers in column */
	if(strcmp(infile1,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile1,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile1);exit(1);}
	n1=0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(sscanf(line,"%f",&a)!=1) a=NAN;
		if((sig1=(float *)realloc(sig1,(n1+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if(isfinite(a)) sig1[n1++]=a;
	}
	if(strcmp(infile1,"stdin")!=0) fclose(fpin);
	fprintf(stderr,"\tn1: %ld\n",n1);

	/* STORE SIGNAL2 - stream of single numbers in column */
	if(strcmp(infile2,"stdin")==0) {fprintf(stderr,"\n--- Error[%s]: input #2 cannot be \"stdin\"\n\n",thisprog);exit(1);}
	else if((fpin=fopen(infile2,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile2);exit(1);}
	n2=0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(sscanf(line,"%f",&a)!=1) a=NAN;
		if((sig2=(float *)realloc(sig2,(n2+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if(isfinite(a)) sig2[n2++]=a;
	}
	fclose(fpin);
	fprintf(stderr,"\tn2: %ld\n",n2);

	result= xf_conv1_f(sig1,n1,sig2,n2,message);

	if(result!=NULL) {
		for(ii=0;ii<(n1+(n2-1));ii++) printf("%f\n",result[ii]);
	}
	else {
		fprintf(stderr,"\a\t--- Error: %s (%s)\n",thisprog,message);
	}

	if(line!=NULL) free(line);
	if(sig1!=NULL) free(sig1);
	if(sig2!=NULL) free(sig2);
	if(result!=NULL) free(result);

	exit(0);
	}
