#define thisprog "xe-makepairs1"
#define TITLE_STRING thisprog" v 1: 16.April.2019 [JRH]"
#define MAXLINELEN 1000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS> string </TAGS>

v 1: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
char *xf_strcat1(char *string1,char *string2,char *delimiter);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char outfile[256],*line=NULL,*templine=NULL,word[256],*pline,*pcol,message[MAXLINELEN];
	long int ii,jj,kk,ll,mm,nn,nbad,nchars=0,maxlinelen=0,prevlinelen=0;
	int x,y,z;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char *words=NULL,*allwords=NULL;
	long nwords=0,*iword=NULL;
	/* arguments */
	char *infile=NULL,*setdel=NULL,*setsep=NULL;
	int setverb=0;

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Make a list of all possible item-pairs\n");
		fprintf(stderr,"USAGE: %s [in] [options]\n",thisprog);
		fprintf(stderr,"	[in]: input file name or \"stdin\" in CSV format\n");
		fprintf(stderr,"		- NOTE: newlines will be treated as commas\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-NONE-\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	echo \"a,b,c,d\" | %s stdin \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	CSV pairs of items, newline separated\n");
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
			else if(strcmp(argv[ii],"-del")==0) setdel= argv[++ii];
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb != 999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0,1, or 999\n\n",thisprog,setverb);exit(1);}


	/********************************************************************************
	STORE DATA AS ONE LONG STRING - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		/* parse the line using a comma delimiter */
		iword= xf_lineparse2(line,",",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);}
		/* combine with previous lines - again using a comma delimiter */
		for(ii=0;ii<nwords;ii++) { allwords= xf_strcat1(allwords,(line+iword[ii]),","); }
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* PARSE THE TOTAL INPUT */
	iword= xf_lineparse2(allwords,",\t",&nwords);
	if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);}

	for(ii=0;ii<nwords;ii++) {
		for(jj=ii+1;jj<nwords;jj++) {
			printf("%s,%s\n",(allwords+iword[ii]),(allwords+iword[jj]));
	}}


/********************************************************************************/
/* CLEANUP AND EXIT */
/********************************************************************************/
END:
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(allwords!=NULL) free(allwords);
	exit(0);
}
