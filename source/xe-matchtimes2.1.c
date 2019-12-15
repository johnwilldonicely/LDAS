#define thisprog "xe-matchtimes2"
#define TITLE_STRING thisprog" v 1: 3 December 2012"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLINELEN 1000
#define MAXCOLS 16

/*
<TAGS>database screen</TAGS>
*/

/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {

	/* general and file type variables */
	char line[MAXLINELEN],padline[MAXLINELEN],*pline,*pline2,*pcol,*pcol2,*tempcol,templine[256];
	long ii,jj,kk,nn;
	int w,x,y,z,col,colmatch;
	int sizeofdouble=sizeof(double);
	float a,b,c;
	double aa,bb,cc;
	FILE *fpin=NULL,*fpout=NULL;
	/* program-specific variables */
	int nout=0,prevgood=0;
	long prevwindow=-1;
	double time=0.0,interval,*tstart=NULL,*tend=NULL;
	/* command line variables */
	char *infile=NULL,*listfile=NULL,*outfile=NULL,tempfile[MAXLINELEN];
	int dcol1=1,dcol2=-1,tcol1=1,tcol2=2,setpause=0,setpad=0;
	double setfreq=-1.0,setzero=0.0;


	sprintf(tempfile,"temp_%s",thisprog);

	/* PRINT INSTRUCTIONS IF ONLY ONE ARGUMENT */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract lines from a data file falling between start-stop times in timefile\n");
		fprintf(stderr,"Output is to a separate file for each window\n");
		fprintf(stderr,"Data may include a time-column or fixed sample-freq. can be assumed\n");
		fprintf(stderr,"Note that if windows overlap, earlier window \"grabs\" overlapping portion\n");
		fprintf(stderr,"USAGE: \n");
		fprintf(stderr,"	%s [infile] [timefile] [options]\n",thisprog);
		fprintf(stderr,"\n");
		fprintf(stderr,"		[infile]: data file with or without time-column\n");
		fprintf(stderr,"		[timefile]: file containing start and stop times (s)\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-d1: time column in infile [%d]\n",dcol1);
		fprintf(stderr,"	-sf: sample-freq. (Hz) of input - assumes constant interval [%g]\n",setfreq);
		fprintf(stderr,"		NOTE : if >0, this will override -d1\n");
		fprintf(stderr,"	-z: set time of first sample (only used if -sf >0) [%g]\n",setfreq);
		fprintf(stderr,"	-t1: start-time column in timefile [%d]\n",tcol1);
		fprintf(stderr,"	-t2: end-time column in timefile [%d]\n",tcol2);
		fprintf(stderr,"EXAMPLE:\n");
		fprintf(stderr,"	%s a.txt times.txt -d1 1 \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	- a chunk of the data file for each window\n");
		fprintf(stderr,"	- files are named %s_[window#].txt\n",tempfile);
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	listfile= argv[2];
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-out")==0) outfile= argv[++ii];
			else if(strcmp(argv[ii],"-d1")==0)  dcol1= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-sf")==0)  setfreq= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-z")==0)   setzero= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-t1")==0)  tcol1= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-t2")==0)  tcol2= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	prevgood= 0;
	prevwindow= -1;
	interval = 1.0/setfreq;
	time= setzero-interval;

	/* READ LISTFILE AND STORE POSSIBLE MATCH VALUES FOR EACH COLUMN */
	if((fpin=fopen(listfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,listfile);exit(1);}
	nn= 0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(line[0]=='#') continue;
		pline=line; colmatch=2; // number of columns to match
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			if(col==tcol1) {z=sscanf(pcol,"%lf",&aa); if(z==1) colmatch--;} // store value - check if input was actually a number
			if(col==tcol2) {z=sscanf(pcol,"%lf",&bb); if(z==1) colmatch--;} // store value - check if input was actually a number
		}
		if(colmatch!=0) continue;
		if((tstart=(double *)realloc(tstart,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((tend=(double *)realloc(tend,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		tstart[nn]= aa;
		tend[nn]= bb;
		nn++;
	}
	fclose(fpin);

	/* IF LIST FILE IS EMPTY, GIVE WARNING AND EXIT */
	if(nn<1) { fprintf(stderr,"\n--- Warning [%s]: no valid time-windows in list file %s\n\n",thisprog,listfile);free(tstart);free(tend);exit(0);}
	if(nn>256) { fprintf(stderr,"\n--- Error [%s]: %d windows specified in %s - exceeds max (256)\n\n",thisprog,nn,listfile);free(tstart);free(tend);exit(0);}


	/* READ DATA FILE AND OUTPUT MATCHING LINES*/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	fprintf(stderr,"\n--------------------------------------------------------------------------------\n");
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		strcpy(templine,line);
		kk= -1;
		pline= line;
		if(setfreq<=0) {
			for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
				pline=NULL;
				if(col==dcol1 && sscanf(pcol,"%lf",&aa)==1) {for(ii=0;ii<nn;ii++) if(aa>=tstart[ii] && aa<=tend[ii]) {kk =ii;break;}}
		}}
		else {
			time+=interval;
			for(ii=0;ii<nn;ii++) if(time>=tstart[ii] && time<=tend[ii]) {kk= ii;break;}
		}

		// if time falls within a time-window, output the line to the current temporary file
		if(kk>=0) {
			if(prevgood==1 && kk!= prevwindow) {
				fclose(fpout);
				fprintf(stderr,"%s\n",outfile);
				prevgood=0;
			}
			if(prevgood==0) {
				sprintf(outfile,"%s_%03ld.txt",tempfile,kk);
				if ((fpout=fopen(outfile,"w"))==NULL) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile);exit(1);}
				prevgood=1; prevwindow=kk; nout++;
			}
			fprintf(fpout,"%s",templine);
		}

		// if time did not fall within any window range continue, or pad as necessary
		else {
			if(prevgood==1) { fclose(fpout); fprintf(stderr,"%s\n",outfile); }
			prevgood=0;
		}
	}
	fclose(fpin);
	if(prevgood==1) { fclose(fpout); fprintf(stderr,"%s\n",outfile);}


	fprintf(stderr,"\n");
	fprintf(stderr,"Data output for %d of %d windows\n",nout,nn);
	if(setfreq>0) {
		fprintf(stderr,"Fixed sample frequency (%g) assumed\n",setfreq);
		fprintf(stderr,"First line presumed to be at time %g seconds\n",setzero);
	}
	fprintf(stderr,"--------------------------------------------------------------------------------\n\n");

	free(tstart);
	free(tend);
	exit(0);
	}
