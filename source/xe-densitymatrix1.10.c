#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#define thisprog "xe-densitymatrix1"
#define TITLE_STRING thisprog" v 10: 23.October.2018 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>signal_processing</TAGS>


TEST DATA GENERATION:
	for i in $(seq 0 9) ; do { for j in $(seq 0 .5 2) ; do { x=$(echo $i $j | awk '{print $1+$2}') ; echo $i $j $x ; } done  ; } done > jj1
	cat jj1 jj1 > jj2

v 10: 23.October.2018 [JRH]
	- corrected uninitialized variables

v 10: 19.January.2016 [JRH]
	- add ability to output mean for 3-column input, not just the sum (-f 3)
	- add ability to autoscale matrix bins to the number of unique elements in the <x> or <y> columns

v 10: 8.March.2013 [JRH]
	- use new  xf_smoothgauss2_d function which assumes NAN to represent missing values

v 9: 6.November.2012 [JRH]
	- bugfix: no longer allows x or y data to go beyond user-set min and max
	- bugfix: error reported when n=0 (empty or non-numeric input, or min/max criteria are too strict)

v 8: 16.August.2012 [JRH]
	- allow use of a third column to hold a value to be summed, rather than simply counting lines with a given x/y

v 1.7: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."

v 1.6: 31.July.2012 [JRH]
	- bugfix: previously output upside-down matrices (y=0 comes out on top, of course)
		this was counter-intuitive so now the matrix is output in reverse
		-yflip option added to output matrix inverted, if required

	- bugfix: changed calculation of inverse-binwidth to 0.999999999/binwidth - to correct for error in double-precision calculations
		- this occassionally led to bin-numbers being out-of range, memory corruption and segmentation faults when x=xmax or y=ymax

	- bugfix: now will ignore lines where x or y are undefined, non-numerical, NAN or INF

v 1.5: 25.March.2012 [JRH]
	- bug-fix: previous versions did not recognize option to normalize data to range of 0-1
*/


/* external functions start */
void xf_norm1_d(double *data,long N,int normtype);
int xf_smoothgauss2_d(double *data,int xbintot,int ybintot,int xsmooth,int ysmooth);
int xf_compare1_d(const void *a, const void *b);
/* external functions end */

int main(int argc, char *argv[]) {

	char infile[256],temp_str[MAXLINELEN],line[MAXLINELEN];
	long i,j,k,n=0,*matrixcount=NULL;
	int x,y,z,matrixsize,sizeofdouble=sizeof(double);
	float a,b,c;
	double aa,bb,cc,dd;
	double *xdata=NULL,*ydata=NULL,*zdata=NULL,*matrix=NULL,*listx=NULL,*listy=NULL;
	double xmin,xmin2,xmax,xmax2,xrange,ymin,ymin2,ymax,ymax2,yrange;
	double xbinwidth,ybinwidth,xbinwidth_inv,ybinwidth_inv;
	/* arguments */
	int setnorm=-1,setyflip=0,setxsmooth=0,setysmooth=0;
	int setxmin=0,setxmax=0,setymin=0,setymax=0,setformat=1;
	long setxbintot=-1,setybintot=-1;
	FILE *fpin;

	xmin2= ymin2= -DBL_MAX;
	xmax2= ymax2=  DBL_MAX;

	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Create a density matrix from 2-column (x,y) or 3-column (x,y,z) input\n");
		fprintf(stderr,"USAGE: %s [input] [options] \n",thisprog);
		fprintf(stderr,"	[input]: input file or \"stdin\")\n");
		fprintf(stderr,"VALID OPTIONS (defaulst in []):\n");
		fprintf(stderr,"	-f : format of input: 1=<x><y>  2=<x><y><z> [%d]\n",setformat);
		fprintf(stderr,"		1:	density= total lines of a given <x><y>\n");
		fprintf(stderr,"			missing x/y values result in zero\n");
		fprintf(stderr,"		2:	density= sum of <z> for a given <x><y>\n");
		fprintf(stderr,"			missing x/y values result in NAN\n");
		fprintf(stderr,"		3:	density= mean of <z> for a given <x><y>\n");
		fprintf(stderr,"			missing x/y values result in NAN\n");
		fprintf(stderr,"	-x : matrix width (bins, 0=AUTO) [%ld]\n",setxbintot);
		fprintf(stderr,"	-y : matrix height (bins, 0=AUTO) [%ld]\n",setybintot);
		fprintf(stderr,"		NOTE: setting -x or -y to 0 sets width or height to\n");
		fprintf(stderr,"		the number of unique elements in the <x> or <y> columns\n");
		fprintf(stderr,"	-n : normalize data: -1=no, 0=0-1 range, 1=z-scores[%d]\n",setnorm);
		fprintf(stderr,"	-sx : 2D gaussian smoothing factor to apply to matrix [%d]\n",setxsmooth);
		fprintf(stderr,"	-sy : 2D gaussian smoothing factor to apply to matrix [%d]\n",setysmooth);
		fprintf(stderr,"	-xmin : force matrix to use this as the x-minimum [unset]\n");
		fprintf(stderr,"	-ymin : force matrix to use this as the y-minimum [unset]\n");
		fprintf(stderr,"	-xmax : force matrix to use this as the x-maximum [unset]\n");
		fprintf(stderr,"	-ymax : force matrix to use this as the y-maximum [unset]\n");
		fprintf(stderr,"	-yflip : flip matrix 0,0=top-left (0=NO, 1=YES) [%d]\n",setyflip);
		fprintf(stderr," - EXAMPLE: %s temp.txt -x 100 -y 25 -p 1 -s 0\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* ASSIGN ARGUMENTS TO VARIABLES */
	strcpy(infile,argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-f")==0)     setformat= atoi(argv[++i]);
			else if(strcmp(argv[i],"-x")==0)     setxbintot= atol(argv[++i]);
			else if(strcmp(argv[i],"-y")==0)     setybintot= atol(argv[++i]);
			else if(strcmp(argv[i],"-n")==0)     setnorm= atoi(argv[++i]);
			else if(strcmp(argv[i],"-sx")==0)    setxsmooth= atoi(argv[++i]);
			else if(strcmp(argv[i],"-sy")==0)    setysmooth= atoi(argv[++i]);
			else if(strcmp(argv[i],"-xmin")==0)  xmin2= atof(argv[++i]);
			else if(strcmp(argv[i],"-xmax")==0)  xmax2= atof(argv[++i]);
			else if(strcmp(argv[i],"-ymin")==0)  ymin2= atof(argv[++i]);
			else if(strcmp(argv[i],"-ymax")==0)  ymax2= atof(argv[++i]);
			else if(strcmp(argv[i],"-yflip")==0) setyflip= atoi(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(setyflip!=0&&setyflip!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -yflip (%d) - must be 0 or 1\n\n",thisprog,setyflip);exit(1);}
	if(setformat<1||setformat>3) {fprintf(stderr,"\n--- Error[%s]: invalid -f (%d) - must be 1 2 or 3\n\n",thisprog,setformat);exit(1);}
	if(xmin2>=xmax2) {fprintf(stderr,"\n--- Error[%s]: -xmin (%g) must be less than -xmax (%g)\n\n",thisprog,xmin2,xmax2);exit(1);}
	if(ymin2>=ymax2) {fprintf(stderr,"\n--- Error[%s]: -ymin (%g) must be less than -ymax (%g)\n\n",thisprog,ymin2,ymax2);exit(1);}


	/* STORE DATA */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	if(setformat==1) {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%lf %lf",&aa,&bb)!=2) continue;
			if(isfinite(aa) && isfinite(bb)) {
				if(aa<xmin2) continue;
				if(aa>xmax2) continue;
				if(bb<ymin2) continue;
				if(bb>ymax2) continue;
				xdata=(double *)realloc(xdata,(n+1)*sizeofdouble); if(xdata==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				ydata=(double *)realloc(ydata,(n+1)*sizeofdouble); if(ydata==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				xdata[n]=aa;
				ydata[n]=bb;
				n++;
	}}}
	if(setformat==2 || setformat==3) {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%lf %lf %lf",&aa,&bb,&cc)!=3) continue;
			if(isfinite(aa) && isfinite(bb) && isfinite(cc)) {
				if(aa<xmin2) continue;
				if(aa>xmax2) continue;
				if(bb<ymin2) continue;
				if(bb>ymax2) continue;
				xdata=(double *)realloc(xdata,(n+1)*sizeofdouble); if(xdata==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				ydata=(double *)realloc(ydata,(n+1)*sizeofdouble); if(ydata==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				zdata=(double *)realloc(zdata,(n+1)*sizeofdouble); if(zdata==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				xdata[n]=aa;
				ydata[n]=bb;
				zdata[n]=cc;
				n++;
	}}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(n<1) {fprintf(stderr,"\n--- Error[%s]: input (%s) is empty, non-numeric, or fails to meet min/max criteria\n\n",thisprog,infile);exit(1);}

	//for(i=0;i<n;i++) printf("%d	%g	%g\n",i,xdata[i],ydata[i]);
	//for(i=0;i<n;i++) printf("%d	%g	%g	%g\n",i,xdata[i],ydata[i],zdata[i]);

	/* AUTO-DETERMINE WIDTH AND HEIGHT OF MATRIX */
	if(setxbintot<=0) {
		if((listx=(double *)realloc(listx,n*sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(i=0;i<n;i++) listx[i]=xdata[i];
		qsort(listx,n,sizeof(double),xf_compare1_d);
		aa=listx[0]; for(i=j=1;i<n;i++) {if(listx[i]!=aa) listx[j++]=listx[i];aa=listx[i]; }
		setxbintot=j;
		xmin=listx[0];
		xmax=listx[j-1];
		free(listx);
	}
	else { xmin=xmax=xdata[0]; for(i=0;i<n;i++) {if(xdata[i]<xmin) xmin=xdata[i];if(xdata[i]>xmax) xmax=xdata[i];}}
	if(setybintot<=0) {
		if((listy=(double *)realloc(listy,n*sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(i=0;i<n;i++) listy[i]=ydata[i];
		qsort(listy,n,sizeof(double),xf_compare1_d);
		aa=listy[0]; for(i=j=1;i<n;i++) {if(listy[i]!=aa) listy[j++]=listy[i];aa=listy[i]; }
		setybintot=j;
		ymin=listy[0];
		ymax=listy[j-1];
		free(listy);
	}
	else { ymin=ymax=ydata[0]; for(i=0;i<n;i++) {if(ydata[i]<ymin) ymin=ydata[i];if(ydata[i]>ymax) ymax=ydata[i];}}

	/* DETERMINE RANGE & BIN WIDTHS */
	xrange=xmax-xmin;
	yrange=ymax-ymin;
	xbinwidth = xrange/(double)setxbintot;
	ybinwidth = yrange/(double)setybintot;
	xbinwidth_inv = 1.0/nextafter(xbinwidth,DBL_MAX);
	ybinwidth_inv = 1.0/nextafter(ybinwidth,DBL_MAX);

	/* INITIALISE MATRIX */
	matrixsize=setxbintot*setybintot;
	matrix = (double *) realloc(matrix,(matrixsize+1)*sizeof(double)); if(matrix==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	matrixcount = (long *) realloc(matrixcount,(matrixsize+1)*sizeof(long)); if(matrixcount==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	for(j=0;j<matrixsize;j++) { matrix[j]=0.0; matrixcount[j]=0; }

	/* BUILD THE MATRIX */
	if(setformat==1) {
		for(i=0;i<n;i++) {
			x=(int)((xdata[i]-xmin)*xbinwidth_inv);
			y=(int)((ydata[i]-ymin)*ybinwidth_inv);
			j=y*setxbintot+x;
			matrix[j]++;
	}}
	if(setformat==2||setformat==3) {
		for(i=0;i<n;i++) {
			x=(int)((xdata[i]-xmin)*xbinwidth_inv);
			y=(int)((ydata[i]-ymin)*ybinwidth_inv);
			j=y*setxbintot+x;
			matrixcount[j]++;
			matrix[j]+=zdata[i];

			/* printf("%d		%g	%d			%g	%g	%g\n",j,matrix[j],matrixcount[j],xdata[i],ydata[i],zdata[i]);  */
	}}

	/* FOR MEANS-OUTPUT, CORRECT MATRIC BY BIN-COUNT */
	if(setformat==3) {
		k=setybintot*setxbintot;
		for(i=0;i<k;i++) {
			if(matrixcount[i]>0) matrix[i]/=matrixcount[i];
			else matrix[i]=NAN;
		}
	}

	/* SMOOTH IF REQUIRED */
	if(setxsmooth>0 || setysmooth>0) {
		z= xf_smoothgauss2_d(matrix,setxbintot,setybintot,setxsmooth,setysmooth);
		if(z!=0) {fprintf(stderr,"\n--- Error[%s]: xf_smoothgauss2_d function encountered insufficient memory\n\n",thisprog);exit(1);}
	}
	/* NORMALISE IF REQUIRED */
	if(setnorm==0 || setnorm==1) xf_norm1_d(matrix,(long)matrixsize,setnorm);

	/* PRINT RESULTS TO STDOUT - INVERT BY DEFAULT SO 0,0 = BOTTOM LEFT, UNLESS -yflip WAS SET TO 1 */
	if(setyflip==0) {
		for(y=setybintot-1;y>=0;y--) {
			for(x=0;x<setxbintot;x++) {
			printf("%f\t",matrix[y*setxbintot+x]);
				}
			printf("\n");
		}
	}
	if(setyflip==1) {
		for(y=0;y<setybintot;y++) {
			for(x=0;x<setxbintot;x++) {
				printf("%f\t",matrix[y*setxbintot+x]);
			}
			printf("\n");
		}
	}

	free(xdata);
	free(ydata);
	if(zdata!=NULL) free(zdata);
	if(matrix!=NULL) free(matrix);
	if(matrixcount!=NULL) free(matrixcount);
}
