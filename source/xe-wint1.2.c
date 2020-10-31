#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-wint1"
#define TITLE_STRING thisprog" v 2: JRH, 10 October 2010"
#define MAXLINE 1000

/*
<TAGS>math signal_processing</TAGS>
*/
/* external functions start */
double *xf_wint1(double *time, int *group, long n, int g1, int g2, float winsize, long *result_l);
double *xf_wint2_di(double *time, int *group, long n, int g1, int g2, double *win1, double *win2, long nwin, long *result_l);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char line[MAXLINE],word[MAXLINE],*pline;
	int w,x,y,z,result_i[32];
	int sizeofint=sizeof(int),sizeoffloat=sizeof(float), sizeofdouble=sizeof(double);
	long ii,jj,kk,nn,result_l[32];
	float a,b,c,result_f[32];
	double aa,bb,cc;
	FILE *fpin,*fpout;
	/* program-specific variables */
	long nwin=0;
	int *group=NULL;
	double *time=NULL,*intervals=NULL, *winstart=NULL, *winend=NULL, halftime=0;
	/* arguments */
	char *infile=NULL, *winfile=NULL;
	int setg1=1,setg2=2,setwinfile=0;
	double setwinsize=1.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculates intervals between 2 events (g1, g2) in time-windows\n");
		fprintf(stderr,"Windows can be: \n");
		fprintf(stderr,"	- specified in a file (use -wf option)\n");
		fprintf(stderr,"	- event-centred and of a specified width (use -w option)\n");
		fprintf(stderr,"If -g1 and -g2 are the same, output is suitable for an autocorellogram\n");
		fprintf(stderr,"NOTE: event times must be in ascending  order\n");
		fprintf(stderr,"- USAGE:\n");
		fprintf(stderr,"	%s [input] [optional arguments]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"		- 2 columns: 1st= time, 2nd= event-id (integer)\n");
		fprintf(stderr,"- valid optional arguments...\n");
		fprintf(stderr,"	-g1: class-1 reference event identifier - default=%d\n",setg1);
		fprintf(stderr,"	-g2: class-2 event identifier - default=%d\n",setg2);
		fprintf(stderr,"	-w: windowsize in seconds - default=%f\n",setwinsize);
		fprintf(stderr,"		- number of windows = number of events in class g1\n");
		fprintf(stderr,"		- interval range is +- winsize/2, because windows are event-centred\n");
		fprintf(stderr,"	-wf: name a file listing window start and end times - unset by default\n");
		fprintf(stderr,"		- number of windows = number of lines in the window file\n");
		fprintf(stderr,"		- interval range is +- winsize\n");
		fprintf(stderr,"		NOTE: setting this variable overrides -w\n");
		fprintf(stderr,"		NOTE: windows must be sequential and non-overlapping\n");
		fprintf(stderr,"- examples:\n");
		fprintf(stderr,"	%s data.txt -wf list.txt -g1 6 -g2 7 \n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -w 0.1 -g1 4 -g2 4 \n",thisprog);
		fprintf(stderr,"- output:\n");
		fprintf(stderr,"	time intervals (s) between events of each group\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile=argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if(ii>=argc) break;
			else if(strcmp(argv[ii],"-g1")==0) setg1= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-g2")==0) setg2= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-w")==0)  setwinsize= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-wf")==0) { winfile= argv[++ii]; setwinfile=1; }
			else {fprintf(stderr,"Error[%s]: invalid argument \"%s\"\n",thisprog,argv[ii]); exit(1);}
	}}


	/* STORE EVENT TIMES & CLASSES - FROM FILE OR STANDARD-INPUT IF INFILE=STDIN */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"Error[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINE,fpin)!=NULL) {
		if(sscanf(line,"%lf %d",&aa,&x)==2) {
			if(x==setg1 || x==setg2) {
				time=(double *)realloc(time,(nn+1)*sizeofdouble);
				if(time==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog);exit(1);}
				time[nn]=aa;
				group=(int *)realloc(group,(nn+1)*sizeofint);
				if(group==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog);exit(1);}
				group[nn]=x;
				nn++;
	}}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(nn<=0){fprintf(stderr,"--- Warning [%s]: no valid data!\n",thisprog);exit(0);}
	/* CHECK THAT EVENT TIMES ARE SEQUENTIAL */
	for(ii=1;ii<nn;ii++) {if(time[ii]<time[ii-1]) {fprintf(stderr,"\n--- Error[%s]: \"%s\" has a non-sequential time at sample %ld\n\n",thisprog,infile,ii);exit(1);}}

	/* IF SETWINFILE==0, USE XF_WINT1: EVENT-CENTRED WINDOWS OF SIZE SPECIFIED BY -W */
	if(setwinfile==0) {
		// call xf_wint1 to build the list of intervals
		intervals= xf_wint1(time,group,nn,setg1,setg2,setwinsize,result_l);
		kk= result_l[0];
	}

	/* IF SETWINFILE==1, USE XF_WINT2: WINDOW START-STOP TIMES READ FROM A FILE */
	else if(setwinfile==1) {
		pline= line;
		// Store the windows
		if((fpin=fopen(winfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: window list file \"%s\" not found\n\n",thisprog,winfile);exit(1);}
		while(!feof(fpin)) {
			if(fscanf(fpin,"%lf %lf",&aa,&bb)!=2) {
				if(fscanf(fpin,"%s",pline)!=1) {fprintf(stderr,"\n--- Error[%s]: line-scan error\n\n",thisprog);exit(1);};
				continue;
			}
			winstart= (double *) realloc(winstart,(nwin+1)*sizeofdouble);
			winend= (double *) realloc(winend,(nwin+1)*sizeofdouble);
			if(winstart==NULL || winend==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			winstart[nwin]= aa;
			winend[nwin]= bb;
			nwin++;
		}
		fclose(fpin);
		// check that the windows are sequential and non-overlapping
		for(ii=1;ii<nwin;ii++) {if(winstart[ii]<winstart[ii-1] || winstart[ii]<winend[ii-1]) {fprintf(stderr,"\n--- Error[%s]: \"%s\" has a non-sequential or overlapping window at line %ld\n\n",thisprog,winfile,ii);exit(1);}}

		// call xf_wint2 to build the list of intervals
		intervals= xf_wint2_di(time,group,nn,setg1,setg2,winstart,winend,nwin,result_l);
		kk= result_l[0];
		free(winstart); free(winend);
	}

	/* OUTPUT LIST OF INTERVALS */
	if(kk<1) {fprintf(stderr,"** Warning [%s]: total intervals detected=%ld\n",thisprog,kk);exit(0);}
	for(ii=0;ii<kk;ii++) printf("%.9f\n",intervals[ii]);

	free(time);
	free(group);
	free(intervals);
	exit(0);
}
