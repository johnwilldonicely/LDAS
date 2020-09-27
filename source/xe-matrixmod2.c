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

v 1: 21.September.2020 [JRH]:
	- new program and functions to deal with non-numerical matrices

*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_matrixflip2_l(long *data1, long *width, long *height, int setflip);
int xf_matrixrotate2_l(long *data1, long *width, long *height, int r);
int xf_matrixtrans2_l(long *data1, long *width, long *height);
/* external functions end */


int main (int argc, char *argv[]) {

	/*  common-use variables */
	char message[MAXLINELEN];
	long int ii,jj,kk,ll,mm,nn;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar= sizeof(char),sizeoflong= sizeof(long);
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin,*fpout;

	/* line-reading and word/column-parsing */
	char *line=NULL,*pword=NULL,*setkeys=NULL;
	long *keycols=NULL,nkeys=0,*iword=NULL,nlines=0,nwords=0,maxlinelen=0,templen=0;

	/* program-specific variables */
	char *matrix1=NULL; // list of all words in the matrix
	long matrixwidth,matrixheight,*imatrix=NULL,nmatrix=0;

	/* arguments */
	char *infile=NULL;
	int setverb=0,sethead=0;
	int setrotate=0,settrans=0,setflip=0;

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"- Flip, rotate or transpose a matrix: numeric or non-numeric\n");
		fprintf(stderr,"	- note that modifications are performed in the above order\n");
		fprintf(stderr,"	- note that flipping in x and y dimensions= 180-deg rotation\n");
		fprintf(stderr,"USAGE: %s [infile] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"	-f : flip matrix (0=NO, 1= x-flip, 2= y-flip) [%d]\n",setflip);
		fprintf(stderr,"	-r : rotation degrees (0,90,180,270 - negative allowed) [%d]\n",setrotate);
		fprintf(stderr,"	-t : transpose (0=NO, 1=YES) [%d]\n",settrans);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -f 2 -r -90\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 1\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	the modified matrix\n");
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
			else if(strcmp(argv[ii],"-f")==0)    setflip=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-r")==0)    setrotate=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-t")==0)    settrans=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb != 999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0,1, or 999\n\n",thisprog,setverb);exit(1);}
	x= abs(setrotate);
	if(x!=0&&x!=90&&x!=180&&x!=270) {fprintf(stderr,"\n--- Error[%s]: invalid rotation setting (-r %d) \n\n",thisprog,setrotate);exit(1);}
	if(settrans!=0&&settrans!=1) {fprintf(stderr,"\n--- Error[%s]: invalid transpose setting (-t %d) \n\n",thisprog,settrans);exit(1);}
	if(setflip<0 || setflip>2) {fprintf(stderr,"\n--- Error[%s]: invalid -flip (%d) must be 0-2\n\n",thisprog,setflip);exit(1);}

	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	nlines= 0;       // total lines in the input, for reporting
	matrixheight= 0; // the number of words on each nrows
	matrixwidth= 0;  // the total number of rows
	nmatrix= 0;      // total number of words in matrix (width x height)

	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		nlines++; // increment line-counter, for reporting
		if(sethead==1) { if(line[0]=='#'||strlen(line)<=1) { printf("%s",line); continue;}} // preserve leading comments and blank lines if required
		/* parse line */
		iword= xf_lineparse2(line,"\t",&nwords); // user-defined delimiter
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		//TEST:	for(ii=0;ii<nwords;ii++) {	pword= line+iword[ii]; printf("%s\n",pword); }
		/* for the first actual data-line, record the matrix width (number of columns) */
		if(matrixheight++ == 0) {
			matrixwidth= nwords;
			if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: first data row (line %ld) is empty: consider using the -skip option to exclude leading blank lines and comments\n\n",thisprog,nlines);exit(1);};
		}
		/* make sure the number of columns is constant  */
		if(nwords!=matrixwidth) {fprintf(stderr,"\n--- Error[%s]: line %ld has %ld fields but matrix width should be %ld, based on first data row. Check that the matrix is symmetrical, that the correct delimiter is specified, and that there are no leading blank lines or comments that should have been excluded with the -skip option\n\n",thisprog,nlines,nwords,matrixwidth);exit(1);};
		/* ADD EACH WORD TO THE MATRIX */
		for(ii=0;ii<nwords;ii++) {
			pword= line+iword[ii];
			kk= strlen(pword); //printf("%s: len=%ld\n",pword,kk);
			/* update the pointer to the latest word */
			imatrix= realloc(imatrix,((nmatrix+1)*sizeoflong));
			if(imatrix==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			imatrix[nmatrix]=templen; /* set pointer to current start position */
			/* add the latest word to the matrix */
			matrix1= realloc(matrix1,(templen+kk+4)*sizeofchar);
			if(matrix1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			sprintf((matrix1+templen),"%s",(line+iword[ii])); /* add new word to end of matrix1, adding terminal NULL */
			/* update templen (total characters) and nmatrix (number of matrix elements) */
			templen+= (kk+1); /* update length, allowing for terminal NULL - serves as pointer to start of next word */
			nmatrix++; /* increment nmatrix with check */
		}
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(setverb!=0) {
		fprintf(stderr,"\ninput_matrix_width= %ld\n",matrixwidth);
		fprintf(stderr,"\ninput_matrix_height= %ld\n",matrixheight);
		fprintf(stderr,"\ntotal_words= %ld\n",nmatrix);
	}


	/********************************************************************************
	APPLY THE TRANSFORMATIONS IN ORDER: FLIP, ROTATE, TRANSPOSE
	********************************************************************************/
	/* FLIP THE MATRIX-INDICES */
	if(setflip!=0) {
		z= xf_matrixflip2_l(imatrix,&matrixwidth,&matrixheight,setflip);
		if(z==-1) {fprintf(stderr,"\n--- Error[%s]: invalid size of input array for flipping\n\n",thisprog);exit(1);}
		if(z==-2) {fprintf(stderr,"\n--- Error[%s]: invalid flipping-type (-f %d) - must be 1 (x) or 2 (y) \n\n",thisprog,setflip);exit(1);}
		if(z==-3) {fprintf(stderr,"\n--- Error[%s]: insufficient memory for matrix rotation\n\n",thisprog);exit(1);}
	}
	/* ROTATE THE MATRIX-INDICES */
	if(setrotate!=0) {
		z= xf_matrixrotate2_l(imatrix,&matrixwidth,&matrixheight,setrotate);
		if(z==-1) {fprintf(stderr,"\n--- Error[%s]: invalid size of input array for rotation\n\n",thisprog);exit(1);}
		if(z==-2) {fprintf(stderr,"\n--- Error[%s]: invalid rotation (-r %d)\n\n",thisprog,setrotate);exit(1);}
		if(z==-3) {fprintf(stderr,"\n--- Error[%s]: insufficient memory for matrix rotation\n\n",thisprog);exit(1);}
	}
	/* TRANSPOSE THE MATRIX-INDICES */
	if(settrans==1) {
		z= xf_matrixtrans2_l(imatrix,&matrixwidth,&matrixheight);
		if(z==-1) {fprintf(stderr,"\n--- Error[%s]: invalid size of input array for transposing\n\n",thisprog);exit(1);}
		if(z==-2) {fprintf(stderr,"\n--- Error[%s]: insufficient memory for matrix transposing\n\n",thisprog);exit(1);}
	}


	/* OUTPUT MODIFIED MATRIX */
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
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(imatrix!=NULL) free(imatrix);
	exit(0);

}
