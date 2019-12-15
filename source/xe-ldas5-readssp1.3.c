#define thisprog "xe-ldas5-readssp1"
#define TITLE_STRING thisprog" v 3: 15.March.2019 [JRH]"
#define MAXLINELEN 1000
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>  /* this and next header allow testing file exists using stat() function */
#include <errno.h>

/************************************************************************
<TAGS>time</TAGS>
v 3: 15.March.2019 [JRH]
	- add output option -2 (summary only) for n_ssp and sample-count
	- allow definition of new first (-a) and/or last (-z) SSP

v 3: 23.August.2018 [JRH]
	- allow output of gaps between SSPS (-inv)
	- allow definition of new first (-a) and/or last (-z) SSP

v 3: 31.August.2016 [JRH]
	- add option to output compressed-SSPs
		- say file chunks were read using the input SSP and output together
		- this option would indicate the output samples corresponding to the original SSPs
v 3: 19.July.2016 [JRH]
	- switch to reading SSPs into memory using xf_readssp1
	- add scaling modification to SSPs to allow referencing to downsampled data
	- eg. original SSPs may refer to a 20KHz .dat file, but you may need to refer to a 1KHz .bin file or 1 Hz spectrum file

3: 8.March.2016 [JRH]
	- renamed to xe-ldas5-readssp1 (previously just xe-ldas5-ssp1)

v 3: 20.November.2015 [JRH]"
	- change "screen" keywords to avoid confusion with commonly used -sf (sample frequency)
	- change "screen" keywords to avoid confusion with commonly used -sf (sample frequency)
		-s becomes -scr
		-sf becomes -scrf
		-sl becomes -scrl

v 2: 1.September.2015 [JRH]
	- use new xf_readssp1 function to read pairs file
*************************************************************************/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
long xf_screen_ssp1(long *start1, long *stop1, long nssp1, long *start2, long *stop2, long nssp2, int mode, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],message[256];
	int sizeoflong=sizeof(long);
	long int ii,jj,kk,mm,nn;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[64];
	FILE *fpin,*fpout;
	struct stat sts;

	/* program-specific variables */
	long *data=NULL;
	long headerbytes=0,maxread,blocksize;
	long *index=NULL,*start1=NULL,*stop1=NULL,*start2=NULL,*stop2=NULL;
	long nssp,nscreen,firststart,laststop,totsamps=0;


	/* arguments */
	char *setscreenfile=NULL,*setscreenlist=NULL;
	int setout=-1,setverb=0,setscreen=0,setcom=0,setinv=0;
	off_t setheaderbytes=0;
	long setwin=-1,seta=-1,setz=-1,setsplit=0;
	double setdiv=1.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Process binary start-stop-pair (.ssp) files\n");
		fprintf(stderr,"- output is either converted to ASCII or kept in binary form\n");
		fprintf(stderr,"- use a file or list of boundaries to screen the start-stop pairs\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: binary file containing start-stop pairs, or \"stdin\"\n");
		fprintf(stderr,"		- pairs are 64-bit long integers (signed)\n");
		fprintf(stderr,"		- pairs generally refer to sample-numbers\n");
		fprintf(stderr,"VALID OPTIONS: operations shown in execution-order defaults in []:\n");
		fprintf(stderr,"	-scr: screen using start-stop boundary pairs [%d]\n",setscreen);
		fprintf(stderr,"		0: no screening (reset to 1 if -scrf or -scrl is set)\n");
		fprintf(stderr,"		1: each pair must fall within one of tinvhe listed bounds\n");
		fprintf(stderr,"		2: each pair must not span any of the listed bounds\n");
		fprintf(stderr,"	-scrf: screen-file (binary ssp) defining bounds for infile1 [unset]\n");
		fprintf(stderr,"	-scrl: screen-list (CSV) defining bounds for infile1 [unset]\n");
		fprintf(stderr,"	-inv: invert SSPs, returning gaps only (0=NO 1=YES) [%d]\n",setinv);
		fprintf(stderr,"	-a: start for a new first-SSP (-1=none) [%ld]\n",seta);
		fprintf(stderr,"		- adds a record\n");
		fprintf(stderr,"		- uses pre-inversion start[0] as new stop[0]\n");
		fprintf(stderr,"	-z: stop for a new last-SSP (-1=none) [%ld]\n",setz);
		fprintf(stderr,"		- adds a record\n");
		fprintf(stderr,"		- uses pre-inversion stop[n-1] as new start[n-1]\n");
		fprintf(stderr,"	-split: split SSPs into n-sample sub-SSPs (0=none) [%ld]\n",setsplit);
		fprintf(stderr,"	-div: SSPs divisor (allows alignment to downsampled data) [%g]\n",setdiv);
		fprintf(stderr,"		- NOTE: this is only applied at the output stage\n");
		fprintf(stderr,"	-com: compress SSPs (0=NO 1=YES) [%d]\n",setcom);
		fprintf(stderr,"		- indicates boundaries in a file merged using the input\n");
		fprintf(stderr,"	-out: output format [%d]:\n",setout);
		fprintf(stderr,"		-2= summary only (total SSP pairs and duration)\n");
		fprintf(stderr,"		-1= ASCII, one start-stop-pair per line\n");
		fprintf(stderr,"		 7= binary (long int), start-stop pairs in sequence\n");
		fprintf(stderr,"	-verb: set verbocity of output (0=low, 1=high) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s pairs.ssp -s 1 -sl 100,200,1500,1600,2350,2450\n",thisprog);
		fprintf(stderr,"	cat thetacycles.ssp | %s stdin -s 1 -sf trials.ssp\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	/************************************************************/
	sprintf(infile,"%s",argv[1]);
	if(strcmp(infile,"stdin")!=0 && stat(infile,&sts)==-1 && errno == ENOENT) {
		fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);
		exit(1);
	}
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-scr")==0)  setscreen=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-scrf")==0) setscreenfile=argv[++ii];
			else if(strcmp(argv[ii],"-scrl")==0) setscreenlist=argv[++ii];
			else if(strcmp(argv[ii],"-a")==0)    seta=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-z")==0)    setz=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-inv")==0)  setinv=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-split")==0)  setsplit=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-div")==0)  setdiv=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-com")==0)  setcom=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)  setout=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setscreen<0 || setscreen>2) {fprintf(stderr,"\n--- Error[%s]: invalid -scr (%d) : must be 0-2\n\n",thisprog,setscreen);exit(1);}
	if(setscreen>0 && setscreenfile==NULL && setscreenlist==NULL) {fprintf(stderr,"\n--- Error[%s]: must define a list (-sl) or file (-sf) if screening is set (-s %d)\n\n",thisprog,setscreen);exit(1);}
	if(setscreenlist!=NULL && setscreenfile!=NULL) {fprintf(stderr,"\n--- Error[%s]: cannot define both a screening file (-scrf) and a screening list (-scrl)\n\n",thisprog);exit(1);}
	if(setinv<0 || setinv>1) {fprintf(stderr,"\n--- Error[%s]: invalid -inv (%d) : must be 0-1\n\n",thisprog,setinv);exit(1);}
	if(setsplit<0) {fprintf(stderr,"\n--- Error[%s]: invalid -split (%ld) : must be >= 0\n\n",thisprog,setsplit);exit(1);}
	if(setcom<0 || setcom>1) {fprintf(stderr,"\n--- Error[%s]: invalid -com (%d) : must be 0-1\n\n",thisprog,setcom);exit(1);}
	if(setout!=-2 && setout!=-1 && setout!=7) {fprintf(stderr,"\n--- Error[%s]: invalid -out (%d) : must be -2, -1 or 7\n\n",thisprog,setout);exit(1);}
	if(setverb<0 || setverb>1) {fprintf(stderr,"\n--- Error[%s]: invalid -verb (%d) : must be 0-1\n\n",thisprog,setverb);exit(1);}


	/************************************************************/
	/* READ THE MAIN SSP FILE  */
	/************************************************************/
	nssp= xf_readssp1(infile,&start1,&stop1,message);
	if(nssp==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	if(nssp==0) {
		if(setverb==1) fprintf(stderr,"\n--- Warning[%s]: no SSP records in %s\n\n",thisprog,infile);
		goto END;
	}


	if(setverb==1) fprintf(stderr,"\nread %ld ssp records\n",nssp);

	/************************************************************/
	/* READ THE INCLUDE OR EXCLUDE LIST */
	/************************************************************/
	if(setscreenlist!=NULL) {
		if(setscreen==0) setscreen=1;
		index= xf_lineparse2(setscreenlist,",",&nscreen);
		if((nscreen%2)!=0) {fprintf(stderr,"\n--- Error[%s]: screen-list does not contain pairs of numbers: %s\n\n",thisprog,setscreenlist);exit(1);}
		nscreen/=2;
		start2= realloc(start2,nscreen*sizeof(*start2));
		stop2= realloc(stop2,nscreen*sizeof(*stop2));
		if(start2==NULL || stop2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<nscreen;ii++) {
			start2[ii]=atol(setscreenlist+index[2*ii]);
			stop2[ii]= atol(setscreenlist+index[2*ii+1]);
		}
		if(setverb==1) fprintf(stderr,"read %ld screening pairs\n\n",nscreen);
	}
	/************************************************************/
	/* ..OR, READ THE INCLUDE OR EXCLUDE FILE */
	/************************************************************/
	else if(setscreenfile!=NULL) {
		if(setscreen==0) setscreen=1;
		nscreen = xf_readssp1(setscreenfile,&start2,&stop2,message);
		if(nscreen==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		if(setverb==1) fprintf(stderr,"read %ld screening pairs\n\n",nscreen);
	}
	//for(ii=0;ii<nscreen;ii++) printf("%ld	%ld	%ld\n",ii,start2[ii],stop2[ii]);exit(0);


	/************************************************************/
	/* SCREEN THE SSPS USING THE SCREEN LIST */
	/************************************************************/
	if(setscreen!=0) {
		kk= xf_screen_ssp1(start2,stop2,nscreen,start1,stop1,nssp,setscreen,message);
		if(kk<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		else nssp=kk;
	}


	/************************************************************/
	/* SAVE THE ORIGINAL EXTENT OF SSPs, AFTER SCREENING */
	/* - this is important for data-inversion and use of -a and -z */
	/************************************************************/
	firststart= start1[0];
	laststop= stop1[(nssp-1)];


	/************************************************************/
	/* INVERT THE RECORDS, KEEPING THE GAPS BETWEEN THE SSPS */
	/************************************************************/
	if(setinv==1) {
		kk= nssp-1;
		for(ii=jj=0;ii<kk;ii++) {
			if(stop1[ii]<start1[(ii+1)]) {
				start1[jj]= stop1[ii];
				stop1[jj]= start1[(ii+1)];
				jj++;
			}
		}
		nssp= jj;
	}


	/************************************************************/
	/* ADD EXTRA SSP-PAIR INNCORPORATING THE FIRST AND/OR LAST PAIR */
	/************************************************************/
	if(seta>=0) {

		if(seta<=firststart) {
			nssp++;
			start1= realloc(start1,nssp*sizeof(*start1));
			stop1= realloc(stop1,nssp*sizeof(*stop1));
			if(start1==NULL || stop1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			for(ii=(nssp-1);ii>0;ii--) {
				start1[ii]= start1[(ii-1)];
				stop1[ii]= stop1[(ii-1)];
			}
			start1[0]= seta;     // start is the set-value
			stop1[0]= firststart; // stop is the beginning of the original first SSP

		}
		else if(setverb==1) fprintf(stderr,"--- Warning: start value ignored (-a %ld) - exceeds original first start-value (%ld)\n",seta,firststart);
	}
	if(setz>=0) {
		if(setz>laststop) {
			nssp++;
			start1= realloc(start1,nssp*sizeof(*start1));
			stop1= realloc(stop1,nssp*sizeof(*stop1));
			if(start1==NULL || stop1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			start1[(nssp-1)]= laststop; // start is the stop for the original last SSP
			stop1[(nssp-1)]= setz; // stop is the set value
		}
		else if(setverb==1) fprintf(stderr,"--- Warning: stop value ignored (-z %ld) - is not greater than original last stop-value (%ld)\n",setz,laststop);
	}


	/************************************************************/
	/* SPLIT THE WINDOWS */
	/************************************************************/
	if(setsplit!=0) {
		mm= 0; // counter for new (recycled) start2/stop2 arrays
		for(ii=0;ii<nssp;ii++) {
			/* set maximum start-time for the subdivisions of the original SSP */
			kk= stop1[ii]-setsplit;
			for(jj=start1[ii];jj<=kk;jj+=setsplit) {
				/* allocate memory */
				start2= realloc(start2,(mm+1)*sizeof(*start2));
				stop2= realloc(stop2,(mm+1)*sizeof(*stop2));
				if(start2==NULL || stop2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
				start2[mm]= jj;
				stop2[mm]= jj+setsplit;
				mm++;
			}
		}
		/* copy the new array back to the original */
		start1= realloc(start1,mm*sizeof(*start1));
		stop1= realloc(stop1,mm*sizeof(*stop1));
		if(start1==NULL || stop1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<mm;ii++) {
			start1[ii]= start2[ii];
			stop1[ii]= stop2[ii];
		}
		nssp= mm;
	}

	/************************************************************/
	/* APPLY THE DIVISOR */
	/************************************************************/
	if(setdiv!=1.0) {
		for(ii=0;ii<nssp;ii++) {
			start1[ii]= (long)(start1[ii]/setdiv);
			stop1[ii]= (long)(stop1[ii]/setdiv);
	}}

	/************************************************************/
	/* APPLY COMPRESSION  */
	/************************************************************/
	if(setcom==1) {
		kk=0; // running sum
		for(ii=0;ii<nssp;ii++) {
			jj=stop1[ii]-start1[ii];  // size of current block
			start1[ii]= kk;
			stop1[ii]= kk+jj;
			kk+= jj;
	}}



	/************************************************************/
	/* OUTPUT */
	/************************************************************/
	if(setout==-2) {
		for(ii=0;ii<nssp;ii++) totsamps+= (stop1[ii]-start1[ii]);
		printf("total_ssp_pairs= %ld\n",nssp);
		printf("total_duration= %ld samples\n",totsamps);

	}
	else if(setout==-1) {
		for(ii=0;ii<nssp;ii++) {
			printf("%ld	%ld\n",start1[ii],stop1[ii]);
			totsamps+= (stop1[ii]-start1[ii]);
		}
	}
	else {
		for(ii=0;ii<nssp;ii++) {
			fwrite(start1+ii,sizeoflong,1,stdout);
			fwrite(stop1+ii,sizeoflong,1,stdout);
			totsamps+= (stop1[ii]-start1[ii]);
		}
	}


	/************************************************************/
	/* CLEANUP AND EXIT */
	/************************************************************/
END:
	if(setverb==1) {
		fprintf(stderr,"total_ssp_pairs= %ld\n",nssp);
		fprintf(stderr,"total_duration= %ld samples\n",totsamps);
	}
 	if(index!=NULL) free(index);
 	if(start1!=NULL) free(start1);
 	if(stop1!=NULL) free(stop1);
	if(start2!=NULL) free(start2);
 	if(stop2!=NULL) free(stop2);
 	exit(0);

}
