#define thisprog "xe-ldas5-wavestats1"
#define TITLE_STRING thisprog" v 1: 24.March.2018 [JRH]"

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>

#define MAX_LINELEN 1000
#define MAX_CH 128


/* <TAGS> ldas spikes stats </TAGS> */

/*
CHANGES THIS VERSION:
v 1: 24.March.2018 [JRH]
	- add option to specify proportion-of-peak at which to calculate width (-thresh)

v 1: 7.July.2017 [JRH]
	- add spike-count to output

v 1: 19.June.2017 [JRH]
	- switch to using xf_wavepeak2, which assumes a fixed time-zero where the peak should be
	- this function also calculates the cross-channel peak minimum, maximum, difference and ratio
	- note that amplitude may be inaccurate if waveforms are somehow shifted (filtering effect in detection?)

v 1: 2.May.2017 [JRH]
	- check for error from xf_wavewidth1_f - if no good data, set output to NAN

v 1: 28.March.2017 [JRH]
	- xf_wavewidth2 bugfixes
		- search for min/max in waveform using FLT_MIN/FLT_MAX instead of zero - do NOT return error if min or max = 0
		- calling function was not checking for error anyway

v 1: 27.March.2017 [JRH]
	- xf_wavewidth2 bugfixes
		- ensure that x2 does not exceed the bounds for a given channel (causes issue below)
		- if voltage does not return to 25% of threshold, width is defined by the edge(s) for that channel

v 1: 17.March.2017 [JRH]
	- xf_wavewidth2 now ignores non-finite numbers
	- bugfix in xf_wavecor1_f: now uses the number of good samples, not presumed spike waveform length, for calculation of sum-of-squares
		- consequently, now properly corrects for bad channels (NAN)

v 1: 16.February.2017 [JRH]
	- bugfix: now outputs first waveform as well

v 1: 7.February.2017 [JRH]
	- new xf_wavecor1_f function for waveform corrleations
	- new xf_readwave1_f function
	- no need for all clusters 0-clumax to be represented in .wfm file

v 1: 15.March.2016 [JRH]
v 1.1: JRH, 24 March 2010
*/

//float *xf_readwave1_f(char *infile, long *resultl, double *resultd, char *message);

/* external functions start */
long xf_interp3_f(float *data, long ndata);
int xf_filter_bworth1_f(float *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
double xf_readwave1_f(char *infile, short **id, long **count, float **data, short **chanlist, long *resultl, char *message);
int xf_wavefilt1_f(float *wave, int nchan, int spklen, float setrate, float setlow, float sethigh, char *message);
long xf_wavepeak2_f(float *wave, long nchan, long spklen, long setzero, int sign, float *resultf, char *message);
int xf_wavewidth3_f(float *wave, int nchan, int spklen, int peakchan, int sign, double setthresh, float *resultf, char *message);

double xf_wavecor1_f(float *wave, long nchan, long spklen, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	// generic variables
	char line[MAX_LINELEN],message[MAX_LINELEN],*pline;
	char outfile[256],path_prog[256],path_data[256],temp_str[256];
	int w,x,y,z;
	long ii,jj,kk,nn,resultl[32];
	float a,b,c,d,resultf[32];
	double aa,bb,cc,resultd[32];
	FILE *fpin,*fpout;

	// program specific variables
	short *id=NULL,*chanlist=NULL;
	int pchan;
	long *count=NULL,clu,clumax,clutot;
	float *wave=NULL,*wavep0,*wavep1,*wavep2;
	float width, correlation=0,pmin,pmax,pdiff,pratio;
	long chan,nchan,chanstart,chanend,spklen,spkpre,wavelen;
	double srate,sratems,uv_per_unit,peak,zmax;

	// command-line variables
	char *infile;
	int setsign=-1;
	float setfiltlow=300.0,setfilthigh=5000.0;
	double setthresh=0.25;

	/*********************************************************************************************************/
	/* PRINT INSTRUCTIONS IF ONLY ONE ARGUMENT */
	/*********************************************************************************************************/
	if(argc<2) {
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate waveform statistics from a multi-channel .wfm file\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [.wfm file] [options]\n",thisprog);
		fprintf(stderr,"	[.wfm file]: file containing mean waveforms for each cluster\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-sign: peak detect sign (-1:negative 1:positive) [%d]\n",setsign);
		fprintf(stderr,"	-low: filter low-cut (0=none) [%g]\n",setfiltlow);
		fprintf(stderr,"	-high: filter high-cut (0=none) [%g]\n",setfilthigh);
		fprintf(stderr,"	-thresh: proportion-of-peak at which to calculate width [%g]\n",setthresh);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	[cluster] [width] [peak] [corr]\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"	cluster : cluster-ID\n");
		fprintf(stderr,"	n : number of spikes in cluster\n");
		fprintf(stderr,"	pchan: channel-ID containing the peak (depends on -sign)\n");
		fprintf(stderr,"	peak (uV) : amplitude in pchan at time zero\n");
		fprintf(stderr,"	width (ms): for max-peak channel, width at half-amplitude\n");
		fprintf(stderr,"	corr : mean Pearson's correlation for all good channel-pairs\n");
		fprintf(stderr,"	pmin : minimum value across channels at peak-time\n");
		fprintf(stderr,"	pmax : maximum value across channels at peak-time\n");
		fprintf(stderr,"	pdiff : the difference between pmax and pmin\n");
		fprintf(stderr,"	pratio: pmin/pmax (range 0-1, 1= identical values)\n");
		fprintf(stderr,"		- if pmin<0 & pmax<0, then the ratio is pmax/pmin\n");
		fprintf(stderr,"		- if pmin<0 & pmax>0, -1= max opposite values\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/*********************************************************************************************************/
	/* READ ARGUMENTS */
	/*********************************************************************************************************/
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if(ii>=argc) break;
			else if(strcmp(argv[ii],"-sign")==0)  setsign=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-low")==0)   setfiltlow=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-high")==0)  setfilthigh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-thresh")==0)  setthresh=atof(argv[++ii]);
			else {fprintf(stderr,"Error[%s]: invalid command line argument \"%s\"\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setthresh<0.0||setthresh>1.0) {fprintf(stderr,"Error[%s]: -thresh (%g) must be between 0 and 1\n",thisprog,setthresh); exit(1);}

	/*********************************************************************************************************/
	/* READ THE .WFM FILE TO GET NCHAN, NPROBES, AND THE CHANNEL LIST FOR EACH PROBE  */
	/*********************************************************************************************************/
	srate = xf_readwave1_f(infile,&id,&count,&wave,&chanlist,resultl,message);
	if(srate<0) {fprintf(stderr,"\nError[%s]: %s\n\n",thisprog,message); exit(1); }

	clutot=resultl[0];
	nchan=resultl[1];
	spklen=resultl[2];
	spkpre=resultl[3];
	wavelen=resultl[4];
	clumax=resultl[5];
	/* get the sample-rate, in millisenconds (samples per ms) */
	sratems= srate/1000.00;

	//TEST: for(ii=0;ii<clutot;ii++) fprintf(stderr,"%ld	%ld\n",id[ii],count[ii]); exit(0);
	//TEST: for(ii=0;ii<nchan;ii++) fprintf(stderr,"chan[%ld]=%ld\n",ii,chanlist[ii]); exit(0);

	printf("cluster	n	pchan	peak	width	corr	pmin	pmax	pdiff	pratio\n");
	for(ii=0;ii<clutot;ii++) {

		pchan=-1;
		width=peak=correlation=pmin=pmax=pdiff=pratio=NAN;

		/* create a pointer to the current cluster's waveform */
		wavep0= wave+(ii*wavelen);

		/* apply per-channel filtering */
		z= xf_wavefilt1_f(wavep0,nchan,spklen,srate,setfiltlow,setfilthigh,message);

		/* get the peak channel and peak value - note that this is channel in depth-order */
		/* also get the min/max values, difference and ratio */
		pchan= xf_wavepeak2_f(wavep0,nchan,spklen,spkpre,setsign,resultf,message);
		if(pchan<0) goto gtprint;
		peak=resultf[0];
		pmin=resultf[1];
		pmax=resultf[2];
		pdiff=resultf[3];
		pratio=resultf[4];

		/* calculate half-width of waveform on highest-amplitude chanel */
		x= xf_wavewidth3_f(wavep0,nchan,spklen,pchan,setsign,setthresh,resultf,message);
		if(x==0) width= resultf[0]/sratems; // width, converted to milliseconds
		else goto gtprint;

		/* calculate the mean multi-channel correlation (whole spike-waveform on each channel correlated with every other) */
		correlation= xf_wavecor1_f(wavep0,nchan,spklen,message);
		if(!isfinite(correlation)) goto gtprint;

		/* convert peak-channel to actual channel number */
		pchan=chanlist[pchan];
		/* print the output to stdout */
		gtprint:
		printf("%d	%ld	%d	%.3f	%.3f	%.3f	%.3f	%.3f	%.3f	%.3f\n",
			id[ii],count[ii],pchan,peak,width,correlation,pmin,pmax,pdiff,pratio);


	}


	/********************************************************************************
	FREE MEMORY AND EXIT
	********************************************************************************/
	if(wave!=NULL) free(wave);
	if(id!=NULL) free(id);
	if(chanlist!=NULL) free(chanlist);
	if(count!=NULL) free(count);

	exit(0);

}
