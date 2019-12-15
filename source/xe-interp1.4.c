#define thisprog "xe-interp1.2.c"
#define TITLE_STRING thisprog" v 4: 30.April.2016 [JRH]"
#define MAXLINELEN 1000

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*
<TAGS>signal_processing filter</TAGS>

v 4: 30.April.2016 [JRH]
	- add ability to define the invalid data value for interpolation
	- update variable names (ii,jj etc) and argument handling

v 4: 5.September.2012 [JRH]
	- upgrade interpolation function to interp3_d - this will interpolate missing/invalid vaues at the beginning and end of the array as well

v 3: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."

v 2: 20.April.2012 [JRH]
	- now properly treats "-" and "." as non-numbers to be interpolated
*/

/* external functions start */
long xf_interp3_d(double *data, long ndata);
int xf_interp4_d(double *data, long ndata, float invalid, int setfill, long *result);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],word[MAXLINELEN],message[MAXLINELEN];
	long ii,jj,kk,nn=0;
	int v,w,x,y,z,sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program specific variables */
	long result[8];
	double *data=NULL,sum,temp_n1,temp_n2;
	/* arguments */
	int setverbose=0;
	double setbad=NAN;

	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Interpolate across blank lines, non-numerical data and NaN/Inf values\n");
		fprintf(stderr,"Outputs exactly the same number of lines as the input\n");
		fprintf(stderr,"USAGE: %s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: single-column data file or stdin\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-bad: alternative value to interpolate across [%g]\n",setbad);
		fprintf(stderr,"		NOTE: if data also contains NAN,INF or non-numeric\n");
		fprintf(stderr,"		values, then these may get used for interpolation\n");
		fprintf(stderr,"	-v: verbose output (0=no, 1=yes) [%d]\n",setverbose);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(1);
	}

	/********************************************************************************/
	/* READ COMMAND-LINE ARGUMENTS */
	/********************************************************************************/
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-bad")==0) setbad=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-v")==0)   setverbose=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}


	/********************************************************************************/
	/* STORE DATA - FROM FILE OR STANDARD-INPUT IF FILENAME=STDIN */
	/********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"Error[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf",&aa)!=1) aa=NAN;
		if((data=(double *)realloc(data,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		data[nn++]=aa;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/********************************************************************************/
	/* APPLY ONE OF TWO TYPES OF INTERPOLATION - NAN/INF, OR USER-DEFINED */
	/********************************************************************************/
	if(!isfinite(setbad)) {
		kk=xf_interp3_d(data,nn);
	}
	else {
		x= xf_interp4_d(data,nn,setbad,3,result);
		kk=result[2];
	}

	/********************************************************************************/
	/* OUTPUT THE DATA  */
	/********************************************************************************/
	for(ii=0;ii<nn;ii++) printf("%g\n",data[ii]);

	if(setverbose==1) {
		aa=100*(double)kk/(double)nn; /* percentage of points interpolated */
		fprintf(stderr,"INTERPOLATED_POINTS %d of %d = %.4f %%\n",kk,nn,aa);
	}
	free(data);
	exit(0);
}
