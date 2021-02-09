#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define thisprog "xe-ldas5-sliceEPSP"
#define TITLE_STRING thisprog" v 1: 6.September.2018 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/* <TAGS> SLICE file signal_processing filter </TAGS> */

/*

TO DO
- change estimation of artefact - time irrelevant, take RMS in unfiltered data between zero and max1?

v 1: 2.October.2018 [JRH]
	- bugfix: fEPSP minimum is now actually the minimum, as stated, not the first negative inflection
	- allow user to specify whether fibre-volley is based on the first or the last inflection between max1 and max2
	- include estimate of fEPSP quality (testing at present - RMS/mean might be best)


v 1: 6.September.2018 [JRH]
	- bugfix - corrected use of wrong range (nn vs nnodes2) for checking fibre-volley nodes

v 1: 2.September.2018 [JRH] - rethink of fibre-volley detection strategy
	1. do not require artefact detection to proceed with FV detection and fEPSP
		- note that stim artefact is quite variable on the electrode in stratum radiatum - detection occassionally fails
		- also note that up to this point A2 (artefact positivity) has been defined as the last positivity before max1
		- consequently, F2 must by definition fall after max1
	2. detect last positivity, OR LAST NEGATIVITY before max2
		- because sometimes there is only a negativity, and the peak of the fibre-volley comes late
		- using the mid-point of max2-max1 as the slope reference can result in the mid-point being near the actual peak
		- revising the algorithm this way ensures that the fibre volley will always include an actual negativity and positivity, one of which must fall between max1 and max2
	3. Impose 1.5*(max2-max1) limit on allowable fibre-volley duration

v 1: 30.August.2018 [JRH]
	- do not omit slope calculation based on issues with artefact amplitude relative to the POP spike
	- do not exit on slope-detection error/warning
	- switch to using half-time of fEPSP for calculating slope, rather than the less reliable half-amplitude (which used to fail for shallow slopes)
	- update instructions in xf_readwinltp1_f
	- update xf_geom_slope2_f to allow slopes to be ignored or to break the loop if they are negative or positive
	- update instructions

v 1: 8.May.2018 [JRH]
	- estimate "fibre-volley" size and time for slope calculation, even if there is no fibre-volley preset

v 1: 14.December.2017 [JRH]
	- three separate filter regimens for artefact, fibre-volley and fEPSP
	- slope is calculated using data filtered for fEPSP (data3)
		- this obliterates the fibre volley and other artefacts
		- however, the scope of the slope-search is defined according to the detection of artefact & fibre-volley using the other filter settings
	- requirements for stim-artefact reduced to a positivity before setmax1 (typically 1.5ms)

v 1: 13.December.2017 [JRH]
	- make data0 the original unfiltered data
	- create separate arrays for strict (data1) and gently (data2) filtered arrays
	- bugfix: EPSP slope parameters now derived from more strictly-filtered data
	- enable filtered trace output (-fout)

v 1: 2.November.2017 [JRH]
	- enable filtered trace output (-fout)
v 1: 2.November.2017 [JRH]
*/

/* external functions start */
long xf_readwinltp1_f(char *setinfile, char *setchan, float **data0, double *result_d, char *message);
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
long xf_interp3_f(float *data, long ndata);
int xf_stats2_f(float *data1, long nn, int setlarge, float *result_f);
double xf_mae1_f(float *data1, float *data2, long nn, char *message);
double xf_rms1_f(float *input, long nn, char *message);
double xf_samplefreq1_d(double *time1, long n1, char *message);
int xf_compare1_d(const void *a, const void *b);
int xf_filter_bworth1_f(float *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
long xf_detectinflect1_f(float *data,long n1,long **itime1,int **isign1,char *message);
long xf_geom_slope2_f(float *data,long n1,long winsize,double interval,int test,double *min,double *max,char *message);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char message[MAXLINELEN];
	long int ii,jj,kk,ll,mm,nn;
	int v,w,x,y,z;
	float result_f[16];
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char outfile1[64],outfile2[64];
	int *isign1=NULL,*isign2=NULL,*isign3=NULL;
	long zero1,nnodes1,nnodes2,nnodes3,*isamp1=NULL,*isamp2=NULL,*isamp3=NULL;
	long sampA1,sampA2,sampF1,sampF2,sampE1,sampE2,slopewin,fvmaxdur,ifirst,ilast;
	float *data0=NULL,*data1=NULL,*data2=NULL,*data3=NULL,*dataslope;
	double *itime1=NULL,*itime2=NULL,*itime3=NULL;
	double timeA1,timeA2,timeF1,timeF2,timeE1,timeE2;
	double valueA1,valueA2,valueF1,valueF2,valueE1,valueE2,errE1;
	double sampint,samprate,baseline,mae,mean,sd,rms,slopemin,slopemax;
	/* arguments */
	char *infile,*setchan;
	int setfiltout=2,setverb=0,setpos=2;
	double setfilthigh1=1500.0,setfilthigh2=1800.0,setfilthigh3=250.0,setmax1=1.25,setmax2=2.5,setmax3=15.0;
	/* define output file names */
	snprintf(outfile1,64,"temp_%s_trace.txt",thisprog);
	snprintf(outfile2,64,"temp_%s_nodes.txt",thisprog);

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Slice-ephys analysis: fibre-volley (FV) and fEPSP\n");
		fprintf(stderr,"- detect stim artefact (ART), FV & fEPSP with separate filters\n");
		fprintf(stderr,"- FV: \n");
		fprintf(stderr,"	- find neg or pos inflection between max1 and max2\n");
		fprintf(stderr,"	- find next pos or previous neg inflection, respectively\n");
		fprintf(stderr,"	- check that duration <= 1.5*(max2-max1)\n");
		fprintf(stderr,"	- otherwise use (max1+max2)/2 as FV start and stop time\n");
		fprintf(stderr,"- fEPSP slope: top half of line connecting FV and fEPSP minimum\n");
		fprintf(stderr,"	- looks for most negative slope in 0.5ms sliding windows\n");
		fprintf(stderr,"	- seeks from FV +ivity to fEPSP trough\n");
		fprintf(stderr,"	- if no FV, use middle of max1-to-max2 (see below)\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:  %s [input] [channel] [options]\n",thisprog);
		fprintf(stderr,"	[input]: WinLTP output filename or  \"stdin\"\n");
		fprintf(stderr,"	[channel]: channel to analyze- typically AD0 or AD1\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"  high-cut filter options (Hz)\n");
		fprintf(stderr,"	-high1 artefact filter [%g]\n",setfilthigh1);
		fprintf(stderr,"	-high2 fibre-volley & slope-detection filter [%g]\n",setfilthigh2);
		fprintf(stderr,"	-high3 fEPSP filter [%g]\n",setfilthigh3);
		fprintf(stderr,"  maximum-times (ms) for phenomena\n");
		fprintf(stderr,"	-max1  ART +ivity, also minimum for FV -ivity [%g]\n",setmax1);
		fprintf(stderr,"	-max2  FV -ivity [%g]\n",setmax2);
		fprintf(stderr,"	-max3  fEPSP trough [%g]\n",setmax3);
		fprintf(stderr,"  other options\n");
		fprintf(stderr,"	-pos: FV detected as first(1) or last(2) inflection [%d]\n",setpos);
		fprintf(stderr,"	-fout  output trace is filtered? (0=NO 1=high1, 2=high2, 3-high3) [%d]\n",setfiltout);
		fprintf(stderr,"	-verb  sets verbosity (0=simple, 1=verbose) [%d]\n",setverb);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s 63290358.P0 AD1 -verb 1\n",thisprog);
		fprintf(stderr,"	cat 63290358.P0 | %s stdin AD1\n",thisprog);
		fprintf(stderr,"\n");
		fprintf(stderr,"SCREEN OUTPUT:\n");
		fprintf(stderr,"	artmv	fvms	fvmv	epspms	epspmv	epspslope\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"FILE OUTPUT:\n");
		fprintf(stderr,"	%s\n",outfile1);
		fprintf(stderr,"		- trace in format <time> <voltage>\n");
		fprintf(stderr,"	%s\n",outfile2);
		fprintf(stderr,"		- fEPSP & fibre-volley nodes (used for plotting)\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	infile= argv[1];
	setchan= argv[2];
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-high1")==0)  setfilthigh1= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-high2")==0)  setfilthigh2= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-high3")==0)  setfilthigh3= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-max1")==0)   setmax1= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-max2")==0)   setmax2= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-max3")==0)   setmax3= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-pos")==0)    setpos= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)   setverb= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-fout")==0)   setfiltout= atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setpos!=1 && setpos!=2) { fprintf(stderr,"\n--- Error [%s]: invalid -pos [%d] must be 1 or 2\n\n",thisprog,setpos);exit(1);}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setmax1>=setmax2) { fprintf(stderr,"\n--- Error [%s]: -max1 [%g] must be < max2 [%g]\n\n",thisprog,setmax1,setmax2);exit(1);}
	if(setmax2>=setmax3) { fprintf(stderr,"\n--- Error [%s]: -max2 [%g] must be < max3 [%g]\n\n",thisprog,setmax2,setmax3);exit(1);}
	if(setfiltout<0 || setfiltout>3) { fprintf(stderr,"\n--- Error [%s]: invalid -fout [%d] must be 0,1 or 2\n\n",thisprog,setfiltout);exit(1);}


	/********************************************************************************/
	/* READ  THE WINLTP FILE  - STORE IN data0 */
	/********************************************************************************/
	nn= xf_readwinltp1_f(infile,setchan,&data0,result_d,message);
	if(nn<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	sampint= result_d[0];
	samprate= result_d[1];
	baseline= result_d[2];
	zero1= (long)(samprate * (baseline/1000.0));
	fvmaxdur= (long)(1.5*samprate*(setmax2-setmax1)/1000.0);

	if(setverb==1) {
		fprintf(stderr,"	infile= %s\n",infile);
		fprintf(stderr,"	total_samples= %ld\n",nn);
		fprintf(stderr,"	sample_interval= %g ms\n",sampint);
		fprintf(stderr,"	sample_rate= %g Hz\n",samprate);
		fprintf(stderr,"	baseline_period= %g ms\n",baseline);
		fprintf(stderr,"	setmax1= %g ms\n",setmax1);
		fprintf(stderr,"	setmax2= %g ms\n",setmax2);
		fprintf(stderr,"	setmax3= %g ms\n",setmax3);
		fprintf(stderr,"	highcut_filter1= %g Hz\n",setfilthigh1);
		fprintf(stderr,"	highcut_filter2= %g Hz\n",setfilthigh2);
		fprintf(stderr,"	highcut_filter3= %g Hz\n",setfilthigh3);
		fprintf(stderr,"	stimart_maxtime= %g ms\n",setmax1);
		fprintf(stderr,"	fibrevolley_maxtime= %g ms\n",setmax2);
		if(setpos==1) fprintf(stderr,"	fibrevolley_inflection= first\n");
		if(setpos==2) fprintf(stderr,"	fibrevolley_inflection= last\n");
		fprintf(stderr,"	fEPSP_maxtime= %g ms\n",setmax3);
	}

	/* INTERPOLATE */
	jj= xf_interp3_f(data0,nn);
	if(jj<0) { fprintf(stderr,"\b\n\t--- Error [%s]: no valid data in %s\n\n",thisprog,infile); exit(1); }

	/********************************************************************************/
	/* NORMALIZE TO THE MEAN OF THE BASELINE  */
	/********************************************************************************/
	aa= 0.0;
	for(ii=0;ii<zero1;ii++) aa+= data0[ii];
	mean= aa/(double)zero1;
	for(ii=0;ii<nn;ii++) data0[ii]-= mean;

	/********************************************************************************/
	/* MAKE COPIES OF THE DATA */
	/********************************************************************************/
	data1= malloc(nn*sizeof(*data1));
	data2= malloc(nn*sizeof(*data2));
	data3= malloc(nn*sizeof(*data3));
	if(data1==NULL) { fprintf(stderr,"\b\n\t--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }
	if(data2==NULL) { fprintf(stderr,"\b\n\t--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }
	if(data3==NULL) { fprintf(stderr,"\b\n\t--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }
	for(ii=0;ii<nn;ii++) data1[ii]= data2[ii]= data3[ii]= data0[ii];

	/********************************************************************************/
	/* APPLY FILTERING TO THE COPIES */
	/********************************************************************************/
	z= xf_filter_bworth1_f(data1,nn,samprate,0,setfilthigh1,1.412,message);
	if(z<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	z= xf_filter_bworth1_f(data2,nn,samprate,0,setfilthigh2,1.412,message);
	if(z<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	z= xf_filter_bworth1_f(data3,nn,samprate,0,setfilthigh3,1.412,message);
	if(z<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }


	/********************************************************************************
	DETECT INFLECTIONS (NODES)
	- only detect for times > zero (after stimulation)
	********************************************************************************/
	nnodes1= xf_detectinflect1_f((data1+zero1),(nn-zero1),&isamp1,&isign1,message);
	if(nnodes1<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	if(nnodes1<5) { fprintf(stderr,"--- Warning [%s]: fewer than 5 nodes in %s\n",thisprog,infile); }
	/* build an inflection-times array: milliseconds, relative to zero */
	itime1= malloc(nnodes1*sizeof(*itime1));
	if(itime1==NULL) { fprintf(stderr,"\b\n\t--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }
	for(ii=0;ii<nnodes1;ii++) itime1[ii]= (double)isamp1[ii]*sampint;
	/* correct the timestamps so they refer to the entire dataset, since detection started at "zero" */
	for(ii=0;ii<nnodes1;ii++) isamp1[ii]+=zero1;
	//TEST: for(ii=0;ii<nnodes1;ii++) fprintf(stderr,"node:%ld\tsamp:%ld\ttime:%.3f\n",ii,isamp1[ii],itime1[ii]);

	nnodes2= xf_detectinflect1_f((data2+zero1),(nn-zero1),&isamp2,&isign2,message);
	if(nnodes2<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	/* build an inflection-times array: milliseconds, relative to zero */
	itime2= malloc(nnodes2*sizeof(*itime2));
	if(itime2==NULL) { fprintf(stderr,"\b\n\t--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }
	for(ii=0;ii<nnodes2;ii++) itime2[ii]= (double)isamp2[ii]*sampint;
	/* correct the timestamps so they refer to the entire dataset, since detection started at "zero" */
	for(ii=0;ii<nnodes2;ii++) isamp2[ii]+= zero1;
	//TEST:for(ii=0;ii<nnodes1;ii++) fprintf(stderr,"node:%ld\tsamp:%ld\ttime:%.3f\n",ii,isamp2[ii],itime2[ii]);

	nnodes3= xf_detectinflect1_f((data3+zero1),(nn-zero1),&isamp3,&isign3,message);
	if(nnodes3<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	/* build an inflection-times array: milliseconds, relative to zero */
	itime3= malloc(nnodes3*sizeof(*itime3));
	if(itime3==NULL) { fprintf(stderr,"\b\n\t--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }
	for(ii=0;ii<nnodes3;ii++) itime3[ii]= (double)isamp3[ii]*sampint;
	/* correct the timestamps so they refer to the entire dataset, since detection started at "zero" */
	for(ii=0;ii<nnodes3;ii++) isamp3[ii]+= zero1;
	//TEST: for(ii=0;ii<nnodes3;ii++) fprintf(stderr,"node:%ld\tsamp:%ld\ttime:%.3f\n",ii,isamp3[ii],itime3[ii]);


	/********************************************************************************
	DETECT STIMULATION-ARTEFACT (A1,A2, <setmax1)
	- based on data1
	- look for last positivity before setmax1, then look back for preceding negativity
	- if the positivity is the first node, take the negativity as time zero
	********************************************************************************/
	/* INITIALIZE VARIABLES TO "INVALID" */
	sampA1=sampA2=sampF1=sampF2=-1;
	timeA1=NAN; timeA2=NAN;
	valueA1=NAN; valueA2=NAN;

	/*  DEFINE THE ARTEFACT START (A1, negative) AND STOP (A2, positive) */
	jj=0;
	for(ii=0;ii<nnodes1;ii++) {
		if(itime1[ii]<setmax1 && isign1[ii]>0) {
			/* set sample-numbers and times for (A)rtefact */
			if(ii>0) {
				timeA1= itime1[ii-1];
				sampA1= isamp1[ii-1];
			}
			/* if the positivity is the first inflection, take the negativity as time zero */
			else {
				timeA1= 0.0;
				sampA1= 0;
			}
			valueA1= data1[sampA1];
			timeA2= itime1[ii];
			sampA2= isamp1[ii];
			valueA2= data1[sampA2];
			jj++;
	}}
	if(jj==0) {
		fprintf(stderr,"--- Warning [%s]: no artefact positivity before %gms in %s\n",thisprog,setmax1,infile);

	}
	//TEST:	fprintf(stderr,"timeA1:%g sampA1:%ld valueA1:%g\n",timeA1,sampA1,valueA1);
	//TEST:	fprintf(stderr,"timeA2:%g sampA2:%ld valueA2:%g\n",timeA2,sampA2,valueA2);


	/********************************************************************************
	DETECT FIBRE-VOLLEY (F1,F2) BETWEEN MAX1 AND MAX2
	- based on data2
	- allow detection of multiple events as a precaution against really noisy data
	- detect last positivity in acceptable region, take preceding negativity as actual FV
	- define time and value for slope-reference (sampF2 valueF2)
	- if there is no fibre volley...
		- slope-reference value = mean of data2 values between setmax1 and setmax2
		- slope-reference time = mid-point between setmax1 and setmax2
	********************************************************************************/
	/* INITIALIZE VARIABLES TO "INVALID" */
	timeF1= timeF2= NAN;
	valueF1= valueF2= NAN;
	/* SCAN NODES FOR LAST POSITIVITY - FV POSITIVITY IS THE NEXT SAMPLE */
	// jj= inflection count, kk= sample-number
	//TEST:for(ii=1;ii<nnodes2;ii++) fprintf(stderr,"%g\n",itime2[ii]);
	jj= kk= 0;
	for(ii=1;ii<nnodes2;ii++) {
		if(isign2[ii]>0 && itime2[ii]>setmax1 && itime2[ii]<setmax2) {
			if(jj==0) ifirst= ii;
			kk= ii;
			jj++;
	}}
	/* if at least one positive inflection was detected... */
	if(jj>0) {
		if(setpos==1) kk= ifirst; /* move pointer to first or last inflection depending on options */
		if(jj>1) {
			if(setpos==1) fprintf(stderr,"--- Warning [%s]: %ld potential FVs in %s - taking the first %.2f-%.2f ms)\n",thisprog,jj,infile,itime2[kk-1],itime2[kk]);
			if(setpos==2) fprintf(stderr,"--- Warning [%s]: %ld potential FVs in %s - taking the last %.2f-%.2f ms)\n",thisprog,jj,infile,itime2[kk-1],itime2[kk]);
		}
		timeF1= itime2[kk-1];
		sampF1= isamp2[kk-1];
		valueF1= data2[sampF1];
		timeF2= itime2[kk];
		sampF2= isamp2[kk];
		valueF2= data2[sampF2];
	}
	/* ALTERNATIVELY, DETECT LAST NEGATIVE INFLECTION - FV POSITIVITY IS THE NEXT SAMPLE */
	else {
		mm= nnodes2-1;
		jj= kk= 0;
		for(ii=0;ii<mm;ii++) {
			if(isign2[ii]<0 && itime2[ii]>setmax1 && itime2[ii]<setmax2) {
				if(jj==0) ifirst= ii;
				kk= ii;
				jj++;
		}}
		if(jj>0) {
			if(setpos==1) kk= ifirst; /* move pointer to first or last inflection depending on options */
			if(jj>1) {
				if(setpos==1) fprintf(stderr,"--- Warning [%s]: %ld potential FVs in %s - taking the first %.2f-%.2f ms)\n",thisprog,jj,infile,itime2[kk-1],itime2[kk]);
				if(setpos==2) fprintf(stderr,"--- Warning [%s]: %ld potential FVs in %s - taking the last %.2f-%.2f ms)\n",thisprog,jj,infile,itime2[kk-1],itime2[kk]);
			}
			timeF1= itime2[kk];
			sampF1= isamp2[kk];
			valueF1= data2[sampF1];
			timeF2= itime2[kk+1];
			sampF2= isamp2[kk+1];
			valueF2= data2[sampF2];
		}
	}

	//TEST: fprintf(stderr,"actual %ld, max %ld\n",(sampF2-sampF1),(fvmaxdur));
	/* IF NEITHER A NEGATIVE NOR POSITIVE INFLECTION WAS DETECTED, IMPROVISE! */
	if(jj==0 || (sampF2-sampF1) > fvmaxdur) {
		fprintf(stderr,"--- Warning [%s]: no fibre-volley found between %gms and %gms in %s\n",thisprog,setmax1,setmax2,infile);
		aa= (setmax1+setmax2)/ 2000.0; // midpoint between max1 and max2, converted to seconds
		sampF1= sampF2= zero1 + (long)(aa * samprate); // convert midpoint to samples, add zero offset
		valueF1= NAN; // invalidate fibre-volley amplitude
		valueF2= data2[sampF2]; // store value for slope
		timeF1= timeF2= NAN;
	}
	//TEST:	fprintf(stderr,"timeF1:%g sampF1:%ld valueF1:%g\n",timeF1,sampF1,valueF1);
	//TEST:	fprintf(stderr,"timeF2:%g sampF2:%ld valueF2:%g\n",timeF2,sampF2,valueF2);



	/********************************************************************************
	DETECT FIELD-EPSP MINIMUM (E1), BETWEEN STETMAX2 AND SETMAX3
	- based on data3 (except for quality, which is based on data2)
	********************************************************************************/
	/* INITIALIZE VARIABLES TO "INVALID" */
	sampE1=sampE2= -1;
	timeE1=valueE1=errE1= NAN;

	/* CALCULATE THE QUALITY, AS MEAN-ABSOLUTE-ERROR (data2-data3) BETWEEN setmax2 AND setmax3 */
	ii= (long)(samprate * ((baseline+setmax2)/1000.0));
	jj= (long)(samprate * ((baseline+setmax3)/1000.0));
	kk= jj-ii;
	mae= xf_mae1_f((data2+ii),(data3+ii),kk,message);
	if(!isfinite(mae)) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

	/* SCAN FOR fEPSP MINIMUM (E1) BETWEEN SETMAX2 AND SETMAX3 - OUTSIDE RANGE OF FIBRE-VOLLEY */
	jj= kk= 0; // jj= counter, kk= node-number of minimum
	aa= DBL_MAX; // value of minimum
	for(ii=0;ii<nnodes3;ii++) {
		if(isign3[ii]==-1 && itime3[ii]>=setmax2 && itime3[ii]<=setmax3) {
			bb= data3[isamp3[ii]];
			if(bb<aa) { aa= bb; kk= ii; jj++; }
			break;
		}
	}
	if(jj>0) {
		timeE1= itime3[kk];
		sampE1= isamp3[kk];
		valueE1= data3[sampE1];
		errE1= mae;
	}
	else { fprintf(stderr,"--- Warning [%s]: no fEPSP minimum found between %gms and %gms in %s\n",thisprog,setmax2,setmax3,infile); }

	//TEST:	fprintf(stderr,"timeE1:%g sampE1:%ld valueE1:%g\n",timeE1,sampE1,valueE1);

	/********************************************************************************
	CALCULATE THE fEPSP SLOPE
	- use the highly filtered data (data3)
	- but feature times from more gently filtered data
	********************************************************************************/
	/* - note that scanning starts from sampF2 */
	timeE2=valueE2= NAN;
	slopemin= slopemax= NAN;
	if(sampE1>0) {
		if(valueE1>valueA2) fprintf(stderr,"--- Warning [%s]: fEPSP minimum is higher than artefact positivity in %s\n",thisprog,infile);
		/* set pointer to appropriately filtered data */
		dataslope= data2;
		/* define 0.5 ms sliding window to use for slope calculation */
		slopewin= (long)(0.5/sampint);
		/* determine the temporal mid-point between the FV positivity and the EPSP minimum */
		sampE2= (sampF2+sampE1)/2;
		timeE2= (sampE2-zero1)*sampint;
		valueE2= dataslope[sampE2];
		/* set the size of the slope-scan region */
		mm= (sampE1-sampF2)+1;
		/* analyze the slope */
		/* note: do not exit on error - for this function it just means the data is too short or contains invalid values */
		jj= xf_geom_slope2_f((dataslope+sampF2),mm,slopewin,sampint,0,&slopemin,&slopemax,message);
		if(jj<0) {
			fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message);
		}
	}

/* TEST
fprintf(stderr,"sampA1=%ld = %gms\n",sampA1,timeA1);
fprintf(stderr,"sampA2=%ld = %gms\n",sampA2,timeA2);
fprintf(stderr,"sampF1=%ld = %gms\n",sampF1,timeF1);
fprintf(stderr,"sampF2=%ld = %gms\n",sampF2,timeF2);
fprintf(stderr,"sampE1=%ld = %gms\n",sampE1,timeE1);
*/
	/********************************************************************************/
	/* OUTPUT THE DATA */
	/********************************************************************************/
	/* WRITE THE TRACE */
	if((fpout=fopen(outfile1,"w"))==NULL) { fprintf(stderr,"\b\n\t--- Error [%s]: could not open file %s for writing\n\n",thisprog,outfile1); exit(1); }
	jj= -zero1;
	fprintf(fpout,"ms	mV\n");
	if(setfiltout==0) for(ii=0;ii<nn;ii++) { aa= jj*sampint; fprintf(fpout,"%.3f\t%g\n",aa,data0[ii]); jj++; }
	if(setfiltout==1) for(ii=0;ii<nn;ii++) { aa= jj*sampint; fprintf(fpout,"%.3f\t%g\n",aa,data1[ii]); jj++; }
	if(setfiltout==2) for(ii=0;ii<nn;ii++) { aa= jj*sampint; fprintf(fpout,"%.3f\t%g\n",aa,data2[ii]); jj++; }
	if(setfiltout==3) for(ii=0;ii<nn;ii++) { aa= jj*sampint; fprintf(fpout,"%.3f\t%g\n",aa,data3[ii]); jj++; }
	fclose(fpout);
	/* WRITE THE NODES */
	if((fpout=fopen(outfile2,"w"))==NULL) { fprintf(stderr,"\b\n\t--- Error [%s]: could not open file %s for writing\n\n",thisprog,outfile2); exit(1); }
	fprintf(fpout,"node	colour	ms	mV\n");
	fprintf(fpout,"A1	1	%g	%g\n",timeA1,valueA1);
	fprintf(fpout,"A2	1	%g	%g\n",timeA2,valueA2);
	fprintf(fpout,"F1	2	%g	%g\n",timeF1,valueF1);
	fprintf(fpout,"F2	2	%g	%g\n",timeF2,valueF2);
	fprintf(fpout,"E1	3	%g	%g\n",timeE1,valueE1);
	fprintf(fpout,"E2	3	%g	%g\n",timeE2,valueE2);
	if(! isfinite(timeF2)) {
		fprintf(fpout,"Fest	10	%gzero1=	%g\n",((double)(sampF2-zero1)*sampint),valueF2);
	}
	fclose(fpout);

	/* WRITE THE SUMMARY */
	printf("artmv	fvms	fvmv	epspms	epspmv	epspslope	epsperr\n");
	printf("%.3f	%.3f	%.3f	%.3f	%.3f	%.3f	%.3f\n",valueA1,timeF1,valueF1,timeE1,valueE1,slopemin,errE1);


	/********************************************************************************/
	/* FREE MEMORY AND EXIT  */
	/********************************************************************************/
	if(data0!=NULL) free(data0);
	if(data1!=NULL) free(data1);
	if(data2!=NULL) free(data2);
	if(data3!=NULL) free(data3);
	if(isamp1!=NULL) free(isamp1);
	if(isamp2!=NULL) free(isamp2);
	if(isamp3!=NULL) free(isamp3);
	if(isign1!=NULL) free(isign1);
	if(isign2!=NULL) free(isign2);
	if(isign3!=NULL) free(isign3);
	if(itime1!=NULL) free(itime1);
	if(itime2!=NULL) free(itime2);
	if(itime3!=NULL) free(itime3);
	exit(0);
}
