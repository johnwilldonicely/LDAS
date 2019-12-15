#define thisprog "xe-curvestats1"
#define TITLE_STRING thisprog" v 5: 21.September.2017 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/*
<TAGS>stats signal_processing</TAGS>

v 5: 21.September.2017 [JRH]
	- replace use of size_t variales with long integer - more flexibility for future development

v 5: 4.August.2017 [JRH]
	- output NAN if input curve has no valid numbers in it

v 5: 21.January.2017 [JRH]
	- switch to using xf_auc1_d for AUC calculation
	- include more robust determination of stop for end-of-file (using nextafter)
	- NOTE that xe-curvestats2 uses a different approach and a function to get the indices

v 5: 02.February.2016 [JRH]
	- more robust free() operations
	- initialized pindex to NULL

v 4: 29.May.2015 [JRH]
	- fixed error in median calculation in xf_curvestats1_d
	- add option to output in keyword format
	- add error handling from xf_curvestats1_d

v 3: 13.May.2014 [JRH]
	- bugfix: now correctly defines indices (seconds) and index arrays (samples) when no indices are specified
	- slight change to curvestats function
		- no invalidation of ngative/min or positive/max results if there were no negative or positive values respectively
		- previously these would have treated as NAN
		- by default the aucn and aucp values will be zero if there are no negative or positive values
		- the minima and maxima are also valid, even if they dont fall on one side of the y-axis or the other

v 2: 7.March.2014 [JRH]
	- efficiency improvement - no need to check for setindices=NULL at the analysis stage because if this were the case the program had already created fake indices corresponding with the beginning and the end of the data

*/


/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_auc1_d(double *curvey, size_t nn, double interval, int ref, double *result ,char *message);
int xf_curvestats1_d(double *curvey, size_t nn, double interval, double *result_d, char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],line[MAXLINELEN],message[MAXLINELEN];
	long int i,j,k,n,baddata;
	long ii,jj,kk,nn;
	int v,w,x,y,z,col,colmatch;
	int sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	long nindices,*pindex=NULL,zone;
	double *data1=NULL,interval,start,stop,result_d[16];
	double auc,aucn,aucp,xn,xp,yn,yp,median,com;
	long *index1=NULL,pre2;
	/* arguments */
	char *setindices=NULL;
	int setasc=1,setformat=1,setref=0;
	double setsampfreq=1.0,setpre=0.0,setdelta=-1.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate area under the curve & other stats for single-column input\n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"This version only uses the x-values to set inclusion limits\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-sf: sample frequency (sample/s) [%f]\n",setsampfreq);
		fprintf(stderr,"	-pre: presample period [%g]\n",setpre);
		fprintf(stderr,"	-index: comma-separated start/stop pairs [%s]\n",setindices);
		fprintf(stderr,"		- if -sf is set to \"1\", -pre and index are in samples\n");
		fprintf(stderr,"		- otherwise, -pre and index are in seconds\n");
		fprintf(stderr,"	-d: delta value to use for integration [default=1/sf]\n");
		fprintf(stderr,"	-f: output format (0=line, 1=line+header,2=keywords) [%d]\n",setformat);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txxt -sf 200 -pre .5 -d 1 -index 0,.05,.05,.17\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -sf 1000 -pre 0\n",thisprog);
		fprintf(stderr,"\n");
		fprintf(stderr,"OUTPUT: format 1\n");
		fprintf(stderr,"	zone start stop n AUC AUCn Xn Yn AUCp Xp Yp median COM\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"OUTPUT: format 2\n");
		fprintf(stderr,"	[zone_keyword] [value]\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-sf")==0) setsampfreq=atof(argv[++i]);
			else if(strcmp(argv[i],"-pre")==0) setpre=atof(argv[++i]);
			else if(strcmp(argv[i],"-index")==0) setindices=argv[++i];
			else if(strcmp(argv[i],"-d")==0) setdelta=atof(argv[++i]);
			else if(strcmp(argv[i],"-f")==0) setformat=atoi(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}
	if(setsampfreq<=0) { fprintf(stderr,"\n--- Error[%s]: sample frequency (-sf %g) must be greater than 0 \n\n",thisprog,setsampfreq); exit(1);}
	if(setpre<0) { fprintf(stderr,"\n--- Error[%s]: presample period (-pre %g) must be 0 or greater\n\n",thisprog,setpre); exit(1);}
	if(setformat<0||setformat>2) {fprintf(stderr,"\n--- Error[%s]: invalid output format (-f %d) - must be 0 or 1\n\n",thisprog,setformat); exit(1);}

	interval=1.0/setsampfreq;
	if(setdelta<=0) setdelta=interval;


	/* READ THE INPUT DATA */
	baddata= 0;
	nn= 0;
	if(setasc==1) {
		/* read stream of ASCII numbers */
		if(strcmp(infile,"stdin")==0) fpin=stdin;
		else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
 			if(sscanf(line,"%lf",&aa)!=1 || !isfinite(aa)) { aa=NAN; baddata++; }
 			if((data1=(double *)realloc(data1,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
 			data1[nn++]=aa;
 		}
		if(strcmp(infile,"stdin")!=0) fclose(fpin);
	}

	/* DETERMINE THE PRESAMPLE OFFSET IN SAMPLES (PRE2) */
	pre2= (long) (setpre * setsampfreq);
	if(pre2>=(nn-1)) {fprintf(stderr,"\n--- Error[%s]: presample period (-pre %g) is >= the length of the data\n\n",thisprog,setpre); exit(1);}

	/* PARSE THE INDICES, OR DETERMINE BASED ON nn */
	if(setindices!=NULL) {
		/* determine number of indices and pointers to times */
		pindex= xf_lineparse2(setindices,",",&nindices);
		/* assuming specified indices are in pairs... */
		if(nindices%2==0) {
			/* allocate memory for indices */
			if((index1=(long *)realloc(index1,(nindices*sizeof(long))))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			for(ii=0;ii<nindices;ii++) {
				/* convert index text to number */
				if(sscanf((setindices+pindex[ii]),"%lf",&aa)!=1 || !isfinite(aa))  {fprintf(stderr,"\n--- Error[%s]: non-numeric index (%s) specified\n\n",thisprog,(setindices+pindex[ii]));exit(1);};
				/* increase aa for end-samples ever so slightly */
				if(aa!=0.0 && ii%2!=0) aa=nextafter(aa,DBL_MAX);
				/* make sure this number fall within appropriate bounds */
				if(aa<(-setpre)) {fprintf(stderr,"\n--- Error[%s]: index (%g) preceeds the presample period (%g)\n\n",thisprog,aa,setpre);exit(1);}
				/* create an index */
				index1[ii]= pre2 + (long) (setsampfreq * aa);
				if(index1[ii]>nn) {fprintf(stderr,"\n--- Error[%s]: index #%ld (%g) is too large for data (%ld samples at %gHz) with a %gs presample period\n\n",thisprog,(ii+1),aa,nn,setsampfreq,setpre);exit(1);}
				/* make sure odd-numbered indices are greater than their preceeding partner */
				if(ii%2!=0 && index1[ii]<=index1[ii-1]) {fprintf(stderr,"\n--- Error[%s]: index #%ld (%g) is not sufficiently greater than the one preceding it\n\n",thisprog,(ii+1),aa);exit(1);}
			}
		}
		else { fprintf(stderr,"\n--- Error[%s]: %ld indices (-pairs) specified - must be an even number\n\n",thisprog,nindices); exit(1); }
	}
	else {
		aa= (0.0-setpre); // start time
		bb= (nn/setsampfreq)-setpre; // end time
		bb= nextafter(bb,DBL_MAX); // decrement very slightly
		/* build a fake setindices and pindex array */
		sprintf(line,"%g,%g",aa,bb);
		setindices=line;
		pindex= xf_lineparse2(setindices,",",&nindices);
		/* determine the one and only start/stop index pair */
		if((index1=(long *)realloc(index1,(2*sizeof(long))))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		index1[0]= pre2 + (long) (setsampfreq * aa);
		index1[1]= pre2 + (long) (setsampfreq * bb);
	}

	if(setformat==1) printf("zone\tstart\tstop\tn\tAUC\tAUCn\tXn\tYn\tAUCp\tXp\tYp\tmedian\tCOM\n");

	/* CALCULATE STATS */
	if(baddata==0) {
		zone=1;
		for(ii=0;ii<nindices;ii+=2) {
			start= atof((setindices+pindex[ii]));
			stop=  atof((setindices+pindex[ii+1]));

			/* temporary length of data defined by current and next index */
			kk= index1[ii+1]-index1[ii];

			/* calculate the AUC stats after imposing an offset on the data */
			x= xf_auc1_d((data1+index1[ii]),(size_t)kk,setdelta,setref,result_d,message);
			if(x!=0) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message); exit(1);}
			auc=result_d[0];
			aucp=result_d[1];
			aucn=result_d[2];

			/* calculate additional stats after imposing an offset on the data */
			x= xf_curvestats1_d((data1+index1[ii]),(size_t)kk,setdelta,result_d,message);
			if(x!=0) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message); exit(1);}
			xn=result_d[0];
			yn=result_d[1];
			xp=result_d[2];
			yp=result_d[3];
			median=result_d[4];
			com=result_d[5];

			/* convert peak/trough/median/com sample-numbers to times */
			aa= (interval*index1[ii])-setpre; // adjustment (seconds) to apply
			if(isfinite(xn)) xn= xn+aa;
			if(isfinite(xp)) xp= xp+aa;
			median= median+aa;
			com= com+aa;

			if(setformat==1) {
				printf("%ld\t%g\t%g\t%ld\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
					zone,start,stop,kk,auc,aucn,xn,yn,aucp,xp,yp,median,com);
			}
			else if(setformat==2) {
				printf("%ld_start %g\n",zone,start);
				printf("%ld_stop %g\n",zone,stop);
				printf("%ld_n %ld\n",zone,kk);
				printf("%ld_AUC %g\n",zone,auc);
				printf("%ld_AUCn %g\n",zone,aucn);
				printf("%ld_Xn %.6f\n",zone,xn);
				printf("%ld_Yn %g\n",zone,yn);
				printf("%ld_AUCp %g\n",zone,aucp);
				printf("%ld_Xp %g\n",zone,xp);
				printf("%ld_Yp %g\n",zone,yp);
				printf("%ld_median %g\n",zone,median);
				printf("%ld_com %g\n",zone,com);
				printf("\n");
			}
			zone++;
		}
	}
	else {
		zone=1;
		for(ii=0;ii<nindices;ii+=2) {
			start= atof((setindices+pindex[ii]));
			stop=  atof((setindices+pindex[ii+1]));
			kk= index1[ii+1]-index1[ii];
			if(setformat==1) {
				printf("%ld\t%g\t%g\t%ld\tnan\tnan\tnan\tnan\tnan\tnan\tnan\tnan\tnan\n",zone,start,stop,kk);
			}
			else if(setformat==2) {
				printf("%ld_start %g\n",zone,start);
				printf("%ld_stop %g\n",zone,stop);
				printf("%ld_n %ld\n",zone,kk);
				printf("%ld_AUC %g\n",zone,NAN);
				printf("%ld_AUCn %g\n",zone,NAN);
				printf("%ld_Xn %.6f\n",zone,NAN);
				printf("%ld_Yn %g\n",zone,NAN);
				printf("%ld_AUCp %g\n",zone,NAN);
				printf("%ld_Xp %g\n",zone,NAN);
				printf("%ld_Yp %g\n",zone,NAN);
				printf("%ld_median %g\n",zone,NAN);
				printf("%ld_com %g\n\n",zone,NAN);
			}
			zone++;
		}
	}

	if(data1!=NULL) free(data1);
	if(index1!=NULL) free(index1);
	if(pindex!=NULL) free(pindex);
	exit(0);
}
