#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

#define thisprog "xe-energyvec1"
#define TITLE_STRING thisprog" v 1: 26.October.2013 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing</TAGS>
*/

/* external functions start */
int xf_stats2_d(double *data, long n, int varcalc, double *result_d);
float *xf_morletwavelet1_f(double f, double Fs, int ncycles, size_t *nwavelet);
float *xf_conv1_f(float *sig1, size_t n1, float *sig2, size_t n2,char message[] );
complex float *xf_morletwavelet2_f(double f, double Fs, int ncycles, size_t *nwavelet);
complex float *xf_conv2_f(complex float *sig1, size_t n1, complex float *sig2, size_t n2,char message[] );
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[MAXLINELEN],outfile[MAXLINELEN],line[MAXLINELEN],message[MAXLINELEN];
	long int i,j,k,n;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d;
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin,*fpout;
	size_t ii,jj,kk,nn,mm;
	size_t sizeoffloat=sizeof(float),sizeofcfloat=sizeof(complex float),sizeofdouble=sizeof(double);

	/* program-specific variables */
	int lenwords=0,*count,grp,bin,bintot,setrange=0,colx=1,coly=2;
	size_t nwavelet;
	long nwords=0,*start=NULL;
	float *dat1=NULL,*dat2=NULL,scaling,absum=0.0;;
	complex float *cdat1=NULL,*cdat2=NULL,*wavelet=NULL;
	/* arguments */
	int setncycles=7,setformat=1,setbintot=25,coldata=1,setverb=1;
	float setfreq=8.0,setsampfreq=24000.0; // sample frequency of input (samples/s)

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Output the energy envelope in a signal at frequency f\n");
		fprintf(stderr,"Complex wavelet convolution is used to filter & transform the signal\n");
		fprintf(stderr,"Assumes one value per input line\n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\", format= single column\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-sf: sample frequency of input (sample/s) [%f]\n",setsampfreq);
		fprintf(stderr,"	-f: central frequency for band-pass filtering [%f]\n",setfreq);
		fprintf(stderr,"	-c: number of cycles in the wavelet [%d]\n",setncycles);
		fprintf(stderr,"	-v: verbocity (0=energyvec, 1=detailed, 2=wavelet) [%d]\n",setverb);
		fprintf(stderr,"		* detailed output = [output-label] [sample] [output]\n");
		fprintf(stderr,"		* labels: 0=original 1=filtered 3=energy envelope\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -sf 400 -f 60 \n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -sf 1500 -f 60 -v 1 \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	-v 0: single column, energy-vector\n");
		fprintf(stderr,"	-v 1: [label] [time] [data] (1=original, 2=filtered, 3=envelope)\n");
		fprintf(stderr,"	-v 2: single column, wavelet\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-sf")==0) 	{ setsampfreq=atof(argv[ii+1]); i++;}
			else if(strcmp(argv[ii],"-f")==0) 	{ setfreq=atof(argv[ii+1]); i++;}
			else if(strcmp(argv[ii],"-c")==0) 	{ setncycles=atoi(argv[ii+1]); i++;}
			else if(strcmp(argv[ii],"-v")==0) 	{ setverb=atoi(argv[ii+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setsampfreq<=0) {fprintf(stderr,"\n--- Error[%s]: sample frequency (-sf %g) must be > 0\n\n",thisprog,setsampfreq);exit(1);};
	if(setfreq<=0) {fprintf(stderr,"\n--- Error[%s]: bandpass frequency (-f %g) must be > 0\n\n",thisprog,setfreq);exit(1);};
	if(setncycles<=0) {fprintf(stderr,"\n--- Error[%s]: number of cycles in wavelet (-c %d) must be > 0\n\n",thisprog,setncycles);exit(1);};
	if(setverb<0||setverb>2){fprintf(stderr,"\n--- Error[%s]: verbocity (-v %d) must be 0,1 or 2\n\n",thisprog,setverb);exit(1);};

	/* STORE DATA METHOD 1 - stream of single numbers in column */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while(fscanf(fpin,"%s",line)==1) {
		if(sscanf(line,"%f",&a)!=1) a=NAN;
		if((cdat1=(complex float *)realloc(cdat1,(nn+1)*sizeofcfloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		cdat1[nn++]= a+I*0.0;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	/* verbose output if required */
	if(setverb==1) for(ii=0;ii<nn;ii++) printf("0\t%f\t%f\n",(float)(ii/setsampfreq),creal(cdat1[ii]));


	/* GENERATE THE WAVELET */
 	wavelet= xf_morletwavelet2_f(setfreq,setsampfreq,setncycles,&nwavelet);

	/* SCALE THE WAVELET SO SUMMED ABSOLUTE-VALUE = 2 */
	/* NOTE: this is necessary so that convoluted signal relfects amplitude of original signal at filter frequency */
	/* NOTE: this scaling also eliminates the need to multiply the energy vector by 2 and square it, as in the original code */
	for(ii=0;ii<nwavelet;ii++) absum+=cabsf(wavelet[ii]);
	scaling=2.0/absum;
	for(ii=0;ii<nwavelet;ii++) wavelet[ii] *= scaling;
	/* verbose output if required */
	if(setverb==2) for(ii=0;ii<nwavelet;ii++) printf("%f\n",creal(wavelet[ii]));


	/* CONVOLUTE THE SIGNAL AGAINST THE SCALED WAVELET */
	/* NOTE the Onslow MATLAB code convolves against the transpose of the wavelet matrix - not necessary here?*/
	cdat2= xf_conv2_f(cdat1,nn,wavelet,nwavelet,message);

	mm=nn+(nwavelet-1);
	jj= (ceil)(nwavelet/2.0);
	kk= mm-(floor)(nwavelet/2.0)+1;
	/* verbose output if required */
	if(setverb==1) for(ii=jj;ii<kk;ii++) printf("1\t%f\t%f\n",((float)(ii-jj)/setsampfreq),creal(cdat2[ii]));

	/* OUTPUT THE ENERGY VECTOR (ABSOLUTE VALUE OF THE COMPLEX RESULT = DISTANCE FROM ORIGIN) */
	/* verbose output if required */
	if(setverb==1) for(ii=jj;ii<kk;ii++) printf("3\t%f\t%f\n",((float)(ii-jj)/setsampfreq),cabsf(cdat2[ii]));
	else if(setverb==0) for(ii=jj;ii<kk;ii++) printf("%f\n",cabsf(cdat2[ii]));

END1:
	fprintf(stderr,"nn=%ld\n",nn);
	fprintf(stderr,"jj=%ld\n",jj);
	fprintf(stderr,"kk=%ld\n",kk);
	fprintf(stderr,"nwavelet=%ld\n",nwavelet);

	if(dat1!=NULL) free(dat1);
	if(dat2!=NULL) free(dat2);
	if(cdat1!=NULL) free(cdat1);
	if(cdat2!=NULL) free(cdat2);
	exit(0);
	}
