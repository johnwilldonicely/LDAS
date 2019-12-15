#define thisprog "xe-csd1"
#define TITLE_STRING thisprog" v 1: 6.October.2014 [JRH]"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>signal_processing</TAGS>


*/


/* external functions start */
double *xf_matrixread1_d(long *nmatrices, long *ncols, long *nrows, char *message, FILE *fpin);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],message[256];
	long int i,j,k;
	size_t nn,mm;
	int v,w,x,y,z,n,m;
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	long index1,index2,nmatrices;
	long width,height;
	double *matrix1=NULL,*matrix2=NULL;
	/* arguments */
	double setspacing=0.1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Convert a voltage matrix of current source density\n");

		fprintf(stderr,"Formula: Ix = -s[ (Ex-h -2*Ex + Ex+h) / (4*h*h) ]\n") ;
		fprintf(stderr,"	Ix = current density at site x\n");
		fprintf(stderr,"	Ex is extracellular voltage at location x\n");
		fprintf(stderr,"	h is the sampling distance between recording sites\n");
		fprintf(stderr,"	s is an unknown constant - ignored\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file or \"stdin\" in matrix (time x depth) format\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-s : spacing between recording sites(mm) [%f]\n",setspacing);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s matrix.txt -s 0.01\n",thisprog);
		fprintf(stderr,"	cat matrix.txt | %s stdin -s 0.01\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	An time x depth CSD matrix\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-s")==0)    setspacing=atof(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}
	if(setspacing<=0.0) {fprintf(stderr,"\n--- Error[%s]: spacing (-s %g) must be greater than zero\n\n",thisprog,setspacing);exit(1);}


	/* STORE THE MATRIX -  THIS METHOD IS ROBUST AGAINST LINES OF UNKNOWN LENGTH */
	fprintf(stderr,"	Reading input %s...\n",infile);
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n\a--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	matrix1= xf_matrixread1_d(&nmatrices,&width,&height,message,fpin);
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(matrix1==NULL) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
	nn= width*height;

	/* MAKE SURE THIS IS NOT A MULTIMATRIX - MATRICES SEPARATED BY BLANK LINES OR COMMENTS  */
 	if(nmatrices>1) {fprintf(stderr,"\n--- Error[%s]: file %s contains more than one matrix - average before passing to this program\n\n",thisprog,infile);exit(1);}

 	/* EXPAND MEMORY FOR ORIGINAL MATRIX - ADD ENOUGH FOR TWO EXTRA ROWS */
 	mm=nn+(2*width);
 	if(  (matrix1=(double *)realloc(matrix1,mm*sizeof(double))) == NULL  ) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

  	/* ALLOCATE MEMORY FOR THE NEW CSD MATRIX */
  	if(  (matrix2=(double *)realloc(matrix2,nn*sizeof(double))) == NULL  ) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/* MOVE ENTIRE MATRIX DOWN A ROW IN INPUT MATRIX - DUPLICATES TOP ROW */
	for(i=(nn-1);i>=0;i--) matrix1[i+width]=matrix1[i];

	/* COPY BOTTOM ROW - START AT POSITION k = ORIGINAL HEIGHT + 1 ROW */
	j=height*width;
	k=(height+1.0)*width;
	for(i=j;i<k;i++) matrix1[i+width]=matrix1[i];

	/* PRE-CALCULATE THE DIVISOR FOR THE EQUATION - EXPRESSING IT AS A MULTIPLIER FOR SPEED */
	aa=(setspacing*setspacing)/4.0;

	/* CALCULATE THE CURRENT SOURCE DENSITY IN THE MATRIX */
	fprintf(stderr,"	Calculating current source density...\n");

fprintf(stderr,"mm=%ld\n",mm);//???
fprintf(stderr,"nn=%ld\n",nn);//???
fprintf(stderr,"width=%ld\n",width);//???
fprintf(stderr,"height=%ld\n",height);//???

	for(i=0;i<width;i++) {
		for(j=0;j<height;j++) {
			index1=((j+1)*width)+i;
			index2=(j*width)+i;
			/* calculate the CSD based on voltage at, above and below the current sample */
			matrix2[index2]= aa * ( (matrix1[(index1-width)]) - (2.0*matrix1[index1]) + (matrix1[(index1+width)]) );
	}}

 	/* OUTPUT THE MATRIX */
	fprintf(stderr,"	Outputting the CSD matrix...\n");
	for(i=0;i<height;i++) {
		j=i*width;
		k=j+width;
		printf("%g",matrix2[j++]);
		for(j=j;j<k;j++) printf(" %g",matrix2[j]); // only print a tab separator for columns after the first column
		printf("\n");
	}

	/* FREE MEMORY */
	if(matrix1!=NULL) free(matrix1);
	if(matrix2!=NULL) free(matrix2);

	exit(0);
}
