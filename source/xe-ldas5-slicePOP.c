#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>

#define thisprog "xe-ldas5-slicePOP"
#define TITLE_STRING thisprog" v 1: 31.August.2018 [JRH]"

#define MAXLINELEN 1000
#define MAXLABELS 1000


/* <TAGS> SLICE file signal_processing filter </TAGS> */

/*
v 1: 31.August.2018 [JRH]
	- bugfix: POP-spike detection for negative inflections only, and compare against nnodes3 (not nnodes2)
v 1: 25.April.2018 [JRH]
	- correct amplitude output - should reflect the filtered data2, not the raw data1
v 1: 24.April.2018 [JRH]
	- summary now outputs popms and popmv (peak) instead of data for the bounding points for the POP SPIKE
	- fixed implicit declaration of string functions
v 1: 14.December.2017 [JRH]
	- remove warning associated with stim-artefact being wrong sign - this is not really important
	- update default filtering to 600Hz
v 1: 2.November.2017 [JRH]
	- change POP-spike detection based on the largest negativity between min1 and max1
	- enable filtered trace output (-fout)
v 1: 2.November.2017 [JRH]
	- new program: extraction of POP-spike parameters from slice electrophysiology data
*/

/* external functions start */
long xf_readwinltp1_f(char *setinfile, char *setchan, float **data1, double *result_d, char *message);
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
long xf_interp3_f(float *data, long ndata);
double xf_samplefreq1_d(double *time1, long n1, char *message);
int xf_compare1_d(const void *a, const void *b);
int xf_filter_bworth1_f(float *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
long xf_detectinflect1_f(float *data,long n1,long **itime1,int **isign1,char *message);
int xf_auc1_f(float *curvey, long nn, double interval, int ref, double *result ,char *message);
double xf_geom_offset1_d(double x1,double y1,double x2,double y2, double x3, double y3, char *message);
double xf_rms1_f(float *input, long nn, char *message);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char message[MAXLINELEN];
	long int ii,jj,kk,ll,mm,nn;
	int v,w,x,y,z;
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char outfile1[64],outfile2[64];
	int *isign1=NULL;
	long zero1,nnodes,nodemin,*isamp1=NULL,sampP1,sampP2,sampP3,filtstart,nfilt;
	float *data1=NULL,*data2=NULL;
	double *itime1=NULL,timeP1,timeP2,timeP3;
	double valueP1,valueP2,valueP3;
	double sampint,samprate,baseline,mean,auc,yoff,rms;
	/* arguments */
	char *infile,*setchan;
	int setfiltout=1,setverb=0;
	double setfilthigh1=500.0,setmin1=2.5,setmax1=15.0;

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
		fprintf(stderr,"Slice-electrophysiology analysis tool: POP-spike analysis\n");
		fprintf(stderr,"- calculate post-synaptic population-spike AUC and total RMS power\n");
		fprintf(stderr,"- POP-spike is the lowest-value between [min1] and [max1] (below)\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:  %s [input] [channel] [options]\n",thisprog);
		fprintf(stderr,"	[input]: WinLTP output filename or  \"stdin\"\n");
		fprintf(stderr,"	[channel]: channel to analyze- typically AD0 or AD1\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-high1 :  POP-spike node-detection high-cut filter (Hz) [%g]\n",setfilthigh1);
		fprintf(stderr,"	-min1  :  earliest time (ms) for pop-spike detection [%g]\n",setmin1);
		fprintf(stderr,"		- filtering starts at min1/2 to avoid stim-artefact\n");
		fprintf(stderr,"	-max1  :  latest time (ms) for pop-spike detection [%g]\n",setmax1);
		fprintf(stderr,"	-fout  :  output trace is filtered? (0=NO 1=YES) [%d]\n",setfiltout);
		fprintf(stderr,"	-verb  :  sets verbosity (0=simple, 1=verbose) [%d]\n",setverb);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s 63290358.P0 AD0 -verb 1\n",thisprog);
		fprintf(stderr,"	cat 63290358.P0 | %s stdin AD0\n",thisprog);
		fprintf(stderr,"\n");
		fprintf(stderr,"SCREEN OUTPUT:\n");
		fprintf(stderr,"	pop1ms	pop1mv	pop3ms	pop3mv	popauc	rms\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"FILE OUTPUT:\n");
		fprintf(stderr,"	%s\n",outfile1);
		fprintf(stderr,"		- trace in format <time> <voltage>\n");
		fprintf(stderr,"	%s\n",outfile2);
		fprintf(stderr,"		- POP-spike nodes (used for plotting)\n");
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
			else if(strcmp(argv[ii],"-min1")==0)   setmin1= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-max1")==0)   setmax1= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-fout")==0)   setfiltout= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)   setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setfiltout!=0 && setfiltout!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -fout [%d] must be 0 or 1\n\n",thisprog,setfiltout);exit(1);}

	/********************************************************************************/
	/* READ  THE WINLTP FILE  */
	/********************************************************************************/
	nn= xf_readwinltp1_f(infile,setchan,&data1,result_d,message);
	if(nn<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	sampint= result_d[0];
	samprate= result_d[1];
	baseline= result_d[2];
	zero1= samprate * (baseline/1000.0);
	/* filtering starts halfway to min1 */
	filtstart= zero1 + (long)(0.5 * samprate * (setmin1/1000.0));
	nfilt= nn-filtstart;

	if(setverb==1) {
		fprintf(stderr,"	infile= %s\n",infile);
		fprintf(stderr,"	total_samples= %ld\n",nn);
		fprintf(stderr,"	sample_interval= %g ms\n",sampint);
		fprintf(stderr,"	sample_rate= %g Hz\n",samprate);
		fprintf(stderr,"	baseline_period= %g ms\n",baseline);
		fprintf(stderr,"	highcut_filter1= %g Hz\n",setfilthigh1);
		fprintf(stderr,"	node1_mintime= %g ms\n",setmin1);
	}

	/* INTERPOLATE */
	jj= xf_interp3_f(data1,nn);
	if(jj<0) { fprintf(stderr,"\b\n\t--- Error [%s]: no valid data in %s\n\n",thisprog,infile); exit(1); }

	/********************************************************************************/
	/* NORMALIZE TO THE MEAN OF THE BASELINE  */
	/********************************************************************************/
	aa= 0.0;
	for(ii=0;ii<zero1;ii++) aa+= data1[ii];
	mean= aa/(double)zero1;
	for(ii=0;ii<nn;ii++) data1[ii]-= mean;

	/********************************************************************************/
	/* MAKE A COPY OF THE DATA  */
	/********************************************************************************/
	data2= malloc(nn*sizeof(*data2));
	if(data2==NULL) { fprintf(stderr,"\b\n\t--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }
	for(ii=0;ii<nn;ii++) data2[ii]= data1[ii];

	/********************************************************************************/
	/* APPLY FILTERING TO THE COPY - start halfway to min1 */
	/********************************************************************************/
	z= xf_filter_bworth1_f(data2+filtstart,nfilt,samprate,0,setfilthigh1,1.412,message);
	if(z<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

	/********************************************************************************
	DETECT INFLECTIONS (NODES) - only detect for times > zero (after stimulation)
	********************************************************************************/
	nnodes= xf_detectinflect1_f((data2+zero1),(nn-zero1),&isamp1,&isign1,message);
	if(nnodes<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	/* build an inflection-times array: milliseconds, relative to zero */
	itime1= malloc(nnodes*sizeof(*itime1));
	if(itime1==NULL) { fprintf(stderr,"\b\n\t--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }
	for(ii=0;ii<nnodes;ii++) itime1[ii]= (double)isamp1[ii]*sampint;
	/* correct the timestamps so they refer to the entire dataset, since detection started at "zero" */
	for(ii=0;ii<nnodes;ii++) isamp1[ii]+=zero1;
	//TEST:for(ii=0;ii<nnodes;ii++) printf("%d	%ld	%g	%g\n",isign1[ii],isamp1[ii],itime1[ii],data2[isamp1[ii]]);



	/********************************************************************************/
	/********************************************************************************/
	/* POP-SPIKE ANALYSIS */
	/********************************************************************************/
	/********************************************************************************/
	/*  run some checks */
	if(nnodes<5) { fprintf(stderr,"--- Warning [%s]: fewer than 5 nodes in %s\n",thisprog,infile); }
	if(setverb==1) fprintf(stderr,"	detected %ld inflections...\n",nnodes);
	/* initialize variables to "invalid" */
	sampP1=sampP2=sampP3= -1;
	timeP1=NAN; timeP2=NAN; timeP3=NAN;
	valueP1=NAN; valueP2=NAN; valueP3=NAN;
	auc=NAN; rms=NAN;
	/* find largest negative inflection between setmin1 and setmax1 */
	aa= DBL_MAX; // default minimum
	nodemin=-1; // node-number of minimum
	for(ii=0;ii<nnodes;ii++) {
		if(isign1[ii]==-1 && itime1[ii]>=setmin1 && itime1[ii]<=setmax1) {
			bb= data2[isamp1[ii]];
			/* is this the new minimum ? */
			if(bb<aa) {	aa=bb; nodemin=ii;}
		}
	}

	if(nodemin==-1) { fprintf(stderr,"--- Warning [%s]: no negative node found between %g and %g ms \n",thisprog,setmin1,setmax1); }
	else if(nodemin==0) { fprintf(stderr,"--- Warning [%s]: POP-spike detected at first inflection - AUC cannot be calculated\n",thisprog); }
	else if(nodemin==(nnodes-1)) { fprintf(stderr,"--- Warning [%s]: POP-spike detected at last inflection - AUC cannot be calculated\n",thisprog); }
	else {
			sampP1= isamp1[nodemin-1]; timeP1= itime1[nodemin-1]; valueP1= data2[sampP1];
			sampP2= isamp1[nodemin+0]; timeP2= itime1[nodemin+0]; valueP2= data2[sampP2];
			sampP3= isamp1[nodemin+1]; timeP3= itime1[nodemin+1]; valueP3= data2[sampP3];
	}

	if(sampP1>=0) {
		/********************************************************************************/
		/* GET THE POP-SPIKE AUC */
		/********************************************************************************/
		z= xf_auc1_f((data1+sampP1),((sampP3-sampP1)+1),sampint,1,result_d,message);
		if(z==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		auc=result_d[0];

		/********************************************************************************/
		/* GET THE POP-SPIKE RELATIVE-AMPLITUDE */
		/********************************************************************************/
		yoff= xf_geom_offset1_d(timeP1,valueP1,timeP3,valueP3,timeP2,valueP2,message);
		if(!isfinite(yoff)) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	}

	/********************************************************************************/
	/* CALCULATE THE POWER  */
	/********************************************************************************/
	rms= xf_rms1_f((data1+zero1),nn,message);
	if(!isfinite(rms)) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

	/********************************************************************************/
	/* OUTPUT THE DATA */
	/********************************************************************************/
	/* WRITE THE TRACE */
	if((fpout=fopen(outfile1,"w"))==NULL) { fprintf(stderr,"\b\n\t--- Error [%s]: could not open file %s for writing\n\n",thisprog,outfile1); exit(1); }
	jj= -zero1;
	fprintf(fpout,"ms	mV\n");
	if(setfiltout==0) for(ii=0;ii<nn;ii++) { aa= jj*sampint; fprintf(fpout,"%.3f\t%g\n",aa,data1[ii]); jj++; }
	if(setfiltout==1) for(ii=0;ii<nn;ii++) { aa= jj*sampint; fprintf(fpout,"%.3f\t%g\n",aa,data2[ii]); jj++; }
	fclose(fpout);
	/* WRITE THE NODES */
	if((fpout=fopen(outfile2,"w"))==NULL) { fprintf(stderr,"\b\n\t--- Error [%s]: could not open file %s for writing\n\n",thisprog,outfile2); exit(1); }
	fprintf(fpout,"node	colour	ms	mV\n");
	fprintf(fpout,"P1	3	%g	%g\n",timeP1,valueP1);
	fprintf(fpout,"P2	3	%g	%g\n",timeP2,valueP2);
	fprintf(fpout,"P3	3	%g	%g\n",timeP3,valueP3);
	fclose(fpout);

	/* OUTPUT THE SUMMARY */
	printf("popms	popmv	popauc	popamp	rms\n");
	printf("%.3f	%.3f	%.3f	%.3f	%.3f\n",timeP2,valueP2,auc,yoff,rms);



	/********************************************************************************/
	/* FREE MEMORY AND EXIT  */
	/********************************************************************************/
	if(data1!=NULL) free(data1);
	if(data2!=NULL) free(data2);
	if(isamp1!=NULL) free(isamp1);
	if(isign1!=NULL) free(isign1);
	if(itime1!=NULL) free(itime1);
	exit(0);
}
