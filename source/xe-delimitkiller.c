#define thisprog "xe-delimitkiller"
#define TITLE_STRING thisprog" v 1: 23.October.2018 [JRH]"
#define MAXLINELEN 10000
#define MAXWORDLEN 256
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>string</TAGS>
v 1: 23.October.2018 [JRH]
	- update style and variable names, add TAGS section
v 1: 10.June.2011 [JRH]
*/

int main(int argc, char *argv[]) {
	char line[MAXLINELEN],temp_str[256],col,*pline,*pcol,*tempcol;
	char newdelimiter,prev,name[MAXLINELEN]="tab\0";
	long ii,jj,kk;
	long linelen;
	FILE *fpin,*fpout;
	/* arguments */
	char *infile= NULL;
	newdelimiter=' ';

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2){
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
	 	fprintf(stderr,"Remove extra delimiters from an input\n");
		fprintf(stderr,"USAGE: %s [infile] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-d delimiter: tab,space,comma,colon,semicolon,underscore or newline [%s]\n",name);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s a.txt -d tab \n",thisprog);
		fprintf(stderr,"	cat a.txt | %s stdin -d comma\n",thisprog);
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-' && ii < argc-1) {
			if(strcmp(argv[ii],"-d")==0) strcpy(name,argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
		}
	}
	/* create new delimiter based on specified name */
	if(strcmp(name,"space")==0) newdelimiter=' ';
	else if(strcmp(name,"comma")==0) newdelimiter=',';
	else if(strcmp(name,"tab")==0)	newdelimiter='\t';
	else if(strcmp(name,"colon")==0) newdelimiter=':';
	else if(strcmp(name,"semicolon")==0) newdelimiter=';';
	else if(strcmp(name,"underscore")==0) newdelimiter='_';
	else if(strcmp(name,"newline")==0) newdelimiter='\n';
	else {fprintf(stderr,"\n--- Error[%s]: invalid delimiter: \"%s\"\n\n",thisprog,name); exit(1);}


	/******************************************************************************
	OPEN INPUT AND START KILLING DELIMITERS
	******************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		linelen=(strlen(line))-1;
		prev='\0';
 		for(ii=0;ii<linelen;ii++) {
			if(line[ii]==newdelimiter && line[ii]==prev) continue;
			printf("%c",line[ii]);
			prev= line[ii];
		}
		printf("\n");
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	exit(0);
}
