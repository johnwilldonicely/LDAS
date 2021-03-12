#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
/* requirements for getting process-id */
#include <sys/types.h>
#include <unistd.h>

#define thisprog "xe-block1"
#define TITLE_STRING thisprog" v 1: 8.February.2021 [JRH]"
#define MAXLINELEN 1000
#define MAXWORDLEN 256


/*
<TAGS>signal_processing filter</TAGS>

v 1: 8.February.2021 [JRH]
	- program to process columns of data in blocks marked by changes in an index-column
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin1);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
double *xf_matrixtrans1_d(double *data1, long *width, long *height);

long xf_norm3_d(double *data,long ndata,int normtype,long start,long stop,char *message);
int xf_smoothgauss1_d(double *original, size_t arraysize,int smooth);
/* external functions end */

int main (int argc, char *argv[]) {

	/* line-reading and word/column-parsing */
	char *line=NULL,*templine=NULL,*pline=NULL,*pword=NULL;
	long *iword=NULL,nwords=0,nlines=0,maxlinelen=0;

	/* general variables */
	char outfile[MAXWORDLEN],message[MAXLINELEN];
	long ii,jj,kk,mm,nn, sizeoflong=sizeof(long), sizeofdouble=sizeof(double);
	int v,w,x,y,z;
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[16];
	FILE *fpin1,*fpin2,*fpout1;

	/* program specific variables */
	char prevblock[MAXWORDLEN],setcdatadefault[]= "2";
	int setstdin=0;
	long *blockstart=NULL,*blocksizeline=NULL,nblocks,colmatch,ncols,nrows,cdatzero=0;;
	double *data=NULL,*pdata;

	/* arguments */
	char *setinfile=NULL,*setmode=NULL,*setcdata=NULL;
	long ncdata=0,*icdata=NULL;
	int setdebug=0,winwidth=-1,smooth=-1,setp=-1,setr100=0;
	long sethead=0,setdatacol=1,setcblock=1;
	long setgwin=3;

	// DEFINE OUTPUT FILE NAME (ONLY USED IF INPUT IS PIPED IN)
	snprintf(outfile,MAXWORDLEN,"temp_%s_%d",thisprog,getpid());
	fpout1= stdout;

	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Modify columns of data in blocks defined by changes in an index-column\n");
		fprintf(stderr," - input should be tab-delimited, with no blank-lines\n");
		fprintf(stderr," - sort data so \"block\" column hold the fastest-changing variable\n");
		fprintf(stderr," - assumes data has a fixed sample-rate\n");
		fprintf(stderr," - non-numeric data ignored\n");
		fprintf(stderr,"USAGE: \n");
		fprintf(stderr,"\t%s [in] [mode] [options]\n",thisprog);
		fprintf(stderr,"    [in]: input file, or \"stdin\"\n");
		fprintf(stderr,"    [mode]: how to process each block. Choose one of:\n");
		fprintf(stderr,"        diff: difference from first sample\n");
		fprintf(stderr,"        ratio: calculate ratio to first sample\n");
		fprintf(stderr,"        gauss: apply a gausian smoother\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"    -head : number of header-lines to pass unaltered [%ld]\n",sethead);
		fprintf(stderr,"    -cblock : column (>0) defining block [%ld]\n",setcblock);
		fprintf(stderr,"    -cdata :  column-list (CSV, >0) to be modified [2]\n");
		fprintf(stderr,"    -gwin : half-size of Gaussian window (samples, 0=none) [%ld]\n",setgwin);
		fprintf(stderr,"    -r100 : convert ratio output to percent (0=NO 1=YES) [%d]\n",setr100);
		fprintf(stderr,"    -p output precision (-2=auto(%%f), -1=auto(%%g), >0=decimals) [%d]\n",setp);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(1);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	setinfile= argv[1];
	setmode= argv[2];
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error [%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-cblock")==0) setcblock= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-cdata")==0) setcdata= argv[++ii];
			else if(strcmp(argv[ii],"-head")==0) sethead= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-gwin")==0) setgwin= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-p")==0) setp= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-r100")==0) setr100= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-debug")==0) setdebug= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setcblock<1) {fprintf(stderr,"\n--- Error [%s]: invalid block-column (-cblock %ld) - must be greater than zero\n\n",thisprog,setcblock);exit(1);}
	if(setp<-1) {fprintf(stderr,"\n--- Error [%s]: -p [%d] must be -2, -1, or >=0\n\n",thisprog,setp); exit(1);}
	if(	strcmp(setmode,"ratio")!=0 &&
		strcmp(setmode,"diff")!=0 &&
		strcmp(setmode,"gauss")!=0 &&
		strcmp(setmode,"auc")!=0) {fprintf(stderr,"\n--- Error [%s]: invalid mode (%s)\n\n",thisprog,setmode); exit(1);
	}
	if(setr100!=0 && setr100!=1) {fprintf(stderr,"\n--- Error [%s]: invalid -r100 (%d) - must be 0 or 1\n\n",thisprog,setr100);exit(1);}
	/* create variable to represent specification of stdin */
	if(strcmp(setinfile,"stdin")==0) setstdin=1;

	/********************************************************************************
	CREATE LIST OF DATA COLUMNS
	********************************************************************************/
	if(setcdata==NULL) setcdata= setcdatadefault;
	icdata= xf_lineparse2(setcdata,",",&ncdata);
	if(ncdata<0) {fprintf(stderr,"\n--- Error [%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
	for(ii=0;ii<ncdata;ii++) {
		jj= atol((setcdata+(icdata[ii])));
		if(jj<1) {fprintf(stderr,"\n--- Error [%s]: invalid data-column (-cdata %ld) - all values must be greater than zero\n\n",thisprog,jj);exit(1);}
		if(jj==setcblock) {fprintf(stderr,"\n--- Error [%s]: invalid data-column, matches -cblock (%ld)\n\n",thisprog,jj);exit(1);}
		icdata[ii]= jj; // decrement to make values zero-offset
		if(jj==1) cdatzero=1; // variable to determine if the first column is a data column - simplifies output-stage
	}
	/* MAKE SURE ALL DATA COLUMNS ARE UNIQE, AND DIFFERENT FROM SETCBLOCK */
	for(ii=0;ii<ncdata;ii++) {
		for(jj=ii+1;jj<ncdata;jj++) {
			if(icdata[jj]==icdata[ii]) {fprintf(stderr,"\n--- Error [%s]: duplicate data-columns (%ld) - column list must contain unique values\n\n",thisprog,icdata[ii]);exit(1);}
	}}
	/* DECREMENT BLOCK-COLUMN SO IT IS ZERO-OFFSET (1=0, 2=1, etc) */
	setcblock--;
	for(ii=0;ii<ncdata;ii++) icdata[ii]--;

	/* debugging */
	if(setdebug==1) {  printf("setcblock= %ld\n",setcblock); for(ii=0;ii<ncdata;ii++) printf("cdata[%ld]= %ld\n",ii,icdata[ii]);}

	/********************************************************************************
	 SET UP FILE-POINTERS TO INPUT AND OUTPUT
	********************************************************************************/
	/* if input is stdin, also open an output file to save all lines */
	if(setstdin==1) {
		fpin1= stdin;
		if((fpout1=fopen(outfile,"w"))==0) {fprintf(stderr,"\n--- Error [%s]: could not open \"%s\" for writing\n\n",thisprog,outfile);exit(1);}
	}
	else if((fpin1=fopen(setinfile,"r"))==0) { fprintf(stderr,"\n--- Error [%s]: input file \"%s\" not found\n\n",thisprog,setinfile);exit(1);}

	/********************************************************************************
	 STORE DATA (and also save lines to temp file if input is stdin)
	********************************************************************************/
	nlines=mm=nn=nblocks=0; // mm= datalines, nn=datatotal
	colmatch= ncdata+1; // the number of columns which must be found

	while((line=xf_lineread1(line,&maxlinelen,fpin1))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error [%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		/* save every line, so that "extra" columns do not neeed to be stord in memory */
		if(setstdin==1) fprintf(fpout1,"%s",line);
		/* do not store "data" for header" */
		if(++nlines<=sethead) continue;
		/* parse line */
		iword= xf_lineparse2(line,"\t",&nwords); // user-defined delimited
		if(nwords<0) {fprintf(stderr,"\n--- Error [%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		/* allocate memory for block-value and data-values  */
		data= realloc(data,(nn+ncdata)*sizeofdouble);
		if(data==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog);exit(1);}
		/* find data-column and save value  */
		kk= colmatch;
		for(ii=0;ii<nwords;ii++) {
			pword= (line+iword[ii]);
			/* determine if this is the start of a new block */
			if(ii==setcblock) {
				kk--;
				/* if contents of block-column have changed, store the start of a new block and size of previous */
				if(strncmp(pword,prevblock,MAXWORDLEN)!=0) {
					blockstart= realloc(blockstart,(nblocks+1)*sizeoflong);
					if(blockstart==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog);exit(1);}
					blocksizeline= realloc(blocksizeline,(nblocks+1)*sizeoflong);
					if(blocksizeline==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog);exit(1);}
					blockstart[nblocks]= mm;
					if(nblocks>0) blocksizeline[nblocks-1]= mm-blockstart[nblocks-1];
					strncpy(prevblock,pword,MAXWORDLEN);
					nblocks++;
			}}
			for(jj=0;jj<ncdata;jj++) {
				if(ii==icdata[jj]) {
					kk--;
					if(sscanf(pword,"%lf",&aa)!=1) aa= NAN;
					data[nn++]= aa;
					continue;
			}}
		}
		if(kk!=0) {fprintf(stderr,"\n--- Error [%s]: some of the required columns not found on line %ld of %s\n\n",thisprog,nlines,setinfile);exit(1);}
		else mm++; /* data-line counter */
	}
	if(setstdin==0) fclose(fpin1);
	else fclose(fpout1);
	/* assume last block ends when file-read ends */
	if(nblocks>0) blocksizeline[nblocks-1]= mm-blockstart[nblocks-1];
	/* make variables reflecting the width and height of the data matrix */
	nrows= mm;
	ncols= ncdata;
	/* debugging */
	if(setdebug==1) {for(ii=0;ii<nblocks;ii++) printf("\tblock[%ld] start=%ld size=%ld\n",ii,blockstart[ii],blocksizeline[ii]); }
	if(setdebug==1) {printf("\nORIGINAL DATA:\n"); for(ii=jj=0;ii<nn;ii++) { printf("\t%g",data[ii]); if(++jj==ncols) {jj=0;printf("\n");}}}


	/********************************************************************************
	TRANSPOSE THE ENTIRE DATASET
	1 row for each input data column, columns are now ascending rows of length mm
	********************************************************************************/
	data= xf_matrixtrans1_d(data,&ncols,&nrows);
	/* debugging */
	if(setdebug==1) {printf("\nTRANSPOSED DATA:\n"); for(ii=jj=0;ii<nn;ii++) { printf("\t%g",data[ii]); if(++jj==ncols) {jj=0;printf("\n");}}}

	/********************************************************************************
	PROCESS EACH INPUT-COLUMN (ii) BY BLOCK (jj)
	********************************************************************************/
	for(ii=0;ii<ncdata;ii++) {
		pdata= data+(ii*mm);
		for(jj=0;jj<nblocks;jj++) {
			if(setdebug==1) {printf("\nBLOCK %ld-%ld: SIZE %ld\n",ii,jj,blocksizeline[jj]); for(kk=0;kk<blocksizeline[jj];kk++) { printf("\t%g",pdata[kk]);printf("\n");}}

			if(strcmp(setmode,"diff")==0) {
				kk= xf_norm3_d(pdata,blocksizeline[jj],3,0,1,message);
				if(kk==-2) { fprintf(stderr,"\b\n\t--- Error [%s]/%s\n\n",thisprog,message); exit(1); }
			}
			else if(strcmp(setmode,"ratio")==0) {
				kk= xf_norm3_d(pdata,blocksizeline[jj],4,0,1,message);
				if(kk==-2) { fprintf(stderr,"\b\n\t--- Error [%s]/%s\n\n",thisprog,message); exit(1); }
				if(setr100==1) { for(kk=0;kk<blocksizeline[jj];kk++) pdata[kk]*=100.0; }
			}
			else if(strcmp(setmode,"gauss")==0 && setgwin>0) {
				z= xf_smoothgauss1_d(pdata,blocksizeline[jj],setgwin);
				if(z<0-2) { fprintf(stderr,"\b\n\t--- Error [%s]/xf_smoothgauss1_d: problem in smoothing function \n\n",thisprog); exit(1); }
			}

			/* increment pointer to start of next block */
			pdata+= blocksizeline[jj];
	}}


	/********************************************************************************
	DE-TRANSPOSE THE ENTIRE DATASET
	********************************************************************************/
	data= xf_matrixtrans1_d(data,&ncols,&nrows);
	/* debugging */
	if(setdebug==1) {printf("\nRESTORED DATA:\n"); for(ii=jj=0;ii<nn;ii++) { printf("\t%g",data[ii]); if(++jj==ncols) {jj=0;printf("\n");}}}


	/********************************************************************************
	OUTPUT THE DATA
	********************************************************************************/
	/* read either from the tempfile (if setinfile was stdin) or the actual setinfile */
	if(setstdin==1) { if((fpin1=fopen(outfile,"r"))==0) { fprintf(stderr,"\n--- Error [%s]: input file \"%s\" not found\n\n",thisprog,setinfile);exit(1);}}
	else { if((fpin1=fopen(setinfile,"r"))==0) { fprintf(stderr,"\n--- Error [%s]: input file \"%s\" not found\n\n",thisprog,setinfile);exit(1);}}
	nlines=mm=0; // mm= datalines
	while((line=xf_lineread1(line,&maxlinelen,fpin1))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error [%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		/* output header-lines unaltered and continue  */
		if(++nlines<=sethead) { printf("%s",line); continue; }
		/* parse non-header lines */
		iword= xf_lineparse2(line,"\t",&nwords); // user-defined delimited
		if(nwords<0) {fprintf(stderr,"\n--- Error [%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};

		/* set pointer to data corresponding to this line */
		pdata= (data+(mm*ncdata));
		mm++;
		/* deal with first column - no preceeding tab */
		if(cdatzero==0) printf("%s",(line+iword[0]));
		else {
			aa= pdata[0];
			if(setp==-2)      printf("\t%f",aa);
			else if(setp==-1) printf("\t%g",aa);
			else              printf("\t%.*f",setp,aa);
		}
		/* deal with other columns */
		for(ii=1;ii<nwords;ii++) {
			pword= (line+iword[ii]);
			for(jj=0;jj<ncdata;jj++) if(ii==icdata[jj]) break;
			if(jj==ncdata) printf("\t%s",(line+iword[ii]));
			else {
				aa= pdata[jj];
				if(setp==-2)      printf("\t%f",aa);
				else if(setp==-1) printf("\t%g",aa);
				else              printf("\t%.*f",setp,aa);
			}
		}
		printf("\n");


	}
	fclose(fpin1);




	/********************************************************************************
	 CLEANUP AN DEXIT
	********************************************************************************/
END:
	if(setstdin==1) remove(outfile);

	if(blockstart!=NULL) free(blockstart);
	if(blocksizeline!=NULL) free(blocksizeline);
	if(data!=NULL) free(data);
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(icdata!=NULL) free(icdata);

	exit(0);
}
