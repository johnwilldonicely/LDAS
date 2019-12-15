#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-detectnoise1"
#define TITLE_STRING thisprog" v 1: 6.October.2017 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing</TAGS>
v 1: 6.October.2017 [JRH]
*/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_interp3_d(double *data, long namp);
long xf_stats3_d(double *data, long n, int varcalc, double *result_d);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char *infile,line[MAXLINELEN],message[MAXLINELEN];
	long ii,jj,kk,nn;
	int v,w,x,y,z;
	float a,b,c,d;
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	long n1,nbad,halfwin,maxfreq,*iword=NULL,nwords;
	double *freq=NULL,*amp=NULL,*list=NULL,max;
	/* arguments */
	char *setlist=NULL;
	int setverb=0;
	long fullwin=3;
	double setthresh=-1,setmin=-1,setmax=-1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Detect single-frequency deflections in a frequency power spectrum\n");
		fprintf(stderr,"	- signal must drop off either side of the noise peak\n");
		fprintf(stderr,"	- hence no detection at edges of spectra\n");
		fprintf(stderr,"	- non-numeric and non-finite values will be interpolated\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\" in format <freq> <amp>\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-t: change-threshold (+ive, or -1=auto) [%g]\n",setthresh);
		fprintf(stderr,"		-if auto, thresh= 1.5*std.dev\n");
		fprintf(stderr,"	-w: window-size (odd number-of-frequencies) [%ld]\n",fullwin);
		fprintf(stderr,"	-min: lowest noise-search frequency (-1=auto) [%g]\n",setmin);
		fprintf(stderr,"	-max: lowest noise-search frequency (-1=auto) [%g]\n",setmax);
		fprintf(stderr,"	-list: CSV list of frequencies to estimate noise levels at []\n");
		fprintf(stderr,"		- NOTE: will override -t\n");
		fprintf(stderr,"	-verb sets verbosity (0=simple, 1=verbose) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s spectxt -t 3 \n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 3\n",thisprog);
		fprintf(stderr,"OUTPUT: for each detected noise peak...\n");
		fprintf(stderr,"	freq	noise\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"	freq: frequency at which noise was found\n");
		fprintf(stderr,"	noise: mean change from window-middle to each edge\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-list")==0) setlist=argv[++ii];
			else if(strcmp(argv[ii],"-t")==0)    setthresh= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-w")==0)    fullwin= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-min")==0)  setmin= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-max")==0)  setmax= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -verb (%d): must be 0 or 1\n\n",thisprog,setverb); exit(1);}
	if(fullwin%2==0) {fprintf(stderr,"\n--- Error[%s]: invalid -w (%ld): must be an odd number\n\n",thisprog,fullwin); exit(1);}

	/********************************************************************************/
	/* STORE THE SPECTRUM */
	/********************************************************************************/
	nn=nbad=0;
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf %lf",&aa,&bb)!=2 || !isfinite(aa) || !isfinite(bb)) { aa=bb=NAN; nbad++;}
		if((freq= realloc(freq,(nn+1)*sizeof(*freq)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((amp= realloc(amp,(nn+1)*sizeof(*amp)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((setmin<0||aa>=setmin) && (setmax<0||aa<=setmax)){
			freq[nn]=aa;
			amp[nn]=bb;
			nn++;
		}
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(setverb==1) fprintf(stderr,"\t- read %ld amp points (%ld needed interpolation)\n",nn,nbad);


	/********************************************************************************/
	/* INTERPOLATE IF SOME DATA IS BAD */
	/********************************************************************************/
	if(nbad>0) {
		xf_interp3_d(freq,nn);
		xf_interp3_d(amp,nn);
	}

	/********************************************************************************/
	/* CALCULATE THE THRESHOLD IF NOT SET, USING THE STANDARD DEVIATION OF THE INPUT */
	/********************************************************************************/
	if(setthresh<0) {
		ii= xf_stats3_d(amp,nn,2,result_d);
		if(ii<0) {fprintf(stderr,"\n--- Error[%s/]: error code %ld\n\n",thisprog,ii);exit(1);};
		setthresh= 1.5*result_d[2];
	}

	/********************************************************************************/
	/* DETECT SPIKEY-NOISE */
	/********************************************************************************/
	/* calculate the half-window size */
	halfwin= (long)(fullwin/2);
	/* calculate the highest sample-number to avoid window overrunning the data  */
	n1= nn-(halfwin);

	/* OPTION 1: CALCULATE NOISE AT A LIST OF SPECIFIED FREQUENCIES */
	if(setlist!=NULL) {
		/* parse the comma-separated comand line list into an array */
		iword= xf_lineparse2(setlist,",",&nwords);
		list= realloc(list,nwords*sizeof(*list));
		if(list==NULL) {{fprintf(stderr,"\n--- Error[%s]: memory allocation error\n\n",thisprog);exit(1);}}
		if(setverb==1) fprintf(stderr,"\t- parsing list of %ld frequencies\n",nwords);
		for(ii=0;ii<nwords;ii++) list[ii]=atof(setlist+iword[ii]);
		/* now get the noise-estimates */
		if(setverb==1) fprintf(stderr,"\t- quantifying noise\n");
		/* output the header */
		printf("freq\tnoise\n");
		/* get to work... */
		for(ii=0;ii<nwords;ii++) {
			for(jj=halfwin;jj<n1;jj++) {
				if(freq[jj]==list[ii]) {
					aa= amp[jj];
					bb= aa-amp[jj-halfwin];
					cc= aa-amp[jj+halfwin];
					dd= (bb+cc)/2.0;
					/* output the results */
					printf("%g\t%g\n",freq[jj],dd);
					/* break the spectrum scan - frequency should not appear more than once */
					break;
				}
			}
		}
	}

	/* OPTION 2: AUTOMATICALLY DETECT NOISE */
	else {
		if(setverb==1) fprintf(stderr,"\t- quantifying noise\n");
		/* output the header */
		printf("freq\tnoise\n");
		/* get to work... */
		for(ii=halfwin;ii<n1;ii++) {
			/* get the amplitude in the middle, start, and end of the window */
			aa= amp[ii];
			bb= amp[ii-halfwin];
			cc= amp[ii+halfwin];
			/* if change meets criterion... */
			if((aa-bb)>setthresh && (aa-cc)>setthresh) {
				/* find the largest value in this window */
				max= amp[ii];
				maxfreq= ii;
				for(jj=(ii-halfwin);jj<=(ii+halfwin);jj++) {
					if(amp[jj]>max) {
						max= amp[jj];
						maxfreq= jj;
					}
				}
				/* if a better peak is found, adjust aa */
				if(jj!=ii) aa= amp[maxfreq];
				/* recalculate bb & cc as differences & get average difference */
				bb= aa-bb;
				cc= aa-cc;
				dd= (bb+cc)/2.0;
				/* output the results */
				printf("%g\t%g\n",freq[maxfreq],dd);
				/* advance to next window - account for natural increment at end of for loop */
				ii+=(fullwin-1);
			}
			/* note that if no peak had been detected, loop advances to next sample instead of jumping a window-size */
		}
	}

	/* CLEANUP AND EXIT  */
	if(freq!=NULL) free(freq);
	if(amp!=NULL) free(amp);
	if(list!=NULL) free(list);
	if(iword!=NULL) free(iword);
	exit(0);
}
