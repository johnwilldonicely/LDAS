#define thisprog "xe-ldas5-clucombine1"
#define TITLE_STRING thisprog" v 4: 17.March.2017 [JRH]"
#define MAXLINE 1000
#define HISTMAXSIZE 200

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

/*
<TAGS>LDAS dt.spikes</TAGS>

Versions History:
v 4: 26.June.2017 [JRH]
	- remove output of "total" refract,t and p values
		- we should always rely on BOTH sides of the histogram having refractoriness

v 4: 3.May.2017 [JRH]
	- remove wave-high-pass-filter - irrelevent for peak-amplitude-depth-profile correlations
	- bugfix columns of output
	- bugfix to autocor update upon combination - cluid2 should be updated, not cluid1
		- may not make much difference if a cross-refractory criterion is applied before combining

v 4: 2.May.2017 [JRH]
	- remove adelta tests and output (anticipated change in auto-refractoriness)
		- this is not appreciably different from the cross-correlogram

v 4: 27.March.2017 [JRH]
	- bugfix: revised autocor intervals need to be built from the original club/t records, not the windows centred on cluid1
	- bugfix: free intervals before each call to wf_wint
	- bugfix: in revised histograms, convert values to seconds

v 4: 25.March.2017 [JRH]
	- replace old waveform correlation score with correlation based on voltage at peaks on each channel
	- this is much more representative of similarity that use of the entire waveform, which can include a lot of redundant info
	- new function xf_wavecor2_f to support this

v 4: 17.March.2017 [JRH]
	- add delta-autorefract score to set criterion for change in recfractoriness expected from combining clusters-pairs
	- add xf_wint3_ls function to support this

v 4: 17.March.2017 [JRH]
	- bugfix: waveform correlations now don't become NAN just because some waveforms have dead channels
		- follows from change to datwavemean program which allocates dead channels NAN values for the waveform mean
		- required mods to functions xf_correlate_simple_f/d do they now will ignore non-finite input values

v 4: 27.February.2017 [JRH]
	- drop option for cumulative combining
		- now clusters-pairs are always combined as they are tested
		- this should make things more robust
		- for -v 0 (report only) behaviour is sill as for previous no-cumulative outut
			- ie. all pairs are tested once

v 4: 20.February.2017 [JRH]
	- add sign argument to specify expected direction of spikes (typically negative, so sign= -1)


v 4: 14.February.2017 [JRH]
	- retire histrefract1_d - replace with more general histratio1_d

v 4: 12.November.2016 [JRH]
	- renamed to ldas5-clucombine1
	- adopted new club(t) file-read method
	- read waveforms from .wfm file instead of legacy binary .spk mfiles

v 2.4: 18.April.2016 [JRH]
	- update correlate function used - switch to simple float version

v 2.4: 1 February 2011
	- switched to using long int for data counters and wint/hist1d functions
v.2.3
	- use function xf_rewindow to speed up cross-correlation calculations
	- narrowed histogram window to +-11ms  to improve speed

v.1.2
	- verbose output now more verbose - prints left and right sides of histograms
*/

/* external functions start */
long xf_readclub1(char *infile1, char *infile2, long **clubt1, short **club1, char *message);
double xf_readwave1_f(char *infile, short **id, long **count, float **data, short **wlistc, long *resultl, char *message);
long xf_rewindow2_ls(long *t1, short *c1, long n1, short id, long winsize, long *t2, short *c2);
long *xf_wint1_ls(long *time, short *group, long nn, short g1, short g2, long winsize, long *nintervals);
long *xf_wint3_ls(long *time, short *group, long nn, short g1, short g2, long winsize, long *nintervals);
long xf_hist1_l(long *data, long nn, double *histx, double *histy, long bintot, long min, long max, int format);
double xf_histratio1_d(double *tsec, double *val, long bintot, double z1, double z2, double *result);
double xf_histratio2_d(double *histy, long nbins_tot, long nbins_c, long nbins_side, double *result_d, char *message);
double xf_correlate_simple_f(float *x, float *y, long nn, double *result_d);
long xf_wavepeak2_f(float *wave, long nchan, long spklen, long setzero, int sign, float *resultf, char *message);
double xf_wavecor2_f(float *wave1, float *wave2, long nchan, long spklen, long zero, char *message);
void xf_qsortindex1_f(float *data, long *index,long nn);
double xf_prob_T1(double t, long df, int tails);
int xf_writebin2_v(char *outfile, void *data0, size_t nn, size_t datasize, char *message);
int xf_writewave1_f(char *outfile, short *wid, long *wn, float *wmean, short *wlistc, long *resultl, long makez, double srate, char *message);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],line[256],*pline,*pcol,message[MAXLINE];
	int v,w,x,y,z,col,result_i[32];
	int sizeofint=sizeof(int),sizeofshort=sizeof(short int),sizeoflong=sizeof(long int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	long ii,jj,kk,mm,nn,index0,index1,index2,resultl[32];
	float a,b,c,d,resultf[32];
	double aa,bb,cc,dd,ee,ff,gg,hh;
	FILE *fpin,*fpout;

	/* program-specific variables */
	char outfileclub[64],outfilewfm[64],outfilehist[64];
	double resultwave[40],resulthist1[40],wsfreq;
	short *wid=NULL,*wlistc=NULL;
	long *wclun=NULL, *wavecorchan=NULL, wnchans, wspklen, wspkpre, wnclust, wtotlen, wclumax, wprobe;
	float *wmean=NULL, *wmeantemp=NULL, wavemin,wavemax,wavecor;

	short *club1=NULL,*club2=NULL,*combine=NULL,cluid1,cluid2,clumin,clumax;
	long *clubt1=NULL,*clubt2=NULL,*clun=NULL,*intervals=NULL,spiketot,spiketot2,nclust;

	int pass,combine_flag,combine_count;
	long intervaltot,histlow,histhigh,histbintot,indexmin,indexmax,ncorchan;
	float *autocor=NULL,refract;
	double *histx=NULL,*histy=NULL,histhalf;
	double r2,temp_ref,temp_t,temp_p;

	FILE *fphist=NULL;

	/* arguments */
	char fileclub1[256],fileclubt1[256],filewfm[256];
	int setpass=1,setverb=2,setminspikes=25,setcumulative=1,setsign=-1,sethistout=0;
	float setrefmax=0.01,setauto=0.02,setwavecor=0.975;
	float setmaxratio=0.8,setmint=3.0,setmaxp=0.5;
	long setxcorside=13,setxcorcentre=2;
	double setacorz1=0.015,setacorz2=0.002;

	sprintf(outfileclub,"temp_%s.club",thisprog);
	sprintf(outfilewfm,"temp_%s.wfm",thisprog);
	sprintf(outfilehist,"temp_%s.hist",thisprog);

	ncorchan=16; // set number of channels used for waveform correlations

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<4) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Combines clusters with common refractoriness and similar waveforms\n");
		fprintf(stderr,"- autocor refract= histogram +-2ms spike-count vs. +-15ms\n");
		fprintf(stderr,"- crosscor refract= histogram +-2ms spike-count vs. +-10ms\n");
		fprintf(stderr,"- one-sided t-tests for spiking probability (0-2 vs 2-10ms)\n");
		fprintf(stderr,"- cluster-pairs meeting criteria are combined as they are tested\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [in1] [in2] [in3] [arguments]\n",thisprog);
		fprintf(stderr,"VALID ARGUMENTS (defaults in []):\n");
		fprintf(stderr,"	[in1]: timestamps (.clubt) file for a given probe\n");
		fprintf(stderr,"	[in2]: cluster-id (.club) file for a given probe\n");
		fprintf(stderr,"	[in3]: waveform (.wfm) file a given probe\n");
		fprintf(stderr,"	-sign: spike detection, -1=NEG, 1=POS [%d]\n",setsign);
		fprintf(stderr,"	-pass: set number of passes[%d]\n",setpass);
		fprintf(stderr,"	-hist: output histograms (.hist file) (0=NO 1=YES) [%d]\n",sethistout);
		fprintf(stderr,"	-v: verbocity [%d]\n",setverb);
		fprintf(stderr,"		0= report only, no combining, single pass, all pairs\n");
		fprintf(stderr,"		1= combine only, no report\n");
		fprintf(stderr,"		2= combine + report: \n");
		fprintf(stderr,"- combine criteria:\n");
		fprintf(stderr,"	-s: minimum spikes (central histogram) [%d]\n",setminspikes);
		fprintf(stderr,"	-a: max auto-refractoriness for both clusters [%g]\n",setauto);
		fprintf(stderr,"	-ad: max auto-refract. change, relative to worst of pair [%g]\n",setauto);
		fprintf(stderr,"	-r: maximum xcor refractoriness (both sides) [%g]\n",setmaxratio);
		fprintf(stderr,"	-t: minimum t-score (either side) [%g]\n",setmint);
		fprintf(stderr,"	-p: maximum p-value (either side) [%g]\n",setmaxp);
		fprintf(stderr,"	-w: minimum multi-channnel waveform peak-correlation [%.4f]\n",setwavecor);
		fprintf(stderr,"		- signed R-sq. for values at peak, in depth order\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s file.clubt file.club file.wfm\n",thisprog);
		fprintf(stderr,"FILE OUTPUT (-v 1 or 2):\n");
		fprintf(stderr,"	modified .club file %s\n",outfileclub);
		fprintf(stderr,"	modified .wfm file %s\n",outfilewfm);
		fprintf(stderr,"	histogram file (optional) %s\n",outfilehist);
		fprintf(stderr,"REPORT OUTPUT (-v 0 or 2):\n");
		fprintf(stderr,"	c1 c2 a1 a2   nxcor  rl tl pl   rr tr pr   wavecor combine\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"	[c1 c2 a1 a2]: cluster IDs and autocorrelation\n");
		fprintf(stderr,"	[nxcor]: number of spikes contriibuting to the cross-correlation\n");
		fprintf(stderr,"	[rl tl pl]: left-side refractoriness, t-stat & prob.\n");
		fprintf(stderr,"	[rr tr pr]: as above for right-hand side\n");
		fprintf(stderr,"	[wavecor]: waveform correlation, 5 channels centred on\n");
		fprintf(stderr,"	           channel with largest mean spike (see -sign)\n");
		fprintf(stderr,"	[combine]: combine criteria bit-flag, 1=histogram, 2=wavecor\n");
		fprintf(stderr,"	           NOTE: must be 3 (both criteria met) to combine\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	sprintf(fileclubt1,"%s",argv[1]);
	sprintf(fileclub1,"%s",argv[2]);
	sprintf(filewfm,"%s",argv[3]);

	for(ii=4;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if(ii>=argc) break;
			else if(strcmp(argv[ii],"-v")==0)   { setverb=atoi(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-sf")==0)  { wsfreq=atof(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-pass")==0){ setpass=atoi(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-sign")==0){ setsign=atoi(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-hist")==0){ sethistout=atoi(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-a")==0)   { setauto=atof(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-s")==0)   { setminspikes=atoi(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-t")==0)   { setmint=atof(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-p")==0)   { setmaxp=atof(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-r")==0)   { setmaxratio=atof(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-w")==0)   { setwavecor=atof(argv[ii+1]); ii++;}
			else {fprintf(stderr,"Error[%s]: invalid command line argument \"%s\"\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setsign!=-1 && setsign!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -sign [%d] must be -1 or 1\n\n",thisprog,setsign);exit(1);}

	/* for simple report output, */
	if(setverb==0) setpass=1;

	/********************************************************************************
	READ THE CLUSTER TIMESTAMPS AND IDs
	********************************************************************************/
 	spiketot= xf_readclub1(fileclubt1,fileclub1,&clubt1,&club1,message);
 	if(spiketot<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
	//TEST: for(ii=0;ii<spiketot;ii++) printf("%ld	%d\n",clubt1[ii],club1[ii]); exit(0);
	/* determine the lowest highest cluster id */
	clumin=clumax= club1[0];
	for(ii=0;ii<spiketot;ii++) {
		cluid1=club1[ii];
		if(cluid1<clumin) clumin=cluid1;
		if(cluid1>clumax) clumax=cluid1;
	}
	/* calculate spike-count in each cluster */
	clun= calloc((clumax+1),sizeof(*clun)); // holds the total spikes in each cluster
	if(clun==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n",thisprog);exit(1);}
	for(ii=0;ii<spiketot;ii++) clun[club1[ii]]++;
	/* determine the total number of populated clusters */
	nclust=0; for(ii=clumin;ii<=clumax;ii++) if(clun[ii]>0) nclust++;
	//TEST: printf("club_max=%d\n",clumax);
	//TEST: for(ii=0;ii<=clumax;ii++) if(clun[ii]>0) printf("%ld	%d\n",ii,clun[ii]); exit(0);


	/********************************************************************************
	READ THE .WFM FILE TO GET MEAN WAVEFORMS AND WAVEFORM PARAMETERS
	- waveform is stored as wavemean[cluster][channel][sample]
	- channel-order is defined by wlistc[]
	- id's and spike-counts for each cluster stored in wid[] and wclun[] respectively
	********************************************************************************/
	wsfreq= xf_readwave1_f(filewfm,&wid,&wclun,&wmean,&wlistc,resultl,message);
	if(wsfreq<0) {fprintf(stderr,"\nError[%s]: %s\n\n",thisprog,message); exit(1); }
	wnclust= resultl[0]; // total clusters, not the same as max_cluster_id (resultl[5])
	wnchans= resultl[1]; // number of channels per combined-waveform
	wspklen= resultl[2]; // number of samples per channel
	wspkpre= resultl[3]; // number of samples before time zero
	wtotlen= resultl[4]; // total length of combined-waveform
	wclumax= resultl[5]; // largest cluster number
	wprobe= resultl[6]; // probe-id
	/* check maximum-cluster-id's match in club and wfm records */
	if(wclumax!=clumax) {fprintf(stderr,"\n--- Error[%s]: max cluster in waveform file (%ld) does not match the club file (%d)\n\n",thisprog,wclumax,clumax);exit(1);}
	/* check that the total number of populated clusters also matches */
	if(wnclust!=nclust) {fprintf(stderr,"\n--- Error[%s]: no. of clusters in waveform file (%ld) does not match the club file (%ld)\n\n",thisprog,wnclust,nclust);exit(1);}
	/* compare spike counts for each cluster  */
	for(ii=0;ii<wnclust;ii++) {if(wclun[ii]!=clun[wid[ii]]) {fprintf(stderr,"\n--- Error[%s]: cluster[%ld] count in waveform file (%ld) file does not match the club file (%ld)\n\n",thisprog,ii,wclun[ii],clun[wid[ii]]);exit(1);}}
	/* allocate memory for filtering data on each channel */
	wmeantemp= malloc(wtotlen * sizeof *wmeantemp);
	if(wmeantemp==NULL) { fprintf(stderr,"\b\n\tError[%s]: insufficient memory\n",thisprog);exit(1);}
	//TEST for(ii=0;ii<wnclust;ii++) fprintf(stderr,"%ld	%ld\n",wid[ii],wclun[ii]); exit(0);
	//TEST WAVEFORM: for(ii=0;ii<wnclust;ii++) {if(wclun[ii]>0) { index1=ii*wtotlen;for(jj=0;jj<wtotlen;jj++) printf("%ld\t%ld\t%g\n",wid[ii],jj,wmean[index1+jj]); }} exit(0);

	/********************************************************************************
	ALLOCATE MEMORY
	********************************************************************************/
	club2= malloc(spiketot * sizeof *club2); // temp-array - spike cluster-ids
	clubt2= malloc(spiketot * sizeof *clubt2); // temp array - spike times
	autocor= malloc((clumax+1) * sizeof *autocor); // holds the autocorrelation refractoriness for each cluster
	histx= malloc(HISTMAXSIZE * sizeof *histx);
	histy= malloc(HISTMAXSIZE * sizeof *histy);
	wavecorchan= calloc(wnclust,sizeof *wavecorchan); // holds start index for principal four channels of a given cluster

	if(clubt2==NULL||club2==NULL||autocor==NULL||histx==NULL||histy==NULL||wavecorchan==NULL) { fprintf(stderr,"\b\n\tError[%s]: insufficient memory\n",thisprog);exit(1);}

	/********************************************************************************
	DETERMINE wavecorchan FOR WAVEFORM CORRELATIONS (SET TO ZERO BY CALLOC AT START)
	- first channel of <ncorchan> channels to be used for correlations
	********************************************************************************/
	for(ii=0;ii<wnclust;ii++) {
		/* only calculate for populated clusters >0 */
		if(wclun[ii]>0 && wid[ii]>0) {
			/* set index to start of compound waveform for current cluster */
			index1=ii*wtotlen;
			/* find the channel (kk) containing the peak at the spike-detection sample (wspkpre) */
			kk= xf_wavepeak2_f((wmean+index1),wnchans,wspklen,wspkpre,setsign,resultf,message);
			if(kk==-1){ fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
			kk-= (long)(ncorchan/2); // look back half the number of channels specified for correlation
			if(kk<0) kk=0; // wavecorchan cannot be less than zero
			if(kk>(wnchans-ncorchan)) kk= (wnchans-ncorchan); // wavecorchan can't be too close to end of multichannel waveform
			wavecorchan[ii]= kk;
			//TEST: printf("cluster[%ld]=%d:\t%ld %ld %ld\n",ii,wid[ii],indexmax,(long)(indexmax/wspklen),kk);
		}
	}
	//TEST WAVECOR: for(ii=0;ii<wnclust;ii++) { index1=ii*wtotlen; printf("cluster[%ld]=%d... index1=%ld... wavecorchan:%ld\n",ii,wid[ii],index1,wavecorchan[ii]);} exit(0);

	/********************************************************************************
	SET HISTOGRAM PARAMETERS FOR ATUTO- AND CROSS-CORRELOGRAMS
	- "aa" should be larger than the 15ms outer-zone used for refratory calculations
	********************************************************************************/
	histhalf=0.050 ; // half-histogram width, seconds
	histbintot=(long)(histhalf*2*1000); // ensures 1 bin per millisecond
	histhigh=(long)(histhalf*wsfreq);
	histlow= 0-histhigh;
	if(histbintot>HISTMAXSIZE) {fprintf(stderr,"\n--- Error [%s]: %g-second histogram requires %ld bins - exceeds predefined HISTMAXSIZE (%d)\n\n",thisprog,histhalf,histbintot,HISTMAXSIZE); exit(1);}

	/********************************************************************************
	CALCULATE STARTING AUTOCORRELATION AND REFRACTORINESS FOR EACH CLUSTER
	- refract = ration of +-2ms to +-15ms
	NOTE: currently, when clusters are combined autocorellations are not recalculated
	********************************************************************************/
	for(ii=0;ii<wnclust;ii++) {
		cluid1=wid[ii];
		/* get the intervals between samples */
		if(clun[cluid1]<1) continue;
		intervals= xf_wint1_ls(clubt1,club1,spiketot,cluid1,cluid1,(histhigh-histlow),&intervaltot);
		if(intervaltot==-1) { fprintf(stderr,"\n--- Error [%s/xf_wint1_ls]: memory allocation error\n\n",thisprog);exit(1);}
		if(intervaltot>0) {
			/* build a histogram */
			kk= xf_hist1_l(intervals,intervaltot,histx,histy,histbintot,histlow,histhigh,1);
			/* convert histx values to seconds */
			for(jj=0;jj<histbintot;jj++) histx[jj]/=wsfreq;
			/* calculate autocor (refractoriness) score: (+-2ms spikes)/(+-15ms spikes) */
			aa= xf_histratio1_d(histx,histy,histbintot,setacorz1,setacorz2,resulthist1);
			autocor[cluid1]= (float)aa;
		}
		else autocor[cluid1]= -1.0;
		//TEST:	for(jj=0;jj<histbintot;jj++) printf("%d	%g	%g\n",cluid1,histx[jj],histy[jj]); exit(0);
	}
	//TEST: for(kk=0;kk<wnclust;kk++) printf("%d	%g\n",wid[kk],autocor[wid[kk]]);exit(0);

	/********************************************************************************
	DETERMINE WHICH CLUSTERS TO COMBINE - CAN SET MULTIPLE PASSES, RECORDING DECISIONS
	- maximum number of spikes in the refractory period
	- minimum number of spikes in the central 15ms zone
	- minimum common refractoriness (15ms v 2ms d2 score), minimum t-value, maximum p
	- minimum waveform similarity
	********************************************************************************/
	if(sethistout==1) {
		fphist=fopen(outfilehist,"w");
		if(fphist==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfilehist);exit(1);}
		fprintf(fphist,"pair\ttime\tcount\n");
	}

	/* FOR EACH COMBINATION-PASS  */
	for(pass=1;pass<=setpass;pass++) {

		/* keep track of how many pairs were combined this pass */
		combine_count=0;
		/* print report header */
		if(setverb==0||setverb==2) printf("c1\tc2\ta1\ta2\tnxcor\tr_left\tt_left\tp_left\tr_right\tt_right\tp_right\twavecor\tcombine\n");

		/* FOR EACH REFERENCE CLUSTER - do not include zero or last cluster */
		for(ii=0;ii<(wnclust-1);ii++) {
			cluid1=wid[ii];
			if(cluid1==0 || wclun[ii]<1) continue;

			/* BUILD TEMPORARY ARRAYS RESTRICTED TO HISTOGRAM HALF-WIDTH AROUND THE REFERENCE SPIKES (cluid1) */
			spiketot2= xf_rewindow2_ls(clubt1,club1,spiketot,cluid1,(histhalf*wsfreq),clubt2,club2);

			/* FOR EACH COMPARISON CLUSTER - start at next cluster from reference, go right up to clumax */
			for(jj=(ii+1);jj<wnclust;jj++) {
				cluid2=wid[jj];
				if(cluid2==0 || wclun[jj]<1) continue;

				/* SET FLAG TO DETERMINE IF THIS PAIR SHOULD BE COMBINED */
				combine_flag= 0; // this needs to get switched to "2" for clusters to be combined

				/* CALCULATE CROSS-CORELLOGRAM STATISTICS */
				// get intervals between spikes: use temporary club(t) data (from windows around the reference cluster spikes)
				intervals= xf_wint1_ls(clubt2,club2,spiketot2,cluid1,cluid2,(histhigh-histlow),&intervaltot);
				if(intervaltot==-1) { fprintf(stderr,"\n--- Error [%s/xf_wint1_ls]: memory allocation error\n\n",thisprog);exit(1);}
				if(intervaltot>0) {
					// generate histogram - total spikes in 100ms window with 1ms resolution
					kk= xf_hist1_l(intervals,intervaltot,histx,histy,histbintot,histlow,histhigh,1);
					// convert histx values to seconds - only necesary because the histrefract function wants seconds
					for(kk=0;kk<histbintot;kk++) histx[kk]/=wsfreq;
					// get the refractoriness score (1=perfect refractoriness) & t-stats
					aa= xf_histratio2_d(histy,histbintot,setxcorcentre,setxcorside,resulthist1,message);
					if(aa<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
					//TEST:	for(kk=0;kk<histbintot;kk++) printf("%g	%g\n",histx[kk],histy[kk]); exit(0);
				}
				else {
					/* if correllogram is empty...  */
					resulthist1[0]=resulthist1[1]=resulthist1[2]=resulthist1[3]=0.00; //set spike counts
					resulthist1[11]=resulthist1[21]= -1.0; // set ratio
					resulthist1[12]=0.0; // t-statistic, min possible value
					resulthist1[13]=1.0; // probability, maximum value
				}

				/* CHECK: IF ALL HISTOGRAM CRITERIA ARE MET, INCREMENT COMBINE_FLAG TO ZERO */
				// check that cells meet autorefract criterion (typically 0.08)
				if(autocor[cluid1]<=setauto && autocor[cluid2]<=setauto) {
					// check that minimum number of spikes are in central histogram (typically 50)
					if( (resulthist1[0]+resulthist1[1]+resulthist1[2]+resulthist1[3]) >= setminspikes ) {
						// check that the refractory score for BOTH sides < setmaxratio (typically 0.08)
						if( resulthist1[11]<=setmaxratio && resulthist1[21]<=setmaxratio) {
							//check that t-statistics and p-values for at least one side meet minimum criterion
							if((resulthist1[12]>=setmint&&resulthist1[13]<=setmaxp) || (resulthist1[22]>=setmint&&resulthist1[23]<=setmaxp)) {
								combine_flag+=1;
				}}}}

				/* WAVEFORM DEPTH-PROFILE CORRELATION SCORE - pull values from channel peaks only */
				/* get indices to first sample to be correlated in each waveform - cluid1 channels serve as the template */
				index1= (ii*wtotlen + wavecorchan[ii]*wspklen) ;
				index2= (jj*wtotlen + wavecorchan[ii]*wspklen) ;
				/* get the correlation */
				bb= xf_wavecor2_f((wmean+index1),(wmean+index2),wnchans,wspklen,wspkpre,message);
				if(!isfinite(bb)) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
				r2= bb*bb; // r-squared
				if(bb<0) r2= (0.0-r2); // apply a sign to r2 to signify anti-correlation
				if(r2>setwavecor) combine_flag+=2;

				/* PRINT REPORT - if report only, skip the combine step */
				if( setverb==0 || setverb==2) {
					aa=resulthist1[0]+resulthist1[1]+resulthist1[2]+resulthist1[3]; // total spikes inXCG
					printf("%d\t%d\t%.3f\t%.3f\t%g\t",cluid1,cluid2,autocor[cluid1],autocor[cluid2],aa); // histratio2: ID's & total spikes in centre+sides
					printf("%.3f\t%.3f\t%.3f\t",resulthist1[11],resulthist1[12],resulthist1[13]); // histratio2d: left-side refract,t,p
					printf("%.3f\t%.3f\t%.3f\t",resulthist1[21],resulthist1[22],resulthist1[23]); // histratio2d: right-side refract,t,p
					printf("%.3f\t%d\n",r2,combine_flag); // wavecor results and combine flag
					if(setverb==0) continue;
				}

				/* COMBINE THE CLUSTERS ONLY IF SETVERB !=0 && FLAG >= 1 */
				/* - we are merging cluid1 into cluid2 */
				if(combine_flag==3) {
					/* keep track of how many clusters are combined in this pass */
					combine_count++;
					/* output histogram */
					if(sethistout==1) {
						for(kk=0;kk<histbintot;kk++) {
							fprintf(fphist,"%03d:%03d\t%g\t%g\n",cluid1,cluid2,histx[kk],histy[kk]);
						}
					}

					// combine clusters - copy lower cluster to higher one
					for(kk=0;kk<spiketot;kk++) { if(club1[kk]==cluid1) club1[kk]=cluid2; }
					clun[cluid2]+= clun[cluid1];
					clun[cluid1]= 0;

					// recalculate autocorellogram for cluid2
					free(intervals);
					intervals= xf_wint1_ls(clubt1,club1,spiketot,cluid2,cluid2,(histhigh-histlow),&intervaltot);
					if(intervaltot==-1) { fprintf(stderr,"\n--- Error [%s/xf_wint1_ls]: memory allocation error\n\n",thisprog);exit(1);}
					if(intervaltot>0) {
						/* build a histogram */
						kk= xf_hist1_l(intervals,intervaltot,histx,histy,histbintot,histlow,histhigh,1);
						/* convert histx values to seconds */
						for(kk=0;kk<histbintot;kk++) histx[kk]/=wsfreq;
						/* calculate autocor (refractoriness) score: (+-2ms spikes)/(+-15ms spikes) */
						aa= xf_histratio1_d(histx,histy,histbintot,setacorz1,setacorz2,resulthist1);
						autocor[cluid2]= (float)aa;
					}
					else autocor[cluid2]= -1.0;

					// recalculate waveform mean
					aa= (double)wclun[ii];
					bb= (double)wclun[jj];
					cc= (double)(wclun[ii]+wclun[jj]);
					index1= ii*wtotlen;
					index2= jj*wtotlen;
					for(kk=0;kk<wtotlen;kk++) {
						wmean[index2+kk]= (float)((wmean[index1+kk]*aa + wmean[index2+kk]*bb) / cc );
						wmean[index1+kk]= NAN;
					}
					wclun[jj]+= wclun[ii];
					wclun[ii]= 0;
					// break out of the cluid2 loop because there can be no more comparisons against cluid1
					break;

				} // end of match criteria to combine
			} // end of for cluid2= loop (cluid1-2)
		} // end of for cluid1= loop (cluid1-1)

		if(setverb==0||setverb==2) printf("\n");
		if(combine_count<=0) break; // don't do extra passes if nothing was combined!

	} // end of pass loop

	if(sethistout==1) fclose(fphist);


	/********************************************************************************
	OUTPUT MODIFIED CLU FILE
	*********************************************************************************/
	if(setverb!=0) {
		z= xf_writebin2_v(outfileclub,(void *)club1,spiketot,sizeof(short),message);
		if(z<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	/************************************************************
	OUTPUT MODIFIED .WFM FILE
	- in effect only the spike-counts (wclun) are altered
	- xf_writewave1_f only outputs a cluster if wclun[id]>0
	************************************************************/
	/* set the values for the header etc  */
	resultl[0]=wnclust; // number of rows in the array - unchanged, even if clusters were killed
	resultl[1]=wnchans;
	resultl[2]=wspklen;
	resultl[3]=wspkpre;
	resultl[4]=wprobe;
	/* write the new waveform file */
	z= xf_writewave1_f(outfilewfm,wid,wclun,wmean,wlistc,resultl,0,wsfreq,message);
	if(z!=0) { fprintf(stderr,"\n\t--- %s\n\n",message); exit(1); }



	free(clubt1);
	free(club1);
	free(clun);
	free(clubt2);
	free(club2);
	free(histx);
	free(histy);
	free(intervals);
	free(autocor);
	free(wmean);
	free(wmeantemp);
	free(wavecorchan);
	free(wid);
	free(wlistc);
	free(wclun);

	exit(0);
}
