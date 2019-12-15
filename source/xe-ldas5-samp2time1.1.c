#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <time.h>

#define thisprog "xe-ldas5-samp2time1"
#define TITLE_STRING thisprog" v 1: 5.March.2016 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/* <TAGS> time </TAGS> */

/*
v 1: 5.March.2016 [JRH]
	- finished program, including completion of xf_datemod1 function 
	
v 1: 8.September.2015 [JRH]
*/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_datemod1(int *input, int adjust, char *field, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],line[MAXLINELEN],*pline,*pcol,message[MAXLINELEN];
	int x,y,z,col,colmatch;
	FILE *fpin;
	long ii,jj,kk,ll,nn,mm;
	double aa;
	/* program-specific variables */ 
	int indate[6];
	long nwords,*words=NULL;
	/* arguments */
	char setstart[256]; // YY:MM:dd:hh:mm:ss
	char remaindertext[256];
	long setsample,sampsecs_l;
	double setsf=19531.25,sampsecs_d,remainder;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate the date and time corresponding with a given sample-number\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [start] [samp] [options]\n",thisprog);
		fprintf(stderr,"	[start]: file or \"stdin\", specifying start date/time \n");
		fprintf(stderr,"		- this is the date & time for sample-zero\n");
		fprintf(stderr,"		- single-column input\n");
		fprintf(stderr,"		- format: YYY:MMM:DDD:hh:mm:ss\n");
		fprintf(stderr,"		- start times must be rounded to nearest second\n");
		fprintf(stderr,"	[samp]: sample number (zero-offset)\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-sf: sample frequency [%g]\n",setsf);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt 20000 -sf 20000\n",thisprog);
		fprintf(stderr,"	echo 16:12:31:23:59:15 | %s stdin 20000 -sf 20000\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	date/time in format YY:MM:DD:hh:mm:ss\n");
		fprintf(stderr,"	NOTE: seconds might include a decimal portion\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item */
	snprintf(infile,256,"%s",argv[1]);
	setsample=atol(argv[2]);

	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-sf")==0) setsf=atof(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	
	sampsecs_d= (double)setsample/setsf;
	sampsecs_l= (long)(sampsecs_d);
	remainder= sampsecs_d - (double)sampsecs_l;
	if(remainder>0.0) {
		sprintf(remaindertext,"%g",remainder); 
		for(ii=0;ii<(strlen(remaindertext));ii++) remaindertext[ii]=remaindertext[ii+1];
	}
	else sprintf(remaindertext,"");

	/* STORE DATA METHOD 1b - newline-delimited single double */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while(fgets(line,MAXLINELEN,fpin)!=NULL) { 

		/* replace delimiters in line with NULLS and create array of indices to each word (day,month or year) */
		words= xf_lineparse2(line,":",&nwords);
		/* there should be three fields */
		if(nwords!=6) {fprintf(stderr,"\b\n\t*** %s [ERROR]: start date/time should have 6 fields, YY:MM:DD:hh:mm:ss\n",thisprog);exit(1);}
		/* set up pointers to words */ 
		indate[0]= atoi(line+words[0]);
		indate[1]= atoi(line+words[1]);
		indate[2]= atoi(line+words[2]);
		indate[3]= atoi(line+words[3]);
		indate[4]= atoi(line+words[4]);
		indate[5]= atoi(line+words[5]);

		x= xf_datemod1(indate,sampsecs_l,"sec",message);
		if(x!=0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

		printf("%d",indate[0]); 
		for(ii=1;ii<6;ii++) printf(":%d",indate[ii]); 
		printf("%s\n",remaindertext); 

	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	exit(0);
}

