#define thisprog "xe-bin1"
#define TITLE_STRING thisprog" v 7: 14.March.2015 [JRH]"
#define MAXLINELEN 1000

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*
<TAGS>stats signal_processing</TAGS>

v 7: 14.March.2015 [JRH]
	- BUGFIX:
		- previously if the last window (leftover data) included too few samples, result could be an extreme
		- now leftover data is summed with the previous window
		- this ensures the last average is based on a minimum of [binsize] samples
		- note that this correction is not required for peak-detection, where the object is to identify extremes

v 6: 19.January.2014 [JRH]
	- add peak-detect capability

v 5: 20.June.2014 [JRH]
	- allow for fractional bin-sizes
	- hence the program now provides an  efficient solution for real-time decimatoin by rational factorsrelational

v 4: 18.February.2014 [JRH]
	- remove output of sample-number
	- incorporate test for non-finite numbers to minimise impact on output

v 1.3: 14.August.2012 [JRH]
	- replace fscanf input method with safer fgets

v 1.2: 17.July.2012 [JRH]
	- bugfix: fixed error when scanning "-" or "." to a number - previously this failed but the character was still read. Replace with 2-step read - fisrst scan to string, then scan to number
*/

/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char line[MAXLINELEN];
	long i,j;
	double aa,bb;
	FILE *fpin;
	/* program specific variables */
	long n1,n2,n3,imin,imax,nprev;
	double *buffer=NULL,limit,sum,sumprev,mean,min,max;
	/* arguments */
	int settype=1;
	char infile[MAXLINELEN];
	double binsize;

	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Bin (down-sample) data using non-overlapping windows\n");
		fprintf(stderr,"Binning is performed on the fly, so there is no memory overhead\n");
		fprintf(stderr,"Two options: averaging or peak-detect\n");
		fprintf(stderr,"	- averaging method acts as a low-pass filter\n");
		fprintf(stderr,"	- for this method any partial bin at the end of the input is\n");
		fprintf(stderr,"	  combined with the previous bin to avoid under-averaging\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE: %s [input] [binsize] [options]\n",thisprog);
		fprintf(stderr,"	[infile]: single-column data file or \"stdin\"\n");
		fprintf(stderr,"		- non-numbers do not contribute to means\n");
		fprintf(stderr,"		- bins without finite numbers will output NAN\n");
		fprintf(stderr,"	[binsize]: number of values to average at a time\n");
		fprintf(stderr,"			- can be a fraction\n");
		fprintf(stderr,"			- must be greater than or equal to 1\n");
		fprintf(stderr,"			- if exactly 1, every input value is output\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-t(ype):[%d]\n",settype);
		fprintf(stderr,"		1= within-bin average\n");
		fprintf(stderr,"		2= largest deviation from within-bin average\n");
		fprintf(stderr,"OUTPUT: \n");
		fprintf(stderr,"	the binned version of the input\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLE: \n");
		fprintf(stderr,"	to reduce the sample-rate of an input from 496Hz to 400Hz\n");
		fprintf(stderr,"	binsize= 496/400 = 1.24\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"	%s  data.txt 1.24\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(1);
	}

	/* READ AND CHECK COMMAND-LINE ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	binsize=atof(argv[2]);

	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-t")==0)   settype=atoi(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(binsize<1.0){fprintf(stderr,"Error[%s]: bin (%g) must be >=1 sample\n",thisprog,binsize);exit(1);}
	if(settype!=1&&settype!=2){fprintf(stderr,"Error[%s]: binning type (%d) must be 1 or 2 \n",thisprog,settype);exit(1);}


	/* INITIALIZE VARIABLES */
	n1=0; // total-line-counter
	n2=0; // total valid values in a given bin
	n3=0; // total "leftover" values which have not been binned at the end of reading the input
	sum=0.0; // the sum for any given bin
	limit=binsize-1.0; // the initial threshold that the sample-number must exceed to trigger output
	nprev=0;
	sumprev=0.0;

	/* READ THE DATA - ASSUME ONE NUMBER (SAMPLE) PER LINE */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"Error[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}

	if(settype==1) {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			// if line cannot be converted to a number, treat as if it is a NAN
			if(sscanf(line,"%lf",&aa)!=1) aa=NAN;
			// if the value is a finite number, it contributes to the sum and n for the current window (i.e. n2)
			if(isfinite(aa)) { sum+=aa; n2++; }
			// if the current line-number (sample) is >= the limit defining the right edge of the curent window, output the mean and reset
			if((double)n1>=limit) {
				if(n2>0) printf("%g\n",(sum/(double)n2));
				else printf("NAN\n");
				limit+=binsize; // increase the sample-number threshold
				sumprev=sum;
				nprev=n2;
				sum=0.0;
				n2=n3=0;
			}
			// increment leftover counter regardless of whether input is a valid number
			else n3++;
			// increment the counter for the total number of samples read
			n1++;
	}}


	if(settype==2) {
		i= (1+(long)binsize)*sizeof(double);
		if((buffer=(double *)realloc(buffer,i))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			// if line cannot be converted to a number, treat as if it is a NAN
			if(sscanf(line,"%lf",&aa)!=1) aa=NAN;
			// if the value is a finite number, it contributes to the buffer contents (i.e. n2)
			if(isfinite(aa)) { sum+=aa; buffer[n2++]=aa; }
			// if the current line-number (sample) is >= the limit defining the right edge of the curent window, output the mean and reset
			if((double)n1>=limit) {
				if(n2>0) {
					max=0.00;
					imax=0;
					mean=sum/(double)n2;
					for(i=1;i<n2;i++) {
						bb=fabs((buffer[i]-mean));
						if(bb>max) { imax=i; max=bb; }
					}
					printf("%g\n",buffer[imax]);
				}
				else {
					printf("NAN\nNAN\n");
				}
				limit+=binsize; // increase the sample-number threshold
				sum=0.0;
				n2=n3=0;
			}
			// increment leftover counter regardless of whether input is a valid number
			else n3++;
			// increment the counter for the total number of samples read
			n1++;
	}}



	// CLOSE THE FILE IF THE INPUT IS NOT "STDIN"
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	/* IF THERE WAS LEFTOVER DATA, DEAL WITH IT */
	if(n3>0) {
		if(settype==1) {
			/* add sum and n  to that for previous window to reduce outliers resulting from very small number of samples in average */
			sum+=sumprev;
			n2+=nprev;
			if(n2>0) printf("%g\n",(sum/(double)n2));
			else printf("NAN\n");
		}
		if(settype==2) {
			if(n2>0) {
				max=0.00;
				imax=0;
				mean=sum/(double)n2;
				for(i=1;i<n2;i++) {
					bb=fabs((buffer[i]-mean));
					if(bb>max) { imax=i; max=bb; }
				}
				printf("%g\n",buffer[imax]);
			}
			else {
				printf("NAN\nNAN\n");
			}
		}
	}

	if(buffer!=NULL) free(buffer);
	exit(0);
}
