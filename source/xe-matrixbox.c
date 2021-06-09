#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-matrixbox"
#define TITLE_STRING thisprog" 9.June.2021 [JRH]"

/*
<TAGS>math dt.matrix stats </TAGS>

9.June.2021 [JRH] - first version

*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
double *xf_matrixread3_d(FILE *fpin, long *ncols, long *nrows, char *header, char *message);
double xf_matrixbox1_d(double *matrix1, long nx, long ny, double *range, double *box, char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *line=NULL,*templine=NULL,*pline,*pcol;
	long int ii,jj,kk,mm,nn,row,col;
	int v,w,x,y,z,colmatch,result_i[16],status=0;
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char header[256],message[256];
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
		fprintf(stderr,"Exract stats for square regions in a matrix (or matrices)\n");
		fprintf(stderr,"- first matrix defines matrix format (rows & columns)\n");
		fprintf(stderr,"- all matrices must have the same number of rows and columns\n");
		fprintf(stderr,"- non-numeric values will not contribute to the mean\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE: %s [infile] [options]\n",thisprog);
		fprintf(stderr,"    [infile]: file (or \"stdin\") containing data matrix/matrices\n");
		fprintf(stderr,"        - format: space-delimited numbers in columns and rows\n");
		fprintf(stderr,"        - matrix separator= blank or lines beginning with \"#\"\n");
		fprintf(stderr,"        - missing values require placeholders (NAN, \"-\", etc.)\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
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
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setsign<-1||setsign>1) {fprintf(stderr,"\n--- Error[%s]: invalid -s [%d] must be -1, 0 or 1\n\n",thisprog,setsign);exit(1);}

	/* STORE MULTI-MATRIX DATA #1  */
	n1=nrows1=ncols1=nmatrices1=0;
	if(strcmp(infile1,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile1,"r"))==0) {fprintf(stderr,"\n\a--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile1);exit(1);}

	/* LOOP: READ AND PROCESS EACH MATRIX UNTIL NONE IS FOUND */

double range[4];
double box[4];
// define matrix real-value ranges
range[0]= 0.5; // x-min
range[1]= 20; // x-max
range[2]= 30; // y-min
range[3]= 130; // y-max

// define box dimensions
box[0]= 4; box[1]= 10; box[2]= 30; box[3]= 50;


	while(1) {

		data1= xf_matrixread3_d(fpin,&ncols1,&nrows1,header,message);
		if(ncols1<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); status=1; goto END; }
		if(data1==NULL) break;
		printf("header: %s",header); // header ends in a newline

		aa= xf_matrixbox1_d(data1,ncols1,nrows1,range,box,message);
		if(!isfinite(aa))  { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); exit(1); }
		printf("result= %f\n",aa);


	}

	/* OUTPUT THE RESULTING AVERAGE MATRIX */
	//for(ii=jj=0;ii<n1;ii++) { printf("%g",mean1[ii]); if(++jj<ncols1) printf(" "); else { jj=0;printf("\n"); } }

END:
	/* CLEANUP AND EXIT */
	if(strcmp(infile1,"stdin")!=0) fclose(fpin);
	if(line!=NULL) free(line);
	if(data1!=NULL) free(data1);
	if(mean1!=NULL) free(mean1);

	exit(status);
}
