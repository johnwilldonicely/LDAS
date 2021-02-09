#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-readscore1"
#define TITLE_STRING thisprog" v 9: 22.February.2016 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000
#define CHANNELMAX 256

/*
<TAGS>file</TAGS>

v 9: 22.February.2016 [JRH]
	- BUGFIX selection of epochs based on a file list

v 9: 02.February.2016 [JRH]
	- add ability to further select epochs based on a file list

v 9: 30.March.2015 [JRH]
	- add check for presence of start-time in file, not simply that time is greater than set-start!
	- previous problem: if all records were > set_start, output begins at first record

v 8: 14.November.2014 [JRH]
	- bugfix in calculating date from header

v 7: 11.November.2014 [JRH]
	- add estimate of clipping to header-only output
	- add ability to specify start record-number using yy:mm:dd:hh:mm:ss format

v 6: 27.October.2014 [JRH]
	- switch to per-record reading of SCORE RAW files
	- use new, simpler xf_readscore_raw1 function (no file handling, or header processing)
	- drop BINX output
	- use new xf_writebin1_v function for binary output

//??? TO DO - VARIABLE RECORD SIZE AND HEADER SIZE?


v 5: 1.July.2013 [JRH]
	- switch to using size_t file data counters, introduce variables ii,jj,kk,mm,nn
	- change -head option to -h, to avoid confusion with Linux head command
	- fix some error messages referring to legacy "-bin" option

 v 4: 28.June.2013 [JRH]
 	- output tweaks
	- minror fixes to xf_writebinx function
	- bugfix to ASCII output - now correctly adjusts pointer to data according to the -start argument

v 3: 12.June.2013 [JRH]
	- program now passes file-pointer to binary read-write functions - allows potential piecemeal writing of files
*/


/* external functions start */
int xf_readscore_raw1(FILE *fpin, char *header, size_t setnhead, char *data, size_t ndata, char *message);
int xf_writebin1_v(FILE *fpout, void *data0, size_t nn, size_t datasize, char *message);
char* xf_strsub1 (char *source, char *str1, char *str2);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[MAXLINELEN],outfile[MAXLINELEN],message[MAXLINELEN],line[MAXLINELEN];
	int v,w,x,y,z;
	float a,b,c,d,result_f[32];
	double aa,bb,cc,dd,result_d[32];
	size_t ii,jj,kk,mm,nn;
	FILE *fpin,*fpout;

	/* program-specific variables */
	int h_month,h_day,h_year,h_hour,h_minute,h_second,h_channel,h_length,h_rate,h_artefact,h_score,h_emg; // header contents
	int startmode=0;
	short *list=NULL;
	long start;
	char *header=NULL;
	unsigned char *data0=NULL;
	size_t ndata,nlist,sizeofshort=sizeof(short);
	float clip;

	/* arguments */
	char *startchar=NULL,*setlistfile=NULL;
	int setout=1;
	unsigned long setstart=0,setnrecords=0;
	size_t setnhead=35;
	double setsampfreq=400.0,setrecdur=10.0;

	if((startchar=(char *)realloc(startchar,32*sizeof(char)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	sprintf(startchar,"0");
	sprintf(outfile,"stdout");

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Read a SCORE raw file and output as ASCII or binary stream\n");
		fprintf(stderr,"Input assumed to be unsigned 8bit ints (1 byte characters)\n");
		fprintf(stderr,"Non-numeric values will be recoded \"NaN\"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: SCORE raw-file name or \"stdin\"\n");
		fprintf(stderr,"		NOTE: data assumed to be \"unsigned char\" (8-bit int)\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-sf: sampling frequency (Hz) [%g]\n",setsampfreq);
		fprintf(stderr,"	-dur: duration (s) of each record [%g]\n",setrecdur);
		fprintf(stderr,"	-head: size of record headers (bytes) [%ld]\n",setnhead);
		fprintf(stderr,"	-start: record or time to start output at [%ld]\n",setstart);
		fprintf(stderr,"		two modes: record-number (integer >= zero) or time\n");
		fprintf(stderr,"		time must be in the format YY:MM:DD:hh:mm:ss\n");
		fprintf(stderr,"		NOTE: corrects for SCORE's use of MM:DD:YY format\n");
		fprintf(stderr,"		NOTE: if exact start-time is not found, there will be no output\n");
		fprintf(stderr,"	-n: max records to output (0 = all) [%ld]\n",setnrecords);
		fprintf(stderr,"	-lf: file listing record-numbers (0-offset) to output [unset]\n");
		fprintf(stderr,"	-out: output format (0=headers only, 1=ASCII, 2=binary) [%d]\n",setout);
		fprintf(stderr,"		0=headers only\n");
		fprintf(stderr,"		1=ASCII\n");
		fprintf(stderr,"		2=binary (unsigned 8-bit integer = unsigned char)\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data30686.hpc4 -start 6 -n 1 -out 2 > output.bin\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	- all output is sent to stdout\n");
		fprintf(stderr,"	- if -out 0, header for each record + %%clipping\n");
		fprintf(stderr,"	- if -out 1, simple ASCII (1-column) data stream\n");
		fprintf(stderr,"	- if -out 2, binary (unsigned char, = 8bit int) data stream\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-sf")==0)    setsampfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-dur")==0)   setrecdur=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-head")==0)  setnhead=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-start")==0) snprintf(startchar,18,"%s",argv[++ii]);
			else if(strcmp(argv[ii],"-lf")==0)    setlistfile=argv[++ii];
			else if(strcmp(argv[ii],"-n")==0)     setnrecords=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)   setout=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setsampfreq<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid sample frequency (-sf %g) : must be > 0\n\n",thisprog,setsampfreq);exit(1);}
	if(setrecdur<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid record duration (-dur %g) : must be > 0\n\n",thisprog,setrecdur);exit(1);}
	if(setnhead<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid record duration (-head %ld) : must be > 0\n\n",thisprog,setnhead);exit(1);}
	if(setout<0 || setout>2) {fprintf(stderr,"\n--- Error[%s]: invalid -dtout (%d) : must be 0-2\n\n",thisprog,setout);exit(1);}
	if(setnrecords<0) {fprintf(stderr,"\n--- Error[%s]: invalid -n (%ld) : must be -1 or >= 0\n\n",thisprog,setnrecords);exit(1);}

	/* read start string - decide if it is a SCORE time or a record-number - convert to long integer */
	if(strstr(startchar,":")!=NULL) {
		startmode=1;
		startchar= xf_strsub1(startchar,":","");
	}
	setstart=atol(startchar);
	if(setstart<0) {fprintf(stderr,"\n--- Error[%s]: invalid -start (%ld) : must be >= 0\n\n",thisprog,setstart);exit(1);}

	ndata=(size_t)(0.5 + (setsampfreq*setrecdur*(double)sizeof(char)));
	if((header=(char *)realloc(header,(setnhead)*sizeof(char)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((data0=(unsigned char *)realloc(data0,(ndata)*sizeof(unsigned char)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

  	/********************************************************************************
	READ THE RECORD LIST
	********************************************************************************/
	nlist=0;
	if(setlistfile!=NULL) {
		nlist=0;
		if((fpin=fopen(setlistfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
		/* find the maximul record-number in the list */
		nlist=0;
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%ld",&ii)==1) {
				if(ii>nlist) nlist=ii;
			}
		}
		nlist++;
		/* allocate memory and initialize list */
		if((list=(short *)calloc(nlist,sizeof(short)))==NULL) {fprintf(stderr,"--- Error[%s]: insufficient memory\n",thisprog);exit(1);}
		/* go back and assign 0 or 1 to record numbers from the list */
		rewind(fpin);
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%ld",&ii)==1) list[ii]=1;
		}
		fclose(fpin);
	}
//fprintf(stderr,"nlist=%ld\n",nlist);
//for(ii=0;ii<nlist;ii++) printf("%ld	%d\n",ii,list[ii]);
//exit(0);

 	/********************************************************************************
	READ THE DATA
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"rb"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	/* HEADER OUTPUT ONLY */
	if(setout==0) {
		nn=-1; mm=z=0;
		printf("date    \ttime    \tname\tchan\tlength\trate\tart\tscore\tEMG\t%%clip\tepoch\n");
		while(!feof(fpin)) {
			nn++;
			// read a block
			x= xf_readscore_raw1(fpin,header,setnhead,data0,ndata,message);
			if(x<0)	{fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1);}

			// determine the start time (or sample) of the current block
			if(startmode==0) start=nn;
			else if(startmode==1) {
				sprintf(startchar,"%02d%02d%02d%02d%02d%02d",atoi(header+15),atoi(header+9),atoi(header+12),atoi(header+18),atoi(header+21),atoi(header+24));
				start=atol(startchar);
			}
			if(start==setstart) setstart=0;

			if(setstart==0) {
			 	if(setnrecords==0||mm++<setnrecords) {

					/* check record number against list */
					if(nlist>0) { if(nn>=nlist) continue; if(list[nn]==0) continue; }

					h_month=atoi(header+9);
					h_day=atoi(header+12);
					h_year=atoi(header+15);
					h_hour=atoi(header+18);
					h_minute=atoi(header+21);
					h_second=atoi(header+24);
					h_channel=(int)header[27];
					h_length=(int)header[28];
					h_rate=(int)header[29];
					h_artefact=(int)header[30];
					h_score=(int)header[31];
					h_emg=(int)header[32];

					/* calculate the percentage clipping in this record */
					jj=0; for(ii=0;ii<ndata;ii++) if(data0[ii]==0||data0[ii]==255) jj++; clip=((float)jj*100.0)/(float)ndata;

					printf("%02d:%02d:%02d	%02d:%02d:%02d	%s	%d	%d	%d	%c	%d	%d	%.2f	%ld\n",
					h_year,h_month,h_day,h_hour,h_minute,h_second,header,h_channel,h_length,h_rate,h_artefact,h_score,h_emg,clip,nn);
	}}}}

	/* ASCII OUTPUT */
	else if(setout==1 && startmode==0) { // start at epoch number
		nn=-1; mm=0;
		while(!feof(fpin)) {
			nn++;
			x= xf_readscore_raw1(fpin,header,setnhead,data0,ndata,message); // read a record header and data
			if(x<0)	{fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1);}
			if(nn>=setstart) {
			 	if(setnrecords==0||mm++<setnrecords) {
					/* check record number against list */
					if(nlist>0) {if(nn>=nlist) continue; else if(list[nn]==0) continue; }
					for(ii=0;ii<ndata;ii++) printf("%u\n",data0[ii]);
	}}}}
	else if(setout==1 && startmode==1) { // start at yy:mm:dd:hh:mm:ss
		nn=-1; mm=0;
		while(!feof(fpin)) {
			nn++;
			x= xf_readscore_raw1(fpin,header,setnhead,data0,ndata,message); // read a record header and data
			if(x<0)	{fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1);}
			/* determine the timestamp for this record  */
			sprintf(startchar,"%02d%02d%02d%02d%02d%02d",atoi(header+15),atoi(header+9),atoi(header+12),atoi(header+18),atoi(header+21),atoi(header+24));
			start=atol(startchar);
			if(start==setstart) setstart=0;
			if(setstart==0) {
			 	if(setnrecords==0||mm++<setnrecords) {
					/* check record number against list */
					if(nlist>0) {if(nn>=nlist) continue; else if(list[nn]==0) continue; }
					for(ii=0;ii<ndata;ii++) printf("%u\n",data0[ii]);
	}}}}

	/* BINARY OUTPUT */
	else if(setout==2 && startmode==0) { // start at epoch number
		nn=-1; mm=0;
		while(!feof(fpin)) {
			nn++;
			x= xf_readscore_raw1(fpin,header,setnhead,data0,ndata,message); // read a record header and data
			if(x<0)	{fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1);}
			if(nn>=setstart) {
			 	if(setnrecords==0||mm++<setnrecords) {
					/* check record number against list */
					if(nlist>0) {if(nn>=nlist) continue; else if(list[nn]==0) continue; }
					x= xf_writebin1_v(stdout,data0,ndata,sizeof(unsigned char),message);
					if(x<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}}}}
	else if(setout==2 && startmode==1) { // start at yy:mm:dd:hh:mm:ss
		nn=-1; mm=0;
		while(!feof(fpin)) {
			nn++;
			x= xf_readscore_raw1(fpin,header,setnhead,data0,ndata,message); // read a record header and data
			if(x<0)	{fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1);}
			/* determine the timestamp for this record  */
			sprintf(startchar,"%02d%02d%02d%02d%02d%02d",atoi(header+15),atoi(header+9),atoi(header+12),atoi(header+18),atoi(header+21),atoi(header+24));
			start=atol(startchar);
			if(start==setstart) setstart=0;
			if(setstart==0) {
			 	if(setnrecords==0||mm++<setnrecords) {
					/* check record number against list */
					if(nlist>0) {if(nn>=nlist) continue; else if(list[nn]==0) continue; }
					x= xf_writebin1_v(stdout,data0,ndata,sizeof(unsigned char),message);
					if(x<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}}}}

	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	free(header);
	free(data0);
	free(startchar);

	}
