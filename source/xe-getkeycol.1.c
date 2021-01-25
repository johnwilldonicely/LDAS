#define thisprog "xe-getkeycol"
#define TITLE_STRING thisprog" v 2: 25.January.2021 [JRH]"
#define MAXWORDLEN 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>database</TAGS>
v 2: 25.January.2021 [JRH]
	- fix memory allocation bug

v 2: 24.March.2019 [JRH]
	- bugfix: previous update failed to set delimiters to default of whitespace-or-comma
	- update instructions to indicate that consecutive delimiters are ignored

v 2: JRH, 24.November.2011

*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line, long *nwords);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
char *xf_strescape1(char *line);
/* external functions end */

int main (int argc, char *argv[]) {

	/* line-reading and word/column-parsing */
	char *line=NULL;
	long *keycols=NULL,nkeys=0,*iword=NULL,nlines=0,nwords=0,maxlinelen=0,prevlinelen=0;

	/* general variables */
	char *delimdefault=" ,\t\n";
	long int ii,jj,kk,nn;
	FILE *fpin,*fpout;

	/* arguments */
	char *infile=NULL,*keyword,delimiters[MAXWORDLEN];
	int setcase=1,setdelimiters=0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Identify the column-number in an ASCII file matching a keyword\n");
		fprintf(stderr,"- will return the column in the first line matchinng the keyword\n");
		fprintf(stderr,"- NOTE: the key does not need to e on the first line\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [in] [key] [options]\n",thisprog);
		fprintf(stderr,"	[in]: file name or \"stdin\"\n");
		fprintf(stderr,"	[key]: the keyword to be matched (any word on the line)\n");
		fprintf(stderr,"VALID OPTIONS...\n");
		fprintf(stderr,"	-c: case sensitive? (0=no, 1=yes, default=%d)\n",setcase);
		fprintf(stderr,"	-d: characters to use as column-delimiters [ ,\\t\\n])\n");
		fprintf(stderr,"		- if manually set, every delimiter = new column\n");
		fprintf(stderr,"		- otherwise, consecutive delimters are collapsed\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt rate -c 0 -d \'\\t -\'\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin PHONE\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	column in the first line matching the keyword\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	keyword= argv[2];
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-c")==0)  setcase= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-d")==0) {
				sprintf(delimiters,"%s",xf_strescape1(argv[++ii]));
				setdelimiters=1;
			}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setcase!=0&&setcase!=1) {fprintf(stderr,"\n--- Error[%s]: invalid value for -c option (%d) - must be 0 or 1\n\n",thisprog,setcase); exit(1);}
	if(setdelimiters==0) snprintf(delimiters,MAXWORDLEN,"%s",delimdefault);

	/* READ THE INPUT AND OUPUT MATCHING KEYWORD VALUES */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

		/* parse the line */
		if(setdelimiters==0) iword= xf_lineparse1(line,&nwords);
		else iword= xf_lineparse2(line,delimiters,&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};

		if(setcase==1) {
			for(ii=0;ii<nwords;ii++) {
				if(strcmp((line+iword[ii]),keyword)==0) {
					printf("%ld\n",(ii+1));
					exit(0);
		}}}
		else {
			for(ii=0;ii<nwords;ii++) {
				if(strcasecmp((line+iword[ii]),keyword)==0) {
					printf("%ld\n",(ii+1));
					exit(0);
		}}}
	}


	if(line!=NULL) free(line);
	if(keycols!=NULL) free(keycols);
	if(iword!=NULL) free(iword);
	exit(0);
	}
