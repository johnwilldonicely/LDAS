#define thisprog "xe-pac2"
#define TITLE_STRING thisprog" v 2: 2.March.2018 [JRH]"
#define MAXLINELEN 1000

#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "kiss_fftr.h"

/*
<TAGS>signal_processing</TAGS>

v 2: 21.March.2018 [JRH]
	- bugfix: make sure there are at least TWO arguments on command line
	- for verbose output high-frequency bounds as well

v 2: 2.March.2018 [JRH]
	- for verbose output, add space after "=" to make output keyword-searchable

v 2: 6.September.2016 [JRH]
	- add ability to set coherence adjustment on/of (change default to off)

v 0 18.February.2016 [JRH]
	- picked up from where we left off with xe-pac1.9
	- update binary read to be consistent with other programs
		- data input types 0-9
		- dropped use of BINX format

v 8 6.May.2015 [JRH]
	- switch to dual-file input as per xe-fftcoh2, to simplify data handling
	- added capability to use FIR filter instead of Butterworth
		- probably better for low sample rates (1KHz or less)
	- updated xf_filter_butterworth4 - proper coefficient initialization
	- added de-meaning and padding for Butterworth filtering
	- added scaling for FFT-coherence calculation, based on accumulation setting

v 7 24.February.2014 [JRH]
	- file defining window-times is now expected to express this in samples, not seconds
	- sensible, as other values used by the program (eg. the window size) are expressed in samples

v 6 1.December.2013 [JRH]
	- replace xf_power_FIRwidth2orthrms1_f with separate "standard" functions xf_filter_bworth2_f and xf_rms2_f
		- these functions have more general utility & will be easier to maintain
		- moving memory allocation to main() reduces overhead for every iteration of frequency and window

v 5 29.November.2013 [JRH]
	- removed padding of data_b
		- Butterworth filter should be robust against DC offset induced edge effects
		- in any case an edge effect will not affect coherence
		- added control of RMS window size by -m option (also used for Goertzel window sizing)

v 4 26.November.2013 [JRH]
	- Goertzel algorithm for calculating power re-introduced as an option (-p 1)

v 3 25.November.2013 [JRH]
	- switch to Butterworth filter + RMS calculation of energy envelope for input B
		- this is faster than the FFT/Goertzel methods
		- it also allows greater frequency specificity without increasing processing demands
		- however it does require more memory - temporary storage of 2*(n+npad+npad)
		- addition of -res option

v 2 19.November.2013 [JRH]
	- introduce freq2 variables to control lower/uper limits on high-frequency
	- accordingly, change options to -min1 -min2 -max1 -max2
	- introduce -m(ult) option to control high-freq wavelength multiplier to determine window width for Goertzel algorithm

*/

/* external functions start */
float *xf_readbin2_f(char *infile, off_t *parameters, char *message);
long xf_interp3_f(float *data, long ndata);
long *xf_window1_l(long n, long winsize, int equalsize, long *nwin);
float xf_round1_f(float input, float setbase, int setdown);
int xf_power_goertzel2_f(float *input, float *power, size_t nn, float sample_freq, float freq, size_t nwin, char *message);
float *xf_padarray3_f(float *data, long nn, long npad, int type, char *message);
int xf_filter_bworth2_f(float *X, float *Y, float *Z, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
double *xf_filter_FIRcoef1(int FIRntaps, int FIRpass, double FIRomegaC, double FIRwidth2, char *FIRwindow, double FIRbeta, char *message);
int xf_filter_FIRapply2_f(float *input, float *output, long nn, double *FIRcoefs, int nFIRcoefs, int shift, char *message);
int xf_rms2_f(float *input, float *output, size_t nn, size_t nwin1, char *message);
int xf_smoothgauss1_f(float *original, size_t arraysize,int smooth);
float *xf_matrixrotate1_f(float *data1, long *width, long *height, int r);
// NOTE: the following function declarations are commented out to avoid re-initialization in kiss headers,
// They are included here only so xs-progcompile (which won't detect that they are commented out) will include them during compilation
/*
void kiss_fftr(kiss_fftr_cfg st,const kiss_fft_scalar *timedata,kiss_fft_cpx *freqdata);
void kiss_fft(kiss_fft_cfg cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout);
*/
/* external functions end */

int main(int argc, char *argv[]) {

	/* general-use variables */
	char infile1[256],infile2[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],message[MAXLINELEN],*pline,*pcol;
	int x,y,z;
	int sizeoflong=sizeof(long),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	long i,j,k,m,n=0;
	size_t ii,jj,kk;
	float a,b,c,d,e,f,g,h;
	double aa,bb,cc,dd,ee,ff,gg,twopi=2.0*M_PI;
	FILE *fpin1,*fpin2,*fpout;
	/* program-specific variables */
	void *data0=NULL;
	char timefile[256];
	int partnbuff,morenbuff,halfnbuff,finished=0,foundnan=0,indexa=-1,indexb=-1;
	long window,nwin=0,nbad_a,nbad_b,*windex=NULL;
	float *data_a=NULL,*data_b=NULL,*data_b2=NULL,*data_b3=NULL,*pdata_a,*pdata_b,*buff1_a=NULL,*buff1_b=NULL,*tempdata_a=NULL,*tempdata_b=NULL;
	float *sumspect_aa=NULL,*sumspect_bb=NULL,*sumspect_abr=NULL,*sumspect_abi=NULL,*freq=NULL;
	float sum_a=0.0,sum_b=0.0,mean_a=0.0,mean_b=0.0,phase_a,phase_b,sample_interval=0.0,scaling1,scaling2;
	float ar,ai,br,bi,setnbuff_f,*coherence=NULL;
	float freq2,freq2step,adja1,adja2;
	double freqres,*tapers=NULL;
	size_t aaa,accumulate=0,nout=0,npad=-1;
	int FIRpass=3;
	double *FIRcoefs=NULL,FIRomegaC,FIRwidth2;
	/* binary file read/write variables */
	off_t datasize,params[6],ntowrite;
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
	/* arguments */
	int setdatatype=-1,setntapers=0,setrms=0,setnbuff=0,setmin=-1,setmax=-1,setstep=0,setadja=0;
	int setmatrix=0,setverb=0,settimefile=0,setgauss=0,setasc=1,setpow=1;
	float setsfreq=1.0,minfreq1=-1.0,maxfreq1=-1.0,minfreq2=-1.0,maxfreq2=-1.0,setmult=7.0;
	float setresonance=0.5; // filter resonance setting ()
	long setaccumulate=0;
	char FIRWindow[16];
	int FIRntaps=101,FIRshift=1;
	double FIRwidth1=3,FIRbeta=10.0;


	snprintf(FIRWindow,16,"kaiser");
	sprintf(outfile,"stdout");

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate coherence between two signals using FFT (Kiss-FFT)\n");
		fprintf(stderr,"Assumes data consists of real values only\n");
		fprintf(stderr,"FFT is performed on overlapping windows in which data is de-meaned and tapered\n");
		fprintf(stderr,"USAGE:	%s [inA] [inB] [options] \n",thisprog);
		fprintf(stderr,"	[inA]: file containing the low, modulating frequency\n");
		fprintf(stderr,"	[inB]: file containing the high, modulated frequency (can be inA)\n");
		fprintf(stderr,"VALID OPTIONS (defaults in [])\n");
		fprintf(stderr,"	-dt: data type (-1=ASCII, 0-9=BINARY) [%d]\n",setdatatype);
		fprintf(stderr,"		* dt -1: input is assumed to be 1 column\n");
		fprintf(stderr,"		* dt 0-9: uchar,char,ushort,short,uint,int,ulong,long,float,or double\n");
		fprintf(stderr,"	-sf: sampling frequency of the input, in Hz [%g]\n",setsfreq);
		fprintf(stderr,"	-min1: bottom of low freq. range [-1 = default= 800/datalength]\n");
		fprintf(stderr,"	-max1: top of low freq. range [-1 = default= sr/2]\n");
		fprintf(stderr,"	-min2: bottom of high-freq range [-1=default = min1]\n");
		fprintf(stderr,"	-max2: top of high-freq range [-1=default = min1]\n");
		fprintf(stderr,"	-res: Butterworth filter resonance (0.1 to sqrt(2)=1.4142) [%g]\n",setresonance);
		fprintf(stderr,"		NOTE: low values = sharper cutoff but produces ringing\n");
		fprintf(stderr,"		NOTE: high values = gentle rolloff and dampened signal\n");
		fprintf(stderr,"	-Ft: FIR filter number of taps [%d]\n",FIRntaps);
		fprintf(stderr,"	-Fw: FIR filter bandwidth [%g]\n",FIRwidth1);
		fprintf(stderr,"	-Fb: FIR filter beta (edge bandwidth 0-10, lower=sharper but more ringing) [%g]\n",FIRbeta);
		fprintf(stderr,"	-Fs: FIR corection for phase-shift (0=NO 1=YES) [%d]\n",FIRshift);
		fprintf(stderr,"	-Fy: FIR window type (none,kaiser,sync) [%s]\n",FIRWindow);
		fprintf(stderr,"	-f: file containing start-samples for windows\n");
		fprintf(stderr,"		* this will override the -s option\n");
		fprintf(stderr,"		* if unset, windows are automatically defined to span the data \n");
		fprintf(stderr,"	-w: length of data windows passed to FFT function) (0 = auto) [%d]\n",setnbuff);
		fprintf(stderr,"		* must be an even number, not necessarily a power of two\n");
		fprintf(stderr,"		* by default, auto = 2*(sr/min)\n");
		fprintf(stderr,"		* larger window = more detailed output but lower temporal resolution\n");
		fprintf(stderr,"		* frequency resolution = sample_frequency / buffer_size\n");
		fprintf(stderr,"	-s: number of steps for the sliding window to span one buffer length [%d]\n",setstep);
		fprintf(stderr,"		* e.g. if -b 8 -s 2, the buffer moves by 8/2=4 samples per FFT\n");
		fprintf(stderr,"		* note: more steps = more data overlap (artificially high coherence)\n");
		fprintf(stderr,"	-t: tapering, 0=NO, 1=YES (Hann taper) [%d]\n",setntapers);
		fprintf(stderr,"		* note: tapering inflates coherence (same taper applied to both inputs)\n");
		fprintf(stderr,"	-p: power calculation method (0=Goertzel, 1=Butterworth+RMS, 2=FIR+RMS) [%d]\n",setpow);
		fprintf(stderr,"	-m: multiplier for high-freq. energy vector window (wavelengths) [%g]\n",setmult);
		fprintf(stderr,"	-a: windows accumulated before calculating coherence (0 = auto) [%ld]\n",setaccumulate);
		fprintf(stderr,"		* note: if not 0, must be at least 2\n");
		fprintf(stderr,"		* note: if -o 0 (avg.spectrum), auto value is \"all windows\"\n");
		fprintf(stderr,"		* note: if -o 1 or 2, auto value is smallest of 8 or \"all windows\"\n");
		fprintf(stderr,"	-adj: adjust coherence to correct for accumulation (0=NO 1=YES) [%d]\n",setadja);
		fprintf(stderr,"	-o: output style (0,1, or 2) [%d]\n",setmatrix);
		fprintf(stderr,"		* 0=average spectrum, 1=time_v_freq matrix, 2=list of columns\n");
		fprintf(stderr,"		* note: if set to 1, each line is the coherence for two buffers\n");
		fprintf(stderr,"	-g: apply Gaussian smoothing to output (avg.spectrum only) (0= none) [%d]\n",setgauss);
		fprintf(stderr,"		* note: -g must be 0 or an odd number 3 or larger)\n");
		fprintf(stderr,"		: low values vive sharper cutoffs\n");
		fprintf(stderr,"	-v: set verbocity of output to quiet (0) or report (1) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES: %s [input] [options] \n",thisprog);
		fprintf(stderr,"	%s data.txt -sf 24000 \n",thisprog);
		fprintf(stderr,"	cat data.bin | %s stdin -sf 1000 -s 8\n",thisprog);
		fprintf(stderr,"OUTPUT: \n");
		fprintf(stderr,"	if -out 0:  <frequency> <coherence>\n");
		fprintf(stderr,"	if -out 1:  matrix of coherence values, row=buffer (time), column=frequency\n");
		fprintf(stderr,"	if -out 2:  <time1> <time2> <frequency> <coherence>\n");
		fprintf(stderr,"		<time1> and <time2> bound the window in which coherence is calculated\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile1,"%s",argv[1]);
	sprintf(infile2,"%s",argv[2]);
	for(i=3;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-dt")==0)   setdatatype=atoi(argv[++i]);
			else if(strcmp(argv[i],"-f")==0)    {settimefile=1; sprintf(timefile,"%s",argv[++i]);}
			else if(strcmp(argv[i],"-sf")==0)   setsfreq=atof(argv[++i]);
			else if(strcmp(argv[i],"-min1")==0) {setmin=1;minfreq1=atof(argv[++i]);}
			else if(strcmp(argv[i],"-max1")==0) {setmax=1;maxfreq1=atof(argv[++i]);}
			else if(strcmp(argv[i],"-min2")==0) minfreq2=atof(argv[++i]);
			else if(strcmp(argv[i],"-max2")==0) maxfreq2=atof(argv[++i]);
			else if(strcmp(argv[i],"-p")==0)    setpow=atoi(argv[++i]);
			else if(strcmp(argv[i],"-res")==0)  setresonance=atof(argv[++i]);
			else if(strcmp(argv[i],"-Fw")==0)   FIRwidth1=atof(argv[++i]);
			else if(strcmp(argv[i],"-Fb")==0)   FIRbeta=atof(argv[++i]);
			else if(strcmp(argv[i],"-Ft")==0)   FIRntaps=atoi(argv[++i]);
			else if(strcmp(argv[i],"-Fs")==0)   FIRshift=atoi(argv[++i]);
			else if(strcmp(argv[i],"-Fy")==0)   snprintf(FIRWindow,16,"%s",argv[++i]);
			else if(strcmp(argv[i],"-w")==0)    setnbuff=atoi(argv[++i]);
			else if(strcmp(argv[i],"-s")==0)    setstep=atoi(argv[++i]);
			else if(strcmp(argv[i],"-t")==0)    setntapers=atoi(argv[++i]);
			else if(strcmp(argv[i],"-o")==0)    setmatrix=atoi(argv[++i]);
			else if(strcmp(argv[i],"-g")==0)    setgauss=atoi(argv[++i]);
			else if(strcmp(argv[i],"-m")==0)    setmult=atof(argv[++i]);
			else if(strcmp(argv[i],"-a")==0)    setaccumulate=atoi(argv[++i]);
			else if(strcmp(argv[i],"-adj")==0)  setadja=atoi(argv[++i]);
			else if(strcmp(argv[i],"-v")==0)    setverb=atoi(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	/* CHECK OPTIONAL ARGUMENTS */
	if(setsfreq>0.0) sample_interval=1.0/setsfreq;
	else { fprintf(stderr,"\n--- Error [%s]: bad sample freq. [-sf %g] must be >0\n\n",thisprog,setsfreq);exit(1);}
	if(setnbuff<0) { fprintf(stderr,"\n--- Error [%s]: -b [%d] must be 0 (auto) or a positive number\n\n",thisprog,setnbuff);exit(1);}
	if(setmatrix!=0&&setmatrix!=1&&setmatrix!=2) { fprintf(stderr,"\n--- Error [%s]: bad output setting [-o %d]:  should be 0,1 or 2\n\n",thisprog,setmatrix);exit(1);}
	if(setmax<0) maxfreq1=setsfreq/2.0;
	if(maxfreq1>(0.5*setsfreq)) { fprintf(stderr,"\n--- Error [%s]: -max [%g] cannot be greater 1/2 the  sample freq. [%g]\n\n",thisprog,maxfreq1,setsfreq);exit(1);}
	if(minfreq1<=0 && minfreq1!=-1.0) { fprintf(stderr,"\n--- Error [%s]: -min [%g] must -1 (auto) or greater than zero\n\n",thisprog,minfreq1);exit(1);}
	if(minfreq1>maxfreq1) { fprintf(stderr,"\n--- Error [%s]: -min [%g] must be less than -max [%g]\n\n",thisprog,minfreq1,maxfreq1);exit(1);}
	if(minfreq2>maxfreq2 && maxfreq2>0) { fprintf(stderr,"\n--- Error [%s]: -min2 [%g] must be less than -max2 [%g]\n\n",thisprog,minfreq2,maxfreq2);exit(1);}
	if(setmult<1){ fprintf(stderr,"\n--- Error [%s]: -m [%g] must >= 1\n\n",thisprog,setmult);exit(1);}
	if(setverb<0&&setverb!=1) { fprintf(stderr,"\n--- Error [%s]: -v [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setpow<0||setpow>2) { fprintf(stderr,"\n--- Error [%s]: -p [%d] must be 0-2\n\n",thisprog,setpow);exit(1);}
	if((setgauss<3&&setgauss!=0)||(setgauss%2==0 && setgauss!=0)) { fprintf(stderr,"\n--- Error [%s]: -g [%d] must be 0 or an odd number 3 or larger\n\n",thisprog,setgauss);exit(1);}
	if(setaccumulate<2&&setaccumulate!=0) { fprintf(stderr,"\n--- Error [%s]: -a [%ld] must be 0 or greater than 2\n\n",thisprog,setaccumulate);exit(1);}
	if(setntapers!=0&&setntapers!=1) { fprintf(stderr,"\n--- Error [%s]: -t [%d] must be 0 or 1\n\n",thisprog,setntapers);exit(1);}
	if(FIRbeta<0||FIRbeta>10) {fprintf(stderr,"\n--- Error[%s]: -beta (%g) must be 0-10\n\n",thisprog,FIRbeta);exit(1);}
	if(FIRshift!=0&&FIRshift!=1) {fprintf(stderr,"\n--- Error[%s]: -shift (%d) must be 0-1\n\n",thisprog,FIRshift);exit(1);}
	if(setadja<0||setadja>1) { fprintf(stderr,"\n--- Error [%s]: -adj [%d] must be 0 or 1\n\n",thisprog,setadja);exit(1);}

	if(setdatatype==0||setdatatype==1) datasize=(off_t)sizeof(char);
	else if(setdatatype==2||setdatatype==3) datasize=(off_t)sizeof(short);
	else if(setdatatype==4||setdatatype==5) datasize=(off_t)sizeof(int);
	else if(setdatatype==6||setdatatype==7) datasize=(off_t)sizeof(long);
	else if(setdatatype==8) datasize=(off_t)sizeof(float);
	else if(setdatatype==9) datasize=(off_t)sizeof(double);
	else if(setdatatype!=-1) {fprintf(stderr,"\n--- Error[%s]: data type (-t %d) must be -1 (ascii) or 0-9 \n\n",thisprog,setdatatype); exit(1);}

	/********************************************************************************/
	/********************************************************************************/
	/* STORE DATA - ASCII OR BINX - IN BOTH CASES, CONVERT TO FLOAT  */
	/********************************************************************************/
	/********************************************************************************/
	if(setverb>0) {
		fprintf(stderr,"%s\n",thisprog);
		fprintf(stderr,"	* Reading data...\n");
	}

	nbad_a=0;
	nbad_b=0;

	if(setdatatype==-1) {
	 	m=n=0;
		fpin1=fopen(infile1,"r");
		if (fpin1==NULL) { fprintf(stderr,"\n--- Error [%s]: could not open file #1 \"%s\"\n\n",thisprog,infile1);exit(1);}
		while(fgets(line,MAXLINELEN,fpin1)!=NULL) {
 			if(sscanf(line,"%f",&a)!=1 || !isfinite(a)) {a=NAN;nbad_a++;}
 			if((data_a=(float *)realloc(data_a,(m+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
 			data_a[m++]=a;
		}
		fpin2=fopen(infile2,"r");
		if (fpin2==NULL) { fprintf(stderr,"\n--- Error [%s]: could not open file #2 \"%s\"\n\n",thisprog,infile2);exit(1);}
		while(fgets(line,MAXLINELEN,fpin2)!=NULL) {
 			if(sscanf(line,"%f",&b)!=1 || !isfinite(b)) {b=NAN;nbad_b++;}
 			if((data_b=(float *)realloc(data_b,(n+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
 			data_b[n++]=b;
		}
		fclose(fpin1);
		fclose(fpin2);
	}
	else {
		params[0]=(off_t)setdatatype;
		params[1]=(off_t)0; // bytes to skip before numbers begin
		params[2]=(off_t)0; // first number to read
		params[3]=(off_t)0; // total numbers to read (0=all in file)

		data_a= xf_readbin2_f(infile1,params,message);
		m=(long)params[3]; // parameters[3] is reset to the number of elements (bytes/datasize) by xf_readbin2_f
		if(data_a==NULL) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}

		params[3]=(off_t)0; // total numbers to read (0=all in file)
		data_b= xf_readbin2_f(infile2,params,message);
		n=(long)params[3]; // parameters[3] is reset to the number of elements (bytes/datasize) by xf_readbin2_f
		if(data_b==NULL) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}

		for(i=0;i<m;i++) if(!isfinite(data_a[i])) nbad_a++;
		for(i=0;i<n;i++) if(!isfinite(data_b[i])) nbad_b++;
	}

	//TEST: for(i=0;i<n;i++) printf("%g\t%g\n",data_a[i],data_b[i]); goto END1;

	/* check the integrity of the data */
	if(n==0 || nbad_a==n) {fprintf(stderr,"\n--- Error[%s]: no valid data in file %s\n\n",thisprog,infile1);exit(1);};
	if(m==0 || nbad_b==m) {fprintf(stderr,"\n--- Error[%s]: no valid data in file %s\n\n",thisprog,infile2);exit(1);};
	if(n!=m) {fprintf(stderr,"\n--- Error[%s]: unequal amount of data in %s (%ld) and %s (%ld)\n\n",thisprog,infile1,n,infile2,m);exit(1);};

	/* interpolate across NAN's or INF's */
	if(nbad_a>0) {k= xf_interp3_f(data_a,(size_t)n); if(k<0) {fprintf(stderr,"\n\t --- Error [%s]: input \"%s\" contains no valid numbers\n\n",thisprog,infile1);exit(1);}}
	if(nbad_b>0) {k= xf_interp3_f(data_b,(size_t)m); if(k<0) {fprintf(stderr,"\n\t --- Error [%s]: input \"%s\" contains no valid numbers\n\n",thisprog,infile2);exit(1);}}

	if(setpow==1) {
		/* de-mean the data - generally advisable before filtering */
		bb=0.0;
		for(i=0;i<n;i++) bb+=data_b[i];
		b=(float)(bb/(double)n);
		for(i=0;i<n;i++) data_b[i]/=b;

		/* apply padding to both inputs for Butterworth Filterimg */
		npad=(size_t)(setsfreq/maxfreq2);
		if(npad<200) npad=200;
		if(npad>n) npad=n;
		data_a = xf_padarray3_f(data_a,(long)n,(long)npad,3,message);
		data_b = xf_padarray3_f(data_b,(long)n,(long)npad,3,message);
		if(data_a==NULL) {fprintf(stderr,"\n\t --- Error [%s]: %s\n\n\n",thisprog,message);free(data_a);exit(1);}
		if(data_b==NULL) {fprintf(stderr,"\n\t --- Error [%s]: %s\n\n\n",thisprog,message);free(data_b);exit(1);}
		n= n+npad+npad;
	}


	/********************************************************************************
	SET UP SETNBUFF (BUFFER LENGTH) & PARTNBUFF (PART-BUFFER LENGTH DETERMINED BY SET-STEP)
	********************************************************************************/
	/* determine minimum frequency if not explicitly defined */
	if(setmin<0) {
		a= (4.0*setsfreq)/(double)n; /* cycle=1/4th data length allows for 2 cycles per window and accumulation over 2 windows  */
		/* for a decent coherogram, increase minfreq1 by up to 200x (yielding ~400 FFT outputs) */
		minfreq1= a;
		b=maxfreq1/2.0;
		c=minfreq1*200;
		while(minfreq1<b && minfreq1<c) minfreq1+=a;
	}
	if(minfreq2<=0.0) minfreq2=minfreq1;
	if(maxfreq2<=0.0) maxfreq2=maxfreq1;

	/* if setstep is undefined, use 0% overlapping windows (1 step spans the buffer) */
	if(setstep<=0) setstep=1;
	/* calculate the number of samples representing the longest wavelength to be analyzed */
	a= setsfreq/minfreq1;
	/* calculate optimal partnbuff : the fraction of two wavelengths spanned by each buffer-shift */
	y= 2 * (int)(0.5+(a/setstep)); if(y%2!=0) y++;
	/* calculate optimal total buffer length */
	z= y*setstep;
	if(z>n) z-=2;
	/* now assign values to setnbuff and partnbuff - compare with optimal values */
	if(setnbuff<1) { setnbuff=z, partnbuff=y; }
	else {
		if(setnbuff<z && setverb>0) { fprintf(stderr,"\n--- Warning [%s]: buffer length [-b %d] is less than the optimal length [%d] for the minimum frequency [-min %g]\n\n",thisprog,setnbuff,z,minfreq1);}
		if(setnbuff%2 != 0) { fprintf(stderr,"\n--- Error [%s]: buffer length [-b %d] must be an even number\n\n",thisprog,setnbuff);exit(1);}
		if(setnbuff%setstep != 0) { fprintf(stderr,"\n--- Error [%s]: step size [-s %d] must be a factor of the buffer length [%d]\n\n",thisprog,setstep,setnbuff);exit(1);}
		partnbuff=setnbuff/setstep;
	}

	/* DEFINE ADDITIONAL BUFFER SIZES */
	setnbuff_f= (float)setnbuff;
	morenbuff= setnbuff+partnbuff; // size of buffer for storage during reading = buffersize + stepsize
	halfnbuff= setnbuff/(int)2;

	/* ALLOCATE EXTRA MEMORY FOR DATA IN CASE DATA LENGTH IS NOT AN EVEN NUMBER OF BUFFERS */
	/* this is repeated for data_2b (energy envelope) in the analysis loop below */
	j= n+setnbuff;
	if((data_a=(float *)realloc(data_a,(j*sizeoffloat)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((data_b=(float *)realloc(data_b,(j*sizeoffloat)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((data_b2=(float *)realloc(data_b2,(j*sizeoffloat)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((data_b3=(float *)realloc(data_b3,(j*sizeoffloat)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	/* pad this extra space with zeros */
	for(i=n;i<j;i++) {
		data_a[i]=0.0;
		data_b[i]=0.0;
	}
	/* make sure there's enough data for at least one buffer! */
	if(n<setnbuff) {fprintf(stderr,"\n--- Error [%s]: number of data points [%ld] is less than window size [%d]\n\n",thisprog,n,setnbuff);exit(1);}


	/********************************************************************************
	SET UP THE START-TIMES (WINDEX) FOR EACH BUFFER TO BE ANALYZED
	********************************************************************************/
	if(settimefile==0) {
		windex= xf_window1_l(n,partnbuff,0,&nwin); /* equal-sized windows incremented by partnbuff */
		if(nwin<0) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	}
	/* If a time-file was specified, use the start times listed in it instead */
	else if(settimefile==1) {
		if((fpin1=fopen(timefile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,timefile);exit(1);}
		nwin=0;
		while(fgets(line,MAXLINELEN,fpin1)!=NULL) {
			if(sscanf(line,"%ld",&ii)!=1 || !isfinite(aa)) continue;
			if((windex=(long *)realloc(windex,(nwin+1)*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);free(data_a);free(data_b);free(data_b2);exit(1);};
			windex[nwin++]=ii;
		}
		fclose(fpin1);
		if(nwin<1) {fprintf(stderr,"\n--- Error[%s]: time-file \"%s\" has no valid entries\n\n",thisprog,timefile);free(data_a);free(data_b);free(data_b2);exit(1);}
		setstep=1;
	}

	/********************************************************************************
	CHECK OR AUTO-DEFINE ACCUMULATION
	********************************************************************************/
	if(setaccumulate==0) {
		if(setmatrix==0) {
			setaccumulate=nwin;
		}
		else {
			if(nwin<8) setaccumulate=nwin;
			else setaccumulate=8;
		}
	}
	else if(setaccumulate>nwin) {fprintf(stderr,"\n--- Error[%s]: not enough windows (%ld) for specified accumulation (%ld)\n\n",thisprog,nwin,setaccumulate);exit(1);}

	/********************************************************************************
	CALCULATE THE COHERENCE ADJUSTMENT BASED ON ACCUMULATION
	coherence = (coherence-adja1) * adja2;
	********************************************************************************/
	adja1=1.0/setaccumulate;
	adja2=1.0/(1.0-adja1);


	/********************************************************************************/
	/* INITIALIZE KISS-FFT CONFIGURATION AND OTHER VARIABLES */
	/********************************************************************************/
	if(setverb>0) fprintf(stderr,"	* Initializing Kiss-FFT variables...\n");
	/* initialize kiss-fft variables: cfgr, ffta_a, fft_b */
	kiss_fftr_cfg cfgr = kiss_fftr_alloc( setnbuff ,0,0,0 );
	/* allocate memory for fft_a, fft_b  and work some Kiss-FFT magic */
	aaa= setnbuff * setaccumulate * sizeof(kiss_fft_cpx*);
	kiss_fft_cpx* fft_a = (kiss_fft_cpx*)malloc(aaa);
	kiss_fft_cpx* fft_b = (kiss_fft_cpx*)malloc(aaa);

	/* allocate memory for working variables */
	if((freq=(float*)calloc(setnbuff,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // buffer which is passed to the FFT function, copied from buff1_a and float-sized to hold placeholder imaginary components
	if((tempdata_a=(float*)calloc(setnbuff,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // buffer which is passed to the FFT function, copied from buff1_a and float-sized to hold placeholder imaginary components
	if((tempdata_b=(float*)calloc(setnbuff,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // buffer which is passed to the FFT function, copied from buff1_a and float-sized to hold placeholder imaginary components
	if((sumspect_aa=(float*)calloc(setnbuff,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((sumspect_bb=(float*)calloc(setnbuff,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((sumspect_abr=(float*)calloc(setnbuff,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((sumspect_abi=(float*)calloc(setnbuff,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((tapers=(double*)calloc(setnbuff,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds the tapers
	/* allocate a bit extra for coherence, as it may be used to hold the freq values as well for BINX output */
	if((coherence=(float*)calloc((setnbuff+2),sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};

	/* CALCULATE THE MIN AND MAX INDEX CORRESPONDING TO THE USER-SPECIFIED FREQUENCY RANGE */
	/* This controls which elements of the FFT results are calculated and output */
	indexa=(int)(minfreq1*setnbuff*sample_interval); if(indexa<1) indexa=1;
	indexb=(int)(maxfreq1*setnbuff*sample_interval); if(indexb>=setnbuff) indexb=setnbuff-1;

	/* SET SCALING (1/n) and SCALING-SQUARED (1/n^2) */
	scaling1= 1.0/(float)(setnbuff);
	scaling2= scaling1 * scaling1;


	/********************************************************************************
	DEFINE FREQUENCY VALUES ASSOCIATED WITH EACH FFT OUTPUT
	********************************************************************************/
	for(i=indexa;i<=indexb;i++) {
			/* calculate frequency, round to nearest 0.001 */
			a= (float) (((double)(i)/(double)sample_interval) / (double)setnbuff_f );
			/* round to 3 digits - NOTE: this will actually add a few zeros after the 3rd digit but may not exactly round it  - sufficient for printing however */
			freq[i]= (float)xf_round1_f(a,0.001,0);
	}

	/* set the frequency resolution of the output */
	freqres= setsfreq/(double)setnbuff;


	/********************************************************************************/
	/* DEFINE THE HANN TAPER */
	/********************************************************************************/
	d=twopi/(setnbuff-1.0);
	for(i=0;i<setnbuff;i++) tapers[i] = 0.5*(1.-cosf(i*d));



	/********************************************************************************
	OPEN OUTPUT FILE OR STREAM
	********************************************************************************/
	if(strcmp(outfile,"stdout")==0) {
		fflush(stdout);
		fpout=stdout;
	}
	else {
		if(setasc==1) fpout=fopen(outfile,"w");
		if(setasc==0) fpout=fopen(outfile,"wb");
		if(fpout==NULL) {
			fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile);
			exit(1);
	}}

	/********************************************************************************/
	/********************************************************************************
	ANALYZE THE DATA
	********************************************************************************/
	/********************************************************************************/
	if(setverb>0) fprintf(stderr,"	* Analyzing the data...\n");

	/* FOR EACH FREQUENCY, FILTER THE DATA AND PROCESS THE WINDOWS */
	freq2step= ((maxfreq2-minfreq2)+1.0)/100.0;
	for(freq2=maxfreq2; freq2>=minfreq2; freq2-=freq2step) {

		/* calculate the energy envelope for data_b - assign to data_b2 */
		if(setpow==0) {
			ii=(size_t)(setmult*(setsfreq/freq2)); // window size = setmult*wavelength
			x= xf_power_goertzel2_f(data_b, data_b2,n,setsfreq,freq2,ii,message);
			if(x<0) { fprintf(stderr,"\n--- Error [%s]: %s\n\n",thisprog,message); goto END1; }
		}
		if(setpow==1) {
			x= xf_filter_bworth2_f(data_b,data_b2,data_b3,n,setsfreq,freq2,freq2,setresonance,message);
			if(x!=0) { fprintf(stderr,"\n--- Error [%s]: %s\n\n",thisprog,message); goto END1; }
			ii=(size_t)(setmult*(setsfreq/freq2)); // window size = setmult*wavelength
			x= xf_rms2_f(data_b3,data_b2,n,ii,message);
			if(x!=0) { fprintf(stderr,"\n--- Error [%s]: %s\n\n",thisprog,message); goto END1; }
		}
		if(setpow==2) {
			/* calculate the filter coefficients */
			FIRomegaC= 2.0 * (freq2/setsfreq) ;
			FIRwidth2= 2.0 * (FIRwidth1/setsfreq) ;
			FIRcoefs= xf_filter_FIRcoef1(FIRntaps,FIRpass,FIRomegaC,FIRwidth2,FIRWindow,FIRbeta,message);
			if(FIRcoefs==NULL) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
			/* apply the coefficients to the data and output */
			x= xf_filter_FIRapply2_f(data_b,data_b3,n,FIRcoefs,FIRntaps,FIRshift,message);
			if(x!=0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
			/* calculate the poswer */
			ii=(size_t)(setmult*(setsfreq/freq2)); // window size = setmult*wavelength
			x= xf_rms2_f(data_b3,data_b2,n,ii,message);
			if(x!=0) { fprintf(stderr,"\n--- Error [%s]: %s\n\n",thisprog,message); goto END1; }
		}

		/* initialize coherence values (cumulative) to zero */
		for(i=0;i<setnbuff;i++) coherence[i]=0.0;
		accumulate=0;
		nout=0;

		/* FOR EACH WINDOW, SET POINTERS TO APPROPRIATE DATA AND ANALYZE */
		for(window=0;window<nwin;window++) {

			/* set index to data for the current window */
			pdata_a= data_a+  windex[window];
			pdata_b= data_b2+ windex[window];


			/* de-mean & apply the taper to a copy of the data */
			a=0.0; for(i=0;i<setnbuff;i++) a+=pdata_a[i]; a*=scaling1;
			b=0.0; for(i=0;i<setnbuff;i++) b+=pdata_b[i]; b*=scaling1;
			if(setntapers>0) {
				for(i=0;i<setnbuff;i++) {
					tempdata_a[i]= (pdata_a[i]-a) * tapers[i];
					tempdata_b[i]= (pdata_b[i]-b) * tapers[i];
			}}
			else {
				for(i=0;i<setnbuff;i++) {
					tempdata_a[i]= pdata_a[i] - a;
					tempdata_b[i]= pdata_b[i] - b;
			}}

			/* set pointer to beginning of block to be written to in circular fft buffers */
			k = accumulate * setnbuff;
			/* call FFT function */
			kiss_fftr(cfgr,tempdata_a,fft_a+k);
			kiss_fftr(cfgr,tempdata_b,fft_b+k);
			/* update accumulation ring-buffer position */
			accumulate++; if(accumulate>=setaccumulate) accumulate=0;
			/* if enough windows have been accumulated, calculate coherence */
			if(window>=(setaccumulate-1)) {
				nout++;
				for(i=indexa;i<=indexb;i++) {
					/* reset the summed spectra */
					sumspect_aa[i]=0.0;
					sumspect_bb[i]=0.0;
					sumspect_abr[i]=0.0;
					sumspect_abi[i]= 0.0;
					/* accumulate the spectra currently in the buffer */
					for(j=0;j<setaccumulate;j++) {
						k = j * setnbuff + i;
						ar = fft_a[k].r;
						ai = fft_a[k].i;
						br = fft_b[k].r;
						bi = fft_b[k].i;
						/* accumulate the auto- and cross-spectra (for traces a & b, r=real, i=imaginary): note that the usual scaling of autospectra by 1/setnbuff^2 (for amplitude calculation) is omitted, as this term cancels in the coherence calculation anyway */
						sumspect_aa[i] += (ar*ar + ai*ai); // unscaled auto-spectrum for series a
						sumspect_bb[i] += (br*br + bi*bi); // unscaled auto-spectrum for series b
						sumspect_abr[i]+= (br*ar - bi*(-ai)); // unscaled cross-spectrum for a vs b, real part
						sumspect_abi[i]+= (br*(-ai) + bi*ar); // unscaled cross-spectrum for a vs b, imaginary part
					}
					/* calculate coherence  - note that we do not need to use g*g in the second step because we are not using the sqrt of the cross-spectrum */
					f = sumspect_aa[i] * sumspect_bb[i] ;
					g = sumspect_abr[i]*sumspect_abr[i] + sumspect_abi[i]*sumspect_abi[i] ;
					if(f!=0) h = g / f;
					else h=0.0;

					/*
					correct coherence for accumulation level (if accumulation=2, baseline=0.5, etc.)
					adja1= 1.0/setaccumulate
					adja2= 1.0/(1.0-setadja1)
					*/
					if(setadja==1) h = (h-adja1) * adja2;

					coherence[i]+=  h;
			}}

		} /* END OF FOR(WINDOW=0 TO NWIN)  LOOP */


		if(setmatrix==0) {
			/* normalize by the number of coherence values calculated */
			bb=(double)nout; for(i=indexa;i<=indexb;i++) coherence[i]/=bb;
			/* smooth portion of data coherence was actually calculated for */
			if(setgauss!=0) xf_smoothgauss1_f((coherence+indexa),(size_t)(indexb-indexa+1),setgauss);
			/* output */
			for(i=indexa;i<indexb;i++) fprintf(fpout,"%.3f ", coherence[i]);
			fprintf(fpout,"%.3f\n",coherence[indexb]);

		}

	} // END OF FOR FREQ= LOOP

	/********************************************************************************
	CLOSE THE OUTPUT FILE IF REQUIRED
	********************************************************************************/
	if(strcmp(outfile,"stdout")!=0) fclose(fpout);

END1:

	// FREE MEMORY
	free(tempdata_a);
	free(tempdata_b);
	free(sumspect_aa);
	free(sumspect_bb);
	free(sumspect_abr);
	free(sumspect_abi);
	free(coherence);
	free(tapers);
	free(cfgr);
	free(fft_a);
	free(fft_b);
	free(freq);
END2:
	free(data_a);
	free(data_b);
	free(data_b2);
	free(data_b3);
	free(windex);

	/* PRINT DIAGNOSTICS TO STDERR */
	if(setverb==1) {
		if(setpow==1) n=n-npad-npad; /* because for Butterworth filtering, padding may have been applied */
		fprintf(stderr,"\n");
		fprintf(stderr,"	input_length= %ld samples\n",n);
		fprintf(stderr,"	sample_rate= %g\n",setsfreq);
		fprintf(stderr,"	duration= %g seconds\n",(n/setsfreq));
		fprintf(stderr,"	minfreq1= %g\n",minfreq1);
		fprintf(stderr,"	maxfreq1= %g\n",maxfreq1);
		fprintf(stderr,"	minfreq2= %g\n",minfreq2);
		fprintf(stderr,"	maxfreq2= %g\n",maxfreq2);
		fprintf(stderr,"	indexa= %d\n",indexa);
		fprintf(stderr,"	indexb= %d\n",indexb);
		// calculate window overlap
		a=100*(1.0-1.0/setstep);
		fprintf(stderr,"	fft_window= %d samples\n",setnbuff);
		fprintf(stderr,"	fft_step= %d \n",setstep);
		fprintf(stderr,"	fft_overlap= %g %%\n",a);
		fprintf(stderr,"	setaccumulate= %ld\n",setaccumulate);
		fprintf(stderr,"	setntapers= %d\n",setntapers);
		fprintf(stderr,"	frequency_resolution= %g Hz\n",freqres);
		fprintf(stderr,"	temporal_resolution= %g seconds\n",(partnbuff/setsfreq));
		fprintf(stderr,"	temporal_shift= %g seconds\n",((setnbuff/2.0)/setsfreq));
		fprintf(stderr,"	nwin= %ld\n",nwin);
		fprintf(stderr,"	nout= %ld\n",nout);

		fprintf(stderr,"\n");
	}
}
