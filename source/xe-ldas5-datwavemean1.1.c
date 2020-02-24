#define thisprog "xe-ldas5-datwavemean1"
#define TITLE_STRING thisprog" v 2: 20.July.2017 [JRH]"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>

#define MAX_LINELEN 1000

/*
<TAGS>LDAS dt.spikes math</TAGS>

CHANGES THIS VERSION:
v 2: 20.July.2017 [JRH]
	- allow specification of a total number of channels
v 2: 17.March.2017 [JRH]
	- allow specification of a list of channels that are good (1) or not (0)
	- for bad channels, NAN is output as the mean
v 2: 30.January.2017 [JRH]
	- add option to output cluster,sample,mean,std.dev
v 2: 12.November.2016 [JRH]
	- include output of cluster 0
v 2: 10.October.2016 [JRH]
	- save channel list to header
	- bugfix memory allocation for sampindex (should have been long instead of int)
v 2: 22 March 2016 [JRH]
	- add error check for if clubt[ii] exceeds ndat
v 2: 14 March 2016 [JRH]
	- uses xe-datwavemean v.1.7 (24 March 2010) as the starting point
	- uses xf_readbin2_v to read entire .dat file in one block
	- uses xf_readclub1 to read cluster time/id
v.1.7
	- skip processing and output if a probe is empty (clumax=0 or nclu=0)
v.1.6
	- addition of CLUSTER_MAX and WAVES_START to header
v.1.5
	- reads Axona .set-file
	- outputs true voltage values based on gain on channel zero
	- outputs SAMPLE_RATE and other stats at top of waveform file
v.1.4
	- changed output so first sample from each wire is subtracted from runing waveform total,
	 instead of first sample from the first wire
*/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_readclub1(char *infile1, char *infile2, long **clubt, short **club, char *message);
off_t xf_readbin2_v(FILE *fpin, void **data, off_t startbyte, off_t bytestoread, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

// generic variables
char line[MAX_LINELEN],templine[MAX_LINELEN],*pline,*pcol,*tempcol,message[MAX_LINELEN];
char infile[256],outfile[256],path_prog[256],path_data[256],temp_str[256];
int sizeofint=sizeof(int),sizeoflong=sizeof(long),sizeofdouble=sizeof(double);
int w,x,y,z,col,colmatch,allfiletot=0,resulti[32];
long ii,jj,kk,mm,nn,oo,pp;
float a,b,c,d;
double aa,bb,cc;
FILE *fpin;
// program specific variables
void *pdat=NULL;
short *dat=NULL,*club=NULL;
long nprobes=1,nchanlist,*probechan=NULL,*goodchan=NULL;
int setbits=12,datasize=2;
long *sampindex=NULL;
long probe,cluster,clumax=0;
long *clubt=NULL,*spiketot=NULL,*index=NULL,*wave,datpos,wavelen,nclu,nres;
double uv_per_unit;
double *sdev=NULL;
off_t ndat,readparam[16];
// command-line variables
char infile1[256],infile2[256],infile3[256],*setchanlist=NULL,*setgoodlist=NULL;
int setout=1;
long setnchan=16;
long spklen=40,spkpre=8; // number of samples defining waveforms
double setrate=19531.25, setgain=70.0, setvmax=0.8;

/*********************************************************************************************************/
/* Print instructions if only one argument */
/*********************************************************************************************************/
if(argc<4) {
	fprintf(stderr,"----------------------------------------------------------------------\n");
	fprintf(stderr,"%s\n",TITLE_STRING);
	fprintf(stderr,"----------------------------------------------------------------------\n");
	fprintf(stderr,"Calculate mean waveform from a given trial (all probes)\n");
	fprintf(stderr,"USAGE:\n");
	fprintf(stderr,"	%s [dat] [clubt] [club] [options]\n",thisprog);
	fprintf(stderr,"	[dat]: interlaced binary data file (short int)\n");
	fprintf(stderr,"	[clubt]: binary file containing cluster-times (long int)\n");
	fprintf(stderr,"	[club]: binary file containing cluster-IDs (short int)\n");
	fprintf(stderr,"VALID OPTIONS: defaults in []\n");
	fprintf(stderr,"	-nch: number of channels in input [%ld]\n",setnchan);
	fprintf(stderr,"	-chl: channel-list (CSV) specifying order of channels [%s]\n",setchanlist);
	fprintf(stderr,"	-chg: CSV list of good (1, default) or bad (0) channels for -chl [%s]\n",setchanlist);
	fprintf(stderr,"	-spklen: samples comprising the waveform on each channel [%d]\n",spklen);
	fprintf(stderr,"	-spkpre: samples before spike-detection event to be included [%d]\n",spkpre);
	fprintf(stderr,"	-sr: .dat sample-rate (Hz) [%g]\n",setrate);
	fprintf(stderr,"	-gain: gain (amplification factor) [%g]\n",setgain);
	fprintf(stderr,"	-vmax: maximum voltage [%g]\n",setvmax);
	fprintf(stderr,"	-out: output: 1=.wfm file, 2=cluster,sample,mean,std.dev [%d]\n",setout);
	fprintf(stderr,"OUTPUT: Waveform file, format= [c] [n] [voltages]\n");
	fprintf(stderr,"	[c]: cluster id for this probe\n");
	fprintf(stderr,"	[n]: number of spikes contributing to the mean waveform\n");
	fprintf(stderr,"	[voltages]: series of [nsamps] mean voltages, in uV\n");
	fprintf(stderr,"EXAMPLES:\n");
	fprintf(stderr,"	%s tr01.dat tr01.clubt tr01.club -spklen 32 -spkpre 8\n",thisprog);
	fprintf(stderr,"\n");
	exit(0);
}

	/************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	************************************************************/
	sprintf(infile1,"%s",argv[1]);
	sprintf(infile2,"%s",argv[2]);
	sprintf(infile3,"%s",argv[3]);
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if(ii>=argc) break;
			else if(strcmp(argv[ii],"-nch")==0)    setnchan=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-chl")==0)    setchanlist=argv[++ii];
			else if(strcmp(argv[ii],"-chg")==0)    setgoodlist=argv[++ii];
			else if(strcmp(argv[ii],"-spklen")==0) spklen=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-spkpre")==0) spkpre=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-sr")==0)     setrate=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-gain")==0)   setgain=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-vmax")==0)   setvmax=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)    setout=atoi(argv[++ii]);
			else {fprintf(stderr,"\b\n\t*** %s [ERROR]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(strcmp(infile1,"stdin")==0) {fprintf(stderr,"\b\n\t*** %s [ERROR]: this program does not accept stdin input: chose file name instead\n\n",thisprog);exit(1);}
	if(setout!=1&&setout!=2) {fprintf(stderr,"\b\n\t*** %s [ERROR]: invalid -out (%d) - must be 1 or 2\n\n",thisprog,setout);exit(1);}

	/********************************************************************************************************
	CALCULATE THE uV/unit VALUE - TO CONVERT THE VALUES BACK TO ORIGINAL (MICRO)VOLTAGES
	*********************************************************************************************************/
	aa= (1000000*setvmax)/setgain;
	bb= pow(2.0,(double)setbits);
	uv_per_unit= aa/bb; // for conversion of .dat input values to true voltages (microvolts)
	//fprintf(stderr,"uv_per_unit=%g\n",uv_per_unit);


	/*********************************************************************************************************/
	/* READ THE .DAT FILE
	/*********************************************************************************************************/
	fprintf(stderr,"\tReading %s\n",infile1);
	if((fpin=fopen(infile1,"rb"))==0) {fprintf(stderr,"\b\n\t*** %s [ERROR]: could not open .dat file: %s\n\n",thisprog,infile1);exit(1);}
	ndat = xf_readbin2_v(fpin,&pdat,0,0,message);
	fclose(fpin);
	if(ndat==0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	if(ndat%(setnchan*datasize)!=0) { fprintf(stderr,"\b\n\t*** %s [ERROR]: corrupt .dat file %s, is not %d channels of short integers\n\n",thisprog,infile1,setnchan); exit(1); }
	dat=(short *)pdat;
	ndat/=(setnchan*datasize);
	//jj=0; kk=16; for(ii=ndat-10;ii<ndat-20000;ii++) printf("%d\n",dat[ii*kk+jj]);



	/************************************************************
	READ THE CLUSTER TIMESTAMPS AND IDs
	***********************************************************/
 	fprintf(stderr,"\tReading cluster timestamps and IDs: %s,%s\n",infile2,infile3);
	nclu= xf_readclub1(infile2,infile3,&clubt,&club,message);
 	if(nclu<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
	/* determine maximum cluster-id and check timestamps against .dat file */
	clumax=0;
	for(ii=0;ii<nclu;ii++) {
		if((long)club[ii]>clumax) clumax=(long)club[ii];
		if(clubt[ii]>=ndat)  { fprintf(stderr,"\b\n\t*** %s [ERROR]: .clubt timestamp (%ld) exceeds samples in .dat file (%ld)\n\n",thisprog,clubt[ii],ndat); exit(1); }
	}

	/********************************************************************************************************
	BUILD THE LIST OF CHANNELS USED BY THE CURRENT PROBE - .DAT FILE MAY REPRESENT MULTIPLE PROBES
	- note that for silicon probes, all the channels in the .dat file represent a single functional probe
	- hence currently this list is built automatically
	*********************************************************************************************************/
	fprintf(stderr,"\tBuilding channel list (setnchan=%ld): ",setnchan);
	if((probechan=(long *)realloc(probechan,setnchan*sizeof(long)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if(setchanlist!=NULL) {
		index= xf_lineparse2(setchanlist,",",&nchanlist);
		if(index==NULL) {fprintf(stderr,"\n--- Error[%s]: xf_lineparse2 failed\n\n",thisprog);exit(1);}
		if(nchanlist<1) {fprintf(stderr,"\n--- Error[%s]: channel list is empty\n\n",thisprog);exit(1);}
		if(nchanlist!=setnchan) {fprintf(stderr,"\n--- Error[%s]: channel list [n=%ld] does not match channel total [n=%ld] \n\n",thisprog,nchanlist,setnchan);exit(1);}
		for(ii=0;ii<setnchan;ii++) probechan[ii]=atol(setchanlist+index[ii]);
	}
	else {
		for(ii=0;ii<setnchan;ii++) probechan[ii]=ii;
	}
	for(ii=0;ii<(setnchan-1);ii++) fprintf(stderr,"%d,",probechan[ii]);fprintf(stderr,"%d\n",probechan[(setnchan-1)]);

	/* now read the list specifying whether each channel is good (1) or not (0) */
	fprintf(stderr,"\tBuilding channel quality list: ");
	if((goodchan=(long *)realloc(goodchan,setnchan*sizeof(long)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if(setgoodlist!=NULL) {
		if(index!=NULL) free(index);
		index= xf_lineparse2(setgoodlist,",",&kk);
		if(index==NULL) {fprintf(stderr,"\n--- Error[%s]: xf_lineparse2 failed\n\n",thisprog);exit(1);}
		if(kk!=setnchan) {fprintf(stderr,"\n--- Error[%s]: channel-quality list [n=%ld] does not match channel total [n=%ld] \n\n",thisprog,kk,setnchan);exit(1);}
		for(ii=0;ii<setnchan;ii++) goodchan[ii]=atol(setgoodlist+index[ii]);
	}
	else {
		for(ii=0;ii<setnchan;ii++) goodchan[ii]=1;
	}
	for(ii=0;ii<(setnchan-1);ii++) fprintf(stderr,"%d,",goodchan[ii]);fprintf(stderr,"%d\n",goodchan[(setnchan-1)]);

 	/* allocate memory for .dat buffer and multi-cluster waveform storage */
 	wavelen = spklen*setnchan; // total length of compound waveform
 	if( (wave=(long *)calloc((clumax+1)*wavelen,sizeoflong))==NULL ) {fprintf(stderr,"\b\n\t*** %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);};
	if( (sdev=(double *)calloc((clumax+1)*wavelen,sizeofdouble))==NULL ) {fprintf(stderr,"\b\n\t*** %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);};
	if( (spiketot=(long *)calloc((clumax+1),sizeof(long)))==NULL ) {fprintf(stderr,"\b\n\t*** %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);};
 	if( (sampindex=(long*)malloc(wavelen*sizeof(long)))==NULL ) {fprintf(stderr,"\b\n\t*** %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);};

	/************************************************************
 	BUILD INDEX FOR TRANSPOSING .DAT RECORDS INTO wave[clu][wavelen]
	- takes into acount defined channel-order
	************************************************************/
	for(ii=0;ii<setnchan;ii++) {
		for(jj=0;jj<spklen;jj++) {
			kk=ii*spklen+jj;
			sampindex[kk] = jj*setnchan + probechan[ii];
		}
	}
 	//TEST: for(ii=0;ii<wavelen;ii++) printf("%d %d\n",ii,sampindex[ii]); exit(0);

	/************************************************************
	STORE THE SUMMED WAVEFORMS
	- values normalized to sample-zero on each channel
	***********************************************************/
	fprintf(stderr,"\tExtracting waveforms...\n");
	for(ii=0;ii<nclu;ii++) {
		cluster= club[ii];
		spiketot[cluster]++; // count total spikes in each cluster
		datpos= (clubt[ii]-spkpre)*setnchan; // start position of waveform
		for(jj=0;jj<setnchan;jj++) {
			if(goodchan[jj]==0) continue;
			mm= jj*spklen;
			for(kk=0;kk<spklen;kk++) {
				nn= mm+kk;
				// build waveform total, .dat voltage minus the voltage from sample-0 from each wire
				wave[cluster*wavelen+nn] += ( dat[datpos+sampindex[nn]] - dat[datpos+sampindex[mm]] );
	}}}

	/************************************************************
	CALCULATE STANDARD DEVIATION
	***********************************************************/
	if(setout==2) {
		fprintf(stderr,"\tCalculating standard deviations...\n");
		for(ii=0;ii<nclu;ii++) {
			cluster= club[ii];
			aa= spiketot[cluster];
			datpos= (clubt[ii]-spkpre)*setnchan; // start position of waveform
			for(jj=0;jj<setnchan;jj++) {
			if(goodchan[jj]==0) continue;
			for(kk=0;kk<spklen;kk++) {
				mm= jj*spklen;
				nn= mm+kk;
				bb= (dat[datpos+sampindex[nn]] - dat[datpos+sampindex[mm]]) ; //corrected current datapoint
				cc= (double)wave[cluster*wavelen+kk]/aa; // mean
				sdev[cluster*wavelen+nn] += (bb-cc)*(bb-cc); // std.dev still needs to be square-rooted and divided by nspikes-1
		}}}
	}

	/************************************************************
	OUTPUT WFM FILE - ONE ROW PER CLUSTER
	************************************************************/
	if(setout==1) {
		printf("PROBE %d\n",probe);
		printf("N_CHANNELS %d\n",setnchan);
		printf("CHANNEL_LIST ");
		for(ii=0;ii<(setnchan-1);ii++) printf("%ld,",probechan[ii]); printf("%ld\n",probechan[(setnchan-1)]);
		printf("SAMPLES_PER_CHANNEL %d\n",spklen);
		printf("SAMPLES_PRE_PEAK %d\n",spkpre);
		printf("SAMPLE_RATE %.2f\n",setrate);
		printf("CLUSTER_MAX %d\n",clumax);
		printf("WAVES_START\n");
		for(cluster=0;cluster<=clumax;cluster++) {
			if(spiketot[cluster]<=0) continue;
			printf("%d\t%ld\t",cluster,spiketot[cluster]);
			for(jj=0;jj<setnchan;jj++) {
				mm= jj*spklen;
				for(kk=0;kk<spklen;kk++) {
					nn= mm+kk;
					if(goodchan[jj]==0) aa=NAN;
					else aa= (wave[cluster*wavelen+nn]/spiketot[cluster]) * uv_per_unit;
					printf("%.3lf ",aa);
				}
			}
			printf("\n");
		}
	}

	/************************************************************
	OUTPUT TABLE OF CLUSTER, SAMPLE, MEAN AND STANDARD DEVIATION
	************************************************************/
	if(setout==2) {
		printf("cluster\tsample\tmean\tstddev\n");
		for(cluster=0;cluster<=clumax;cluster++) {
			if(spiketot[cluster]<=0) continue;
			for(jj=0;jj<setnchan;jj++) {
				mm= jj*spklen;
				for(kk=0;kk<spklen;kk++) {
					nn= mm+kk;
					if(goodchan[jj]==0) aa=bb=NAN;
					else {
						aa= (wave[cluster*wavelen+nn]/spiketot[cluster]) * uv_per_unit; // mean
						bb= (sqrt(sdev[cluster*wavelen+kk])/(nn-1)) * uv_per_unit; // standard deviation
					}
					printf("%ld\t%ld\t%.3lf\t%.3lf\n",cluster,nn,aa,bb);
	}}}}

	/************************************************************
	FREE MEMORY AND EXIT
	************************************************************/
	if(dat!=NULL) free(dat);
	if(clubt!=NULL) free(clubt);
	if(club!=NULL) free(club);
	if(probechan!=NULL) free(probechan);
	if(goodchan!=NULL) free(goodchan);
	if(spiketot!=NULL) free(spiketot);
	if(index!=NULL) free(index);
	if(sampindex!=NULL) free(sampindex);
	if(wave!=NULL) free(wave);
	if(sdev!=NULL) free(sdev);
	exit(0);

}
