#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-matrixsub1"
#define TITLE_STRING thisprog" v 6: 28.November.2012 [JRH]"

/*
<TAGS>math matrix</TAGS>

v 7: 16.December.2012 [JRH]
	- now uses function _readline1 to read lines of unknown length

v 6: 28.November.2012 [JRH]
	- relax column-check function - now will correct for any number of missing/extra columns, provided number of lines is correct

v 5: 23.November.2012 [JRH]
	- replace fixed line-length read with new read method for matrices - can deal with lines of any length


v 4: 22.November.2012 [JRH]
	- now checks number of columns on each row - if different by less than 1 from the reference matrix (the first) then it adjusts accordingly:
		- one extra column on any given row will be dropped
		- one less column on any given row will result in generation of a dummy column (NAN)
		- if the column-number difference on any row is >1 or <-1, an error results
	- if any matrix has fewer or more LINES than the reference, an error is generated

v 3: 10.November.2012 [JRH]
	- test new method for organnizing input/ouput - could serve as template for matrixavg program

v 2: 10.November.2012 [JRH]
	- correct instructions

v 1: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
int xf_stats2_d(double *data, long n, int varcalc, double *result_d);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile1[256],infile2[256],outfile[256],*line=NULL,word[256],*matchstring=NULL,*pline,*pcol;
	long int i,j,k,m,n,row,col,maxlinelen=0;
	int v,w,x,y,z,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin1,*fpin2,*fpout;
	/* program-specific variables */
	int grp,bin,bintot,setrange=0,colx=1,coly=2;
	long index,n1=0,n2=0,nrows1=0,nrows2=0,*ncols1=NULL,*ncols2=NULL;
	double *dat1=NULL,*dat2=NULL;
	/* arguments */
	int setformat=1,setbintot=25,coldata=1;
	float setlow=0.0,sethigh=0.0,setbinwidth=0.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Subtract the values in one matrix from those in another\n");
		fprintf(stderr,"Rows in a given matrix may have different number of columns\n");
		fprintf(stderr,"But both matrices must be similar in format:\n");
		fprintf(stderr,"	- the first matrix is taken as the reference for formatting\n");
		fprintf(stderr,"	- any difference in the number of lines in a matrix results in an error\n");
		fprintf(stderr,"	- if a row has more data than it should, the extras are dropped\n");
		fprintf(stderr,"	- if a row has fewer data than it should, NANs are inserted\n");
		fprintf(stderr,"Non-numbers in either matrix will result in NAN in output matrix\n");
		fprintf(stderr,"USAGE:	%s [in1] [in2]\n",thisprog);
		fprintf(stderr,"		[in1] matrix of interest\n");
		fprintf(stderr,"		[in2] reference matrix from which [in1] is subtracted\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s matrix1.txt matrix2.txt\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	A difference matrix matching the original matrix format\n");
		fprintf(stderr,"	[in2] minus [in1]\n");
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
			else if(strcmp(argv[i],"-t")==0) 	{ setformat=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}


	/* STORE DATA FOR MATRIX-1 : NEWLINE DELIMITED INPUT */
	if(strcmp(infile1,"stdin")==0) fpin1=stdin;
	else if((fpin1=fopen(infile1,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile1);exit(1);}
	nrows1=n1=0;

	while((line=xf_lineread1(line,&maxlinelen,fpin1))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(line[0]=='#') continue;
		pline=line;
		for(col=1;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
			pline=NULL; aa=NAN;
			dat1=(double *)realloc(dat1,(n1+1)*sizeofdouble); if(dat1==NULL) {fprintf(stderr,"\t\aError[%s]: insufficient memory\n",thisprog);exit(1);}
			sscanf(pcol,"%lf  ",&aa);
			if(isfinite(aa)) dat1[n1]=aa;
			else dat1[n1]=NAN;
			n1++;
		}
		ncols1=(long *)realloc(ncols1,(nrows1+1)*sizeoflong); if(ncols1==NULL) {fprintf(stderr,"\t\aError[%s]: insufficient memory\n",thisprog);exit(1);}
		ncols1[nrows1]=(col-1);
		nrows1++;

	}
	if(strcmp(infile1,"stdin")!=0) fclose(fpin1);


	/* STORE DATA FOR MATRIX-2 : NEWLINE DELIMITED INPUT */
	if(strcmp(infile2,"stdin")==0) fpin2=stdin;
	else if((fpin2=fopen(infile2,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile2);exit(1);}
	nrows2=n2=0;

	while((line=xf_lineread1(line,&maxlinelen,fpin2))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(line[0]=='#') continue;
		pline=line;
		for(col=1;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
			pline=NULL; aa=NAN;
			dat2=(double *)realloc(dat2,(n2+1)*sizeofdouble); if(dat2==NULL) {fprintf(stderr,"\t\aError[%s]: insufficient memory\n",thisprog);exit(1);}
			sscanf(pcol,"%lf  ",&aa);
			if(isfinite(aa)) dat2[n2]=aa; else dat2[n2]=NAN;
			n2++;
		}
		ncols2=(long *)realloc(ncols2,(nrows2+1)*sizeoflong); if(ncols2==NULL) {fprintf(stderr,"\t\aError[%s]: insufficient memory\n",thisprog);exit(1);}
		ncols2[nrows2]=(col-1);

		/* check if column-cout matches template - tolerate minor differences ( +- 1 column ) */
		k=ncols2[nrows2]-ncols1[nrows2];
		if(k<0) {
			fprintf(stderr,"\tWarning [%s]: fewer columns column in matrix2 row %ld (%ld) compared to matrix1 (%ld) - adding a NULL value\n",thisprog,(nrows2+1),ncols2[nrows2],ncols1[nrows2]);
			for(i=k;i<0;i++) {
				dat2=(double *)realloc(dat2,(n2+1)*sizeofdouble); if(dat2==NULL) {fprintf(stderr,"\t\aError[%s]: insufficient memory\n",thisprog);exit(1);}
				dat2[n2]=NAN;
				n2++;
			}
		}
		if(k>0) {
			fprintf(stderr,"\tWarning [%s]: extra columns in matrix2 row %ld (%ld) compared to matrix1 (%ld) - dropping extra values\n",thisprog,(nrows2+1),ncols2[nrows2],ncols1[nrows2]);
			for(i=k;i>0;i--) n2--;
		}

		/* add up total number of rows */
		nrows2++;
	}
	if(strcmp(infile2,"stdin")!=0) fclose(fpin2);

	/* MAKE SURE MATRICES HAVE SAME NUMBER OF LINES */
	if(nrows1!=nrows2) {fprintf(stderr,"\t\aError[%s]: total lines in %s (%ld) don't match %s (%ld)\n",thisprog,infile1,nrows1,infile2,nrows2);exit(1);}
	/* JUST MAKE SURE NO ROGUE DATA SLIPPED THROUGH AFTER MINOR CORECTIONS MADE TO EACH LINE */
	if(n1!=n2) {fprintf(stderr,"\t\aError[%s]: total elements in %s (%ld) don't match %s (%ld)\n",thisprog,infile1,n1,infile2,n2);exit(1);}

	row=col=0; // row=line, col=word;
	for(i=0;i<n1;i++) {
		aa=dat2[i]-dat1[i];
		printf("%g",aa);
		if(++col<ncols1[row]) printf("\t");
		else {
			printf("\n");
			col=0;
			row++;
		}
	}

	free(dat1);
	free(dat2);
	free(ncols1);
	free(ncols2);
	exit(0);
	}
