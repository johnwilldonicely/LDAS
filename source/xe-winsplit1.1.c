#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

#define thisprog "xe-winsplit1"
#define TITLE_STRING thisprog" v 1: 24.February.2014 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>file signal_processing</TAGS>

v 1: 21.August.2018 [JRH]
	- add option to handle long-integers
v 1: 24.February.2014 [JRH]
	- original
*/

/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],line[MAXLINELEN];
	long ii,jj,kk;
	double aa,bb;
	FILE *fpin;

	/* program-specific variables */
	long lstart,lstop,lsplit,ldur;
	double start,stop;
	/* arguments */
	int settype=8;
	double setsplit=1.0,setdur=-1.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Split time-windows into smaller chunks\n");
		fprintf(stderr,"Can accept window start-stop pairs or just start-times if -d is set\n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"		* if -d =0, assumed format is [start] [stop] \n");
		fprintf(stderr,"		* if -d >0, assumed format is [start] (only reads first column)\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-dt: data type (7=long_integer, 8=double_precision_float) [%d]\n",settype);
		fprintf(stderr,"	-d: original duration of each window (if single-column input) [%g]\n",setdur);
		fprintf(stderr,"	-s: size of chunks to break windows down into [%g]\n",setsplit);
		fprintf(stderr,"		* best results if split is an integer fraction of the window size\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -d 100 -s 20\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -d -1 -s 1\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	if -d =0: single column of revised [start] times\n");
		fprintf(stderr,"	if -d >0: [start] [stop] pairs\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-dt")==0)  settype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-d")==0)   setdur=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-s")==0)   setsplit=atof(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(settype!=7 && settype!=8) { fprintf(stderr,"\n--- Error [%s]: invalid data type (-t %d) -  must be 7 or 8\n\n",thisprog,settype);exit(1);}

	/* MAKE LONG VERSIONS OF PARAMETERS FOR LONG-INTEGER OPERATIOS, IF NEEDED */
	ldur= (long)setdur;
	lsplit= (long)setsplit;

	/* STORE DATA - newline-delimited pairs of data */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	if(setdur>0) {
		if(settype==7) {
			while(fgets(line,MAXLINELEN,fpin)!=NULL) {
				if(sscanf(line,"%ld",&lstart)!=1) continue;
				lstop= (lstart+ldur)-lsplit;
				for(kk=lstart;kk<=lstop;kk+=lsplit) printf("%ld\n",kk);
			}
		}
		else if(settype==8) {
			while(fgets(line,MAXLINELEN,fpin)!=NULL) {
				if(sscanf(line,"%lf",&start)!=1) continue;
				if(isfinite(start)) {
					stop= (start+setdur)-setsplit;
					for(aa=start;aa<=stop;aa+=setsplit) printf("%f\n",aa);
			}}
		}
	}
	else {
		if(settype==7) {
			while(fgets(line,MAXLINELEN,fpin)!=NULL) {
				if(sscanf(line,"%ld %ld",&lstart,&lstop)!=2) continue;
				lstop-= lsplit;
				for(kk=lstart;kk<=lstop;kk+=lsplit) printf("%ld\t%ld\n",kk,kk+lsplit);
			}
		}
		else if(settype==8) {
			while(fgets(line,MAXLINELEN,fpin)!=NULL) {
				if(sscanf(line,"%lf %lf",&start,&stop)!=2) continue;
				if(isfinite(start) && isfinite(stop)) {
					stop= stop-setsplit;
					for(aa=start;aa<=stop;aa+=setsplit) printf("%f	%f\n",aa,aa+setsplit);
			}}
		}
	}


	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	exit(0);
	}
