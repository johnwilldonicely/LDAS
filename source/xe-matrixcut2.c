#define thisprog "xe-matrixcut2"
#define TITLE_STRING thisprog" 7.February.2021 [JRH]"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>dt.matrix</TAGS>

7.February.2021 [JRH]
	- allow setting of decimal precision for output

24.February.2019 [JRH]
	- new matrix read program to read matrices one at a time with selection criteria
	- accepts numeric or alphanumeric matches
*/


/* external functions start */
double *xf_matrixread3_d(FILE *fpin, long *ncols, long *nrows, char *header, char *message);
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL,*templine=NULL,*pline,*pcol,message[256];
	long ii,jj,kk,nn;
	double aa;
	FILE *fpin;
	/* program-specific variables */
	char header[256],tempheader[256];
	long *start=NULL,nwords,width,height,idlong=0;
	double *matrix1=NULL,iddouble=0.0;
	/* arguments */
	char *infile,*setid=NULL;
	int sethead=1,setmatch=2,setp=-1;
	long setcol=1;

	/********************************************************************************/
	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	/********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract matrices (numbers only) from a multi-matrix file\n");
		fprintf(stderr,"- looks for a match with an ID on comment-lines separating matrices\n");
		fprintf(stderr,"- NOTE: max 256 characters are scanned from comment-lines\n");
		fprintf(stderr,"USAGE: %s [matrix] [options]\n",thisprog);
		fprintf(stderr,"	[matrix]: file or \"stdin\" in (multi)matrix format\n");
		fprintf(stderr,"		- matrices separated by \"# comment\" lines\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"    -col   : column on comment-lines holding the ID [%ld]\n",setcol);
		fprintf(stderr,"    -id    : ID to match (if unset, matches all matrices) []\n");
		fprintf(stderr,"    -match : mode (1=partial 2=exact, 3=integer 4=float) [%d]\n",setmatch);
		fprintf(stderr,"    -head  : output header line (0=NO 1=YES) [%d]\n",sethead);
		fprintf(stderr,"    -p output decimal precision [%d]\n",setp);
		fprintf(stderr,"        -2 : %%f \n");
		fprintf(stderr,"	-1 : %%g (minimum required decimals, may truncate to 4 places)\n");
		fprintf(stderr,"      >= 0 : value represents decimal precision\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s matrix.txt -col 4 -id dose3 -match 2\n",thisprog);
		fprintf(stderr,"	%s matrix.txt -col 7 -id 0.50 -match 4\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-id")==0)    setid= argv[++ii];
			else if(strcmp(argv[ii],"-col")==0)   setcol= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-match")==0) setmatch=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-head")==0)  sethead= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-p")==0)     setp=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setmatch<1 || setmatch>4) { fprintf(stderr,"\n--- Error [%s]: invalid -match [%d] must be 1-4\n\n",thisprog,setmatch);exit(1);}
	if(sethead!=0 && sethead!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -head [%d] must be 0 or 1\n\n",thisprog,sethead);exit(1);}
	if(setcol<=0) { fprintf(stderr,"\n--- Error [%s]: invalid -col [%d] must be >0\n\n",thisprog,sethead);exit(1);}

	/* adjust setcol to reflect the zero-offset word-pointers returned from xf_lineparse1 */
	setcol--;
	/* create the long and double forms of the setid */
	if(setid!=NULL) { idlong= atol(setid); iddouble= atof(setid); }
	/* initialize the header and messgae to a simple newline */
	header[0]='\n'; header[1]='\0';
	message[0]='\n'; message[1]='\0';
	/* open the matrix input */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	/* READ THE MATRIX */
	while(1) {
		/* grab a matrix, stopping at the subsequent blank or header */
		matrix1= xf_matrixread3_d(fpin,&width,&height,header,message);
		if(height<0) {fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message);exit(1);}
		if(matrix1==NULL) break;
		/* copy the header to preserve the original form, before parsing */
		strncpy(tempheader,header,256);
		/* parse the header into */
		start= xf_lineparse1(tempheader,&nwords);
		/* can we reject this matrix based on the header? */
		if(setid!=NULL) {
			/* reject if header doesn't contain the id-column */
			if(nwords<=setcol) continue;
			else pcol= tempheader+start[setcol];
			/* reject if the id-column doesn't partial match the set id */
			if(setmatch==1 && strstr(pcol,setid)==NULL) continue;
			/* reject if the id-column doesn't exact match the set id */
			if(setmatch==2 && strcmp(pcol,setid)!=0) continue;
			/* reject if the id-column isnt a numerical match (long) */
			if(setmatch==3 && atol(pcol)!=idlong) continue;
			/* reject if the id-column isnt a numerical match (long) */
			if(setmatch==4 && atof(pcol)!=iddouble) continue;
		}
		/* otherwise, output the current header and matrix */
		if(sethead==1) printf("%s",header);
		for(ii=0;ii<height;ii++) {
			kk= ii*width;
			for(jj=0;jj<width;jj++) {
				if(jj>0) printf(" ");
				aa= matrix1[kk+jj];
				if(setp>0) printf("%.*f\t",setp,aa);
				else if(setp==0) printf("%g\t",aa);
				else printf("%f\t",aa);
			}
			printf("\n");
		}
	}


	END:
	/* CLOSE FILES, FREE MEMORY AND EXIT */
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(matrix1!=NULL) free(matrix1);
	if(start!=NULL) free(start);
	exit(0);

	}
