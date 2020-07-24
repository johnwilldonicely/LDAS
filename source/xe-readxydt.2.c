
#define thisprog "xe-readxydt"
#define TITLE_STRING thisprog" v 2: 21.January.2018 [JRH]"
#define MAXLINELEN 1000
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>  /* this and next header allow testing file exists using stat() function */
#include <errno.h>

/* <TAGS> file behaviour </TAGS> */

/************************************************************************
v 2: 21.January.2018 [JRH]
	- apply ssp-screening AFTER velocity calculation and velocity-event-detection
	- this ensures artefactual velocities are not created by the screening

v 2: 9.January.2018 [JRH]
	- revised -out options (1-3 instead of -1-2)
	- begin adding option for summary output (-out 4) - for now just outputs mean velocity to stdout

v 2: 20.July.2016 [JRH]
		- bugfix in xf_detectevents3_lf: used to increment last timestamp of input if all input was good (no lonoger does this)

v 2: 17.July.2016 [JRH]
	- fix xf_screen_xydt so that check for overlapping SSPs cannot return error for very first start-signal
	- ie. do not initialize prev to zero because start[0] could be negative

v 2: 4.June.2016 [JRH]
	- allow user to set sampfreq, vidfreq, vidint

v 2: 1.April.2016 [JRH]
	- remove requirement to set -scr : assume filtering is inclusive (it was, anyway!)
	- bugfix: previously missing definition of binary file output pointers
	- bugfix: previously incorrect data-size (short) specified for binary output
	- bugfix: ensure at least two filenames are provided

v 2: 8.March.2016 [JRH]
	- add interpolation function
	- add event-detection (behavioural filter) function
	- add option to output SSP

v 2: 3.March.2016 [JRH]
	- switch to using xf_screen_xydt function (at present, inclusion only, not exclusion)
	- include elapsed time and velocity calculation in ASCII output

v 2: 20.November.2015 [JRH]
	- change "screen" keywords to avoid confusion with commonly used -sf (sample frequency)
	- change "screen" keywords to avoid confusion with commonly used -sf (sample frequency)
		-s becomes -scr
		-sf becomes -scrf
		-sl becomes -scrl
*************************************************************************/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
long xf_readxydt(char *infile1, char *infile2, long **post, float **posx, float **posy, float **posd, char *message);
long xf_dejump2_f(float *posx, float *posy, long nn, double sfreq, double thresh);
long xf_screen_xydt(long *start, long *stop, long nlist2, long *xydt, float *xydx, float *xydy, float *xydd, long ndata, char *message);
long xf_screen_ssp1(long *start1, long *stop1, long nlist1, long *start2, long *stop2, long nlist2, int mode, char *message);
long xf_screen_lf(long *start, long *stop, long nssp, long *time1, float *data, long ndata, char *message);
int xf_velocity1(float *posx, float *posy, float *velocity, long nn, double winsecs, double samprate, char *message);
long xf_detectevents3_lf(long *tstamp,float *data,long n1,float min,float max,long mindur,long **vstart,long **vstop,char *message);
long xf_interp3max_f(float *data, long ndata, long max);
long xf_stats3_f(float *data, long n1, int varcalc, double *result_d);
int xf_percentile1_f(float *data, long nn, double *result);
int xf_compare1_d(const void *a, const void *b);
int xf_filter_bworth1_f(float *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile1[256],infile2[256],outfile1[256],outfile2[256],message[256];
	int v,w,x,y,z,sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeoffloat=sizeof(float);;
	long int ii,jj,kk,mm,nn;
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[64];
	FILE *fpin,*fpout1,*fpout2;
	struct stat sts;

	/* program-specific variables */
	void *buffer1=NULL,*buffer2=NULL;
	int datasize,blocksread,setscreen=0;
	long headerbytes=0,maxread,blocksize,nlist2;
	long *xydt=NULL,*start1=NULL,*stop1=NULL,*index=NULL;
	long *start2=NULL,*stop2=NULL;
	float *xydx=NULL,*xydy=NULL,*xydd=NULL,*velocity=NULL;
	off_t params[4]={0,0,0,0},block,nread,nreadtot,nout,nlist1;

	/* arguments */
	char *setscreenfile=NULL,*setscreenlist=NULL;
	int setout=1,setverb=0;
	double setvidfreq=25.0, setsampfreq=19531.25,setdejump=-1.0,setvelmin=NAN,setvelmax=NAN,setveldur=0.0,setvelint=0.4,sethigh=2.5;
	long setinterp=0;
	off_t setheaderbytes=0;

	sprintf(outfile1,"temp_%s.xydt",thisprog);
	sprintf(outfile2,"temp_%s.xyd",thisprog);

	/************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	************************************************************/
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Read binary position timestamps (.xydt) and position values (.xyd)\n");
		fprintf(stderr,"- output is either converted to ASCII or kept in binary form\n");
		fprintf(stderr,"- use a file or list of boundaries to screen the start-stop pairs\n");
		fprintf(stderr,"- this program does not accept input piped via stdin\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [xydt] [xyd] [options]\n",thisprog);
		fprintf(stderr,"	[xydt]: binary file containing position-times (long int)\n");
		fprintf(stderr,"	[xyd]: binary file containing position-values (3x float)\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-sf: sample-frequency of clock used to timestamp video [%g]\n",setsampfreq);
		fprintf(stderr,"	-vf: video sample-frequency [%g]\n",setvidfreq);
		fprintf(stderr,"	-dejump: max cm/s, to invalidate jumpy x/y points [%g]\n",setdejump);
		fprintf(stderr,"	-scrf: screen-file (binary ssp) defining inclusion bounds []\n");
		fprintf(stderr,"	-scrl: screen-list (CSV) defining inclusion bounds []\n");
		fprintf(stderr,"	-int: max interpolation gap (video-samples) [%ld]\n",setinterp);
		fprintf(stderr,"		-1: no limit - interpolate all missing data\n");
		fprintf(stderr,"		 0: no interpolation\n");
		fprintf(stderr,"		 for 25Hz video, 10-25 (0.4-1.0 sec) recommended\n");
		fprintf(stderr,"	-high: path smoothing (1/sec), 0=NONE [%g]\n",sethigh);
		fprintf(stderr,"	-verb: set verbocity of output (0=low, 1=high) [%d]\n",setverb);
		fprintf(stderr,"	-out: output format [%d]:\n",setout);
		fprintf(stderr,"		0= summary: duration, velocity mean & median\n");
		fprintf(stderr,"		1= ASCII, one timestamp-id and x,y,d triplet per line\n");
		fprintf(stderr,"		2= binary files x2\n");
		fprintf(stderr,"		 	%s (long)\n",outfile1);
		fprintf(stderr,"		 	%s  (float triplets)\n",outfile2);
		fprintf(stderr,"		3= binary SSPs delineating velocity criteria (see below)\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"Velocity criterion for -out 3 option ...\n");
		fprintf(stderr,"	-velint: time (sec) over which velocity is integrated [%g]\n",setvelint);
		fprintf(stderr,"	-velmin: minimum (cm/s, nan = no limit) [%g]\n",setvelmin);
		fprintf(stderr,"	-velmax: maximum (cm/s, nan = no limit) [%g]\n",setvelmax);
		fprintf(stderr,"	-veldur: minimum duration (s) [%g]\n",setveldur);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.xydt data.xyd -scr 1 -scrl 10,20,40,50\n",thisprog);
		fprintf(stderr,"	%s A.xydt A.xyd -velmin 5 -out 3 > B.ssp\n",thisprog);
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
			else if(strcmp(argv[ii],"-sf")==0)     setsampfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-vf")==0)     setvidfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-dejump")==0) setdejump=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-int")==0)    setinterp=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-scr")==0)    setscreen=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-scrf")==0)   setscreenfile=argv[++ii];
			else if(strcmp(argv[ii],"-scrl")==0)   setscreenlist=argv[++ii];
			else if(strcmp(argv[ii],"-high")==0)   sethigh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)    setout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)   setverb=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-velint")==0) setvelint=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-velmin")==0) setvelmin=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-velmax")==0) setvelmax=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-veldur")==0) setveldur=atof(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setout<0 || setout>3) {fprintf(stderr,"\n--- Error[%s]: invalid -out (%d) : must be 0-3\n\n",thisprog,setout);exit(1);}
	if(setverb<0 || setverb>1) {fprintf(stderr,"\n--- Error[%s]: invalid -verb (%d) : must be 0-1\n\n",thisprog,setverb);exit(1);}
	if(setscreenfile!=NULL && setscreenlist!=NULL) {fprintf(stderr,"\n--- Error[%s]: cannot define both a screen-list and a screen-file\n\n",thisprog);exit(1);}


	/************************************************************
	READ THE POSITION TIMESTAMPS AND VALUES
	*************************************************************/
 	nn= xf_readxydt(infile1,infile2,&xydt,&xydx,&xydy,&xydd,message);
	if(nn<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
	if(setverb==1) {fprintf(stderr,"original_samples= %ld\n",nn);}


	/************************************************************
	READ THE INCLUDE OR EXCLUDE LIST
	/************************************************************/
	if(setscreenlist!=NULL) {
		setscreen=1;
		index= xf_lineparse2(setscreenlist,",",&nlist1);
		if((nlist1%2)!=0) {fprintf(stderr,"\n--- Error[%s]: screen-list does not contain pairs of numbers: %s\n\n",thisprog,setscreenlist);exit(1);}
		nlist1/=2;
		if((start1= realloc(start1,nlist1*sizeof(*start1)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((stop1=  realloc(stop1,nlist1*sizeof(*stop1)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<nlist1;ii++) {
			start1[ii]=atol(setscreenlist+index[2*ii]);
			stop1[ii]= atol(setscreenlist+index[2*ii+1]);
	}}
	else if(setscreenfile!=NULL) {
		setscreen=1;
		nlist1 = xf_readssp1(setscreenfile,&start1,&stop1,message);
		if(nlist1==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	}
	else {
		nlist1=1;
		if((start1= realloc(start1,nlist1*sizeof(*start1)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((stop1=  realloc(stop1,nlist1*sizeof(*stop1)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		/* set start/stop boundaries to entire record duration, if no list was defined */
		start1[0]= xydt[0];
		stop1[0]= xydt[(nn-1)];
	}
	//for(jj=0;jj<nlist1;jj++) printf("%ld	%ld	%ld\n",jj,start1[jj],stop1[jj]);free(start1);free(stop1);exit(0);


	/************************************************************
	CORRECT SCREEN-LIST FOR OUT-OF-BOUNDS VALUES
	- mainly so time-of-day-type SSPs don't completely mess up the trial-duration value
	*************************************************************/
	jj= xydt[0];
	kk= xydt[(nn-1)];
	for(ii=0;ii<nlist1;ii++) {
		if(start1[ii]<jj) start1[ii]=jj;;
		if(stop1[ii]>kk) stop1[ii]=kk;;
	}
	if(setverb==1) {
		fprintf(stderr,"total_screening_SSPs= %ld\n",nlist1);
		for(ii=0;ii<nlist1;ii++) fprintf(stderr,"	%ld	%ld\n",start1[ii],stop1[ii]);
	}


	/************************************************************
	DEJUMP THE DATA
	***********************************************************/
	if(setdejump>0.0){
		kk= xf_dejump2_f(xydx,xydy,nn,setvidfreq,setdejump);
	}

	/************************************************************
	APPLY INTERPOLATION  - units of setinterp (max) are video-samples
	***********************************************************/
	ii= xf_interp3max_f(xydx,nn,setinterp);
	ii= xf_interp3max_f(xydy,nn,setinterp);


	/************************************************************
	APPLY THE SMOOTHING (BUTTERWORTH FILTER)
	*************************************************************/
	if(sethigh>0.0) {
		xf_filter_bworth1_f(xydx,nn,setvidfreq,0,sethigh,1.4142,message);
		xf_filter_bworth1_f(xydy,nn,setvidfreq,0,sethigh,1.4142,message);
	}

	/************************************************************
	CALCULATE THE VELOCITY ARRAY
	*************************************************************/
	if(setverb==1) fprintf(stderr,"calculating velocity...\n");
	velocity= realloc(velocity,nn*sizeof(*velocity)); if(velocity==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	x= xf_velocity1(xydx,xydy,velocity,nn,setvelint,setvidfreq,message);
	if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

//for(ii=0;ii<nn;ii++) if(xydt[ii]>=278997 && xydt[ii]<308686) printf("%ld %.2f %.2f	%.3f\n",xydt[ii],xydx[ii],xydy[ii],velocity[ii]); exit(0);


	/************************************************************
	DETECT SSPs FOR GOOD VELOCITY EPOCHS
	- NOTE: the STOP signals should not be included in output
	*************************************************************/
	if(isfinite(setvelmin) || isfinite(setvelmax)) {
		if(setverb==1) fprintf(stderr,"detecting velocity epochs (%g-%g cm/s)...\n",setvelmin,setvelmax);
		/* detect velocity events */
		kk=(long)(setveldur*setsampfreq);
		nlist2= xf_detectevents3_lf(xydt,velocity,nn,setvelmin,setvelmax,kk,&start2,&stop2,message);
		if(nlist2==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		if(setverb==1) fprintf(stderr,"total_velocity_SSPs= %ld\n",nlist2);
	}
	else {
		nlist2= 1;
		if((start2= realloc(start2,nlist2*sizeof(*start2)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(2);};
		if((stop2=  realloc(stop2,nlist2*sizeof(*stop2)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(2);};
		start2[0]= 0;
		stop2[0]= nn;
	}

	/************************************************************
	1. SCREEN USING THE SCREEN LIST/FILE (TYPICALLY TRIALS)
	*************************************************************/
	if(setscreen==1) {
		if(setverb==1) fprintf(stderr,"extracting screening epochs...\n");
		/* screen the velocity array */
		mm= xf_screen_lf(start1,stop1,nlist1,xydt,velocity,nn,message);
		if(mm==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		/* screen the xydt data  */
		mm= xf_screen_xydt(start1,stop1,nlist1,xydt,xydx,xydy,xydd,nn,message);
		if(mm==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		/* update nn */
		nn= mm;
		/* screen the vel-list */
		if(isfinite(setvelmin) || isfinite(setvelmax)) {
			if(setverb==1) fprintf(stderr,"screening velocity SSPs...\n");
			mm= xf_screen_ssp1(start1,stop1,nlist1,start2,stop2,nlist2,1,message);
			if(mm==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
			nlist2= mm;
		}
	}


//for(ii=0;ii<nn;ii++) printf("%ld %.2f %.2f	%.3f\n",xydt[ii],xydx[ii],xydy[ii],velocity[ii]); exit(0);

	/************************************************************
	2. SCREEN USING VELOCITY
	- epochs and duration defined by setsampfreq (ephys time)
	*************************************************************/
	if(isfinite(setvelmin) || isfinite(setvelmax)) {
		if(setverb==1) fprintf(stderr,"extracting velocity epochs...\n");
		/* screen the velocity array */
		mm= xf_screen_lf(start2,stop2,nlist2,xydt,velocity,nn,message);
		if(mm==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		/* screen the xydt data  */
		mm= xf_screen_xydt(start2,stop2,nlist2,xydt,xydx,xydy,xydd,nn,message);
		if(mm==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

		/* update nn */
		nn= mm;
	}


//for(ii=0;ii<nn;ii++) printf("%ld %.2f %.2f	%.3f\n",xydt[ii],xydx[ii],xydy[ii],velocity[ii]); exit(0);

	/************************************************************
	OUTPUT THE DATA
	***********************************************************/
	/* output summary */
	if(setout==0) {
		/* get the record-duration */
		jj=0; for(ii=0;ii<nlist1;ii++) jj+= (stop1[ii]-start1[ii]);
		aa= (double)jj/setsampfreq;
		/* get the mean velocity */
		jj= xf_stats3_f(velocity,nn,2,result_d);
		if(jj>=0) bb= result_d[0]; else bb= NAN;
		/* get the median velocity */
		z= xf_percentile1_f(velocity,nn,result_d);
		if(z!=-1) cc= result_d[5]; else cc= NAN;
		printf("setvelmin= %.3f\n",setvelmin);
		printf("setvelmax= %.3f\n",setvelmax);
		printf("dur= %.3f\n",aa);
		printf("vmean= %.3f\n",bb);
		printf("vmedian= %.3f\n",cc);
	}
	/* ASCII output, including elapsed time (discontinuities ignored) and velocity */
	if(setout==1) {

		printf("xyd_t	xyd_x	xyd_y	xyd_d	vel	elapsed	absolute\n");
		for(ii=0;ii<nn;ii++) {
			aa=((double)ii/setvidfreq);
			bb=((double)(xydt[ii]-xydt[0])/setsampfreq);
			printf("%ld\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\n",xydt[ii],xydx[ii],xydy[ii],xydd[ii],velocity[ii],aa,bb);
		}
	}
	/* output binary xyd(t) file pair */
	if(setout==2) {
		if((fpout1=fopen(outfile1,"wb"))==0) {fprintf(stderr,"\n--- Error[%s]: could not open file %s for writing\n\n",thisprog,outfile1);exit(1);}
		if((fpout2=fopen(outfile2,"wb"))==0) {fprintf(stderr,"\n--- Error[%s]: could not open file %s for writing\n\n",thisprog,outfile2);exit(1);}
		for(ii=0;ii<nn;ii++) {
			fwrite(xydt+ii,sizeoflong,1,fpout1);
			fwrite(xydx+ii,sizeoffloat,1,fpout2);
			fwrite(xydy+ii,sizeoffloat,1,fpout2);
			fwrite(xydd+ii,sizeoffloat,1,fpout2);
 		}
		fclose(fpout1);
		fclose(fpout2);
	}
	/* output binary SSPs (start stop pairs) to stdout, for epochs matching velocity criteria */
	if(setout==3) {
		for(ii=0;ii<nlist2;ii++) {
			fwrite(start2+ii,sizeoflong,1,stdout);
			fwrite(stop2+ii,sizeoflong,1,stdout);
		}
	}

END:
 	if(xydt!=NULL) free(xydt);
 	if(xydx!=NULL) free(xydx);
 	if(xydy!=NULL) free(xydy);
 	if(xydd!=NULL) free(xydd);
 	if(index!=NULL) free(index);
 	if(start1!=NULL) free(start1);
 	if(stop1!=NULL) free(stop1);
 	if(start2!=NULL) free(start2);
	if(stop2!=NULL) free(stop2);
 	if(velocity!=NULL) free(velocity);
 	exit(0);

}
