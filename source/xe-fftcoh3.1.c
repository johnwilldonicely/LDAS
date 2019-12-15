#define thisprog "xe-fftcoh3"
#define TITLE_STRING thisprog" v1: 27.May.2019 [JRH]"
#define MAXLINELEN 1000

#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "kiss_fftr.h"


/*
<TAGS>signal_processing spectra</TAGS>

v.1: 8.01.2019 [JRH]
	- new version of coherence based on fftcoh2
	- allows block-wise analysis using a screen-file, like xe-fftpow programs
	- also corrected a lot of print formatting warnings from previous versions

*/

/* external functions start */
float *xf_readbin2_f(char *infile, off_t *parameters, char *message);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
long xf_interp3_f(float *data, long ndata);
long *xf_window1_l(long n, long winsize, int equalsize, long *wintot);
float xf_round1_f(float input, float setbase, int setdown);
int xf_smoothgauss1_f(float *original, size_t arraysize,int smooth);
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
	long ii,jj,kk,mm,nn=0;
	float a,b,c,d,e,f,g,h;
	double aa,bb,cc,dd,ee,ff,gg,twopi=2.0*M_PI;
	FILE *fpin1,*fpin2,*fpout;
	/* program-specific variables */
	void *data0=NULL;
	char timefile[256];
	int partnwin,morenbuff,halfnbuff,finished=0,foundnan=0,indexa=-1,indexb=-1;
	long blocksize,block,blocktot=1,*blockstart=NULL,*blockstop=NULL,window,wintot=0,nbad_a,nbad_b,accsamps,*windex=NULL;
	float *data_a=NULL,*data_b=NULL,*pdata_a,*pdata_b,*tempdata_a=NULL,*tempdata_b=NULL;
	float *sumspect_aa=NULL,*sumspect_bb=NULL,*sumspect_abr=NULL,*sumspect_abi=NULL,*freq=NULL;
	float sum_a=0.0,sum_b=0.0,mean_a=0.0,mean_b=0.0,phase_a,phase_b,sample_interval=0.0,scaling1,scaling2;
	float ar,ai,br,bi,adja1,adja2,setnwin_f,*coherence=NULL;
	double time1,time2,*tapers=NULL,freqres;
	size_t aaa,accumulate=0,nout=0;
	// binary file read/write variables
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
	char *setblockfile=NULL;
	int setntapers=1,setrms=0,setnwin=0,setmin=-1,setmax=-1,setstep=1,setmatrix=0;
	int setverb=0,settimefile=0,setgauss=0,setdatatype=1,setadja=1;
	float setsfreq=1.0,minfreq=1.0,maxfreq=100.0;
	long setaccumulate=8;

	sprintf(outfile,"stdout");

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate coherence between two inputs using FFT (Kiss-FFT)\n");
		fprintf(stderr,"Assumes data consists of real values only\n");
		fprintf(stderr,"FFT is performed on overlapping windows in which data is de-meaned and tapered\n");
		fprintf(stderr,"USAGE:	%s [in1] [in2] [options] \n",thisprog);
		fprintf(stderr,"	[in1]: first input file stream \n");
		fprintf(stderr,"	[in2]: second input file, of the same length \n");
		fprintf(stderr,"VALID OPTIONS (defaults in [])\n");
		fprintf(stderr,"	-dt: data type (-1=ASCII, 0-9=BINARY) [%d]\n",setdatatype);
		fprintf(stderr,"	    0-9: uchar,char,ushort,short,uint,int,ulong,long,float,or double\n");
		fprintf(stderr,"	-sf: sampling frequency of the input, in Hz [%g]\n",setsfreq);
		fprintf(stderr,"	-max: highest frequency to output [default= sr/2]\n");
		fprintf(stderr,"	-min: lowest frequency to output [default= 800/datalength]\n");
		fprintf(stderr,"	    : note: typically produces decent-looking 400-timepoint coherograms\n");
		fprintf(stderr,"	-w: length of data windows passed to FFT function) (0=AUTO) [%d]\n",setnwin);
		fprintf(stderr,"	    : must be an even number, AUTO= 2*(sr/min)\n");
		fprintf(stderr,"	-scrf: screening file (.ssp) for defining blocks of data [unset]\n");
		fprintf(stderr,"		* NOTE: use only to define large blocks of data (minutes)\n");
		fprintf(stderr,"		* if unset, single block for entire input is assumed\n");
		fprintf(stderr,"		* -o 0: output's mean spectrum for all windows and all blocks\n");
		fprintf(stderr,"		* -o 1: each matrix will have a block-header\n");
		fprintf(stderr,"	-a: windows accumulated before calculating coherence, min=2 [%ld]\n",setaccumulate);
		fprintf(stderr,"	-adj: adjust coherence to correct for accumulation (0=NO 1=YES) [%d]\n",setadja);
		fprintf(stderr,"	-s: number of steps for the sliding window to span one buffer length [%d]\n",setstep);
		fprintf(stderr,"	    : e.g. if -b 8 -s 2, the buffer moves by 8/2=4 samples per FFT\n");
		fprintf(stderr,"	    : note: more steps = more data overlap (artificially high coherence)\n");
		fprintf(stderr,"	-t: tapering, 0=NO, 1=YES (Hann taper) [%d]\n",setntapers);
		fprintf(stderr,"	    : note: tapering inflates coherence (same taper applied to both inputs)\n");
		fprintf(stderr,"	-v: set verbocity of output to quiet (0) or report (1) [%d]\n",setverb);
		fprintf(stderr,"	-o: output style (0,1, or 2) [%d]\n",setmatrix);
		fprintf(stderr,"	    : 0=average spectrum, 1=time_v_freq matrix, 2=list of columns\n");
		fprintf(stderr,"	    : if set to 1, each line is the coherence for two buffers\n");
		fprintf(stderr,"	-g: apply Gaussian smoothing to output (avg.spectrum only) (0= none) [%d]\n",setgauss);
		fprintf(stderr,"	    : -g must be 0 or an odd number 3 or larger)\n");
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
	if(strcmp(infile1,"stdin")==0) {fprintf(stderr,"\n--- Error[%s]: this program should not be used with standard input (stdin)\n\n",thisprog); exit(1);}
	if(strcmp(infile2,"stdin")==0) {fprintf(stderr,"\n--- Error[%s]: this program should not be used with standard input (stdin)\n\n",thisprog); exit(1);}

	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-dt")==0)  setdatatype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-scrf")==0)   setblockfile=argv[++ii];
			else if(strcmp(argv[ii],"-min")==0) { setmin=1;minfreq=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-max")==0) { setmax=1;maxfreq=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-sf")==0)  setsfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-a")==0)   setaccumulate=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-adj")==0) setadja=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-w")==0)   setnwin=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-s")==0)   setstep=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-t")==0)   setntapers=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-o")==0)   setmatrix=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-g")==0)   setgauss=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-v")==0)   setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	/* CHECK OPTIONAL ARGUMENTS */
	if(setsfreq>0.0) sample_interval=1.0/setsfreq;
	else { fprintf(stderr,"\n--- Error [%s]: bad sample freq. [-sf %g] must be >0\n\n",thisprog,setsfreq);exit(1);}
	if(setnwin<0) { fprintf(stderr,"\n--- Error [%s]: -w [%d] must be 0 (auto) or a positive number\n\n",thisprog,setnwin);exit(1);}
	if(setmatrix!=0&&setmatrix!=1&&setmatrix!=2) { fprintf(stderr,"\n--- Error [%s]: bad output setting [-o %d]:  should be 0,1 or 2\n\n",thisprog,setmatrix);exit(1);}

	if(setmin>=0 && setmax>=0 && minfreq>maxfreq) { fprintf(stderr,"\n--- Error [%s]: -min [%g] must be less than -max [%g]\n\n",thisprog,minfreq,maxfreq);exit(1);}
	if(setmax<0) maxfreq=setsfreq/2.0;
	if(maxfreq>(0.5*setsfreq)) { fprintf(stderr,"\n--- Error [%s]: -max [%g] cannot be greater 1/2 the  sample freq. [%g]\n\n",thisprog,maxfreq,setsfreq);exit(1);}
	if(minfreq<=0) { fprintf(stderr,"\n--- Error [%s]: -min [%g] must greater than zero\n\n",thisprog,minfreq);exit(1);}

	if(setverb<0&&setverb!=-1) { fprintf(stderr,"\n--- Error [%s]: -v [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if((setgauss<3&&setgauss!=0)||(setgauss%2==0 && setgauss!=0)) { fprintf(stderr,"\n--- Error [%s]: -g [%d] must be 0 or an odd number 3 or larger\n\n",thisprog,setgauss);exit(1);}
	if(setaccumulate<2) { fprintf(stderr,"\n--- Error [%s]: -a [%ld] must be greater than 2\n\n",thisprog,setaccumulate);exit(1);}
	if(setadja<0||setadja>1) { fprintf(stderr,"\n--- Error [%s]: -adj [%d] must be 0 or 1\n\n",thisprog,setadja);exit(1);}
	if(setntapers!=0&&setntapers!=1) { fprintf(stderr,"\n--- Error [%s]: -t [%d] must be 0 or 1\n\n",thisprog,setntapers);exit(1);}
	if(setstep<1) { fprintf(stderr,"\n--- Error [%s]: -s [%d] must be >0\n\n",thisprog,setstep);exit(1);}

	if(setdatatype==0||setdatatype==1) datasize=(off_t)sizeof(char);
	else if(setdatatype==2||setdatatype==3) datasize=(off_t)sizeof(short);
	else if(setdatatype==4||setdatatype==5) datasize=(off_t)sizeof(int);
	else if(setdatatype==6||setdatatype==7) datasize=(off_t)sizeof(long);
	else if(setdatatype==8) datasize=(off_t)sizeof(float);
	else if(setdatatype==9) datasize=(off_t)sizeof(double);
	else if(setdatatype!=-1) {fprintf(stderr,"\n--- Error[%s]: data type (-t %d) must be -1 (ascii) or 0-9 \n\n",thisprog,setdatatype); exit(1);}

	/********************************************************************************/
	/********************************************************************************/
	/* STORE DATA - ASCII OR BINARY - IN BOTH CASES, CONVERT TO FLOAT  */
	/********************************************************************************/
	/********************************************************************************/
	if(setverb>0) {
		fprintf(stderr,"%s\n",thisprog);
		fprintf(stderr,"	* Reading data...\n");
	}

	nbad_a=0;
	nbad_b=0;

	if(setdatatype==-1) {
	 	mm=nn=0;
		fpin1=fopen(infile1,"r");
		if (fpin1==NULL) { fprintf(stderr,"\n--- Error [%s]: could not open file #1 \"%s\"\n\n",thisprog,infile1);exit(1);}
		while(fgets(line,MAXLINELEN,fpin1)!=NULL) {
 			if(sscanf(line,"%f",&a)!=1 || !isfinite(a)) {a=NAN;nbad_a++;}
 			if((data_a=(float *)realloc(data_a,(mm+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
 			data_a[mm++]=a;
		}
		fpin2=fopen(infile2,"r");
		if (fpin2==NULL) { fprintf(stderr,"\n--- Error [%s]: could not open file #2 \"%s\"\n\n",thisprog,infile2);exit(1);}
		while(fgets(line,MAXLINELEN,fpin2)!=NULL) {
 			if(sscanf(line,"%f",&b)!=1 || !isfinite(b)) {b=NAN;nbad_b++;}
 			if((data_b=(float *)realloc(data_b,(nn+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
 			data_b[nn++]=b;
		}
		fclose(fpin1);
		fclose(fpin2);
	}
	else {
		params[0]= (off_t)setdatatype;
		params[1]= (off_t)0; // bytes to skip before numbers begin
		params[2]= (off_t)0; // first number to read
		params[3]= (off_t)0; // total numbers to read (0=all in file)

		data_a= xf_readbin2_f(infile1,params,message);
		mm= (long)params[3]; // parameters[3] is reset to the number of elements (bytes/datasize) by xf_readbin2_f
		if(data_a==NULL) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}

		params[3]=(off_t)0; // total numbers to read (0=all in file)
		data_b= xf_readbin2_f(infile2,params,message);
		nn= (long)params[3]; // parameters[3] is reset to the number of elements (bytes/datasize) by xf_readbin2_f
		if(data_b==NULL) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}

		for(ii=0;ii<mm;ii++) if(!isfinite(data_a[ii])) nbad_a++;
		for(ii=0;ii<nn;ii++) if(!isfinite(data_b[ii])) nbad_b++;
	}
	//TEST: for(ii=0;ii<10;ii++) printf("%g\t%g\n",data_a[ii],data_b[ii]); exit(0);

	/* check the integrity of the data */
	if(mm==0 || nbad_a==mm) {fprintf(stderr,"\n--- Error[%s]: no valid data in input A\n\n",thisprog);exit(1);};
	if(nn==0 || nbad_b==nn) {fprintf(stderr,"\n--- Error[%s]: no valid data in input B\n\n",thisprog);exit(1);};
	if(mm!=nn) {fprintf(stderr,"\n--- Error[%s]: unequal items in inputs A (%ld) and B (%ld)\n\n",thisprog,mm,nn);exit(1);};

	/* interpolate if some data is bad */
	if(nbad_a>0) xf_interp3_f(data_a,nn);
	if(nbad_b>0) xf_interp3_f(data_b,nn);

	/********************************************************************************
	AUTO-DEFINE MIN & MAX FREQUENCY IF NECESSARY
	********************************************************************************/
	/* determine minimum frequency if not explicitly defined */
	if(setmin<0) {
		minfreq=(2.0*setsfreq)/(double)nn;
		if(minfreq<0.1) minfreq=0.1; // prevent very large FFT windows which might cause memory issues, unless requested
	}
	/* set maximum frequency to the Nyquist frequency (1/2 sampling-rate) */
	if(setmax<0) maxfreq=setsfreq/2.0;

	if(maxfreq>(0.5*setsfreq)) { fprintf(stderr,"\n--- Error [%s]: -max [%g] cannot be greater 1/2 the  sample freq. [%g]\n\n",thisprog,maxfreq,setsfreq);exit(1);}
	if(minfreq<=0)  	{ fprintf(stderr,"\n--- Error [%s]: -min [%g] must greater than zero\n\n",thisprog,minfreq);exit(1);}
	if(minfreq>maxfreq) { fprintf(stderr,"\n--- Error [%s]: -min [%g] must be less than -max [%g]\n\n",thisprog,minfreq,maxfreq);exit(1);}

	/********************************************************************************
	SET UP setnwin (BUFFER LENGTH) & partnwin (PART-BUFFER LENGTH DETERMINED BY SET-STEP)
	********************************************************************************/
	/* calculate the number of samples representing the longest wavelength to be analyzed */
	a= setsfreq/minfreq;
	/* calculate optimal partnwin : the fraction of two wavelengths spanned by each buffer-shift */
	y= 2 * (int)(0.5+(a/setstep)); if(y%2!=0) y++;
	/* calculate optimal total buffer length */
	z= y*setstep;
	if(z>nn) z-=2;
	/* now assign values to setnwin and partnwin - compare with optimal values */
	if(setnwin<1) { setnwin= z, partnwin= y; }
	else {
		if(setnwin<z && setverb>0) { fprintf(stderr,"\n--- Warning [%s]: window size [-w %d] is less than the optimal length [%d] for the minimum frequency [-min %g]\n\n",thisprog,setnwin,z,minfreq);}
		if(setnwin%2 != 0) { fprintf(stderr,"\n--- Error [%s]: window size [-w %d] must be an even number\n\n",thisprog,setnwin);exit(1);}
		if(setnwin%setstep != 0) { fprintf(stderr,"\n--- Error [%s]: step size [-s %d] must be a factor of the window size [%d]\n\n",thisprog,setstep,setnwin);exit(1);}
		partnwin=setnwin/setstep;
	}

	/* DEFINE ADDITIONAL BUFFER SIZES */
	setnwin_f= (float)setnwin;
	morenbuff= setnwin+partnwin; // size of buffer for storage during reading = buffersize + stepsize
	halfnbuff= setnwin/(int)2;

	/* ALLOCATE EXTRA MEMORY FOR DATA IN CASE DATA LENGTH IS NOT AN EVEN NUMBER OF BUFFERS */
	jj= nn+setnwin;
	if((data_a=(float *)realloc(data_a,(jj*sizeoffloat)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((data_b=(float *)realloc(data_b,(jj*sizeoffloat)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	/* pad this extra space with zeros */
	for(ii=nn;ii<jj;ii++) data_a[ii]= data_b[ii]=0.0;
	/* make sure there's enough data for at least one buffer! */
	if(nn<setnwin) {fprintf(stderr,"\n--- Error [%s]: number of data points [%ld] is less than window size [%d]\n\n",thisprog,nn,setnwin);exit(1);}


	/********************************************************************************/
	/* SET UP THE BLOCK START-TIMES (FOR ALIGNING FFT TO BLOCKS OF DATA) */
	/********************************************************************************/
	if(setverb>0) fprintf(stderr,"	* Defining blocks...\n");
	/* If no block time-file was specified, assume a single block spanning the whole record */
	if(setblockfile==NULL) {
		blocktot=1;
		if((blockstart=(long *)realloc(blockstart,1*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		if((blockstop=(long *)realloc(blockstop,1*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		blockstart[0]= 0;
		blockstop[0]= nn;
	}
	/* If a block time-file was specified, use the start times listed in it instead */
	else {

		/* read block file */
		blocktot = xf_readssp1(setblockfile,&blockstart,&blockstop,message);
		if(blocktot==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		//TEST_OUTPUT_BLOCK_TIMES: for(ii=0;ii<blocktot;ii++) { jj=blockstart[ii]; kk=blockstop[ii]; fprintf(stderr,"block %ld = [%ld]-[%ld]	dur=%ld\n",ii,jj,kk,(kk-jj)); }

		/* reject out-of-range blocks and blocks that are smaller than the FFT window size */
		mm= 0;
		for(ii=0;ii<blocktot;ii++) {
			jj= blockstart[ii];
			kk= blockstop[ii];
			if(jj>=0 && jj<=nn && kk>=0 && kk<=nn && (kk-jj)>=setnwin ) {
				blockstart[mm]= jj;
				blockstop[mm]= kk;
				mm++;
		}}
		if(mm<blocktot) {
			if(setverb>0) fprintf(stderr,"\t\t-Warning: %ld blocks dropped (out of range or smaller than FFT window)\n",(blocktot-mm));
			blocktot=mm;
		}
		if(blocktot==0) {fprintf(stderr,"\n--- Error[%s]: no valid blocks in %s\n\n",thisprog,setblockfile);exit(1);}

		/* flag if block sizes vary */
		z= 0;
		mm= blockstop[0]-blockstart[0];
		for(ii=1;ii<blocktot;ii++) {
			jj= blockstart[ii];
			kk= blockstop[ii];
			//TEST: fprintf(stderr,"%ld to %ld - mm=%ld  dur=%ld\n",blockstart[ii],blockstop[ii],mm,(kk-jj));
			if((kk-jj) != mm) z=1;
		}
		if(setverb>0 && setmatrix==1 && z!=0) {fprintf(stderr,"\t\t--- Warning[%s]: block size varies. Number of matrix rows will vary as a consequence\n",thisprog);}

	}

	/* CHECK BLOCK SIZE(S) AGAINST ACCUMULATION SETTINGS */
	accsamps= setnwin*setaccumulate; // min samples needed for accumulation
	mm= 0;
	for(block=0;block<blocktot;block++) {
		ii= blockstart[block];
		jj= blockstop[block];
		kk= jj-ii; // samples in block
		if(kk>=accsamps) {
				blockstart[mm]= ii;
				blockstop[mm]= jj;
				mm++;
	}}
	if(mm<blocktot) {
		if(setverb>0) fprintf(stderr,"\t\t-Warning: %ld blocks dropped - smaller than accumulation setting (%ld)\n",(blocktot-mm),setaccumulate);
		blocktot=mm;
	}

	/* MAKE SURE THERE IS AT LEAST 1 BLOCK REMAINING! */
	if(blocktot<1) {fprintf(stderr,"\n--- Error[%s]: no valid blocks after checking length\n\n",thisprog);exit(1);}
	//TEST_OUTPUT_BLOCK_TIMES: for(ii=0;ii<blocktot;ii++) fprintf(stderr,"block %ld = [%ld]-[%ld]\n",ii,blockstart[ii],blockstop[ii]); exit(0);


	/********************************************************************************
	CALCULATE THE ADJUSTMENT FOR COHERENCE BASED ON ACCUMULATION
	coherence = (coherence-adja1) * adja2;
	********************************************************************************/
	adja1= 1.0/setaccumulate;
	adja2= 1.0/(1.0-adja1);

	/********************************************************************************/
	/* INITIALIZE KISS-FFT CONFIGURATION AND OTHER VARIABLES */
	/********************************************************************************/
	if(setverb>0) fprintf(stderr,"	* Initializing Kiss-FFT variables...\n");
	/* initialize kiss-fft variables: cfgr, ffta_a, fft_b */
	kiss_fftr_cfg cfgr = kiss_fftr_alloc( setnwin ,0,0,0 );
	/* allocate memory for fft_a, fft_b  and work some Kiss-FFT magic */
	aaa= setnwin * setaccumulate * sizeof(kiss_fft_cpx*);
	kiss_fft_cpx* fft_a = (kiss_fft_cpx*)malloc(aaa);
	kiss_fft_cpx* fft_b = (kiss_fft_cpx*)malloc(aaa);

	/* allocate memory for working variables */
	if((freq=(float*)calloc(setnwin,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((tempdata_a=(float*)calloc(setnwin,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // copy of data, passed to the FFT function
	if((tempdata_b=(float*)calloc(setnwin,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // copy of data, passed to the FFT function
	if((sumspect_aa=(float*)calloc(setnwin,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((sumspect_bb=(float*)calloc(setnwin,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((sumspect_abr=(float*)calloc(setnwin,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((sumspect_abi=(float*)calloc(setnwin,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((tapers=(double*)calloc(setnwin,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds the tapers
	/* allocate a bit extra for coherence, as it may be used to hold the freq values as well for BINX output */
	if((coherence=(float*)calloc((setnwin+2),sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};

	/* CALCULATE THE MIN AND MAX INDEX CORRESPONDING TO THE USER-SPECIFIED FREQUENCY RANGE */
	/* This controls which elements of the FFT results are calculated and output */
	indexa=(int)(minfreq*setnwin*sample_interval); if(indexa<1) indexa=1;
	indexb=(int)(maxfreq*setnwin*sample_interval); if(indexb>=setnwin) indexb=setnwin-1;

	/* SET SCALING (1/n) and SCALING-SQUARED (1/n^2) */
	scaling1= 1.0/(float)(setnwin);
	scaling2= scaling1 * scaling1;


	/********************************************************************************
	DEFINE FREQUENCY VALUES ASSOCIATED WITH EACH FFT OUTPUT
	********************************************************************************/
	for(ii=indexa;ii<=indexb;ii++) {
			/* calculate frequency, round to nearest 0.001 */
			a= (float) (((double)(ii)/(double)sample_interval) / (double)setnwin_f );
			/* round to 3 digits - NOTE: this will actually add a few zeros after the 3rd digit but may not exactly round it  - sufficient for printing however */
			freq[ii]= (float)xf_round1_f(a,0.001,0);
	}


	/********************************************************************************/
	/* DEFINE THE HANN TAPER */
	/********************************************************************************/
	d=twopi/(setnwin-1.0);
	for(ii=0;ii<setnwin;ii++) tapers[ii] = 0.5*(1.-cosf(ii*d));
	//TEST: for(ii=0;ii<setnwin;ii++) printf("%ld\t%g\n",i,tapers[ii]);exit(0);

	/********************************************************************************
	OPEN OUTPUT FILE OR STREAM
	********************************************************************************/
	if(strcmp(outfile,"stdout")==0) {
		fflush(stdout);
		fpout=stdout;
	}
	else {
		fpout=fopen(outfile,"w");
		if(fpout==NULL) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile); exit(1);}
	}

	/********************************************************************************/
	/********************************************************************************
	ANALYZE THE DATA
	********************************************************************************/
	/********************************************************************************/
	if(setverb>0) fprintf(stderr,"	* Analyzing the data...\n");
	if(setmatrix==2) printf("start\tstop\tfreq\tcoherence\n");
	nout=0; /* total fft-outputs */

	for(block=0;block<blocktot;block++) {

		/* print block header to matrix output if required */
		if(setmatrix==1||setmatrix==2) printf("# block %ld\n",block);
		/* determine current block size */
		blocksize= blockstop[block]-blockstart[block];
		/* how many windows will fit span the current block? allow overrun but correct start-postions in next step */
		if(blocksize%partnwin==0) kk= blocksize/partnwin;
		else kk= (blocksize/partnwin)+1;
		/* reallocate memory if required */
		if(kk>wintot) {if((windex=(long *)realloc(windex,kk*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}}
		wintot= kk;
		/* set up the window start-times (windex) for each fft within a block  */
		for(ii=0;ii<wintot;ii++) {
			jj= blockstart[block]+(ii*partnwin);
			if((jj+setnwin)>blockstop[block]) jj= blockstop[block]-setnwin;
			windex[ii]= jj;
		}
		//TEST: for(window=0;window<wintot;window++) fprintf(stderr,"block:%ld size %ld\twindow:%ld\twindex:%ld\n",block,blocksize,window,windex[window]);

		accumulate=0;
		/* if outputting every coherence point in time, initialize to NAN */
		if(setmatrix==1) for(ii=0;ii<setnwin;ii++) coherence[ii]=NAN;

		for(window=0;window<wintot;window++) {
			/* set index to data for the current window */
			pdata_a= data_a+windex[window];
			pdata_b= data_b+windex[window];
			// TEST: for(ii=0;ii<setnwin;ii++) printf("%g\t%g\n",pdata_a[ii],pdata_b[ii]) ; exit(0);

			/******************************************************************************/
			/* DE-MEAN & APPLY THE TAPER TO A COPY OF THE DATA */
			a=0.0; for(ii=0;ii<setnwin;ii++) a+=pdata_a[ii]; a*=scaling1;
			b=0.0; for(ii=0;ii<setnwin;ii++) b+=pdata_b[ii]; b*=scaling1;
			if(setntapers>0) {
				for(ii=0;ii<setnwin;ii++) {
					tempdata_a[ii]= (pdata_a[ii]-a) * tapers[ii];
					tempdata_b[ii]= (pdata_b[ii]-b) * tapers[ii];
			}}
			else {
				for(ii=0;ii<setnwin;ii++) {
					tempdata_a[ii]= pdata_a[ii] - a;
					tempdata_b[ii]= pdata_b[ii] - b;
			}}
			// TEST: for(ii=0;ii<setnwin;ii++) printf("%g\t%g\n",tempdata_a[ii],tempdata_b[ii]) ; exit(0);

			/******************************************************************************/
			/* PERFORM THE FFT */
			/* set pointer to beginning of block to be written to in circular fft buffers */
			kk= accumulate * setnwin;
			/* call FFT function */
			kiss_fftr(cfgr,tempdata_a,fft_a+kk);
			kiss_fftr(cfgr,tempdata_b,fft_b+kk);
			// TEST:for(ii=0;ii<setnwin;ii++) printf("%g\t%g\t%g\t%g\n",fft_a[ii].r,fft_a[ii].i,fft_b[ii].r,fft_b[ii].i) ; exit(0);
			/* update accumulation ring-buffer position */
			accumulate++; if(accumulate>=setaccumulate) accumulate=0;

			/******************************************************************************/
			/* IF ENOUGH WINDOWS HAVE BEEN ACCUMULATED, CALCULATE COHERENCE */
			if(window>=(setaccumulate-1)) {
				nout++;
				for(ii=indexa;ii<=indexb;ii++) {
					/* reset the summed spectra */
					sumspect_aa[ii]=0.0;  sumspect_bb[ii]=0.0;
					sumspect_abr[ii]=0.0; sumspect_abi[ii]= 0.0;
					/* accumulate the spectra currently in the buffer */
					for(jj=0;jj<setaccumulate;jj++) {
						kk = jj * setnwin + ii;
						ar = fft_a[kk].r;  ai = fft_a[kk].i;
						br = fft_b[kk].r;  bi = fft_b[kk].i;
						/* accumulate the auto- and cross-spectra (for traces a & b, r=real, i=imaginary): note that the usual scaling of autospectra by 1/setnwin^2 (for amplitude calculation) is omitted, as this term cancels in the coherence calculation anyway */
						sumspect_aa[ii] += (ar*ar + ai*ai); // unscaled auto-spectrum for series a
						sumspect_bb[ii] += (br*br + bi*bi); // unscaled auto-spectrum for series b
						sumspect_abr[ii]+= (br*ar - bi*(-ai)); // unscaled cross-spectrum for a vs b, real part
						sumspect_abi[ii]+= (br*(-ai) + bi*ar); // unscaled cross-spectrum for a vs b, imaginary part
					}
					/* calculate coherence  - note that we do not need to use g*g in the second step because we are not using the sqrt of the cross-spectrum */
					f = sumspect_aa[ii] * sumspect_bb[ii] ;
					g = sumspect_abr[ii]*sumspect_abr[ii] + sumspect_abi[ii]*sumspect_abi[ii] ;
					if(f!=0) h= g/f; else h= 0.0;
					/*
					correct coherence for accumulation level (if accumulation=2, baseline=0.5, etc.)
					adja1= 1.0/setaccumulate  adja2= 1.0/(1.0-adja1)
					*/
					if(setadja==1) h= (h-adja1) * adja2;
					if(setmatrix==0) coherence[ii]+=  h; else coherence[ii]= h;
				}
				// TEST BUFFERED FFT: for(ii=0;ii<setnwin;ii++) { printf("\n"); for(j=0;j<setaccumulate;j++){k = j * setnwin + i; printf("%g\t%g\t%g\t%g\n",fft_a[kk].r,fft_a[kk].i,fft_b[kk].r,fft_b[kk].i);}} exit(0);
				// TEST SUMMEDSPECTRA: for(ii=0;ii<setnwin;ii++) printf("%g\t%g\t%g\t%g\n",sumspect_aa[ii],sumspect_bb[ii],sumspect_abr[ii],sumspect_abi[ii]); exit(0);
				// TEST exit(0);
			}

			/* OUTPUT COHERENCE FOR MATRIX OR TIME*FREQ TABLE */
			if(setmatrix==1) {
				fprintf(fpout,"%f",coherence[indexa]);
				for(ii=(indexa+1);ii<=indexb;ii++) fprintf(fpout,"\t%f",coherence[ii]);
				fprintf(fpout,"\n");
			}
			if(setmatrix==2) {
				kk=(window-(setaccumulate-1));
				if(kk<0)kk=0;
				time1=windex[kk]/setsfreq;
				time2=(windex[(window)]+setnwin)/setsfreq;
				/* print to screen */
				for(ii=indexa;ii<=indexb;ii++) {
					fprintf(fpout,"%f	%f	%.3f	%g\n",time1,time2,freq[ii],coherence[ii]);
				}
			}
		} /* END OF for(window=0 to wintot)  LOOP */
	} // END OF for(block=0;block<blocktot;block++)


	if(setmatrix==0) {
		/* normalize by the number of coherence values calculated */
		bb=(double)nout; for(ii=indexa;ii<=indexb;ii++) coherence[ii]/=bb;
		/* smooth portion of data coherence was actually calculated for */
		if(setgauss!=0) xf_smoothgauss1_f((coherence+indexa),(size_t)(indexb-indexa+1),setgauss);
		/* output */
		for(ii=indexa;ii<=indexb;ii++) fprintf(fpout,"%.3f\t%f\n",freq[ii],coherence[ii]);
	}

	/********************************************************************************
	CLOSE THE OUTPUT FILE IF REQUIRED
	********************************************************************************/
	if(strcmp(outfile,"stdout")!=0) fclose(fpout);

	// FREE MEMORY
	if(tempdata_a!=NULL) free(tempdata_a);
	if(tempdata_b!=NULL) free(tempdata_b);
	if(sumspect_aa!=NULL) free(sumspect_aa);
	if(sumspect_bb!=NULL) free(sumspect_bb);
	if(sumspect_abr!=NULL) free(sumspect_abr);
	if(sumspect_abi!=NULL) free(sumspect_abi);
	if(coherence!=NULL) free(coherence);
	if(tapers!=NULL) free(tapers);
	if(cfgr!=NULL) free(cfgr);
	if(fft_a!=NULL) free(fft_a);
	if(fft_b!=NULL) free(fft_b);
	if(freq!=NULL) free(freq);
	if(data_a!=NULL) free(data_a);
	if(data_b!=NULL) free(data_b);
	if(blockstart!=NULL) free(blockstart);
	if(blockstop!=NULL) free(blockstop);

	/* set the frequency resolution of the output */
	freqres= setsfreq/(double)setnwin;

	/* PRINT DIAGNOSTICS TO STDERR */
	if(setverb==1) {
		fprintf(stderr,"\n");
		fprintf(stderr,"	samplefreq= %g Hz\n",((float)setsfreq));
		fprintf(stderr,"	nn= %ld samples\n",nn);
		fprintf(stderr,"	duration= %g seconds\n",((float)nn/(float)setsfreq));
		fprintf(stderr,"	minfreq=%g\n",minfreq);
		fprintf(stderr,"	maxfreq=%g\n",maxfreq);
		fprintf(stderr,"	setnwin(windowsize)=%d\n",setnwin);
		fprintf(stderr,"	setntapers=%d\n",setntapers);
		fprintf(stderr,"	indexa=%d\n",indexa);
		fprintf(stderr,"	indexb=%d\n",indexb);
		fprintf(stderr,"	setstep=%d\n",setstep);
		fprintf(stderr,"	setaccumulate=%ld\n",setaccumulate);
		// calculate window overlap
		a=100*(1.0-1.0/setstep);
		fprintf(stderr,"	fft_window= %d samples\n",setnwin);
		fprintf(stderr,"	fft_overlap= %g %%\n",a);
		fprintf(stderr,"	fft_step= %d \n",setstep);
		fprintf(stderr,"	blocktot= %ld\n",blocktot);
		fprintf(stderr,"	wintot=%ld\n",wintot);
		fprintf(stderr,"	nout=%ld\n",nout);
		fprintf(stderr,"	frequency_resolution= %g Hz\n",freqres);
		fprintf(stderr,"	temporal_resolution= %g seconds\n",(partnwin/setsfreq));
		fprintf(stderr,"\n");
	}

	exit(0);
}
