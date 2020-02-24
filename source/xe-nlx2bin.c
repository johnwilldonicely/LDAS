#define thisprog "xe-nlx2bin"
#define TITLE_STRING thisprog" v 1: 11.November.2019 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/*
<TAGS>file NLX</TAGS>

v 1: 11.November.2019 [JRH]
	- program to convert a Neuralynx CSC (.ncs) file into a flat-binary .bin file (float)
*/

/* external functions start */
float *xf_readnlx_ncs(char *infile, float eegscale, long *result, char *message);
int xf_writebin2_v(char *outfile, void *data0, size_t nn, size_t datasize, char *message);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char outfile[256],*line=NULL,*templine=NULL,message[MAXLINELEN];
	long int ii,jj,kk,ll,mm,nn,maxlinelen=0,prevlinelen=0;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin,*fpout;

	/* program-specific variables */
	int datasize;
	long result[64];
	float *nlxdata=NULL;

	/* arguments */
	char *infile=NULL,*setlist=NULL;
	int setdatatype=-1,setbintot=25,setverb=0;
	float setscale=1.0,setbinwidth=0.0;

	if((line=(char *)realloc(line,6))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((templine=(char *)realloc(templine,MAXLINELEN))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Convert a Neuralynx CSC (.ncs) file to a flat-binary .bin file (float)\n");
		fprintf(stderr,"USAGE: %s [infile] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-dt: type of data to write [%d]\n",setdatatype);
		fprintf(stderr,"		-1: ASCII\n");
		fprintf(stderr,"		0: unsigned char\n");
		fprintf(stderr,"		1: signed char\n");
		fprintf(stderr,"		2: unsigned short\n");
		fprintf(stderr,"		3: signed short\n");
		fprintf(stderr,"		4: unsigned int\n");
		fprintf(stderr,"		5: signed int\n");
		fprintf(stderr,"		6: unsigned long\n");
		fprintf(stderr,"		7: signed long\n");
		fprintf(stderr,"		8: float\n");
		fprintf(stderr,"		9: double\n");
		fprintf(stderr,"	-scale: scaling factor to apply to data (eg -1 to invert) [%g]\n",setscale);
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -t 1\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 3\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	\n");
		fprintf(stderr,"	\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-dt")==0) setdatatype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-scale")==0) setscale= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setscale==0) { fprintf(stderr,"\n--- Error [%s]: invalid -scale [%g] - setting to zero will destroy data\n\n",thisprog,setscale);exit(1);}
	if(setverb!=0 && setverb!=1 && setverb != 999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0,1, or 999\n\n",thisprog,setverb);exit(1);}

	if(setdatatype==-1)                     datasize=sizeof(char);
	else if(setdatatype==0||setdatatype==1) datasize=sizeof(char);
	else if(setdatatype==2||setdatatype==3) datasize=sizeof(short);
	else if(setdatatype==4||setdatatype==5) datasize=sizeof(int);
	else if(setdatatype==6||setdatatype==7) datasize=sizeof(long);
	else if(setdatatype==8)                 datasize=sizeof(float);
	else if(setdatatype==9)                 datasize=sizeof(double);
	else {fprintf(stderr,"\n--- Error[%s]: data type (-dt %d) must be 0-9 \n\n",thisprog,setdatatype); exit(1);}

	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(setverb==1) fprintf(stderr,"- reading file %s ...\n",infile);
	nlxdata= xf_readnlx_ncs(infile,setscale,result,message);
	if(nlxdata==NULL) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

	if(setverb==1) {
		fprintf(stderr,"\tsample_rate= %ld Hz\n",result[3]);
		fprintf(stderr,"\tsamples_per_record= %ld\n",result[1]);
		fprintf(stderr,"\tn_records= %ld Hz\n",result[0]);
		fprintf(stderr,"\tn_samples= %ld Hz\n",result[2]);
	}

	nn= result[2];

	/********************************************************************************
	WRITE THE DATA
	********************************************************************************/
	if(setverb==1) fprintf(stderr,"- writing data (%ld samples)...\n",nn);
	if(setdatatype==-1) {
		for(ii=0;ii<nn;ii++) printf("%f\n",nlxdata[ii]);
	}
	else {
		x= xf_writebin2_v("stdout",(void *)nlxdata,nn,datasize,message);
		if(x<0) { fprintf(stderr,"\n--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	/********************************************************************************/
	/* CLEANUP AND EXIT */
	/********************************************************************************/
END:
	if(nlxdata!=NULL) free(nlxdata);
	exit(0);
}
