#define thisprog "xe-ldas5-clurate1"
#define TITLE_STRING thisprog" v 1: 24.November.2017 [JRH]"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>  /* this and next header allow testing file exists using stat() function */
#include <errno.h>

/************************************************************************
<TAGS>dt.spikes</TAGS>

v 1: 24.November.2017 [JRH]
	- bugfix: duration is now defined properly
	- realign function updated to also adjust cluster-id's
v 1: 13.November.2017 [JRH]
*************************************************************************/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
long xf_readclub1(char *infile1, char *infile2, long **clubt, short **club, char *message);
long xf_matchclub1_ls(char *list1, long *clubt, short *club, long nn, char *message);
long xf_blockrealign1_ls(long *samplenum, short *class, long nn, long *bstart, long *bstop, long nblocks, char *message);
long *xf_density1_l(long *etime,long nn,long min,long max,double winsize,long *nwin,char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile1[256],infile2[256],message[256];
	int v,w,x,y,z;
	long int ii,jj,kk,mm,nn;
	double aa,bb,cc,dd,result_d[64];
	FILE *fpin,*fpout1,*fpout2;
	struct stat sts;

	/* program-specific variables */
	short *club=NULL,*clukeep;
	int setscreen=0;
	long *clubt=NULL,*start1=NULL,*stop1=NULL,*index1=NULL,*clucount=NULL;
	long *density1=NULL,clumaxp1=-1,tmin,tmax,duration1,nwin;
	off_t nstart,nclulist;
	double duration2;

	/* arguments */
	char *setscreenfile=NULL,*setscreenlist=NULL,*setclulist=NULL;
	int setverb=0;
	float setwinsize=1.0,setfreq=19531.25;

	nstart=nclulist=0;

	/************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	************************************************************/
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate firing-rate timecourse for clusters\n");
		fprintf(stderr,"- \n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [clubt] [club] [options]\n",thisprog);
		fprintf(stderr,"	[clubt]: binary file containing cluster-times (long int)\n");
		fprintf(stderr,"	[club]: binary file containing cluster-IDs (short int)\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-clu: screen using CSV list of cluster IDs [unset]\n",setclulist);
		fprintf(stderr,"	-scrf: screen-file (binary ssp) defining bounds for infile1 [unset]\n");
		fprintf(stderr,"	-scrl: screen-list (CSV) defining bounds for infile1 [unset]\n");
		fprintf(stderr,"	-winsize: density-window-size (seconds) [%.3f]:\n",setwinsize);
		fprintf(stderr,"	-sf: sample freq to convert winsize to samples [%.3f]\n",setfreq);
		fprintf(stderr,"	-verb: set verbocity of output (0=low, 1=high) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.clubt data.club -s 1 -sl 100,200,1500,1600\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	************************************************************/
	sprintf(infile1,"%s",argv[1]);
	sprintf(infile2,"%s",argv[2]);
	if(strcmp(infile1,"stdin")!=0 && stat(infile1,&sts)==-1 && errno == ENOENT) {
		fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile1);
		exit(1);
	}
	if(strcmp(infile2,"stdin")!=0 && stat(infile2,&sts)==-1 && errno == ENOENT) {
		fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile2);
		exit(1);
	}
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-sf")==0) setfreq= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-clu")==0)  setclulist=argv[++ii];
			else if(strcmp(argv[ii],"-scrf")==0) setscreenfile=argv[++ii];
			else if(strcmp(argv[ii],"-scrl")==0) setscreenlist=argv[++ii];
			else if(strcmp(argv[ii],"-winsize")==0)  setwinsize=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb<0 || setverb>1) {fprintf(stderr,"\n--- Error[%s]: invalid -verb (%d) : must be 0-1\n\n",thisprog,setverb);exit(1);}
	if(setscreenfile!=NULL && setscreenlist!=NULL) {fprintf(stderr,"\n--- Error[%s]: cannot define both a screen-list and a screen-file\n\n",thisprog);exit(1);}


	/************************************************************
	READ THE CLUSTER TIMESTAMPS AND IDs
	***********************************************************/
	if(setverb==1) {
		fprintf(stderr,"reading input files...\n");
		fprintf(stderr,"	clubt file: %s\n",infile1);
		fprintf(stderr,"	club  file: %s\n",infile2);
	}
 	nn= xf_readclub1(infile1,infile2,&clubt,&club,message);
 	if(nn<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}

	/************************************************************
	READ THE TIMESTAMP-SCREENING LIST OR FILE
	/************************************************************/
	if(setscreenlist!=NULL) {
		setscreen=1;
		index1= xf_lineparse2(setscreenlist,",",&nstart);
		if((nstart%2)!=0) {fprintf(stderr,"\n--- Error[%s]: screen-list does not contain pairs of numbers: %s\n\n",thisprog,setscreenlist);exit(1);}
		nstart/=2;
		start1= realloc(start1,nstart*sizeof(*start1));
		stop1= realloc(stop1,nstart*sizeof(*stop1));
		if(start1==NULL || stop1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		for(ii=0;ii<nstart;ii++) {
			start1[ii]= atol(setscreenlist+index1[2*ii]);
			stop1[ii]=  atol(setscreenlist+index1[2*ii+1]);
	}}
	else if(setscreenfile!=NULL) {
		setscreen=1;
		nstart = xf_readssp1(setscreenfile,&start1,&stop1,message);
		if(nstart==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	}

	/************************************************************
	INITIALIZE TRIAL-DURATION PARAMETERS USING ORIGINAL CLU RECORDS
	***********************************************************/
	tmin= clubt[0];
	tmax= clubt[nn-1];
	duration1= tmax-tmin;
	duration2= (double)duration1/(double)setfreq;

	/************************************************************
	KEEP ONLY THE CLUSTERS OF INTEREST
	***********************************************************/
	if(setclulist!=NULL) {
		nn= xf_matchclub1_ls(setclulist,clubt,club,nn,message);
		if(nn<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
	}

	/************************************************************
	SCREEN THE REMAINING TIMESTAMPS USING THE SCREENING LIST
	- timestamps will be adjusted to start from 0
	- all clusters still remaining after cluster-screening are treated as one
	***********************************************************/
	if(setscreen==1) {
		if(setverb==1)	fprintf(stderr,"sample-screen list:\n");
		/* calculate the recording duration */
		for(jj=duration1=0;jj<nstart;jj++) {
			kk= stop1[jj]-start1[jj];
			duration1+= kk;
			if(setverb==1) fprintf(stderr,"	start:%ld	stop:%ld	duration: %ld	cumulative: %ld\n",start1[jj],stop1[jj],kk,duration1);
		}
		tmin= 0;
		tmax= duration1;
		duration2= (double)duration1/(double)setfreq;
		/* realign the cluster data to the blocks */
		mm= xf_blockrealign1_ls(clubt,club,nn,start1,stop1,nstart,message);
		if(mm<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
		nn= mm;
	}
	//TEST:	for(ii=0;ii<clumaxp1;ii++) fprintf(stderr,"%ld	%d	%ld\n",ii,clucount[ii],clukeep[ii]); exit(0);
	//TEST:	for(ii=0;ii<mm;ii++) printf("%ld	%d\n",clubt[ii],club[ii]);
	//TEST:	fprintf(stderr,"nn=%ld	mm=%ld	tmin=%ld	tmax=%ld	duration1=%ld	duration2=%g\n",nn,mm,tmin,tmax,duration1,duration2);


	/************************************************************
	CALCULATE DENSITY FUNCTION (events-per-window)
	************************************************************/
	if(setverb==1) fprintf(stderr,"calculating density in %gs windows\n",setwinsize);
	density1= xf_density1_l(clubt,nn,tmin,tmax,(setwinsize*setfreq),&nwin,message);
	if(density1==NULL) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
	if(setverb==1) fprintf(stderr,"number of windows: %ld\n",nwin);

	/************************************************************
	OUTPUT THE DENSITY
	************************************************************/
	if(setverb==1) fprintf(stderr,"beginning output...\n");
	for(ii=0;ii<nwin;ii++) {
		aa= (double)density1[ii]/setwinsize;
		printf("%.3f\n",aa);
	}



 	if(club!=NULL) free(club);
 	if(clubt!=NULL) free(clubt);
 	if(index1!=NULL) free(index1);
 	if(start1!=NULL) free(start1);
	if(stop1!=NULL) free(stop1);
	if(density1!=NULL) free(density1);
 	exit(0);

}
