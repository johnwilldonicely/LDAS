#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-ldas5-cofiring1"
#define TITLE_STRING thisprog" v 1: 27.November.2017 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>dt.spikes</TAGS>

v 1: 27.November.2017 [JRH] :
	- based on xe-cofiring1 & (now retired) xe-cofiring2
*/



/* external functions start */
long xf_readclub1(char *fileclubt, char *fileclub, long **clubt, short **club, char *message);
long xf_matchclub1_ls(char *setclulist, long *clubt, short *club, long nclub, char *message);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
long xf_sspsplit1_l(long *start1, long *stop1, long nssp, long winsize, long **start2, long **stop2, char *message);
long xf_correlate_l(long *x, long *y, long nn, long setinv, double *result, char *message);
int xf_prob_F(float F, int dfa, int dfb);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char outfile[256],line[MAXLINELEN],*pline,*pcol,message[256];
	long ii,jj,kk,ll,mm,nn;
	int v,w,x,y,z,col,colmatch;
	int sizeofint=sizeof(int),sizeofdouble=sizeof(double);
	float a,b,c,d,resultf[64];
	double aa,bb,cc,dd,resultd[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	short *club=NULL,clumaxp1,c1,c2;
	long *clubt=NULL,*winstart=NULL,*winstop=NULL,*winstart2=NULL,*winstop2=NULL;
	long *clucount,*wincount=NULL,*pwc1=NULL,*pwc2=NULL;
	long nc1,nc2,nwin,win,ncor,cofir;
	double ratio,r,F,prob;
	/* arguments */
	char *fileclubt,*fileclub,*filessp;
	char *setclulist=NULL;
	int setminspikes=0,setverb=0;
	long setwin=-1;
	double setwinsize=0.1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<4) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate co-firing of cell pairs\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [clubt] [club] [ssp] [options]\n",thisprog);
		fprintf(stderr,"	[clubt]: binary file containing cluster-times (long int)\n");
		fprintf(stderr,"	[club]: binary file containing cluster-IDs (short int)\n");
		fprintf(stderr,"	[ssp]: binary start-stop-pair file defining time-windows\n");
		fprintf(stderr,"VALID ARGUMENTS (defaults in []) ...\n");
		fprintf(stderr,"	-clu: CSV list of clusters to use [unset]\n",setclulist);
		fprintf(stderr,"	-win: break SSPs into windows of this size (samples, -1=NO) [%ld]:\n",setwin);
		fprintf(stderr,"	-ms: minimum spikes from both cells in a pair across all windows [%d]\n",setminspikes);
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt cells_place.txt times.txt\n",thisprog);
		fprintf(stderr,"	cat data.txt | %s stdin list.txt times.txt\n",thisprog);
		fprintf(stderr,"OUTPUT: c1 c2 cofir totfir ratio  r F p\n");
		fprintf(stderr,"	c1,c2: cell id's for each pair\n");
		fprintf(stderr,"	cofir: sum of spikes from c1+c2 ocurring inside time windows\n");
		fprintf(stderr,"	totfir: sum of spikes from c1+c2, regardless of timing\n");
		fprintf(stderr,"	ratio: cofiring ratio score [0-1]\n");
		fprintf(stderr,"		NOTE: this may be an unreliable scrore - correlates poorly with r, below\n");
		fprintf(stderr,"		NOTE: cofir correlates well, but maybe c1*c2 would be better than c1+c2\n");
		fprintf(stderr,"	n: number win time windows\n");
		fprintf(stderr,"	r,F,p: Pearson's-r results for c1 vs. c2 clucounts in each window\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	/********************************************************************************/
	fileclubt= argv[1];
	fileclub= argv[2];
	filessp= argv[3];
	for(ii=4;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-clu")==0)  setclulist= argv[++ii];
			else if(strcmp(argv[ii],"-win")==0)  setwin= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-ms")==0)   setminspikes= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}


	/************************************************************
	STORE THE CLUSTER TIMESTAMPS AND IDs
	***********************************************************/
	if(setverb==1) fprintf(stderr,"Reading club(t) files...\n");
 	nn= xf_readclub1(fileclubt,fileclub,&clubt,&club,message);
 	if(nn<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
	/* keep only the clusters of interest */
	if(setclulist!=NULL) {
		nn= xf_matchclub1_ls(setclulist,clubt,club,nn,message);
		if(nn<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
	}
	if(setverb==1) fprintf(stderr,"	- total records: %ld\n",nn);


	/************************************************************
	STORE THE WINDOWS
	***********************************************************/
	if(setverb==1) fprintf(stderr,"Reading start-stop pairs...\n");
	nwin = xf_readssp1(filessp,&winstart,&winstop,message);
	if(nwin==-1) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	if(nwin==0) {fprintf(stderr,"\n--- Error[%s]: no SSPs in %s\n\n",thisprog,filessp);exit(1);}
	if(setverb==1) fprintf(stderr,"	- total %ld SSPs\n",nwin);
	/* break into discrete smaller windows */
	if(setwin!=-1.0) {
		kk= xf_sspsplit1_l(winstart,winstop,nwin,setwin,&winstart2,&winstop2,message);
		if(kk<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
		if(setverb==1) fprintf(stderr,"	- total %ld SSPs after windowing\n",kk);
		free(winstart);
		free(winstop);
		winstart= winstart2;
		winstop= winstop2;
		if(kk==0) {fprintf(stderr,"\n--- Error[%s]: no SSPs after %ld-sample windowing: consider smaller window-size\n\n",thisprog,setwin);exit(1);}
		nwin= kk;
	}

	/************************************************************
	ALLOCATE SPIKE-COUNT MEMORY
	***********************************************************/
	if(setverb==1) fprintf(stderr,"Allocating memory for spiking-in-windows...\n");
	/* get the highest cluster id */
	clumaxp1=-1; for(ii=0;ii<nn;ii++) if(club[ii]>clumaxp1) clumaxp1=club[ii]; clumaxp1++;
	/* counts in each cluster - use to skip enmpty clusters */
	clucount= calloc(clumaxp1,sizeof(*clucount));
	if(clucount==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	/* matrix of counts, cluster x window */
	wincount= calloc(clumaxp1*nwin,sizeof(*wincount));
	if(wincount==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/************************************************************
	ADD UP SPIKE COUNTS FOR EACH CLUSTER AND IN EACH WINDOW
	***********************************************************/
	if(setverb==1) fprintf(stderr,"Calculating spike-counts in windows...\n");
	if(wincount==NULL||clucount==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	win=0;
	for(ii=0;ii<nn;ii++) {
		x= club[ii];
		kk= clubt[ii];
		/* incriment the total spikes for that cell */
		clucount[x]++;
		/* advance to first eligible window which stops after the spike */
		while(winstop[win]<=kk && win<nwin) win++;
		/* have we run out of windows? */
		if(win==nwin) break;
		/* does the spike also occur >= the start-time of this window? */
		/* ...if yes, add to the count for that cluster/window */
		if(kk>=winstart[win]) wincount[x*nwin+win]++;
	}

	/************************************************************
	AT THIS POINT WE HAVE, FOR EACH CELL, A LIST OF SPIKE-COUNTS IN EACH TIME WINDOW
	- note that there may be a lot of empty rows in the wincount matrix
	- this is because not all clusters between 0 and clumaxp1 are valis
	NOW CORRELATE THE SPIKE COUNTS IN EACH WINDOW FOR EACH CELL-PAIR
	***********************************************************/
	if(setverb==1) fprintf(stderr,"Calculating cofiring...\n");
	printf("c1	c2	cofir	totfir	ratio	n	r	F	p\n");
	for(c1=0;c1<clumaxp1;c1++) {
		if(clucount[c1]<1) continue;
		pwc1= wincount+(c1*nwin); // pointer to data for cell-1 of this pair
		for(c2=c1+1;c2<clumaxp1;c2++) {
			if(clucount[c2]<1) continue;
			pwc2= wincount+(c2*nwin); // pointer to data for cell-2 of this pair

			/* correlate spike-counts in the matching window-timeseries */
			ncor= xf_correlate_l(pwc1,pwc2,nwin,-1,resultd,message);
			if(ncor<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
			r= resultd[1];
			F= resultd[4];
			prob= resultd[7];

			/* count the spikes in each cluster as found within the windows */
			nc1=nc2=0;
			for(ii=0;ii<nwin;ii++) { nc1+=pwc1[ii];	nc2+=pwc2[ii]; }

			/* get cofiring, total firing, and ratio */
			cofir= nc1+nc2; // total co-firing
			kk= clucount[c1]+clucount[c2]; // total firing
			ratio= (double)cofir/(double)kk; // ratio-score

			/* output the results */
			if(nc1>=setminspikes && nc2>=setminspikes) {
				printf("%d\t%d\t%ld\t%ld	%.3f\t%ld\t%.4f\t%.4f\t%.4f\n",c1,c2,cofir,kk,ratio,ncor,r,F,prob);
			}
			else {
				printf("%d	%d	-	-	-	-	-	-	-\n",c1,c2);
			}
		}
	}


	if(clubt!=NULL) free(clubt);
	if(club!=NULL) free(club);
	if(winstart!=NULL) free(winstart);
	if(winstop!=NULL) free(winstop);
	if(clucount!=NULL) free(clucount);
	if(wincount!=NULL) free(wincount);
	exit(0);
}
