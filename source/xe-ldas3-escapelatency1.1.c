#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-ldas3-escapelatency1"
#define TITLE_STRING thisprog" v 1: 24.September.2012 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing behaviour </TAGS>
*/


/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],word[MAXLINELEN],*matchstring=NULL,*pline,*pcol;
	long int i,j,k,n;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d; 
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */ 
	int onplatform=0,omissions=0;
	double start=-1.0;
	/* arguments */
	int setformat=1,setbintot=25,coldata=1;
	float setlow=0.0,sethigh=0.0,setbinwidth=0.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calcualte escape latency from a comment file input\n");
		fprintf(stderr,"Assumes input format = <time><TAB><comment>\n");
		fprintf(stderr,"	TONE_ON marks the start of a trial\n");
		fprintf(stderr,"	ENTER_Z2 marks when the subject escapes to the platform\n");
		fprintf(stderr,"	ENTER_Z2 marks when the subject leaves the platform\n");
		fprintf(stderr,"	if the subject is on the platform at trial-start, the result is \"NAN\"\n");
		fprintf(stderr,"	if the subject does not escape before the next trial the result is \"OMIT\"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s 001-120426_00.cmt\n",thisprog);
		fprintf(stderr,"	cat 001-120426_00.cmt | %s stdin\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	trial, start-time and escape latency\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-t")==0) 	{ setformat=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}



	/* STORE DATA METHOD 3 - newline delimited input  from user-defined columns */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	n=0;
	printf("trial\tstart\tescape\n");
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(line[0]=='#') continue;
		pline=line; colmatch=2; // number of columns to match
		for(col=1;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
			pline=NULL; 
			if(col==1 && sscanf(pcol,"%lf",&aa)==1) colmatch--; // store value - check if input was actually a number
			if(col==2 && sscanf(pcol,"%s",&word)==1) colmatch--; // store value - check if input was actually a number
		}
		if(colmatch!=0) continue;
		
		if(strcmp(word,"TONE_START")==0) {
			if(start>=0.0) printf("OMIT\n"); // terminate any previously begun waiting-periods in which a platform-entry was not detected
			n++;
			start=aa;
			printf("%d\t%g\t",n,start); // output the new trial-number and start-time
			if(onplatform==1) { // if the subject is already on the platform then the trial is invalid
				start=-1.0;
				printf("NAN\n");
			} 
		}

		if(strcmp(word,"ENTER_Z2")==0) {
			onplatform=1;
			if(start>=0.0) {
				printf("%g\n",(aa-start));
				start=-1.0;
			}
		}
		if(strcmp(word,"EXIT_Z2")==0) onplatform=0;

	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	
	exit(0);
	}

