#define thisprog "xe-cor3"
#define TITLE_STRING thisprog" v 10, 20.May.April.2016 [JRH]"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAXLINELEN 1000

/*
<TAGS>stats</TAGS>
CHANGES:

v 10, 20.May.April.2016 [JRH]
	- bugfix: improper initialization of the correlate_simple function led to invalid interpretation of "r"

v 10, 17.April.2016 [JRH]
	- add explicit fclose command at end of file read - somehow this had been omitted!

v 10  29.February.2016 [JRH]
	- add internal de-meaning

v 10  24.February.2016 [JRH]
	- add FIR filter option (still not quite working!)

v 10  28.January.2016 [JRH]
	- allow use of a fixed window size

v 10  9.May.2014 [JRH]
	- bugfix: previously padding for smaller datasets with very low frequency cutoffs could exceed data length
				- resulted in garbage being inserted into dataset, causing artefacts
				- now ensures that npad <=n
	- filter resonance (setresonance) now a user-controlled option
	- NOTE: output from xf_filter_bworth4 is slightly different from xf_filter_bworth3, which is more widely used in other programs
				- this may require some more investigation

v 9  9.December.2013 [JRH]
	- renamed from old file xe-coherence2.8 - to avoid confusion, as this is not actually coherence!
	- interpolation only applied if non-finite values of xdata or ydata found
	- padding changed from n/4 to 4/setfmin - usually this will mean a big reduction in the padding
	- switch to xf_filter_bworth4
		- data must be copied to new array anyway - better to do it as part of filter operation
		- also, allows preallocation of memory for swap variable - saves allocation & free for every frequency
	- removed redundant references to old FFT coherence variables


v 8  7.October.2013 [JRH]
	- update execution of padding and filtering

v 7  11.July.2013 [JRH]
	- remove calculation of "phase-coherence" (cycle-detection and circular phase-correlation)
	- allow calculated sample frequency to be a float, instead of rounding to an integer
	- use immproved Butterworth filter function  xf_filter_bworth1_f

v 6  5.May.2013 [JRH]
	- include xf_compare_d to support revised version of xf_percentile1_d

v 5: 15.March.2013 [JRH]
	- if -fstep is set to -1:
		- the entire band from -fmin to -fmax is analyzed in a single step
		- window size is scaled to the lowest frequency
		- time-step is scaled to the highest frequency

v 4: 8.March.2013
	- drop calculation of power
	- updated xf_correlate_simple_f function now returns r=0 instead of r=NAN for data where "y" is unchanging (correlation = horizonatal line)

v 3: 6.October.2012 [JRH]
	- updated call to xf_percentile1_d
		- "message" string is no longer used, as printing messages with the function name complicated determination of program dependencies

v 2:  5.September.2012 [JRH]
	- drop requirement for time-file - replace with sample interval
	- read ALL samples save non-numbers etc. as NAN
	- internally interpolate data arrays

v 1:  4.September.2012 [JRH]
	- a single program to read, trim, filter, and correlate data in sliding windows
	- designed to replace scripts performing similar functions

*/

/* external functions start */
long xf_interp3_f(float *data, long ndata);
float *xf_padarray1_f(float *data, long nn, long npad, int type, char *message);
int xf_filter_bworth1_f(float* data, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
int xf_filter_bworth2_f(float *X, float *Y, float *Z, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
double *xf_filter_FIRcoef1(int NumTaps, int PassType, double OmegaC, double BW, char *WindowType, double WinBeta, char *message);
int xf_filter_FIRapply2_f(float *input, float *output, long nn, double *coefs, int ncoefs, int shift, char *message);
double xf_correlate_simple_f(float *x, float *y, long n, double *result_d);
int xf_percentile1_d(double *data, long n, double *result);
int xf_compare1_d(const void *a, const void *b);
/* external functions end */

int main(int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],message[MAXLINELEN],*matchstring=NULL,*pline,*pcol;
	long int i,j,k,n;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd,tt,result_d[32];
	FILE *fpin,*fpout;
	/* program-specific variables */
	size_t nbadx,nbady;
	long n2,n3,npad,timebin,*cstart=NULL,*cend=NULL;
	float *xdat1=NULL,*ydat1=NULL,*xdat2=NULL,*ydat2=NULL,*swap=NULL;
	float fstep,thisfreq;
	double *time=NULL,*intervals=NULL,sample_freq=24000,start,rdata,refpow1,pow1;
	double winsize=0.0, winstep=-1.0,sum,mean;
	double *coefs=NULL;
	/* arguments */
	int colt=1,colx=2,coly=3,setcirc=0,setverb=0,setlabel=1,setfilter=1;
	float setfmin=1.0,setfmax=100.0,setfstep=1.0, setresonance=1.4142;
	double setwinsize=0.0;

	if(argc==1) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate correlation over time between signals at multiple frequencies\n");
		fprintf(stderr,"- interpolates non-numeric data, NaN and Inf\n");
		fprintf(stderr,"- pads either end of the two data series (padding = 4.0/min, see below)\n");
		fprintf(stderr,"- bandpass filters at each frequency of interest\n");
		fprintf(stderr,"- correlates signals in a sliding (50%% overlap) window 2 wavelengths wide\n");
		fprintf(stderr,"USAGE:	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\" with time, x, and y columns\n");
		fprintf(stderr,"VALID OPTIONS [DEFAULT IN BRACKETS]...\n");
		fprintf(stderr,"	-ct column containing time values [%d]\n",colt);
		fprintf(stderr,"	-cx column containing independent variable [%d]\n",colx);
		fprintf(stderr,"	-cy column containing dependent variable [%d]\n",coly);
		fprintf(stderr,"	-min lower frequency at which to calc. correlation [%g]\n",setfmin);
		fprintf(stderr,"	-max upper frequency at which to calc. corelation [%g]\n",setfmax);
		fprintf(stderr,"	-fstep by which to incriment frequency from min to max [%g]\n",setfstep);
		fprintf(stderr,"		NOTE: set to -1 to analyze entire range in one steb\n");
		fprintf(stderr,"			- in this case time-window size is scaled to low freq.\n");
		fprintf(stderr,"			- in this case time-step is scaled to high freq.\n");
		fprintf(stderr,"	-label: window-labels identify start(1) middle(2) or end(3) time [%d]\n",setlabel);
		fprintf(stderr,"	-filt: filter type (1=Butterworth, 2=FIR) [%d]\n",setfilter);
		fprintf(stderr,"	-res: filter resonance (0.1 to sqrt(2)=1.4142) [%g]\n",setresonance);
		fprintf(stderr,"		NOTE: low values can produce ringing in the output\n");
		fprintf(stderr,"		NOTE: high values can dampen the signal\n");
		fprintf(stderr,"	-w: use fixed window size (0=AUTO, 2/frequency) [%g]\n",setwinsize);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLE:\n");
 		fprintf(stderr,"	%s temp.dat -cx 5 -cy 7\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	freq	window	time	nsamps	r_data	r_phase\n");
		fprintf(stderr,"NOTE: output times correspond with the start of each window\n");
		fprintf(stderr,"NOTE: last window for each line may be NAN if there are <3 data\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-ct")==0)    colt=atoi(argv[++i]);
			else if(strcmp(argv[i],"-cx")==0)    colx=atoi(argv[++i]);
			else if(strcmp(argv[i],"-cy")==0)    coly=atoi(argv[++i]);
			else if(strcmp(argv[i],"-min")==0)   setfmin=atof(argv[++i]);
			else if(strcmp(argv[i],"-max")==0)   setfmax=atof(argv[++i]);
			else if(strcmp(argv[i],"-fstep")==0) setfstep=atof(argv[++i]);
			else if(strcmp(argv[i],"-label")==0) setlabel=atoi(argv[++i]);
			else if(strcmp(argv[i],"-filt")==0)  setfilter=atoi(argv[++i]);
			else if(strcmp(argv[i],"-res")==0)   setresonance=atof(argv[++i]);
			else if(strcmp(argv[i],"-w")==0)     setwinsize=atof(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}
	if(setlabel<1||setlabel>3) {fprintf(stderr,"\a\n\t--- Error[%s]: -label (%d) invalid - must be 1,2 or 3\n\n",thisprog,setlabel);exit(1);}
	if(setfilter<1||setfilter>2) {fprintf(stderr,"\a\n\t--- Error[%s]: -filt (%d) invalid - must be 1 or 2\n\n",thisprog,setfilter);exit(1);}
	if(setwinsize<0) {fprintf(stderr,"\a\n\t--- Error[%s]: -w (%g) invalid - must be >=0\n\n",thisprog,setwinsize);exit(1);}


	/* READ THE DATA - SPECIFIC COLUMNS HOLD DATA  */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	n=0;
	nbadx=nbady=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		pline=line;
		colmatch=3; // number of columns to match
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			if(col==colt && sscanf(pcol,"%lf",&tt)==1) colmatch--; // store time value - check if input was actually a number
			if(col==colx) {colmatch--; if(sscanf(pcol,"%lf",&aa)!=1) aa=NAN;} // store value even if non-numeric
			if(col==coly) {colmatch--; if(sscanf(pcol,"%lf",&bb)!=1) bb=NAN;} // store value even if non-numeric
		}
		if(colmatch==0) {
			if(isfinite(tt)) {
				if((time=(double *)realloc(time,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				if((xdat1=(float *)realloc(xdat1,(n+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				if((ydat1=(float *)realloc(ydat1,(n+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				if(!isfinite(aa)) nbadx++;
				if(!isfinite(bb)) nbady++;
				time[n]=tt;
				xdat1[n]=aa;
				ydat1[n]=bb;
				n++;
	}}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

//for(i=0;i<n;i++) printf("%g\t%g\n",xdat1[i],ydat1[i]);exit(0);

	/* DETERMINE THE SAMPLE FREQUENCY ( 1/[MEDIAN INTERVAL] rounded to the nearest integer ) */
	if((intervals=(double *)realloc(intervals,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	aa=0.0; j=n-1;
	for(i=0;i<j;i++) intervals[i]=time[(i+1)]-time[i];
	z=xf_percentile1_d(intervals,(n-1),result_d);
	if(z!=0) {fprintf(stderr,"\t\aError[%s]: insufficient memory for calculation of percentiles\n",thisprog);exit(1);}

	sample_freq=1.0F/result_d[5];

	/* PRINT THE HEADER */
	printf("freq	window	time	nsamps	r_data	r_phase\n");

	/* MUST BE AT LEAST 4 DATA-POINTS TO CONTINUE */
	if(n<4) {
		pow1=rdata=NAN;
		for(thisfreq=setfmin; thisfreq<=setfmax;thisfreq+=setfstep) {
			printf("%.4f	0	0	%ld	%g\n",thisfreq,n,rdata);
		}
		exit(0);
	}

	/* LINEAR INTERPOLATION OF THE DATA - REPLACE ALL NANs */
	if(nbadx>0) xf_interp3_f(xdat1,(size_t)n);
	if(nbady>0) xf_interp3_f(ydat1,(size_t)n);

	/* DE-MEAN THE DATA */
	sum=0.0; for(i=0;i<n;i++) sum+=xdat1[i]; mean=sum/(double)n; for(i=0;i<n;i++) xdat1[i]-=mean;
	sum=0.0; for(i=0;i<n;i++) sum+=ydat1[i]; mean=sum/(double)n; for(i=0;i<n;i++) ydat1[i]-=mean;


	/* EXPAND THE DATA ARRAYS TO ACCOMMODATE PADDING */
	npad=(int)(4.0/setfmin);
	if(npad>n)npad=n;
	n2=n+npad+npad;

	/* ALLOCATE MEMORY FOR ARRAYS PASSED TO FILTER AND CORRELATION FUNCTIONS */
	/* "X" is series 1, "Y" is series 2 */
	/* three variables assigned for each for the Butterworth function, to avoid repeated reallocation of output- and swap-data during filtering */
	if((xdat1=(float *)realloc(xdat1,(n2)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((ydat1=(float *)realloc(ydat1,(n2)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((xdat2=(float *)realloc(xdat2,(n2)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((ydat2=(float *)realloc(ydat2,(n2)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((swap=(float *)realloc(swap,(n2)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((cstart=(long *)realloc(cstart,(n2)*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((cend=(long *)realloc(cend,(n2)*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/* PAD THE ARRAYS AT THE BEGINNING AND END - DATA ON ENDS WILL TREND TO ZERO RATE OF CHANGE */
	xdat1= xf_padarray1_f(xdat1,n,npad,3,message);
	if(xdat1==NULL) { fprintf(stderr,"Error in %s (%s)\n",thisprog,message); exit(1);}
	ydat1= xf_padarray1_f(ydat1,n,npad,3,message);
	if(ydat1==NULL) { fprintf(stderr,"Error in %s (%s)\n",thisprog,message); exit(1);}
	//TEST:for(i=0;i<n2;i++) printf("%g\t%g\n",xdat1[i],ydat1[i]);exit(0);

	/* IF SETFSTEP<0, assume that the entire range is to be analyzed in one step */
	if(setfstep<0) fstep=(setfmax-setfmin)+1.0;
	else fstep=setfstep;

	/* FOR EACH FREQUENCY, FILTER THE DATA, CORRELATE, AND CALCULATE RMS POWER */
	for(thisfreq=setfmin; thisfreq<=setfmax;thisfreq+=fstep) {

		/* bandpass filter both data series - results copied to xdat2 and ydat2 */

		if(setfilter==1) {
			if(setfstep<0) { a=setfmin ; b=setfmax; } else a=b=thisfreq;
			i = xf_filter_bworth2_f(xdat1,xdat2,swap,(size_t)n2,sample_freq,a,b,setresonance,message); // filter x-data (lowpass)
			if(i<0) {fprintf(stderr,"\n\t --- Error [%s]: %s\n\n\n",thisprog,message);goto END1;}
			i = xf_filter_bworth2_f(ydat1,ydat2,swap,(size_t)n2,sample_freq,a,b,setresonance,message); // filter x-data (lowpass)
			if(i<0) {fprintf(stderr,"\n\t --- Error [%s]: %s\n\n\n",thisprog,message);goto END1;}
		}
		else if(setfilter==2) {
			if(setfstep<0) { aa= (setfmax+setfmin)/2.0; bb= setfmax-setfmin; }
			else { aa=thisfreq; bb=fstep;	}
			/* convert frequency (aa) and bandwidth (bb) to proportion of Pi */
			aa= 2.0 * (aa/sample_freq) ;
			bb= 2.0 * (bb/sample_freq) ;

			coefs= xf_filter_FIRcoef1(101,3,aa,bb,"kaiser",0,message);
			if(coefs==NULL) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

			i=xf_filter_FIRapply2_f(xdat1,xdat2,n2,coefs,101,1,message);
			if(i!=0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

		}
// 		// THIS IS THE OLD CODE USING THE REGULAR BUTTER WORTH FILTER
// 		/* copy the data to temp arrays */
// 		for(i=0;i<n2;i++) { xdat2[i]=xdat1[i]; ydat2[i]=ydat1[i]; }
// 		/* bandpass filter both data series */
// 		if(setfstep<0) { a=setfmin ; b=setfmax; } else a=b=thisfreq;
// 		i = xf_filter_bworth1_f(xdat2,(size_t)n2,sample_freq,a,b,setresonance,message); // filter x-data (lowpass)
// 		if(i<0) {fprintf(stderr,"\n\t --- Error [%s]: %s\n\n\n",thisprog,message);	goto END1;}
// 		i = xf_filter_bworth1_f(ydat2,(size_t)n2,sample_freq,a,b,setresonance,message); // filter x-data (lowpass)
// 		if(i<0) {fprintf(stderr,"\n\t --- Error [%s]: %s\n\n\n",thisprog,message);goto END1;}


		/* move pointers for array to beginning of original data */
		xdat2=xdat2+npad;
		ydat2=ydat2+npad;

		/* determine winstep and winsizefor this frequency - winstep is set for 50% overlapping windows */
		if(setfstep<0) {
			if(setwinsize==0.0) winsize=(2.0/setfmin); /* for broadband analysis, winsize is determined by the low frequency */
			else winsize=setwinsize;
			winstep=(1.0/setfmax); /* step is 1/2 winsize */
		}
		else {
			if(setwinsize==0) winsize=(2.0/thisfreq); /* for non-broadband analysis, winsize is determined by the current frequency  */
			else winsize=setwinsize;
			winstep=(1.0/thisfreq); /* step is 1/2 winsize */
		}

		start=time[0];
		k=n-1;
		timebin=0;

		/* apply the sliding window: i=start-index of current window, j=current sample up to end of the current window */
		for(i=0;i<k;i++) {
			start=time[i];
			/* look forward until window width = winsize */
			for(j=i+1;j<n;j++) {
				if(j==k||(time[j]-start)>=winsize) {
					n3= j-i; rdata=NAN;
					if(n3>1) {
						/* calculate correlation between filtered data, starting at index i */
						rdata= xf_correlate_simple_f((xdat2+i),(ydat2+i),n3,result_d);
					}
					if(setlabel==1) aa=start;
					if(setlabel==2) aa=(start+time[j])/2.0;
					if(setlabel==3) aa=time[j];
					if(setfstep<0) a=(setfmax+setfmin)/2.0; else a=thisfreq;
					printf("%g	%ld	%g	%ld	%g\n",a,timebin,aa,n3,rdata);
					timebin++;
					break;
				}
			}
			/* advance the window */
			for(j=(i+1);j<n;j++) if((time[j]-start)>=winstep) break;
			i=j-1;
		}

		/* restore pointers to original position for next round of filtering */
		xdat2=xdat2-npad;
		ydat2=ydat2-npad;
	}

END1:
	free(time);
	free(xdat1);
	free(ydat1);
	free(xdat2);
	free(ydat2);
	free(swap);
	free(cstart);
	free(cend);
	free(intervals);
	free(coefs);

	exit(0);
}
