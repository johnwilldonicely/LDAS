#define thisprog "xe-transpose2"
#define TITLE_STRING thisprog" v 1: 14.August.2012 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>file</TAGS>

13.April.2019 [JRH]
	- bugfix printing of redundant extra delimiter at end of line
*/


/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],word[256],*matchstring=NULL,*pline,*pcol;
	long int i,j,k,n;
	int v,w,x,y,z,col;
	FILE *fpin;
	/* program-specific variables */
	char new_delimiter[5];
	/* arguments */
	char name[MAXLINELEN]="tab\0";

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"- Transpose input to a single row (remove newlines)\n");
		fprintf(stderr,"- Max input line length = %d\n",MAXLINELEN);
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\" containing newline-separated data\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt \n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-d")==0) 		{strcpy(name,argv[i+1]);i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}
	/* create new delimiter based on specified name */
	if(strcmp(name,"space")==0) strcpy(new_delimiter," ");
	if(strcmp(name,"comma")==0) strcpy(new_delimiter,",");
	if(strcmp(name,"tab")==0)	strcpy(new_delimiter,"\t");
	if(strcmp(name,"colon")==0) strcpy(new_delimiter,":");
	if(strcmp(name,"semicolon")==0) strcpy(new_delimiter,";");
	if(strcmp(name,"newline")==0) strcpy(new_delimiter,"\n");
	if(strcmp(new_delimiter,"0")==0) {fprintf(stderr,"\n--- Error[%s]: invalid delimiter: \"%s\"\n\n",thisprog,name); exit(1);}

	/* READ DATA - OUTPUT IN A SINGLE ROW */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	/* initialize variables */
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		pline=line;
		for(col=1;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
			pline=NULL;
			if(col>1) printf("%s",new_delimiter);
			printf("%s",pcol);
		}
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	printf("\n");
	exit(0);
	}
