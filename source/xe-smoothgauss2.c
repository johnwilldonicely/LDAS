#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
/* requirements for getting process-id */
#include <sys/types.h>
#include <unistd.h>

#define thisprog "xe-smoothgauss2"
#define TITLE_STRING thisprog" v 1: 23.January.2021 [JRH]"
#define MAXLINELEN 1000
#define MAXWORDLEN 256


/*
<TAGS>signal_processing filter</TAGS>

v 1: 23.January.2021 [JRH]
	- new version allows selection of a column to modify
	- also added ability to deal with long lines
	- built from version of smoothgauss1.3 from 28 March 2016
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin1);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_smoothgauss1_d(double *original, size_t arraysize,int smooth);
/* external functions end */

int main (int argc, char *argv[]) {

	/* line-reading and word/column-parsing */
	char *line=NULL,*templine=NULL,*pline=NULL,*pword=NULL;
	long *keycols=NULL,nkeys=0,*iword=NULL,nlines=0,nwords=0,maxlinelen=0,prevlinelen=0;

	/* general variables */
	char infile[MAXWORDLEN],outfile[MAXWORDLEN],message[MAXLINELEN];
	long int ii,jj,kk,nn=0;
	int v,w,x,y,z,sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin1,*fpin2,*fpout1;

	/* program specific variables */
	int setstdin=0;
	double *data=NULL;

	/* arguments */
	int winwidth=-1,smooth=-1,setp=-1;
	long setdatacol=1,sethead=0;


	// DEFINE OUTPUT FILE NAME (ONLY USED IF INPUT IS PIPED IN)
	snprintf(outfile,MAXWORDLEN,"temp_%s_%d",thisprog,getpid());
	fpout1= stdout;

	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Apply Gaussian smoothing kernel to data in a column\n");
		fprintf(stderr," - input must be tab-delimited\n");
		fprintf(stderr," - assumes data has a fixed sample-rate\n");
		fprintf(stderr," - non-numeric data and blank lines ignored\n");
		fprintf(stderr,"USAGE: \n");
		fprintf(stderr,"\t%s [in] [w] [options]\n",thisprog);
		fprintf(stderr,"	[in]: input file, or \"stdin\"\n");
		fprintf(stderr,"	[w]: smoothing window (samples)\n");
		fprintf(stderr,"		- uneven numbers only\n");
		fprintf(stderr,"		- use \"1\" to specify no smoothing\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-cy : column-number containing data [%ld]\n",setdatacol);
		fprintf(stderr,"	-head : number of header-lines to pass unaltered [%ld]\n",sethead);
		fprintf(stderr,"	-p output precision (-1=auto (%%f), 0=auto (%%g), >0=decimals) [%d]\n",setp);

		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(1);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	snprintf(infile,MAXWORDLEN,"%s",argv[1]);
	winwidth= atoi(argv[2]);
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-cy")==0) setdatacol= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-head")==0) sethead= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-p")==0) setp= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setdatacol<1) {fprintf(stderr,"\n--- Error[%s]: invalid data column (-cy %ld) - must be greater than zero\n\n",thisprog,setdatacol);exit(1);}
	if(winwidth%2==0) {fprintf(stderr,"\n--- Error[%s]: smoothing window (%d) must be an odd number\n\n",thisprog,winwidth);exit(1);}
	if(setp<-1) {fprintf(stderr,"\n--- Error[%s]: -p [%d] must be -1 or greater\n\n",thisprog,setp); exit(1);}


	// DECREMENT DATA-COLUMN SO IT IS ZERO-OFFSET (1=0, 2=1, etc)
	setdatacol--;
	// CREATE VARIABLE TO REPRESENT SPECIFICATION OF STDIN
	if(strcmp(infile,"stdin")==0) setstdin=1;


	/********************************************************************************
	 SPECIAL CASE: IF NO SMOOTHING, JUST PASS DATA THROUGH AND EXIT
	 ********************************************************************************/
	if(winwidth<=1) {
		if(strcmp(infile,"stdin")==0) fpin1=stdin;
		else if((fpin1=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
		while((line=xf_lineread1(line,&maxlinelen,fpin1))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			printf("%s",line);
		}
		if(strcmp(infile,"stdin")!=0) fclose(fpin1);
		exit(0);
	}

	/********************************************************************************
	 SET UP FILE-POINTERS TO INPUT AND OUTPUT
	********************************************************************************/
	/* if input is stdin, also open an output file to save all lines */
	if(setstdin==1) {
		fpin1= stdin;
		if((fpout1=fopen(outfile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: could not open \"%s\" for writing\n\n",thisprog,outfile);exit(1);}
	}
	else if((fpin1=fopen(infile,"r"))==0) { fprintf(stderr,"\n--- Error[%s]: input file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	/********************************************************************************
	 STORE DATA (and also save lines to temp file if input is stdin)
	********************************************************************************/
	nn=0;
	while((line=xf_lineread1(line,&maxlinelen,fpin1))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(setstdin==1) fprintf(fpout1,"%s",line);
		/* parse line */
		iword= xf_lineparse2(line,"\t",&nwords); // user-defined delimited
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		/* find datacolumn and save value  */
		for(ii=0;ii<nwords;ii++) {
			if(ii==setdatacol) {
				if(sscanf((line+iword[ii]),"%lf",&aa)!=1) aa= NAN;
		}}
		data=(double *)realloc(data,(nn+1)*sizeofdouble); /* otherwise, increase storage for data */
		if(data==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		data[nn]= aa;
		nn++;
	}
	if(setstdin==0) fclose(fpin1);
	else fclose(fpout1);
	//TEST:	for(ii=0;ii<nn;ii++) printf("stored: %f\n",data[ii]);

	/********************************************************************************
	 APPLY THE SMOOTHER
	 - note that xf_smoothgauss2_d handles NAN and INF values
	 - this means that any header may also be filled with smothed values
	********************************************************************************/
	smooth=(winwidth-1)/2;	/* calculate smoothing half-window size */
	z= xf_smoothgauss1_d(data,(size_t)nn,smooth); 	/* call function to appy smoothing kernel*/
	if(z!=0) {fprintf(stderr,"\n--- Error[%s]: insufficient memory for smoothing function\n\n",thisprog);exit(1);}
	//TEST:	for(ii=0;ii<nn;ii++) printf("smoothed: %f\n",data[ii]);

	/********************************************************************************
	OUTPUT THE DATA
	********************************************************************************/
	/* read either from the tempfile (if infile was stdin) or the actual infile */
	if(setstdin==1) { if((fpin1=fopen(outfile,"r"))==0) { fprintf(stderr,"\n--- Error[%s]: input file \"%s\" not found\n\n",thisprog,infile);exit(1);}}
	else { if((fpin1=fopen(infile,"r"))==0) { fprintf(stderr,"\n--- Error[%s]: input file \"%s\" not found\n\n",thisprog,infile);exit(1);}}
	nn= 0;
	while((line=xf_lineread1(line,&maxlinelen,fpin1))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

		/* output header-lines unaltered and continue  */
		if(nn<sethead) { printf("%s",line); nn++; continue; }

		/* parse non-header lines */
		iword= xf_lineparse2(line,"\t",&nwords); // user-defined delimited
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};

		/* ouput original field or smoothed data */
		if(setdatacol==0) {
			aa= data[nn];
			if(setp>0) printf("\t%.*f",setp,aa);
			else if(setp==0) printf("\t%g",aa);
			else printf("\t%f",aa);
		}
		else printf("%s",(line+iword[0]));
		for(ii=1;ii<nwords;ii++) {
			if(setdatacol==ii) {
				aa= data[nn];
				if(setp>0) printf("\t%.*f",setp,aa);
				else if(setp==0) printf("\t%g",aa);
				else printf("\t%f",aa);
			}
			else printf("\t%s",(line+iword[ii]));
		}
		printf("\n");
		nn++;
	}
	fclose(fpin1);




	/********************************************************************************
	 CLEANUP
	********************************************************************************/
	if(setstdin==1) remove(outfile);

	if(data!=NULL) free(data);
	if(line!=NULL) free(line);
	if(keycols!=NULL) free(keycols);
	if(iword!=NULL) free(iword);

	exit(0);
}
