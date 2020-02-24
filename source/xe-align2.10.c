#define thisprog "xe-align2"
#define TITLE_STRING thisprog" v 11: 22.February.2020 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* <TAGS> dt_block signal_processing transform filter stats </TAGS> */

/*
v 11: 22.February.2020 [JRH]
	- add pre-interpolation of data, plus warning if interpolation is >10%
	- add QC check for first/last arguments
	- change all size_t arguments to long to allow checks for sign
	- pre-screen blocks for first/last criteria and for pre/post falling within data range
	- add option to exclude blocks based o nnoise criterion (rectified-mean-difference from median curve)

v 10: 6.September.2019 [JRH]
	- add filter option to input

v 10: 6.April.2015 [JRH]
	- use new version of binning function xf_bin1a_f
		- better allocation of data - no undersampled bins
		- guaranteed "zero" bin in which first sample is the first sample after zero
	- correction to output times

v 9: 16.February.2015 [JRH]
	- added ability to normalize each aligned block by the overall mean in the block (-norm 4)
	- added ability to normalize each aligned block by the overall linear trend in the block (-norm 5)
	- simplified indexing and improved efficiency in loops creating dat2 from dat1 (pdat1 pointer introduced)
	- improved instructions to reflect how samples are included or excluded

v 8: 1.September.2014 [JRH]
	- add ability to EXCLUDE some data just before zero fromo calculation of mean & std.dev
	- this allows user to avoid normalization to include data influenced by events after zero
	- this could happen if overlapping windows or smoothing were used in generation of the input

v 7: 24.February.2014 [JRH]
	- bugfix: sample-interval for output was incorrectly calculated if -bin was set to zero

v 6: 19.February.2014 [JRH]
	- elimintate -verb 2 - no need

v 5: 17.February.2014 [JRH]
	- allows ASCII input
	- allows -post to be zero (so if -pre is also zero, program only outputs datum at start-times)
	- allows mean aligned output as an alternative to each block of aligned data
		- this will save a lot of disk space for large files if we only need the mean
		- may also eliminate the need for calculating the mean with another program
	- remove all references to start/stop pairs - this program only considers start-times
	- remove option controlling shifting of zero-designation to beginning of presample - never used in practice
	- error if -norm 1 (normalize to start) is combined with a presample normalization period >0

v 4: 26.January.2014 [JRH]
	- use new xf_bin1_f function for downsampling curve if required

*/

/* external functions start */
long xf_interp3_f(float *data, long ndata);

long xf_outlier2_f(float *dat1, long ndat1, long *start, long nblocks, long blocklen, long zero, char *ref, char *score, float thresh, double *result, char *message);
int xf_compare1_d(const void *a, const void *b);
int xf_percentile1_f(float *data, long nn, double *result);
float xf_stats1_f(double *dat1, long nn, int digits);

float *xf_readbin2_f(char *infile, off_t *parameters, char *message);
int xf_filter_bworth1_f(float *X, off_t nn, float sample_freq, float setlow, float sethigh, float res, char *message);
double xf_bin1a_f(float *data, size_t *setn, size_t *setz, size_t setnbins, char *message);
int xf_detrend1_f(float *y, size_t nn, double *result_d);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],message[MAXLINELEN],*pcol;
	long ii,jj,kk,nn;
	int z,col,colmatch,setcolmatch;
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[32];
	FILE *fpin,*fpout;
	/* program-specific variables */
	float *dat1=NULL,*dat2=NULL,*dat3=NULL,*sumdat2=NULL,*pdat1=NULL;
	long *start=NULL,ndat1,ndat2=0,n3=0,block,nblocks=0,nblocks3=0,usedblocks=0;
	long index1,index1b,index1c,index2,index3,zero,bmin,bmax;
	off_t parameters[5];
	double sum,mean,sumofsq,stddev,sampint,binsize,binint=0;
	/* arguments */
	char cmtfile[256];
	int setnorm=0,setflip=0,setverb=0,setdatatype=0,setout=0;
	long setfirst=0,setlast=-1,setpre=0,setpost=10,setpn=0,setpnx=0,setnbins=0;
	float setlow=0.0,sethigh=0.0;  // filter cutoffs
	float setresonance=1.4142; // filter resonance setting ()
	float setnoise=3.0; // standard-deviations cutoff for defining an outlier-block to be excluded (0=skip)
	double setsampfreq=1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Align data to start signals in a separate file\n");
		fprintf(stderr,"	- assumes input has constant sampling rate\n");
		fprintf(stderr,"	- all \"times\" are expressed as samples\n");
		fprintf(stderr,"MEMORY_REQUIREMENTS: 4*double (32 bytes) for every <time><data> pair\n");
		fprintf(stderr,"USAGE: %s [input] [start] [options]\n",thisprog);
		fprintf(stderr,"	[input]: data file\n");
		fprintf(stderr,"	[start]: file containing block start-samples (zero-offset)\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-dt: type of data [%d]\n",setdatatype);
		fprintf(stderr,"		-1  = ASCII\n");
		fprintf(stderr,"		0-9 = uchar,char,ushort,short,uint,int,ulong,long,float,double\n");
		fprintf(stderr,"	-sf: sample frequency (Hz) - used to adjust output-samples to times [%g]\n",setsampfreq);
		fprintf(stderr,"	-low: low frequency limit, 0=NONE [%g]\n",setlow);
		fprintf(stderr,"	-high: high frequency limit, 0=NONE [%g]\n",sethigh);
		fprintf(stderr,"	-pre: samples preceding block-starts to include [%ld]\n",setpre);
		fprintf(stderr,"	-post: samples after block-starts to include [%ld]\n",setpost);
		fprintf(stderr,"		NOTE: total block size = pre+post+1, as the new \"0\" is included\n");
		fprintf(stderr,"	-pn: pre-start samples to include for normalization [default:same as -pre]\n");
		fprintf(stderr,"		NOTE: includes the new \"0\", so #samples actually = pn+1\n");
		fprintf(stderr,"	-pnx: samples (including start) to exclude from normalization [%ld]\n",setpnx);
		fprintf(stderr,"		- use if input was smoothed, to exclude predictive samples\n");
		fprintf(stderr,"		- e.g. if -pnx = 1, only new sample \"0\" will be excluded\n");
		fprintf(stderr,"		- e.g. if -pnx = -pn, only 1 sample will be used!\n");
		fprintf(stderr,"	-norm: normalization method [%d]\n",setnorm);
		fprintf(stderr,"		0:none, 1: sample zero (start), 2: -pn average, 3:z-score\n");
		fprintf(stderr,"		4:de-mean 5: de-trend (4 & 5 apply to entire block\n");
		fprintf(stderr,"	-first: select first block to use (numbered from zero) [%ld]\n",setfirst);
		fprintf(stderr,"	-last: select last block to use (-1=to the last one) [%ld]\n",setlast);
		fprintf(stderr,"	-nbins: number of non-overlapping bins to average output (0=none) [%ld]\n",setnbins);
		fprintf(stderr,"	-flip: flip data (multiply by -1) (0=NO, 1=YES) [%d]\n",setflip);
		fprintf(stderr,"	-noise: exclude noisy blocks (S.DEV from median trace, -1=SKIP) [%g]\n",setnoise);
		fprintf(stderr,"	-out: output type (0=average, 1=all aligned blocks) [%d]\n",setout);
		fprintf(stderr,"	-verb: verbosity of reporting (0=none,1=report to stderr) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s sub001.dat sub001.cmt -start \"pump on\"\n",thisprog);
		fprintf(stderr,"	cat temp.dat | %s stdin temp.cmt -start \"ON\" -dur 60\n",thisprog);
		fprintf(stderr,"OUTPUT - ASCII version of data as follows:\n");
		fprintf(stderr,"	if -out 0: [time-in-block] [average]\n");
		fprintf(stderr,"	if -out 1: [block] [time-in-block] [original-value]\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE INPUT FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	sprintf(cmtfile,"%s",argv[2]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-dt")==0) setdatatype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-sf")==0) setsampfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-pre")==0) setpre=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-post")==0) setpost=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-pn")==0) setpn=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-pnx")==0) setpnx=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-norm")==0) setnorm=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-flip")==0) setflip=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-low")==0) setlow=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-high")==0) sethigh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-first")==0)setfirst=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-last")==0) setlast=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-noise")==0) setnoise=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-nbins")==0) setnbins=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0) setout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setverb!=0&&setverb!=1) {fprintf(stderr,"\n--- Error[%s]: -verb (%d) must be 0 or 1\n\n",thisprog,setverb);exit(1);};
	if(setpost<0) {fprintf(stderr,"\n--- Error[%s]: -post (%ld) must be >=0\n\n",thisprog,setpost);exit(1);};
	if(setsampfreq<=0) {fprintf(stderr,"\n--- Error[%s]: -sf (%g) must be >0\n\n",thisprog,setsampfreq);exit(1);};
	if(setlow<0) {fprintf(stderr,"\n--- Error[%s]: low frequency cutoff (-low %g) must be >= 0 \n\n",thisprog,setlow);exit(1);};
	if(sethigh<0) {fprintf(stderr,"\n--- Error[%s]: high frequency cutoff (-high %g) must be >= 0 \n\n",thisprog,sethigh);exit(1);};
	if(setpn>setpre) {fprintf(stderr,"\n--- Error[%s]: -pn (%ld) must not exceed -pre (%ld)\n\n",thisprog,setpn,setpre);exit(1);};
	if(setpnx>setpn) {fprintf(stderr,"\n--- Error[%s]: -pnx (%ld) must not exceed -pn (%ld)\n\n",thisprog,setpnx,setpn);exit(1);};
	if(setnorm<0||setnorm>5) {fprintf(stderr,"\n--- Error[%s]: -norm (%d) must be 0,1,2,3,4 or 5\n\n",thisprog,setnorm);exit(1);};
	if(setflip!=0&&setflip!=1) {fprintf(stderr,"\n--- Error[%s]: -flip (%d) must be 0 or 1\n\n",thisprog,setflip);exit(1);};
	if(setout!=0&&setout!=1) {fprintf(stderr,"\n--- Error[%s]: -out (%d) must be 0 or 1\n\n",thisprog,setout);exit(1);};
	if(setlast!=-1 && setfirst>setlast) {fprintf(stderr,"\n--- Error[%s]: -first (%ld) is greater than -last (%ld)\n\n",thisprog,setfirst,setlast);exit(1);};
	if(setpn==0) setpn=setpre;
	else if(setnorm==1) {fprintf(stderr,"\n--- Error[%s]: cannot set -pn (%ld) with single-sample normalization (-norm %d)\n\n",thisprog,setpn,setnorm);exit(1);};

	/* calculate block size (ndat2) */
	ndat2=setpre+setpost+1;
	/* determine the sample-interval */
	sampint= 1.0 / setsampfreq;

	/********************************************************************************
	READ THE DATA
	********************************************************************************/
	ndat1=0;
	/* if data is ASCII */
	if(setdatatype==-1) {
		kk= sizeof(*dat1);
		if(strcmp(infile,"stdin")==0) fpin=stdin;
		else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%f",&a)!=1) continue;
			dat1= realloc(dat1,(ndat1+1)*kk);
			if(dat1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			dat1[ndat1++]= a;
		}
		if(strcmp(infile,"stdin")!=0) fclose(fpin);
	}
	/* if data is binary */
	else if(setdatatype>=0) {
		if(strcmp(infile,"stdin")==0) {fprintf(stderr,"\n--- Error[%s]: piped input from \"stdin\" input not enabled for binary data (-dt %d). Please select file name for input\n\n",thisprog,setdatatype);exit(1);}
		if(setverb>0) fprintf(stderr,"Reading data file %s\n",infile);
		parameters[0]=setdatatype;
		parameters[1]=0; // header-bytes
		parameters[2]=0; // numbers to skip
		parameters[3]=0; // bytes to read if set to zero, will read all available bytes after the header+(start*datasize)
		dat1 = xf_readbin2_f(infile,parameters,message);
		ndat1=parameters[3]; // parameters[3] is reset to the number of elements (bytes/datasize) by xf_readbin2_f
		if(dat1==NULL) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
	}
	//TEST	for(ii=0;ii<10;ii++) printf("%g\n",dat1[ii]);exit(1);
	if(setverb>0) {
		for(ii=0;ii<argc;ii++) fprintf(stderr,"%s ",argv[ii]);
		fprintf(stderr,"\n");
		fprintf(stderr,"\tpre-start samples: %ld\n",setpre);
		fprintf(stderr,"\tpost-start samples: %ld\n",setpost);
		fprintf(stderr,"\tnormalization_presample: %ld\n",setpn);
		if(setnorm==0) fprintf(stderr,"\tnormalized: no\n");
		if(setnorm==1) fprintf(stderr,"\tnormalized: sample-zero\n");
		if(setnorm==2) fprintf(stderr,"\tnormalized: presample average\n");
		if(setnorm==3) fprintf(stderr,"\tnormalized: presample z-score\n");
		fprintf(stderr,"\tbinning: %ld\n",setnbins);
		fprintf(stderr,"\tflip data: "); if(setflip==0) fprintf(stderr,"no\n"); else fprintf(stderr,"yes\n");
		fprintf(stderr,"\ttotal_samples: %ld\n",ndat1);
	}

	/* INTERPOLATE THE DATA*/
	kk= xf_interp3_f(dat1,ndat1);
	if(kk<0) {fprintf(stderr,"\n--- Error[%s]: no valid numbers in input \"%s\" not found\n\n",thisprog,infile);exit(1);}
	if((double)kk/(double)ndat1>0.10) {fprintf(stderr,"--- Warning[%s]: >10%% of data points in %s required interpolation\n",thisprog,infile);}

	/* FLIP THE DATA IF REQUIRED */
	if (setflip==1) { for(ii=0;ii<ndat1;ii++) dat1[ii]= 0.0-dat1[ii]; }

	/* APPLY FILTERING */
	if(setverb>0) fprintf(stderr,"\tfiltering...\n");
	ii= xf_filter_bworth1_f(dat1,ndat1,setsampfreq,setlow,sethigh,setresonance,message);
	if(ii<0) { fprintf(stderr,"\n\t --- Error [%s]: %s\n\n\n",thisprog,message); goto END1; }


	/******************************************************************************
	READ THE START SIGNALS DEFINING BLOCKS
	- eliminate blocks falling outside the range
	- eliminate blocks where pre/post settings cause them to fall outside the data range
	******************************************************************************/
	if((fpin=fopen(cmtfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: time-file \"%s\" not found\n\n",thisprog,cmtfile);exit(1);}
	nblocks=0;
	kk= sizeof(*start);
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf",&aa)!=1) continue;
		if(isfinite(aa)) {
			if((start= realloc(start,(nblocks+1)*kk))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			start[nblocks]= aa;
			nblocks++;
	}}
	fclose(fpin);
	if(nblocks<1) {fprintf(stderr,"--- Warning[%s]: no start-times in file %s\n",thisprog,cmtfile);exit(0);}

	/* determine first & last block to use */
	if(setfirst<0) setfirst=0;
	if(setlast<0) setlast=(nblocks-1);
	if(setverb>0) {
		fprintf(stderr,"\tuse blocks ");
		if(setfirst==-1) fprintf(stderr,"0 to ");
		else fprintf(stderr,"%ld to ",setfirst);
		if(setlast==-1) fprintf(stderr,"[end]\n");
		else fprintf(stderr,"%ld\n",setlast);
	}
	/* reset starts accordingly */
	for(ii=jj=0;ii<nblocks;ii++) { if(ii>=setfirst && ii<=setlast) start[jj++]= start[ii]; }
	nblocks= jj;
	if(nblocks<1) {fprintf(stderr,"--- Warning[%s]: no blocks remain after selecting between -first(%ld) and -last(%ld)\n",thisprog,setfirst,setlast);exit(0);}

	/* now screen for valid pre/post settings and finite "zero" */
	for(block=jj=0;block<nblocks;block++) {
		index2= start[block]; // "zero"
		if(!isfinite(dat1[index2])) {fprintf(stderr,"--- Warning[%s]: block %ld skipped- data at start-time (zero) is invalid\n",thisprog,block);continue;}
		if(index2>=setpre) index1= index2-setpre; // "pre"
		else {fprintf(stderr,"--- Warning[%s]: block %ld skipped- start (%ld) less than -pre (%ld)\n",thisprog,block,index2,setpre);continue;}
		index3= index2+setpost; // "post"
		if(index3>=ndat1) {fprintf(stderr,"--- Warning[%s]: block %ld skipped- end (%ld) exceeds data length (%ld)\n",thisprog,block,index3,ndat1);continue;}
		/* if all criteria met, save the block */
		start[jj++]= start[block];
	}
	nblocks= jj;
	if(nblocks<1) {fprintf(stderr,"--- Warning[%s]: no blocks remain after selecting between -first(%ld) and -last(%ld)\n",thisprog,setfirst,setlast);exit(0);}

	//TEST: for(block=0;block<nblocks;block++) printf("block %ld start %ld\n",block,start[block]);


	/******************************************************************************
	ALLOCATE MEMORY FOR TEMP DATA STORAGE
	******************************************************************************/
	/* allocate memory for aligned data */
	dat2= realloc(dat2,(ndat2*sizeof(*dat2))); /* actual block length is 1 larger than stop-start */
	/* allocate memory for median-data calculations */
	dat3= realloc(dat3,(nblocks*sizeof(*dat3)));
	/* allocate memory for mean arrays - allow for binning to reduce required memory size */
	sumdat2=calloc(ndat2,sizeof(double));

	if(dat2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	if(dat3==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	if(sumdat2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}


	/********************************************************************************
	NOISY OUTLIER EXCLUSION
	********************************************************************************/
	if(setnoise>=0.0) {
		jj= xf_outlier2_f(dat1,ndat1,start,nblocks,ndat2,setpre,"median","diff",setnoise,result_d,message);
		if(jj<0) {fprintf(stderr,"%s/%s\n",thisprog,message); exit(1); }
		if(jj!=nblocks) {
			kk= nblocks-jj;
			nblocks= jj			if(nblocks==0) {fprintf(stderr,"--- Warning[%s]: no blocks remain after selecting between -first(%ld) and -last(%ld)\n",thisprog,setfirst,setlast);exit(0);}
			if(setverb>0) fprintf(stderr,"\t%ld outlier blocks (threshold %g mean %g sd %g)\n",kk,result_d[0],result_d[1],result_d[2]);
		}
	}


	/*************************************************************************************************
	NOW ALIGN THE DATA
	- read each block - scan entire record for data falling within this block
	- this is the only strategy which allows for the possibility of overlapping blocks
	- we must consider the possibility of overlapping blocks because of the pre-sample and duration settings
	*************************************************************************************************/
	if(setverb>0) fprintf(stderr,"\tBlocks used:\n");
	usedblocks=0;

	/* for each data block */
	for(ii=0;ii<nblocks;ii++) {

		index2= start[ii]; //" zero"
		index1= index2-setpre; // "pre"
		index1b= index2-setpn; // set start index for normalization
		index1c= index2-setpnx; // set stop index for normalization - e.g. if -pnx = 1, sample zero will be excluded
		index3= index2+setpost; // "post"
		// set pointer to current block for dat1
		pdat1= (dat1+index1);

		/********************************************************************************/
		/* NORMALIZE THE DATA */
		/* form normalization methods requiring it, calculate mean and standard deviation in pre-sample portion of the block */
		if(setnorm==0) {
			for(jj=0;jj<ndat2;jj++) { dat2[jj]= pdat1[jj]; }
		}
		else if(setnorm<4) {
			kk=0; sum=0.0; sumofsq=0.0;
			for(jj=index1b;jj<=index1c;jj++) { cc=dat1[jj]; if(isfinite(cc)) { sum+=cc; sumofsq+=cc*cc; kk++; }	}
			if(kk<1) {fprintf(stderr,"--- Warning[%s]: block %ld skipped- no valid data in pre-sample interval\n",thisprog,ii);continue;}
			else if(sum==0.0) mean=0.0;
			else mean=sum/(double)kk;
			if(kk>1) stddev= sqrt((double)((kk*sumofsq-(sum*sum))/(double)(kk*(kk-1))));
			else stddev=1.1;
			//TEST: fprintf(stderr,"%ld	%ld	%ld	%g %g\n",index1b,index1c,ndat2,mean,stddev); continue;

			if(setnorm==1) {
				cc=dat1[index2];
				if(!isfinite(cc)) {fprintf(stderr,"--- Warning[%s]: block %ld skipped- invalid data at start-sample invalidates entire block\n\n",thisprog,ii);continue;}
				for(jj=0;jj<ndat2;jj++) { dat2[jj]= pdat1[jj]-cc; }
			}
			else if(setnorm==2) { for(jj=0;jj<ndat2;jj++) { dat2[jj]=  pdat1[jj]-mean;}}
			else if(setnorm==3) { for(jj=0;jj<ndat2;jj++) { dat2[jj]= (pdat1[jj]-mean)/stddev;}}
		}
		/* normalize to the overall mean in the block  */
		else if(setnorm==4) {
			// get the mean
			sum= 0.0; for(jj=0;jj<ndat2;jj++) sum+= pdat1[jj];	mean= sum/ndat2;
			// define dat2
			for(jj=0;jj<ndat2;jj++) dat2[jj]= pdat1[jj]/mean;
		}
		/* normalize to the linear trend in the block  */
		else if(setnorm==5) {
			// copy dat1 to dat2
			for(jj=0;jj<ndat2;jj++) dat2[jj]= pdat1[jj];
			kk= xf_detrend1_f(dat2,(size_t)ndat2,result_d);
		}

		/********************************************************************************/
		/* BIN THE DATA FOR THIS BLOCK (IF REQUIRED) TO FACILITATE AVERAGING ACROSS BLOCKS BY OTHER PROGRAMS */
		/* - dat2, n3 and zero will be adjusted */
		/* - binsize is converted from the number of samples averaged (may be a fraction) to seconds */
		zero=setpre;
		n3=ndat2;

		if(setnbins>0) {
			binsize= xf_bin1a_f(dat2,&n3,&zero,setnbins,message);
			if(binsize==0.0) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
			binint= binsize*sampint;
		}
		else {
			binsize=1.0;
			binint= sampint;
		}

		/* calculate the mean */
		if(setout==0) {
			for(jj=0;jj<n3;jj++) sumdat2[jj]+=dat2[jj];
		}
		/* otherwise output each block of aligned data: [block=i] [time] [data]  - this will be a shortened array if setnbins>0 */
		/* note adjustment of calculation to avoid setting a size_t variable to negative */
		else if(setout==1) {
			if(zero==0) bb=0.0; else bb=(double)(zero)*(-1)*binint;
			for(jj=0;jj<n3;jj++) { if(jj==zero) bb=0; printf("%ld\t%g\t%f\n",ii,bb,dat2[jj]); bb+=binint; }
		}
		usedblocks++;

		if(setverb>0) fprintf(stderr,"\tblock: %ld	pre:%ld start:%ld	stop:%ld\n",ii,index1,index2,index3);



	} // END OF BLOCKS LOOP


	/*************************************************************************************************
	OUTPUT THE AVERAGE ALIGNED DATA:
	[time] [data]  - this will be a shortened array if setnbins>0
	note adjustment of calculation to avoid setting a size_t variable to negative
	*************************************************************************************************/
	if(setout==0 && usedblocks>0) {
		if(zero==0) bb=0.0; else bb=(double)(zero)*(-1.0)*binint; // initial timestamp
		for(jj=0;jj<n3;jj++) { if(jj==zero) bb=0; printf("%g\t%f\n",bb,(sumdat2[jj]/usedblocks));	bb+=binint; }
	}

END1:
	if(dat1!=NULL) free(dat1);
	if(dat2!=NULL) free(dat2);
	if(dat3!=NULL) free(dat3);
	if(sumdat2!=NULL) free(sumdat2);
	if(start!=NULL) free(start);
	if(setverb>0) fprintf(stderr,"\n");
	exit(0);
	}
