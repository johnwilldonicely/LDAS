#define thisprog "xe-curvestats2"
#define TITLE_STRING thisprog" v 7: 7.February.2021 [JRH]"
#define MAXLINELEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/*
<TAGS>stats signal_processing</TAGS>

v 7: 7.February.2021 [JRH]
	- bugfix: if no indices are specified, now does not exclude the last value on the line

v 7: 27.February.2018 [JRH]
	- add -avg option to output average rather than AUC per se

v 7: 19.September.2017 [JRH]
	- do not use dummy index array
	- bugfix: no error if index exceeds setmax if setdelta is specified
		- in this instance setmax is determined by the number of items on each line
		- rather than report an error, output for the inappropriate zone will be "NAN"

v 7: 23.January.2017 [JRH]
	- bugfix: last point was omitted when indices not set

v 7: 21.January.2017 [JRH]
	- bugfix: auto-index designation requires dummy array to be pre-allocated
	- bugfix: now properly handles definition and readjustment of stop-indices
	- drop curvestats1_d in favour of auc1_d, since only AUC is output
	- NOTE: because getindex function cannot return indices beyond the end of the input, special treatment is required for the last stop

v 7: 24.October.2014 [JRH]
	- add option to output all zones on a single line: <line> <zone0> <zone1> <zone2> etc


v 6: 29.August.2014 [JRH]
	- adjusted warning/error if index falls outside of min/max
	- now only an error if end-index is less that start, or start-index is less than end
	- warning, otherwise

v 5: 13.May.2014 [JRH]
	- slight change to curvestats function
		- no invalidation of negative/min or positive/max results if there were no negative or positive values respectively
		- previously these would have treated as NAN
		- by default the aucn and aucp values will be zero if there are no negative or positive values
		- the minima and maxima are also valid, even if they dont fall on one side of the y-axis or the other


v 4: 31.March.2014 [JRH]
	- start with maxlinelen zero
	- bugfix: reset of maxwords (if nwords exceeds it) had been defective - was a conditional test instead of assignment
	- bugfix: switch to assigning the return from xf_lineread1 to "pline" instead of the input pointer "line" itself
		- under some circumstances this leads to a memory corruption error
		- specifically, if the first lines are exactly the same length and then a longer line is encountered, an error occurred, and only if xf_lineparse was called previously
		- still not sure why this is, but not using the same pointer for input & output of xf_lineread1 seems to fix the problem

v 3: 11.March.2014 [JRH]
	- change input to require min and max instead of offset and interval
	- interval is now determined by the number of values on the line
	- use xf_getindex1_d to identify start/stop indices for each line

v 2: 10.March.2014 [JRH]
	- added interpolation of non-finite or non-numeric data
	- changed use of xf_lineread1: seems to be safer not to pre-allocate memory for line if preallocated amount is exactly the actual length of the line
	- checks for non-finite input (convert to NAN)
*/


/* external functions start */
long xf_interp3_d(double *data, long ndata);
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_getindex1_d(double min, double max, long n, double value, char *message);
int xf_auc1_d(double *curvey, long nn, double interval, int ref, double *result ,char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],*line=NULL,*pline,message[MAXLINELEN];
	long i,j,k,n,maxlinelen=0,maxwords=maxlinelen;
	long ii,jj,kk,nn;
	int v,w,x,y,z,col,colmatch;
	int sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	char *cstart,*cstop;
	long nindices,*pindex=NULL,zone;
	long nwords=0,*pwords=NULL;
	long nlines=0,gooddata=1;
	double *data1=NULL,interval,start,stop,result_d[16];
	double auc,aucn,aucp,xn,xp,yn,yp,median,com,bias;
	double *index1=NULL;
	/* arguments */
	char *setindices=NULL;
	int setdeltaflag=0,setindicesflag=0,setformat=1,setref=0,setavg=0;
	double setmin=NAN,setmax=NAN,setdelta=NAN;


	if((data1=(double *)realloc(data1,(maxwords)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate area under the curve (AUC) for multiple rows of input\n");
		fprintf(stderr,"Large-input-friendly: only one row of data stored in memory at a time\n");
		fprintf(stderr,"Interprets the x-values from min,max and number of values on each line\n");
		fprintf(stderr,"Assumes rows are tab or space-delimited\n");
		fprintf(stderr,"Missing and non-numeric values will be interpolated\n");
		fprintf(stderr,"NOTE: extra delimiters are interpreted as missing values (NAN)\n");
		fprintf(stderr,"NOTE: all input lines processed (including comments, etc)\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-min: presumed minimum x-value of first item on each line [0]\n");
		fprintf(stderr,"	-max: presumed maximum x-value of last item on each line [1]\n");
		fprintf(stderr,"		- delta inferred as (max - min)/(items_on_line - 1)\n");
		fprintf(stderr,"	-d: fixed AUC delta for interation on each line [unset by default]\n");
		fprintf(stderr,"		- if set, overrides -max\n");
		fprintf(stderr,"		- new max = min + delta x items_on_line\n");
		fprintf(stderr,"	-index: comma-separated start/stop index pairs [unset by default]\n");
		fprintf(stderr,"		- stop for each pair is excluded\n");
		fprintf(stderr,"		- indices must be >=min and <=(max+1)\n");
		fprintf(stderr,"		- if unset, uses min and max+delta \n");
		fprintf(stderr,"		- note that AUC calculation requires at least two points\n");
		fprintf(stderr,"	-avg: divide AUC values by number of values (0=NO 1=YES)  [%d]\n",setavg);
		fprintf(stderr,"		- use this for normalized values like coherence\n");
		fprintf(stderr,"	-f: output format: 1=line per zone, 2=all zones on one line [%d]\n",setformat);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -int .001 -off -5 -d 1 -index 0,.05,.05,.17\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -sf 1000 -pre 0\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	format1: <line><zone><start><stop><n><AUC>\n");
		fprintf(stderr,"	format2: <line><AUC0><AUC1><AUC2>... etc.\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-avg")==0)   setavg=atoi(argv[++i]);
			else if(strcmp(argv[i],"-min")==0)   setmin=atof(argv[++i]);
			else if(strcmp(argv[i],"-max")==0)   setmax=atof(argv[++i]);
			else if(strcmp(argv[i],"-index")==0) setindices=argv[++i];
			else if(strcmp(argv[i],"-d")==0)     setdelta=atof(argv[++i]);
			else if(strcmp(argv[i],"-f")==0)     setformat=atoi(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(setavg!=0&&setavg!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -avg (%d): must be 0 or 1\n\n",thisprog,setavg);exit(1);}
	if(setformat!=1&&setformat!=2) {fprintf(stderr,"\n--- Error[%s]: invalid output format (-f %d): must be 1 or 2\n\n",thisprog,setformat);exit(1);}

	if(!isfinite(setmin)) setmin= 0.0;
	if(!isfinite(setmax)) setmax= setmin+9.0;
	else if(isfinite(setdelta)) {fprintf(stderr,"\n--- Error[%s]: cannot set both max (-max %g) and delta (-d %g)\n\n",thisprog,setmax,setdelta);exit(1);}
	if(isfinite(setdelta)) {
		setdeltaflag= 1;
		setmax= setmin+setdelta; // temporary max value
	}


	/********************************************************************************/
	/* SET UP AN ARRAY OF INDICES WHERE EVEN-ELEMENTS ARE STARTS AND ODD-ELEMENTS ARE STOPS */
	/* - if no indices defined, build indices (x2) based on min and max */
	/* - note that the "max" index is overridden on each line if delta is set */
	/********************************************************************************/
	if(setindices==NULL) {
		nindices=2;
		if((index1=(double *)realloc(index1,(nindices*sizeof(double))))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		index1[0]=setmin;
		index1[1]=setmax;
	}
	else {
		setindicesflag=1;
		pindex= xf_lineparse2(setindices,",",&nindices);
		/* assuming specified indices are in pairs... */
		if(nindices%2==0) {
			/* allocate memory for indices */
			bb=0;
			if((index1=(double *)realloc(index1,(nindices*sizeof(double))))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			for(ii=0;ii<nindices;ii++) {
				/* convert index text to number */
				if(sscanf((setindices+pindex[ii]),"%lf",&aa)!=1 || !isfinite(aa))  {fprintf(stderr,"\n--- Error[%s]: non-numeric index (%s) specified\n\n",thisprog,(setindices+pindex[ii]));exit(1);};
				/* make sure this number falls within appropriate bounds */
				if(ii%2==0) {
					if(aa>setmax && setdeltaflag==0) {fprintf(stderr,"\n--- Error[%s]: start index (%g) > maximum (-max %g)\n\n",thisprog,aa,setmax);exit(1);}
					if(aa<setmin) {
						fprintf(stderr,"\t--- Warning[%s]: adjusting start index (%g) up to -min (%g)\n",thisprog,aa,setmin);
						aa=setmin;
					}
				}
				else if(ii%2!=0 && aa<setmin) {fprintf(stderr,"\n--- Error[%s]: stop index (%g) < minimum (-min %g)\n\n",thisprog,aa,setmin);}
				/* make sure odd-numbered indices are greater than their preceeding partner */
				if(ii%2!=0 && aa<=bb) {fprintf(stderr,"\n--- Error[%s]: index #%ld (%g) is not larger than the one preceding it (%g)\n\n",thisprog,(ii+1),aa,bb);exit(1);}
				/* create an index */
				index1[ii]= aa;
				/* keep track of preceeding index */
				bb=aa;
			}
		}
		else { fprintf(stderr,"\n--- Error[%s]: %ld indices specified - must be an even number\n\n",thisprog,nindices); exit(1); }
		//TEST: for(ii=0;ii<nindices;ii++) fprintf(stderr,"index1[%ld]=%g\n",ii,index1[ii]); //exit(0);
	}

	/********************************************************************************/
	/* PRINT THE HEADER LINE  */
	/********************************************************************************/
 	if(setformat==1) printf("line\tzone\tstart\tstop\tn\tAUC\n");
	else {
		printf("line");
		for(jj=ii=0;ii<nindices;ii+=2) printf("\tAUC%ld",jj++);
		printf("\n");
	}

	/********************************************************************************/
	/* READ THE INPUT DATA */
	/********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nlines=0;
	maxlinelen=-1;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		/* check memory allocation for line */
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		/* parse the line into words */
		pwords = xf_lineparse1(line,&nwords);
		if(nwords>maxwords){ maxwords=nwords; if((data1=(double *)realloc(data1,(maxwords)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}}
		/* determine delta or max for this line depending on if setdeltaflag is set  */
		if(setdeltaflag==1) setmax= setmin + (nwords-1)*setdelta;
		else setdelta=(setmax-setmin)/(nwords-1);
		/* if no indices were set, then also set index to the max possible value for the line */
		if(setindices==NULL) {index1[0]= setmin; index1[1]= setmax + setdelta;}
		// TEST: fprintf(stderr,"nwords=%ld\n",nwords);	fprintf(stderr,"setdelta=%g\n",setdelta); fprintf(stderr,"setmin=%g\n",setmin); fprintf(stderr,"setmax=%g\n",setmax); fprintf(stderr,"indices=%g %g\n",index1[0],index1[1]);

		/* store the words as an array of numbers */
		z=0; for(ii=0;ii<nwords;ii++) {
			if(sscanf(line+pwords[ii],"%lf",&cc)!=1 || !isfinite(cc)) { cc=NAN; z=1; }
			data1[ii]=cc;
		}
		/* interpolate missing values (NAN) */
		gooddata=0;
		if(z>0) gooddata= xf_interp3_d(data1,nwords);

		/* scan each zone */
		zone=0;
		if(setformat==2) printf("%ld", nlines);
		for(ii=0;ii<nindices;ii+=2) {
			aa=index1[ii];   // start x-value
			bb=index1[ii+1]; // stop x-value
			/* if start or stop indices overshoot max, adjust to max (if start overshoots, both are set to max and output will be zero) */
			if(gooddata<0||aa>setmax) {
				auc=NAN;
				aa=bb=setmax;
				nn=0;
			}
			else {
				/* get start-sample */
				jj= xf_getindex1_d(setmin,setmax,nwords,aa,message);
				if(jj<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message);exit(1);}

				/* get stop-sample (this point will not be included) */
				if(bb>setmax) {
					bb=(nwords)*setdelta;
					kk=(nwords);
				}
				else {
					kk= xf_getindex1_d(setmin,setmax,nwords,bb,message);
					if(kk<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message);exit(1);}
				}
				/* calculate size of chunk to process - this will exclude stop-sample kk */
				nn= (kk-jj);
				// TEST: fprintf(stderr,"aa=%g\tbb=%g\tjj=%ld\tkk=%ld\tnn=%ld\n",aa,bb,jj,kk,nn);
				/* calculate the AUC after imposing an offset (jj) on the data */
				x= xf_auc1_d((data1+jj),nn,setdelta,setref,result_d,message);
				if(x!=0) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message); exit(1);}
				if(setavg==0) auc=result_d[0];
				else auc=result_d[0]/nn;
			}
			/* output the AUC results */
			if(setformat==1) printf("%ld\t%ld\t%g\t%g\t%ld\t%g\n", nlines,zone,aa,bb,nn,auc);
			else printf("\t%g",auc);
			zone++;
		}
		if(setformat==2) printf("\n");
		nlines++;
 	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/********************************************************************************/
	/* FREE MEMORY AND EXIT*/
	/********************************************************************************/
	if(line!=NULL) free(line);
	if(pwords!=NULL) free(pwords);
	free(data1);
	free(index1);
	exit(0);
}
