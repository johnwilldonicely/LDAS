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
/*
<TAGS> SPIKE2 </TAGS>

1.June.2021 [JRH]

*/



/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line, char *delimiters, long *nwords);
double *xf_readspike2_text_d(char *infile, long *nn, double *samprate, char *message);
int xf_smoothgauss1_d(double *original,size_t arraysize,int smooth);

int xf_bin1b_d(double *data, long *setn, long *setz, double setbinsize, char *message);

long xf_scale1_l(long data, long min, long max);
int xf_bin3_d(double *data, short *flag, long setn, long setz, double setbinsize, char *message);
double xf_bin1a_d(double *data, size_t *setn, size_t *setz, size_t setbins, char *message);
int xf_timeconv1(double seconds, int *days, int *hours, int *minutes, double *sec2);

char* xf_strsub1 (char *source, char *str1, char *str2);

/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL,message[MAXLINELEN];
	int x,y,z,vector[] = {1,2,3,4,5,6,7};
	long ii,jj,kk,nn,maxlinelen=0;
	float a,b,c;
	double aa,bb,cc;
	FILE *fpin,*fpout;
	/* date and time variables */
	char timestring[256];
	time_t t1,t2;
	struct tm *tstruct1;
	int days,hours,minutes;
	double seconds;

	/* program-specific variables */
	char *header=NULL,*pchar=NULL;
	char *infileeeg=NULL,*infileemg=NULL;
	int sizeofdata;
	long *iword=NULL,nwords,binsamps,zero1,zero2;
	double *datact=NULL,*pdata;
	double sampintact,samprateact,duract,binsize=10.0;

	/* arguments */
	char *infileact=NULL;
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
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error[%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(strcmp(infileact,"stdin")==0) { fprintf(stderr,"\n--- Error[%s]: this program does not accept \"stdin\" as an input. Please specify a filename\n\n",thisprog);exit(1);}


	/********************************************************************************
	CHECK ACTIVITY FILENAME AND GENERATE FILENAMES FOR EEG AND EMG
	********************************************************************************/
	pchar= strstr(infileact,"ACT_"); // infileact must contain the word "activity.txt"
	if(pchar==NULL) { fprintf(stderr,"\n--- Error[%s]: invalid infileact [%s] - must include the keyword \"ACT_\"\n\n",thisprog,infileact);exit(1);}
	infileeeg= xf_strsub1(infileact,"ACT","EEG"); if(infileeeg==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	infileemg= xf_strsub1(infileact,"ACT","EMG"); if(infileemg==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	fprintf(stderr,"\n");
	fprintf(stderr,"...activity file= %s\n",infileact);
	fprintf(stderr,"...matching EEG=  %s\n",infileeeg);
	fprintf(stderr,"...matching EMG=  %s\n",infileemg);

	/********************************************************************************/
	/********************************************************************************
	STORE ACTIVITY DATA
	- probably collected at 1Hz
	- immobility is registered as zero
	- max mobility is probably ~6
	********************************************************************************/
	/********************************************************************************/

	if(setverb>0) fprintf(stderr,"...reading ACTIVITY data...\n");
//	datact= xf_readtable1_d(infileact,"\t",&ncols,&nrows,&header,message);
	datact= xf_readspike2_text_d(infileact,&nn,&sampintact,message);
	if(datact==NULL) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
	samprateact= 1.0/sampintact;
	//TEST:	printf("header: %s",header); for(ii=0;ii<nrows;ii++) { pdata= datact+(ii*ncols); printf("%g",pdata[0]); for(jj=1;jj<ncols;jj++) printf("\t%g",pdata[jj]); printf("\n"); }

	/* FIND DURATION N SECONDS AND REPORT */
	duract= nn*sampintact;
	z= xf_timeconv1(duract,&days,&hours,&minutes,&seconds);
	fprintf(stderr,"        label= %s\n",message);
	fprintf(stderr,"        records= %ld\n",nn);
	fprintf(stderr,"        samplerate= %g Hz\n",samprateact);
	fprintf(stderr,"        duration= %g seconds (%02d:%02d:%02d:%.3f)\n",duract,days,hours,minutes,seconds);

	/* RECTIFY: because the DSI receiver system creates brief 1s negativities either side of periods of activity */
	for(ii=0;ii<nn;ii++) if(datact[ii]<0.0) datact[ii]*=-1.0;

	/* AVERAGE THE DATA IN 10 SECOND BINS (EPOCHS) */
	zero1= (long)(setzero*samprateact);
	z= xf_bin1b_d(datact,&nn,&zero1,binsize,message);

	/********************************************************************************/
	/********************************************************************************/
	/* EMG DATA: STORE AND PROCESS */
	/********************************************************************************/
	/********************************************************************************/





	//TEST for(ii=0;ii<nn;ii++) { printf("%g\n",datact[ii]); }

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
	if(header!=NULL) free(header);
	exit(0);
}
