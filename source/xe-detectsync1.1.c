#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

#define thisprog "xe-detectsync1"
#define TITLE_STRING thisprog" v 1: 28.March.2014 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing</TAGS>
v 1: 28.March.2014 [JRH]
*/

/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],line[MAXLINELEN],message[MAXLINELEN];
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	int sizeofdouble=sizeof(double);
	size_t ii,jj,kk,nn,mm;
	/* program-specific variables */
	size_t count;
	double *tstamp=NULL;
	/* arguments */
	size_t setcount=1;
	double setgap=0.0,setdur=0.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Detect valid sync-pulse-sequences in a series of timestamps\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"		- a list of individual sync pulse times (any units)\n");
		fprintf(stderr,"		- assumes one valid numeric value per input line\n");
		fprintf(stderr,"		- blank lines and non-numeric values will be ignored\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-count: number of sync-pulses required in a row [%ld]\n",setcount);
		fprintf(stderr,"	-dur: duration (units) within which the sequence must be detected [%g]\n",setdur);
		fprintf(stderr,"	-gap: gap (units) required before start & end of the sequence [%g]\n",setgap);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	to detect 4-sync-pulses within 0.5s surrounded by a 1s gap...\n");
		fprintf(stderr,"	%s sync.txt -count 4 -dur 0.5 -gap 1\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	time representing the first pulse in each valid sequence\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	/********************************************************************************/
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-gap")==0)   setgap=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-dur")==0)   setdur=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-count")==0)   setcount=atol(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setgap<0) {fprintf(stderr,"\n--- Error[%s]: -gap (%g) must be greater than zero\n\n",thisprog,setgap);exit(1);};
	if(setdur<0) {fprintf(stderr,"\n--- Error[%s]: -dur (%g) must be greater than zero\n\n",thisprog,setdur);exit(1);};
	if(setcount<0) {fprintf(stderr,"\n--- Error[%s]: -count (%ld) must be greater than zero\n\n",thisprog,setcount);exit(1);};


	/********************************************************************************/
	/* STORE DATA METHOD - newline-delimited timestamps */
	/********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf",&aa)!=1) continue;
		if(isfinite(aa)) {
			if((tstamp=(double *)realloc(tstamp,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			tstamp[nn++]=aa;
	}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/********************************************************************************/
	/* DETECT SEQUENCES OF TIMESTAMPS REPRESENTING A VALID SYNC-PULSE SERIES */
	/********************************************************************************/
	aa=(0.00-setgap); // allow that first detected timestamp can always be part of a valid sequence
	for(ii=0;ii<nn;ii++) {
		// if there is a gap separating this sample and the previous one, look for a sequence of close timestamps
		if((tstamp[ii]-aa)>=setgap) {

			// record time of start for potentially valid sequence
			bb=tstamp[ii];
			// set counter for the number of pulses detected in a row
			count=1;

			for(jj=ii+1;jj<nn;jj++) {
				if((tstamp[jj]-bb) <setdur) count++;
				else break;
			}
			// reset ii to avoid going back over same data
			ii=jj-1;
			// record the gap between the next timestamp and the last timestamp of the series
			if(jj<nn) cc=tstamp[jj]-tstamp[ii];
			// if the last timestamp of the series is also the last timestamp, there is a valid gap by definition
			else cc=setgap;

			if(cc>=setgap && count==setcount)printf("%.6f\n",bb);
		}

		aa=tstamp[ii];
	}

	if(tstamp!=NULL) free(tstamp);
	exit(0);
	}
