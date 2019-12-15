#define thisprog "xe-ldas5-packetloss3"
#define TITLE_STRING thisprog" 24.March.2019 [JRH]"
#define MAXLINELEN 1000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> /* to get maximum possible short value */

/*
<TAGS> signal_processing time </TAGS>

24.March.2019 [JRH]
	- derived from packetloss2, but to use a lost.ssp file as input
*/

/* external functions start */
long xf_readssp1(char *infile, long **start, long **stop, char *message);
long xf_screen_ssp1(long *start1, long *stop1, long nssp1, long *start2, long *stop2, long nssp2, int mode, char *message);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long *xf_density2_l(long *start1,long *stop1,long nn,long min,long max,double winsize,long *nwin,char *message);
int xf_writebin1_v(FILE *fpout, void *data0, size_t nn, size_t datasize, char *message);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char *infile,*line=NULL,*templine=NULL,word[256],*pline,*pcol,message[MAXLINELEN];
	long int ii,jj,kk,ll,mm,nn,nbad,nchars=0,maxlinelen=0;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	size_t iii,jjj,kkk,lll,nnn,mmm;
	size_t sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);

	/* program-specific variables */
	long *start1=NULL,*stop1=NULL,*start2=NULL,*stop2=NULL,*index=NULL,*density=NULL;
	long nscreen,winsize,nwin;
	double min,max,mean;

	/* arguments */
	char *setscreenfile=NULL,*setscreenlist=NULL;
	int setverb=0,setscreen=0,setout=2;
	long setmin=-1,setmax=-1;
	double setsf=19531.25,setwin=1.0;

	if((line=(char *)realloc(line,6))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((templine=(char *)realloc(templine,MAXLINELEN))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate packet-loss using a lost.ssp file\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: binary .ssp file name or \"stdin\"\n");
		fprintf(stderr,"		- defines blocks of lost-packets (start-stop samples)\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-sf: sample frequency (Hz) [%.3f]\n",setsf);
		fprintf(stderr,"	-win: averaging-window size (seconds) [%g]\n",setwin);
		fprintf(stderr,"	-min: minimum-sample (-1=auto) [%ld]\n",setmin);
		fprintf(stderr,"	-max: maximum-sample (-1=auto) [%ld]\n",setmax);
		fprintf(stderr,"	-scrf: screen-file (binary ssp) defining inclusion bounds []\n");
		fprintf(stderr,"		- NOTE: SSP/sample-numbers output will be inaccurate\n");
		fprintf(stderr,"	-scrl: screen-list (CSV) defining inclusion bounds []\n");
		fprintf(stderr,"		- NOTE: SSP/sample-numbers output will be inaccurate\n");
		fprintf(stderr,"	-out: output [%d]\n",setout);
		fprintf(stderr,"		1= sumamry\n");
		fprintf(stderr,"		2= density time-series, using sf,win,min,max\n");
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES) [%d]\n",setverb);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	time	%%lost\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}



	/* READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item */
	infile=argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-sf")==0)    setsf= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-win")==0)   setwin= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-min")==0)   setmin= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-max")==0)   setmax= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-scrf")==0)  setscreenfile= argv[++ii];
			else if(strcmp(argv[ii],"-scrl")==0)  setscreenlist= argv[++ii];
			else if(strcmp(argv[ii],"-out")==0)   setout= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)  setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setscreenfile!=NULL && setscreenlist!=NULL) {fprintf(stderr,"\n--- Error[%s]: cannot define both a screen-list and a screen-file\n\n",thisprog);exit(1);}
	if(setwin<=0.0) { fprintf(stderr,"\n--- Error [%s]: -win (%g) must be >0\n\n",thisprog,setwin);exit(1);}
	if(setmin>setmax && setmax!=-1) { fprintf(stderr,"\n--- Error [%s]: -min (%ld) is greater than -max (%ld)\n\n",thisprog,setmin,setmax);exit(1);}
	if(setverb!=0 && setverb!=1 && setverb!=999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1 or 999\n\n",thisprog,setverb);exit(1);}
	if(setout<1||setout>2) {fprintf(stderr,"\n--- Error[%s]: invalid -out (%d) : must be 1-2\n\n",thisprog,setout);exit(1);}

	/************************************************************/
	/* STORE THE BLOCKS OF LOST-PACKET-SAMPLES  */
	/************************************************************/
	if(setverb>0) fprintf(stderr,"\treading input...");
	nn= xf_readssp1(infile,&start1,&stop1,message);
	if(nn==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	if(setverb>0) fprintf(stderr,"\tread %ld start-stop pairs\n",nn);
	if(setverb==999) { for(ii=0;ii<nn;ii++) fprintf(stderr,"INPUT %ld: %ld\t%ld\n",ii,start1[ii],stop1[ii]); }

	/************************************************************
	READ THE INCLUDE OR EXCLUDE LIST
	/************************************************************/
	if(setscreenlist!=NULL) {
		if(setverb>0) fprintf(stderr,"\tparsing screen-list\n");
		setscreen=3;
			index= xf_lineparse2(setscreenlist,",",&nscreen);
		if((nscreen%2)!=0) {fprintf(stderr,"\n--- Error[%s]: screen-list does not contain pairs of numbers: %s\n\n",thisprog,setscreenlist);exit(1);}
		nscreen/=2;
		if((start2=(long *)realloc(start2,nscreen*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((stop2=(long *)realloc(stop2,nscreen*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<nscreen;ii++) {
			start2[ii]=atol(setscreenlist+index[2*ii]);
			stop2[ii]= atol(setscreenlist+index[2*ii+1]);
	}}
	else if(setscreenfile!=NULL) {
		if(setverb>0) fprintf(stderr,"\treading screen-file %s\n",setscreenfile);
		setscreen=2;
		nscreen= xf_readssp1(setscreenfile,&start2,&stop2,message);
		if(nscreen==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	}
	else {
		if(setverb>0) fprintf(stderr,"\tno screen file or list defined - use all lost-packet pairs\n");
		setscreen=1;
		nscreen=1;
		if((start2=(long *)realloc(start2,nscreen*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((stop2=(long *)realloc(stop2,nscreen*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		start2[0]= start1[0];
		stop2[0]= stop1[(nn-1)];
	}
	if(setverb==999) { for(jj=0;jj<nscreen;jj++) fprintf(stderr,"SCREEN: %ld	%ld	%ld\n",jj,start2[jj],stop2[jj]); }

	/************************************************************/
	/* SCREEN THE DATA USING THE START=STOP PAIRS  */
	/************************************************************/
	if(setscreen>=2){
		if(setverb>0) fprintf(stderr,"\tscreening the data...\n");
		mm= xf_screen_ssp1(start2,stop2,nscreen,start1,stop1,nn,1,message);
		if(mm==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		nn=mm;
		if(setverb>0) fprintf(stderr,"\t%ld start-stop pairs after screening\n",nn);
	}

	/************************************************************/
	/* OUTPUT A SUMMARY OF RESULTS */
	/************************************************************/
	if(setout==1) {
		/* determine the total samples in the screening set */
		for(ii=jj=0;ii<nscreen;ii++) jj+= (stop2[ii]-start2[ii]);
		printf("total_samples= %ld\n",jj);
		for(ii=kk=0;ii<nn;ii++) kk+= (stop1[ii]-start1[ii]);
		printf("total_lost= %ld\n",kk);
		aa= 100.0*(double)kk/(double)jj;
		printf("percent_lost= %.12f\n",aa);
		goto END;
	}

	/************************************************************/
	/* BUILD THE DENSITY ARRAY */
	/************************************************************/
	if(setout==2) {
		winsize= setwin*setsf;
		if(setverb>0) fprintf(stderr,"\tbuilding density array (%gs window= %ld samples)...\n",setwin,winsize);
		density= xf_density2_l(start1,stop1,nn,setmin,setmax,winsize,&nwin,message);
		if(density==NULL) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		/* update setmin,setmax - internally, this is what the density function does */
		if(setmin<0) setmin= start1[0];
		if(setmax<0) setmax= stop1[(nn-1)];
		/* OUTPUT */
		if(setverb>0) fprintf(stderr,"\toutputting density data...\n");
		double offset= (double)setmin/setsf; /* time-offset of output */
		for(ii=0;ii<nwin;ii++) {
			aa= offset+ (double)ii*setwin;

			bb= (double)density[ii]/(double)winsize; /* convert density-counts to fraction of window */
			cc= 100*bb; /* convert to percentage */
			printf("%g\t%.4f\n",aa,cc);
		}
	}


	/************************************************************/
	/* CLEANUP AND EXIT  */
	/************************************************************/
END:
	if(start1!=NULL) free(start1);
	if(stop1!=NULL) free(stop1);
	if(start2!=NULL) free(start2);
	if(stop2!=NULL) free(stop2);
	exit(0);
}
