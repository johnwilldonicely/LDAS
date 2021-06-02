#define thisprog "xe-spike2-sleep"
#define TITLE_STRING thisprog" 1.June.2021 [JRH]"
#define MAXLINELEN 1000
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define N 20
/*
<TAGS> SPIKE2 </TAGS>

1.June.2021 [JRH]

*/



/* external functions start */
double *xf_readtable1_d(char *infile, char *delimiters, long *ncols, long *nrows, char **header, char *message);
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line, char *delimiters, long *nwords);
long xf_scale1_l(long data, long min, long max);
int xf_bin3_d(double *data, short *flag, long setn, long setz, double setbinsize, char *message);
double xf_bin1a_d(double *data, size_t *setn, size_t *setz, size_t setbins, char *message);

/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL,message[MAXLINELEN];
	int x,y,z,vector[] = {1,2,3,4,5,6,7};
	long ii,jj,kk,nn,maxlinelen=0;
	float a,b,c;
	double aa,bb,cc;
	FILE *fpin,*fpout;
	/* program-specific variables */
	char *header=NULL,*pchar=NULL,*basename=NULL;
	int sizeofdata;
	long *iword=NULL,nwords,nrows,ncols;
	double *data1=NULL,*pdata;
	/* arguments */
	char *infile=NULL;
	int setverb=0;
	double setsfact=1.0;

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
		fprintf(stderr,"    - activity: s2_export_activity.s2s\n");
		fprintf(stderr,"    - EMG:      s2_eeg2bin.s2s\n");
		fprintf(stderr,"    - EEG:      s2_emg2bin.s2s\n");
		fprintf(stderr,"    - \n");
		fprintf(stderr,"USAGE: %s [in] [options]\n",thisprog);
		fprintf(stderr,"    [in]: filename for activity record\n");
		fprintf(stderr,"        - assumes data is exported at 1 Hz and includes a header\n");
		fprintf(stderr,"        - the base-name will be used to detect matching EMG/EEG files\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"    -verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
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
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error[%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(strcmp(infile,"stdin")==0) { fprintf(stderr,"\n--- Error[%s]: this program does not accept \"stdin\" as an input. Please specify a filename\n\n",thisprog);exit(1);}


	/********************************************************************************
	CHECK FILENAME AND GENERATE BASENAME FOR READING OTHER FILES
	********************************************************************************/
	pchar= strstr(infile,"activity.txt"); // infile must contain the word "activity.txt"
	if(pchar==NULL) { fprintf(stderr,"\n--- Error[%s]: invalid infile [%s] - must end in \"activity.txt\"\n\n",thisprog,infile);exit(1);}

	ii= pchar-infile; // the position at which "activity" is found in the filename
	basename= realloc(basename,strlen(infile));
	if(basename==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	strncpy(basename,infile,ii);

	printf("infile= %s\n",infile);
	printf("basename= %s\n",basename);
	exit(0);


	/********************************************************************************
	STORE ACTIVITY DATA
	********************************************************************************/
	if(setverb>0) fprintf(stderr,"...reading data...\n");
	data1= xf_readtable1_d(infile,"\t",&ncols,&nrows,&header,message);
	if(data1==NULL) { fprintf(stderr,"\n--- Error: %s/%s\n\n",thisprog,message); exit(1); }
	//TEST:	printf("header: %s",header); for(ii=0;ii<nrows;ii++) { pdata= data1+(ii*ncols); printf("%g",pdata[0]); for(jj=1;jj<ncols;jj++) printf("\t%g",pdata[jj]); printf("\n"); }

	// DETERMINE TIME ZERO

	// DETERMINE RECORDING DURATION AND NUMBER OF EPOCHS BEFORE AND AFTER ZERO

	//FOR EACH SUBJECT
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
