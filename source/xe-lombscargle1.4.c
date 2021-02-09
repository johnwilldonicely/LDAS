#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-lombscargle1"
#define TITLE_STRING thisprog" v 4: 21.August.2018 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>signal_processing stats</TAGS>


TO DO:
	P-calculation is almost certainly incorrect

Recommend for cell rhythmicity:
	- calculate +-500ms autocorrelogram, use 200 bins (0.005s = 5ms per bin)

	NOTE!!! There is no limit to the number of bins which can be used to construct the histogram.
	This does not change the shape of the resulting Lomb-Scargle periodogram
	However it DOES increase the values, such that ALL frequencies can be made significant if the histogram is dense enough
	QUESTION: Is there an appropriate way to scale the periodogram based on the number of data points?

	ANSWER: Horne & Baliunas, 1986: correction using the variance in the input signal

v 4: 21.August.2018 [JRH]
	- add option to normalize LS-periodogram (output only)

v 4: 13.October.2017 [JRH]
	- fix AUC calculation - update instructions

v 4: 24.April.2017 [JRH]

	- potential bugfix: copy argument to line[MAXLINELEN] before calling line-parser function
	- add check for empty input
	- replace potentially dangerous use of variable name "time"
	- update memory allocation
	- separate zone-parsing and testing
	- remove option to base power on zone means instead of sums
	- use 50th-percentile-interval rather than mean-interval for calculating range
	- use xf_auc2() to calculate AUC instead of just taking sums

v 3: 9.October.2016 [JRH]
	- add Gaussian smoothing option for output
	- restore scaling of histogram for reducing probability of significance in dense histograms

v 3: 1.December.2015 [JRH]
	- add rhythmicity-ratio for frequency bands
	- switch to using ii,jj,kk etc for counters

v 2: 11.Jan.2013 [JRH]
	- add -n option to specify the number of frequencies in the output periodogram
	- correct estimation of significance threshold - tied to the number of frequencies analyzed, not the size of the original data set
*/


/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
void xf_lombscargle(double* t, double* x, double* w, double* P, int Nt, int Nw);
void xf_norm1_d(double *data,long N,int normtype);
int xf_smoothgauss1_d(double *original, size_t arraysize,int smooth);
int xf_percentile1_d(double *data, long n, double *result);
int xf_compare1_d(const void *a, const void *b);
int xf_auc2_d(double *curvex, double *curvey, size_t nn, int ref, double *result ,char *message);
int xf_norm2_d(double *data,long ndata,int normtype);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],line[MAXLINELEN],message[MAXLINELEN];
	long int ii,jj,kk,nn,mm,maxlinelen=MAXLINELEN;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd,ee,n2,result_d[16];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char *words=NULL;
	int *iword=NULL;
	int lenwords=0,nwords=0,*count,grp,bin,bintot,setrange=0,colx=1,coly=2;
	long *index=NULL,nindices,nzones,zone;
	double *xdat=NULL,*xdat2=NULL,*ydat=NULL,*freq=NULL,*start1=NULL,*stop1=NULL,*output=NULL,*prob=NULL;
	double min,max,interval,fstep,twopi=2.0*M_PI,thresh05,lombtot,lombmean;
	/* arguments */
	char *setindices=NULL;
	int nfreq=100,setverb=0,setout=1,setgauss=0,setnorm=-1;
	float setmin=-1.0,setmax=-1.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate the Lombs-Scargle periodogram for an autocorrelation\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\", format = autocorrelation <interval> <count>\n");
		fprintf(stderr,"		- recommend 1using +-500ms autocorellogram with 200 bins (5ms width)\n");
		fprintf(stderr,"		- note only intervals >0 will be considered\n");
		fprintf(stderr,"		- input counts will be normalized (mean=0, s.d.=1)\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-min: lowest frequency to analyze (-1 = AUTO) [%g]\n",setmin);
		fprintf(stderr,"	-max: highest frequency to analyze (-1 = AUTO) [%g]\n",setmax);
		fprintf(stderr,"	-nfreq: number of frequencies to output [%d]\n",nfreq);
		fprintf(stderr,"	-bands: comma-separated freq-bands-pairs for ratio calculation [unset]\n");
		fprintf(stderr,"	-g: apply Gaussian smoothing (samples) to output [%d]\n",setgauss);
		fprintf(stderr,"		- must be an odd integer >=3, or 0= no smoothng)\n");
		fprintf(stderr,"		- no effect on calculations (applied to output only)\n");
		fprintf(stderr,"	-out: output (1=summary, 2=periodogram) [%d]\n",setout);
		fprintf(stderr,"	-norm: normalize LS-periodogram (-1=NO, 0=0-1, 1=Zscore) [%d]\n",setnorm);
		fprintf(stderr,"	-verb sets verbosity (0=simple, 1=summary to stderr) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"OUTPUT (-out1):\n");
		fprintf(stderr,"	summary stats\n");
		fprintf(stderr,"	AUC for each zone\n");
		fprintf(stderr,"	Ratio of AUCzone / (AUCzone + AUCtotal)\n");
		fprintf(stderr,"OUTPUT (-out2):\n");
		fprintf(stderr,"	1st column: Frequency\n");
		fprintf(stderr,"	2nd column: Periodogram value [scaling uncertain at this point]\n");
		fprintf(stderr,"	3rd column: Significance (p)\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-min")==0)   setmin= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-max")==0)   setmax= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-nfreq")==0) nfreq=  atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-bands")==0) setindices= argv[++ii];
			else if(strcmp(argv[ii],"-g")==0)     setgauss=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)   setout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-norm")==0)   setnorm=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)  setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setout!=1 && setout!=2) { fprintf(stderr,"\n--- Error [%s]: invalid output (-out %d) - must be 1 or 2\n\n",thisprog,setout);exit(1);}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid verbocity (-verb %d) - must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid verbocity (-verb %d) - must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setnorm<-1||setnorm>1) { fprintf(stderr,"\n--- Error [%s]: -norm (%d) must be -1, 0 or 1\n\n",thisprog,setnorm);exit(1);}
	if((setgauss<3&&setgauss!=0)||(setgauss%2==0 && setgauss!=0)) { fprintf(stderr,"\n--- Error [%s]: -g [%d] must be 0 or an odd number 3 or larger\n\n",thisprog,setgauss);exit(1);}

	/* PARSE THE INDICES TO ZONES IN THE PERIODOGRAM, IF SET, FOR RHYTHMICITY-RATIO SCORES */
	nzones=nindices=0;
	if(setindices!=NULL) {
		strncpy(line,setindices,MAXLINELEN);
		index= xf_lineparse2(line,",",&nindices);
		if((nindices%2)!=0) {fprintf(stderr,"\n--- Error[%s]: index-list does not contain pairs of numbers: %s\n\n",thisprog,setindices);exit(1);}
		nzones= nindices/2;
		start1= realloc(start1, nzones*(sizeof *start1));
		stop1= realloc(stop1, nzones*(sizeof *stop1));
		if(start1==NULL||stop1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		for(ii=zone=0;ii<nindices;ii+=2) {
			start1[zone]= atof(line+index[ii]);
 			stop1[zone]=  atof(line+index[(ii+1)]);
			zone++;
	}}

	/* STORE THE INPUT HISTOGRAM - POSITIVE TIMES ONLY */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf %lf",&aa,&bb)!=2) continue;
		if(isfinite(aa) && isfinite(bb)) {
			if(aa>0.0) {
				xdat= realloc(xdat,(nn+1) * sizeof *xdat);
				ydat= realloc(ydat,(nn+1) * sizeof *ydat);
				if(xdat==NULL || ydat==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				xdat[nn]=aa;
				ydat[nn]=bb;
				nn++;
	}}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	// /* make a double-version of the number of elements in the histogram, for later maths  */
	n2=(double)nn;
	/* check input is not empty */
	if(nn==0) {fprintf(stderr,"\n--- Error[%s]: input is emmpty or contains only a single column\n\n",thisprog);exit(1);}
	/* normalize the input data so mean=0 and s.d.=1 */
	xf_norm1_d(ydat,nn,1);
	//TEST:	for(ii=0;ii<nn;ii++) printf("%g	%g\n",xdat[ii],ydat[ii]);exit(0);


	/* ALLOCATE MEMORY */
	xdat2= realloc(xdat2,nn*sizeof *xdat2);
	freq= realloc(freq,nfreq*sizeof *freq);
	output= realloc(output,nfreq*sizeof *output);
	prob= realloc(prob,nfreq*sizeof *prob);
	if(freq==NULL || output==NULL || prob==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	/* ESTIMATE THEORETICAL RANGE OF FREQUENCIES WHICH CAN BE ANALYZED */
	/* this is based o nthe median interval between timestamps (xdat) in the input histogram */
	for(ii=1;ii<nn;ii++) xdat2[ii]= xdat[ii]-xdat[(ii-1)];
	if(xf_percentile1_d(xdat2,(nn-1),result_d)>=0) interval=result_d[5];
	else {fprintf(stderr,"\n--- Error[%s/xf_percentile1_d]: insufficient memory\n\n",thisprog);exit(1);}
	min= 1.0/(interval*n2);
	max= 1.0/(interval*2.0);

	/* OVERIDE MIN & MAX, CHECKING SET VALUES ARE IN RANGE FIRST */
	if(setmin>0.0 && setmin<min) {fprintf(stderr,"\n--- Error[%s]: -min (%g) must be -1 (auto) or greater than the theoretical minimum frequency (%g)\n\n",thisprog,setmin,min); exit(1);}
	if(setmax>0.0 && setmax>max) {fprintf(stderr,"\n--- Error[%s]: -max (%.16f) must be -1 (auto) or less than the theoretical maximum frequency (%.16f)\n\n",thisprog,setmax,max); exit(1);}
	if(setmin>0.0) min=setmin;
	if(setmax>0.0) max=setmax;
	fstep=(max-min)/(double)(nfreq-1);



	/* BUILD THE LIST OF FREQUENCIES TO BE ANALYZED */
	aa=min; for(ii=0;ii<nfreq;ii++) { freq[ii]=aa; aa+=fstep; }

	/* CHECK THAT ZONES (IF THERE ARE ANY) FALL WITHIN DATA RANGE */
	if(setindices!=NULL) {
		for(ii=0;ii<nzones;ii++) {
			if(start1[ii]<min||stop1[ii]>max) {fprintf(stderr,"\n--- Error[%s]: index pair %ld (%g-%g) falls outside min-max (%g-%g)\n\n",thisprog,(ii+1),start1[ii],stop1[ii],min,max);exit(1);}
			if(stop1[ii]<=start1[ii]) {fprintf(stderr,"\n--- Error[%s]: for index pair %ld, start(%g) is >= stop(%g)\n\n",thisprog,(ii+1),start1[ii],stop1[ii]);exit(1);}
	}}

	/********************************************************************************/
	/* BUILD THE PERIODOGRAM */
	/********************************************************************************/
	xf_lombscargle(xdat,ydat,freq,output,(int)nn,nfreq);
	/* scale the results : seems this may be needed to avoid inflating the values by having dense histograms as an input (ie. when nfreq is large) */
	for(ii=0;ii<nfreq;ii++) {
		aa= output[ii]/(n2/nfreq);
		output[ii]= aa*aa;
	}
	/* apply smoothing  */
	if(setgauss!=0) xf_smoothgauss1_d(output,(size_t)nfreq,setgauss);


	/********************************************************************************/
	/* CALCULATE THE PROBABILITY AT EACH FREQUENCY AND TOTAL IN THE PERIODOGRAM */
	/********************************************************************************/
	for(ii=0;ii<nfreq;ii++) {
		aa= 1.0 - ((2.0*output[ii])/n2);
		bb= (n2-3.0)/2.0;
		cc= pow(aa,bb);
		dd= 1.0-cc;
		ee= 1.0-pow(dd,(double)nfreq);
		prob[ii]= ee;
	}
	thresh05= -1.0 * log( 1.0 - pow(0.95,(1.0/nfreq)) );

	/********************************************************************************/
	/* OUTPUT THE LOMB-SCARGLE PERIODOGRAM  */
	/********************************************************************************/
	if(setout==2) {

		if(setnorm!=-1) {
			x= xf_norm2_d(output,nfreq,setnorm);
			if(x==-1) { fprintf(stderr,"\b\n\t--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }
			if(x==-2) { fprintf(stderr,"\b\n\t--- Error [%s]: no valid numbers in histogram\n\n",thisprog); exit(1); }
		}

		for(ii=0;ii<nfreq;ii++) printf("%g	%f	%.4f\n",freq[ii],output[ii],prob[ii]);
	}

	/********************************************************************************/
	/* SUMMARY OUTPUT */
	/********************************************************************************/
	if(setout==1) {
		printf("nn= %ld\n",nn);
		printf("nfreq= %d\n",nfreq);
		printf("min_freq= %.16f\n",min);
		printf("max_freq= %.16f\n",max);
		printf("interval= %g\n",interval);
		printf("p=0.05\n");
		printf("thresh= %g\n",thresh05);
	}
	if(setverb==1) {
		fprintf(stderr,"nn= %ld\n",nn);
		fprintf(stderr,"nfreq= %d\n",nfreq);
		fprintf(stderr,"min_freq= %.16f\n",min);
		fprintf(stderr,"max_freq= %.16f\n",max);
		fprintf(stderr,"interval= %g\n",interval);
		fprintf(stderr,"p=0.05\n");
		fprintf(stderr,"thresh= %g\n",thresh05);
	}

	/********************************************************************************/
	/* CALCULATE THE POWER RATIO IN EACH BAND */
	/********************************************************************************/
	if(setindices!=NULL) {

		// calculate total area in Lomb-Scargle periodogram
		x= xf_auc2_d(freq,output,nfreq,0,result_d,message);
		if(x==0) bb= result_d[0];
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

		// calculate area in each zone
		for(zone=0;zone<nzones;zone++) {

			/* determine where the zone starts (ii) ends (jj) in samples */
			for(jj=ii=0;jj<nfreq;jj++) { if(freq[jj]<=start1[zone]) ii=jj; else if(freq[jj]>stop1[zone]) break; }
			/* calculate the size of the band (mm) in samples */
			jj--; mm= (jj-ii)+1;
			//TEST: fprintf(stderr,"ii=%ld	jj=%ld	mm=%ld\n",ii,jj,mm);
			/* calculate the AUC in the band */
			x= xf_auc2_d((freq+ii),(output+ii),(size_t)mm,0,result_d,message);
			if(x==0) aa= result_d[0];
			else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

			/* repeat to calculate the AUC in an equi-sized band just above the current band  */
			/* for bands near the upper end of the spectrum, adjust the comparison band */
			ii= jj;  if(ii>=(nfreq-1)) ii=(nfreq-2);
			jj+= (mm-1); if(jj>=nfreq) jj=(nfreq-1);
			/* calculate the size of the band (mm) in samples */
			mm= (jj-ii)+1;
			//TEST:fprintf(stderr,"ii=%ld	jj=%ld	mm=%ld\n",ii,jj,mm);
			/* calculate the AUC in the band */
			x= xf_auc2_d((freq+ii),(output+ii),(size_t)mm,0,result_d,message);
			if(x==0) aa= result_d[0];
			else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

			/* print the ratio to stderr */
			if(setverb==1) {
				fprintf(stderr,"zone_%ld= %g\n",zone,aa);
				fprintf(stderr,"ratio_%ld= %g\n",zone,(aa/(aa+bb)));
			}
			if(setout==1) {
				printf("zone_%ld= %g\n",zone,aa);
				printf("ratio_%ld= %g\n",zone,(aa/(aa+bb)));
			}
	}}


	if(xdat!=NULL) free(xdat);
	if(xdat2!=NULL) free(xdat2);
	if(ydat!=NULL) free(ydat);
	if(freq!=NULL) free(freq);
	if(prob!=NULL) free(prob);
	if(output!=NULL) free(output);
	if(index!=NULL) free(index);
	if(start1!=NULL) free(start1);
	if(stop1!=NULL) free(stop1);

	exit(0);
	}
