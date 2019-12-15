#define thisprog "xe-strgroup1"
#define TITLE_STRING thisprog" v 1: 10.March.2019 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>string</TAGS>

v 1: 10.March.2019 [JRH]
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL,*pline,message[MAXLINELEN];
	long int ii,jj,kk,ll,mm,nn,maxlinelen=0;
	int x,y,z,col;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char *words=NULL;
	long nwords=0,*iword=NULL,*start=NULL,*start1=NULL,*stop1=NULL,*list=NULL;
	long *matrix=NULL;

	/* arguments */
	char *infile,*pword,setd1[MAXLINELEN],setd2='#';
	int setverb=0;
	long setn=2;

	snprintf(setd1,MAXLINELEN,", \t");

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Group items in a list\n");
		fprintf(stderr,"USAGE: %s [in1] [options]\n",thisprog);
		fprintf(stderr,"	[in1]: file name or \"stdin\", each input row = list of items\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-d1: input item-delimiters (multi-char) [, \\t]\n");
		fprintf(stderr,"	-d2: output within-group item-delimiter (single-char) [%c]\n",setd2);
		fprintf(stderr,"	-n:  number of items in each group - does not span lines [%ld]\n",setn);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	echo \"1,,3,4,5,6\" | %s stdin -n 2 -d2 \"#\"\n",thisprog);
		fprintf(stderr,"	output:	1#	3#4	5#6");
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	- for each input line, tab-delimited groups of [n] items\n");
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
			else if(strcmp(argv[ii],"-d1")==0)   snprintf(setd1,MAXLINELEN,argv[++ii]);
			else if(strcmp(argv[ii],"-d2")==0)   setd2= *(argv[++ii]+0);
			else if(strcmp(argv[ii],"-n")==0)    setn= atol(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb != 999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0,1, or 999\n\n",thisprog,setverb);exit(1);}
	if(setn<1) { fprintf(stderr,"\n--- Error [%s]: invalid -n [%ld], must be >0\n\n",thisprog,setn);exit(1);}

	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn= 0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		nn++;
		/* parse the line */
		iword= xf_lineparse2(line,setd1,&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		if((nwords%setn)!=0) {fprintf(stderr,"\n--- Error[%s]: words (%ld) on line %ld is not a multiple of setn (%ld)\n\n",thisprog,nwords,nn,setn);exit(1);};
		for(ii=jj=0;ii<nwords;ii++) {
			printf("%s",line+iword[ii]);
			jj++;
			if(jj<setn) printf("%c",setd2);
			else {if(jj<nwords) printf("\t"); jj=0;}
		}
		printf("\n");
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(words!=NULL) free(words);
	if(iword!=NULL) free(iword);
	if(line!=NULL) free(line);
	exit(0);
}
