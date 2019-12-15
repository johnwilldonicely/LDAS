#define thisprog "xe-matchtimes1"
#define TITLE_STRING thisprog" v 6: 11.February.2014 [JRH]"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> /* must be included for directory reading */
#include <sys/stat.h> /* must be included for directory reading */
#include <math.h>

#define MAXLINELEN 1000
#define MAXCOLS 16

/*
<TAGS>database screen</TAGS>

v 6: 11.February.2014 [JRH]
	- simplified and thread-safe implimentation, using xf_lineparse1 to manage words
	- add ability to do inverse-matching (omit data lines falling within time ranges time-file)
	- ??? output MAY be faster for matching lines without padding by using strcpy - test?

v 5: 3.Decmber.2012 [JRH]
	- allow stdin input for datafile
	- remove refernces to hux_error and hux_getpath functions
	- rename from xe-matchtimes to xe-matchtimes1
*/


/* external functions start */
long *xf_lineparse1(char *line,long *nwords);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general and file type variables */
	char line[MAXLINELEN],padline[MAXLINELEN],*pline,*pline2,*pcol,*pcol2,*tempcol,temp_str[256];
	long i,j,k,n;
	int w,x,y,z,col,colmatch;
	int sizeofdouble=sizeof(double);
	float a,b,c;
	double aa,bb,cc;
	FILE *fpin,*fpout;
	/* program-specific variables */
	char infile[256],listfile[256],outfile[256]="stdout",outfile2[256],padstr[64];
	long nwords=0,*start=NULL;
	double *tstart=NULL,*tend=NULL;
	/* command line variables */
	int dcol1=1,dcol2=-1,tcol1=1,tcol2=2,setpause=0,setpad=0,setinv=-1;

	sprintf(padstr,"");

	/* Print instructions if only one argument */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract lines from a data file falling between start-stop times in timefile\n");
		fprintf(stderr,"infile may specify time in 1 or 2 columns times per line\n");
		fprintf(stderr,"Two columns specify a range which must fall within those in the timefile\n");
		fprintf(stderr,"USAGE: \n");
		fprintf(stderr,"	%s [datafile] [timefile] [options]\n",thisprog);
		fprintf(stderr,"		[datafile]: filename or \"stdin\" containing time+data\n",thisprog);
		fprintf(stderr,"		[timefile]: filename containing start-stop times\n",thisprog);
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-d1: time column in infile [%d]\n",dcol1);
		fprintf(stderr,"	-d2: optional end-time column in infile (-1 = ignore) [%d]\n",dcol2);
		fprintf(stderr,"	-t1: start-time column in timefile [%d]\n",tcol1);
		fprintf(stderr,"	-t2: end-time column in timefile [%d]\n",tcol2);
		fprintf(stderr,"	-inv: inverse match - OMIT matching lines (-1=NO,1=YES) [%d]\n",setinv);
		fprintf(stderr,"	-pad: define a pad string (eg \"-1\") for out-of-range lines\n",padstr);
		fprintf(stderr,"		if undefined, out-of-range lines are omitted\n",padstr);
		fprintf(stderr,"		NOTE: padding only affects non-time columns, so if infile\n");
		fprintf(stderr,"		only has time columns, padding will not be apparent!\n");
		fprintf(stderr,"	-out destination filename or stdout or stderr (%s)\n",outfile);
		fprintf(stderr,"EXAMPLE:\n");
		fprintf(stderr,"	%s a.txt times.txt -d1 1 -d2 -1 -out results.txt\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	[tab]-delimited contents of each line meeting time-criteria\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	sprintf(listfile,"%s",argv[2]);
	for(i=3;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-out")==0) {strcpy(outfile,argv[i+1]);i++;}
			else if(strcmp(argv[i],"-d1")==0) {dcol1=atoi(argv[i+1]);i++;}
			else if(strcmp(argv[i],"-d2")==0) {dcol2=atoi(argv[i+1]);i++;}
			else if(strcmp(argv[i],"-t1")==0) {tcol1=atoi(argv[i+1]);i++;}
			else if(strcmp(argv[i],"-t2")==0) {tcol2=atoi(argv[i+1]);i++;}
			else if(strcmp(argv[i],"-inv")==0) {setinv=atoi(argv[i+1]);i++;}
			else if(strcmp(argv[i],"-pad")==0) {strcpy(padstr,argv[i+1]);setpad=1;i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(setinv!=-1 && setinv!=1) {fprintf(stderr,"\n--- Error[%s]: inverse-match flag (-inv %d) must be set to -1 or 1\n\n",thisprog,setinv); exit(1);}

	/* READ LISTFILE AND STORE POSSIBLE MATCH VALUES FOR EACH COLUMN */
	if((fpin=fopen(listfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,listfile);exit(1);}
	n=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(line[0]=='#') continue;
		pline=line; colmatch=2; // number of columns to match
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			if(col==tcol1) {z=sscanf(pcol,"%lf",&aa); if(z==1) colmatch--;} // store value - check if input was actually a number
			if(col==tcol2) {z=sscanf(pcol,"%lf",&bb); if(z==1) colmatch--;} // store value - check if input was actually a number
		}
		if(colmatch!=0) continue;
		if((tstart=(double *)realloc(tstart,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((tend=(double *)realloc(tend,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		tstart[n]=aa;
		tend[n]=bb;
		n++;
	}
	fclose(fpin);


	/* IF LIST FILE IS EMPTY, GIVE WARNING AND EXIT */
	if(n<1) {
		if(setinv==-1) { fprintf(stderr,"\n--- Warning[%s]: no valid time-windows in list file %s\n\n",thisprog,listfile);free(tstart);free(tend);exit(0);}
	}

	/* DECREMENT DCOL1 AND DCOL2 BECAUSE COLUMN POINTERS FROM xf_lineparse1 WILL BE ZERO-OFFSET */
	dcol1--;
	dcol2--;

	/* OPEN OUTPUT FILE OR PREPARE TO WRITE TO SCREEN OR STDERR... */
	if(strcmp(outfile,"stdout")==0) fpout=stdout;
	else if(strcmp(outfile,"stderr")==0) fpout=stderr;
	else {
		sprintf(outfile2,"%s",outfile);
		if ((fpout=fopen(outfile2,"w"))==NULL) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile2);exit(1);}
	}

	/* READ DATA FILE AND OUTPUT MATCHING LINES*/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	/* THIS LOOP CHECKS FOR SINGLE TIMES IN THE DATA FILE WHICH FALL WITHIN RANGES IN THE TIMEFILE */
	if(dcol2<0) {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			z=setinv;
			start= xf_lineparse1(line,&nwords);
			if(nwords>=dcol1) {
				if(sscanf(line+start[dcol1],"%lf",&aa)==1) {
					if(isfinite(aa)) {
						for(i=0;i<n;i++) { if(aa>=tstart[i] && aa<=tend[i]) {z*=-1;break;} }
			}}}
			// if time did not fall within any window range continue, or pad as necessary
			if(z>0) { fprintf(fpout,"%s",line+start[0]); for(i=1;i<nwords;i++) fprintf(fpout,"\t%s",line+start[i]);	fprintf(fpout,"\n"); }
			// if time did not fall within any window range continue, or pad as necessary
			else if(setpad==1) {fprintf(fpout,"%s",padstr); for(i=1;i<nwords;i++) fprintf(fpout,"\t%s",padstr); fprintf(fpout,"\n"); }
	}}
	/* THIS LOOP CHECKS FOR RANGES IN THE DATA FILE WHICH FALL WITHIN RANGES IN THE TIMEFILE */
	else {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			z=setinv;
			start= xf_lineparse1(line,&nwords);
			if(nwords>=dcol1 && nwords>=dcol2) {
				if(sscanf(line+start[dcol1],"%lf",&aa)==1 && sscanf(line+start[dcol2],"%lf",&bb)==1) {
					if(isfinite(aa) && isfinite(bb)) {
						for(i=0;i<n;i++) { if(aa>=tstart[i] && bb>=tstart[i] && aa <=tend[i] && bb <=tend[i]) {z*=-1;break;} }
			}}}
			// if time did not fall within any window range continue, or pad as necessary
			if(z>0) { fprintf(fpout,"%s",line+start[0]); for(i=1;i<nwords;i++) fprintf(fpout,"\t%s",line+start[i]);	fprintf(fpout,"\n"); }
			// if time did not fall within any window range continue, or pad as necessary
			else if(setpad==1) {fprintf(fpout,"%s",padstr); for(i=1;i<nwords;i++) fprintf(fpout,"\t%s",padstr); fprintf(fpout,"\n"); }
	}}


	fclose(fpin);
	fclose(fpout);

	free(tstart);
	free(tend);
	free(start);
	exit(0);

	}
