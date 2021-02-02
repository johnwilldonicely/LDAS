#define thisprog "xe-matrixsub1"
#define TITLE_STRING thisprog" 2.February.2021 [JRH]"
#define MAXLINELEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
<TAGS>math dt.matrix math </TAGS>

2.February.2021 [JRH]"
	- new program to adjust matrix1 based on contents of matrix2
*/


/* external functions start */
double *xf_matrixread1_d(long *nmatrices, long *ncols, long *nrows, char *message, FILE *fpin);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char outfile[256],*line=NULL,message[MAXLINELEN];
	long ii,jj,kk,mm,nn,maxlinelen=0;
	int v,w,x,y,z;
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin1,*fpin2,*fpout;
	/* program-specific variables */
	int mode;
	long width1,height1,width2,height2,nmatrices,n1,n2;
	double *dat1=NULL,*dat2=NULL;
	/* arguments */
	char *infile1=NULL,*infile2=NULL,*setmode=NULL;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<4) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"- Adjust matrix-1 using the corresponding cell in matrix-2\n");
		fprintf(stderr,"- Both matrices must have identical dimensions\n");
		fprintf(stderr,"- Both should contain only numerical values - no headers or labels\n");
		fprintf(stderr,"- Non-numbers in either matrix will result in NAN in output matrix\n");
		fprintf(stderr,"- NOTE: consecutive delimiters (tabs or spaces) are treated as one\n");
		fprintf(stderr,"USAGE: %s [matrix-1] [mode] [matrix-2]  [options]\n",thisprog);
		fprintf(stderr,"    [matrix1] : original matrix filename or \"stdin\"\n");
		fprintf(stderr,"    [mode]: add,sub,mul,div\n");
		fprintf(stderr,"    [matrix2] : matrix of modifiers - filename\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"    - none\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"    %s matrix1.txt sub matrix2.txt > newmatrix.txt\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"    A modified matrix, sent to standard output\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile1= argv[1];
	setmode= argv[2];
	infile2= argv[3];
	for(ii=4;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(strcmp(setmode,"add")==0) mode=1;
	else if(strcmp(setmode,"sub")==0) mode=2;
	else if(strcmp(setmode,"mul")==0) mode=3;
	else if(strcmp(setmode,"div")==0) mode=4;
	else {fprintf(stderr,"\n--- Error[%s]: invalid mode (%s)\n\n",thisprog,setmode);exit(1);}

	/********************************************************************************
	STORE DATA FOR MATRIX-1 : NEWLINE DELIMITED INPUT
	********************************************************************************/
	if(strcmp(infile1,"stdin")==0) fpin1=stdin;
	else if((fpin1=fopen(infile1,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile1);exit(1);}
	dat1= xf_matrixread1_d(&nmatrices,&width1,&height1,message,fpin1);
	if(strcmp(infile1,"stdin")!=0) fclose(fpin1);
	if(dat1==NULL) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
	n1= width1*height1;
	//TEST:for(ii=0;ii<n1;ii++) printf("%g ",dat1[ii]); exit(0);

	/********************************************************************************
	STORE DATA FOR MATRIX-2 : NEWLINE DELIMITED INPUT
	********************************************************************************/
	if(strcmp(infile2,"stdin")==0) fpin2=stdin;
	else if((fpin2=fopen(infile2,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile2);exit(1);}
	dat2= xf_matrixread1_d(&nmatrices,&width2,&height2,message,fpin2);
	if(strcmp(infile2,"stdin")!=0) fclose(fpin2);
	if(dat2==NULL) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
	n2= width2*height2;
	//TEST:	for(ii=0;ii<n2;ii++) printf("%g ",dat2[ii]); exit(0);

	/********************************************************************************
	MAKE SURE MATRICES HAVE SAME NUMBER OF ROWS AND COLUMNS
	********************************************************************************/
	if(height1!=height2) {fprintf(stderr,"\n--- Error[%s]: total lines in %s (%ld) don't match %s (%ld)\n\n",thisprog,infile1,height1,infile2,height2);exit(1);}
	if(width1!=width2)   {fprintf(stderr,"\n--- Error[%s]: total columns in %s (%ld) don't match %s (%ld)\n\n",thisprog,infile1,width1,infile2,width2);exit(1);}

	/********************************************************************************
	ADJUST MATRIX-1 BY MATRIX-2
	********************************************************************************/
	if(strcmp(setmode,"add")==0) for(ii=0;ii<n1;ii++) dat1[ii] += dat2[ii];
	else if(strcmp(setmode,"sub")==0) for(ii=0;ii<n1;ii++) dat1[ii] -= dat2[ii];
	else if(strcmp(setmode,"mul")==0) for(ii=0;ii<n1;ii++) dat1[ii] *= dat2[ii];
	else if(strcmp(setmode,"div")==0) for(ii=0;ii<n1;ii++) dat1[ii] /= dat2[ii];
	else {fprintf(stderr,"\n--- Error[%s]: invalid mode (%s)\n\n",thisprog,setmode);exit(1);}

	/********************************************************************************
	OUTPUT
	********************************************************************************/
	for(ii=jj=0;ii<n1;ii++) {
		printf("%g",dat1[ii]);
		if(++jj<width1) printf("\t");
		else { printf("\n"); jj=0;}
	}

	/********************************************************************************/
	/* CLEANUP AND EXIT */
	/********************************************************************************/
END:
	if(dat1!=NULL) free(dat1);
	if(dat2!=NULL) free(dat2);
	exit(0);
}
