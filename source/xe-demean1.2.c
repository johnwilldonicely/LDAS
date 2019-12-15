#define thisprog "xe-demean1"
#define TITLE_STRING thisprog"  v 2: 29.February.2016 [JRH]"
#define MAXLINELEN 1000

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*
<TAGS>signal_processing filter</TAGS>

 v 2: 29.February.2016 [JRH]
 	- bugfix handling of beginning and end - now outputs proper values instead of zeros
	- bugfix calculation of midwin and total output (was consistently outputting 1-2 less samples than input
	- internally force nwin to even number
	- allow output of NAN where input value or window-mean is NAN

 v 2: 10.February.2015 [JRH]
 	- simplified handling of first and last half-window
	- previously these points were corrected by the average of data they were not in the middle of - tended to produce edge effects
	- now simply outputs zero at the edges, mimicing perfect filtering

*/


/* external functions start */
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char line[MAXLINELEN],message[MAXLINELEN];
	size_t ii,jj,kk,sizeofdouble=sizeof(double);
	int x,y,z;
	long n,m;
	double aa,bb;
	FILE *fpin;

	/* program specific variables */
	size_t nn=0,mm=0,pwin,midwin,nmean;
	double *window=NULL,*tempx=NULL,*tempy=NULL,sum,mean;

	/* arguments */
	char infile[MAXLINELEN];
	size_t nwin;

	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"De-mean a data series using a sliding ring-buffer window\n");
		fprintf(stderr,"Averaging is performed on the fly, so there is little memory overhead\n");
		fprintf(stderr,"This is effectively a high-pass filter, with the advantage of\n");
		fprintf(stderr,"	permitting in-line processing (only the buffer is stored in memory)\n");
		fprintf(stderr,"Non-numerical values, NAN and INF will be converted to zero\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE: \n");
		fprintf(stderr,"	%s [input][nwin]\n",thisprog);
		fprintf(stderr,"		[infile]: single-column data file or \"stdin\"\n");
		fprintf(stderr,"		[nwin]: number of values to average at a time\n");
		fprintf(stderr,"			- must be an integer greater than or equal to 3\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"OUTPUT: \n");
		fprintf(stderr,"	- the de-meaned data, one output for every input \n");
		fprintf(stderr,"EXAMPLE: apply a 5Hz high-pass filter to data stream sampled at 1KHz\n");
		fprintf(stderr,"	%sdata.txt 200\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(1);
	}

	/* READ AND CHECK COMMAND-LINE ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	nwin=atol(argv[2]);
	if(nwin<3.0){fprintf(stderr,"Error[%s]: averaging window (%ld) must be >=3 samples\n",thisprog,nwin);exit(1);}

	/* CREATE RING_BUFFER WINDOW  */
	if((window=(double*)calloc(nwin,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};

	/* CREATE TEMP ARRAYS TO HOLD LINEAR COPY OF DATA */
	if((tempx=(double*)calloc(nwin,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((tempy=(double*)calloc(nwin,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};

	/* ADJUST NWIN IF NECESSARY TO MAKE IT AN EVEN NUMBER */
	if((nn%2)!=0) nwin+=1;

	/* SET THE STARTING MIDPOINT FOR THE WINDOW */
	midwin= nwin/2.0;

	pwin=0;
	nmean=0;
	sum=mean=0.0;
	n=0;

	//for(ii=0;ii<nwin;ii++) fprintf(stderr,"%ld	%g\n",ii,window[ii]);

	/* STORE DATA METHOD */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	while(fgets(line,MAXLINELEN,fpin)!=NULL) {

		if(sscanf(line,"%lf",&aa)!=1) aa=NAN;

		/* if less than one window has been acquired, fill the ring-buffer */
		if(nn<nwin) {
			window[pwin]=aa;
			if(isfinite(aa)) { sum+=aa; nmean++;}
			if(++pwin==nwin) pwin=0;
		}
		else {
			/* if exactly one window has been acquired, output the first half now, before updating it */
			if(nn==nwin) {
				if(nmean>0) {
					mean=sum/nmean;
					for(ii=0;ii<midwin;ii++) {
						if(mean!=0.0 && isfinite(window[ii])) fprintf(stdout,"%g\n",(window[ii]-mean));
						else fprintf(stdout,"0.00\n");
					}
				}
				else for(ii=0;ii<midwin;ii++) fprintf(stdout,"NAN\n");
			}
			/* update running average by dropping window datum about to be removed */
			if(isfinite(window[pwin])) { sum-=window[pwin];	if(nmean>0) nmean--; }
			/* update running average by adding window datum about to be inserted */
			if(isfinite(aa)) { sum+=aa;	if(nmean<nwin) nmean++; }
			/* output the corrected midpoint of the circular buffer window */
			if(nmean>0) {
				mean=sum/nmean;
				if(mean!=0.0 && isfinite(window[midwin]))
					fprintf(stdout,"%g\n",(window[midwin]-mean));
				else
					fprintf(stdout,"0.00\n");
			}
			else fprintf(stdout,"NAN\n");

			/* update the window datum */
			window[pwin]=aa;
			/* increment pointer to next data and mid-point in the ring buffer window */
			if(++pwin==nwin) pwin=0;
			if(++midwin==nwin) midwin=0;
		}
		nn++;
	}

	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* OUTPUT THE LAST HALF-WINDOW */
	jj=nwin/2.0;	/* use jj to the value representing 1/2 the window size */
	if(nmean>0) {
		mean=sum/nmean;
		for(ii=0;ii<jj;ii++) {
			if(mean!=0.0 && isfinite(window[midwin])) fprintf(stdout,"%g\n",(window[midwin]-mean));
			else fprintf(stdout,"0.00\n");
			/* increment midwin pointer */
			if(++midwin==nwin) midwin=0;
		}
	}
	else for(ii=0;ii<jj;ii++) fprintf(stdout,"NAN\n");


	free(tempx);
	free(tempy);
	free(window);
	exit(0);
}
