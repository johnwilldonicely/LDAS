#define thisprog "xe-matrixmod1"
#define TITLE_STRING thisprog" v 21.October.2017 [JRH]"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>signal_processing matrix</TAGS>

v 21.October.2017 [JRH]
	- update variable naming conventions for ii,jj,kk

v 14.October.2017 [JRH]
	- added -pn option to preserve NAN's in plot after smoothing

v 19: 30.September.2016 [JRH]
	- major overhaul to reduce memory overhead associated with resampling
		- now uses two separate functions to bin and expand matrixes
		- binning has no memory overhead
		- expansion temporarily increases memory requirements but this is offset by reducing memory first if binning is performed in one dimension

v 19: 08.December.2015 [JRH]
	- add ability to flip matrixes in the x- or y-axis

v 18: 10.March.2014 [JRH]
	- new output method eliminates extra tab on end of lines

v 17: 11.February.2014 [JRH]
	- bugfix: reports memory allocation errors for resampling (resampling function is particularly memory intense)

v 16: 23.September.2013 [JRH]
	- bugfix: normalization (1 or 2) now takes into account that the function doing the normalizing expects zero or 1

v 15: 7.May.2013 [JRH]
	- add transpose capability (-t)

v 14: 15.April.2013 [JRH]
	- Fisher's transform is now a separate step
	- option to apply Fisher's transform scaled to correlation values limited to a range of 0-1
	- limit of +-3.8 applied to Fisher's transform - this is done in the xf_fisherstransform1_d function

v 13: 6.April.2013 [JRH]
	- bugfix in xf_resample2_d
		- now allocates sufficient memory for new matrix based on maximum dimension in input or ourput

v 12: 18.March.2013 [JRH]
	- bugfix - rotation was not working properly
	- use new function xf_resample2_d for resampling the matrix

v 11: 12.March.2013 [JRH]
	- add ability to applyFishers-transform to data (to normalize Pearson's correlation coefficients)

v 10: 10.March.2013 [JRH]
	- use xf_matrixread1_d to read matrix and assign values to nrows, ncols etc

v 9: 4.March.2013 [JRH]
	- use newer smoothing function xf_smoothgauss2_d
		- assumes NAN or INF are invalid
		- no longer smooths NANs into adjacent bins!

v 7: 15.February.2013 [JRH]
	- use new matrixrotate function
	- now assumes input files are rectangular matrices (ie. no uneven rows) - an error arises otherwise
	- added width and height resampling option

v 6: 19.January.2013 [JRH]
	- allow resampling of each line to generate a fixed-width matrix from a variable-width matrix

v 5: 16.December.2012 [JRH]
	- now uses function _lineread1 to read lines of unknown length
	- also handles initialization and memory allocation for line
	- MAXLINELEN definition is unnecessary

v 4: 10.December.2012 [JRH]
	- remove unused "invalid value" setting
	- initialize maxlinelen to MAXLINELEN at start of program and pre-allocate memory

v 3: 19.November.2012 [JRH]
	- change print format of output from %f to %g, to better preserve precision

*/


/* external functions start */
double *xf_matrixread1_d(long *nmatrices, long *ncols, long *nrows, char *message, FILE *fpin);
void xf_norm2_d(double *data,long N,int normtype);
int xf_smoothgauss2_d(double *data,int xbintot,int ybintot,int xsmooth,int ysmooth);
double *xf_matrixrotate1_d(double *data1, long *nx1, long *ny1, int r);
int xf_matrixbin1_d(double *matrix1, long nx1, long ny1, long nx2, long ny2, char *message);
double *xf_matrixexpand1_d(double *matrix1, long nx1, long ny1, long nx2, long ny2, char *nessage);
void xf_fishertransform2_d(double *data, long n, int type);
double *xf_matrixtrans1_d(double *data1, long *width, long *height);
double *xf_matrixflipy_d(double *data1, long nx, long ny);
double *xf_matrixflipx_d(double *data1, long nx, long ny);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],*line=NULL,*templine=NULL,*pline,*pcol,message[256];
	long int ii,jj,kk;
	int v,w,x,y,z,n=0,col,colmatch,result_i[32];
	int sizeofchar=sizeof(char),sizeoflong=sizeof(long),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	int bintot=0,count=0;
	long N=0,N1=0,nmatrices;
	long width1,width2,height1,height2;
	double *matrix1=NULL,*matrix2=NULL;
	/* arguments */
	int setnorm=0,setfishers=0,setxsmooth=0,setysmooth=0,setkeepnans=0,setrotate=0,settrans=0,setflip=0;
	long setwidth=0,setheight=0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Modify a matrix:\n");
		fprintf(stderr,"	flip/rotate/transpose,resample,smooth,Fisher-transform,normalize - in that order\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file or \"stdin\" in matrix format\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-flip : flip matrix (0=NO, 1= x-flip, 2= y-flip) [%d]\n",setflip);
		fprintf(stderr,"	-t : transpose (0=NO, 1=YES - cannot be combined with rotation) [%d]\n",settrans);
		fprintf(stderr,"	-r : rotation, in degrees (choose 0,+-90,+-180,+-270) [%d]\n",setrotate);
		fprintf(stderr,"	-w : width, set (resample) number of columns [%d]\n",setwidth);
		fprintf(stderr,"	-h : height, set (resample) number of rows [%d]\n",setheight);
		fprintf(stderr,"		NOTE: expands or averages data in each row/column as needed\n");
		fprintf(stderr,"		NOTE: set to zero to leave as-is\n");
		fprintf(stderr,"	-sx : 2D gaussian smoothing factor to apply to matrix [%d]\n",setxsmooth);
		fprintf(stderr,"	-sy : 2D gaussian smoothing factor to apply to matrix [%d]\n",setysmooth);
		fprintf(stderr,"	-pn : preserve NANs when smoothing (0=NO 1=YES) [%d]\n",setkeepnans);
		fprintf(stderr,"	-n : normalization, 0=no, 1=0-1 range, 2=z-scores [%d]\n",setnorm);
		fprintf(stderr,"	-f : Fisher z' transformation : 0=no, 1 or 2 = yes [%d]\n",setfishers);
		fprintf(stderr,"			transform for r-values\n");
		fprintf(stderr,"			set to 1 of numbers range from -1 to 1\n");
		fprintf(stderr,"			set to 2 of numbers range from  0 to 1\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s matrix.txt -n 0 -sx 1 -sy 2 \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	An [width]x[height] modified matrix\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-flip")==0) setflip=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-t")==0)    settrans=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-r")==0)    setrotate=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-n")==0)    setnorm=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-f")==0)    setfishers=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-w")==0)    setwidth=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-h")==0)    setheight=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-sx")==0)   setxsmooth=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-sy")==0)   setysmooth=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-pn")==0)   setkeepnans=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	x=abs(setrotate);
	if(x!=0&&x!=90&&x!=180&&x!=270) {fprintf(stderr,"\n--- Error[%s]: invalid rotation setting (-r %d) \n\n",thisprog,setrotate);exit(1);}
	if(setnorm!=0&&setnorm!=1&&setnorm!=2) {fprintf(stderr,"\n--- Error[%s]: invalid normalization setting (-n %d) \n\n",thisprog,setnorm);exit(1);}
	if(setfishers!=0&&setfishers!=1&&setfishers!=2) {fprintf(stderr,"\n--- Error[%s]: invalid Fisher transform setting (-f %d) \n\n",thisprog,setfishers);exit(1);}
	if(settrans!=0&&settrans!=1) {fprintf(stderr,"\n--- Error[%s]: invalid transpose setting (-t %d) \n\n",thisprog,settrans);exit(1);}
	if(setkeepnans!=0&&setkeepnans!=1) {fprintf(stderr,"\n--- Error[%s]: invalid setting (-pn %d) \n\n",thisprog,setkeepnans);exit(1);}
	if(settrans==1 && x>0) {fprintf(stderr,"\n--- Error[%s]: cannot both rotate and transpose the input\n\n",thisprog);exit(1);}
	if(setflip<0 || setflip>2) {fprintf(stderr,"\n--- Error[%s]: invalid -flip (%d) must be 0-2\n\n",thisprog,setflip);exit(1);}


	/* STORE THE MATRIX -  THIS METHOD IS ROBUST AGAINST LINES OF UNKNOWN LENGTH */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n\a--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	matrix1= xf_matrixread1_d(&nmatrices,&width1,&height1,message,fpin);
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(matrix1==NULL) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
	N1= width1*height1;

	if(nmatrices>1) {fprintf(stderr,"\n--- Error[%s]: file %s contains more than one matrix - average before passing to this program\n\n",thisprog,infile);exit(1);}

	/* FLIP THE MATRIX  */
	if(setflip==1) matrix1= xf_matrixflipx_d(matrix1,width1,height1);
	if(setflip==2) matrix1= xf_matrixflipy_d(matrix1,width1,height1);


	/* TRANSPOSE THE MATRIX - this will also update the variables width1 and height1 */
	if(settrans==1) matrix1= xf_matrixtrans1_d(matrix1,&width1,&height1);
	if(matrix1==NULL) {fprintf(stderr,"\n--- Error[%s]: transpose function encountered insufficient memory\n\n",thisprog);exit(1);}

	/* ROTATE THE MATRIX - this will also update the variables width1 and height1 */
	if(setrotate!=0) matrix1= xf_matrixrotate1_d(matrix1,&width1,&height1,setrotate);
	if(matrix1==NULL) {fprintf(stderr,"\n--- Error[%s]: rotation function encountered insufficient memory\n\n",thisprog);exit(1);}


	/*
	RESAMPLE THE MATRIX
	- note that the bin and expand functions will complain if passed arguments indicating the wrong type of modification
	- hence we call the bin function first, and separately for width and height
	- then, the expansion function only sees the matrix which has already been binned in one dimension, if required
	*/
	if(setwidth>0 || setheight>0) {
		/* for auto settings, assign existing matrix size to setwidth and/or setheight */
		if(setwidth<=0) setwidth=width1;
		if(setheight<=0) setheight=height1;
		/* if matrix is to be downsampled (binned)... */
		if(setwidth<width1) {
			z= xf_matrixbin1_d(matrix1,width1,height1,setwidth,height1,message);
			if(z!=0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
			width1=setwidth;
			matrix1=(double *)realloc(matrix1,((width1*height1)*sizeof(double)));
		}
		if(setheight<height1) {
			z= xf_matrixbin1_d(matrix1,width1,height1,width1,setheight,message);
			if(z!=0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
			height1=setheight;
			matrix1=(double *)realloc(matrix1,((width1*height1)*sizeof(double)));
		}
		/* if matrix is to be expanded... */
		if(setwidth>width1 || setheight>height1) {
			matrix1= xf_matrixexpand1_d(matrix1,width1,height1,setwidth,setheight,message);
			if(matrix1==NULL) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
			if(setwidth>width1) width1=setwidth;
			if(setheight>height1) height1=setheight;
		}
		//else {fprintf(stderr,"\n--- Error[%s]: a single call to this program cannot mix matrix downsampling in one dimension with expansion in another: call this program twice instead\n\n",thisprog);exit(1);}
		N1=width1*height1;
	}

 	/* SMOOTH THE MATRIX */
  	if(setxsmooth>0 || setysmooth>0) {
		if(setkeepnans==1) {
			/* make a copy to store NAN's if present */
			matrix2= realloc(matrix2,((width1*height1)*sizeof(*matrix2)));
			for(ii=0;ii<N1;ii++) matrix2[ii]= matrix1[ii];
			/* apply smoothing */
			z=xf_smoothgauss2_d(matrix1,width1,height1,setxsmooth,setysmooth);
			if(z!=0) {fprintf(stderr,"\n--- Error[%s]: smoothing function encountered insufficient memory\n\n",thisprog);exit(1);}
			/* reinstate NAN's where required */
			for(ii=0;ii<N1;ii++) if(!isfinite(matrix2[ii])) matrix1[ii]=NAN;
		}
		else {
			z=xf_smoothgauss2_d(matrix1,width1,height1,setxsmooth,setysmooth);
			if(z!=0) {fprintf(stderr,"\n--- Error[%s]: smoothing function encountered insufficient memory\n\n",thisprog);exit(1);}
		}
  	}

  	/* APPLY A FISHER'S TRANSFORM FOR CORRELATION VALUES  */
  	if(setfishers==1 || setfishers==2)
		xf_fishertransform2_d(matrix1,N1,setfishers);

  	/* NORMALISE THE MATRIX */
  	if(setnorm==1 || setnorm==2)
		xf_norm2_d(matrix1,N1,(setnorm-1));

 	/* OUTPUT THE MATRIX */
	for(ii=0;ii<height1;ii++) {
		jj=ii*width1;
		kk=jj+width1;
		printf("%g",matrix1[jj++]);
		for(jj=jj;jj<kk;jj++) printf(" %g",matrix1[jj]); // only print a tab separator for columns after the first column
		printf("\n");
	}

	/* FREE MEMORY */
	if(matrix1!=NULL) free(matrix1);
	if(matrix2!=NULL) free(matrix2);

	exit(0);
	}
