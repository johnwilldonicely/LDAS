#define thisprog "xe-fftpow1"
#define TITLE_STRING thisprog" v 31: 11.November.2014 [JRH]"
#define MAXLINELEN 1000

#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "kiss_fftr.h"


/*
<TAGS>signal_processing spectra</TAGS>

REFERENCES:
http://www.ni.com/white-paper/4278/en/
http://zone.ni.com/reference/en-XX/help/371361B-01/lvanlsconcepts/compute_amp_phase_spectrums/
http://zone.ni.com/reference/en-XX/help/371361B-01/lvanlsconcepts/convert_to_log_units/

NOTES/TO DO:

	* consider removing multitaper altogether - save step of copying ampall to amp
	* consider renaming amp to spect given new options of units (RMS, etc)
	* rename counter variables to ii,jj,kk etc to indicate type as size_t
	* I think Fval (and degf?) are not being generated properly at present?
	* how to calculate confidence intervals and significance tests for each F statistic?
	* how to average df and F across multiple window???

	*** implement similar changes made to xe-fftcoh1 between 16.Sept and 22.October 2013

v 31: 11.November.2014 [JRH]
	- bugfix:
		- previously, large files authomatically generated very small minimum frequency values
		- consequently the FFT window had to be very large and memory problems could occur
		- now the default minimum is 0.1Hz if the data length permits, or higher as required

v 30: 22 June 2014 [JRH]
	- brinf min/max auto-definition together in the code, as well as checks for validity
	- pre-set sample frequency to 1KHz instead of 1Hz to reduce chance of incompatible


v 29: 6 June 2014 [JRH]
	- add capability to output spectrum in peak-amplitude, RMS, RMS-squared, or dB conversion of either of these (-u option)
	- add pre-calculation of frequency values for FFT output

v 28: 11.April.2014 [JRH]
	- allow Gaussian smoothing of the average FFT output
	- allow log-transform of amplitude values

 v 27: 3.March.2014 [JRH]
 	- only report too-small window size if verbose output is selected - this is not a serious warning

v 26: 24.February.2014 [JRH]
	- file defining window-times is now expected to express this in samples, not seconds
	- sensible, as other values used by the program (eg. the window size) are expressed in samples

v 25: 29.November.2013 [JRH]
	- remove stdbool header (not needed)
	- remove unnecesary dropping of windows if buffer overruns - this is already handled by expanding data and padding with zeros

v 24: 22.Oct.2013 [JRH]
	- small tweak to improve matrix output - no extra spaces at end of lines

v 23: 16.September.2013 [JRH]
	- bugfix: ensure output file is closed if not stdout (in practice this option is not enabled anyway, yet).
	- automatic min-frequency now determined by total data length
	- instructions updated to describe how buffer length determines frequency resolution (resolution = setsamplefreq/setnbuff)
	- revert to using Hann taper if only one taper is specified - alteration to amplitude is more reliable and aside from scaling looks almost exactly like the first DPSS taper anyway


v 22: 9.September.2013 [JRH]
	- now correctly stores FFT results to taper-specific memory
	- added missing free() command for tapsum and lambda arrays
	- now use Slepians (DPSS) for all tapers
		- previously used Hann if setntapers = 1, Sleppian tapers if setntapers >1
		- the results from the Hann and the first taper in the Sleppian sequence are virtually identical
		- this simplifies the code a little
		- plus the Slepian has slightly lower side lobes
		- plus the Slepian can be tweaked using the -p option (order of the taper)

v 21: 1.July.2013 [JRH]
	- switch to using size_t for readbinx functions and other counters

v 20: 28.June.2013 [JRH]
	- output tweaks
	- minor fixes to xf_writebinx function

v 19: 25.June.2013 [JRH]
	- check windows BEFORE processing data - warn and remove windows falling past the end of the data
	- remove sqrt(2) corrections to spectra - instead, just multiply spectrum by two AFTER taking sqrt in the initial calculation, instead of placing 2 inside the sqrt operator

v 18: 12.June.2013 [JRH]
	- program now passes file-pointer to binary read-write functions - allows piecemeal reading/writing of files

v 17: 10.June.2013 [JRH]
	- switch from accepting binary short files to BINX file format

v 16: 3.June.2013 [JRH]
	- Kiss-FFT cfgr array now freed upon exit
	- more use of goto to ensure data is freed upon exit
	- use size_t for variable-type size definitions
	- added capability to read binary input (16-bit number stream - i.e. short integer)

v 15: 3.May.2013 [JRH]
	- add option for no tapering at all (-t 0 )

v 14: 21.March.2013 [JRH]
	- use new function xf_window1_l to define windows - ensures last window has same number of elements in it

v 13: 22.February.2013 [JRH]
	- improved automatic calculation of optimal buffer size
	- now, setstep is factored into the calculation so if setnbuff is automatic, it is always a factor of setstep (less chance for error messages)

v 12: 5.February.2013 [JRH]


v 11: 14.January.2013 [JRH]
	- switch to method in which all data is read into memory first, instead of buffered reading
	- add ability to specify a time-file holding start-times for triggered FFT, instead of analyzing sequential buffers
	- windex[nwin] holds start times for each window - bufftot=nwin and is now a redundant variable
	- NOTE that the length of each buffer is still fixed
	- NOTE this option is intended for large numbers of small chunks of data (e.g. theta cycles) rather than behavioural/experimental events

v 10: 8.January.2013 [JRH]
	- include calculate ion F-statistic for each frequency (xf_mtm_F - note this function must include the kiss_fftr.h header)

v 9: 4.January.2013 [JRH]
	- switch to using adaptive weighted averaging of spectra for multitaper (xf_mtm_spectavg1)
	- allow manual setting of the order of the tapers (-p option)
		- note that scaling the order of the sleppians to the number of tapers prevents failed iterrations in mtm_spectavg1
		- also ensures all tapers trend to zero at the edge of the buffer
		- however it also produces very smeared spectra (high spectral leakage)
		- in contrast anchoring the order of the taper to (say) 3 or 4 keeps the spectrum sharp, but increases the number of failed iterrations

v 8: 2.January.2013 [JRH]
	- implement multi-taper method (option -t >1, 5 is typical)
	- automatic setting of the order of the tapers (ntapers+1)
	- mean-correction of data optional - main benefit to multitaper, where it seems to reduce artificially high low-frquency power with larger numbers of tapers

v 7: 10.December.2012 [JRH]
	- bugfix - now reports correct trial duration
	- add -v option for controlling verbocity of output
	- now allows buffer size to be any even number (seems just as efficient as using a power of two)
	- store taper as 2-dimensional array initialized after setnbuff is defined
		- this reduces calculations required for each buffer
		- also paves the way for multitaper analysis if required

v 6: 30.November.2012 [JRH]
	- FINALLY got scaling correct (back to using 1/setnbuff)
	- output is in units RMS power

v 5: 28.November.2012 [JRH]
	- correction to scaling to account for the fact that we only analyze a portion of the buffer for the ranfge of frequencies of interest

v 4: 27.November.2012 [JRH]
	- apply interpolation to missing values within each buffer before calculating mean and applying taper
			- linear interpolation is used - no overshoot or real data possible
	- assume input is real

v 3: 19.November.2012 [JRH]
	- bugfix - now properly stores input as "float" instead of "double-precision float"

v 2: 9.November.2012 [JRH]
	- add memory check
	- bugfix for transferring data from temp variable to buffer 1


*/

/* external functions start */
long *xf_window1_l(long n, long winsize, int equalsize, long *nwin);
long xf_interp3_f(float *data, long ndata);
int xf_mtm_slepian1(int num_points, int setntapers, float npi, double *lam, double *tapers, double *tapsum);
int xf_mtm_spectavg1(double *sqr_spec,double *lambda, int nwin, int nfreq, double *ares, double *degf, double avar);
void xf_mtm_F(kiss_fft_cpx *fft, int nf, int nwin, float *Fval, double *tapsum);
float xf_round1_f(float input, float setbase, int setdown);
double xf_round1_d(double input, double setbase, int setdown);
size_t xf_readbinx1(FILE *fpin, void **data, size_t *params, char *message);
size_t xf_writebinx1(FILE *fpout, void *data, size_t *params, size_t ntowrite, char *message);
int xf_smoothgauss1_d(double *original, size_t arraysize,int smooth);
// NOTE: the following function declarations are commented out to avoid re-initialization in kiss headers,
// They are included here only so xs-progcompile (which won't detect that they are commented out) will include them during compilation
/*
void kiss_fftr(kiss_fftr_cfg st,const kiss_fft_scalar *timedata,kiss_fft_cpx *freqdata);
void kiss_fft(kiss_fft_cfg cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout);
*/
/* external functions end */


int main(int argc, char *argv[]) {
	/* general-use variables */
	char infile[256],outfile[256],line[MAXLINELEN],message[MAXLINELEN],*pline,*pcol;
	short nbinbuff=1000,binbuffread,binbuff[nbinbuff];
	int q,r,s,t,x,y,z,stat=0;
	size_t sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	size_t i,j,k,m,n;
	float a,b,c,d,e,f;
	double aa,bb,cc,dd,ee,ff,gg,twopi=2.0*M_PI;
	FILE *fpin,*fpout;
	/* program-specific variables */
	size_t nbuff,halfnbuff,partnbuff,morenbuff,nbad=0;
	size_t window,nwin=0,*windex=NULL;
	float *buff2=NULL;
	float *data=NULL,*pdata,*freq=NULL,*Fval=NULL,sum=0.0,mean,sample_interval=0.0F,scaling1;
	double *tapers=NULL,*lambda=NULL,*tapsum=NULL,*amp=NULL,*ampmean=NULL,*ampall=NULL,*degf=NULL,ar,ai;
	// binary file read variables
	void *data0=NULL;
	size_t params[6],ntowrite;
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
	char timefile[256];
	int indexa=-1,indexb=-1,setnbuff=-1,setnbuffplus1,setmin=-1,setmax=-1,setstep=2,setmatrix=0,setverb=0,setorder=-1,setgauss=0,setunits=0;
	int setntapers=1,setmean=1,settimefile=0,setasc=1;
	float setsfreq=1000.0,minfreq=1.0,maxfreq=100.0,setnbuff_f;
	double time;

	sprintf(outfile,"stdout");

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"Use KISS-FFT functions to produce a power spectrum density output\n");
		fprintf(stderr,"Requires input from a file or standard input, one column only\n");
		fprintf(stderr,"FFT is performed on overlapping chunks of buffered data\n");
		fprintf(stderr,"Power at each frequency is then averaged for entire file\n");
		fprintf(stderr,"Produces two columns: freq, power\n");
		fprintf(stderr,"USAGE:	%s [input] [options] \n",thisprog);
		fprintf(stderr,"	[input]: provide a filename or \"stdin\" to receive piped data\n");
		fprintf(stderr,"VALID OPTIONS (defaults in [])\n");
		fprintf(stderr,"	-asc: is input/output ASCII? (1=YES, 0=NO, BINX binary input is assumed) [%d]\n",setasc);
		fprintf(stderr,"	-sf: sampling frequency (Hz) of the input [%g]\n",setsfreq);
		fprintf(stderr,"	-min: lowest frequency to output [default= 2*sf/n or 0.1, whichever is higher]\n");
		fprintf(stderr,"	-max: highest frequency to output [default= sf/2]\n");
		fprintf(stderr,"	-b: length of buffers (windows) of data passed to FFT function) [-1 = auto]\n");
		fprintf(stderr,"		* must be an even number, not necessarily a power of two\n");
		fprintf(stderr,"		* by default, auto = 2*(sr/min)\n");
		fprintf(stderr,"		* longer window = more detailed output but lower temporal resolution\n");
		fprintf(stderr,"		* frequency resolution = sample_frequency / buffer_size\n");
		fprintf(stderr,"	-s: number of steps for the sliding window to span one buffer length [%d]\n",setstep);
		fprintf(stderr,"		* e.g. if -b 8 -s 4, the buffer moves by 8/4=2 samples per FFT\n");
		fprintf(stderr,"		* NOTE: must be a factor of the buffer length\n");
		fprintf(stderr,"	-f: file containing start-samples for windows\n");
		fprintf(stderr,"		* this will override the -s option\n");
		fprintf(stderr,"		* if unset, windows are automativcally defined to span the data \n");
		fprintf(stderr,"	-m: mean-correct data in each window (0=NO, 1=YES) [%d]\n",setmean);
		fprintf(stderr,"		* mean-correction will slow processing\n");
		fprintf(stderr,"		* benefit may be seen with >2 tapers, reducing low-frequency power\n");
		fprintf(stderr,"	-t: number of tapers to use, 5 is typical [%d]\n",setntapers);
		fprintf(stderr,"		* if set to 0, no tapering is applied\n");
		fprintf(stderr,"		* if set to 1, a single Hann taper is applied\n");
		fprintf(stderr,"		* otherwise, uses Sleppian (DPSS) tapers, multi-taper method applied\n");
		fprintf(stderr,"	-p: order of the tapers, typically 1 to 5, or -1=auto (ntapers+1) [%d]\n",setorder);
		fprintf(stderr,"	-g: apply Gaussian smoothing to output (avg.spectrum only) (0= none) [%d]\n",setgauss);
		fprintf(stderr,"		* note: -g must be 0 or an odd number 3 or larger)\n");
		fprintf(stderr,"	-u: set the units for spectrum output [%d]\n",setunits);
		fprintf(stderr,"		0=peak amplitude (PA) = 2 x sqrt(FFTreal^2 + FFTimag^2)/windowsize\n");
		fprintf(stderr,"		1=RMS .......= PA x (sqrt(2)/2) \n");
		fprintf(stderr,"		2=power......= RMS-squared\n");
		fprintf(stderr,"		3=dB PA......= 20*log10(PA)\n");
		fprintf(stderr,"		4=dB RMS.... = 20*log10(RMS)\n");
		fprintf(stderr,"		5=dB power...= 10*log10(power)\n");
		fprintf(stderr,"	-v: set verbocity to quiet (0) report (1) or tapers-only (-1) [%d]\n",setverb);
		fprintf(stderr,"	-o: output average spectrum (0) or matrix of values (1) [%d]\n",setmatrix);
		fprintf(stderr,"		* NOTE: if set to 1, each line is the spectrum for one buffer\n");
		fprintf(stderr,"		* this can be used to plot the spectrum over time\n");
		fprintf(stderr,"EXAMPLES: %s [input] [options] \n",thisprog);
		fprintf(stderr,"	%s data.txt -t bin -sf 1500 \n",thisprog);
		fprintf(stderr,"	cat data.bin | %s stdin -t asc \n",thisprog);
		fprintf(stderr,"OUTPUT: \n");
		fprintf(stderr,"	if -out 0:  <frequency> <power>\n");
		fprintf(stderr,"	if -out 1:  matrix of power values, row=buffer (time), column=frequency\n");
		fprintf(stderr,"	if -out 2:  <time> <frequency> <power>\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-asc")==0) { setasc=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-f")==0) 	{ settimefile=1; sprintf(timefile,"%s",argv[i+1]); i++;}
			else if(strcmp(argv[i],"-sf")==0) 	{ setsfreq=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-min")==0) 	{ setmin=1;minfreq=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-max")==0) 	{ setmax=1;maxfreq=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-b")==0) { setnbuff=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-s")==0) { setstep=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-t")==0) { setntapers=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-p")==0) { setorder=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-g")==0) { setgauss=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-u")==0) { setunits=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-m")==0) { setmean=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-o")==0) 	{ setmatrix=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-v")==0) 	{ setverb=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}
 	/* CHECK OPTIONAL ARGUMENTS */
	if(setasc<0&&setasc!=-1) { fprintf(stderr,"\n--- Error [%s]: -asc [%d] must be 0 or 1\n\n",thisprog,setasc);exit(1);}
	if(setsfreq>0.0) sample_interval=1.0F/setsfreq;
	else { fprintf(stderr,"\n--- Error [%s]: bad sample freq. (%g) must be >0\n\n",thisprog,setsfreq);exit(1);}
	if(setnbuff<0&&setnbuff!=-1) { fprintf(stderr,"\n--- Error [%s]: -b [%d] must be -1 (auto) or a positive number\n\n",thisprog,setnbuff);exit(1);}
	if(setmatrix!=0&&setmatrix!=1&&setmatrix!=2) { fprintf(stderr,"\n--- Error [%s]: bad output setting [-o %d]:  should be 0,1 or 2\n\n",thisprog,setmatrix);exit(1);}
	if(setverb<0&&setverb!=-1) { fprintf(stderr,"\n--- Error [%s]: -v [%d] must be -1 or >= 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setmean<0&&setmean!=-1) { fprintf(stderr,"\n--- Error [%s]: -m [%d] must be 0 or 1\n\n",thisprog,setmean);exit(1);}
	if(setntapers<0) { fprintf(stderr,"\n--- Error [%s]: -t [%d] must be >= 0 \n\n",thisprog,setntapers);exit(1);}
	if(setorder<0&&setorder!=-1) { fprintf(stderr,"\n--- Error [%s]: -p [%d] must be -1 or >= 0 or 1\n\n",thisprog,setorder);exit(1);}
	if((setgauss<3&&setgauss!=0)||(setgauss%2==0 && setgauss!=0)) { fprintf(stderr,"\n--- Error [%s]: -g [%d] must be 0 or an odd number 3 or larger\n\n",thisprog,setgauss);exit(1);}
	if(setunits<0||setunits>5) { fprintf(stderr,"\n--- Error [%s]: -u [%d] must be 0,1,2,3,4, or 5\n\n",thisprog,setunits);exit(1);}


	/********************************************************************************/
	/********************************************************************************/
	/* STORE DATA - ASCII OR BINX - IN BOTH CASES, CONVERT TO FLOAT  */
	/********************************************************************************/
	/********************************************************************************/
	if(setverb>0) {
		fprintf(stderr,"%s\n",thisprog);
		fprintf(stderr,"	* Reading data...\n");
	}
	if (strcmp(infile,"stdin")==0) fpin=stdin;
	else {
		if(setasc==1) fpin=fopen(infile,"r");
		else fpin=fopen(infile,"rb");
		if (fpin==NULL) { fprintf(stderr,"\n--- Error [%s]: could not open file \"%s\"\n\n",thisprog,infile);exit(1);}
	}
 	n=nbad=0;

	if(setasc==1) {
		/* read stream of ASCII numbers */
		while(fscanf(fpin,"%s",&line)==1) {
 			if(sscanf(line,"%f",&a)!=1 || !isfinite(a)) {a=NAN; nbad++;}
 			if((data=(float *)realloc(data,(n+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
 			data[n++]=a;
 	}}
	else if(setasc==0) {
		/* initialize parameters */
		for(i=0;i<6;i++) params[i]=0;
		/* read the data - treat as 1-dimensional */
		n = xf_readbinx1(fpin,&data0,params,message);
		/* check for errors */
		if(n<1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		/* allocate memory for floating-point output */
		if((data=(float *)realloc(data,n*sizeof(float)))==NULL) { fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); return(-1); }
		/* assign appropriate pointer to data0, and convert nummbers */
 		if(setverb>0) fprintf(stderr,"	* Converting to float - n=%d...\n",n);
 		if(params[2]==0) { p0 = data0; for(i=0;i<n;i++) data[i]=(float)p0[i]; if(!isfinite(data[i])) nbad++; } // uchar
 		if(params[2]==1) { p1 = data0; for(i=0;i<n;i++) data[i]=(float)p1[i]; if(!isfinite(data[i])) nbad++;} // char
 		if(params[2]==2) { p2 = data0; for(i=0;i<n;i++) data[i]=(float)p2[i]; if(!isfinite(data[i])) nbad++; } // ushort
 		if(params[2]==3) { p3 = data0; for(i=0;i<n;i++) data[i]=(float)p3[i]; if(!isfinite(data[i])) nbad++; } // short
 		if(params[2]==4) { p4 = data0; for(i=0;i<n;i++) data[i]=(float)p4[i]; if(!isfinite(data[i])) nbad++; } // uint
 		if(params[2]==5) { p5 = data0; for(i=0;i<n;i++) data[i]=(float)p5[i]; if(!isfinite(data[i])) nbad++; } // int
 		if(params[2]==6) { p6 = data0; for(i=0;i<n;i++) data[i]=(float)p6[i]; if(!isfinite(data[i])) nbad++; } // ulong
 		if(params[2]==7) { p7 = data0; for(i=0;i<n;i++) data[i]=(float)p7[i]; if(!isfinite(data[i])) nbad++; } // long
 		if(params[2]==8) { p8 = data0; for(i=0;i<n;i++) data[i]=(float)p8[i]; if(!isfinite(data[i])) nbad++; } // float
 		if(params[2]==9) { p9 = data0; for(i=0;i<n;i++) data[i]=(float)p9[i]; if(!isfinite(data[i])) nbad++; } // double
		free(data0);
	}
	/* close the input if it was a file, not stdin */
  	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	/* check the integrity of the data */
	if(n==0 || nbad==n) {fprintf(stderr,"\n--- Error[%s]: no valid data in input\n\n",thisprog);free(data);exit(1);};
	/* interpolate if some data is bad */
	if(nbad>0) xf_interp3_f(data,n);
	if(setverb>0) fprintf(stderr,"	* Complete: %d data-points read\n",n);


	/********************************************************************************
	AUTO-DEFINE MIN & MAX FREQUENCY IF NECESSARY
	********************************************************************************/
	/* determine minimum frequency if not explicitly defined */
	if(setmin<0) {
		minfreq=(2.0*setsfreq)/(double)n;
		if(minfreq<0.1) minfreq=0.1; // prevent very large FFT windows which might cause memory issues, unless requested
	}
	/* set maximum frequency to the Nyquist frequency (1/2 sampling-rate) */
	if(setmax<0) maxfreq=setsfreq/2.0;

	if(maxfreq>(0.5*setsfreq)) { fprintf(stderr,"\n--- Error [%s]: -max [%g] cannot be greater 1/2 the  sample freq. [%g]\n\n",thisprog,maxfreq,setsfreq);exit(1);}
	if(minfreq<=0)  	{ fprintf(stderr,"\n--- Error [%s]: -min [%g] must greater than zero\n\n",thisprog,minfreq);exit(1);}
	if(minfreq>maxfreq) { fprintf(stderr,"\n--- Error [%s]: -min [%g] must be less than -max [%g]\n\n",thisprog,minfreq,maxfreq);exit(1);}


	/********************************************************************************
	SET UP SETNBUFF (BUFFER LENGTH) & PARTNBUFF (PART-BUFFER LENGTH DETERMINED BY SET-STEP)
	********************************************************************************/
	/* if setstep is undefined, use non-overlapping windows */
	if(setstep<=0) setstep=1;
	/* calculate the number of samples representing the longest wavelength to be analyzed */
	a= setsfreq/minfreq;
	/* calculate optimal partnbuff : the fraction of two wavelengths spanned by each buffer-shift */
	y= 2 * (int)(0.5+(a/setstep));
	/* calculate optimal total buffer length */
	z= y*setstep;
	if(z>n) z-=2;
	/* now assign values to setnbuff and partnbuff - compare with optimal values */
	if(setnbuff<=0) { setnbuff=z, partnbuff=y; }
	else {
		if(setnbuff<z && setverb>0) { fprintf(stderr,"\t--- Warning [%s]: buffer length [-b %d] is less than the optimal length [%d] for the minimum frequency [-min %g]\n",thisprog,setnbuff,z,minfreq);}
		if(setnbuff%2 != 0) { fprintf(stderr,"\n--- Error [%s]: buffer length [-b %d] must be an even number\n\n",thisprog,setnbuff);free(data);exit(1);}
		if(setnbuff%setstep != 0) { fprintf(stderr,"\n--- Error [%s]: step size [-s %d] must be a factor of the buffer length [%d]\n\n",thisprog,setstep,setnbuff);free(data);exit(1);}
		partnbuff=setnbuff/setstep;
	}

	/* DEFINE ADDITIONAL BUFFER SIZES */
	setnbuff_f=(float)setnbuff; // to simplify floating-point calculations
	morenbuff=setnbuff+partnbuff; // size of buffer for storage during reading = buffersize + stepsize
	halfnbuff=setnbuff/2; // defines highest index in FFT result for which unique information can be obtained

	/* CALCULATE THE MIN AND MAX INDEX CORRESPONDING TO THE USER-SPECIFIED FREQUENCY RANGE */
	/* This controls which elements of the FFT results are calculated and output */
	indexa=(int)(minfreq*setnbuff*sample_interval); if(indexa<1) indexa = 1;
	indexb=(int)(maxfreq*setnbuff*sample_interval); if(indexb>=(setnbuff/2)) indexb = (setnbuff/2)-1;

	/* DEFINE SCALING FACTORS - size of data sent to FFT */
	scaling1=1.0/(float)setnbuff; /* defining this way permits multiplication instead of (slower) division */

	/* ALLOCATE EXTRA MEMORY FOR DATA IN CASE DATA LENGTH IS NOT AN EVEN NUMBER OF BUFFERS */
	j=n+setnbuff;
	if((data=(float *)realloc(data,(j*sizeoffloat)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	/* pad this extra space with zeros */
	for(i=n;i<j;i++) data[i]=0.0;
	/* make sure there's enough data for at least one buffer! */
	if(n<setnbuff) {fprintf(stderr,"\n--- Error [%s]: number of data points [%d] is less than window size [%d]\n\n",thisprog,n,setnbuff); free(data); exit(1);}


	/********************************************************************************/
	/* INITIALIZE KISS-FFT CONFIGURATION AND OTHER VARIABLES */
	/********************************************************************************/
	if(setverb>0) fprintf(stderr,"	* Initializing Kiss-FFT variables...\n");
	 /* x is used to calc. memory required for the tapers. If ntapers=0, there is actually one taper but all the values are "1" */
	if(setntapers<1) x=1 ; else x=setntapers;
	/* initialize kiss-fft variables: cfgr, ffta_a, fft_b */
	kiss_fftr_cfg cfgr = kiss_fftr_alloc( setnbuff ,0,0,0 ); /* configuration structure: memory assigned using malloc - needs to be freed at end */
	kiss_fft_cpx fft[(x * setnbuff)]; /* holds fft results: memory assigned explicitly, so does not need to be freed */

	/* allocate memory for working variables */
	if(setverb>0) fprintf(stderr,"	* Allocating memory...\n");
	if((freq=(float*)calloc(setnbuff,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // buffer which is passed to the FFT function, copied from buff1_a and float-sized to hold placeholder imaginary components
	if((buff2= (float*)calloc(setnbuff,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // buffer which is passed to the FFT function, copied from pdata
	if((amp= (double*)calloc(setnbuff,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds amplitude of the FFT results
	if((ampmean= (double*)calloc(setnbuff,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds mean FFT results (power) from multiple buff2-s
	if((ampall= (double*)calloc(x * setnbuff,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // for multitaper, holds all of the spectral amplitudes (x * setnbuff values)
	if((tapers= (double*)calloc(x * setnbuff,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds the tapers
	if((tapsum= (double *) malloc((size_t) x * sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds the sum op the tapers (diagnostic?)
	if((lambda= (double *) malloc((size_t) x * sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds the vector of eigenvalues for taper calculation and adaptive spectrum estimation
	if((degf= (double *) malloc((size_t) halfnbuff * sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};  // holds the degrees of freedom for F-statistics
	if((Fval= (float *) malloc((size_t) halfnbuff * sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};  // holds the F-statistics

	/********************************************************************************
	DEFINE FREQUENCY VALUES ASSOCIATED WITH EACH FFT OUTPUT
	********************************************************************************/
	for(i=indexa;i<=indexb;i++) {
			/* calculate frequency, round to nearest 0.001 */
			a= (float) (((double)(i)/(double)sample_interval) / (double)setnbuff_f );
			/* round to 3 digits - NOTE: this will actually add a few zeros after the 3rd digit but may not exactly round it  - sufficient for printing however */
			freq[i]= (float)xf_round1_f(a,0.001,0);
	}



	/********************************************************************************/
	/* DEFINE THE TAPER(S) */
	/********************************************************************************/
	/* if no taper is to be applied, all taper values are set to "1" */
	if(setntapers==0) {
		setntapers=1;
		for(i=0;i<setnbuff;i++) tapers[i] = 1.0;
		lambda[0]=0.0;
		tapsum[0]=0.0;
	}
	/* if only one taper, us a modified Hann taper as taper[0] */
	/* the modification is to normalize the mean value of the taper to 1, instead of the peak */
	/* this allows the taper to maintain the power in each window (normally a Hann taper cuts power by exactly 50%) */
	/* we also allow the taper to be compressed by the "setorder" variable normally used for multitaper */
	/* if you don't use the default setorder (-1, becomes 1 for 1 taper) this has the effect of padding the edges of the windows with zeros */
	else if(setntapers==1) {
		if(setorder==-1) setorder=1;
		d=twopi/(setnbuff-1.0);
		sum=0.0;
		for(i=0;i<setnbuff;i++) {
			tapers[i] = pow( 0.5*(1.-cosf(i*d)) , setorder ); /* raising to a power increases the slope of the taper */
			sum+=tapers[i];
		}
		mean=sum/(float)setnbuff;
		for(i=0;i<setnbuff;i++) tapers[i] /= mean; /* normalizing to the mean preserves amplitude while pushing down the tails of the taper */
	}
	/* otherwise, call the _mtm_slepian function */
	else if(setntapers>1){
		/* the more tapers, the higher the order should be to ensure that the "higher" tapers trend to zero at the edge of the buffer */
		if(setorder==-1) setorder=setntapers+1;
		xf_mtm_slepian1(setnbuff,setntapers,setorder,lambda,tapers,tapsum);
	}
	/* IF -v IS SET TO -1, JUST OUTPUT THE TAPERS */
	if(setverb==-1) {
		for(i=0;i<setntapers;i++) {k=i*setnbuff;for(j=0;j<setnbuff;j++) {printf("%d	%d	%g\n",i,j,tapers[k+j]);}}
		fprintf(stderr,"lambda: %f\ntapsum: %f\n",lambda[i],tapsum);
		goto END1;
	}


	/********************************************************************************/
	/* SET UP THE START-TIMES (WINDEX) FOR EACH BUFFER TO BE ANALYZED */
	/********************************************************************************/
	if(setverb>0) fprintf(stderr,"	* Defining windows...\n");
	if(settimefile==0) {
		/* define a set of equal-sized windows */
		windex= xf_window1_l(n,partnbuff,1,&nwin);
		if(nwin<0) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);stat=1;goto END1;};
	}
	/* If a time-file was specified, use the start times listed in it instead */
	else if(settimefile==1) {
		if((fpin=fopen(timefile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);stat=1;goto END1;}
		nwin=0;
		while(fscanf(fpin,"%s",&line)==1) {
			if(sscanf(line,"%ld",&i)!=1 || !isfinite(aa)) continue;
			if((windex=(long *)realloc(windex,(nwin+1)*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);stat=1;goto END1;}
			windex[nwin++]=i;
		}
		fclose(fpin);
		if(nwin<1) {fprintf(stderr,"\n--- Error[%s]: time-file \"%s\" has no valid entries\n\n",thisprog,timefile);stat=1;goto END1;}
		setstep=1;
	}

//TEST_OUTPUT_WINDOW_TIMES: fprintf(stderr,"n=%d\n",n); for(i=0;i<nwin;i++) fprintf(stderr,"--- windex[%d]=%d\n",i,windex[i]);

	/********************************************************************************
	DEFINE PARAMETERS FOR BINARY OUTPUT (USED IF SETASC==0)
	********************************************************************************/
	params[0] = 0; /* headersize - initialize to 0 for first call, to be set by the write function */
	params[1] = sizeof(double); /* datasize, bytes per datum  */
	params[2] = 9; /* datatype, 9 = double */
	ntowrite = (indexb+1) - indexa ;
	if(setmatrix==0) {
		params[3] = ntowrite; /* dim1 = total number of FFT results output */
		params[4] = 1; /* dim2 */
		params[5] = 1; /* dim3 */
	}
	if(setmatrix==1) {
		params[3] = ntowrite ; /* dim1 = total number of FFT results output */
		params[4] = (size_t)nwin; /* dim2 = blocks of output produced */
		params[5] = 1; /* dim3 */
	}

	/********************************************************************************
	OPEN OUTPUT FILE OR STREAM
	********************************************************************************/
	if(strcmp(outfile,"stdout")==0) {
		fflush(stdout);
		fpout= stdout;
	}
	else {
		if(setasc==1) fpout=fopen(outfile,"w");
		else fpout=fopen(outfile,"wb");
		if(fpout==NULL) {
			fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile);
			exit(1);
	}}

	/********************************************************************************/
	/********************************************************************************/
	/* ANALYZE THE DATA  */
	/********************************************************************************/
	/********************************************************************************/
	if(setverb>0) fprintf(stderr,"	* Analyzing the data...\n");
	setnbuffplus1=setnbuff+1.0;
	for(window=0;window<nwin;window++) {
		/* set index to data */
		pdata=data+windex[window];
		/* determine the current time for this window */
		time=windex[window]/setsfreq;
		/* determine mean-correction to buffer, if required */
		if(setmean==1) { sum=0; for(i=0;i<setnbuff;i++) sum+=pdata[i]; mean=sum*scaling1;}
		else mean=0.0;

		/* get FFT amplitude using multi-taper method */
		for(t=0;t<setntapers;t++) {
			/* define index to the start of the current taper, FFT output, and amplitude */
			k= t*setnbuff;
			/* copy real data from pdata to buff2, and apply taper */
			for(i=0;i<setnbuff;i++) buff2[i]= (pdata[i]-mean) * tapers[k+i];

			/* CALL THE FFT FUNCTION **********/
			kiss_fftr(cfgr,buff2,(fft+k));

			/* STORE THE FFTREAL, FFTIMAG & SPECTRUM:  NOTE that at present this must be done for the entire half-spectrum, not just indexa to indexb */
			if(setunits==0) { /* units= amplitude peak */
				aa=2.0 * scaling1;
				for(i=0;i<halfnbuff;i++) { ar= fft[k+i].r; ai= fft[k+i].i; ampall[k+i]= aa * sqrtf( ar*ar + ai*ai ); }
			}
			if(setunits==1) { /* units= RMS */
				aa=sqrtf(2.0) * scaling1;
				for(i=0;i<halfnbuff;i++) { ar= fft[k+i].r; ai= fft[k+i].i; ampall[k+i]= aa * sqrtf( ar*ar + ai*ai ); }
			}
			if(setunits==2) { /* units= RMS-Squared */
				aa= 2.0 * scaling1 * scaling1;
				for(i=0;i<halfnbuff;i++) { ar= fft[k+i].r; ai= fft[k+i].i; ampall[k+i]= aa * ( ar*ar + ai*ai ); }
			}

			if(setunits==3) { /* units= dB peak */
				aa=2.0 * scaling1;
				for(i=0;i<halfnbuff;i++) { ar= fft[k+i].r; ai= fft[k+i].i; ampall[k+i]= 20*log10(aa * sqrtf( ar*ar + ai*ai )); }
			}
			if(setunits==4) { /* units= db RMS */
				aa=sqrtf(2.0) * scaling1;
				for(i=0;i<halfnbuff;i++) { ar= fft[k+i].r; ai= fft[k+i].i; ampall[k+i]= 20*log10(aa * sqrtf( ar*ar + ai*ai )); }
			}
			if(setunits==5) { /* units= dB RMS-Squared */
				aa= 2.0 * scaling1 * scaling1;
				for(i=0;i<halfnbuff;i++) { ar= fft[k+i].r; ai= fft[k+i].i; ampall[k+i]= 10*log10(aa * ( ar*ar + ai*ai )); }
			}
		}
		/* average the spectra with adaptive weighting, if required - for single taper, just copy ampall[] to amp[]  */
		if(setntapers>1) {
			/* calculate the variance in the original data */
			aa=0; for(i=0;i<setnbuff;i++) aa+=(pdata[i]*pdata[i]); aa/=(setnbuff*setnbuff);
			/* get the adaptive weighted average of the spectra - stored in amp */
			z=xf_mtm_spectavg1(ampall,lambda,setntapers,halfnbuff,amp,degf,aa);
			if(z>0 && setverb>0) fprintf(stderr,"	--- Warning [%s]:%d failed iterations\n",thisprog,z);
			/* get F-values */
			xf_mtm_F(fft,halfnbuff,setntapers,Fval,tapsum);
		}
		else {	for(i=0;i<halfnbuff;i++) amp[i]=ampall[i]; degf=0; Fval=0; }

//TEST_NORMAL_AVERAGING: for(i=0;i<halfnbuff;i++) { amp[i]=0; for(t=0;t<setntapers;t++) amp[i] += ampall[t*setnbuff+i] ; amp[i]/=setntapers; }
//TEST_OUTPUT_PER-TAPER_SPECTRA: for(t=0;t<setntapers;t++) { k= t*setnbuff;for(i=indexa;i<=indexb;i++) { printf("%d	%d	%f\n",t,i,ampall[k+i]); }} goto END1;

		/* output option-1: build mean amplitude spectrum:  sqrt(FFTx2) = magnitude, divided by n = amplitude, with n reflected in scaling1 (scaling1 = 1/setnbuff)  */
		if(setmatrix==0) {
				for(i=indexa;i<=indexb;i++) ampmean[i]+= amp[i];
		}
		/* output option-2: amplitude spectrum for current buffer - convert to [units] RMS power by mulitplying amplitude by sqrt(2) */
		else if(setmatrix==1) {
			/* if input is ascii, output is ascii */
			if(setasc==1) {
				fprintf(fpout,"%g",amp[indexa]);
				for(i=(indexa+1);i<=indexb;i++) printf("\t%g",amp[i]);
				printf("\n");
			}
			/* if input is binary, output is binary  */
			else {
				z = xf_writebinx1(fpout,(amp+indexa),params,params[3],message);
				if(z<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); }
		}}
		/* option-3: (ASCII only) column output for current buffer (time, freq, power) */
		else if(setmatrix==2) {
			time= windex[window]/setsfreq;
			for(i=indexa;i<=indexb;i++) {
				printf("%f	%.3f	%g\n",time,freq[i],amp[i]);
		}}

	} /* END OF LOOP for(window=0;window<nwin;window++) */


	/********************************************************************************
	IF OUTPUT IS NOT A MATRIX, CALCULATE THE MEAN FFT RESULT ACROSS WINDOWS (BUFFERS)
	AND  OUTPUT THE RESULTS
	********************************************************************************/
	if(setmatrix==0) {

		b=(float)nwin;
		c=(float)setnbuff;
		for(i=indexa;i<=indexb;i++) ampmean[i]= ( ampmean[i]/b );

		/* smooth portion of fft output that power was actually calculated for */
		if(setgauss!=0) xf_smoothgauss1_d((ampmean+indexa),(size_t)(indexb-indexa+1),setgauss);


		if(setasc==1) {
			for(i=indexa;i<=indexb;i++) printf("%.04f\t%f\n",freq[i], ampmean[i]);
		}
		else {
			ampmean=(ampmean+indexa);
			size_t ii = xf_writebinx1(fpout,ampmean,params,ntowrite,message);
			ampmean=(ampmean-indexa);
			if(ii<1) fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message);
		}

	}

	/********************************************************************************
	CLOSE THE OUTPUT FILE IF REQUIRED
	********************************************************************************/
	if(strcmp(outfile,"stdout")!=0) fclose(fpout);


//TEST_OUTPUT_F_AND_DF: for(i=indexa;i<=indexb;i++) printf("%g	%g	%g	%g\n",((float)i/sample_interval/c), ampmean[i],degf[i],Fval[i]);
	/********************************************************************************/
	/* PRINT DIAGNOSTICS TO STDERR */
	/********************************************************************************/
	if(setverb>0) {
		fprintf(stderr,"	* minfreq= %g\n",minfreq);
		fprintf(stderr,"	* maxfreq= %g\n",maxfreq);
		fprintf(stderr,"	* setntapers= %d\n",setntapers);
		fprintf(stderr,"	* setgauss= %d\n",setgauss);
		fprintf(stderr,"	* setunits= %d\n",setunits);
		fprintf(stderr,"	* n= %d samples\n",n);
		fprintf(stderr,"	* duration= %g seconds\n",(n/setsfreq));
		fprintf(stderr,"	* indexa= %d\n",indexa);
		fprintf(stderr,"	* indexb= %d\n",indexb);
		fprintf(stderr,"	* setnbuff= %d\n",setnbuff);
		fprintf(stderr,"	* partnbuff= %d\n",partnbuff);
		fprintf(stderr,"	* setstep= %d\n",setstep);
		fprintf(stderr,"	* nwin= %d\n",nwin);
		fprintf(stderr,"	* frequency_resolution= %g\n",(setsfreq/(double)setnbuff));
		fprintf(stderr,"\n");
	}


	/********************************************************************************/
	/* FREE MEMORY AND EXIT */
	/********************************************************************************/
END1:
	if(data!=NULL) free(data);
	if(buff2!=NULL) free(buff2);
	if(amp!=NULL) free(amp);
	if(ampall!=NULL) free(ampall);
	if(ampmean!=NULL) free(ampmean);
	if(tapers!=NULL) free(tapers);
	if(lambda!=NULL) free(lambda);
	if(tapsum!=NULL) free(tapsum);
	if(degf!=NULL) free(degf);
	if(Fval!=NULL) free(Fval);
	if(windex!=NULL) free(windex);
	if(cfgr!=NULL) free(cfgr);
	if(freq!=NULL) free(freq);

	exit(stat);

}
