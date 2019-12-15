#define thisprog "xe-getkeycol"
#define TITLE_STRING thisprog" v 2: 24.March.2019 [JRH]"
#define MAXLINELEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>database</TAGS>

v 2: 24.March.2019 [JRH]
	- bugfix: previous update failed to set delimiters to default of whitespace-or-comma
	- update instructions to indicate that consecutive delimiters are ignored

v 2: JRH, 24.November.2011

*/

/* external functions start */
char *xf_strescape1(char *line);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char line[MAXLINELEN],word[MAXLINELEN],*pcol,*pline;
	long int i,j,k,col;
	FILE *fpin,*fpout;
	/* arguments */
	char *infile,*keyword,delimiters[MAXLINELEN],delimout[4],*delimdefault=" ,\t\n";
	int setcase=1,setdelimiters=0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Identify the column-number in an ASCII file header matching a keyword\n");
		fprintf(stderr,"- Other progs can use this number to read the correct data column\n");
		fprintf(stderr,"- NOTE: multiple delimiters are treated as a single delimiter\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [in] [key] [options]\n",thisprog);
		fprintf(stderr,"	[in]: file name or \"stdin\"\n");
		fprintf(stderr,"	[key]: the keyword to be matched (any word on the line)\n");
		fprintf(stderr,"VALID OPTIONS...\n");
		fprintf(stderr,"	-c: case sensitive? (0=no, 1=yes, default=%d)\n",setcase);
		fprintf(stderr,"	-d: characters to use as column-delimiters [ ,\\t\\n])\n");
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
	for(i=3;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-c")==0)  setcase=atoi(argv[++i]);
			else if(strcmp(argv[i],"-d")==0) {
				sprintf(delimiters,"%s",xf_strescape1(argv[++i]));
				sprintf(delimout,"%c",delimiters[0]);
				setdelimiters=1;
			}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(setcase!=0&&setcase!=1) {fprintf(stderr,"\n--- Error[%s]: invalid value for -c option (%d) - must be 0 or 1\n\n",thisprog,setcase); exit(1);}
	if(setdelimiters==0) {
		sprintf(delimiters,"%s",delimdefault);
		sprintf(delimout,delimiters);
	}

	/* READ THE INPUT AND OUPUT MATCHING KEYWORD VALUES */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"Error[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}

	if(setcase==1) {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			pline=line;
			for(col=1;(pcol=strtok(pline,delimiters))!=NULL;col++) {
				pline=NULL;
				if(strcmp(pcol,keyword)==0) {printf("%d\n",col); exit(0);}
	}}}

	else {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			pline=line;
			for(col=1;(pcol=strtok(pline,delimiters))!=NULL;col++) {
				pline=NULL;
				if(strcasecmp(pcol,keyword)==0) {printf("%d\n",col); exit(0);}
	}}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	exit(0);
	}
