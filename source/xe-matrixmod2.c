#define thisprog "xe-matrixmod2"
#define TITLE_STRING thisprog" v 1: 21.September.2020 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS> matrix string</TAGS>

v 1: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
/* external functions end */


int main (int argc, char *argv[]) {

	/* line-reading and word/column-parsing */
	char *line=NULL,*templine=NULL,*pline=NULL,*pword=NULL;
	long *keycols=NULL,nkeys=0,*iword=NULL,nlines=0,nwords=0,maxlinelen=0,templen=0;

	char *matrix1=NULL; // list of all words in the matrix

	/*  common-use variables */
	char message[MAXLINELEN];
	long int ii,jj,kk,ll,mm,nn;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar= sizeof(char), sizeoflong= sizeof(long);
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin,*fpout;

	/* program-specific variables */
	long matrixwidth,*imatrix=NULL,nmatrix=0;

	int *count,grp,bin,bintot,setrange=0,colx=1,coly=2;
	off_t sizeofdata;
	float *data1=NULL;

	/* arguments */
	char *infile=NULL,*outfile=NULL,*setkeys=NULL;
	int setformat=1,setbintot=25,coldata=1,setverb=0,sethead=0;
	long setcolx=1,setcoly=2;
	float setlow=0.0,sethigh=0.0,setbinwidth=0.0;


	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"IN PROGRESS: Rotate or transpose a matrix of numeric or non-numeric items\n");
		fprintf(stderr,"USAGE: %s [infile] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-t(ype): 1(counts) 2(range 0-1) or 3(probability)\n");
		fprintf(stderr,"	-list: comma-separated list of numbers\n");
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -t 1\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 3\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	1st column: lower limit of each bin\n");
		fprintf(stderr,"	2nd column: value (eg. counts) in that bin\n");
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
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb != 999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0,1, or 999\n\n",thisprog,setverb);exit(1);}


	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	sizeofdata= sizeof(*data1);
	nlines=mm=nn=0; // nlines= total lines read (for reporting), mm= nonblank/noncomment lines, nn= data stored

	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		nlines++; // increment line-counter, for reporting
		if(sethead==1) { if(line[0]=='#'||strlen(line)<=1) { printf("%s",line); continue;}} // preserve leading comments and blank lines if required
		mm++; // increment non-comment/blank line counter, to detect first data-line

		/* parse non-header lines */
		iword= xf_lineparse2(line,"\t",&nwords); // user-defined delimiter
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		//TEST:	for(ii=0;ii<nwords;ii++) {	pword= line+iword[ii]; printf("%s\n",pword); }

		/* ASSIGN KEYCOL NUMBERS: THIS IS FOR KEYWORD-SELECTION OF COLUMNS */
		if(mm==1) {
			matrixwidth= nwords;
			if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: first data row (line %ld) is empty: consider using the -skip option to exclude leading blank lines and comments\n\n",thisprog,nlines);exit(1);};
		}

		/* make sure required columns are present */
		if(nwords!=matrixwidth) {fprintf(stderr,"\n--- Error[%s]: line %ld has %ld fields but matrix width should be %ld, based on first data row. Check that the matrix is symmetrical, that the correct delimiter is specified, and that there are no leading blank lines or comments that should have been excluded with the -skip option\n\n",thisprog,nlines,nwords,matrixwidth);exit(1);};

		for(ii=0;ii<nwords;ii++) {
			pword= line+iword[ii];
			kk= strlen(pword); //printf("%s: len=%ld\n",pword,kk);
			matrix1= realloc(matrix1,(templen+kk+4)*sizeofchar);
			if(matrix1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

			imatrix= realloc(imatrix,((nmatrix+1)*sizeoflong));
			if(imatrix==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			imatrix[nmatrix]=templen; /* set pointer to start position (currently, the end of the labels string) */
			sprintf((matrix1+templen),"%s",(line+iword[ii])); /* add new word to end of matrix1, adding terminal NULL */
			templen+= (kk+1); /* update length, allowing for terminal NULL - serves as pointer to start of next word */
			nmatrix++; /* increment nmatrix with check */
		}
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	//TEST

	for(ii=jj=0;ii<nmatrix;ii++) {
		printf("%s",matrix1+imatrix[ii]);
		if(++jj<matrixwidth) printf("\t");
		else { printf("\n"); jj=0;}
	}



	goto END;



	/********************************************************************************/
	/* CLEANUP AND EXIT */
	/********************************************************************************/
END:
	if(data1!=NULL) free(data1);
	if(line!=NULL) free(line);
	if(templine!=NULL) free(templine);
	if(iword!=NULL) free(iword);
	exit(0);

}
