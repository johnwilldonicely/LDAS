#define thisprog "xe-spike2-sleep"
#define TITLE_STRING thisprog" 1.June.2021 [JRH]"
#define MAXLINELEN 1000
/*  define and include required (in this order!) for time functions */
#define __USE_XOPEN // required specifically for strptime()
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "kiss_fftr.h"
/*
<TAGS> SPIKE2 </TAGS>

1.June.2021 [JRH]
- Reads data exported from Spike2 recording using the following Spike2 scripts:
	- Activity (text)     : s2_export_act2txt.s2s - missing data = 999999
	- EMG (binary float)  : s2_export_eeg2bin.s2s - missing data = 999999
	- EEG (binary float)  : s2_export_emg2bin.s2s - missing data = 999999

- Activity
	- normally minimum is zero, but negative values occur due to filtering in Spike2
	- these negative values rarely exceed -1, and occur around "true" positive activity as a result of ringing in the filter
- EEG/EMG
	- -999 will represent missing data at the start and end of recording
	- if the data was collected with DSI transmitters, "0" may also represent missing data when the transmitter is out of range
	- these zeros are also potentially valid data and cannot be used to assume missing data
	- however, multiple consecutive values of exactly zero are highly unlikely
	-


*/



/* external functions start */
// NOTE: the following function declarations are commented out to avoid re-initialization in kiss headers,
/*
void kiss_fftr(kiss_fftr_cfg st,const kiss_fft_scalar *timedata,kiss_fft_cpx *freqdata);
void kiss_fft(kiss_fft_cfg cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout);
*/
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line, char *delimiters, long *nwords);
float *xf_readspike2_text_f(char *infile, long *ndata, double *sampint, char *message);
float *xf_readbin2_f(char *infile, off_t *parameters, char *message);
char* xf_strsub1 (char *source, char *str1, char *str2);
long xf_interp3_f(float *data, long ndata);

float *xf_bin_simple_f(float *data1, long n1, long binsize, int type, long *nbins, char *message);
float xf_percentile2_f(float *data, long nn, double setper, char *message);
int xf_percentile3_f(float *data, long nn, double setper, double *per1, double *per2, char *message);
int xf_compare1_d(const void *a, const void *b);

long xf_outlier1_f(float *data, long nn, float setlow, float sethigh, char *message);
int xf_norm2_f(float *data,long ndata,int normtype);




long *xf_definebands1(char *setbands,float *bstart1,float *bstop1, long *btot, char *messsage);
long xf_getindex1_d(double min, double max, long n, double value, char *message);
int xf_auc1_d(double *curvey, long nn, double interval, int ref, double *result ,char *message);
double *xf_taperhann_d(long setn, int setnorm, float setpow, char *message);
int xf_timeconv1(double seconds, int *days, int *hours, int *minutes, double *sec2);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL,message[MAXLINELEN];
	int x,y,z,vector[] = {1,2,3,4,5,6,7};
	long ii,jj,kk,nn,mm,maxlinelen=0;
	float a,b,c;
	double aa,bb,cc,resultd[16];
	FILE *fpin,*fpout;

	/* date and time variables */
	char timestring[256];
	time_t t1,t2;
	struct tm *tstruct1;
	int days,hours,minutes;
	double seconds;

	/* program-specific variables */
	char *header=NULL,*pchar=NULL;
	char *infileeeg=NULL,*infileemg=NULL,*infiletemp=NULL;
	int sizeofdata;
	long *iword=NULL,nwords,binsamps,zero1act,zero1emg,zero1eeg;
	long nwinfft,nnact,nnemg,nneeg,nscores;
	off_t parameters[8]; // parameters for xf_readbin2_f()
	float *datact=NULL,*datemg=NULL,*dateeg=NULL,*pdataf=NULL;
	float *scoreact=NULL,*scoreemg=NULL,*scoreeeg=NULL;
	double siact,sfact,duract,duremg,dureeg,sfemg=500.0,sfeeg=500.0;

	long fftmaxindex=-1,window,epoch,epochsamps,nepochs;
	double *taper=NULL,*spect=NULL,*spectmean=NULL,*spectmean2=NULL,ar,ai,fftmaxfreq=-1.0,freqres;
	float *buff2=NULL,*fftfreq=NULL,scaling1,sum,mean;

	// Sandor Kantor / Sleepsign sleep-detection band definitions
	// NOTE "traditional" sigma (10-15Hz) is subsumed by Sandor's "spindle-alpha" (6,15)
	// 	char setbandsdefault[]= "delta,0.6,4.5,spinalph,6,15,theta,6,10,beta,12,25,gamma,30,100";
	// Gyorgi Buzsaki band definitions
	// 	char setbandsdefault[]= "delta,2,4,theta,4,8,alpha,8,12,beta,12,20,gamma1,20,90,gamma2,90,13,ripple,130,160";
	// Functional bands for test dataset (TPEEG054):
	//	 char setbandsdefault[]= "delta 1,5,theta,5,10,sigma,10,18,beta,18,35,gamma,35,80"
	/* band definition */
	char setbandsdefault[]= "delta,0.6,4.5,spinalph,6,15,theta,6,10,beta,12,25,gamma,30,100";
	long *ibands=NULL,band,btot=0,bstart2[16],bstop2[16];
	float bstart1[16],bstop1[16];

	/* arguments */
	char *infileact=NULL,*setbands=NULL;
	int setverb=0,setout=1;
	double setzero=0.0,settrim=99.5,setepoch=10.0,setmissing=999999.0;

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Read Spike2 exported-data to perform sleep-stage analysis\n");
		fprintf(stderr,"- requires Activity, EMG, and EEG output - scripts from XTP library:\n");
		fprintf(stderr,"    - ACT:  s2_export_act2txt.s2s\n");
		fprintf(stderr,"    - EMG:  s2_export_eeg2bin.s2s\n");
		fprintf(stderr,"    - EEG:  s2_export_emg2bin.s2s\n");
		fprintf(stderr,"    - \n");
		fprintf(stderr,"USAGE: %s [in] [options]\n",thisprog);
		fprintf(stderr,"    [in]: filename for activity record\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"    -verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"    -bands: CSV band-triplets: name,start,stop\n");
		fprintf(stderr,"        - default: %s\n",setbandsdefault);
		fprintf(stderr,"    -trim: outlier definition for EMG & EEG (percentile) [%g]\n",settrim);
		fprintf(stderr,"    -out: output (1=epochs, 2=1s_scores [%d]\n",setout);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"    %s data.txt\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	- \n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infileact= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-bands")==0) setbands= argv[++ii];
			else if(strcmp(argv[ii],"-trim")==0) settrim= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0) setout= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb!=999) { fprintf(stderr,"\n--- Error[%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(strcmp(infileact,"stdin")==0) { fprintf(stderr,"\n--- Error[%s]: this program does not accept \"stdin\" as an input. Please specify a filename\n\n",thisprog);exit(1);}
	if(settrim<0||settrim>100) { fprintf(stderr,"\n--- Error[%s]: invalid -trim [%g] must be 0-100\n\n",thisprog,settrim);exit(1);}
	if(setout<0||setout>100) { fprintf(stderr,"\n--- Error[%s]: invalid -out [%d] must be 1-2\n\n",thisprog,setout);exit(1);}


// CHECK VALIDITY OF BAND NAMES  MUST BE Delta,Theta,Sigma,Beta,Gamma

	/********************************************************************************
	CHECK ACTIVITY FILENAME AND GENERATE FILENAMES FOR EEG AND EMG
	********************************************************************************/
	pchar= strstr(infileact,"ACT_"); // infileact must contain the word "activity.txt"
	if(pchar==NULL) { fprintf(stderr,"\n--- Error[%s]: invalid infileact [%s] - must include the keyword \"ACT_\"\n\n",thisprog,infileact);exit(1);}
	infiletemp= xf_strsub1(infileact,".txt",".bin");
	infileemg= xf_strsub1(infiletemp,"ACT","EMG");
	infileeeg= xf_strsub1(infiletemp,"ACT","EEG");
	fprintf(stderr,"\n");
	fprintf(stderr,"...activity file= %s\n",infileact);
	fprintf(stderr,"...matching EMG=  %s\n",infileemg);
	fprintf(stderr,"...matching EEG=  %s\n",infileeeg);


	/********************************************************************************
	PROCESS THE SETBANDS STRING - MAKE NEW LIST OF NAMES, STARTS, STOPS
	********************************************************************************/
	if(setbands==NULL) setbands= setbandsdefault;
	ibands= xf_definebands1(setbands,bstart1,bstop1,&btot,message);
	if(ibands==NULL) { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); exit(1); }
	//TEST:	for(ii=0;ii<btot;ii++) printf("%s\t%g\t%g\n",(setbands+ibands[ii]),bstart1[ii],bstop1[ii]); exit(0);

	/******************************************************************************/
	/******************************************************************************/
	/******************************************************************************/
	/* STEP 1: STORE DATA
	/******************************************************************************/
	/******************************************************************************/
	/******************************************************************************/


	/********************************************************************************
	A. STORE ACTIVITY DATA
	- probably collected at 1Hz
	- immobility is registered as zero
	- max mobility is probably ~6
	********************************************************************************/
	fprintf(stderr,"...reading ACTIVITY data...\n");
	datact= xf_readspike2_text_f(infileact,&nnact,&siact,message);
	if(datact==NULL) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
	sfact= 1.0/siact;
	if(sfact<1) { fprintf(stderr,"\n--- Error[%s]: activity sample-frequency (%g) is less than 1 - scores cannot be generated\n\n",thisprog,sfact);exit(1);}
	/* FIND DURATION N SECONDS AND REPORT */
	duract= (double)nnact/sfact;
	z= xf_timeconv1(duract,&days,&hours,&minutes,&seconds);
	fprintf(stderr,"        label= %s\n",message);
	fprintf(stderr,"        records= %ld\n",nnact);
	fprintf(stderr,"        samplerate= %g Hz\n",sfact);
	fprintf(stderr,"        duration=\033[0;32m %.3f\033[0m seconds (%02d:%02d:%02d:%.3f)\n",duract,days,hours,minutes,seconds);

	/********************************************************************************
	B. STORE EMG DATA
	********************************************************************************/
	fprintf(stderr,"...reading EMG data...\n");
	parameters[0]= 8; /// data-type
	parameters[1]= 0; // number of bytes at the top of the file (header) to ignore
	parameters[2]= 0; // number of numbers to skip (bytes skipped calculated based on size of data type)
	parameters[3]= 0; // number of numbers to be read (0=all)
	datemg= xf_readbin2_f(infileemg,parameters,message);
	if(datemg==NULL) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
	nnemg= parameters[3];
	/* FIND DURATION N SECONDS AND REPORT */
	duremg= (double)nnemg/sfemg;
	z= xf_timeconv1(duremg,&days,&hours,&minutes,&seconds);
	fprintf(stderr,"        records= %ld\n",nnemg);
	fprintf(stderr,"        samplerate= %g Hz\n",sfemg);
	fprintf(stderr,"        duration=\033[0;32m %.3f\033[0m seconds (%02d:%02d:%02d:%.3f)\n",duremg,days,hours,minutes,seconds);

	/********************************************************************************
	C.  STORE EEG DATA
	********************************************************************************/
	fprintf(stderr,"...reading EEG data...\n");
	parameters[0]= 8; /// data-type
	parameters[1]= 0; // number of bytes at the top of the file (header) to ignore
	parameters[2]= 0; // number of numbers to skip (bytes skipped calculated based on size of data type)
	parameters[3]= 0; // number of numbers to be read (0=all)
	dateeg= xf_readbin2_f(infileeeg,parameters,message);
	if(dateeg==NULL) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
	nneeg= parameters[3];
	/* FIND DURATION N SECONDS AND REPORT */
	dureeg= (double)nneeg/sfeeg;
	z= xf_timeconv1(dureeg,&days,&hours,&minutes,&seconds);
	fprintf(stderr,"        records= %ld\n",nneeg);
	fprintf(stderr,"        samplerate= %g Hz\n",sfeeg);
	fprintf(stderr,"        duration=\033[0;32m %.3f\033[0m seconds (%02d:%02d:%02d:%.3f)\n",dureeg,days,hours,minutes,seconds);
	//TEST	fprintf(stderr,"testing!\n");
	//for(ii=0;ii<nnact;ii++) { if(ii>=nnemg || ii>=nneeg) break ; printf("%g\t%g\t%g\n",datact[ii],datemg[ii],dateeg[ii]); }


	/******************************************************************************/
	/******************************************************************************/
	/******************************************************************************/
	/* STEP 2. ALLOCATE MEMORY & SET UP BANDS & SUCH /
	/* - this is done after reading the EEG/EMG data to determine appropriate window-size */
	/******************************************************************************/
	/******************************************************************************/

	/********************************************************************************
	A. DETERMINE THE ALLOWABLE NUMBER OF SCORES & ALLOCATE EEG MEMORY
	- minimum duration (seconds) between act,emg,eeg
	********************************************************************************/
	if(duract<duremg) { if(duract<dureeg) aa= duract; else aa=dureeg; }
	else { if(duremg<dureeg) aa= duremg; else aa=dureeg; }
	nscores= (long)aa; // total number of 1s-scores for activity, EMG and EEG
	fprintf(stderr,"...total 1-second scores (minimum of activity,emg,eeg): %ld\n",nscores);
	/* allocate EEM memory because this will NOT be done by a call to xf_bin_simple_f() as per activity and emg */
	scoreeeg= malloc(nscores * sizeof(*scoreeeg) * btot);
	if(scoreeeg==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }

	/******************************************************************************
	B. SET-UP FFT MODEL, TAPER, AND BANDS
	******************************************************************************/
	fprintf(stderr,"...setting up FFT model and taper...\n");
	nwinfft= 1.0*(long)(sfeeg*1.0);
	if(nwinfft%2 != 0) { fprintf(stderr,"\t--- Warning [%s]: FFT window-length (%ld) cannot be odd. Adjusting to %ld\n",thisprog,nwinfft,(nwinfft+1)); nwinfft++; }
	scaling1=1.0/(float)nwinfft; /* defining this way permits multiplication instead of (slower) division */
	// setup taper
	taper= xf_taperhann_d(nwinfft,1,1,message);
	if(taper==NULL) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	// setup fft configuration
	kiss_fftr_cfg cfgr = kiss_fftr_alloc( nwinfft ,0,0,0 ); /* configuration structure: memory assigned using malloc - needs to be freed at end */
	kiss_fft_cpx fft[nwinfft]; /* holds fft results: memory assigned explicitly, so does not need to be freed */
	/* allocate memory for working variables */
	if((buff2= calloc(nwinfft,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);} // buffer which is passed to the FFT function, copied from pdataf
	if((spect= calloc(nwinfft,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);} // holds amplitude of the FFT results
	if((spectmean= calloc(nwinfft,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);} // holds per-block mean FFT results (power) from multiple buff2-s

	/* DEFINE FREQUENCIES FOR EACH FFT OUTPUT, STARTING FROM ZERO, WHICH IS JUST THE DC OFFSET IN EACH WINDOW */
	/* note that we only really need the first half of these values, due to the Nyquist limit*/
	if((fftfreq=(float*)calloc(nwinfft,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds pre-caluclated frequencies associated with each FFT index
	aa= sfeeg/(double)nwinfft;
	kk= nwinfft/2;
	for(ii=0;ii<=nwinfft;ii++) {
		if(ii<kk) fftfreq[ii]= (float) ( (double)ii * aa ) ; else fftfreq[ii]= -1.0;
	}
	fftmaxindex= (nwinfft/2)-1; // the actual max-allowable index, not the Nyquist limit of nwinfft/2
	fftmaxfreq= fftfreq[fftmaxindex]; // ... so this is actually the maximum allowable frequency
	if(setverb==999) for(ii=0;ii<nwinfft;ii++) printf("____ FFT[%ld] frequency= %g\n",ii,fftfreq[ii]);

	/* FOR EACH BAND DETERMINE THE START-STOP INDICES */
	for(ii=0;ii<btot;ii++) {
		bstart2[ii]= xf_getindex1_d( 0.00,fftmaxfreq,(nwinfft/2),bstart1[ii],message) ;
	 	if(bstart2[ii]<0) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
		bstop2[ii]= xf_getindex1_d( 0.00,fftmaxfreq,(nwinfft/2),bstop1[ii],message) ;
	 	if(bstop2[ii]<0) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
		if(bstart2[ii]>fftmaxindex || bstop2[ii]>fftmaxindex) {
			fprintf(stderr,"\n--- Error [%s]: invalid band %s (%g-%g) - out of range for sample-rate or fft window size\n\n",thisprog,(setbands+ibands[ii]),bstart1[ii],bstop1[ii]);
			exit(1);
		}
	}
	if(setverb==999) for(ii=0;ii<btot;ii++) printf("____ BAND: %s : %g-%g Hz : indices %ld-%ld\n",(setbands+ibands[ii]),bstart1[ii],bstop1[ii],bstart2[ii],bstop2[ii]);


	/******************************************************************************/
	/******************************************************************************/
	/******************************************************************************/
	/* STEP 3: MAKE 1-SECOND SCORES FOR EACH DATA-TYPE
	/* - convert invalid scores to NAN, bin, then trim and normalise (EMG & EEG) */
	/******************************************************************************/
	/******************************************************************************/
	/******************************************************************************/

	/********************************************************************************
	A. SCORE ACTIVITY
	- DON'T rectify because the DSI system creates brief negativities either side of periods of activity - these appear to be filtering artefacts. Instead, treat these negativities as invalid and interpolate across them
	- however replace invalid values with NAN
	********************************************************************************/
	fprintf(stderr,"...processing activity...\n");
	/* replace negative and missing-values with NAN */
	fprintf(stderr,"    - detecting invalid values: ");
	kk=0; for(ii=0;ii<nnact;ii++) if(datact[ii]<0.0 || datact[ii]==setmissing) {datact[ii]=NAN;kk++;}
	fprintf(stderr,"%ld/%ld = %.3f%%\n",kk,nnact,(100*(double)kk/(double)nnact));
	/* average the data in non-overlapping 1 second bins - binsize is sample-frequency - it's ok if kk>nscores - extras can be ignored */
	fprintf(stderr,"    - binning (1-second)...\n");
	scoreact= xf_bin_simple_f(datact,nnact,(long)sfact,2,&kk,message);
	if(scoreact==NULL) { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); goto END; }

	/********************************************************************************
	B SCORE EMG - method based on Silvani et al (2017): https://academic.oup.com/sleep/article/40/4/zsx029/3044361
			- however use 1s binning instead of 0.5s, and trim +-0.1% instead of +-0.5%
	********************************************************************************/
	fprintf(stderr,"...processing EMG...\n");
	/* replace  missing-values with NAN - Spike2 fills (999999) and cases where both EEG and EMG are zero (lost signal)*/
	for(ii=jj=kk=0;ii<nnemg;ii++) {
		if(datemg[ii]==setmissing) {datemg[ii]=NAN;jj++;} // this is what Spike2 fills in at start & end of data
		if(datemg[ii]==0.0 && dateeg[ii]==0.0) {datemg[ii]=NAN;kk++;} // generally indicates lost data from DSI transmitter
	}
	fprintf(stderr,"    - %ld missing (%.3f%%) %ld \"zeros\" (%.3f%%)\n",jj,(100*(double)jj/(double)nnemg),kk,(100*(double)kk/(double)nnemg));
	/* rectify: because the signal is centred on zero and we want total "power" */
	for(ii=0;ii<nnemg;ii++) if(isfinite(datemg[ii]) && datemg[ii]<0.0) datemg[ii]*=-1.0;
	/* average the data in non-overlapping 1 second bins - binsize is sample-frequency - it's ok if kk>nscores - extras can be ignored */
	scoreemg= xf_bin_simple_f(datemg,nnemg,(long)sfemg,2,&kk,message);
	if(scoreemg==NULL) { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); goto END; }
	/* trim outliers - pre-existing NANs will also be preserved */
	kk= xf_outlier1_f(scoreemg,nscores,0.1,99.9,message);
	if(kk<0) { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); exit(1); }
	else fprintf(stderr,"    - removed %ld outliers (%.3f%%)\n",kk,(100*(double)kk/(double)nscores));
	/* normalise 0-1 */
	z= xf_norm2_f(scoreemg,nscores,0);


	/********************************************************************************
	C. SCORE EEG - scores for each band
	- assume a 1-second window (nwinfft= sfeeg) - FFT itself does the binning here
	- only process 1-100 Hz data
	********************************************************************************/
	fprintf(stderr,"...processing EEG...\n");
	/* replace missing-values with NAN */
	if(nneeg>nnemg) mm=nnemg; else mm=nneeg;
	for(ii=jj=kk=0;ii<mm;ii++) {
		if(dateeg[ii]==setmissing) {dateeg[ii]=NAN;jj++;} // this is what Spike2 fills in at start & end of data
		if(!isfinite(datemg[ii]) && dateeg[ii]==0.0) {dateeg[ii]=NAN;kk++;} // generally indicates lost data from DSI transmitter
	}
	fprintf(stderr,"    - %ld missing (%.3f%%) %ld \"zeros\" (%.3f%%)\n",jj,(100*(double)jj/(double)nnemg),kk,(100*(double)kk/(double)nnemg));
	for(window=0;window<nscores;window++) {
		/* CONVERT A WINDOW OF DATA TO A DE-MEANED, TAPERED DATA-BUFFER FOR FFT */
		pdataf= dateeg+(window*nwinfft); /* set index to data */
		/* determine if FFT should be skipped (>10% NANs) */
		for(ii=kk=0;ii<nwinfft;ii++) if(!isfinite(pdataf[ii])) kk++;
		if(kk>((double)nwinfft/10.0)) { for(band=0;band<btot;band++) scoreeeg[band*nscores+window]= NAN; continue; }
		/* apply interpolation */
		kk= xf_interp3_f(pdataf,nwinfft);
		/* fill the buffer with de-meaned, tapered data */
		sum=0; for(ii=0;ii<nwinfft;ii++) sum+= pdataf[ii]; /* sum the values in the window */
		mean= sum*scaling1; /* calculate the mean-correction to window */
		for(ii=0;ii<nwinfft;ii++) buff2[ii]= (pdataf[ii]-mean) * taper[ii]; /* copy real data from pdata to buff2, and apply mean-correction + taper */
		/* RUN THE FFT */
		kiss_fftr(cfgr,buff2,fft);
		// GENERATE THE SCALED AMPLITUDE SPECTRUM
		aa=2.0 * scaling1;
		kk= nwinfft/2; if(kk>100) kk=100; // with an upper limit of 100 (Hz), defines highest index in FFT result for which unique information can be obtained
		for(ii=0;ii<kk;ii++) { ar= fft[ii].r; ai= fft[ii].i; spect[ii]= aa * sqrtf( ar*ar + ai*ai ); }
		// CALCULATE THE BAND SCORES - SAVE IN THE scoreeeg MATRIX (row=band, column=window)
		for(band=0;band<btot;band++) {
			z= xf_auc1_d( (spect+bstart2[band]) , (bstop2[band]-bstart2[band]+1),1.0,0,resultd,message);
			if(z!=0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); goto END; }
			scoreeeg[band*nscores+window]= (float)resultd[0]; // total AUC, positive + negative
		}
	}
	/* TRIM AND NORMALISE BAND-SCORES RESULTS TO 0-1 */
	for(band=0;band<btot;band++) {
		pdataf= scoreeeg+(band*nscores); /* set index to data */
		/* trim outliers - pre-existing NANs will also be preserved */
		kk= xf_outlier1_f(scoreeeg,nscores,0.1,99.9,message);
		if(kk<0) { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); exit(1); }
		/* normalise 0-1 */
		z= xf_norm2_f(scoreeeg,nscores,0);
	}

	/* OUTPUT SCORES IF REQUIRED */
	if(setout==2) {
		fprintf(stderr,"    - outputting 1-second scores (not epochs)...\n");
		printf("act\temg"); for(jj=0;jj<btot;jj++) printf("\t%s",(setbands+ibands[jj])); printf("\n");
		for(ii=0;ii<nscores;ii++) {
			printf("%f\t%f",scoreact[ii],scoreemg[ii]);
			for(band=0;band<btot;band++) printf("\t%f",scoreeeg[(band*nscores)+ii]);
			printf("\n");}
		goto END;
	}


	/******************************************************************************/
	/******************************************************************************/
	/******************************************************************************/
	/* STEP 4: CALCULATE 10-SECOND EPOCH VALUES - reuse score-arrays for results
	/******************************************************************************/
	/******************************************************************************/
	/******************************************************************************/
	epochsamps= (long)(setepoch);
	nepochs= nscores/epochsamps;

	for(epoch=0;epoch<nepochs;epoch++) {

		pdataf= scoreact+(epoch*epochsamps);
		z= xf_percentile3_f(pdataf,epochsamps,50,&aa,&bb,message);
		if(z==-1) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
		scoreact[epoch]= (float)aa;

		pdataf= scoreemg+(epoch*epochsamps);
		z= xf_percentile3_f(pdataf,epochsamps,50,&aa,&bb,message);
		if(z==-1) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
		scoreemg[epoch]= (float)aa;

		for(band=0;band<btot;band++) {
			pdataf= scoreeeg+(band*nscores);
			z= xf_percentile3_f((pdataf+(epoch*epochsamps)),epochsamps,50,&aa,&bb,message);
			if(z==-1) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
			pdataf[epoch]= aa;
		}
	}
	nepochs= epoch;


	/* TEST: */
	fprintf(stderr,"    - outputting epochs...\n");
	printf("act\temg"); for(jj=0;jj<btot;jj++) printf("\t%s",(setbands+ibands[jj])); printf("\n");
	for(ii=0;ii<nepochs;ii++) {
		printf("%f\t%f",scoreact[ii],scoreemg[ii]);
		for(band=0;band<btot;band++) printf("\t%f",scoreeeg[(band*nscores)+ii]);
		printf("\n");
	}
	goto END;




	// DETERMINE TIME ZERO
	// DETERMINE RECORDING DURATION AND NUMBER OF EPOCHS BEFORE AND AFTER ZERO

	// SAVE ACTIVITY EPOCHS
	// READ EMG DATA & SAVE EPOCHDATA FOR POWER & NOISE
	// READ EEG DATA & SAVE EPOCHDATA FOR DELTA & GAMMA & NOISE

	// variables eatot[] eanoise[] eedelta[] eetheta[] eenoise empow[] emnoise[]]



goto END;
	/********************************************************************************/
	/* CLEANUP AND EXIT */
	/********************************************************************************/
END:
	if(setverb>0) fprintf(stderr,"...complete!\n");

	if(infileeeg!=NULL) free(infileeeg);
	if(infileemg!=NULL) free(infileemg);
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);

	if(datact!=NULL) free(datact);
	if(datemg!=NULL) free(datemg);
	if(dateeg!=NULL) free(dateeg);
	if(header!=NULL) free(header);

	if(scoreact!=NULL) free(scoreact);
	if(scoreemg!=NULL) free(scoreemg);
	if(scoreeeg!=NULL) free(scoreeeg);

	if(taper!=NULL) free(taper);
	if(spect!=NULL) free(spect);
	if(spectmean!=NULL) free(spectmean);
	if(buff2!=NULL) free(buff2);
	if(fftfreq!=NULL) free(fftfreq);

	exit(0);
}
