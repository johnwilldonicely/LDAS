#define thisprog "xe-ldas5-clukiller1"
#define TITLE_STRING thisprog" v 1: 10.November.2016 [JRH]"
#define MAXLINELEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* <TAGS>ldas spikes</TAGS> */

/* external functions start */
long xf_readclub1(char *infile1, char *infile2, long **clubt, short **club, char *message); 
long *xf_wint1_ls(long *time, short *group, long nn, short g1, short g2, long winsize, long *nintervals);
long xf_hist1_l(long *data, long nn, double *histx, double *histy, long bintot, long min, long max, int format); 
float xf_histrefract1_d(double *time,double *val,int n,double *result);
int xf_norm2_d(double *data,long N,int normtype);
int xf_stats2_d(double *data, long n, int varcalc, double *result);
/* external functions end */

int main (int argc, char *argv[]) {


	/* general variables */
	char infile[256],outfile[256],line[256],*pline,*pcol,message[MAXLINELEN];
	int w,x,y,z,col,result_i[32];
	long ii,jj,kk,nn;
	int sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	long result_l[32];
	float a,b,c,result_f[32]; 
	double aa,bb,cc,dd,result_d[32];
	FILE *fpin,*fpout;

	/* program-specific variables */ 
	short *club=NULL, cluster,clumax=0;
	long *clubt=NULL,*clun,*intervals=NULL;
	long histbintot=100;
	long intervaltot=0,histlow,histhigh,zonetot;
	double *histx=NULL,*histy=NULL,*histz=NULL;
	double histmean,histsem,refract,noise;

	/* arguments */
	char infile1[256],infile2[256];
 	int setminspikes=75;
	float setrefractmax=0.08;
	float setnoisemax=2.0;
	float setfreq=19531.25;
	int setverb=1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Reset non-refractory or noisy clusters to category zero (noise)\n");
		fprintf(stderr,"Analysis is based on the 100ms autocorellation histogram\n");
		fprintf(stderr,"No reset is if the central +-15ms of the histogram is sparse\n");
		fprintf(stderr,"- refract= ratio of spikes in the central +-2ms vs +-15ms portions\n");
		fprintf(stderr,"	- this captures clusters violating refractoriness requirements\n");
		fprintf(stderr,"- noise= variance in the normalized (0-1), differenced histogram\n");
		fprintf(stderr,"	- this captures highly temporally-stereotypic artefacts\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [clubt] [club] [options]\n",thisprog);
		fprintf(stderr,"	[clubt]: binary file containing club-times (long int)\n");
		fprintf(stderr,"	[club]: binary file containing club-IDs (short int)\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-sf (sample freq): convert samples to seconds [%.2f]\n",setfreq);
		fprintf(stderr,"	-ks: skip clusters with < this minimum +-15ms spike-count [%d]\n",setminspikes);
		fprintf(stderr,"	-kr: maximum refractory score [%.4f]\n",setrefractmax);
		fprintf(stderr,"	-kn: maximum noise score [%g]\n",setnoisemax);
		fprintf(stderr,"	-v: verbocity (0=report only, 1=cull, 2=cull+report) [%d]\n",setverb);
		fprintf(stderr,"- examples:\n");
		fprintf(stderr,"	%s data.club.3 data.clubt.3 -sf 20000\n",thisprog);
		fprintf(stderr,"- output:\n");
		fprintf(stderr,"	modified .club file\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile1,"%s",argv[1]);
	sprintf(infile2,"%s",argv[2]);
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if(ii>=argc) break;
			else if(strcmp(argv[ii],"-sf")==0) setfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-kn")==0) setnoisemax=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-kr")==0) setrefractmax=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-ks")==0) setminspikes=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-v")==0)  setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"Error[%s]: invalid command line argument \"%s\"\n",thisprog,argv[ii]); exit(1);}
	}}

	

	/************************************************************
	READ THE CLUSTER TIMESTAMPS AND IDs
	************************************************************/
 	nn= xf_readclub1(infile1,infile2,&clubt,&club,message); 
 	if(nn<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
	//TEST: for(ii=0;ii<nn;ii++) printf("%ld	%d\n",clubt[ii],club[ii]); exit(0);
	
	/************************************************************
	DETERMINE THE HIGHEST CLUSTER ID 
	************************************************************/
	clumax=-1; for(ii=0;ii<nn;ii++) if(club[ii]>clumax) clumax=club[ii]; 
	//TEST:	printf("club_max=%d\n",clumax);exit(0);

	/************************************************************
	BUILD A LIST OF CLUSTER COUNTS
	************************************************************/
	clun=(long *)calloc(clumax+1,sizeof(long));
	for(ii=0;ii<nn;ii++) clun[club[ii]]++;
	//TEST: for(ii=0;ii<=clumax;ii++) if(clun[ii]>0) printf("clun[%ld]=%ld\n",ii,clun[ii]); exit(0);
	
	/************************************************************
	ALLOCATE MEMORY FOR HISTOGRAMS
	************************************************************/
	histx=(double *)malloc(histbintot*sizeofdouble); 
	histy=(double *)malloc(histbintot*sizeofdouble); 
	if(histx==NULL||histy==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog);exit(1);}

	/************************************************************
	DETERMINE THE +-50MS INDEX VALUES ACCORDING TO SAMPLE FREQ
	************************************************************/
	histhigh=(long)(0.05*setfreq);
	histlow= 0-histhigh;
	//TEST: printf("histlow=%ld\n",histlow); printf("histhigh=%ld\n",histhigh); exit(0);
	
	/* IDENTIFY CLUSTERS WITH ABSOLUTELY NO REFRACTORINESS - DUMP TO CLUSTER ZERO */
	if(setverb==0) printf("clust	n_15ms	refract	noise\n");
	if(setverb==2) fprintf(stderr,"clust	n_15ms	refract	noise\n");


	/************************************************************
	CREATE AND ANALYZE THE AUTOCORRELOGRAM FOR EACH CLUSTER
	************************************************************/
	for(cluster=0;cluster<=clumax;cluster++) {

		if(clun[cluster]<1) continue;

		/* CALCULATE INTERVALS FOR AUTOCORELLOGRAM */
		intervals= xf_wint1_ls(clubt,club,nn,cluster,cluster,(histhigh-histlow),&intervaltot);
		if(intervaltot==-1) { fprintf(stderr,"\n--- Error [%s/xf_wint1_ls]: memory allocation error\n\n",thisprog);exit(1);}
		else if(intervaltot==0) continue; // no matches to current cluster within the specified window
		//TEST: for(ii=0;ii<intervaltot;ii++) printf("%ld\n",intervals[ii]);

		/* GENERATE HISTOGRAM - TOTAL SPIKES IN 100MS WINDOW WITH 1MS RESOLUTION */
		// output is counts, but expressed as double because it could be probability or 0-1 depending on format
		kk= xf_hist1_l(intervals,intervaltot,histx,histy,histbintot,histlow,histhigh,1); 
		// convert histx values to seconds - only necesary because the histrefract function wants seconds
		for(jj=0;jj<histbintot;jj++) histx[jj]/=setfreq;	
		//TEST: for(ii=0;ii<histbintot;ii++) printf("%g\t%g\n",histx[ii],histy[ii]);exit(0);		

		/* GET REFRACTORY VALUE FOR HISTOGRAM */
		xf_histrefract1_d(histx,histy,histbintot,result_d); 
		refract= result_d[0];
		zonetot= (long)(result_d[2]+result_d[3]); // total spike count in +-15ms zone

		/* CALCULATE THE NOISE SCORE (VARIANCE IN THE NORMALIZED, DIFFERENCED HISTOGRAM) */
		// normalize the histogram 0-1
		xf_norm2_d(histy,histbintot,0);
		// get the bin-to-bin difference in values
		for(ii=0;ii<(histbintot-1);ii++) histy[ii]= histy[ii]-histy[ii+1];
		// get the variance 
		xf_stats2_d(histy,(histbintot-1),1,result_d); 
		noise=result_d[1];
		
		/* IF SETVERB==0, OUTPUT THE HISTORAM SUMMARY STATS */
		if(setverb==0) {printf("%d\t%ld\t%.4f\t%.4f\n",cluster,zonetot,refract,noise);continue;}

		/* OTHERWISE, MOVE CLUSTERS WITH SUFFICIENTLY-POPULATED HISTOGRAMS TO ZERO IF THEY FAIL THE REFRACT OR NOISE CRITERA  */
		if(zonetot>setminspikes && (refract>setrefractmax || noise>setnoisemax)) { 
			if(setverb==2) fprintf(stderr,"%d\t%ld\t%.4f\t%.4f\n",cluster,zonetot,refract,noise);
			for(ii=0;ii<nn;ii++) { if(club[ii]==cluster) club[ii]=0; } 
			continue;	
	}}				

	/* OUTPUT MODIFIED CLU FILE */
	if(setverb!=0) {
		for(ii=0;ii<nn;ii++) printf("%d\n",club[ii]);
	}

	free(clubt);
	free(club);
	free(clun);
	free(intervals);
	free(histx);
	free(histy);
	exit(0);
	}

