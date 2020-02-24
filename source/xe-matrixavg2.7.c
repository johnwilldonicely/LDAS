#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-matrixavg2"
#define TITLE_STRING thisprog" v 1: 22.January.2019 [JRH]"

/*
<TAGS>math dt.matrix noise</TAGS>

v 1: 8.September.2019 [JRH]
	- add row-normalization capabilities
	- simplify coding workflow 

v 1: 22.January.2019 [JRH]
	- add clipping capabilities

v 1: 3.January.2019 [JRH]
	- add de-noising cpabilities
	- large update of code, based on matrixavg1

v 7: 25.June.2014 [JRH]
	- now reports error if unequal numbers of columns or rows are found in a given matrix, relative to the first (reference) matrix

v 6: 1.March.2013 [JRH]
	- use new xf_matrixread1_d function load multi-matrices into memory
	- use xf_matrixavg1 function to calculate the average
	- output now ensures no additional delimiters at the end of each line

v 5: 8.February.2013 [JRH]
	- corrected instructions
	- bugfix: now pre-initializes variable "line" to MAXLINELEN elements

v 4: 16.December.2012 [JRH]
	- now uses function _readline1 to read lines of unknown length

v 3: 28.November.2012 [JRH]
	- relax column-check function - now will correct for any number of missing/extra columns, provided number of lines is correct

v 2: 22.November.2012 [JRH]
	- bugfix: now resets line character counter if a comment line is encountered, and sets prevblank to 1
	- now checks number of columns on each row - if different by less than 1 from the reference matrix (the first) then it adjusts accordingly:
		- one extra column on any given row will be dropped
		- one less column on any given row will result in generation of a dummy column (NAN)
		- if the column-number difference on any row is >1 or <-1, an error results
	- if any matrix has fewer or more LINES than the reference, an error is generated

v 1: 13.November.2012 [JRH]
	- bugfix - averaging was not being performed correctly - failed to initialize sum and ngood variables to zero
*/


/* external functions start */
double *xf_matrixread1_d(long *nmatrices, long *ncols, long *nrows, char *message, FILE *fpin);
long xf_spectdenoise1_d(double *matrix1,long width,long height,double setclip,double setz,int setsign,double setper,int setrotate,char *message);
int xf_matrixrotate2_d(double *data1, long *width, long *height, int r);
double *xf_matrixavg1_d(double *multimatrix, long n_matrices, long bintot, char message[]);
double *xf_matrixrotate1_d(double *data1, long *width, long *height, int r);
long xf_norm3_d(double *data,long ndata,int normtype,long start,long stop,char *message);
long xf_interp3_d(double *data, long ndata);
int xf_filter_bworth_matrix1_d(double *matrix1, size_t width, size_t height, float sample_freq, float low_freq, float high_freq, float res, char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *line=NULL,*templine=NULL,*pline,*pcol;
	long int ii,jj,kk,mm,nn,row,col;
	int v,w,x,y,z,colmatch,result_i[16];
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char message[256];
	long n1,nrows1,ncols1,nmatrices1,bintot1;
	double *data1=NULL,*pmatrix=NULL,*pmatrix2=NULL,*mean1=NULL;
	/* arguments */
	char *infile1=NULL;
	int setsign=0,setrotate=0,setnorm=-1;
	long setn1=-1,setn2=-1;
	float setflo=0.0,setfhi=0.0,setfsr=1.0;
	double setclip=-1.0,setz=NAN,setp=25.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Average multiple matrices separated by blank-lines or comments\n");
		fprintf(stderr,"- first matrix defines matrix format (rows & columns)\n");
		fprintf(stderr,"- all matrices must have the same number of rows and columns\n");
		fprintf(stderr,"- non-numeric values will not contribute to the mean\n");
		fprintf(stderr,"- includes option to de-noise and filter the matrix, in that order\n");
		fprintf(stderr,"	- these assume columns=time (but see -r option below)\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE: %s [infile] [options]\n",thisprog);
		fprintf(stderr,"	[infile]: file (or \"stdin\") containing data matrix/matrices\n");
		fprintf(stderr,"		- format: space-delimited numbers in columns and rows\n");
		fprintf(stderr,"		- matrix separator= blank or lines beginning with \"#\"\n");
		fprintf(stderr,"		- missing values require placeholders (NAN, \"-\", etc.)\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"DE-NOISING OPTIONS aplied to each matrix: defaults in []\n");
		fprintf(stderr,"	-z: Z-score threshold for noise at each freq (NAN=skip) [%g]\n",setz);
		fprintf(stderr,"	-c: Z-score clipping-value, to avoid outliers (-1=noclip) [%g]\n",setclip);
		fprintf(stderr,"	-s: sign of thesholding (-1=NEG,+1=POS,0=BOTH) [%d]\n",setsign);
		fprintf(stderr,"	-p: %% of freq > z needed to invalidate timepoint [%g]\n",setp);
		fprintf(stderr,"		- e.g. Z>3 for 25%% of the spectrum at column 872\n");
		fprintf(stderr,"	-r: 90-deg rotate for analysis (0=NO 1=YES) [%d]\n",setrotate);
		fprintf(stderr,"		- use for if input column=freq and row=time\n");
		fprintf(stderr,"		- matrix will be rotated back for output\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"NORMALIZATION OPTIONS (applied to rows): defaults in []\n");
		fprintf(stderr,"	-norm: normalization type: [%d]\n",setnorm);
		fprintf(stderr,"		-1: no normalization \n");
		fprintf(stderr,"		 0: 0-1 range\n");
		fprintf(stderr,"		 1: z-scores (see -n1/-n2)\n");
		fprintf(stderr,"		 2: difference from first valid sample (see -n1)\n");
		fprintf(stderr,"		 3: difference from mean (see -n1/-n2) \n");
		fprintf(stderr,"		 4: ratio of mean (see -n1/-n2)\n");
		fprintf(stderr,"	-n1: start of normalization zone (samples) [%ld]\n",setn1);
		fprintf(stderr,"	-n2: end of normalization zone (sample) [%ld]\n",setn2);
		fprintf(stderr,"		- set n1|n2 to -1 to signify first|last valid sample\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"FILTER OPTIONS (applied to rows): defaults in []\n");
		fprintf(stderr,"	-fsr: sample-rate (Hz) [%g]\n",setfsr);
		fprintf(stderr,"	-flo: low-frequency cut (0=NONE) [%g]\n",setflo);
		fprintf(stderr,"	-fhi: high-frequency cut (0=NONE) [%g]\n",setfhi);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s matrix.txt\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	A single average of the individual matrices\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile1= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-c")==0)    setclip= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-z")==0)    setz=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-s")==0)    setsign=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-p")==0)    setp=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-r")==0)    setrotate=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-norm")==0) setnorm= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-n1")==0)   setn1= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-n2")==0)   setn2= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-fsr")==0)  setfsr=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-flo")==0)  setflo=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-fhi")==0)  setfhi=atof(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setsign<-1||setsign>1) {fprintf(stderr,"\n--- Error[%s]: invalid -s [%d] must be -1, 0 or 1\n\n",thisprog,setsign);exit(1);}
	if(setrotate!=0&&setrotate!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -r [%d] must be 0 or 1\n\n",thisprog,setrotate);exit(1);}
	if(setz==0.0) {fprintf(stderr,"\n--- Error[%s]: invalid -z [%g], cannot be zero\n\n",thisprog,setz);exit(1);}
	if(setp<=0.0) {fprintf(stderr,"\n--- Error[%s]: invalid -per [%g], must be >0\n\n",thisprog,setp);exit(1);}
	if(setnorm<-1||setnorm>4) {fprintf(stderr,"\n--- Error[%s]: -norm (%d) must be -1 or 0-4\n\n",thisprog,setnorm);exit(1);}
	if(setfsr<=0.0) {fprintf(stderr,"\n--- Error[%s]: invalid -fsr [%g], must be >0\n\n",thisprog,setfsr);exit(1);}
	if(setflo<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid -flo [%g], must be >=0\n\n",thisprog,setflo);exit(1);}
	if(setfhi<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid -fhi [%g], must be >=0\n\n",thisprog,setfhi);exit(1);}

	/* STORE MULTI-MATRIX DATA #1  */
	n1=nrows1=ncols1=nmatrices1=0;
	if(strcmp(infile1,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile1,"r"))==0) {fprintf(stderr,"\n\a--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile1);exit(1);}
	data1= xf_matrixread1_d(&nmatrices1,&ncols1,&nrows1,message,fpin);
	if(strcmp(infile1,"stdin")!=0) fclose(fpin);
	if(data1==NULL) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

	/* GET SOME PARAMETERS */
	bintot1= nrows1*ncols1;
	n1= bintot1*nmatrices1;
	bb=0.00;
	mm=0;

	for(ii=0;ii<nmatrices1;ii++) {
		pmatrix= data1+(ii*bintot1);

		if(setrotate==1) {z= xf_matrixrotate2_d(pmatrix,&ncols1,&nrows1,-90); if(z<0){ fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }}

		/* APPLY DE-NOISING IF REQUIRED */
		if(isfinite(setz)) {
			mm= xf_spectdenoise1_d(pmatrix,ncols1,nrows1,setclip,setz,setsign,setp,0,message);
			if(mm==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
			bb= 100.0*(double)mm/(double)ncols1;
			fprintf(stderr,"matrix= %ld sd= %g per= %g abs= %d rotate= %d noise= %.2f %% count=%ld\n",(ii+1),setz,setp,setsign,setrotate,bb,mm);
		}

		/* APPLY INTERPOLATION-IN-TIME AS REQUIRED FOR FILTERING OR NORMALIZATION */
		if(setflo>0.0 || setfhi>0.0 || setnorm>=0) {
			for(jj=0;jj<nrows1;jj++) kk= xf_interp3_d(pmatrix+(jj*ncols1),ncols1);
		}

		/* APPLY NORMALIZATION ON EACH ROW */
		if(setnorm>=0) {
			for(jj=0;jj<nrows1;jj++) {
				pmatrix2=data1+(ii*bintot1+jj*ncols1);
				kk= xf_norm3_d(pmatrix2,ncols1,setnorm,setn1,setn2,message);
				if(kk==-2) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
				if(ii==-1) {
					fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message);
					for(kk=0;kk<ncols1;kk++) pmatrix2[kk]= NAN;
		}}}

		/* APPLY FILTERING IF REQUIRED */
		if(setflo>0.0 || setfhi>0.0) {
			z= xf_filter_bworth_matrix1_d(pmatrix,ncols1,nrows1,setfsr,setflo,setfhi,1.4142,message);
			if(z!=0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		}


		if(setrotate==1) {z= xf_matrixrotate2_d(pmatrix,&ncols1,&nrows1,90); if(z<0){ fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }}
	}



	/* CALCULATE THE AVERAGE, EXCLUDING NANs */
	mean1= xf_matrixavg1_d(data1,nmatrices1,bintot1,message);
	if(mean1==NULL) { fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message); exit(1);}
	n1=nrows1*ncols1;

	/* OUTPUT THE RESULTING AVERAGE MATRIX */
	for(ii=jj=0;ii<n1;ii++) { printf("%g",mean1[ii]); if(++jj<ncols1) printf(" "); else { jj=0;printf("\n"); } }

	/* CLEANUP AND EXIT */
	if(line!=NULL) free(line);
	if(data1!=NULL) free(data1);
	if(mean1!=NULL) free(mean1);
	exit(0);
}
