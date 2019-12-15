#define thisprog "xe-decimate1"
#define TITLE_STRING thisprog" v 1: 23.June.2014 [JRH]"
#define MAXLINELEN 1000

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*
<TAGS>signal_processing</TAGS>

v 1: 23.June.2014 [JRH]
	- based on xe-bin1.5.c, but simply output rather than sum and average data
*/

int main (int argc, char *argv[]) {
	/* general variables */
	char line[MAXLINELEN];
	double aa;
	FILE *fpin;
	/* program specific variables */
	long n1,n2,n3;
	double limit,sum,mean;
	/* arguments */
	char infile[MAXLINELEN];
	double setdec=1.0;

	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Decimate data by outputting every Nth line (N can be a fraction)\n");
		fprintf(stderr,"Decimation is performed on the fly, so there is no memory overhead\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE: %s [input][N]\n",thisprog);
		fprintf(stderr,"	[infile]: data file or \"stdin\" - can be multi-column\n");
		fprintf(stderr,"	[N]: samples between each output [%g]\n",setdec);
		fprintf(stderr,"		- can be a fraction\n");
		fprintf(stderr,"		- must be greater than or equal to 1\n");
		fprintf(stderr,"		- if exactly 1, every input value is output\n");
		fprintf(stderr,"		- first input value is always outptut\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"OUTPUT: \n");
		fprintf(stderr,"	the decimated (down-sampled) version of the input\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLE: \n");
		fprintf(stderr,"	to reduce the sample-rate of an input from 496Hz to 400Hz\n");
		fprintf(stderr,"	N= 496/400 = 1.24\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"	%s  data.txt 1.24\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(1);
	}

	/* READ AND CHECK COMMAND-LINE ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	setdec=atof(argv[2]);
	if(setdec<1.0){fprintf(stderr,"Error[%s]: decimation factor (%g) must be >=1 sample\n",thisprog,setdec);exit(1);}

	/* INITIALIZE VARIABLES */
	n1=0; // total-line-counter
	limit=0.0; // the initial threshold that the sample-number must exceed to trigger output


	/* READ THE DATA - ASSUME ONE NUMBER (SAMPLE) PER LINE */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"Error[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}

	while(fgets(line,MAXLINELEN,fpin)!=NULL) {

		// if the current line-number is >= the limit defining the right edge of the curent window, output the line and reset
		if((double)n1>=limit) {
			printf("%s",line);
			limit+=setdec; // increase the sample-number threshold
		}
		// increment the counter for the total number of lines read
		n1++;
	}
	// close the file if the input is not "stdin"
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	exit(0);
}
