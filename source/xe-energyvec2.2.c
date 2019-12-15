#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-energyvec2"
#define TITLE_STRING thisprog" v 2: 15.February.2014 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing</TAGS>
v 2: 15.February.2014 [JRH]
	- change -ws option to -c (number of cycles to span for RMS window) - easier to use

*/



/* external functions start */
float *xf_padarray2_f(float *data, size_t nn, size_t npad, int type, char *message);
int xf_filter_bworth1_f(float *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
int xf_rms2_f(float *input, float *output, size_t nn, size_t nwin1, char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],message[MAXLINELEN];
	long i,j,k,l,m,n;
	size_t ii,jj,kk,ll,mm,nn;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	size_t n1,n2,start,stop,npad=0,winsize;
	float *data1=NULL,*output=NULL;
	double rms;
	/* arguments */
	long setpad=-1;
	int setoutpad=0, setverb=0;
	float setsampfreq=100.0; // sample frequency of input (samples/s)
	float setfreq=0.0,setncycles=5.0;  // filter cutoffs and cycles spanned by RMS window
	float setresonance=1.0; // filter resonance setting ()

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Output the energy envelope in a signal at frequency f\n");
		fprintf(stderr,"Biquad Butterworth filter applied and sliding-window RMS calculated\n");
		fprintf(stderr,"Assumes one value per input line\n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\", format= single column\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-sf: sample frequency (sample/s) [%f]\n",setsampfreq);
		fprintf(stderr,"	-f: central frequency for band-pass filtering [%f]\n",setfreq);
		fprintf(stderr,"	-res: resonance (0.1 to sqrt(2)=1.4142) [%g]\n",setresonance);
		fprintf(stderr,"		NOTE: low values can produce ringing in the output\n");
		fprintf(stderr,"		NOTE: high values can dampen the signal\n");
		fprintf(stderr,"	-pad: apply cosine-tapered padding? (-1=AUTO,0=NO,>0=SAMPLES) [%ld]\n",setpad);
		fprintf(stderr,"		*note: auto = 1/4 record or 4*interval if -f is specified\n");
		fprintf(stderr,"	-c: number of cycles spanned by window for RMS calculation [%g]\n",setncycles);
		fprintf(stderr,"	-v: set verbose output (0=NO,1=YES) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -t 1\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 3\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-sf")==0) 	{ setsampfreq=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-f")==0) { setfreq=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-res")==0) 	{ setresonance=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-pad")==0) 	{ setpad=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-c")==0) 	{ setncycles=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-v")==0) 	{ setverb=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}
	if(setsampfreq<=0) {fprintf(stderr,"\n--- Error[%s]: sample frequency (-sf %g) must be > 0\n\n",thisprog,setsampfreq);exit(1);}
	if(setfreq<0) {fprintf(stderr,"\n--- Error[%s]: frequency-of-interest (-setfreq %g) must be >= 0 \n\n",thisprog,setfreq);exit(1);}
	if(setpad<0&&setpad!=-1) {fprintf(stderr,"\n--- Error[%s]: padding option (-pad %ld) must be -1, 0 or >0\n\n",thisprog,setpad);exit(1);}
	if(setverb!=0 && setverb!=1) {fprintf(stderr,"\n--- Error[%s]: -v option (%d) must be 0 or 1 \n\n",thisprog,setverb);exit(1);}
	if(setncycles<=0)  {fprintf(stderr,"\n--- Error[%s]: -c option (%g) must be > 0 \n\n",thisprog,setncycles);exit(1);}


	/********************************************************************************/
	/* STORE RAW DATA - SINGLE COLUMN */
	/********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	n1=0;z=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%f",&a)!=1) a=NAN;
		if((data1=(float *)realloc(data1,(n1+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if(!isfinite(a)) z=1;
		data1[n1++]=a;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	n2=n1;

	/********************************************************************************/
	/* ERROR & EXIT IF THERE ARE INVALID DATAPOINTS */
	/********************************************************************************/
	if(z==1) {
		if(data1!=NULL) free(data1);
		fprintf(stderr,"\n\t --- Error [%s]: input \"%s\" contains invalid data - please interpolate before filtering\n",thisprog,infile);
		exit(1);
	}
	/********************************************************************************
	WARN, OUTPUT & EXIT IF THERE ARE TOO FEW DATA POINTS TO ACTUALLY FILTER
	********************************************************************************/
	if(n1<4) {
		for(ii=0;ii<n1;ii++) printf("%g\n",data1[ii]);
		if(data1!=NULL) free(data1);
		fprintf(stderr,"--- Warning[%s]: input \"%s\" has less than 4 data points - no filtering performed\n",thisprog,infile);
		exit(0);
	}

	/********************************************************************************
	PAD THE DATA
	********************************************************************************/
	/* automatic padding selection: 4/setfreq or n1/4 */
	if(setpad<0) {
		if(setfreq<=0.0) npad=(int)((float)n1/4.0);
		else npad=(int)((setsampfreq*4.0)/setfreq);
		if(npad<1 || npad>n1) npad=n1;
		n2=n1+npad+npad;
	}
	/* user-specified padding */
	else if(setpad>0) {
		npad=setpad;
		n2=n1+npad+npad;
	}
	/* no padding */
	else {
		npad=0;
		n2=n1;
	}
 	/* apply the cosine-taper padding type 3 (beginning and end) */
 	if(npad>0) {
		data1 = xf_padarray2_f(data1,n1,npad,3,message);
		if(data1==NULL) {fprintf(stderr,"\n\t --- Error [%s]: %s\n\n\n",thisprog,message);free(data1);exit(1);}
	}


	/********************************************************************************/
	/* ALLOCATE MEMORY FOR RESULT */
	/********************************************************************************/
	if((output=(float *)realloc(output,n2*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);free(data1);exit(1);};

	/********************************************************************************
	FILTER THE DATA
	********************************************************************************/
	x= xf_filter_bworth1_f(data1,n2,setsampfreq,setfreq,setfreq,setresonance,message);
	if(x!=0) { fprintf(stderr,"\n--- Error [%s]: %s\n\n",thisprog,message); goto END1; }

	/********************************************************************************
	CALCULATE THE RMS POWER & PRINT RESULTS, OMITTING ANY PADDING
	********************************************************************************/
	/* calculate window size (number of cycle-lengths for frequency of interest) */
	winsize= (size_t) ((setsampfreq*setncycles)/setfreq);
	x= xf_rms2_f(data1,output,n2,winsize,message);
	if(x!=0) { fprintf(stderr,"\n--- Error [%s]: %s\n\n",thisprog,message); goto END1; }

	start=npad;
	stop=n1+npad;
	for(ii=start;ii<stop;ii++) printf("%g\n",output[ii]);

	if(setverb==1) {
		fprintf(stderr,"\n");
		fprintf(stderr,"%s\n",message);
		fprintf(stderr,"samples: %ld\n",n1);
		fprintf(stderr,"padding: %ld\n",npad);
		fprintf(stderr,"rms_windowsize: %ld\n",winsize);
		fprintf(stderr,"\n");
	}

END1:
	free(data1);
	free(output);
	exit(0);

}
