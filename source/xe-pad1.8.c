#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-pad1"
#define TITLE_STRING thisprog" v 8: 7.May.2016 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing</TAGS>

v 8: 21.May.2016 [JRH]
	- bugfix for type3 (beginning and end) in padding function xf_padarray0_f - wasn't starting at correct sample for end-padding
	- removed requirement for xf_padarray0_f to use padding >=2
	- add additional padding options 3 & 4 (sample-and-hold mean or first/last values)

v 8: 7.May.2016 [JRH]
	- fixed memory allocation in xf_padarray0_f - brought in-line with other pad functions
	- allow n (number of padding points) to be <=0 - data will passed unaltered, with a warning
	- fix mistake in warnings re: edge formats
	- standardized notes in pad functions

v 8: 23.January.2015 [JRH]
	- use revised padding functions

v 7: 3.October.2013 [JRH]
	- added improved trend-to-zero padding function

v 6: 4.September.2012 [JRH]
	- fix to function padarray1_d - now restricts padded values to within +- the total "real" data range
	- add padarray3_d option - reverse-copy trended to zero

v 5: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."

v.4: 2.May.2012 [JRH]
	- add new option to pad with edges tending towards zero-change
	- this is superior for eliminating edge-effects from filtering if there are large changes in values at either end of the data

v.3: 30.April.2012 [JRH]
	- bugfix: for larger amounts of padding memory re-allocation seems to not have been successful when performed inside the function (memory lost upon return?)
	- memory is now allocated in main(), before calling the padding function
*/

/* external functions start */
float *xf_padarray0_f(float *data, long nn, long npad, int type, char *message);
float *xf_padarray1_f(float *data, long nn, long npad, int type, char *message);
float *xf_padarray2_f(float *data, long nn, long npad, int type, char *message);
float *xf_padarray3_f(float *data, long nn, long npad, int type, char *message);
float *xf_padarray4_f(float *data, long *nn, long npad, int type, char *message);

/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],message[MAXLINELEN],*pline,*pcol;
	int v,w,x,y,z,col,colmatch,result_i[32];
	long ii,jj,kk,nn;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	long n2=(long)0;
	float *data=NULL;
	/* arguments */
	int setpadtype=1,setedge=1;;
	long setnpad=5;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Pad a series of numbers with extra values at the start, end, or both\n");
		fprintf(stderr,"Useful for avoiding edge-effects with filtering, but trim data afterwards\n");
		fprintf(stderr,"Non-numeric values will be assigned to NaN\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\" - single column of data\n");
		fprintf(stderr,"VALID OPTIONS (deafulats in []):\n");
		fprintf(stderr,"	-n(umber) of padded values to add [%d]\n",setnpad);
		fprintf(stderr,"	-t(ype) of padding: 1(beginning) 2(end) or 3(both) [%d]\n",setpadtype);
		fprintf(stderr,"	-e(dge) treatment: [%d]\n",setedge);
		fprintf(stderr,"		0: zeros\n");
		fprintf(stderr,"		1: trend to zero rate-of-change\n");
		fprintf(stderr,"		2: reverse-copy, cosine-trended to zero\n");
		fprintf(stderr,"		3: mean of first and/or last n-values\n");
		fprintf(stderr,"		4: sample-and-hold first and/or last value\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -n 5 -t 1\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 2 -n 25\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	Padded data in a single column\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-t")==0) setpadtype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-n")==0) setnpad=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-e")==0) setedge=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setedge<0||setedge>4) {fprintf(stderr,"\n--- Error[%s]: invalid edge-treatment (-e = %d): must be 0,1,2,3 or 4\n\n",thisprog,setedge);exit(1);}

	if(setnpad<=0) {
		if(strcmp(infile,"stdin")==0) fpin=stdin;
		else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
		while(fgets(line,MAXLINELEN,fpin)!=NULL) printf("%s",line);
		if(strcmp(infile,"stdin")!=0) fclose(fpin);
		fprintf(stderr,"--- Warning[%s]: padding set to %ld: input is not altered\n",thisprog,setnpad);
		exit(0);
	}

	/* STORE DATA */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		// if line contains non-numerical values then read the whole thing as a character array
		if(sscanf(line,"%f",&a)!=1) a=NAN;
		data=(float *)realloc(data,(nn+1)*sizeoffloat); if(data==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		data[nn]=a;
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* ALLOCATE EXTRA MEMORY FOR THE SOON-TO-BE-PADDED ARRAY */
	n2=nn+setnpad; if(setpadtype==3) n2+=setnpad;

	/* PAD THE ARRAY */
	if(setedge==0) data= xf_padarray0_f(data,nn,setnpad,setpadtype,message);
	if(setedge==1) data= xf_padarray1_f(data,nn,setnpad,setpadtype,message);
	if(setedge==2) data= xf_padarray2_f(data,nn,setnpad,setpadtype,message);
	if(setedge==3) data= xf_padarray3_f(data,nn,setnpad,setpadtype,message);
	if(setedge==4) { data= xf_padarray4_f(data,&nn,setnpad,setpadtype,message); n2=nn; }
	if(data==NULL) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }


	for(ii=0;ii<n2;ii++) printf("%g\n",data[ii]);

	free(data);
	exit(0);
	}
