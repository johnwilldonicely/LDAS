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

*/



/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line, char *delimiters, long *nwords);
double *xf_readspike2_text_d(char *infile, long *nn, double *samprate, char *message);
float *xf_readbin2_f(char *infile, off_t *parameters, char *message);
char* xf_strsub1 (char *source, char *str1, char *str2);
long xf_interp3_f(float *data, long ndata);
long xf_interp3_d(double *data, long ndata);
int xf_norm2_f(float *data,long ndata,int normtype);
int xf_smoothgauss1_f(float *original,size_t arraysize,int smooth);
long *xf_definebands1(char *setbands,float *bstart1,float *bstop1, long *btot, char *messsage);
long xf_getindex1_d(double min, double max, long n, double value, char *message);
int xf_auc1_d(double *curvey, long nn, double interval, int ref, double *result ,char *message);

int xf_percentile2_f(float *data, long nn, double setper, double *per1, double *per2, char *message);
int xf_compare1_d(const void *a, const void *b);


// NOTE: the following function declarations are commented out to avoid re-initialization in kiss headers,
/*
void kiss_fftr(kiss_fftr_cfg st,const kiss_fft_scalar *timedata,kiss_fft_cpx *freqdata);
void kiss_fft(kiss_fft_cfg cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout);
*/
double *xf_taperhann_d(long setn, int setnorm, float setpow, char *message);

int xf_timeconv1(double seconds, int *days, int *hours, int *minutes, double *sec2);
int xf_bin1b_d(double *data, long *setn, long *setz, double setbinsize, char *message);
int xf_bin1b_f(float *data, long *setn, long *setz, double setbinsize, char *message);
int xf_filter_bworth1_f(float *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);

int xf_smoothgauss1_d(double *original,size_t arraysize,int smooth);
long xf_scale1_l(long data, long min, long max);
int xf_bin3_d(double *data, short *flag, long setn, long setz, double setbinsize, char *message);
double xf_bin1a_d(double *data, size_t *setn, size_t *setz, size_t setbins, char *message);


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
	float *datemg=NULL,*dateeg=NULL,*pdataf=NULL;
	double *datact=NULL,*pdatad=NULL;
	double siact,sfact,duract,duremg,dureeg,sfemg=500.0,sfeeg=500.0,epochsize=10.0;

	long fftmaxindex=-1,window;
	double *taper=NULL,*spect=NULL,*spectmean=NULL,*spectmean2=NULL,ar,ai,fftmaxfreq=-1.0,freqres;
	float *buff2=NULL,*fftfreq=NULL,scaling1,sum,mean;

	float *scoreact=NULL,*scoreemg=NULL,*scoreeeg=NULL;

	/* BAND DEFINITION */

	// Sandor Kantor / Sleepsign sleep-detection band definitions
	// NOTE "traditional" sigma (10-15Hz) is subsumed by Sandor's "spindle-alpha" (6,15)
	// char setbandsdefault[]= "delta,0.6,4.5,spinalph,6,15,theta,6,10,beta,12,25,gamma,30,100";

	// Gyorgi Buzsaki band definitions
	// char setbandsdefault[]= "delta,2,4,theta,4,8,alpha,8,12,beta,12,20,gamma1,20,90,gamma2,90,13,ripple,130,160";

	// Functional bands for test dataset (TPEEG054):
	// char setbandsdefault[]= "delta 1,5,theta,5,10,sigma,10,18,beta,18,35,gamma,35,80"
	char setbandsdefault[]= "delta,1,5,theta,5,10,sigma,10,18,beta,18,35,gamma,35,80";
	long *ibands=NULL,band,btot=0,bstart2[16],bstop2[16];
	float bstart1[16],bstop1[16];

	/* arguments */
	char *infileact=NULL,*setbands=NULL;
	int setverb=0;
	double setzero=0.0;

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
		fprintf(stderr,"    - ACT:  s2_export_activity_perchannel.s2s\n");
		fprintf(stderr,"    - EMG:  s2_eeg2bin.s2s\n");
		fprintf(stderr,"    - EEG:  s2_emg2bin.s2s\n");
		fprintf(stderr,"    - \n");
		fprintf(stderr,"USAGE: %s [in] [options]\n",thisprog);
		fprintf(stderr,"    [in]: filename for activity record\n");
		fprintf(stderr,"        - the base-name will be used to detect matching EMG/EEG files\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"    -verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"    -zero: time (seconds) to take as \"zero\" in the recording [%g]\n",setzero);
		fprintf(stderr,"    -bands: CSV band-triplets: name,start,stop\n");
		fprintf(stderr,"        - default: %s\n",setbandsdefault);
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
			else if(strcmp(argv[ii],"-zero")==0) setzero= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-bands")==0) setbands= argv[++ii];
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb!=999) { fprintf(stderr,"\n--- Error[%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(strcmp(infileact,"stdin")==0) { fprintf(stderr,"\n--- Error[%s]: this program does not accept \"stdin\" as an input. Please specify a filename\n\n",thisprog);exit(1);}


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
	datact= xf_readspike2_text_d(infileact,&nnact,&siact,message);
	if(datact==NULL) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
	sfact= 1.0/siact;
	//TEST:	printf("header: %s",header); for(ii=0;ii<nrows;ii++) { pdatad= datact+(ii*ncols); printf("%g",pdatad[0]); for(jj=1;jj<ncols;jj++) printf("\t%g",pdatad[jj]); printf("\n"); }
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
	/* STEP 2. ALLOCATE MEMORY /
	/* - this is done after reading the EEG/EMG data to determine appropriate window-size */
	/******************************************************************************/
	/******************************************************************************/

	/********************************************************************************
	A. ALLOCATE MEMORY FOR 1-SECOND SUB-SCORES (10 per epoch) and band indices
	********************************************************************************/
	if(duract<duremg) { if(duract<dureeg) aa= duract; else aa=dureeg; }
	else { if(duremg<dureeg) aa= duremg; else aa=dureeg; }
	nscores= (long)aa; // total number of 1s-scores for activity, EMG and EEG
	fprintf(stderr,"...total 1-second scores (minimum of activity,emg,eeg): %ld\n",nscores);
	fprintf(stderr,"\n");
	scoreact= malloc(nscores * sizeof(*scoreact)); if(scoreact==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }
	scoreemg= malloc(nscores * sizeof(*scoreemg)); if(scoreemg==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }
	scoreeeg= malloc(nscores * sizeof(*scoreeeg) * btot); if(scoreeeg==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }


// DETERMINE FFTWIN SIZE
// BASED ON SAMPLE RATE DETERMINE BAND START-STOPS
// CHECK VALIDITY

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
	/******************************************************************************/
	/******************************************************************************/
	/******************************************************************************/

	/********************************************************************************
	A. SCORE ACTIVITY - DON'T rectify because the DSI system creates brief negativities either side of periods of activity - these appear to be filtering artefacts. Instead, treat these negativities as invalid and interpolate across them
	********************************************************************************/
	fprintf(stderr,"...processing activity...\n");
	/* REMOVE NEGATIVE VALUES (ARTEFACTS OF FILTERING) */
	for(ii=0;ii<nnact;ii++) if(datact[ii]<0.0) datact[ii]=NAN;
	/* APPLY INTERPOLATION */
	ii= xf_interp3_d(datact,nnact);
	/* AVERAGE THE DATA IN NON-OVERLAPPING 1 SECOND BINS - do not use binning functions (they overwrite the original array) */
	mm= (long)sfact; // binsize
	sum= 0.0;
	for(ii=jj=kk=0;ii<nnact;ii++) {
		sum+= datact[ii];
		if(++jj==mm) {
			scoreact[kk]= sum/(double)jj;
			if(kk>=nscores) break; // do not exceed limits
			kk++; jj=0; sum=0.0;
	}} // note that partial bins at the end of data are ignored

	/********************************************************************************
	B SCORE EMG - method based on Silvani et al (2017) but zero is minimum and 1s binning rather than 0.5s
	 ??? there is a problem here
	- basically, zero in our data appears to be a data-loss value, and other values represent "very little muscular activity"
	- this is difficult to prove, but either way sometimes "zero" is included after trimming because there is a lot of missing ddta
	********************************************************************************/
	fprintf(stderr,"...processing EMG...\n");
	/* APPLY INTERPOLATION */
	ii= xf_interp3_f(datemg,nnemg);
	/* RECTIFY: because the signal is centred on zero */
	fprintf(stderr,"    - rectifying...\n");
	for(ii=0;ii<nnemg;ii++) if(datemg[ii]<0.0) datemg[ii]*=-1.0;
	/* AVERAGE THE DATA IN NON-OVERLAPPING 1 SECOND BINS - do not use binning functions (they overwrite the original array) */
	fprintf(stderr,"    - binning (1-second)...\n");
	mm= (long)sfemg; // binsize= 1 second
	sum= 0.0;
	for(ii=jj=kk=0;ii<nnemg;ii++) {
		sum+= datemg[ii];
		if(++jj==mm) {
			scoreemg[kk]= sum/(double)jj;
			if(kk>=nscores) break; // do not exceed limits
			kk++; jj=0; sum=0.0;
	}} // note that partial bins at the end of data are ignored
	/* GET THE UPPER AND LOWER PERCENTILE CUTOFFS - 0.5% and 99.5% */
	fprintf(stderr,"    - trim and normalise...\n");
	z= xf_percentile2_f(scoreemg,nscores,.5,&aa,&bb,message);
	if(z==-1) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
	/* TRIM AND NORMALIZE THE DATA */
	/* - modified from Silvani 2017 - assume lower limit is zero */
	for(ii=0;ii<nscores;ii++) {
		cc= scoreemg[ii];
		if(cc>=bb) scoreemg[ii]= NAN;
		// else if(cc<aa) scoreemg[ii]= NAN; // original method sets lower limit - for outrpurposes it is useful not to exclude zero, which can have special meaning
		// else scoreemg[ii]= 100.0 * (cc-aa) / (bb-aa);
		else scoreemg[ii]= 100.0 * (cc / bb);
	}

	/********************************************************************************
	C. SCORE EEG - scores for each band
	- assume a 1-second window (nwinfft= sfeeg)
	- only process 1-100 Hz data
	********************************************************************************/
	fprintf(stderr,"...processing EEG...\n");
	/* APPLY INTERPOLATION */
	ii= xf_interp3_f(dateeg,nneeg);
	window= -1;
	for (ii=0;ii<nneeg;ii+=nwinfft) {
		window++;
		if(window>=nscores) { fprintf(stderr,"\t Warning [%s]: exceeding max scores - stopping at window %ld\n",thisprog,window); break; }
		// CONVERT A WINDOW OF DATA TO A DE-MEANED, TAPERED DATA-BUFFE RFOR FFT
		pdataf= (dateeg+ii); /* set index to data */
		sum=0; for(jj=0;jj<nwinfft;jj++) sum+= pdataf[jj]; /* sum the values in the window */
		mean= sum*scaling1; /* calculate the mean-correction to window */
		for(jj=0;jj<nwinfft;jj++) buff2[jj]= (pdataf[jj]-mean) * taper[jj]; /* copy real data from pdata to buff2, and apply mean-correction + taper */
		// RUN THE FFT
		kiss_fftr(cfgr,buff2,fft);
		// GENERATE THE SCALED AMPLITUDE SPECTRUM
		aa=2.0 * scaling1;
		kk= nwinfft/2; if(kk>100) kk=100; // with an upper limit of 100 (Hz), defines highest index in FFT result for which unique information can be obtained
		for(jj=0;jj<kk;jj++) { ar= fft[jj].r; ai= fft[jj].i; spect[jj]= aa * sqrtf( ar*ar + ai*ai ); }
		// CALCULATE THE BAND SCORES - SAVE IN THE scoreeeg MATRIX
		for(jj=0;jj<btot;jj++) {
			z= xf_auc1_d( (spect+bstart2[jj]) , (bstop2[jj]-bstart2[jj]+1) ,1.0 , 0 ,resultd,message);
			if(z!=0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); goto END; }
			scoreeeg[window*btot+jj] = resultd[0]; // total AUC, positive + negative
	}}


	//TEST:
	fprintf(stderr,"    - outputting 1-second scores (not epochs)...\n");
	printf("ACT\tEMG"); for(jj=0;jj<btot;jj++) printf("\t%s",(setbands+ibands[jj])); printf("\n");
	for(ii=0;ii<nscores;ii++) {
		printf("%f\t%f",scoreact[ii],scoreemg[ii]);
		kk= ii*btot;
		for(jj=0;jj<btot;jj++) printf("\t%f",scoreeeg[kk+jj]);
		printf("\n");

	}


// ??? NORMALISE EEG SCORES SIMILARLY TO EMG - 0-100 range

	goto END;


	/******************************************************************************/
	/******************************************************************************/
	/******************************************************************************/
	/* STEP 3: CALCULATE 10-SECOND EPOCH VALUES
	/******************************************************************************/
	/******************************************************************************/
	/******************************************************************************/


	/********************************************************************************
	EMG EPOCHS
	********************************************************************************/

	/* FOR EACH EPOCH SAVE MEDIAN */
	mm= (long)(epochsize);
	kk= 0; // new epoch-counter
	for(ii=0;ii<nscores;ii+=mm) {
		pdataf= scoreemg+ii;
		z= xf_percentile2_f(pdataf,mm,50.0,&aa,&bb,message);
		if(z==-1) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
		scoreemg[kk++]= aa;
	}




exit(0);

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
