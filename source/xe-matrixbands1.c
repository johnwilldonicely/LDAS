#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-matrixbands1"
#define TITLE_STRING thisprog" v 1: 3.February.2021 [JRH]"

/*
<TAGS>math dt.matrix noise</TAGS>

v 1: 3.February.2021 [JRH]
*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
double *xf_matrixread3_d(FILE *fpin, long *ncols, long *nrows, char *header, char *message1);
int xf_matrixrotate2_d(double *matrix1, long *width, long *height, int r);
int xf_auc1_d(double *curvey, long nn, double interval, int ref, double *result ,char *message1);

/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	long int ii,jj,kk,mm,nn,row,col;
	int v,w,x,y,z,colmatch,result_i[16];
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;

	/* line-reading and word/column-parsing */
	char *line=NULL;
	long *iword=NULL,nwords;

	/* program-specific variables */
	char header[256],message1[256],message2[256];
	long *ids=NULL,nids=0,idmax=-1,n1,nrows1,ncols1,nmatrices,bintot1;
	long *bandA2=NULL,*bandZ2=NULL,nbands,band,bandmax=-1;
	double *bandA1=NULL,*bandZ1=NULL;
	double *matrix1=NULL,*pmatrix=NULL;

	/* arguments */
	char *infile1=NULL;
	int setsign=0,setrotate=0,setnorm=-1;
	long setn1=-1,setn2=-1;

	double setxmin=1, setxint=1;
	double setymin=1.0, setyint=1.0;
	char setyunits[256]="time";
	char *setheadids=NULL,*setbands=NULL;

	/* PRE-DEFINE PROTOTYPE FREQUENCY BANDS */
	nbands=4;
	if((bandA1= (double*)calloc(nbands,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((bandZ1= (double*)calloc(nbands,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((bandA2= (long*)calloc(nbands,sizeof(long)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((bandZ2= (long*)calloc(nbands,sizeof(long)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	bandA1[0]=0.5;  bandZ1[0]=4.0; // delta - Buzsaki
	bandA1[1]=4.0;  bandZ1[1]=12.0; // theta - Whishaw
	bandA1[2]=13.0; bandZ1[2]=30.0; // beta - Magill (20Hz mean)
	bandA1[3]=30.0; bandZ1[3]=100.0; // gamma (mixed)

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract bands (eg.frequency) from a matrix representing a timeseries\n");
		fprintf(stderr,"- assumes each row is a timepoint (but see -r option)\n");
		fprintf(stderr,"- bands (AUC for multiple columns) are defined for each time (row)\n");
		fprintf(stderr,"- input should have no labels for row & columns\n");
		fprintf(stderr,"    - hence user defines minimum & interval for rows & columns\n");
		fprintf(stderr,"- blank lines or comments (\"#\") should separate multiple matrices\n");
		fprintf(stderr,"- non-numeric values will be ignored\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE: %s [infile] [options]\n",thisprog);
		fprintf(stderr,"    [infile]: file (or \"stdin\") containing data matrix/matrices\n");
		fprintf(stderr,"        - format: space-delimited numbers in columns and rows\n");
		fprintf(stderr,"        - missing values require placeholders (NAN, \"-\", etc.)\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"    -r: rotate matrix before processing (0,-90,90,180) [%d]\n",setrotate);
		fprintf(stderr,"\n");
		fprintf(stderr,"     OPTIONS FOR DEFINING DATA-RANGES, AFTER ROTATING: \n");
		fprintf(stderr,"    -xmin: leftmost column-value [%.3f]\n",setxmin);
		fprintf(stderr,"    -xint: interval between columns [%.3f]\n",setxint);
		fprintf(stderr,"    -ymin: topmost row-value [%.3f]\n",setymin);
		fprintf(stderr,"    -yint: interval between rows [%.3f]\n",setyint);
		fprintf(stderr,"\n");
		fprintf(stderr,"    -ids: CSV list of fields in matrix-headers to use as IDs [unset]\n");
		fprintf(stderr,"        - NOTE: ony applies for comment-line (\"#\") matrix separators\n");
		fprintf(stderr,"        - NOTE: fields should be whitespace delimited\n");
		fprintf(stderr,"    -bands: CSV band list [%g,%g,%g,%g,%g,%g,%g,%g]\n",bandA1[0],bandZ1[0],bandA1[1],bandZ1[1],bandA1[2],bandZ1[2],bandA1[3],bandZ1[3]);
		fprintf(stderr,"        - NOTE: default is common EEG bands:\n");
		fprintf(stderr,"            delta = %g - %g Hz\n",bandA1[0],bandZ1[0]);
		fprintf(stderr,"            theta = %g - %g Hz\n",bandA1[1],bandZ1[1]);
		fprintf(stderr,"            beta  = %g - %g Hz\n",bandA1[2],bandZ1[2]);
		fprintf(stderr,"            gamma = %g - %g Hz\n",bandA1[3],bandZ1[3]);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"    %s matrix.txt -xmin 0.5 -xint 0.5 -ymin 0 -yint 0.01 -ids 2,4,6\n",thisprog);
		fprintf(stderr,"    %s matrix.txt -bands 0,4,4,12\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"    A long-format timeseries output:\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile1= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-xmin")==0) setxmin= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-xint")==0) setxint= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-ymin")==0) setymin= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-yint")==0) setyint= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-bands")==0) setbands= argv[++ii];
			else if(strcmp(argv[ii],"-ids")==0) setheadids= argv[++ii];
			else if(strcmp(argv[ii],"-r")==0)  setrotate= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setrotate!=0 && setrotate!=-90 && setrotate!=90 && setrotate!=180) {fprintf(stderr,"\n--- Error[%s]: invalid -r [%d]: must be 0,-90, 90, or 180\n\n",thisprog,setrotate);exit(1);}


	/********************************************************************************
	PARSE THE CSV LIST OF HEADER-IDS INTO AN ID ARRAY, FIND IDMAX
	********************************************************************************/
	if(setheadids!=NULL) {
		iword= xf_lineparse2(setheadids,",",&nids);
		ids= realloc(ids,nids*sizeof(long));
		if(ids==NULL) {{fprintf(stderr,"\n--- Error[%s]: memory allocation error\n\n",thisprog);exit(1);}}
		for(ii=0;ii<nids;ii++) {
			ids[ii]= atol(setheadids+iword[ii]);
			if(ids[ii]<0) {fprintf(stderr,"\n--- Error[%s]: invalid id-field (%ld): must be >=0\n\n",thisprog,ids[ii]);exit(1);}
			if(ids[ii]>idmax) idmax= ids[ii];
	}}
	//TEST	for(ii=0;ii<nids;ii++) printf("ids[%ld]: %ld\n",ii,ids[ii]); exit(0);

	/********************************************************************************
	PARSE THE USER-DEFINED BANDS LIST
	********************************************************************************/
	if(setbands!=NULL) {
		kk= nbands; // save original nbands
		iword= xf_lineparse2(setbands,",",&nbands);
		if((nbands%2)!=0) {fprintf(stderr,"\n--- Error[%s]: band-list (-bands) does not contain pairs of numbers\n\n",thisprog);exit(1);}
		nbands/=2;
		if(nbands>kk) {
			if((bandA1= (double*)calloc(nbands,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
			if((bandZ1= (double*)calloc(nbands,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
			if((bandA2= (long*)calloc(nbands,sizeof(long)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
			if((bandZ2= (long*)calloc(nbands,sizeof(long)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
		}
		for(ii=0;ii<nids;ii++) {
			bandA1[ii]= atol(setbands+iword[(ii*2)]);
			bandZ1[ii]= atol(setbands+iword[(ii*2+1)]);
			if(bandZ1[ii]<0) {fprintf(stderr,"\n--- Error[%s]: invalid band (%g-%g): values must be >=0\n\n",thisprog,bandA1[ii],bandZ1[ii]);exit(1);}
	}}

	/********************************************************************************
	DEFINE FREQUENCY-BAND INDICES (DELTA,THETA,BETA,GAMMA)
	********************************************************************************/
	kk= (long)(setxmin/setxint);
	for(ii=0;ii<nbands;ii++) {
		bandA2[ii]= (long)(bandA1[ii]/setxint) - kk; // start
		bandZ2[ii]= (long)(bandZ1[ii]/setxint) - kk; // stop - increment below
		// TEST: printf("band:%ld \tA1:%g \tZ1:%g \tA2:%ld \tZ2:%ld\n",ii,bandA1[ii],bandZ1[ii],bandA2[ii],bandZ2[ii]);
		if(bandA2[ii]<0) {
			bandA2[ii]=0;
			fprintf(stderr,"--- Warning[%s]: band-start %ld adjusted to align with edge of matrix\n",thisprog,band);
		}
		if(bandA2[ii]>=bandZ2[ii]) {
			fprintf(stderr,"\n--- Error[%s]: band %ld (%g-%g) too narrow or reverse-order\n\n",thisprog,ii,bandA1[ii],bandZ1[ii]);
			exit(1);
		}
		bandZ2[ii]++; // increment here to make it a true "stop" value (not included in AUC output)
		if(bandZ2[ii]>bandmax) bandmax= bandZ2[ii];
	}
	// TEST:for(ii=0;ii<nbands;ii++) printf("band:%ld \tA1:%g \tZ1:%g \tA2:%ld \tZ2:%ld\n",ii,bandA1[ii],bandZ1[ii],bandA2[ii],bandZ2[ii]);

	/* PRINT HEADER */
	for(ii=0;ii<nids;ii++) printf("id%ld\t",ids[ii]);
	printf("%s",setyunits);
	for(ii=0;ii<nbands;ii++) printf("\tband%ld",ii);
	printf("\n");

	/* INITIALISE HEADER & MESSAGES ARRAYS - THIS IS REQUIRED FOR THE MATRIX READ FUNCTION */
	header[0]='\n'  ; header[1]='\0';
	message1[0]='\n' ; message1[1]='\0';
	nmatrices=0;

	/********************************************************************************
	READ MATRICES ONE AT A TIME
	********************************************************************************/
	if(strcmp(infile1,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile1,"r"))==0) {fprintf(stderr,"\n\a--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile1);exit(1);}
	while(1) {

		/* READ A HEADER + MATRIX  */
		n1=nrows1=ncols1= 0;
		matrix1= xf_matrixread3_d(fpin,&ncols1,&nrows1,header,message1);
		if(ncols1<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message1); exit(1); }
		else if(matrix1==NULL) break;
		else nmatrices++;
		n1= nrows1*ncols1;
		//TEST: printf("%s\n",header); for(ii=jj=0;ii<n1;ii++) {printf("%g",matrix1[ii]);if(++jj<ncols1) printf(" ");else { jj=0;printf("\n"); }}

		/* APPLY ROTATION IF REQUIRED */
		if(setrotate!=0) {z= xf_matrixrotate2_d(matrix1,&ncols1,&nrows1,-90); if(z<0){ fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message1); exit(1); }}

		/* CHECK MATRIX SIZE IS OK */
		if(ncols1<bandmax) {fprintf(stderr,"\n--- Error[%s]: matrix too small to accomodate specified bands - consider rotating, or adjusting xmin/xint/ymin/yint or bands\n\n",thisprog);exit(1);};

		/* IF IDs ARE TO BE EXTRACTED, PARSE THE HEADER */
		if(nids>0) {
			strncpy(message2,header,256);
			iword= xf_lineparse1(message2,&nwords);
			if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
			if(nwords<idmax) {fprintf(stderr,"\n--- Error[%s]: header line has insuffcient fields to accomodate max id-list item %ld\n\n",thisprog,idmax);exit(1);};
			//TEST: for(ii=0;ii<nwords;ii++) printf("%ld: %s\n",ii,(message2+iword[ii]));exit(0);
		}


		/* FOR EACH ROW (TIME), PRINT ID'S, TIME, AND AUC-VALUES FOR EACH BAND */
		printf("\n");
		for(row=0;row<nrows1;row++) {
			/* print seelected ids from the header */
			for(ii=0;ii<nids;ii++) printf("%s\t",message2+iword[(ids[ii]-1)]);
			/* print the y-value */
			printf("%g",(setymin+ setyint*row));
			/* calculate and print AUC for each band */
			for(band=0;band<nbands;band++) {
				mm= bandZ2[band] - bandA2[band];
				pmatrix= matrix1+(row*ncols1)+bandA2[band];
				x= xf_auc1_d(pmatrix,mm,setxint,0,result_d,message2);
				if(x!=0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message2); exit(1); }
				printf("\t%g",result_d[0]);
			}
			printf("\n");
		}
	}


	/********************************************************************************
	CLEANUP AND EXIT
	********************************************************************************/
END:
	if(strcmp(infile1,"stdin")!=0) fclose(fpin);

	if(matrix1!=NULL) free(matrix1);

	exit(0);
}
