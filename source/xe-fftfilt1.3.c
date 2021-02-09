#define thisprog "xe-fftfilt1"
#define TITLE_STRING thisprog" v 3: 15.March2014 [JRH]"
#define MAXLINELEN 1000

#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "kiss_fftr.h"


/*
<TAGS>signal_processing filter</TAGS>
NOTES/TO DO:

	*** still discontinuities in output!
	*** remove options for step other than 2 - assumed half-overlap in most of program at present
	*** improve efficency of the bopy & add process
		- set up pointers to buff2a (previous window) and buff2b (current window)
		- avoid copying to buff3 and simply update pointers
				- allow setnbuff+halfnbuff memory for buff2 (three half-buffers basically)

				- pbuff2b+=halfnbuff
				- if >setnbuff pbuff2a=0

				-  inversefft into (buff2+pbuff2b)
				- [add and output]

				- pbuff2a = pbuff2b


v 3: 15.March2014 [JRH]
	- add set-to-zero for second half of fft results as well (negative spectrum)
		- previously program was not treating left and right sides of spectrum properly


v 2: 27.September.2013 [JRH]
	- all the windowing put back in
	- proper ? implementation of the overlap-and-add method
	- however, there are still "jumps in the output at the half-buffer edges - why???


v 1: 27.September.2013 [JRH]
	- derived from xe-fftpow1.23.c

*/




/* external functions start */
long *xf_window1_l(long n, long winsize, int equalsize, long *nwin);
long xf_interp3_f(float *data, long ndata);
float *xf_matrixavg1_f(float *multimatrix, size_t n_matrices, size_t bintot, char message[]);
size_t xf_readbinx1(FILE *fpin, void **data, size_t *params, char *message);
size_t xf_writebinx1(FILE *fpout, void *data, size_t *params, size_t ntowrite, char *message);
int xf_detrend1_f(float *y, size_t nn, double *result_d);
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
	double aa,bb,cc,dd,ee,ff,gg,twopi=2.0*M_PI,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	size_t halfnbuff,partnbuff,morenbuff,nbad=0;
	size_t window,nwin=0,*windex=NULL;
	float *buff1=NULL,*buff2=NULL,*buff3=NULL,*data2=NULL;
	float *data=NULL,*pdata,*Fval=NULL,sum=0.0,mean,sample_interval=0.0F,scaling1,scaling2;
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
	int indexa=-1,indexb=-1,setnbuff=-1,setnbuffplus1,setmin=-1,setmax=-1,setstep=2,setmatrix=0,setverb=0,setorder=-1;
	int setntapers=1,setmean=1,settimefile=0,setasc=1;
	float setsfreq=1.0,minfreq=1.0,maxfreq=100.0,setnbuff_f,freq;
	double time;

	sprintf(outfile,"stdout");


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"Use KISS-FFT functions to filter data using overlap-and-add method\n");
		fprintf(stderr,"Requires input from a file or standard input, one column only\n");
		fprintf(stderr,"FFT is performed on overlapping chunks of buffered data\n");
		fprintf(stderr,"Produces one column of filtered output\n");
		fprintf(stderr,"USAGE:	%s [input] [options] \n",thisprog);
		fprintf(stderr,"	[input]: provide a filename or \"stdin\" to receive piped data\n");
		fprintf(stderr,"VALID OPTIONS (defaults in [])\n");
		fprintf(stderr,"	-asc: is input/output ASCII? (1=YES, 0=NO, BINX binary input is assumed) [%g]\n",setasc);
		fprintf(stderr,"	-sf: sampling frequency (Hz) of the input [%g]\n",setsfreq);
		fprintf(stderr,"	-min: lowest frequency to output [default= 2/nsamples]\n");
		fprintf(stderr,"	-max: highest frequency to output [default= sr/2]\n");
		fprintf(stderr,"	-b: length of buffers (windows) of data passed to FFT function) [-1 = auto]\n");
		fprintf(stderr,"		* must be an even number, not necessarily a power of two\n");
		fprintf(stderr,"		* by default, auto = 2*(sr/min)\n");
		fprintf(stderr,"		* longer window = more detailed output but lower temporal resolution\n");
		fprintf(stderr,"		* frequency resolution = sample_frequency / buffer_size\n");
		fprintf(stderr,"	-s: number of steps for the sliding window to span one buffer length [%d]\n",setstep);
		fprintf(stderr,"		* NOTE: CURRENTLY MUST BE TWO!\n");
		fprintf(stderr,"	-v: set verbocity to quiet (0) report (1) or tapers-only (-1) [%d]\n",setverb);
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
			else if(strcmp(argv[i],"-v")==0) 	{ setverb=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}
 	/* CHECK OPTIONAL ARGUMENTS */
	if(setasc<0&&setasc!=-1) { fprintf(stderr,"\n--- Error [%s]: -asc [%d] must be 0 or 1\n\n",thisprog,setasc);exit(1);}
	if(setsfreq>0.0) sample_interval=1.0F/setsfreq;
	else { fprintf(stderr,"\n--- Error [%s]: bad sample freq. (%g) must be >0\n\n",thisprog,setsfreq);exit(1);}
	if(setnbuff<0&&setnbuff!=-1) { fprintf(stderr,"\n--- Error [%s]: -b [%d] must be -1 (auto) or a positive number\n\n",thisprog,setnbuff);exit(1);}
	if(setmax<0) maxfreq=setsfreq/2.0;
	if(maxfreq>(0.5*setsfreq)) { fprintf(stderr,"\n--- Error [%s]: -max [%g] cannot be greater 1/2 the  sample freq. [%g]\n\n",thisprog,maxfreq,setsfreq);exit(1);}
	if(minfreq<=0)  	{ fprintf(stderr,"\n--- Error [%s]: -min [%g] must greater than zero\n\n",thisprog,minfreq);exit(1);}
	if(minfreq>maxfreq) { fprintf(stderr,"\n--- Error [%s]: -min [%g] must be less than -max [%g]\n\n",thisprog,minfreq,maxfreq);exit(1);}
	if(setverb<0&&setverb!=-1) { fprintf(stderr,"\n--- Error [%s]: -v [%d] must be -1 or >= 0 or 1\n\n",thisprog,setverb);exit(1);}


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
		if(n<1) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
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
	SET UP SETNBUFF (BUFFER LENGTH) & PARTNBUFF (PART-BUFFER LENGTH DETERMINED BY SET-STEP)
	********************************************************************************/
	/* determine minimum frequency if not explicitly defined */
	if(setmin<0) minfreq=(2.0*setsfreq)/(double)n;
	/* if setstep is undefined, use 50% overlapping windows (2 steps span the buffer) */
	if(setstep<=0) setstep=1;
	/* calculate the number of samples representing the longest wavelength to be analyzed */
	a= setsfreq/minfreq;
	/* calculate optimal partnbuff : the fraction of two wavelengths spanned by each buffer-shift */
	y= 2 * (int)(0.5+(a/setstep));
	/* calculate optimal total buffer length */
	z= y*setstep;
	/* now assign values to setnbuff and partnbuff - compare with optimal values */
	if(setnbuff<=0) { setnbuff=z, partnbuff=y; }
	else {
		if(setnbuff<z) { fprintf(stderr,"\t--- Warning [%s]: buffer length [-b %d] is less than the optimal length [%d] for the minimum frequency [-min %g]\n",thisprog,setnbuff,z,minfreq);}
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
	scaling2=2.0/(float)setnbuff; /* this allows remove of the 2x term in the calculation of amplitude  */

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
	kiss_fftr_cfg cfgri = kiss_fftr_alloc( setnbuff ,1,0,0 ); /* configuration structure (inverse FFT) : memory assigned using malloc - needs to be freed at end */
	/* allocate memory for working variables */
	if(setverb>0) fprintf(stderr,"	* Allocating memory...\n");
	if((buff1= (float*)calloc(setnbuff,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // buffer which is passed to the FFT function, copied from pdata
	if((buff2= (float*)calloc(setnbuff,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // buffer which is passed to the FFT function, copied from pdata
	if((buff3= (float*)calloc(setnbuff,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // buffer which is passed to the FFT function, copied from pdata


	/********************************************************************************/
	/* SET UP THE START-TIMES (WINDEX) FOR EACH BUFFER TO BE ANALYZED */
	/********************************************************************************/
	if(setverb>0) fprintf(stderr,"	* Defining windows...\n");
	if(settimefile==0) {
		/* define a set of equal-sized windows */
		windex= xf_window1_l(n,partnbuff,1,&nwin);
		if(nwin<0) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);stat=1;goto END1;};
	}


//TEST_OUTPUT_WINDOW_TIMES: fprintf(stderr,"n=%d\n",n); for(i=0;i<nwin;i++) fprintf(stderr,"--- windex[%d]=%d\n",i,windex[i]);


	/********************************************************************************/
	/********************************************************************************/
	/* ANALYZE THE DATA  */
	/********************************************************************************/
	/********************************************************************************/
	if(setverb>0) fprintf(stderr,"	* Analyzing the data...\n");
	setnbuffplus1=setnbuff+1.0;


	/* pre-fill second half of buffer to be passed to FFT with zeros */
	for(i=halfnbuff;i<setnbuff;i++) buff1[i]=0.0;

	for(window=0;window<nwin;window++) {

		/* set index to data */
		pdata=data+windex[window];

		/* fill the first half of buff1 with data */
		for(i=0;i<halfnbuff;i++) buff1[i]=pdata[i];

		/* pass buff1 to the FFT function */
		kiss_fftr(cfgr,buff1,fft);

		/* remove unwanted signals (essentially a rectangular filter kernal) */
		j=0;k=indexa;
		for(i=j;i<k;i++) fft[i].r =  fft[i].i = 0.0;
		j=indexb+1; k=halfnbuff;
		for(i=j;i<k;i++) fft[i].r =  fft[i].i = 0.0;

		j=halfnbuff;k=indexa+halfnbuff;
		for(i=j;i<k;i++) fft[i].r =  fft[i].i = 0.0;
		j=indexb+1+halfnbuff; k=setnbuff;
		for(i=j;i<k;i++) fft[i].r =  fft[i].i = 0.0;

		/* perform inverse FFT - output to buff2 */
		kiss_fftri(cfgri,fft,buff2);

		/* if this is the first window, output first half of buff2 and put the second half into buff3 */
		if(window==0) {
			for(i=0;i<halfnbuff;i++) printf("%f\n",buff2[i]*scaling1);
			for(i=halfnbuff;i<setnbuff;i++) buff3[i] = buff2[i];
		}
		/* otherwise, output the sum of the first half of current buff2 and second half of previous buff2 */
		else {
			/* copy the first half of buff2 to buff3 - second half of buff3 already contains second half of buff2 from previous window */
			for(i=0;i<halfnbuff;i++) buff3[i] = buff2[i];
			/* calculate and output the sum of the first and second halves of buff3  */
			for(i=0;i<halfnbuff;i++) { a = scaling1 * (buff3[i]+buff3[i+halfnbuff]) ; printf("%f\n",a); }
			/* copy the second half of buff2 to buff3  */
			for(i=halfnbuff;i<setnbuff;i++) buff3[i] = buff2[i];
		}


	} /* END OF LOOP for(window=0;window<nwin;window++) */



//TEST_OUTPUT_F_AND_DF: for(i=indexa;i<=indexb;i++) printf("%g	%g	%g	%g\n",((float)i/sample_interval/c), ampmean[i],degf[i],Fval[i]);
	/********************************************************************************/
	/* PRINT DIAGNOSTICS TO STDERR */
	/********************************************************************************/
	if(setverb>0) {
		fprintf(stderr,"	* n= %d samples\n",n);
		fprintf(stderr,"	* duration= %g seconds\n",(n/setsfreq));
		fprintf(stderr,"	* indexa= %d\n",indexa);
		fprintf(stderr,"	* indexb= %d\n",indexb);
		fprintf(stderr,"	* minfreq= %g\n",minfreq);
		fprintf(stderr,"	* maxfreq= %g\n",maxfreq);
		fprintf(stderr,"	* frequency_resolution= %g\n",(setsfreq/(double)setnbuff));
		fprintf(stderr,"	* setnbuff= %d\n",setnbuff);
		fprintf(stderr,"	* partnbuff= %d\n",partnbuff);
		fprintf(stderr,"	* setstep= %d\n",setstep);
		fprintf(stderr,"	* nwin= %d\n",nwin);
		fprintf(stderr,"\n");
	}


	/********************************************************************************/
	/* FREE MEMORY AND EXIT */
	/********************************************************************************/
END1:
	if(data!=NULL) free(data);
	if(buff1!=NULL) free(buff1);
	if(buff2!=NULL) free(buff2);
	if(buff3!=NULL) free(buff3);
	if(windex!=NULL) free(windex);
	if(cfgr!=NULL) free(cfgr);
	if(cfgri!=NULL) free(cfgri);

	exit(stat);

}
