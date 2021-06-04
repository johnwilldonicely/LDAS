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
	char *header=NULL,*pchar=NULL,*basename=NULL;
	int sizeofdata;
	long *iword=NULL,nwords,binsamps,zero1,zero2;
	double *data1=NULL,*pdata;
	double sampintact,samprateact,duract,binsize=10.0;

	/* arguments */
	char *infile=NULL;
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
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-zero")==0) setzero= atof(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error[%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(strcmp(infile,"stdin")==0) { fprintf(stderr,"\n--- Error[%s]: this program does not accept \"stdin\" as an input. Please specify a filename\n\n",thisprog);exit(1);}


	/********************************************************************************
	CHECK FILENAME AND GENERATE BASENAME FOR READING OTHER FILES
	********************************************************************************/
	pchar= strstr(infile,"ACT_"); // infile must contain the word "activity.txt"
	if(pchar==NULL) { fprintf(stderr,"\n--- Error[%s]: invalid infile [%s] - must include the keyword \"ACT_\"\n\n",thisprog,infile);exit(1);}
	ii= pchar-infile; // the position at which "activity" is found in the filename
	basename= realloc(basename,strlen(infile));
	if(basename==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	strncpy(basename,infile,ii);
	fprintf(stderr,"\n");
	fprintf(stderr,"...infile= %s\n",infile);
	fprintf(stderr,"...basename= %s\n",basename);


	/********************************************************************************
	STORE ACTIVITY DATA
	- probably collected at 1Hz
	- immobility is registered as zero
	- max mobility is probably ~6
	********************************************************************************/
	if(setverb>0) fprintf(stderr,"...reading ACTIVITY data...\n");
//	data1= xf_readtable1_d(infile,"\t",&ncols,&nrows,&header,message);
	data1= xf_readspike2_text_d(infile,&nn,&sampintact,message);
	if(data1==NULL) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
	samprateact= 1.0/sampintact;
	//TEST:	printf("header: %s",header); for(ii=0;ii<nrows;ii++) { pdata= data1+(ii*ncols); printf("%g",pdata[0]); for(jj=1;jj<ncols;jj++) printf("\t%g",pdata[jj]); printf("\n"); }

	/* FIND DURATION N SECONDS AND REPORT */
	duract= nn*sampintact;
	z= xf_timeconv1(duract,&days,&hours,&minutes,&seconds);
	fprintf(stderr,"        label= %s\n",message);
	fprintf(stderr,"        records= %ld\n",nn);
	fprintf(stderr,"        samplerate= %g Hz\n",samprateact);
	fprintf(stderr,"        duration= %g seconds (%02d:%02d:%02d:%.3f)\n",duract,days,hours,minutes,seconds);

	/* RECTIFY: because the DSI receiver system creates brief 1s negativities either side of periods of activity */
	for(ii=0;ii<nn;ii++) if(data1[ii]<0.0) data1[ii]*=-1.0;

	/* AVERAGE THE DATA IN 10 SECOND BINS (EPOCHS) */
	zero1= (long)(setzero*samprateact);
	z= xf_bin1b_d(data1,&nn,&zero1,binsize,message);


	for(ii=0;ii<nn;ii++) {
		printf("%g\n",data1[ii]);
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

	if(basename!=NULL) free(basename);
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(data1!=NULL) free(data1);
	if(header!=NULL) free(header);
	exit(0);
}
