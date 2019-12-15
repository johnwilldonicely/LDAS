#define thisprog "xe-ldas5-ripdet1"
#define TITLE_STRING thisprog" v 1: 6.March.2018 [JRH]"
#define MAXLINELEN 1000

#include <limits.h> /* to get maximum possible short value */
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "kiss_fftr.h"

/* <TAGS> signal_processing detect </TAGS> */

/*
REFERENCES:

Csicsvari et al., 1999
--------------------------------------------------------------------------------
For the extraction of
ripple waves, the wide-band recorded data were digitally band-pass
filtered (150-250 Hz; Fig. 1). The power (root mean square) of the
filtered signal was calculated for each electrode and summed across
electrodes to reduce variability. During SPW-ripple episodes, the power
substantially increased which enabled us to determine the beginning,
peak, and end of individual ripple episodes. The threshold for ripple
detection was set to 7 SDs above the background mean power. Epochs
with <4 SD above the background mean power were designated no-SPW
periods. Epochs with intermediate power were not included in the
analysis

Sullivan et al 2011
--------------------------------------------------------------------------------
(ripple= 140-220 Hz)
(fast gamma = 90-140 Hz)
(LFP sample rate 1250 Hz)

Detection of ripples and fast gamma oscillations. The procedures for
detection were based on those described previously (Csicsvari et al.,
1999b), but with additional refinements to minimize false-positive de-
tections and align detected oscillatory events to the peak of the CSD
source in the CA1 pyramidal layer.

First, the LFP signals (1250 samples/sec) from one or more selected channels from the CA1 pyramidal
layer were filtered between 50 and 250 Hz, rectified, smoothed with a three-sample (2.4ms) uniform
kernel, and then z-score normalized. Candidate oscillatory events (epochs during
which this normalized signal exceeded a 2 SD threshold) were detected first.
Each candidate event was aligned to the CA1 pyramidal CSD signal within a
~25 ms window around the midpoint of the detected suprathreshold epoch.
Because this alignment procedure can result in the same oscillatory event
being detected multiple times, we edited the realigned candidate oscillatory events
to enforce a minimum 50 ms spacing between events, deleting all but the event with
the highest CSD peak when multiple events occurred within a 50 ms time span. Spectral anal-
ysis using multitaper FFT on the wideband LFP and CSD signals (Mitra and Pesaran, 1999) (see
below, Multitaper FFT) was then performed on each remaining candidate oscillatory event to
remove false positives. We required that each oscillation in the final data set have a spectral
peak at any frequency between 120 and 200 Hz that is at least 2 SDs above the SWS background
for that frequency, for power spectra computed using both the LFP and CSD. The fre-
quency at the spectral peak of each remaining oscillatory event was then calculated. If this
peak frequency was under 140 Hz, we categorized the oscillatory event as "fast gamma" for
the purposes of our analysis; otherwise the event was categorized as a ripple.

Multitaper FFT. For spectral analysis of oscillatory patterns, we used a
modified version of the multitaper FFT MATLAB package by Mitra and
Pesaran (1999). Using an FFT window length of 100 ms, over frequencies
ranging 50-400 Hz, spectra of individual events contained power esti-
mates over 35 discrete frequency bins. Power spectra were z-score normal-
ized. These z-scored power measurements indicate the number of SDs by
which power in a given 100 ms window (e.g., a ripple oscillation) differs from
average background power during SWS. The normalizing means and SDs
used in calculating the z-score were derived by randomly triggering at 20,000
different points within slow-wave sleep, computing the FFT at each of
these points, and then calculating the mean and SD of power at each fre-
quency; this procedure was performed independently for every animal and
channel analyzed, for both CSD and LFP.

v 1: 6.March.2018 [JRH]
	- allow manual setting of event-detection thresholds (min and max Z-score)

v 1: 9.February.2018 [JRH]
	- bugfix: new function to reject SSPs AND peak values if they overlap block boundaries
	- previously, adjusting only the start and stop causeed peak-values to go out of alignment

v 1: 19.January.2018 [JRH]
	- update instructions to reflect ability to process .dat or simple binary files

v 1: 10.January.2017 [JRH]
	- bugfix: avoid adaptive spectral averaging if there is only one taper
	- bugfix: make sure waveform file is written to regardless of detection and before channel loop
	- make 2-taper (multitaper) default FFT analysis

v 1: 19.November.2016 [JRH]
	- change option for ripple ewaveform output to -outwave 0|1
	- this avoids having to call program twice to get stats + wave

v 1: 18.November.2016 [JRH]
	- fixed event alignment function

v 1: 6.November.2016 [JRH]
	- improve read capability, allowing use of start-stop-pairs to read chunks of data

v 1: 1.November.2016 [JRH]
	- add peak-frequency output

v 1: 26.October.2016 [JRH]
	- output information before printing tapers

v 1: 21.October.2016 [JRH]
	- restore ability to automatically determine taper order (ntaps+1)

v 1: 20.October.2016 [JRH]
	- allow output of tapers only
	- add midpoint output for ripple events

v 1: 19.October.2016 [JRH]
	- allow option to do simple (-avg 1) in addition to weighted (-avg 2) averaging of spectra

v 1: 18.October.2016 [JRH]
	- allow definition of data-type (-dt) float, in addition to short
	- correct output sample-numbers for effect of setting the start-sample
		- output should now correctly refer to sample-number of original input
		- this will allow multiple calls reading chunks of data, if required

v 1: 13.May.2016 [JRH]
	- built from xe-fftpow2


*/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
void *xf_readbin3_v(char *infile, off_t *params, off_t *start, off_t *toread, char *message);
int xf_interp4_f(float *data, long ndata, float invalid, int setfill, long *result);
int xf_filter_bworth1_f(float *X, off_t nn, float sample_freq, float setlow, float sethigh, float res, char *message);
int xf_smoothbox2_f(float *input, size_t nn, size_t nwin1, char *message);
int xf_norm2_f(float *data,long N,int normtype);
long xf_detectevents2_f(float *data,long n1,float thresh, float upper, float edge, int sign,long nmin,long nmax,long **estart, long **epeak, long **estop,char *message);
int xf_eventadjust1_f(float *data, long ndata, long *etime, long nevents, int sign, long shiftmax, char *message);
long xf_screen_ssp2(long *start1, long *stop1, long nssp1, long *start2, long *stop2, long *extra2, long nssp2, int mode, char *message);
double *xf_taperhann_d(long setn, int setmean, float setpow, char *message);
int xf_smoothgauss1_d(double *original, size_t arraysize,int smooth);
long xf_getindex1_d(double min, double max, long n, double value, char *message);
int xf_mtm_slepian1(int num_points, int setntapers, float npi, double *lam, double *tapers, double *tapsum);
int xf_mtm_spectavg1(double *sqr_spec,double *lambda, int nwin, int nfreq, double *ares, double *degf, double avar);
int xf_blockrealign2(long *samplenum, long nn, long *bstart, long *bstop, long nblocks, char *message);
// NOTE: the following function declarations are commented out to avoid re-initialization in kiss headers,
// They are included here only so xs-progcompile (which won't detect that they are commented out) will include them during compilation
/*
void kiss_fftr(kiss_fftr_cfg st,const kiss_fft_scalar *timedata,kiss_fft_cpx *freqdata);
void kiss_fft(kiss_fft_cfg cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout);
*/
/* external functions end */


int main(int argc, char *argv[]) {

	/* general-use variables */
	char outfile[256],line[MAXLINELEN],message[MAXLINELEN],*pline,*pcol;
	int q,r,s,t,x,y,z,stat=0;
	size_t sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	size_t ii,jj,kk,ll,mm,nn;
	float a,b,c,d,e,f;
	double aa,bb,cc,dd,ee,ff,gg,result_d[64];
	FILE *fpin,*fpout;

	// binary file read variables
	void *pdatvoid=NULL;
	short *pdatshort=NULL;
	float *pdatfloat=NULL;
	int chan,bestchan;
	long nssp;
	long *index=NULL,*start1=NULL,*stop1=NULL,*start2=NULL,*stop2=NULL;
	off_t startbyte,bytestoread,params[8];
	unsigned char *p0=NULL;
	unsigned short *p2=NULL;
	unsigned int *p4=NULL;
	unsigned long *p6=NULL;
	char *p1=NULL;
	short *p3=NULL;
	int *p5=NULL;
	long *p7=NULL;
	float *p8=NULL;
	double *p9=NULL;

	// pad filter & event-detect variables
	short *eventgood=NULL;
	long *estart=NULL,*epeak=NULL,*estop=NULL,tempstart,tempstop,tempmid;
	long event,enmin,enmax,nevents,goodevents,neventsmax,npad,result_l[8];
	float *aucrip=NULL;
	double *epeakval=NULL,*eventfreq=NULL,*eventamp=NULL,median,medianmax,noiselow;
	float eedge=0.5;

	/* FFT-variables */
	int datasize,chanstart,chanstop;
	size_t ffthalf,fftpart,blocksize,nbad=0,n2;
	size_t window,block;
	float *buff2=NULL,*data0=NULL,*data1=NULL,*data2=NULL,*freq=NULL,*pdata;
	float mean,sampint=0.0F,scaling1,scaling2;
	double *taper=NULL,*spect=NULL,*spectmean=NULL,ar,ai,freqres,spectpeak,fftrate,shiftsecs,sum,var;
	double *lambda=NULL,*tapsum=NULL,*degf=NULL;
	long indexriplow,indexriphigh,indexnoiselow,indexpeak,shiftfftsamps;

	/* arguments */
	char *infile=NULL,*setscreenfile=NULL,*setscreenlist=NULL,*setwavefile=NULL;
	int setbad=1,setnchan=16,setchan=-1,setout=4,setdatatype=3,setspectavg=2;
	int indexa=-1,indexb=-1,setnblock=-1,fftstep=1,setverb=0,setsmoothgauss=3;
	int setntaps=2,setmean=1,fftwin=-1;
	long setsmoothbox=-1; /// currently undefined - set for whole-file read
	float setsfreq=19531.25,fftmin=-1.0,fftmax=-1.0,setemin=2,setemax=20;
	double riplow=140.0,riphigh=220.0,setfiltlow=-1,setfilthigh=0,setfftwin=0.1,setorder=-1,setampmin=0.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"Detect ripple oscillations in a binary file (short or float accepted)\n");
		fprintf(stderr,"Based on methodology published by Sullivan & Buzsaki, 2011\n");
		fprintf(stderr,"	A. read, extract the data for each channel, and interpolate\n");
		fprintf(stderr,"	B. copy1 (detection): \n");
		fprintf(stderr,"	 	- band-pass filter in the defined ripple range\n");
		fprintf(stderr,"	 	- rectify, smooth,  and convert to Z-scores\n");
		fprintf(stderr,"		- detect events >%g SD (edge=%g SD) lasting 25-150ms\n",setemin,eedge);
		fprintf(stderr,"	C. copy2 (validation): \n");
		fprintf(stderr,"	 	- high-pass filter (defined by -low and -high)\n");
		fprintf(stderr,"	 	- extract a data chunk centred on each event\n");
		fprintf(stderr,"		- FFT on each chunk (100ms window, mode determined by -taps, below)\n");
		fprintf(stderr,"		- smooth the mean spectrum for each event (3-point Gaussian)\n");
		fprintf(stderr,"		- reject events with a peak outside ripple range \n");
		fprintf(stderr,"		- reject events with higher-freq. peaks > 0.5x the ripple peak\n");
		fprintf(stderr,"USAGE:	%s [input] [options] \n",thisprog);
		fprintf(stderr,"	[input]: binary input file\n");
		fprintf(stderr,"READ OPTIONS (defaults in [])\n");
		fprintf(stderr,"	-dt: type of data (3=short int, 8=float) [%d]\n",setdatatype);
		fprintf(stderr,"	-nch: total number of channels [%d]\n",setnchan);
		fprintf(stderr,"	-ch: specify the channel to detect on (-1 = ALL) [%d]\n",setchan);
		fprintf(stderr,"	-sf: sampling frequency (Hz) of the input [%.2f]\n",setsfreq);
		fprintf(stderr,"	-scrf: screen-file (binary SSP) defining read-chunks [unset]\n");
		fprintf(stderr,"	-scrl: screen-list (CSV) defining read-chunks [unset]\n");
		fprintf(stderr,"		NOTE: -scrf & -scrl define start/stop sample pairs\n");
		fprintf(stderr,"		NOTE: setting any stop to zero indicates read-to-end-of-file\n");
		fprintf(stderr,"DETECTION OPTIONS\n");
		fprintf(stderr,"	-riplow: ripple low-frequency boundary [%g]\n",riplow);
		fprintf(stderr,"	-riphigh: ripple low-frequency boundary [%g]\n",riphigh);
		fprintf(stderr,"	-b: boxcar smoother half-width (samples: -1= AUTO, 7.5ms) [%ld]\n",setsmoothbox);
		fprintf(stderr,"	-emin: threshold (Z-score) for detecting events [%g]\n",setemin);
		fprintf(stderr,"	-emax: max-allowable event size (Z-score) [%g]\n",setemax);
		fprintf(stderr,"	-ampmin: minimum amplitude (spectral AUC) for \"good\" ripples [%g]\n",setampmin);
		fprintf(stderr,"VALIDATION OPTIONS\n");
		fprintf(stderr,"	-low: filter low-cut filter  (0=none, -1= riplow/2, minimum=50) [%g]\n",setfiltlow);
		fprintf(stderr,"	-high: filter high-cut filter(0=none, -1= riphigh*2), maximum=sf/2 [%g]\n",setfilthigh);
		fprintf(stderr,"	-taps: number of tapers (0=none 1=Hann >1=multi-taper) [%d]\n",setntaps);
		fprintf(stderr,"		0-1: 6x 100ms sliding windows estimate ripple spectrum\n");
		fprintf(stderr,"		>=2: Slepian tapers in a fixed 100ms window (mid-event-centred)\n");
		fprintf(stderr,"	-ord: taper order (rate of change to zero) (-1 = auto = taps+1) [%g]\n",setorder);
		fprintf(stderr,"	-win: size of FFT-window (seconds) - will be adjusted [%g]\n",setfftwin);
		fprintf(stderr,"	-min: min FFT freq. [-1= default= sf/windowsize]\n");
		fprintf(stderr,"	-max: max FFT freq. [-1= default= sf/2]\n");
		fprintf(stderr,"	-avg: method for spectral averaging (1=arithmetic, 2=adaptive) [%d]\n",setspectavg);
		fprintf(stderr,"	-g: half-width (samples) for Gaussian smoothing of spectrum only [%d]\n",setsmoothgauss);
		fprintf(stderr,"		NOTE: -g must be 0 (none) or an odd number >=3)\n");
		fprintf(stderr,"	-out: output [%d]\n",setout);
		fprintf(stderr,"		0: tapers only\n");
		fprintf(stderr,"		1,2: spectra for all(1) or good(2) events (event,freq,amp)\n");
		fprintf(stderr,"		3,4: times for all(3) or good(4) events (event,start,peak,stop,amp)\n");
		fprintf(stderr,"	-wave : file to write ripple waveforms to (NULL=omit) [%s]\n",setwavefile);
		fprintf(stderr,"	-v: set verbosity to quiet (0) or report (1) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES: %s [input] [options] \n",thisprog);
		fprintf(stderr,"	%s data.txt -t bin -sf 1500 \n",thisprog);
		fprintf(stderr,"	cat data.bin | %s stdin -t asc \n",thisprog);
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-dt")==0)   setdatatype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-nch")==0)  setnchan=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-ch")==0)   setchan=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-sf")==0)   setsfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-scrf")==0) setscreenfile=argv[++ii];
			else if(strcmp(argv[ii],"-scrl")==0) setscreenlist=argv[++ii];
			else if(strcmp(argv[ii],"-emin")==0)  setemin=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-emax")==0)  setemax=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-ampmin")==0)  setampmin=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-riplow")==0)  riplow=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-riphigh")==0) riphigh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-low")==0)  setfiltlow=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-high")==0) setfilthigh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-taps")==0) setntaps=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-ord")==0)  setorder=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-min")==0)  fftmin=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-max")==0)  fftmax=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-avg")==0)  setspectavg=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-win")==0)  setfftwin=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-b")==0)    setsmoothbox=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-g")==0)    setsmoothgauss=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)  setout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-wave")==0) setwavefile=argv[++ii];
			else if(strcmp(argv[ii],"-v")==0)    setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

 	if(setverb>0) {	fprintf(stderr,"\n"); }

	/* CHECK OPTIONAL ARGUMENTS */
	if(setsfreq>0.0) sampint=1.0F/setsfreq;
	else { fprintf(stderr,"\n--- Error [%s]: bad sample freq. (%g) must be >0\n\n",thisprog,setsfreq);exit(1);}
	if((setsmoothgauss<3&&setsmoothgauss!=0)||(setsmoothgauss%2==0 && setsmoothgauss!=0)) { fprintf(stderr,"\n--- Error [%s]: -g [%d] must be 0 or an odd number 3 or larger\n\n",thisprog,setsmoothgauss);exit(1);}
	if(setntaps<0) { fprintf(stderr,"\n--- Error [%s]: -taps [%d] must be >=0\n\n",thisprog,setntaps);exit(1);}
	if(setorder<0 && setorder!=-1) { fprintf(stderr,"\n--- Error [%s]: -ord [%d] must be -1 or >0\n\n",thisprog,setorder);exit(1);}
	if(fftstep<1) { fprintf(stderr,"\n--- Error [%s]: -s [%d] must be >0\n\n",thisprog,fftstep);exit(1);}
	if(setchan>setnchan) { fprintf(stderr,"\n--- Error [%s]: -ch [%d] must be <= -nch (%d)\n\n",thisprog,setchan,setnchan);exit(1);}
	if(setdatatype!=3&&setdatatype!=8) { fprintf(stderr,"\n--- Error [%s]: -dt [%d] must be 3 or 8\n\n",thisprog,setdatatype);exit(1);}
	if(setspectavg!=1&&setspectavg!=2) { fprintf(stderr,"\n--- Error [%s]: -avg [%d] must be 1 or 2\n\n",thisprog,setspectavg);exit(1);}
	if(setscreenlist!=NULL && setscreenfile!=NULL) {fprintf(stderr,"\n--- Error[%s]: cannot define both a screening file (-scrf) and a screening list (-scrl)\n\n",thisprog);exit(1);}
	if(setout<0||setout>4) { fprintf(stderr,"\n--- Error [%s]: -out [%d] must be 0-4\n\n",thisprog,setout);exit(1);}
	if(setverb!=0&&setverb!=1) { fprintf(stderr,"\n--- Error [%s]: -v [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}

	if(setdatatype==3) datasize=sizeof(short);
	if(setdatatype==8) datasize=sizeof(float);
	if(setorder==-1) setorder=setntaps+1;

	/* define the invalid read-value */
	if(setbad==-1) setbad=SHRT_MIN;
	if(setbad==1)  setbad=SHRT_MAX;

	enmin=(long)(setsfreq*0.025); // 25ms minimum event duration
	enmax=(long)(setsfreq*0.150); // 150ms maximum event duration

	noiselow=(riphigh+((riphigh-riplow)/2.0)); // half-ripple-bandwidth above ripple high cutoff

 	if(setfiltlow<0)  { setfiltlow=  riplow/2.0;   if(setfiltlow<50) setfiltlow=50; }
 	if(setfilthigh<0) { setfilthigh= riphigh*2.0 ; if(setfilthigh>=setsfreq) setfilthigh= (setsfreq/2.0)-1.0; }

	// set smoothing window to 15ms (not the 2.4ms used by Buzsaki) - 1.5x the wavelength for a 100Hz oscillation
	if(setsmoothbox<0) setsmoothbox=(long)(0.0075*setsfreq);

	/************************************************************/
	/* READ THE INCLUDE OR EXCLUDE LIST */
	/************************************************************/
	if(setscreenlist!=NULL) {
		index= xf_lineparse2(setscreenlist,",",&nssp);
		if((nssp%2)!=0) {fprintf(stderr,"\n--- Error[%s]: screen-list does not contain pairs of numbers: %s\n\n",thisprog,setscreenlist);exit(1);}
		nssp/=2;
		if((start1=(long *)realloc(start1,nssp*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((stop1=(long *)realloc(stop1,nssp*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<nssp;ii++) {
			start1[ii]=atol(setscreenlist+index[2*ii]);
			stop1[ii]= atol(setscreenlist+index[2*ii+1]);
		}
		if(setverb==1) fprintf(stderr,"\tread %ld screening pairs\n",nssp);
		//convert stop1 array to n
		for(ii=0;ii<nssp;ii++) stop1[ii]-=start1[ii];
	}
	/************************************************************/
	/* ..OR, READ THE INCLUDE OR EXCLUDE FILE */
	/************************************************************/
	else if(setscreenfile!=NULL) {
		nssp = xf_readssp1(setscreenfile,&start1,&stop1,message);
		if(nssp==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		if(setverb==1) fprintf(stderr,"\tread %ld screening pairs\n",nssp);
		//convert stop1 array to n
		for(ii=0;ii<nssp;ii++) stop1[ii]-=start1[ii];
	}
	/************************************************************/
	/* ..OR, USE -s AND -n PARAMERTERS */
	/************************************************************/
	else {
		nssp=1;
		if((start1=(long *)realloc(start1,nssp*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((stop1=(long *)realloc(stop1,nssp*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		start1[0]=0;
		stop1[0]=0;
	}
	//TEST: for(ii=0;ii<nssp;ii++) printf("%ld	%ld	%ld\n",ii,start1[ii],stop1[ii]);exit(0);



	/*********************************************************************************************************/
	/*********************************************************************************************************/
	/* READ THE BINARY FILE (.DAT or .bin) AND ASSIGN MEMORY FOR PER-CHANNEL DATA
	/*********************************************************************************************************/
	/*********************************************************************************************************/
	if(setverb>0) {	fprintf(stderr,"\treading %s...\n",infile); }
	params[0]=datasize*setnchan; // account for number of channels to properly check file validty
	params[1]=0; // headerbytes - assume there is no header
	params[2]=nssp; // a single block if no screen-list or screen-file were defined
	params[3]=0; // initialize number of values read

	/* read the data */
 	pdatvoid= xf_readbin3_v(infile,params,start1,stop1,message);
 	if(pdatvoid==NULL) { fprintf(stderr,"\n\t--- Error[%s/%s]\n\n",thisprog,message); exit(1); }
 	if(params[3]<1) {fprintf(stderr,"\n\t--- Error [%s]: file %s is empty\n",thisprog,infile);exit(1);}
 	else nn= params[3];
	mm=nn*setnchan; // total datapoints, to simply data-copy loop below

	/* restore stop1 (read function will have now adjusted any zeros to total multi-channel items read)  */
	for(ii=0;ii<nssp;ii++) stop1[ii]+=start1[ii];

	if(setverb>0) {
		fprintf(stderr,"\t\tn=%ld\n",nn,setnchan);
		fprintf(stderr,"\t\tchannels=%ld\n",setnchan);
		fprintf(stderr,"\t\ttotal_samples=%ld\n",mm);
		fprintf(stderr,"\t\tduration=%.2f minutes\n",(((double)nn/setsfreq)/60.0));
	}

	/* setup pointer to the input */
	if(setdatatype==3) pdatshort=(short *)pdatvoid;
	if(setdatatype==8) pdatfloat=(float *)pdatvoid;

	/* ALLOCATE MEMORY FOR THE PER-CHANNEL DATA AND A COPY OF THE DATA : ALLOW FOR PADDING */
	npad=(long)setsfreq; // 1-second padding
	data0=(float *)realloc(data0,(nn+(npad*2))*sizeof(float)); if(data0==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	data1=(float *)realloc(data1,(nn+(npad*2))*sizeof(float)); if(data1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	data2=(float *)realloc(data2,(nn+(npad*2))*sizeof(float)); if(data2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/* ALLOCATE MEMORY FOR READ-BLOCK BOUNDARIES IN DATA WHICH IS NOW IN MEMORY */
	start2=(long *)realloc(start2,nssp*sizeof(long)); if(start2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	stop2=(long *)realloc(stop2,nssp*sizeof(long)); if(stop2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/* BUILD SSP LIST REPRESENTING READ-BLOCK BOUNDARIES IN DATA */
	/* this will be used to reject events spanning read-blocks */
	jj=0; for(ii=0;ii<nssp;ii++) { start2[ii]=jj; jj+=(stop1[ii]-start1[ii]); stop2[ii]=jj; }


	/*********************************************************************************************************/
	/*********************************************************************************************************/
	/* SETUP FFT VARIABLES */
	/*********************************************************************************************************/
	/*********************************************************************************************************/
	if(setntaps<=1){
		fftstep=10;
		fftpart= (long)( 0.5 + (setsfreq * setfftwin)/(double)fftstep ) ; // samples spanning an FFT step (default is 100ms window / fftstep )
		fftwin=fftstep*fftpart; // final window-size - this ensures that fftwin is a multiple of fftstep
	}
	else {
		fftstep=1;
		fftwin= (long)( 0.5 + (setsfreq * setfftwin) ) ; // samples spanning a window
		if(fftwin%2!=0) fftwin++;
		fftpart=fftwin;
	}

	if(fftwin>nn) {
		fftwin= 2*(long)((double)nn/4.0); // ensures fftwin is an even number <= 1/2 the data length
		fftstep=2.0;
		fftpart=fftwin/2;
	}
	ffthalf=fftwin/2; // defines highest index in FFT result for which unique information can be obtained
	if(fftmin<0) fftmin= setsfreq/fftwin; // minimum possible FFT frequency
	if(fftmax<0) fftmax= setsfreq/2.0; // maximum possible (Nyquist)
	freqres= setsfreq/(double)fftwin; // set the frequency resolution of the output
	fftrate= setsfreq/(double)fftpart; // temporal resolution of the FFT output
	shiftsecs= (double)(fftwin/setsfreq)/2.0;  // amount of shift (time) induced by using windowed FFT to analyze the power
	shiftfftsamps= (long)(shiftsecs*fftrate);  // the shift converted to FFT-windows in the output - ie. padding needed to be applied to align events
	/* apply a resolution-correction to the minimum & maximum frequencies */
	a=fftmin; if(a<freqres) fftmin=freqres; else if(a>freqres) fftmin=freqres*(int)((double)a/(double)freqres);
	a=fftmax; if(fmod(a,freqres)!=0) fftmax=freqres*(int)((double)a/(double)freqres);
	/* define scaling factors - size of data sent to FFT */
	scaling1=1.0/(float)fftwin; /* defining this way permits multiplication instead of (slower) division */
	scaling2=2.0/(float)fftwin;

	/*********************************************************************************************************/
	/*********************************************************************************************************/
	/* GENERATE THE TAPERS */
	/*********************************************************************************************************/
	/*********************************************************************************************************/
	if(setntaps<=0) {
		setntaps=1;
		/* create a dummy taper */
		if((taper= (double*)malloc(setntaps*fftwin*sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds the tapers
		for(ii=0;ii<fftwin;ii++) taper[ii]=1;
	}
	else if(setntaps==1) {
		/* create a modified hann taper */
		taper= xf_taperhann_d(fftwin,1,1,message);
		if(taper==NULL) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	}
	else {
		if((lambda= (double*)malloc(setntaps*sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds the vector of eigenvalues for taper calculation and adaptive spectrum estimation
		if((taper= (double*)calloc(setntaps*fftwin,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds the tapers
		if((tapsum= (double*)calloc(setntaps,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds the sum op the tapers (diagnostic?)
		// normally taporder = ntaps+1, to make the tails converge to zero, but for ripple detection this seems to cause problems with the spectra
		// consequently we opt for a fixed oreder
		// setorder=8
		xf_mtm_slepian1(fftwin,setntaps,setorder,lambda,taper,tapsum);
	}

	/*********************************************************************************************************/
	/*********************************************************************************************************/
	/* OUTPUT INFORMATION  */
	/*********************************************************************************************************/
	/*********************************************************************************************************/
	if(setverb==1) {
		aa= 100.0*(1.0-(1.0/fftstep));
		bb= setsfreq/(double)fftpart;
		fprintf(stderr,"\tFILTERS: riplow=%g  riphigh=%g  noiselow=%g\n",riplow,riphigh,noiselow);
		fprintf(stderr,"\tEVENT_SIZE: min=%ld  max=%ld\n",enmin,enmax);
		fprintf(stderr,"\tFFT: ntaps=%d  order=%g  winsize=%ld  overlap=%g%%  min=%g  max=%g\n",setntaps,setorder,fftwin,aa,fftmin,fftmax);
		fprintf(stderr,"\tFFT_OUT: rate=%gHz  shift=%gs shiftsamples=%ld\n",bb,shiftsecs,shiftfftsamps);
	}

	/********************************************************************************/
	/* OUTPUT TAPERS ONLY */
	/********************************************************************************/
	if(setout==0) {
		for(ii=0;ii<setntaps;ii++) {
			for(jj=0;jj<fftwin;jj++) {
				printf("%ld	%ld	%g\n",ii,jj,taper[ii*fftwin+jj]);
			}
		}
		exit(0);
	}

	/*********************************************************************************************************/
	/********************************************************************************/
	/* INITIALIZE KISS-FFT CONFIGURATION AND FFT-RELATED STORAGE  */
	/********************************************************************************/
	/*********************************************************************************************************/
	if(setverb>0) fprintf(stderr,"\tinitializing kiss-fft variables...\n");
	/* initialize kiss-fft variables: cfgr, fft */
	kiss_fftr_cfg cfgr = kiss_fftr_alloc( fftwin ,0,0,0 ); /* configuration structure: memory assigned using malloc - needs to be freed at end */
	kiss_fft_cpx fft[fftwin]; /* holds fft results: memory assigned explicitly, so does not need to be freed */
	/* allocate memory for working variables */
	if(setverb>0) fprintf(stderr,"\tallocating memory...\n");
	if((buff2= (float*)calloc(fftwin,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // buffer which is passed to the FFT function, copied from pdata
	if((spect= (double*)calloc((fftwin*setntaps),sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds amplitude of the FFT results
	if((spectmean= (double*)calloc((fftwin*setntaps),sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds per-block mean FFT results (power) from multiple buff2-s
	if((freq=(float*)calloc(fftwin,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds pre-caluclated frequencies associated with each FFT index
	if((degf= (double*)malloc((ffthalf)*sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};  // holds the degrees of freedom for F-statistics

	/* calculate the indices corresponding to the user-specified frequency range */
	/* this controls which elements of the fft results are calculated and output */
	indexa=(int)(fftmin*fftwin*sampint); if(indexa<1) indexa= 1;
	indexb=(int)(fftmax*fftwin*sampint); if(indexb>=(ffthalf)) indexb= ffthalf-1;

	/* calculate frequency associated with each FFT index */
	for(ii=indexa;ii<=indexb;ii++) freq[ii]= (float)( ((double)(ii)*(double)setsfreq) / (double)fftwin );

	/* define indices to the ripple and noise bands */
	aa=0.00; // 0 and not FFT min, because the spectmean-array actually starts at 0
	bb=(double)fftmax;
	kk=(long)(1+indexb-indexa);
	indexriplow= xf_getindex1_d(aa,bb,kk,riplow,message); if(freq[indexriplow]<riplow) indexriplow++;
 	indexriphigh= xf_getindex1_d(aa,bb,kk,riphigh,message); if(freq[indexriphigh]>riphigh) indexriphigh--;
 	indexnoiselow= xf_getindex1_d(aa,bb,kk,noiselow,message);

	/*********************************************************************************************************/
	/*********************************************************************************************************
	NOW FOR EACH CHANNEL....
	- pdatshort -> data0
	- data0: interpolate
	- data0 -> data2
	- data0: filter(ripple)
	- data0 -> data1
	- data2: filter (high-pass)
	- data1: rectified, smoothed, z-score, detect
	- data2: FFT tests
	- data0: waveform out
	- stats out
	*********************************************************************************************************/
	/*********************************************************************************************************/
	neventsmax=0;
	medianmax=0.0;
	bestchan=-1;
	if(setchan<0) { chanstart=0; chanstop=setnchan; }
	else { chanstart=setchan; chanstop=setchan+1; }

	if(setwavefile!=NULL) {
		if((fpout=fopen(setwavefile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: unable to write to file \"%s\"\n\n",thisprog,setwavefile);exit(1);}
		fprintf(fpout,"chan\tevent\tgood\ttime\tuV\n");
		fclose(fpout);
	}

	for(chan=chanstart;chan<chanstop;chan++) {

		/* COPY THE DATA, LEAVING SPACE FOR PADDING, CONVERTING TO FLOAT IF NEEDED  */
		if(setverb==1) fprintf(stderr,"\tcopying channel: %ld\n",chan);
		if(setdatatype==3) { for(ii=chan,jj=npad;ii<mm;ii+=setnchan) data0[jj++]=(float)pdatshort[ii]; }
		if(setdatatype==8) { for(ii=chan,jj=npad;ii<mm;ii+=setnchan) data0[jj++]=(float)pdatfloat[ii]; }

		/* INTERPOLATE, OMITTING THE SPACE LEFT FOR PADDING */
		if(setverb==1) fprintf(stderr,"\t\tinterpolating\n");
		z= xf_interp4_f((data0+npad),nn,(float)SHRT_MAX,3,result_l);
		if(z==1) {fprintf(stderr,"\n--- Error[%s]: no valid data in channel %dt\n\n",thisprog,chan);exit(1);};
		//TEST: for(ii=0;ii<nn;ii++) printf("%ld\t%g\t%g\n",chan,(ii/setsfreq),(data0+npad)[ii]); continue;

		/* ADD SAMPLE-AND-HOLD PADDING AT EITHER END (1-SECOND IS SUFFICIENT FOR THIS AND ALSO FOR FFT)  */
		if(setverb==1) fprintf(stderr,"\t\tpadding\n");
		for(ii=0;ii<npad;ii++) data0[ii]=data0[npad];
		jj=nn+npad-1;
		kk=nn+npad+npad;
		for(ii=nn+npad;ii<kk;ii++) data0[ii]=data0[jj];

		/* COPY DATA0 (USED FOR DETECTION) TO DATA2 (USED FOR FFT-VALIDATION)  */
		if(setverb==1) fprintf(stderr,"\t\tduplicate for FFT\n");
		for(ii=0;ii<nn+npad*2;ii++) data2[ii]=data0[ii];
		//TEST: for(ii=0;ii<nn;ii++) printf("%ld\t%g\t%g\n",chan,(ii/setsfreq),data2[ii]); data0-=npad; continue;

		//TEST: for(ii=0;ii<nn;ii++) printf("%ld\t%g\t%g\n",chan,(ii/setsfreq),data2[ii]); data0-=npad; continue;
		/* FILTER DATA0 STRICTLY IN THE RIPPLE FREQUENCY BAND */
		if(setverb==1) fprintf(stderr,"\t\tfiltering for event detection (%g-%g Hz)\n",riplow,riphigh);
		z= xf_filter_bworth1_f(data0,(nn+npad*2),setsfreq,riplow,riphigh,1.4142,message);
		if(z==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		//TEST: for(ii=0;ii<nn;ii++) printf("%ld\t%g\t%g\n",chan,(ii/setsfreq),(data0+npad)[ii]); continue;

		/* FILTER DATA2 WITH MORE RELAXED CRITERIA FOR ARTEFACT REJECTION  */
		if(setverb==1) fprintf(stderr,"\t\tfiltering for validation (%g-%g Hz)\n",setfiltlow,setfilthigh);
		z= xf_filter_bworth1_f(data2,(nn+npad*2),setsfreq,setfiltlow,setfilthigh,1.4142,message);
		if(z==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		//TEST: for(ii=0;ii<nn;ii++) printf("%ld\t%g\t%g\n",chan,(ii/setsfreq),(data0+npad)[ii]); continue;

		/* SHIFT THE POINTERS TO DATA0 & DATA2 FOR CONVENIENCE */
		data0+=npad; // for event detection
		data2+=npad; // for spectral analysis

		/* RECTIFY THE DATA0 SIGNAL, COPYING TO DATA1 FOR DETECTION FUNCTION  */
		if(setverb==1) fprintf(stderr,"\t\trectifying\n");
		for(ii=0;ii<nn;ii++) {
			if(data0[ii]<0) data1[ii]= (0.0-data0[ii]);
			else data1[ii]=data0[ii];
		}
		//TEST: for(ii=0;ii<nn;ii++) printf("%ld\t%g\t%g\n",chan,(ii/setsfreq),data1[ii]); exit(0);

		/* APPLY UNIFORM SMOOTHER TO DATA1 (BUZSAKI = 2.4MS, HERE DEFAULT IS 15MS (7.5MS HALF-WINDOW) */
		if(setverb==1) fprintf(stderr,"\t\tsmoothing: %ld samples\n",setsmoothbox);
		z= xf_smoothbox2_f(data1,nn,setsmoothbox,message);
		if(z==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		//TEST:for(ii=0;ii<nn;ii++) printf("%ld\t%g\t%g\n",chan,(ii/setsfreq),data1[ii]); exit(0);

		/* CONVERT DATA1 TO Z-SCORES */
		if(setverb==1) fprintf(stderr,"\t\tnormalizing\n");
		z= xf_norm2_f(data1,nn,1);
		if(z==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,"xf_norm2: memory allocation error"); exit(1); }
		if(z==-2) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,"xf_norm2: no finite numbers in data"); exit(1); }
		//TEST: for(ii=0;ii<nn;ii++) printf("%ld\t%g\t%g\n",chan,(ii/setsfreq),data1[ii]); exit(0);

		/* DETECT EVENTS IN DATA1 */
		if(setverb==1) fprintf(stderr,"\t\tdetecting events\n");
		nevents= xf_detectevents2_f(data1,nn,setemin,setemax,eedge,1,enmin,enmax,&estart,&epeak,&estop,message);
		if(nevents==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		//TEST:printf("nevents=%ld\n",nevents); for(ii=0;ii<nevents;ii++) printf("%ld\t%ld\t%ld\t%ld\n",ii,estart[ii],epeak[ii],estop[ii]); //data0-=npad; continue; exit(0);

		/* ADJUST EVENT PEAK TO ALIGN WITH NEAREST POSITIVE INFLECTION */
		/* note that alignment is to data0, which is filtered but not rectified or smoothed  */
		kk= (long)(setsfreq/riplow); // max acceptable shift = 1 ripple cycle (lowest frequency)
		z= xf_eventadjust1_f(data0,nn,epeak,nevents,1,kk,message);
		//TEST:printf("nevents=%ld\n",nevents); for(ii=0;ii<nevents;ii++) printf("%ld\t%ld\t%ld\t%ld\n",ii,estart[ii],epeak[ii],estop[ii]); //data0-=npad; continue;exit(0);

		/* REJECT EVENTS SPANNING READ-BLOCK BOUNDARIES - impossible to reconstitute the timestamps */
		nevents= xf_screen_ssp2(start2,stop2,nssp,estart,epeak,estop,nevents,1,message);
		if(nevents==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

		/* ALLOCATE & INITIALIZE MEMORY FOR STORING IF EACH EVENT IS GOOD OR NOT : FREE AT THE END OF EACH EVENT LOOP */
		if((eventgood= (short*)calloc(nevents,sizeof(short)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // buffer which is passed to the FFT function, copied from pdata
		if((eventfreq= (double*)calloc(nevents,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // buffer which is passed to the FFT function, copied from pdata
		if((eventamp= (double*)calloc(nevents,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // buffer which is passed to the FFT function, copied from pdata

		/*********************************************************************************************************/
		/* FOR EACH EVENT, SCREEN FOR FFT-PEAKS IN THE RIPPLE BAND */
		/*********************************************************************************************************/
		if(setverb==1) fprintf(stderr,"\t\trunning FFT on %ld events\n",nevents);
		if(setout==1||setout==2) printf("chan\tevent\tfreq\tamp\n");
		goodevents=0; // after FFT, this will mark the position of the last good event in the array
 		for(event=0;event<nevents;event++) {

			/* CALCULATE SPECTMEAN USING MULTI-TAPER-METHOD (MTM) */
			tempmid= (long)((double)(estop[event]+estart[event])/2.0); // mid-point of event
			tempstart= tempmid - 0.5*fftwin;
			tempstop=  tempmid + 0.5*fftwin;
			if(tempstart<0) { tempstart= 0; tempstop=(long)(1.0*fftwin); }
			if(tempstop>=nn) { tempstart= nn-(long)(1.0*fftwin); tempstop=nn; }

			/* set pointer to the data using the window-index */
			pdata=(data2+tempstart);
			/* calculate mean and variance for the window */
			sum=var=0.0; for(jj=0;jj<fftwin;jj++) { sum+=pdata[jj]; var+=(pdata[jj]*pdata[jj]); }
			mean=(float)(sum*scaling1);
			var/=(fftwin*fftwin);
			/* estimate spect[] for each taper */
			for(ii=0;ii<setntaps;ii++) {
				/* set index kk to current taper and spectral-result */
				kk=ii*fftwin;
				/* copy real data from pdata to buff2, and apply mean-correction + taper */
				for(jj=0;jj<fftwin;jj++) buff2[jj]= (pdata[jj]-mean) * taper[kk+jj];
				/* call the FFT function ***************/
				kiss_fftr(cfgr,buff2,fft);
				/* calculate the spectrum amplitude using the real (r) and imaginary (i) components of the FFT results */
				for(jj=0;jj<ffthalf;jj++) { ar= fft[jj].r; ai= fft[jj].i; spect[kk+jj]= (scaling2 * sqrtf( ar*ar + ai*ai )); }
			}

			/* AVERAGE THE SPECTRA */
			if(setspectavg==1 || setntaps<2) { /* simple mean */
				for(jj=0;jj<=ffthalf;jj++) spectmean[jj]=0.0;
				for(ii=0;ii<setntaps;ii++) {for(jj=0;jj<ffthalf;jj++) {spectmean[jj]+=spect[ii*fftwin+jj];}}
				for(jj=0;jj<=ffthalf;jj++) spectmean[jj]/=(float)setntaps;
			}
			else { /* adaptive weighted mean, assuming more than 1 taqper */
				z= xf_mtm_spectavg1(spect,lambda,setntaps,ffthalf,spectmean,degf,var);
				if(z>0 && setverb>0) fprintf(stderr,"	--- Warning [%s]:%d failed iterations\n",thisprog,z);
			}

			/* APPLY GAUSSIAN SMOOTHER TO SPECTRUM (REGION-OF-INTEREST ONLY) */
			z= xf_smoothgauss1_d(spectmean+indexa,(size_t)(indexb-indexa),setsmoothgauss);

			/*  GET EVENT STATISTICS AND DETERMINE IF IT'S GOOD */
			/* find the location of the spectral peak */
			jj=indexa; aa=spectmean[jj]; for(ii=jj;ii<=indexb;ii++) { if(spectmean[ii]>aa) {jj=ii; aa=spectmean[ii]; } }
			/* find the largest peak in the noise band (starts 1/2 ripple-band above ripple-high-threshold) */
			kk=indexnoiselow; bb=spectmean[kk]; for(ii=kk;ii<=indexb;ii++) { if(spectmean[ii]>bb) { kk=ii; bb=spectmean[ii]; } }
			/* assign principal frequency to event */
			eventfreq[event]=freq[jj];
			/* calculate amplitude - AUC in ripple band */
			eventamp[event]=0.0; for(ii=indexriplow;ii<=indexriphigh;ii++) eventamp[event]+=spectmean[ii];
			/* assign goodness to event - peak in ripple band and noise < 0.5 x ripple amplitude */
			if(jj>=indexriplow && jj<=indexriphigh && (bb/aa)<0.5)  {
				if(eventamp[event]>=setampmin) {
					eventgood[event]=1;
					goodevents++;
			}}

			/* OUTPUT SPECTRA */
			if(setout==1) {
				for(ii=indexa;ii<=indexb;ii++) printf("%ld\t%ld\t%g\t%g\n",chan,event,freq[ii],spectmean[ii]);
			}
			if(setout==2 && eventgood[event]==1) {
				for(ii=indexa;ii<=indexb;ii++) printf("%ld\t%ld\t%g\t%g\n",chan,event,freq[ii],spectmean[ii]);
			}


		} // END OF EVENTS LOOP


		/* OUTPUT THE RIPPLE WAVEFORMS (FILTERED TRACES) */
 		if(setwavefile!=NULL) {
			if((fpout=fopen(setwavefile,"a"))==0) {fprintf(stderr,"\n--- Error[%s]: unable to write to file \"%s\"\n\n",thisprog,setwavefile);exit(1);}
			aa=1000.0/setsfreq;
 			kk= (long)(0.5*fftwin);
			for(event=0;event<nevents;event++) {
	 			tempstart= epeak[event]-kk;
				for(jj=0;jj<=fftwin;jj++) {
					bb=((double)jj-(double)kk)*aa;
					fprintf(fpout,"%ld\t%ld\t%d\t%g\t%g\n",chan,event,eventgood[event],bb,data0[tempstart+jj]);
			}}
			fclose(fpout);
		}

		/* CORRECT THE EVENT START/PEAK/STOP TIMES */
		if(xf_blockrealign2(estart,nevents,start1,stop1,nssp,message)<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		if(xf_blockrealign2(epeak,nevents,start1,stop1,nssp,message)<0)  { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		if(xf_blockrealign2(estop,nevents,start1,stop1,nssp,message)<0)  { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

		/* OUTPUT RIPPLE STATS */
		if(setout==3 || setout==4) {
			printf("chan\tevent\tstart\tpeak\tstop\tmid\tfreq\tamp\n");
			for(ii=0;ii<nevents;ii++) {
				if(setout==4 && eventgood[ii]==0) continue;
				tempmid= (long)((double)(estop[ii]+estart[ii])/2.0); // mid-point of event
				printf("%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%g\t%g\n",
				chan,ii,estart[ii],epeak[ii],estop[ii],tempmid,eventfreq[ii],eventamp[ii]);

		}}

		if(setverb==1) fprintf(stderr,"\t\tevents passing criteria: %ld\n",goodevents);

		/* restore pointer to data0 */
		data0-=npad;
		data2-=npad;

		/* free eventgood array - size needs to be specific to nevents on each channel */
		free(eventgood);
		free(eventfreq);
		free(eventamp);


 	} /* END OF CHANNEL LOOP


	/********************************************************************************/
	/* FREE MEMORY AND EXIT */
	/********************************************************************************/

	if(start1!=NULL) free(start1);
 	if(stop1!=NULL) free(stop1);
	if(start2!=NULL) free(start2);
 	if(stop2!=NULL) free(stop2);
  	if(pdatshort!=NULL) free(pdatshort);
 	if(pdatfloat!=NULL) free(pdatfloat);
 	if(data0!=NULL) free(data0);
 	if(data1!=NULL) free(data1);
 	if(data2!=NULL) free(data2);
 	if(taper!=NULL) free(taper);
	if(tapsum!=NULL) free(tapsum);
	if(epeakval!=NULL) free(epeakval);
	if(buff2!=NULL) free(buff2);
	if(spect!=NULL) free(spect);
	if(spectmean!=NULL) free(spectmean);
	if(cfgr!=NULL) free(cfgr);
	if(freq!=NULL) free(freq);
	if(lambda!=NULL) free(lambda);
	if(degf!=NULL) free(degf);

	exit(0);

}
