#define thisprog "xe-ldas5-placefields1"
#define TITLE_STRING thisprog" v 2: 16.March.2019 [JRH]"
#define MAXLINELEN 1000
#define MAXCLUSTER 1000

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <sys/stat.h>  /* this and next header allow testing file exists using stat() function */
#include <errno.h>

/************************************************************************
<TAGS>dt.spikes</TAGS>

v 2: 16.March.2019 [JRH]
	- update to use new function xf_screen_club - because the older xs-screen_ls should not update the timestamps

v 2: 25.November.2017 [JRH]
	- add ability to screen using a cluster list

v 2: 9.November.2017 [JRH]
		- change way fields are defined - now specify binsize instead of number of xbins and ybins

v 2: 1.April.2016
	- remove requirement to set -scr : assume filtering is inclusive (it was, anyway!)
	- bugfix: removed diagnostics from end of multimap output
	- add minimum dwelltime requirement (-dwell) for position matrix (if<minimum, dwell-count is reset to zero)
	- add bin-contiguity requirement (-con)
	- refine definition of xmin/ymin/xmax/ymax to allow scaling of xydx/xydy

v 2: 12.February.2016
	- add ability to output either firing rate or spike-counts
	- update instructions

v 2: 9.February.2016
	- two map smoothing options added
	- range definition added

v 2: 8.February.2016
	- matrix output now corrects for the fact that 0,0 is top-left
	- i.e. matrix output is now right-way-up

v 2: 8.December.2015 [JRH]
	- add density matrix calculation for dwelltimes and spikes

v 1: 20.November.2015 [JRH]"
	- change "screen" keywords to avoid confusion with commonly used -sf (sample frequency)
		-s becomes -scr
		-sf becomes -scrf
		-sl becomes -scrl

	- add temporary capability to output spike time,x,y

v 0: 10.November.2015 [JRH]"
*************************************************************************/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
long xf_readxydt(char *infile1, char *infile2, long **post, float **posx, float **posy, float **posd, char *message);
long xf_readclub1(char *infile1, char *infile2, long **clubt, short **club, char *message);
long xf_matchclub1_ls(char *list1, long *clubt, short *club, long nn, char *message);
long xf_screen_club(long *start, long *stop, long nssp, long *clubt, short *club, long nclub, char *message);
long xf_screen_xydt(long *start, long *stop, long nssp, long *xydt, float *xydx, float *xydy, float *xydd, long ndata, char *message);
long xf_fillinterp_lf(long *Atime, long *Btime, float *Aval, float *Bval, long Atot, long Btot, long maxinvalid, char *message);
int xf_matrixcontig1_l(long *matrix1, long setxbintot, long setybintot, long setcontig, char *message);
int xf_densitymatrix2_l(float *xdata, float *ydata, long nn, long *matrix, long setxbintot, long setybintot, float *ranges, char *message);
int xf_smoothgauss2_d(double *data,int xbintot,int ybintot,int xsmooth,int ysmooth);
int xf_matrixpeak1_d(double *data1,int *mask, long width, long height, float thresh, double *result);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char message[256];
	int v,w,x,y,z,sizeofshort=sizeof(short),sizeoflong=sizeof(long);
	long int ii,jj,kk,mm,nn;
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[64];
	FILE *fpin;
	struct stat sts;

	/* program-specific variables */
	void *buffer1=NULL,*buffer2=NULL;
	short *club=NULL,foundgood;
	long *clubt=NULL,*xydt=NULL,*start1=NULL,*stop1=NULL,*index=NULL;
	float *xydx=NULL,*xydy=NULL,*xydd=NULL;
	float *clubx=NULL,*cluby=NULL,*clubd=NULL;
	float *tempx=NULL,*tempy=NULL;
	float xmin,ymin,xmax,ymax,ranges[4];

	int *mask=NULL;
	long *index1=NULL,*matrix0=NULL,*matrix2=NULL,xbintot=-1,ybintot=-1;
	double *matrix1=NULL,*matrix3=NULL,matrixmax,matrixpeak;
	double binsize=-1;

	int datasize,blocksread,setscreen=0;
	long headerbytes=0,maxread,blocksize,matrixsize;
	long cluster,clustern[MAXCLUSTER];
	off_t params[4]={0,0,0,0},block,nread,nreadtot,nout,nssp,nclulist;

	/* arguments */
	char infile1[256],infile2[256],infile3[256],infile4[256];
	char *setscreenfile=NULL,*setscreenlist=NULL,*setclulist=NULL;
	char *setvrange=NULL;
	int setout=4,setverb=0,setsmoothtype=2,setflip=0;
	long setxsmooth=0,setysmooth=0,setdwellmin=0,setcontig=0;
	off_t setheaderbytes=0;
	float setxmin=NAN,setxmax=NAN,setymin=NAN,setymax=NAN;
	double setvidrate=25,setbinsize=1.0;

	/************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Read binary cluster-timestamps (.clubt) and cluster-ids (.club)\n");
		fprintf(stderr,"- output is either converted to ASCII or kept in binary form\n");
		fprintf(stderr,"- use a file or list of boundaries to screen the start-stop pairs\n");
		fprintf(stderr,"- this program does not accept input piped via stdin\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"  	%s [clubt] [club] [xyd] [xydt] [options]\n",thisprog);
		fprintf(stderr,"	[clubt]: binary file containing cluster-times (long int)\n");
		fprintf(stderr,"	[club]: binary file containing cluster-IDs (short int)\n");
		fprintf(stderr,"	[xydt]: binary file containing position-times (long int)\n");
		fprintf(stderr,"	[xyd]: binary file containing position-values (3x float)\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-clu: screen using CSV list of cluster IDs [unset]\n",setclulist);
		fprintf(stderr,"	-scrf: screen-file (binary ssp) defining bounds for infile1 [unset]\n");
		fprintf(stderr,"	-scrl: screen-list (CSV) defining bounds for infile1 [unset]\n");
		fprintf(stderr,"	-dwell: minimum samples for a dwell-map bin to be valid [%ld]\n",setdwellmin);
		fprintf(stderr,"	-con: number of contiguous bins required to keep a bin [%ld]\n",setcontig);
		fprintf(stderr,"	-vrate: video sample rate (samples/s) [%g]\n",setvidrate);
		fprintf(stderr,"	-vrange: 4-item CSV list defining xmin,ymin,xmax,ymax [unset]\n");
		fprintf(stderr,"	-binsize : size of map-pixels (cm) [%d]\n",setbinsize);
		fprintf(stderr,"	-sx : 2D gaussian smoothing (bins) applied to rate matrix [%ld]\n",setxsmooth);
		fprintf(stderr,"	-sy : 2D gaussian smoothing (bins) applied to matrix [%ld]\n",setysmooth);
		fprintf(stderr,"	-st : smoothing type (0,1 or 2) [%d]\n",setsmoothtype);
		fprintf(stderr,"		0= no smoothing\n");
		fprintf(stderr,"		1= smooth dwellmap and spikemap before calculating rate\n");
		fprintf(stderr,"		2= smooth the ratemap\n");
		fprintf(stderr,"	-flip: flip position data in y-dimension [%d]:\n",setflip);
		fprintf(stderr,"	-out: output format [%d]:\n",setout);
		fprintf(stderr,"		0= position traces only\n");
		fprintf(stderr,"		1= dwelltime density matrix\n");
		fprintf(stderr,"		2= path+spike x/y coordinates\n");
		fprintf(stderr,"		3= spike-density matrix (counts)\n");
		fprintf(stderr,"		4= spike-firing rate (Hz)\n");
		fprintf(stderr,"	-verb: set verbocity of output (0=low, 1=high) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.clubt data.club -scrl 100,200,1500,1600\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	************************************************************/
	sprintf(infile1,"%s",argv[1]);
	sprintf(infile2,"%s",argv[2]);
	sprintf(infile3,"%s",argv[3]);
	sprintf(infile4,"%s",argv[4]);
	if(strcmp(infile1,"stdin")!=0 && stat(infile1,&sts)==-1 && errno == ENOENT) {
		fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile1);
		exit(1);
	}
	if(strcmp(infile2,"stdin")!=0 && stat(infile2,&sts)==-1 && errno == ENOENT) {
		fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile2);
		exit(1);
	}
	if(strcmp(infile3,"stdin")!=0 && stat(infile3,&sts)==-1 && errno == ENOENT) {
		fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile3);
		exit(1);
	}
	if(strcmp(infile4,"stdin")!=0 && stat(infile4,&sts)==-1 && errno == ENOENT) {
		fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile4);
		exit(1);
	}
	for(ii=5;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-clu")==0)     setclulist=argv[++ii];
			else if(strcmp(argv[ii],"-vrange")==0)  setvrange= argv[++ii];
			else if(strcmp(argv[ii],"-vrate")==0)   setvidrate= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-scrf")==0)    setscreenfile= argv[++ii];
			else if(strcmp(argv[ii],"-scrl")==0)    setscreenlist= argv[++ii];
			else if(strcmp(argv[ii],"-dwell")==0)   setdwellmin= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-con")==0)     setcontig= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-binsize")==0) setbinsize= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-sx")==0)      setxsmooth=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-sy")==0)      setysmooth=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-st")==0)      setsmoothtype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-flip")==0)    setflip= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)     setout= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)    setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setout<0) {fprintf(stderr,"\n--- Error[%s]: invalid -out (%d) : must be >=0\n\n",thisprog,setout);exit(1);}
	if(setbinsize<=0) {fprintf(stderr,"\n--- Error[%s]: invalid -binsize (%d) : must be >0\n\n",thisprog,setbinsize);exit(1);}
	if(setverb<0 || setverb>1) {fprintf(stderr,"\n--- Error[%s]: invalid -verb (%d) : must be 0-1\n\n",thisprog,setverb);exit(1);}
	if(setflip<0 || setflip>1) {fprintf(stderr,"\n--- Error[%s]: invalid -flip (%d) : must be 0-1\n\n",thisprog,setflip);exit(1);}
	if(setscreenfile!=NULL && setscreenlist!=NULL) {fprintf(stderr,"\n--- Error[%s]: cannot define both a screen-list and a screen-file\n\n",thisprog);exit(1);}
	if(setdwellmin<0) {fprintf(stderr,"\n--- Error[%s]: invalid -dwellmin (%ld) : must be >=0\n\n",thisprog,setdwellmin);exit(1);}
	if(setcontig<0 || setcontig>8) {fprintf(stderr,"\n--- Error[%s]: invalid -contig (%ld) : must be 0-8\n\n",thisprog,setcontig);exit(1);}
	if(setvidrate<1) {fprintf(stderr,"\n--- Error[%s]: invalid -vr (%g) : must be >0 \n\n",thisprog,setvidrate);exit(1);}
	if(setxsmooth<0) {fprintf(stderr,"\n--- Error[%s]: invalid -sx (%ld) : must be >=0\n\n",thisprog,setxsmooth);exit(1);}
	if(setysmooth<0) {fprintf(stderr,"\n--- Error[%s]: invalid -sy (%ld) : must be >=0\n\n",thisprog,setysmooth);exit(1);}
	if(setsmoothtype<0 || setsmoothtype>2) {fprintf(stderr,"\n--- Error[%s]: invalid -st (%d) : must be 1 or 2\n\n",thisprog,setsmoothtype);exit(1);}

	/************************************************************
	READ THE VIDEO RANGES
	/************************************************************/
	if(setvrange!=NULL) {
		index= xf_lineparse2(setvrange,",",&kk);
		if(kk!=4) {fprintf(stderr,"\n--- Error[%s]: -vrange (%s) must be a 4-item list\n\n",thisprog,setvrange);exit(1);}
		setxmin=atof(setvrange+index[0]);
		setymin=atof(setvrange+index[1]);
		setxmax=atof(setvrange+index[2]);
		setymax=atof(setvrange+index[3]);
	}

	/************************************************************
	READ THE INCLUDE OR EXCLUDE LIST
	/************************************************************/
	if(setscreenlist!=NULL) {
		setscreen=1;
		index= xf_lineparse2(setscreenlist,",",&nssp);
		if((nssp%2)!=0) {fprintf(stderr,"\n--- Error[%s]: screen-list does not contain pairs of numbers: %s\n\n",thisprog,setscreenlist);exit(1);}
		nssp/=2;
		if((start1=(long *)realloc(start1,nssp*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((stop1=(long *)realloc(stop1,nssp*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<nssp;ii++) {
			start1[ii]=atol(setscreenlist+index[2*ii]);
			stop1[ii]= atol(setscreenlist+index[2*ii+1]);
	}}
	else if(setscreenfile!=NULL) {
		setscreen=1;
		nssp = xf_readssp1(setscreenfile,&start1,&stop1,message);
		if(nssp==-1) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}
	//for(jj=0;jj<nssp;jj++) printf("%ld	%ld	%ld\n",jj,start1[jj],stop1[jj]);free(start1);free(stop1);exit(0);

	/************************************************************
	READ THE POSITION TIMESTAMPS AND VALUES
	***********************************************************/
 	mm= xf_readxydt(infile3,infile4,&xydt,&xydx,&xydy,&xydd,message);
	if(mm<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}

	/************************************************************
	READ THE CLUSTER TIMESTAMPS AND IDs
	***********************************************************/
 	nn= xf_readclub1(infile1,infile2,&clubt,&club,message);
 	if(nn<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}

	/************************************************************
	APPLY CORRECTIONS TO XYD DATA
	- invalidate (set to NAN) data outside the maxima and maxima
	- reduce all data above the minima by the minima
	- leave NANs untouched
	***********************************************************/
	if(setvrange!=NULL) {
		for(ii=0;ii<mm;ii++) {
			a=xydx[ii];
			b=xydy[ii];
			if(isfinite(a)) { if(a<setxmin || a>setxmax) a=NAN; else a-=setxmin; }
			if(isfinite(b)) { if(b<setymin || b>setymax) b=NAN; else b-=setymin; }
		}
		/* set the data ranges */
		xmin= 0.0;
		ymin= 0.0;
		xmax= setxmax-setxmin;
		ymax= setymax-setymin;
	}
	/* otherwise determine xyd ranges based on the input */
	else {
		xmin= nextafter(FLT_MAX,0.0);  for(ii=0;ii<mm;ii++) { a=xydx[ii]; if(isfinite(a) && a<=xmin) xmin=a; }
		ymin= nextafter(FLT_MAX,0.0);  for(ii=0;ii<mm;ii++) { a=xydy[ii]; if(isfinite(a) && a<=ymin) ymin=a; }
		xmax= nextafter(-FLT_MAX,0.0); for(ii=0;ii<mm;ii++) { a=xydx[ii]; if(isfinite(a) && a>=xmax) xmax=a; }
		ymax= nextafter(-FLT_MAX,0.0); for(ii=0;ii<mm;ii++) { a=xydy[ii]; if(isfinite(a) && a>=ymax) ymax=a; }
	}

	/************************************************************
	FINAL CHECK - IS THERE GOOD XYD DATA AFTER APPLYING MIN-MAX CRITERA?
	***********************************************************/
	foundgood=0;
	for(ii=0;ii<mm;ii++) {
		a=xydx[ii];
		b=xydy[ii];
		if(isfinite(a) && a>=xmin && a<=xmax && isfinite(b) && b>=ymin && b<=ymax) { foundgood=1; break; }
	}
	if(foundgood==0) {fprintf(stderr,"\n\t--- %s [ERROR]: xyd(t) input contains no valid in-range numbers\n\n",thisprog);exit(1);}
	// fprintf(stderr,"\n");
	// fprintf(stderr,"xmin=%f\n",xmin);
	// fprintf(stderr,"ymin=%f\n",ymin);
	// fprintf(stderr,"xmax=%f\n",xmax);
	// fprintf(stderr,"ymax=%f\n",ymax);


	/************************************************************
	DETERMINE MATIX-SIZE
	- this could be a running speed screen, sharp-waves, etc
	- redefines "n" for video (mm) and spike (nn) records
	***********************************************************/
	if(setbinsize>0) {
		xbintot= (long)((xmax-xmin)/setbinsize);
		ybintot= (long)((ymax-ymin)/setbinsize);
	}
	matrixsize= xbintot*ybintot;


	/************************************************************
	APPLY SSP SCREEN (LIST OR FILE) TO SPIKES and XYD
	- this could be a running speed screen, sharp-waves, etc
	- redefines "n" for video (mm) and spike (nn) records
	***********************************************************/
	if(setscreen==1) {
		mm= xf_screen_xydt(start1,stop1,nssp,xydt,xydx,xydy,xydd,mm,message);
		nn= xf_screen_club(start1,stop1,nssp,clubt,club,nn,message);
	}

	/************************************************************
	KEEP ONLY THE CLUSTERS OF INTEREST
	***********************************************************/
	if(setclulist!=NULL) {
		nn= xf_matchclub1_ls(setclulist,clubt,club,nn,message);
		if(nn<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
	}


	/************************************************************
	ALLOCATE MEMORY
	clubx   = spike-x-coordinates
	cluby   = spike-y-coordinates
	clubd   = spike-direction
	tempx   = temporary spike-x-coordinates array for a single cluster
	tempy   = temporary spike-y-coordinates array for a single cluster
	matrix0 = dwell-counts
	matrix1 = dwell-time
	matrix2 = spike-counts
	matrix3 = spike-rate
	***********************************************************/
	if((clubx=realloc(clubx,nn*sizeof(*clubx)))==NULL) {fprintf(stderr,"\n\t--- %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);}
	if((cluby=realloc(cluby,nn*sizeof(*cluby)))==NULL) {fprintf(stderr,"\n\t--- %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);}
	if((clubd=realloc(clubd,nn*sizeof(*clubd)))==NULL) {fprintf(stderr,"\n\t--- %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);}
	if((tempx=realloc(tempx,nn*sizeof(*tempx)))==NULL) {fprintf(stderr,"\n\t--- %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);}
	if((tempy=realloc(tempy,nn*sizeof(*tempy)))==NULL) {fprintf(stderr,"\n\t--- %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);}
	if((matrix0=realloc(matrix0,matrixsize*sizeof(*matrix0)))==NULL) {fprintf(stderr,"\n\t--- %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);}
	if((matrix1=realloc(matrix1,matrixsize*sizeof(*matrix1)))==NULL) {fprintf(stderr,"\n\t--- %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);}
	if((matrix2=realloc(matrix2,matrixsize*sizeof(*matrix2)))==NULL) {fprintf(stderr,"\n\t--- %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);}
	if((matrix3=realloc(matrix3,matrixsize*sizeof(*matrix3)))==NULL) {fprintf(stderr,"\n\t--- %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);}
	if((mask=realloc(mask,matrixsize*sizeof(*mask)))==NULL) {fprintf(stderr,"\n\t--- %s [ERROR]: insufficient memory\n\n",thisprog);exit(1);}

	/************************************************************
	DETERMINE SPIKE XYD DATA
	- based on hux_fillinterp functions
	************************************************************/
	kk= xf_fillinterp_lf(xydt,clubt,xydx,clubx,mm,nn,0,message);
	kk= xf_fillinterp_lf(xydt,clubt,xydy,cluby,mm,nn,0,message);
	//for(ii=0;ii<nn;ii++) printf("%f	%f\n",clubx[ii],cluby[ii]); exit(1);

	/************************************************************
	CALCULATE DWELLTIME DENSITY MATRICES
	- MATRIX0 = COUNTS (long)
	- MATRIX1 = TIME (double)
	************************************************************/
	/* matrix0: get dwell-counts */
	ranges[0]=xmin;
	ranges[1]=ymin;
	ranges[2]=xmax;
	ranges[3]=ymax;
	z= xf_densitymatrix2_l(xydx,xydy,mm,matrix0,xbintot,ybintot,ranges,message);
	if(z==-1) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	/* apply minimum-dwelltime (samples) to position matrix0 */
	for(ii=0;ii<matrixsize;ii++) { if(matrix0[ii]<setdwellmin) matrix0[ii]=0; }
	/* set count in non-contiguous bins to 0 */
	z= xf_matrixcontig1_l(matrix0,xbintot,ybintot,setcontig,message);
	if(z==-1) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	/* matrix1: calculate dwelltime (seconds) */
	for(ii=0;ii<matrixsize;ii++) { if(matrix0[ii]!=0) matrix1[ii]=(double)matrix0[ii]/setvidrate; else matrix1[ii]=NAN; }
	/* smooth dwell-map if required */
	if(setsmoothtype==1) {
		z= xf_smoothgauss2_d(matrix1,xbintot,ybintot,setxsmooth,setysmooth);
		if(z!=0) {fprintf(stderr,"\n\t--- %s [ERROR]: smoothing function encountered insufficient memory\n\n",thisprog);exit(1);}
		for(ii=0;ii<matrixsize;ii++) { if(matrix0[ii]==0) matrix1[ii]=NAN; }
	}
	// set top left dwelltime to NAN - right now, this bin contains "mistracked" samples
	matrix0[0]=0;
	matrix1[0]=NAN;


	/************************************************************
	OUTPUT 0: PATH ONLY
	***********************************************************/
	if(setout==0) {
		for(ii=0;ii<mm;ii++) printf("%ld\t%g\t%g\t%g\n",xydt[ii],xydx[ii],xydy[ii],xydd[ii]);
	}
	/************************************************************
	OUTPUT 1: DWELLTIME MATRIX ONLY
	***********************************************************/
	else if(setout==1) {
		printf("# %g cm/pixel dwelltime matrix\n",setbinsize);
		for(ii=ybintot-1;ii>=0;ii--) {
			for(jj=0;jj<xbintot;jj++) {
				kk= ii*xbintot + jj;
				if(jj>0) printf("\t");
				printf("%g",matrix1[kk]);
			}
		printf("\n");
		}
	}

	/************************************************************
	OUTPUT 2+ : PER-CLUSTER OUTPUT
		2= trace + spike positions
		3= spike density matrix
		4= firing rate matrix
	***********************************************************/
	else if(setout>=2) {

		/* initialize & get the spike counts in each cluster */
		for(ii=0;ii<MAXCLUSTER;ii++) clustern[ii]=0;
		for(ii=0;ii<nn;ii++) clustern[club[ii]]++;

 		for(cluster=0;cluster<MAXCLUSTER;cluster++) {

			if(clustern[cluster]==0) continue;
			if(setverb==1) fprintf(stderr,"	 -building cluster %ld\n",cluster);

			/* generate tempx & tempy */
			for(ii=jj=0;ii<nn;ii++) {
				if(club[ii]==cluster) {
					tempx[jj]=clubx[ii];
					tempy[jj]=cluby[ii];
					jj++;
				}
			}

			/* if only outputting spike positions, this is as far as we need to go */
			if(setout==2) {
				for(ii=0;ii<mm;ii++) printf("0\t%g\t%g\n",xydx[ii],xydy[ii]);
				for(ii=0;ii<clustern[cluster];ii++) printf("1\t%g\t%g\n",tempx[ii],tempy[ii]);
				continue;
			}

			/* calculate spike-count density matrix (matrix2) - use position ranges previously defined for dwell-map */
			z= xf_densitymatrix2_l(tempx,tempy,clustern[cluster],matrix2,xbintot,ybintot,ranges,message);
			if(z==-1) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

			/* convert to to double (matrix3) */
		 	for(ii=0;ii<matrixsize;ii++) matrix3[ii]= (double)matrix2[ii];

			/* apply smoothing to spike-counts  */
			if(setsmoothtype==1) {
				z= xf_smoothgauss2_d(matrix3,(int)xbintot,(int)ybintot,(int)setxsmooth,(int)setysmooth);
				if(z!=0) {fprintf(stderr,"\n\t--- %s [ERROR]: smoothing function encountered insufficient memory\n\n",thisprog);exit(1);}
			}

			/* convert spike counts to rates (modify matrix3) */
		 	if(setout==4) for(ii=0;ii<matrixsize;ii++) { if(matrix0[ii]>0) matrix3[ii]/=matrix1[ii]; else matrix3[ii]=NAN; }

			/* apply smoothing to rate-map */
			if(setsmoothtype==2) {
				z= xf_smoothgauss2_d(matrix3,xbintot,ybintot,setxsmooth,setysmooth);
	  			if(z!=0) {fprintf(stderr,"\n\t--- %s [ERROR]: smoothing function encountered insufficient memory\n\n",thisprog);exit(1);}
			}

			/* restore NANs to bins that were unvisited */
			for(ii=0;ii<matrixsize;ii++) if(matrix0[ii]==0) matrix3[ii]=NAN;

			/* print the matrix with row "0" last (i.e. low values of y at the botttom) */
			printf("# %ld : %ld spikes\n",cluster,clustern[cluster]);
			for(ii=ybintot-1;ii>=0;ii--) {
				for(jj=0;jj<xbintot;jj++) {
					kk= ii*xbintot + jj;
					if(jj>0) printf("\t");
					printf("%g",matrix3[kk]);
				}
				printf("\n");
			}

 		} // END OF LOOP: for cluster=0 to maxcluster
	} // END OF LOOP: else if(setout==2)

	//TEST: if(setout>=0) { or(ii=0;ii<nn;ii++) if(club[ii]==setout) fprintf(stderr,"%ld\t%f\t%f\n",clubt[ii],clubx[ii],cluby[ii]); }


	/********************************************************************************
	WRAP-UP: FREE MEMORY AND EXIT
	********************************************************************************/
	if(index1!=NULL) free(index1);
	if(club!=NULL) free(club);
 	if(clubt!=NULL) free(clubt);
	if(clubx!=NULL) free(clubx);
 	if(cluby!=NULL) free(cluby);
 	if(clubd!=NULL) free(clubd);
 	if(xydx!=NULL) free(xydx);
 	if(xydy!=NULL) free(xydy);
 	if(xydd!=NULL) free(xydd);
 	if(xydt!=NULL) free(xydt);
 	if(tempx!=NULL) free(tempx);
 	if(tempy!=NULL) free(tempy);
 	if(index!=NULL) free(index);
 	if(start1!=NULL) free(start1);
 	if(stop1!=NULL) free(stop1);
 	if(matrix0!=NULL) free(matrix0);
 	if(matrix1!=NULL) free(matrix1);
 	if(matrix2!=NULL) free(matrix2);
	if(matrix3!=NULL) free(matrix3);
	if(mask!=NULL) free(mask);
	exit(0);

}
