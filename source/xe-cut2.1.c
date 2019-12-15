#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define thisprog "xe-cut2"
#define TITLE_STRING thisprog" 15.February.2019 [JRH]"

/*
<TAGS>database file screen</TAGS>

15.February.2019 [JRH]
	- add -s4 option: indentation-block as a stop-signal
		- helps parsing .yaml (and possibly .json) files

12.November.2017 [JRH]
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL;
	long ii,jj,kk,maxlinelen=0;
	FILE *fpin,*fpout;
	/* program-specific variables */
	int charmatch,out=0,indent;
	/* arguments */
	char *infile=NULL,*setstart=NULL,*setstop3=NULL;
	int setverb=0,setstop1=0,setstop2=0,setstop4=0;

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract blocks of lines from a text file\n");
		fprintf(stderr,"- must define a character-string marking the beginning of each block\n");
		fprintf(stderr,"- default behaviour is to extract this line and all subsequent lines\n");
		fprintf(stderr,"- override this by using any combination of stop-options (s1,s2,s3,s4)\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [start]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"	[start]: quoted text marking start of block to be extracted\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"- stop on line (and do not extract) if...\n");
		fprintf(stderr,"	-s1 line is blank (0=NO 1=YES) [%d]\n",setstop1);
		fprintf(stderr,"	-s2 1st character matches that of start (eg. #,%%,/) (0=NO 1=YES) [%d]\n",setstop2);
		fprintf(stderr,"	-s3 line matches this quoted string of characters [unset]\n");
		fprintf(stderr,"	-s4 line indentation is <= that of [start] (0=NO 1=YES) [%d]\n",setstop4);
		fprintf(stderr,"	-verb sets verbosity (0=simple, 1=verbose) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt \"# block1\" -s1 1 -s3 \"# block2\"\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin \"%% START\" -s1 1\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	the desired block of text, sent to stdout\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/********************************************************************************
	READ THE FILENAME, START-SIGNAL AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile= argv[1];
	setstart= argv[2];
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-s1")==0)      setstop1= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-s2")==0)      setstop2= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-s3")==0)      setstop3= argv[++ii];
			else if(strcmp(argv[ii],"-s4")==0)      setstop4= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)    setverb=  atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setstop1!=0 && setstop1!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -s1 [%d] must be 0 or 1\n\n",thisprog,setstop1);exit(1);}
	if(setstop2!=0 && setstop2!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -s2 [%d] must be 0 or 1\n\n",thisprog,setstop2);exit(1);}
	if(setstop4!=0 && setstop4!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -s4 [%d] must be 0 or 1\n\n",thisprog,setstop4);exit(1);}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}

	/* ASSIGN FIRST CHARACTER OF SECTION IDENTIFIER AS SUBSEQUENT STOP SIGNAL */
	if(setstop2==1) charmatch= setstart[0];
	else charmatch= -1;

	/* READ THE DATA  */
	indent=-1;
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(strstr(line,setstart)!=NULL) {
			out= 1;
			indent= 0;
			while(isspace(line[indent])) indent++;
		}
		else {
			if(setstop1==1) { if(strlen(line)==1) out= 0; }
			if(setstop2==1) { if(line[0]==charmatch) out= 0; }
			if(setstop3!=NULL) { if(strstr(line,setstop3)!=NULL) out= 0; }
			if(setstop4==1) {
				kk= 0;
				while(isspace(line[kk])) kk++;
				if(kk<=indent) out=0;
			}
		}
		if(out==1) printf("%s",line);
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	if(line!=NULL) free(line);
	exit(0);
}
