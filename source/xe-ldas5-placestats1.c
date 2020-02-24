#define thisprog "xe-ldas5-placestats1"
#define TITLE_STRING thisprog" v 3: 21.October.2017 [JRH]"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>dt.spikes</TAGS>

v 3: 21.October.2017 [JRH]
*/


/* external functions start */
long xf_matrixread2_d(char *infile, long idcol, double **matrix1, double **id1, long *ncols, long *nrows, char *message);
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_smoothgauss2_d(double *data,int xbintot,int ybintot,int xsmooth,int ysmooth);
int xf_matrixpeak1_d(double *data1,int *mask, size_t width, size_t height, double thresh, double *result);
int xf_placestats1_d(double *dwell,double *rate,long width,long height,double dwelltot,double *result, char *message);
long xf_stats3_d(double *data, long n, int varcalc, double *result_in);
double xf_correlate_simple_d(double *x, double *y, long nn, double *result_d);
int xf_percentile1_d(double *data, long n, double *result);
int xf_compare1_d(const void *a, const void *b);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char outfile[256],*line=NULL,*templine=NULL,*pline,*pcol,message[256];
	long ii,jj,kk,nn,*index=NULL;
	int v,w,x,y,z,n=0,col,colmatch,result_i[32];
	int sizeofint=sizeof(int),sizeoflong=sizeof(long),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[8];
	FILE *fpin,*fpout;
	/* program-specific variables */
	int *mask;
	long nmatrices,width,height,count=0,peakfound;
	double xmin,xmax,ymin,ymax;
	double *matrix0=NULL,*matrix1=NULL,*id0=NULL,*id1=NULL,*pmatrix1,binsize,dwelltot,fsize,fx,fy,fmax1,thresh;
	double rate_mean,rate_base= result_d[2],rate_max,rate_median,rate_peak,space_info,space_spar,space_coh;

	/* arguments */
	char *filedwell,*filerate,*setvrange=NULL;
	int setout=1;
	long setsize=0,setsmooth=4;
	double setxmin=NAN,setxmax=NAN,setymin=NAN,setymax=NAN;
	double setthresh=.25;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Perform place-field analysis on a multi-matrix file\n");
		fprintf(stderr," - this peak starts at the highest-value pixel and propogates outward\n");
		fprintf(stderr," - diagonal propogation is not permitted\n");
		fprintf(stderr," - if peak size is below a minimum, another attempt is made\n");
		fprintf(stderr," - attempts are made until no peak is detected at all\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [dwell] [rate] [options]\n",thisprog);
		fprintf(stderr,"		[dwell]: dwelltime matrix file\n");
		fprintf(stderr,"		[rate]: firing-rate multi-matrix file\n");
		fprintf(stderr,"			- matrices separated by \"# <id-number>\" lines\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-vrange: 4-item CSV list defining xmin,ymin,xmax,ymax [unset]\n");
		fprintf(stderr,"	-smooth : 2D gaussian smoothing (bins) applied for field-detection [%ld]\n",setsmooth);
		fprintf(stderr,"	-thresh : edge-detection threshold (proportion of peak) [%g]\n",setthresh);
		fprintf(stderr,"	-size   : size threshold (number of pixels, zero=any) [%d]\n",setsize);
		fprintf(stderr,"	-out    : output [%d]\n",setout);
		fprintf(stderr,"		1: map-pixel statistics are sent to stderr\n");
		fprintf(stderr,"			cluster: cell-ID\n");
		fprintf(stderr,"			rmax: highest firing-rate pixel\n");
		fprintf(stderr,"			rmean: firing-rate mean\n");
		fprintf(stderr,"			rbase: firing rate 10th-percentile - \"background\" rate\n");
		fprintf(stderr,"			rmed: firing-rate 50th-percentile\n");
		fprintf(stderr,"			rpeak: firing rate 97.5th percentile - \"peak\" rate\n");
		fprintf(stderr,"			info: information content\n");
		fprintf(stderr,"			spar: spatial-sparsity\n");
		fprintf(stderr,"			coh: spatial coherence\n");
		fprintf(stderr,"			fmax: field-max-rate after smoothing\n");
		fprintf(stderr,"			fsize: field-size (pixels)\n");
		fprintf(stderr,"			fx: field centroid position (pixel), x-axis\n");
		fprintf(stderr,"			fy: field centroid position (pixel), y-axis\n");
		fprintf(stderr,"		2: the final place-field mask sent to stdout\n");
		fprintf(stderr,"		3: both 1 & 2\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	{ %s matrix.txt -thresh 1 -first 0 > matrix2.txt ; } 2> report.txt \n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	filedwell= argv[1];
	filerate=  argv[2];
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-vrange")==0)  setvrange= argv[++ii];
			else if(strcmp(argv[ii],"-smooth")==0) setsmooth=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-thresh")==0) setthresh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-size")==0) setsize=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0) setout=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setsize<0) {fprintf(stderr,"\n--- Error[%s]: -s (%ld) must be zero or greater\\n\n",thisprog,setsize); exit(1);}
	if(setthresh<0||setthresh>1) {fprintf(stderr,"\n--- Error[%s]: invalid -thresh (%g) must be 0-1\\n\n",thisprog,setthresh); exit(1);}

	/************************************************************
	READ THE VIDEO RANGES
	/************************************************************/
	if(setvrange!=NULL) {
		index= xf_lineparse2(setvrange,",",&ii);
		if(ii!=4) {fprintf(stderr,"\n--- Error[%s]: -vrange (%s) must be a 4-item list\n\n",thisprog,setvrange);exit(1);}
		setxmin= atof(setvrange+index[0]);
		setymin= atof(setvrange+index[1]);
		setxmax= atof(setvrange+index[2]);
		setymax= atof(setvrange+index[3]);
	}

	/********************************************************************************/
	/* STORE THE DWELL MATRIX - use jj & kk to temporarily store the width & height */
	/********************************************************************************/
	/* call the read function */
	nmatrices= xf_matrixread2_d(filedwell,1,&matrix0,&id0,&jj,&kk,message);
	if(nmatrices==-1) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
	if(nmatrices!=1) {fprintf(stderr,"\n--- Error[%s]: %s is not a dwell-time matrix: more than one matrix in file\n\n",thisprog,filedwell);exit(1);}
	binsize= id0[0];

	/********************************************************************************/
	/* STORE THE RATE MATRIX */
	/********************************************************************************/
	/* call the read function */
	nmatrices= xf_matrixread2_d(filerate,1,&matrix1,&id1,&width,&height,message);
	if(nmatrices==-1) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
	/* check matrices are the same size */
	if(jj!=width | kk!=height) {fprintf(stderr,"\n--- Error[%s]: dwell and rate matrices have different dimensions (%ldx%ld vs. %ldvs%ld)\\n\n",thisprog,jj,width,kk,height); exit(1);}
	/* allocate memory for the mask */
	nn= width*height;
	mask= malloc(nn*sizeof(*mask));
	if(mask==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	/********************************************************************************/
	/* DEFINE THE DATA-POSITION RANGES AUTOMATICALLY */
	/********************************************************************************/
	xmin=0; xmax= (double)width*binsize;
	ymin=0; ymax= (double)height*binsize;
	// fprintf(stderr,"xmin=%g ymin=%g xmax=%g ymax=%g\n",xmin,ymin,xmax,ymax);

	/********************************************************************************/
	/* CONVERT DWELL NANs TO ZERO AND GET THE TOTAL DWELLTIME - THIS IS USED BY THE MATRIXSTATS FUNCTION */
	/********************************************************************************/
	dwelltot=0.0;
	for(ii=0;ii<nn;ii++) {
		aa= matrix0[ii];
		if(!isfinite(aa)) aa= 0.0;
		matrix0[ii]= aa;
		dwelltot+= aa;
	}

	/********************************************************************************/
	/* FOR EACH MATRIX */
	/********************************************************************************/
	printf("cluster	rmax	rmean	rbase	rmed	rpeak	info	spar	coh	fmax	fsize	fx	fy\n");
	for(kk=0;kk<nmatrices;kk++) {

		/* set the matrix pointer */
		pmatrix1= matrix1+(nn*kk);

		/* CALCULATE BASIC AND SPATIAL STATISTICS */
		ii= xf_placestats1_d(matrix0,pmatrix1,width,height,dwelltot,result_d,message);
		rate_max= result_d[0];
		rate_mean= result_d[1];
		rate_base= result_d[2];   // 10th percentile
		rate_median= result_d[3]; // 50th percentile
		rate_peak= result_d[4];   // 97.5th percentile = midpoint of 95th percentile
		space_info= result_d[6];
		space_spar= result_d[7];
		space_coh= result_d[8];

		/* DEFINE THE PLACE FIELD */
		/* use mask array to store NAN status */
		for(ii=0;ii<nn;ii++) { if(isfinite(pmatrix1[ii])) mask[ii]=0; else mask[ii]=-1; }
		/* apply 2d smoothing */
		z= xf_smoothgauss2_d(pmatrix1,width,height,setsmooth,setsmooth);
		/* use the mask to restore NAN's  */
		for(ii=0;ii<nn;ii++) { if(mask[ii]==-1) pmatrix1[ii]= NAN; }
		/* get the max value */
		fmax1=0; for(ii=0;ii<nn;ii++) { aa= pmatrix1[ii]; if(isfinite(aa) && aa>fmax1) fmax1=aa; }
		/* define the matrix-specific threshold */
		thresh= fmax1*setthresh;
		/* re-initialize the mask */
		for(ii=0;ii<nn;ii++) mask[ii]=0;
		/* detect the highest-rate peak firing zone */
		peakfound= xf_matrixpeak1_d(pmatrix1,mask,width,height,thresh,result_d);
		if(peakfound==1 && result_d[0]>setsize) {
			fsize= result_d[0];   // number of pixels in the field
			fx= result_d[3];      // x-centroid
			fy= result_d[4];      // y-centroid
			fmax1= result_d[6];  // highest-value peak in the field
			/* convert pixels to cm */
			fmax1*= binsize;
			fsize*= binsize;
			fx*= binsize;
			fy= ymax-(fy*binsize); // for y-value, convert back to cartesian coordinates where 0,0 = bottom,left
		}
		else fsize=fx=fy=fmax1=NAN;


		/* OUTPUT THE RESULTS */
		printf("%g	%.3f	%.3f	%.3f	%.3f	%.3f	%.3f	%.3f	%.3f	%.3f	%.3f	%.3f	%.3f\n",
			id1[kk],
			rate_max,rate_mean,rate_base,rate_median,rate_peak,
			space_info,space_spar,space_coh,
			fmax1,fsize,fx,fy
		);



	} // END OF PER-MATRIX LOOP (KK)

	/* FREE MEMORY AND EXIT */
	if(matrix0!=NULL) free(matrix0);
	if(matrix1!=NULL) free(matrix1);
	if(id0!=NULL) free(id0);
	if(id1!=NULL) free(id1);
	if(mask!=NULL) free(mask);
	exit(0);

	}
