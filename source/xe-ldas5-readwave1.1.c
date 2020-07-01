#define thisprog "xe-ldas5-readwave1"
#define TITLE_STRING thisprog" v 1: 6.July.2017 [JRH]"

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>

#define MAX_LINELEN 1000
#define MAX_CH 128

/* <TAGS> file dt.spikes </TAGS> */

/*
CHANGES THIS VERSION:
v 1: 6.July.2017 [JRH]
	- use new function getheader1 to read the header
v 1: 27.June.2017 [JRH]
*/

//float *xf_readwave1_f(char *infile, long *resultl, double *resultd, char *message);

/* external functions start */
char *xf_getheader1(char *infile, char *terminus, char *message);
double xf_readwave1_f(char *infile, short **id, long **count, float **data, short **chanlist, long *resultl, char *message);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_wavefilt1_f(float *wave, int nchan, int spklen, float setrate, float setlow, float sethigh, char *message);
long xf_interp3_f(float *data, long ndata);
int xf_filter_bworth1_f(float *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	// generic variables
	char line[MAX_LINELEN],message[MAX_LINELEN],*pline;
	char outfile[256],path_prog[256],path_data[256],temp_str[256];
	int w,x,y,z;
	long ii,jj,kk,nn,resultl[32],*iword=NULL,nwords;
	float a,b,c,d,resultf[32];
	double aa,bb,cc,resultd[32];
	FILE *fpin,*fpout;

	// program specific variables
	char *header=NULL;
	short *id=NULL,*chanlist=NULL;
	int *outclu=NULL,found=0;
	long *count=NULL,*setclu=NULL,clu,clumax,clutot;
	float *wave=NULL,*wavep0,*wavep1,*wavep2;
	float width, correlation=0,pmin,pmax,pdiff,pratio;
	long chan,nchan,chanstart,chanstop,spklen,spkpre,wavelen;
	double srate,sratems,uv_per_unit,peak,zmax;

	// command-line options
	char *infile=NULL,*setclulist=NULL;
	int setout=1,setverb=0;
	long setchan=-1;
	float setfiltlow=0.0,setfilthigh=0.0;

	/*********************************************************************************************************/
	/* PRINT INSTRUCTIONS IF ONLY ONE ARGUMENT */
	/*********************************************************************************************************/
	if(argc<2) {
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Read waveform file (.wfm)\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [.wfm file] [options]\n",thisprog);
		fprintf(stderr,"	[.wfm file]: file containing mean waveforms for each cluster\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-low: filter low-cut (0=none) [%g]\n",setfiltlow);
		fprintf(stderr,"	-high: filter high-cut (0=none) [%g]\n",setfilthigh);
		fprintf(stderr,"	-clust: restrict output to a CSV list of clusters) [unset]\n");
		fprintf(stderr,"		- if unset (null), all clusters are output\n");
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES) [%d]\n",setverb);
		fprintf(stderr,"	-out: output format [%d]\n",setout);
		fprintf(stderr,"		0= header only\n");
		fprintf(stderr,"		1= .wfm format (header + waveforms & metadata)\n");
		fprintf(stderr,"		2= [cluster] [channel] [time:ms] [value]\n");
		fprintf(stderr,"...if -out is set to 2...\n");
		fprintf(stderr,"	-chan: restrict output to channel-ID (-1 = no restriction) [%ld]\n",setchan);
		fprintf(stderr,"		NOTE: derived from CHANNEL_LIST, not sequence\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/*********************************************************************************************************/
	/* READ ARGUMENTS */
	/*********************************************************************************************************/
	infile=argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if(ii>=argc) break;
			else if(strcmp(argv[ii],"-low")==0)   setfiltlow=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-high")==0)  setfilthigh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-clust")==0) setclulist=(argv[++ii]);
			else if(strcmp(argv[ii],"-chan")==0)  setchan=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)   setout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)  setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"Error[%s]: invalid command line argument \"%s\"\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setout<0 || setout>2) { fprintf(stderr,"\n--- Error [%s]: invalid -out [%d] must be 0-2\n\n",thisprog,setout);exit(1);}
	if(setchan<-1) { fprintf(stderr,"\n--- Error [%s]: invalid -chan [%ld] must be -1 or >=0\n\n",thisprog,setchan);exit(1);}
	if(setchan!=-1 && setout!=2) { fprintf(stderr,"\n--- Error [%s]: channel-selection (-chan %ld) only valid for output option 2 (-out 2)\n\n",thisprog,setchan);exit(1);}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}

	/*********************************************************************************************************/
	/* READ THE .WFM FILE HEADER */
	/*********************************************************************************************************/
	if(setout==0 || setout==1) {
		if(setverb==1) fprintf(stderr,"... reading %s ...\n",infile);

		header= xf_getheader1(infile,"WAVES_START",message);
		if(header!=NULL) printf("%s\n",header);
		else {fprintf(stderr,"\nError[%s]: %s\n\n",thisprog,message); exit(1); }

		/* exit here if output is header-only */
		if(setout==0) exit(0);
	}


	/*********************************************************************************************************/
	/* READ THE .WFM FILE TO GET NCHAN, NPROBES, AND THE CHANNEL LIST FOR EACH PROBE  */
	/*********************************************************************************************************/
	srate = xf_readwave1_f(infile,&id,&count,&wave,&chanlist,resultl,message);
	if(srate<0) {fprintf(stderr,"\nError[%s]: %s\n\n",thisprog,message); exit(1); }
	clutot=resultl[0];
	nchan=resultl[1];
	spklen=resultl[2];
	spkpre=resultl[3];
	wavelen=resultl[4];
	clumax=resultl[5];
	/* get the sample-rate, in millisenconds (samples per ms) */
	sratems= srate/1000.00;
	//TEST: for(ii=0;ii<clutot;ii++) fprintf(stderr,"%ld	%ld\n",id[ii],count[ii]); exit(0);
	//TEST: for(ii=0;ii<nchan;ii++) fprintf(stderr,"chan[%ld]=%ld\n",ii,chanlist[ii]); exit(0);

	/*********************************************************************************************************/
	/* FOR EACH CLUSTER, APPLY WITHIN-CHANNEL FILTERING */
	/*********************************************************************************************************/
	if(setfiltlow!=0.0 || setfilthigh!=0.0) {
		if(setverb==1) fprintf(stderr,"... filtering ...\n");
		for(ii=0;ii<clutot;ii++) {
			z= xf_wavefilt1_f((wave+(ii*wavelen)),nchan,spklen,srate,setfiltlow,setfilthigh,message);
			if(z==-1 && setverb==1) { fprintf(stderr,"\t--- Warning[%s]: cluster %d, %s\n",thisprog,id[ii],message);}
	}}

	/*********************************************************************************************************/
	/* BUILD THE CLUSTER LIST */
	/*********************************************************************************************************/
	/* allocate memory for output cluster-list */
	outclu= calloc(clutot,sizeof(*outclu));
	if(outclu==NULL) {fprintf(stderr,"\n--- Error[%s]: memory allocation error\n\n",thisprog);exit(1);}
	/* option1: all-clusters */
	if(setclulist==NULL) { for(ii=0;ii<clutot;ii++) outclu[ii]=1; }
	/* option1: CSV list of clusters - needs parseing */
	else {
		iword= xf_lineparse2(setclulist,",",&nwords);
		if(nwords>clutot) {fprintf(stderr,"\n--- Error[%s]: specified number of output clusters (%ld) cannot exceed actual number of clusters (%ld)\n\n",thisprog,nwords,clutot);exit(1);}
		for(ii=0;ii<nwords;ii++) {
			kk= atol(setclulist+iword[ii]); // temporary cluster-ID to output
			for(jj=0;jj<clutot;jj++) if(id[jj]==kk) outclu[jj]=1; // match cluster to the index in the waveform id list
		}
	}
	//TEST: for(ii=0;ii<clutot;ii++) printf("id[%ld]=%ld, outclu=%d\n",ii,id[ii],outclu[ii]); exit(0);

	/*********************************************************************************************************/
	/* OUTPUT THE WAVEFORMS */
	/*********************************************************************************************************/
	if(setverb==1) fprintf(stderr,"... producing output ...\n");
	if(setout==1) {
		for(ii=0;ii<clutot;ii++) {
			/* skip omitted clusters */
			if(outclu[ii]==0) continue;
			/* create a pointer to the current cluster's waveform */
			wavep0= (wave+(ii*wavelen));
			printf("%d\t%ld\t",id[ii],count[ii]);
			printf("%.3f",wavep0[0]);
			for(jj=1;jj<wavelen;jj++) printf(" %.3f",wavep0[jj]);
			printf("\n");
		}
	}
	if(setout==2) {
		/* define the section (channel) of the multi_channel waveform to output  */
		if(setchan==-1) { chanstart=0; chanstop=wavelen; }
		else {
			kk=-1;
			for(jj=0;jj<nchan;jj++) if(setchan==chanlist[jj]) kk=jj;
			if(kk==-1) {fprintf(stderr,"\n--- Error[%s]: specified channel (%ld) was not found in the input file\n\n",thisprog,setchan);exit(1);}
			chanstart= kk*spklen;
			chanstop= (kk+1)*spklen;
		}
		printf("cluster	channel	ms	uV\n");
		for(ii=0;ii<clutot;ii++) {
			/* skip omitted clusters */
			if(outclu[ii]==0) continue;
			/* create a pointer to the current cluster's waveform */
			wavep0= (wave+(ii*wavelen));
			/* cluster id */
			clu= id[ii];
			/* output the waveform */
			for(jj=chanstart;jj<chanstop;jj++) {
				kk= jj/spklen; // channel number
				aa= 1000.0 * ((1+jj-kk*spklen-spkpre) / srate); // time in ms
				printf("%ld	%d	%.3f	%g\n",
					clu,chanlist[kk],aa,wavep0[jj]);
	}}}

	/********************************************************************************
	FREE MEMORY AND EXIT
	********************************************************************************/
	if(header!=NULL) free(header);
	if(id!=NULL) free(id);
	if(count!=NULL) free(count);
	if(wave!=NULL) free(wave);
	if(chanlist!=NULL) free(chanlist);
	if(iword!=NULL) free(iword);
	if(outclu!=NULL) free(outclu);

	exit(0);

}
