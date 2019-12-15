#define thisprog "xe-strsub1"
#define TITLE_STRING thisprog" v 3: 14.May.2018 [JRH]"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>string</TAGS>

v 3: 14.May.2018 [JRH]
	- make length of input line and old/new/in/ex variables unlimited
	- bugfix logic for excluding lines if include is also unset

v 3: 17.November.2015 [JRH]
	- add ability to replace text only on lines which also match (or don't match) a certain pattern
	- use definition of MAXWORDLEN to control printing to character arrays

v 2: 24.September.2012 [JRH]
	- remove unused references to preserved delimiter
	- update instructions

v 1: 4.September.2011 [JRH]
	- use new xf_strsub1 function which returns pointer to string - updates
*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
char* xf_strsub1 (char *source, char *str1, char *str2);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char *infile=NULL,*line=NULL,*newline=NULL;
	long ii,maxlinelen=0;
	FILE *fpin;
	/* program-specific variables */
	char *newword;
	int x,y;
	/* optional arguments */
	char *setold=NULL,*setnew=NULL,*setinclude=NULL,*setexclude=NULL;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<4) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Replace character-strings (find and replace)\n");
		fprintf(stderr,"- no line-length limit\n");
		fprintf(stderr,"- newlines automatically signify start of new words\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [old] [new]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"	[old]: string to replace\n");
		fprintf(stderr,"	[new]: replacement string\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-in: subtitution only in lines containing this pattern\n");
		fprintf(stderr,"	-ex: exclude subtitution in lines containing this pattern\n");
		fprintf(stderr,"		NOTE: both unset by default (all lines analyzed)\n");
		fprintf(stderr,"		NOTE: -ex will override -in\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s sdatafile \" \" \"_\"\n",thisprog);
		fprintf(stderr,"	cat datafile | %s stdin ABC abc -ex \"#\"\n",thisprog);
		fprintf(stderr,"	head datafile | grep Names | %s stdin Jones Janes\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	modified lines\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	setold= argv[2];
	setnew= argv[3];

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item */
	for(ii=4;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-in")==0)  setinclude= argv[++ii];
			else if(strcmp(argv[ii],"-ex")==0)  setexclude= argv[++ii];
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setinclude!=NULL) y=1; else y=0;
	if(setexclude!=NULL) x=1; else x=0;

	/* READ DATA - PARSE INTO WORDS */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n*** %s [ERROR: file \"%s\" not found]\n\n",thisprog,infile);exit(1);}

	/* first case - no criterion for including or excluding lines */
	if(x==0 && y==0) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			newline= xf_strsub1(line,setold,setnew);
			printf("%s",newline);
			free(newline);
		}
		if(strcmp(infile,"stdin")!=0) fclose(fpin);
	}
	/* second case - inclusion or exclusion criteria set */
	else {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

			if(x==1 && strstr(line,setexclude)!=NULL) {
				printf("%s",line);
				continue;
			}
			if(y==1 && strstr(line,setinclude)==NULL) {
				printf("%s",line);
				continue;
			}

			newline= xf_strsub1(line,setold,setnew);
			printf("%s",newline);
			free(newline);
		}
		if(strcmp(infile,"stdin")!=0) fclose(fpin);
	}
	exit(0);
}
