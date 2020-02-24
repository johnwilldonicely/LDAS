
#define thisprog "xe-ldas5-clusort1"
#define TITLE_STRING thisprog" v 1: 1.July.2017 [JRH]"

#define MAXLINELEN 1000
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/*
<TAGS>dt.spikes</TAGS>

v 1: 1.July.2017 [JRH]
	- add filtering before sorting
	- use function to detect peak channel
v 1: 31.March.2017 [JRH]
	- original version
*/

/* external functions start */
long xf_readclub1(char *setfileclubt, char *setfileclub, long **clubt, short **club, char *message);
double xf_readwave1_f(char *infile, short **id, long **count, float **data, short **wchans, long *resultl, char *message);
long xf_interp3_f(float *data, long ndata);
int xf_filter_bworth1_f(float *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
int xf_wavefilt1_f(float *wave, int nchan, int spklen, float setrate, float setlow, float sethigh, char *message);
long xf_wavepeak2_f(float *wave, long nchan, long spklen, long setzero, int sign, float *resultf, char *message);
void xf_qsortindex1_l(long *data, long *index,long n);
int xf_writebin2_v(char *outfileclub, void *data0, size_t nn, size_t datasize, char *message);
int xf_writewave1_f(char *outfile, short *wid, long *wn, float *wmean, short *wchans, long *resultl, long makez, double srate, char *message);
/* external functions end */

int main (int argc, char *argv[]) {


	/* general variables */
	char line[256],*pline,*pcol,message[MAXLINELEN];
	int w,x,y,z;
	long ii,jj,kk,mm,nn,resultl[16];
	float a,b,c,resultf[8];
	double aa,bb,cc;
	FILE *fpin,*fpout;

	/* program-specific variables */
	char outfileclub[64],outfilewfm[64];
	short *club=NULL,clumax=0;
	long *clubt=NULL,*clun=NULL,spiketot,nclust;
	long *cluid=NULL,*maxchan=NULL,*rank=NULL;

	short *wid=NULL,*wchans=NULL,rankstart;
	long *wclun=NULL, *wavecorchan=NULL, wnchans, wspklen, wspkpre, wnclust, wtotlen, wclumax, wprobe,index,goodsamps;
	float *wmean=NULL, *wtemp=NULL, *wavep0,wavemin,wavemax,wavecor;
	double wsfreq;
	long *listindex=NULL;

	/* arguments */
	char *setfileclubt,*setfileclub,*setfilewfm=NULL;
	int setsign=-1,setverb=1;
	float setfiltlow=300, setfilthigh=5000;

	sprintf(outfileclub,"temp_%s.club",thisprog);
	sprintf(outfilewfm,"temp_%s.wfm",thisprog);

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<4) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Sort cluster ID's by channel with max-amplitude peak in .wfm file\n");
		fprintf(stderr,"- assumes channels are in depth-order\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [clubt] [club] [wfm] [options]\n",thisprog);
		fprintf(stderr,"	[clubt]: .clubt timestamp file\n");
		fprintf(stderr,"	[club]: .club cluster-id file\n");
		fprintf(stderr,"	[wfm]: .wfm waveform file\n");
		fprintf(stderr,"VALID OPTIONS, defaults in []:\n");
		fprintf(stderr,"	-sign: wave amplitude sign (1=positive -1=negative) [%d]\n",setsign);
		fprintf(stderr,"	-low: filter low-cut (0=none) [%g]\n",setfiltlow);
		fprintf(stderr,"	-high: filter high-cut (0=none) [%g]\n",setfilthigh);
		fprintf(stderr,"	-verb: verbode output (0=NO 1=YES) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.clubt data.club 4,13,27\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	modified .club file (%s)\n",outfileclub);
		fprintf(stderr,"	modified ..wfm file file (%s)\n",outfilewfm);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	************************************************************/
	setfileclubt=argv[1];
	setfileclub=argv[2];
	setfilewfm=argv[3];
	for(ii=4;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-sign")==0) setsign=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-low")==0)   setfiltlow=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-high")==0)  setfilthigh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setsign!=-1&&setsign!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -sign (%d), must be -1 or 1\n\n",thisprog,setsign); exit(1);}
	if(setverb!=0&&setverb!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -verb (%d), must be 0 or 1\n\n",thisprog,setverb); exit(1);}


	/************************************************************
	STORE THE CLUSTER TIMESTAMPS AND IDS
	************************************************************/
	/* read the clubt and club files */
 	spiketot= xf_readclub1(setfileclubt,setfileclub,&clubt,&club,message);
 	if(spiketot<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
	/* find highest cluster id */
	clumax=-1; for(ii=0;ii<spiketot;ii++) if(club[ii]>clumax) clumax=club[ii];

	/* allocate memory for additional variables */
	/* technically only rank needs to accomodate 0-clumax, but let's keep it simple... */
	clun= calloc((clumax+1),sizeof *clun);
	cluid= calloc((clumax+1),sizeof *cluid);
	maxchan= calloc((clumax+1),sizeof *maxchan);
	rank= calloc((clumax+1),sizeof *rank);
	if(clun==NULL||cluid==NULL||maxchan==NULL||rank==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n",thisprog);exit(1);}

	/* calculate spike-count in each cluster */
	for(ii=0;ii<spiketot;ii++) clun[club[ii]]++;
	//TEST: for(ii=0;ii<=clumax;ii++) printf("clun[%ld]=%ld\n",ii,clun[ii]); exit(0);

	/* determine the total number of populated clusters */
	/* also populate arrays to hold initial values of max-channel and depth-rank */
	rankstart=1;
	nclust=0;
	for(ii=0;ii<=clumax;ii++) {
		if(clun[ii]>0) {
			if(ii==0) rankstart=0;
			cluid[nclust]= ii;
			maxchan[nclust]= -1;
			nclust++;
	}}

	/********************************************************************************
	READ THE .WFM FILE
	********************************************************************************/
	wsfreq = xf_readwave1_f(setfilewfm,&wid,&wclun,&wmean,&wchans,resultl,message);
	if(wsfreq<0) {fprintf(stderr,"\nError[%s]: %s\n\n",thisprog,message); exit(1); }
	wnclust= resultl[0]; // total clusters, not the same as max_cluster_id (resultl[5])
	wnchans= resultl[1]; // number of channels per combined-waveform
	wspklen= resultl[2]; // number of samples per channel
	wspkpre=resultl[3];  // the number of samples preceding the peak
	wtotlen=resultl[4];  // total length of combined-waveform
	wclumax=resultl[5];  // largest cluster-number
	wprobe=resultl[6];   // probe number
	/* check maximum-cluster-id's match in club and wfm records */
	if(wclumax!=clumax) {fprintf(stderr,"\n--- Error[%s]: max cluster in waveform file (%ld) does not match the club file (%ld)\n\n",thisprog,wclumax,clumax);exit(1);}
	/* check that the total number of populated clusters also matches */
	if(wnclust!=nclust) {fprintf(stderr,"\n--- Error[%s]: no. of clusters in waveform file (%ld) does not match the club file (%ld)\n\n",thisprog,wnclust,nclust);exit(1);}
	/* compare spike counts for each cluster  */
	/* also check if a cluster-zero is defined */
	for(ii=0;ii<wnclust;ii++) {
		z=wid[ii];
		if(z!=cluid[ii]) {fprintf(stderr,"\n--- Error[%s]: id mismatch between .club file [%d] and .wfm file (%d) file does not match the club file (%ld)\n\n",thisprog,cluid[ii],z);exit(1);}
		if(wclun[ii]!=clun[z]) {fprintf(stderr,"\n--- Error[%s]: cluster[%ld] count in waveform file (%ld) file does not match the club file (%ld)\n\n",thisprog,ii,wclun[ii],clun[z]);exit(1);}
	}

	/************************************************************
	CREATE DEPTH-RANK FOR EACH CLUSTER
	************************************************************/
	/* build a temporary array for filtering and scanning */
	kk= wnclust*wtotlen;
	wtemp= malloc(kk*sizeof(*wtemp));
	if(wtemp==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n",thisprog);exit(1);}
	for(ii=0;ii<kk;ii++) wtemp[ii]= wmean[ii];

	/* for each cluster... */
	for(ii=0;ii<nclust;ii++) {
		if(cluid[ii]==0) {maxchan[ii]=-2; continue;} // ensures cluster zero will always come first
		/* create pointer to temporary array */
		wavep0= wtemp+(ii*wtotlen);
		/* apply Butterworth filter (interpolate,demean,filter,rezero) */
		z= xf_wavefilt1_f(wavep0,wnchans,wspklen,wsfreq,setfiltlow,setfilthigh,message);
		if(z==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		/* find the channel with the largest value at the peak-time (wspkpre) */
		kk= xf_wavepeak2_f(wavep0,wnchans,wspklen,wspkpre,setsign,resultf,message);
		if(kk==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		maxchan[ii]= kk;
	}

	/************************************************************
	SORT CLUSTER IDs BY DEPTH-ORDER
	************************************************************/
	xf_qsortindex1_l(maxchan,cluid,nclust);
	if(setverb==1) {
		printf("newid	oldid	maxchan\n");
		for(ii=0;ii<nclust;ii++) printf("%ld	%ld	%ld\n",(ii+rankstart),cluid[ii],maxchan[ii]);
	}

	/************************************************************
	DETERMINE UNIQUE RANKS, INDEXED BY ORIGINAL CLUSTER-ID
	- adjusted by rankstart
	************************************************************/
	/* apply the rank, indexed by cluid */
	for(ii=0;ii<nclust;ii++) rank[cluid[ii]]= ii+rankstart;

	/************************************************************
	MODIFY AND EXPORT NEW CLUB FILE
	************************************************************/
	for(ii=0;ii<spiketot;ii++) club[ii]= rank[club[ii]];
	z= xf_writebin2_v(outfileclub,(void *)club,spiketot,(sizeof *club),message);
	if(z!=0) { fprintf(stderr,"\b\n\t*** %s\n\n",message); exit(1); }

	/************************************************************
	MODIFY WAVEFORMS AND CREATE NEW .WFM FILE
	- in effect only the spike-counts (wclun) are altered
	- xf_writewave1_f only outputs a cluster if wclun[id]>0
	************************************************************/
	/* adjust waveform IDs wid[] and cluid[] should match */
	for(ii=0;ii<wnclust;ii++) wid[ii]= rank[wid[ii]];

	/* set the values for the header etc  */
	resultl[0]=wnclust; // number of rows in the array - unchanged, even if clusters were killed
	resultl[1]=wnchans;
	resultl[2]=wspklen;
	resultl[3]=wspkpre;
	resultl[4]=wprobe;
	/* write the new waveform file */
	z= xf_writewave1_f(outfilewfm,wid,wclun,wmean,wchans,resultl,0,wsfreq,message);
	if(z!=0) { fprintf(stderr,"\b\n\t*** %s\n\n",message); exit(1); }


	/************************************************************
	CLEANUP AND EXIT
	************************************************************/
	free(club);
	free(clubt);
	free(clun);
	free(cluid);
	free(wmean);
	free(wtemp);
	free(wid);
	free(wchans);
	free(wclun);
	free(maxchan);
	free(rank);
	exit(0);
	}
