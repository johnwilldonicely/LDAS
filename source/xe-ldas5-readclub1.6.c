#define thisprog "xe-ldas5-readclub1"
#define TITLE_STRING thisprog" v 6: 16.March.2019 [JRH]"
#define MAXLINELEN 1000
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>  /* this and next header allow testing file exists using stat() function */
#include <errno.h>

/************************************************************************
<TAGS>dt.spikes</TAGS>

v 6: 16.March.2019 [JRH]
	- update to use new function xf_screen_club - because the older xs-screen_ls should not update the timestamps
v 6: 5.November.2018 [JRH]
	- bugfix: break if no clusters found matching setclulist
v 6: 24.November.2017 [JRH]
	- bugfix: duration is now defined properly
v 6: 14.November.2017 [JRH]
		- bugfix: clukeep array was under-allocated if cluster-screening lists were used - now corrected
v 6: 10.July.April.2017 [JRH]
	- bugfix: summary output now reflects application of screening list
	- move scrreening step to just after cluster-selection
v 6: 19.April.2017 [JRH]
	- add simple cluster spike-count output
v 6: 2.May.2016 [JRH]
	- variable cleanup
v 6: 1.April.2016 [JRH]
	- remove requirement to set -scr : assume filtering is inclusive
	- bugfix: ensure at least two filenames are provided

v 6: 11.January.2016 [JRH]
	- add ability to select a list of clusters to extract

v 6: 20.November.2015 [JRH]
	- change "screen" keywords to avoid confusion with commonly used -sf (sample frequency)
		-s becomes -scr
		-sf becomes -scrf
		-sl becomes -scrl
*************************************************************************/

/* external functions start */
long xf_readclub1(char *infile1, char *infile2, long **clubt, short **club, char *message);
long xf_matchclub1_ls(char *list1, long *clubt, short *club, long nn, char *message);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
long xf_screen_club(long *start, long *stop, long nssp, long *time, short *data, long ndata, char *message);
void xf_qsortindex1_l(long *data, long *index,long nn);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile1[256],infile2[256],outfile1[256],outfile2[256],message[256];
	int v,w,x,y,z,sizeofshort=sizeof(short),sizeoflong=sizeof(long);
	long int ii,jj,kk,mm,nn;
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[64];
	FILE *fpin,*fpout1,*fpout2;
	struct stat sts;

	/* program-specific variables */
	short *club=NULL,*tempclub=NULL;
	long *clubt=NULL,*start1=NULL,*stop1=NULL,*index1=NULL,*index2=NULL,*clucount=NULL,*index3=NULL;
	long clumaxp1=-1,duration1;
	int setscreen=0;
	off_t nstart,nclulist;
	double duration2;

	/* arguments */
	char *setscreenfile=NULL,*setscreenlist=NULL,*setclulist=NULL;
	int setsort=0,setout=-1,setverb=0;
	float setfreq=19531.25;

	sprintf(outfile1,"temp_%s.clubt",thisprog);
	sprintf(outfile2,"temp_%s.club",thisprog);
	nstart=nclulist=0;

	/************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	************************************************************/
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Read binary cluster-timestamps (.clubt) and cluster-ids (.club)\n");
		fprintf(stderr,"- output is either converted to ASCII or kept in binary form\n");
		fprintf(stderr,"- use a file or list of boundaries to screen the start-stop pairs\n");
		fprintf(stderr,"- this program does not accept input piped via stdin\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [clubt] [club] [options]\n",thisprog);
		fprintf(stderr,"	[clubt]: binary file containing cluster-times (long int)\n");
		fprintf(stderr,"	[club]: binary file containing cluster-IDs (short int)\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-clu: screen using CSV list of cluster IDs [unset]\n");
		fprintf(stderr,"	-scrf: screen-file (binary ssp) defining bounds for infile1 [unset]\n");
		fprintf(stderr,"	-scrl: screen-list (CSV) defining bounds for infile1 [unset]\n");
		fprintf(stderr,"	-sort: sort records by ascending timestamps (0=NO 1=YES) [%d]\n",setsort);
		fprintf(stderr,"	-out: output format [%d]:\n",setout);
		fprintf(stderr,"		-2= summary (ID and COUNT) for each cluster\n");
		fprintf(stderr,"		-1= ASCII, one timestamp-id pair per line\n");
		fprintf(stderr,"		 0= binary files x2 (long,short)\n");
		fprintf(stderr,"		 	%s\n",outfile1);
		fprintf(stderr,"		 	%s\n",outfile2);
		fprintf(stderr,"	-sf: sample freq to calculate firing rates [%.3f]\n",setfreq);
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
			else if(strcmp(argv[ii],"-sort")==0) setsort=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)  setout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setout!=-2&&setout!=-1&&setout!=0) {fprintf(stderr,"\n--- Error[%s]: invalid -out (%d) : must be -2 -1 or 0\n\n",thisprog,setout);exit(1);}
	if(setsort<0 || setsort>1) {fprintf(stderr,"\n--- Error[%s]: invalid -sort (%d) : must be 0-1\n\n",thisprog,setsort);exit(1);}
	if(setverb<0 || setverb>1) {fprintf(stderr,"\n--- Error[%s]: invalid -verb (%d) : must be 0-1\n\n",thisprog,setverb);exit(1);}
	if(setscreenfile!=NULL && setscreenlist!=NULL) {fprintf(stderr,"\n--- Error[%s]: cannot define both a screen-list and a screen-file\n\n",thisprog);exit(1);}

	/************************************************************
	READ THE TIMESTAMP INCLUDE LIST OR FILE
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

	/* FIRST REPORT */
	if(setverb==1) {
		if(nstart>0) {
			fprintf(stderr,"sample-screen list...");
			for(jj=0;jj<nstart;jj++) fprintf(stderr,"start:%ld	stop:%ld\n",start1[jj],stop1[jj]);
		}
		fprintf(stderr,"reading input files...\n");
		fprintf(stderr,"	clubt file: %s\n",infile1);
		fprintf(stderr,"	club  file: %s\n",infile2);
	}


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
	FILTER USING THE SCREENING LIST
	- also set the recording-duration
	***********************************************************/
	if(setscreen==1) {
		nn= xf_screen_club(start1,stop1,nstart,clubt,club,nn,message);
		for(ii=duration1=0;ii<nstart;ii++) duration1+= (stop1[ii]-start1[ii]);
	}
	else { duration1= clubt[nn-1]-clubt[0]; }
	duration2= (double)duration1/(double)setfreq;
	if(setverb==1) fprintf(stderr,"- recording duration: %g seconds\n",duration2);

	/************************************************************
	KEEP ONLY THE CLUSTERS OF INTEREST
	***********************************************************/
	if(setclulist!=NULL) {
		nn= xf_matchclub1_ls(setclulist,clubt,club,nn,message);
		if(nn<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
		if(nn==0 && setverb==1) {fprintf(stderr,"--- Warning[%s]: no clusters matching input list\n",thisprog);}
	}

	/************************************************************
	SORT RECORDS BY TIMESTAMP
	***********************************************************/
	if(setsort==1) {
		index3= realloc(index3,nn*sizeof(*index3));
		tempclub= realloc(tempclub,nn*sizeof(*tempclub));
		if(index3==NULL||tempclub==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n",thisprog);}
		for(ii=0;ii<nn;ii++) index3[ii]=ii;
		xf_qsortindex1_l(clubt,index3,nn);
		for(ii=0;ii<nn;ii++) tempclub[ii]= club[index3[ii]];
		for(ii=0;ii<nn;ii++) club[ii]= tempclub[ii];
	}


	/************************************************************
	GET THE HIGHEST CLUSTER ID AND SET THE CLUSTER-COUNTS
	***********************************************************/
	clumaxp1=-1;
	if(nn>0) {
		for(ii=0;ii<nn;ii++) if(club[ii]>clumaxp1) clumaxp1=club[ii]; clumaxp1++;
		/* determine the counts in each cluster  */
		clucount= calloc(clumaxp1,sizeof(*clucount));
		if(clucount==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<nn;ii++) clucount[club[ii]]++;
	}

	/************************************************************
	OUTPUT THE DATA, SCREENING IF REQUIRED
	***********************************************************/
	/* output <ID> <count> */
	if(setout==-2) {
		printf("cluster\tcount\trate\n");
		for(ii=0;ii<clumaxp1;ii++) {
			if(clucount[ii]>0) {
				kk= clucount[ii];
				printf("%ld\t%ld\t%.4f\n",ii,kk,((double)kk/duration2));
	}}}
	/* output <timestamp> <ID> */
	else if(setout==-1) {
		for(ii=0;ii<nn;ii++) printf("%ld\t%hd\n",clubt[ii],club[ii]);
	}
	/* output clubt and club file  */
	else if(setout==0) {
		if((fpout1=fopen(outfile1,"wb"))==0) {fprintf(stderr,"\n--- Error[%s]: could not open file %s for writing\n\n",thisprog,outfile1);exit(1);}
		if((fpout2=fopen(outfile2,"wb"))==0) {fprintf(stderr,"\n--- Error[%s]: could not open file %s for writing\n\n",thisprog,outfile2);exit(1);}
		for(ii=0;ii<nn;ii++) { fwrite(clubt+ii,sizeoflong,1,fpout1);fwrite(club+ii,sizeofshort,1,fpout2);}
		fclose(fpout1);
		fclose(fpout2);
	}

 	/* SECOND REPORT - re-use clubt[] array for cluster-counts */
	if(setverb==1) {
		fprintf(stderr,"cluster\tcount\n");
		for(ii=0;ii<nn;ii++) clubt[ii]=0;
		for(ii=0;ii<nn;ii++) clubt[club[ii]]++;
		for(ii=0;ii<nn;ii++) { if(clubt[ii]>0) fprintf(stderr,"%ld\t%ld\n",ii,clubt[ii]); }
	}

 	if(club!=NULL) free(club);
 	if(clubt!=NULL) free(clubt);
 	if(index1!=NULL) free(index1);
	if(clucount!=NULL) free(clucount);
 	if(index2!=NULL) free(index2);
	if(index3!=NULL) free(index3);
	if(tempclub!=NULL) free(tempclub);
 	if(start1!=NULL) free(start1);
 	if(stop1!=NULL) free(stop1);
 	exit(0);

}
