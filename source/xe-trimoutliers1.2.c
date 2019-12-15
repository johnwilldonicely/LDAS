#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-trimoutliers1"
#define TITLE_STRING thisprog" v 2: 14.August.2012 [JRH]"
#define MAXLINELEN 1000
#define MAXGROUP 8000

/*
<TAGS>filter signal_processing</TAGS>

v 2: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/


void xfinternal_minisortf(double *array,long n) {
	long l,j,ir,i;
	double rra, *ra;
	if(n<2) return;
	ra=array-1; l=(n >> 1)+1; ir=n;
	for(;;) {
		if (l>1) rra=ra[--l];
		else { rra=ra[ir]; ra[ir]=ra[1]; if (--ir ==1) {ra[1]=rra; return;}}
		i=l; j=l << 1;
		while (j<=ir) {if(j<ir&&ra[j]<ra[j+1]) j++; if(rra<ra[j]){ra[i]=ra[j];j+=(i=j);} else j=ir+1;}
		ra[i]=rra;
}}


/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],*pline,*pcol,*perror;
	long int i,j,k,n=0;
	int w,x,y,z,col;
	int sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,result_f[64];
	double aa,bb,cc,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	double *data=NULL,min,max,lcut,ucut;
	/* arguments */
	int groupcol=-1,varcalc=2;
	float setlower=0.0,setupper=100.0;

	void xfinternal_minisortf(double *array,long n);

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Trim a data stream to remove outliers\n");
		fprintf(stderr,"Non-numeric values and NAN/INF are always removed\n");
		fprintf(stderr,"USAGE: %s [filename] [options]\n",thisprog);
		fprintf(stderr,"	[filename]: file name or \"stdin\"\n");
		fprintf(stderr,"OPTIONS ( defaults in [] ):\n");
		fprintf(stderr,"	-l lower percentile cutoff [%g]\n",setlower);
		fprintf(stderr,"	-u upper percentile cutoff [%g]\n",setupper);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -l 10 -u 90\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -l 25 -u 75\n",thisprog);
		fprintf(stderr,"OUTPUT: trimmed and sorted dataset\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-l")==0) 	{ setlower=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-u")==0) 	{ setupper=atof(argv[i+1]); i++;}
			else {fprintf(stderr,"\t\aError[%s]: invalid command line argument \"%s\"\n",thisprog,argv[i]); exit(1);}
	}}

	// CHECK VALIDITY OF ARGUMENTS
	if(setlower<0||setlower>100|setupper<0||setupper>100) {fprintf(stderr,"\n--- Error[%s]: -l and -u must specify a percentile from 0-100\n\n",thisprog); exit(1);}
	if(setlower>=setupper) {fprintf(stderr,"\n--- Error[%s]: -l (%g) must be less than -u (%g)\n\n",thisprog,setlower,setupper); exit(1);}

	// CONVERT PERCENTILES TO FRACTIONS
	setlower/=100.0; setupper/=100.0;

	/* STORE DATA **************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\t\aError[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}
	n=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf",&aa)!=1) continue;
		if(isfinite(aa)) {
			data=(double *)realloc(data,(n+1)*sizeofdouble);
			if(data==NULL) {fprintf(stderr,"\t\aError[%s]: insufficient memory\n",thisprog);exit(1);}
			data[n++]=aa;
	}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	// SORT THE DATA
	xfinternal_minisortf(data,n);

	// GET LOWER & UPPER CUTOFFS
	a=n*setlower; j=(int)a;	if(j<0)j=0;	// calculate the lower cutoff - make an integer version
	if(a!=(double)j) lcut=data[j];  	// if the percentile limit does not fall exactly on an item, cutoff is unchanged
	else lcut=(data[j]+data[j-1])/2.0;  // otherwise if the cutoff is exactly an item number, take the avergage of this point and the previous item

	a=n*setupper; j=(int)a;	if(j>=n)j=n-1;	// repeat for the upper cutoff
	if(a!=(double)j) ucut=data[j];
	else ucut=(data[j]+data[j-1])/2.0;

	// OUTPUT THE TRIMMED DATASET
	for(i=0;i<n;i++) if(data[i]>=lcut && data[i]<=ucut) printf("%g\n",data[i]);

	free(data);
	exit(0);
}
