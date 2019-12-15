#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define thisprog "xe-posstats1"
#define TITLE_STRING thisprog" v 3: 8.February.2015 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>stats</TAGS>

v 3: 8.February.2015 [JRH]
	- add speed threshold for angular velocity output
	- remove Butterworth filtering option
*/

/* external functions start */
void xf_geom_distangle(float x1,float y1,float x2,float y2,float *result) ;
int xf_smoothgauss1_f(float *original,size_t arraysize,int smooth);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],word[256],message[MAXLINELEN];
	long int i,j,k,n,nchars=0,maxlinelen=0;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin,*fpout;
	size_t ii,jj,kk,nn,mm;
	size_t sizeoffloat=sizeof(float);

	/* program-specific variables */
	long nwin,nhalfwin;
	long ngauss1,nhalfgauss1,ngauss2,nhalfgauss2;
	float *xdat=NULL,*ydat=NULL,*xdat2=NULL,*ydat2=NULL,*vel=NULL,*angle=NULL,*avel=NULL;

	/* arguments */
	int setformat=1,setbintot=25,coldata=1;
	float setfreq=1.0,setwin=0.4,setvelmin=1.0;
	float setgauss1=0.0, setgauss2=0.0;



	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Analyze a position record\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\" in format <xpos> <ypos>\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-sf: sample frequency of input [%g]\n",setfreq);
		fprintf(stderr,"	-w: window size (sec) for integrating movement [%g]\n",setwin);
		fprintf(stderr,"	-v: minimum velocity (cm/s) for calculating ang.velocity [%.2f]\n",setvelmin);
		fprintf(stderr,"	-g1: Gaussian smoothing window (sec) for position [%g]\n",setgauss1);
		fprintf(stderr,"		- applied before calculating velocity, angles and ang.velocity\n");
		fprintf(stderr,"	-g2: Gaussian smoothing window (sec) for angles [%g]\n",setgauss2);
		fprintf(stderr,"		- applied before calculating ang.velocity\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -t 1\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 3\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-sf")==0) setfreq= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-w")==0)  setwin= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-v")==0)  setvelmin= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-g1")==0) setgauss1= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-g2")==0) setgauss2= atof(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	nwin=(long)(setwin*setfreq); // ?? maybe add one because this needs to be time elapsed
	if(nwin%2==0) nhalfwin= (nwin/2)-1;
	else nhalfwin= (nwin-1)/2;

	ngauss1=(long)(setgauss1*setfreq); // ?? maybe add one because this needs to be time elapsed
	if(ngauss1%2==0) nhalfgauss1= (ngauss1/2)-1;
	else nhalfgauss1= (ngauss1-1)/2;
	ngauss2=(long)(setgauss2*setfreq); // ?? maybe add one because this needs to be time elapsed
	if(ngauss2%2==0) nhalfgauss2= (ngauss2/2)-1;
	else nhalfgauss2= (ngauss2-1)/2;

	/* STORE DATA METHOD 1 - newline-delimited pairs of data */
	/* - all lines are stored, to preserve line-count of input */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if((xdat=(float *)realloc(xdat,(nn+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((ydat=(float *)realloc(ydat,(nn+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if(sscanf(line,"%lf %lf",&aa,&bb)==2) {
			if(isfinite(aa) && isfinite(bb)) {
				xdat[nn]=aa;
				ydat[nn]=bb;
			}
			else xdat[nn]=ydat[nn]=NAN;
		}
		else xdat[nn]=ydat[nn]=NAN;
		nn++;;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	/* ALLOCATE MEMORY FOR ADDITIONAL VARIABLES */
	if((vel=(float *)realloc(vel,(nn+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((angle=(float *)realloc(angle,(nn+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((avel=(float *)realloc(avel,(nn+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

// DEJUMP THE DATA

// INTERPOLATE MISSING POINTS (SET MAX INTERPOLATION)

	// APPLY SMOOTHER TO X/Y VALUES
	if(ngauss1>0) {
		xf_smoothgauss1_f(xdat,nn,nhalfgauss1);
		xf_smoothgauss1_f(ydat,nn,nhalfgauss1);
	}

	/* CALCULATE MID-WINDOW (j) VARIABLES FOR MIDDLE OF DATA */
	mm= (nn-nwin)+1; // set maximum start window
	jj=nhalfwin; // middle of window
	kk=nwin-1; // end of window
	for(ii=0;ii<mm;ii++) {
		xf_geom_distangle(xdat[ii],ydat[ii],xdat[kk],ydat[kk],result_f) ;
		vel[jj]=result_f[0]/setwin; // momentary velocity
		angle[jj]=result_f[1]; // angle of motion
		jj++;
		kk++;
	}

	/* APPLY SMOOTHER TO ANGLE VALUES  */
	if(ngauss2>0) {
		xf_smoothgauss1_f(angle,nn,nhalfgauss2);
	}

	/* CALCULATE ANGULAR VELOCITY, PROVIDED SPEED THRESHOLD IS EXCEEDED  */
	jj=nhalfwin;
	kk=nwin-1;
	for(ii=0;ii<mm;ii++) {
		if(vel[jj]>=setvelmin) {
			aa= angle[kk]-angle[ii];
			if(aa>180.0) aa-=360.0;
			if(aa<-180.0) aa+=360.0;
			avel[jj]= aa/setwin;
		}
		jj++;
		kk++;
	}


	printf("xdat\tydat\tvel\tangle\tavel\n");
	for(ii=0;ii<nn;ii++) printf("%g\t%g\t%g\t%g\t%g\n",xdat[ii],ydat[ii],vel[ii],angle[ii],avel[ii]);



	if(xdat!=NULL) free(xdat);
	if(ydat!=NULL) free(ydat);
	if(xdat2!=NULL) free(xdat2);
	if(ydat2!=NULL) free(ydat2);
	if(vel!=NULL) free(vel);
	if(angle!=NULL) free(angle);
	if(avel!=NULL) free(avel);
	exit(0);
}
