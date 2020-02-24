#define thisprog "xe-ldas5-cluhist1"
#define TITLE_STRING thisprog" v 1: 28.July.2017 [JRH]"
#define MAXLINELEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
<TAGS>dt.spikes</TAGS>

v 1: 21.August.2018 [JRH]
	- remove limit on maximum histogram size

v 1: 28.July.2017 [JRH]
	- add burst-firing measure using xf_histburst1_d
		- histogram downsampling, smoothing, and spline interpolation
	- only calculate complex stats if output is a summary table

v 1: 17.July.2017 [JRH]
	- add calculation of positive ACG mean

v 1: 2.May.2017 [JRH]
	- bugfix: ensure histy[] array is filled with zeros if a cluster has no spike-intervals

v 1: 30.March.2017 [JRH]
	- add option to produce cross-corellograms for all potential cluster-pairs
	- add option to skip cluster zero

v 1: 1.March.2017 [JRH]
	- add cross-correlation capability (-xcor)

v 1: 16.February.2017 [JRH]
	- drop output of summary for non-existent clusters
	- use new long-int functions for calculating intervals and histograms
	- remove -bins option: program now automatically makes histogram using 1ms bins
	- these changes makethe program align with methods used in xe-ldas5-clucombine1

v 1: 16.October.2016 [JRH]
	- allow different types (-t) of histogram: counts, 0-1 range, or probability

v 1: 2.May.2016 [JRH]
	- new version for LDAS5


v 1: 3.April.2010 [JRH] (based on xe-clustats1)
*/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_readclub1(char *infile1, char *infile2, long **clubt, short **club, char *message);
double *xf_wint1(double *time, int *group, long n, int g1, int g2, float winsize, long *result_l);
long xf_hist1d(double *data,long n,double *x,double *y,int bintot,double min,double max,int format);
long *xf_wint1_ls(long *time, short *group, long nn, short g1, short g2, long winsize, long *nintervals);
long xf_hist1_l(long *data, long nn, double *histx, double *histy, long bintot, long min, long max, int format);
double xf_histratio1_d(double *tsec, double *val, long bintot, double z1, double z2, double *result);

int xf_histburst1_d(double *histx1, double *histy1, long nbins1, double *result_d, char *message);
double xf_bin1a_d(double *data, size_t *setn, size_t *setz, size_t setbins, char *message);
double xf_round1_d(double input, double setbase, int setdown);
int xf_smoothgauss1_d(double *original,size_t arraysize,int smooth);
void xf_spline1_d(double xin[],double yin[],long nin,double xout[],double yout[],long nout);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],line[256],*pline,*pcol,message[MAXLINELEN];
	int w,x,y,z,col,result_i[32];
	long ii,jj,kk,mm,nn,result_l[32];
	float a,b,c,result_f[32];
	double aa,bb,cc,result_d[32];
	FILE *fpin,*fpout;

	/* program-specific variables */
	short *club=NULL,clumax=0,cluid1,cluid2;
	long *clubt=NULL,*clu_n=NULL,*listclu=NULL;
	long *iword=NULL,nlistclu,spiketot=0,intervaltot=0,histbintot,histhigh,histlow,histn1,histn2,histn3,startindex;
	long *intervals=NULL;
	float refract;
	double *histx=NULL,*histy=NULL,histmean,burst1,burst2;
	double width,halfwidth,zone1,zone2;

	/* arguments */
	char infile1[256],infile2[256],*setlist=NULL;
	int setcor=1,settype=1,setout=1,setskipz=0,setverb=0;
	float setfreq=19531.25;
	long setwidth=50,sethistbintot=0,setz1=15,setz2=2;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate spike-histogram-related statistics for club(t) records\n");
		fprintf(stderr,"- auto- or cross-corerelograms\n");
		fprintf(stderr,"- binsize fixed at 1ms\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [infile1] [infile2] [options]\n",thisprog);
		fprintf(stderr,"	[infile1]: .clubt timestamp file\n");
		fprintf(stderr,"	[infile2]: .club cluster-id file\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []:\n");
		fprintf(stderr,"	-list: CSV list of clusters to analyze [all]\n");
		fprintf(stderr,"	-cor: auto-(1) or cross-(2) corellogram? [%d]\n",setcor);
		fprintf(stderr,"	-t: type, 1(counts) 2(range 0-1) or 3(probability) [%d]\n",settype);
		fprintf(stderr,"	-sf: sample freq. to convert samples to seconds [%.3f]\n",setfreq);
		fprintf(stderr,"	-width: histogram half-width (milliseconds) [%ld]\n",setwidth);
		fprintf(stderr,"	-bins: override default number of bins (0= 1ms/bin) [%ld]\n",sethistbintot);
		fprintf(stderr,"		NOTE: use only for histogram output (-out 0)\n");
		fprintf(stderr,"	-z1: zone1 half-width (milliseconds) [%ld]\n",setz1);
		fprintf(stderr,"	-z2: zone2 half-width (milliseconds) [%ld]\n",setz2);
		fprintf(stderr,"	-skipz: skip cluster zero (0=NO, 1=YES) [%d]\n",setskipz);
		fprintf(stderr,"	-out: output (0=histograms, 1=stats) [%d]\n",setout);
		fprintf(stderr,"		NOTE: for stats, use a window >= +-50ms\n");
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES) [%d]\n",setverb);
		fprintf(stderr,"OUTPUT (-out 0, histograms):\n");
		fprintf(stderr,"	cluster	time	count\n");
		fprintf(stderr,"OUTPUT (-out 1, statistics):\n");
		fprintf(stderr,"	cluster   n   histn1   histn2   refract   mean   burst\n");
		fprintf(stderr,"	- cluster: cluster id\n");
		fprintf(stderr,"	- n: total spikes fired by this cell\n");
		fprintf(stderr,"	- histn1: spikes in histogram\n");
		fprintf(stderr,"	- histn2: spikes in zone1\n");
		fprintf(stderr,"	- histn3: spikes in zone2\n");
		fprintf(stderr,"	- refract: refractory score (spikes in zone2/zone1)\n");
		fprintf(stderr,"	- mean: mean (ms) for positive portion of histogram\n");
		fprintf(stderr,"	- burst: burst-firing tendency of the cell (0-1)\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.clu.3 data.res.3 -sf 24000\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile1,"%s",argv[1]);
	sprintf(infile2,"%s",argv[2]);
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-list")==0) setlist=argv[++ii];
			else if(strcmp(argv[ii],"-cor")==0) setcor=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-t")==0) settype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-sf")==0) setfreq= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-width")==0) setwidth= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-bins")==0) sethistbintot= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-z1")==0) setz1= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-z2")==0) setz2= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-skipz")==0) setskipz= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0) setout= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setcor<1||setcor>2) {fprintf(stderr,"\a\n\t--- Error[%s]: invalid -cor (%d) - must be 1 or 2\n\n",thisprog,setcor);exit(1);}
	if(settype<1||settype>3) {fprintf(stderr,"\a\n\t--- Error[%s]: invalid histogram type (-t %d) - must be 1,2 or 3\n\n",thisprog,settype);exit(1);}
	if(setout!=0 && setout!=1) { fprintf(stderr,"\n--- Error [%s]: invalid output (-out %d) must be 0 or 1\n\n",thisprog,setout);exit(1);}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid verbocity (-verb %d) must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setskipz!=0 && setskipz!=1) { fprintf(stderr,"\n--- Error [%s]: invalid skipz (-skipz %d) must be 0 or 1\n\n",thisprog,setskipz);exit(1);}
	if(setz1>setwidth) { fprintf(stderr,"\n--- Error [%s]: zone1 (%ld) must be less than total histogram width (%ld)\n\n",thisprog,setz1,setwidth);exit(1);}
	if(setz2>setwidth) { fprintf(stderr,"\n--- Error [%s]: zone2 (%ld) must be less than total histogram width (%ld)\n\n",thisprog,setz2,setwidth);exit(1);}
	if(setz2>setz1)    { fprintf(stderr,"\n--- Error [%s]: zone2 (%ld) must be less than zone1 (%ld)\n\n",thisprog,setz2,setz1);exit(1);}
	if(sethistbintot!=0 && setout !=0) { fprintf(stderr,"\n--- Error [%s]: manual bin-counts (-bin %ld) should not be used with stats output (-out %d)\n\n",thisprog,sethistbintot,setout);exit(1);}

	/************************************************************
	DEFINE TOTAL WIDTH AND ZONE-SIZES IN SECONDS
	- note that width is full-width while zones are half-zones
	***********************************************************/
	halfwidth= (double)setwidth/1000.0;
	width= setwidth*2.0;
	zone1= (double)(setz1)/1000.0;
	zone2= (double)(setz2)/1000.0;

	/* ALLOCATE MEMORY FOR HISTOGRAMS */
	histhigh= (long)(halfwidth*setfreq); // sample-max for right side of histogram
	histlow= 0-histhigh;                 // sample-min for left  side of histogram
	if(sethistbintot==0) histbintot= (long)width; // ensures 1 bin per millisecond
	else histbintot=sethistbintot;
	//TEST:	fprintf(stderr,"histlow=%ld	histhigh=%ld	histbintot=%ld\n",histlow,histhigh,histbintot);exit(0);
	histx= malloc(histbintot * sizeof histx);
	histy= malloc(histbintot * sizeof histy);
	if(histx==NULL||histy==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	/************************************************************
	READ THE CLUSTER TIMESTAMPS AND IDs
	***********************************************************/
 	spiketot= xf_readclub1(infile1,infile2,&clubt,&club,message);
 	if(spiketot<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
	/* find highest cluster id */
	clumax=-1; for(ii=0;ii<spiketot;ii++) if(club[ii]>clumax) clumax=club[ii]; clumax++;
	/* calculate spike-count in each cluster */
	clu_n= calloc(clumax,sizeof clu_n);
	if(clu_n==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	for(ii=0;ii<spiketot;ii++) clu_n[club[ii]]++;
	//TEST: for(ii=0;ii<clumax;ii++) if(clu_n[ii]>0) fprintf(stderr,"%ld\t%d\n",ii,clu_n[ii]);

	/************************************************************
	BUILD THE CLUSTER LIST
	***********************************************************/
	if(setlist==NULL) setlist="all\0";
	if(strcmp(setlist,"all")==0) {
		startindex=0;
		listclu= realloc(listclu,clumax * sizeof *listclu);
		if(listclu==NULL) {{fprintf(stderr,"\n--- Error[%s]: memory allocation error\n\n",thisprog);exit(1);}}
		nlistclu=0;
		for(ii=setskipz;ii<clumax;ii++) if(clu_n[ii]>0) listclu[nlistclu++]=ii;
	}
	else  {
		startindex=1;
		/* build array of clusters to compare */
		iword= xf_lineparse2(setlist,",",&nlistclu);
		listclu= realloc(listclu,nlistclu * sizeof *listclu);
		if(listclu==NULL) {{fprintf(stderr,"\n--- Error[%s]: memory allocation error\n\n",thisprog);exit(1);}}
		for(ii=0;ii<nlistclu;ii++) {
			listclu[ii]= atoi(setlist+iword[ii]);
			if(listclu[ii]==0 && setskipz==1) {fprintf(stderr,"\n--- Error[%s]: cannot skip cluster zero (-skipz 1) and also include it in the cluster list\n\n",thisprog);exit(1);}
		}
		/* make sure clusters match input */
		for(ii=0;ii<nlistclu;ii++) if(clu_n[listclu[ii]]<=0) {fprintf(stderr,"\n--- Error[%s]: listed cluster (%ld) missing from %s\n\n",thisprog,listclu[ii],infile2);exit(1);}
		//TEST: /for(ii=0;ii<nlistclu;ii++) printf("listclu[%ld]=%d\n",ii,listclu[ii]); exit(0);
	}
	//TEST:	for(ii=0;ii<nlistclu;ii++) printf("listclu[%ld]=%d\n",ii,listclu[ii]);printf("startindex=%d\n",startindex);exit(0);


	/************************************************************
	AUTOCORRELATION
	************************************************************/
	if(setcor==1) {
		if(setout==0) printf("cluster	time	count\n");
		if(setout==1) printf("cluster	n	%ldms	%ldms	%ldms	refract	mean	burst\n",setwidth,setz1,setz2);

		for(ii=0;ii<nlistclu;ii++) {

			/* define current cluster id (links to clun_n[cluid1] */
			cluid1=listclu[ii];
			/* initialize statistics */
			refract= -1.0;
			histmean=-1.0;
			burst1= -1.0;
			burst2= -1.0;
			histn1=histn2=histn3= 0;
			/* calculate intervals for autocorellogram */
			intervals= xf_wint1_ls(clubt,club,spiketot,cluid1,cluid1,(histhigh-histlow),&intervaltot);
			if(intervaltot==-1) { fprintf(stderr,"\n--- Error [%s/xf_wint1_ls]: memory allocation error\n\n",thisprog);exit(1);}
			if(intervaltot>0) {
				/* build a histogram  - histx & histy will be double-precision floats */
				mm= xf_hist1_l(intervals,intervaltot,histx,histy,histbintot,histlow,histhigh,1);
				/* convert histx values to seconds */
				for(kk=0;kk<histbintot;kk++) histx[kk]/=setfreq;
			}
			/* if there were no intervals, fill the histogram with empty values */
			else { for(kk=0;kk<histbintot;kk++) histy[kk]= 0.0; }

			/* output the histogram */
			if(setout==0) {	for(kk=0;kk<histbintot;kk++) printf("%d\t%g\t%g\n",cluid1,histx[kk],histy[kk]);	}

			/* otherwise, calculate additional stats and output a table */
			else if(setout==1) {
				if(intervaltot>0) {
					/* calculate refractoriness score: typically (+-2ms spikes)/(+-15ms spikes) */
					refract= xf_histratio1_d(histx,histy,histbintot,zone1,zone2,result_d);
					histn1=(long)(result_d[4]/2.0); // half-total spikes in the autocorellogram
					histn2=(long)result_d[0];  // spikes in -zone1 of autocorellogram
					histn3=(long)result_d[2];  // spikes in -zone2 of autocorellogram
					/* calculate the mean of the autocorrelogram (ms) */
					aa=bb=0.0;
					for(kk=0;kk<histbintot;kk++) if(histx[kk]>=0) { aa+= histx[kk]*histy[kk]; bb+= histy[kk]; }
					histmean= aa/bb;
					/* calculate burstiness based on downsampled,smoothed,spline-interpolated autocorrelogram */
					z= xf_histburst1_d(histx,histy,histbintot,result_d,message);
					burst1= result_d[2];
					if(z==-3) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
					else if(z<0 && setverb==1) fprintf(stderr,"\t*** %s/%s\n",thisprog,message);
				}
				printf("%d	%ld	%ld	%ld	%ld	%.3f	%.3f	%.3f\n",
				cluid1,clu_n[cluid1],histn1,histn2,histn3,refract,histmean,burst1);
			} // END OF IF SETOUT==1 CONDITION
		} // END OF FOR CLUSTER LOOP
	} // END OF IF AUTOCORRELATION CONDITION

	/************************************************************
	CROSS-CORRELATION
	***********************************************************/
	if(setcor==2) {

		if(setout==0) printf("c1	c2	time	count\n");
		if(setout==1) printf("c1	c2	%ldms	%ldms	%ldms	refract\n",setwidth,setz1,setz2);

		for(ii=0;ii<nlistclu;ii++) {
		if(startindex>0) startindex=ii+1; // determines whether cross-correlations start at the next cluster or from the beginning
		for(jj=startindex;jj<nlistclu;jj++) {
			/* determine cluster id's (we already knows these cannot be empty clusters) */
			cluid1=listclu[ii];
			cluid2=listclu[jj];
			/* initialize statistics */
			refract= -1.0;
			histn1=histn2=histn3= 0;
			/* calculate intervals for autocorellogram */
			intervals= xf_wint1_ls(clubt,club,spiketot,cluid1,cluid2,(histhigh-histlow),&intervaltot);
			if(intervaltot==-1) { fprintf(stderr,"\n--- Error [%s/xf_wint1_ls]: memory allocation error\n\n",thisprog);exit(1);}
			if(intervaltot>0) {
				/* build a histogram */
				mm= xf_hist1_l(intervals,intervaltot,histx,histy,histbintot,histlow,histhigh,1);
				/* convert histx values to seconds */
				for(kk=0;kk<histbintot;kk++) histx[kk]/=setfreq;
			}
			/* if there were no intervals, fill the histogram with empty values */
			else { for(kk=0;kk<histbintot;kk++) histy[kk]= 0.0; }

			/* output the histogram */
			if(setout==0) {	for(kk=0;kk<histbintot;kk++) printf("%d\t%d\t%g\t%g\n",cluid1,cluid2,histx[kk],histy[kk]);}
			/* calculate and output the stats */
			else if(setout==1) {
				if(intervaltot>0) {
					/* calculate refractoriness score: typically (+-2ms spikes)/(+-15ms spikes) */
					refract= xf_histratio1_d(histx,histy,histbintot,zone1,zone2,result_d);
					histn1=(long)(result_d[4]/2.0); // half-total spikes in the autocorellogram
					histn2=(long)result_d[0];  // spikes in -zone1 of autocorellogram
					histn3=(long)result_d[2];  // spikes in -zone2 of autocorellogram
				}
				printf("%d	%d	%ld	%ld	%ld	%.3f\n",cluid1,cluid2,histn1,histn2,histn3,refract);
			}
		}}
	}



	/* FREE MEMORY AND EXIT */
	free(clubt);
	free(club);
	free(clu_n),
	free(listclu);
	free(intervals);
	free(histx);
	free(histy);
	exit(0);
}
