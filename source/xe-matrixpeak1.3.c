#define thisprog "xe-matrixpeak1"
#define TITLE_STRING thisprog" v 3: 15.February.2019 [JRH]"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>matrix</TAGS>

v 3: 15.February.2019 [JRH]
	- update variable usage, assign input filename directly from argv[1]

v 3: 21.October.2017 [JRH]
	- now able to handle multi-matrix files, and outputs peak-mask

v 3: 23.February.2014 [JRH]
	- -s option added to set minimum size for detection

v 2: 20.February.2014 [JRH]
	- update to xf_matrixpeak function - nw reports centroid and mean value of pixels in peak-zone
*/


/* external functions start */
long xf_matrixread2_d(char *infile, long idcol, double **matrix1, double **id1, long *ncols, long *nrows, char *message);
int xf_matrixpeak1_d(double *data1,int *mask, size_t width, size_t height, double thresh, double *result);
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *line=NULL,*templine=NULL,*pline,*pcol,message[256];
	long ii,jj,kk,nn;
	int v,w,x,y,z,n=0,col,colmatch,result_i[32];
	int sizeofint=sizeof(int),sizeoflong=sizeof(long),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[8];
	FILE *fpin,*fpout;
	/* program-specific variables */
	int *mask;
	long nmatrices,width,height,count=0,peakfound,npeaks;
	double *matrix1=NULL,*id1=NULL,*pmatrix,max=0,thresh;
	/* arguments */
	char *infile;
	int setfirst=1,setout=1;
	long setsize=0,setidcol=1;

	double setthresh=.25;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Detect contiguous pixels exceeding a threshold in individual matrices \n");
		fprintf(stderr," - this peak starts at the highest-value pixel and propogates outward\n");
		fprintf(stderr," - diagonal propogation is not permitted\n");
		fprintf(stderr," - if peak size is below a minimum, another attempt is made\n");
		fprintf(stderr," - attempts are made until no peak is detected at all\n");
		fprintf(stderr,"USAGE: %s [matrix] [options]\n",thisprog);
		fprintf(stderr,"	[matrix]: file or \"stdin\" in (multi)matrix format\n");
		fprintf(stderr,"		- matrices separated by \"# <id-number>\" lines\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-idcol: the column on comment-lines holding the ID [%ld]\n",setidcol);
		fprintf(stderr,"	-thresh : edge-detection threshold (proportion of peak) [%g]\n",setthresh);
		fprintf(stderr,"	-size   : size threshold (number of pixels, zero=any) [%d]\n",setsize);
		fprintf(stderr,"	-first  : output first good peak only (0=NO 1=YES) [%d]\n",setfirst);
		fprintf(stderr,"	-out    : output [%d]\n",setout);
		fprintf(stderr,"		1: peak statistics are sent to stderr\n");
		fprintf(stderr,"		2: the mask sent to stdout\n");
		fprintf(stderr,"		3: both\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	{ %s matrix.txt -thresh 1 -first 0 > matrix2.txt ; } 2> report.txt \n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile=argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-idcol")==0)  setidcol= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-thresh")==0) setthresh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-size")==0)   setsize=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-first")==0)  setfirst=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)    setout=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setsize<0) {fprintf(stderr,"\n--- Error[%s]: -s (%ld) must be zero or greater\\n\n",thisprog,setsize); exit(1);}
	if(setthresh<0||setthresh>1) {fprintf(stderr,"\n--- Error[%s]: -t (%g) must be 0-1\\n\n",thisprog,setthresh); exit(1);}

	/********************************************************************************/
	/* STORE THE MATRIX -  THIS METHOD IS ROBUST AGAINST LINES OF UNKNOWN LENGTH */
	/********************************************************************************/
	/* call the read function */
	nmatrices= xf_matrixread2_d(infile,setidcol,&matrix1,&id1,&width,&height,message);
	if(nmatrices==-1) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
	if(nmatrices==0)  {fprintf(stderr,"\n--- Error[%s]: file %s contains no matrices\n\n",thisprog,infile);exit(1);}

	/* allocate memory for the mask */
	nn= width*height;
	mask= malloc(nn*sizeof(*mask));
	if(mask==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	/* output the basic stats */
	if(setout==1 || setout==3) {
		fprintf(stderr,"nmatrices= %ld\n",nmatrices);
		fprintf(stderr,"matrixwidth= %ld\n",width);
		fprintf(stderr,"matrixheight= %ld\n",height);
	}

	/********************************************************************************/
	/* FOR EACH MATRIX */
	/********************************************************************************/
	for(kk=0;kk<nmatrices;kk++) {
		/* set the matrix pointer */
		pmatrix= matrix1+(nn*kk);
		/* get the max value and update setthresh */
		max=0; for(ii=0;ii<nn;ii++) { aa= pmatrix[ii]; if(isfinite(aa) && aa>max) max=aa; }
		thresh= max*setthresh;
		/* initialize the mask */
		for(ii=0;ii<nn;ii++) mask[ii]=0;
		/* output the basic stats */
		if(setout==1 || setout==3) {
			fprintf(stderr,"--------------------------------------------------------------------------------\n");
			fprintf(stderr,"matrixid= %g\n",id1[kk]);
			fprintf(stderr,"max= %g\n",max);
			fprintf(stderr,"peakthresh= %.3f\n",thresh);
		}


		/********************************************************************************/
		/* NOW DETECT THE PEAKS USING THE FIXED THRESHOLD */
		/********************************************************************************/
		npeaks=peakfound=0;
		while(1) {
			/* CALL THE PEAK DETECTION FUNCTION */
			peakfound= xf_matrixpeak1_d(pmatrix,mask,width,height,thresh,result_d);
			/* if no peak was detected, stop here */
			if(peakfound==0) break;
			/* if peak found was below size threshold, mask it and try again */
			else if(peakfound>=0 && result_d[0]<setsize) { for(ii=0;ii<nn;ii++) if(mask[ii]>0) mask[ii]=-1;	continue; }
			/* so this is a good peak - increment the counter & carry on... */
			npeaks++;

			/* OUTPUT THE SUMMARY STATISTICS TO STDERR */
			if(setout==1 || setout==3) {
				fprintf(stderr,"\n");
				fprintf(stderr,"peaknumber %ld\n",npeaks);
				fprintf(stderr,"pixels %g\n",result_d[0]);
				fprintf(stderr,"peak_x %g\n",result_d[1]);
				fprintf(stderr,"peak_y %g\n",result_d[2]);
				fprintf(stderr,"centroid_x %g\n",result_d[3]);
				fprintf(stderr,"centroid_y %g\n",result_d[4]);
				fprintf(stderr,"mean %g\n",result_d[5]);
				fprintf(stderr,"max %g\n",result_d[6]);
			}

			/* OUTPUT THE MASK INDICATING THE PEAK - CHANGE -1 TO NAN */
			if(setout==2 || setout==3) {
		 		count=0;
				printf("# %g	%ld\n",id1[kk],npeaks);
				for(ii=0;ii<nn;ii++) {
					/* if pixel was detected as part of peak, output and set mask to exclude re-detection */
					if(mask[ii]>=0) printf("%d\t",mask[ii]);
					/* otherwise output nan - this ixel may be detected in subsequent calls to xf_matrixpeak */
					else printf("nan ");
					/* counter - to tell when a newline should be printed */
					if(++count>=width) {count=0; printf("\n");}
				}
			}

			/* IF ANOTHER ITERATION IS REQUIRED, REMOVE CURRENT FIELD FROM THE MASK */
			if(setfirst==0) {for(ii=0;ii<nn;ii++) { if(mask[ii]>0) mask[ii]=-1; }}
			else break;

		} // END OF PER-PEAK WHILE CONDITION

		/* REPORT THE TOTAL PEAKS DETECTED */
		if(setout==1 || setout==3) fprintf(stderr,"\nnpeaks %d\n",npeaks);

	} // END OF PER-MATRICX LOOP

	/* FREE MEMORY AND EXIT */
	if(matrix1!=NULL) free(matrix1);
	if(id1!=NULL) free(id1);
	if(mask!=NULL) free(mask);
	exit(0);

	}
