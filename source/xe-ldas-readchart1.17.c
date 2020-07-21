#define thisprog "xe-ldas-readchart1"
#define TITLE_STRING thisprog" v 17: 12.March.2015 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* <TAGS> file O2 </TAGS> */

/*
v 17: 12.March.2015 [JRH]
	- bugfix: now correctly deals with path-names which include a "."

v 16: 11.July.2013 [JRH]
	- remove option of using "stdin" as input, as rewind() function may not be compatible on all platforms
	- minor bugfix - should not report long time-gap at first time-stamp!

v 15:  13.May.2013 [JRH]
	- report warning if sample-interval is too long (gaps in time record)

v 14: 6.October.2012 [JRH]
	- updated call to xf_percentile1_d
		- "message" string is no longer used, as printing messages with the function name complicated determination of program dependencies

v1.13  27.June.2012 [JRH]
	- refinement on comment-detection from v1.12
	- now program detects if CHART output is old (header contains keyword TopValue) or new (it doesn't)
	- for old files, the channel-delimiters for comments are #, \t and \n
	- for new files, the channel-delimiters for comments are \t and \n, while "#" delineates multiple comments within a channel

v1.12  26.June.2012 [JRH]
	- allow for multiple comments for a given timestamp on a given channel
	- a tab is the only allowable channel-separator for comments n the .txt input file
	- multiple "#" symbols in a given field are taken to indicate multiple comments for the same channel
	- all "#" symbols are removed in the output .cmt file
	- multiple comments from the same line are output on separate lines, but with the same time-stamp

v1.11  8.February.2012 [JRH]
	- minor correction to creation of sample_interval array (line 255) - now correctly fills array starting from element zero (i-1) instead of 1 (i)

v1.10  10.November.2011 [JRH]
	- basename now also strips leading PATHNAME (everything up to last "/") off the input filename
		- this will allow local output even if input file is remote
	- remove requirement for Range= tag in header to calculate number of channels
		- this is now read from the number of labels instead
	- incorporate Windows-style carriage-return character into delimiters for reading header labels

v1.9  5.September.2011 [JRH]
	- bugfix - replaced xf_strsub with xf_strsub1
		- potential (never-encountered) error if replacing a string with a longer string

v1.8  12.July.2011 [JRH]
	- simple name-change from lilly-readchart1 to ldas-readchart1

v1.7  4.July.2011 [JRH]
	- correct time output if record is seen to stop and start (if timestamps do not incriment)
		- ensures that next timestamp is at least 2x the normal sample interval more than previous timestamp
	- stores comments efficiently in memory
		- allows output of comments with corrected timestamps
	- ignores sample_interval information in CHART file header - this is unreliable - uses median interval instead

v1.6  19.May.2011 [JRH]
	- stripped down version - remove filtering options - program now simply creates new raw data files

v1.4  6.May.2011 [JRH]
	- remove from channel labels the "#[ch]" CHART inserts, and replace other spaces with underscores

v 1.3: JRH, 26.April.2011
 	- saves probe data (one column only) to new named file
 	- saves comments to a matching file for same probe (time + comment)
	- optional save time series (only needs to be done once - same for all probes)

v 1.2: JRH, 8.April.2011
 	- added storage for start & end times to define blocks of data to be later averaged
	- added specification of a channel name to identify column to use
		- program looks for this word in the header
		- this could be a number, or an array of numbers
 	- reverted back to accepting only a single input channel
*/

/* external functions start */
char* xf_strsub1 (char *source, char *str1, char *str2);
int xf_percentile1_d(double *data, long n, double *result);
int xf_compare1_d(const void *a, const void *b);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],delimiters[256],templine[MAXLINELEN],message[256],*matchstring=NULL,*pline,*pcol,*pkey,*pval;
	long int i,j,k,l,n;
	int v,w,x,y,z,col,colmatch,datcoltot,result_i[32];
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d,result_f[32];
	double aa,bb,cc,dd,result_d[32];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char *words=NULL,tempword[MAXLINELEN];
	int *iword=NULL,lenwords=0,nwords=0,foundword=0;
	long *posword=NULL;

	char basefile[256],tempfile[256],datafile[256],commentfile[256],timefile[256];
	int oldchart=0; // is this an old version of CHART? (detemined by presence of TopValue in header)
	int headerlines=0; // keeps track of the number of lines in the CHART output file header
	int nchans=0; // total number of channels (this will be obtained from the header)
	int colchan=-1; // column containing channel to be read
	double sample_interval=-1.0;
	float sample_freq=-1.0;
	float *data1=NULL;
	double *time=NULL,*interval=NULL,tcur,tprev,tadj;
	FILE *fpoutdata, *fpouttime, *fpoutcomment;
	/* arguments */
	char setoutstring[266],setchanname[256];
	int filttype=1; // this specifies a lowpass filter, provided setfilthigh is also set
	int setinvert=0,setouttime=1,setdatacol=2;
	float setfilthigh=-1.0;

	sprintf(setchanname,"");

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Read biosensing data output from Chart program\n");
		fprintf(stderr,"Assumptions:\n");
		fprintf(stderr,"	- input has header specifying date, channel names, labels etc.\n");
		fprintf(stderr,"	- data format: time ch1 ch2 <etc>... comment1 comment2 <etc>\n");
		fprintf(stderr,"	- tab separates columns for time, data and comments\n");
		fprintf(stderr,"	- for old CHART files: \n");
		fprintf(stderr,"		\"#\" separates the comment for each channel\n");
		fprintf(stderr,"		simultaneous comments for a given channel are disallowed\n");
		fprintf(stderr,"	- for newer CHART files: \n");
		fprintf(stderr,"		a tab separates comments for each channel\n");
		fprintf(stderr,"		\"#\"  preceeds each comment for a given channel\n");
		fprintf(stderr,"Calculates sample frequency based on median sample-interval\n");
		fprintf(stderr,"Will handle comments and channel names with spaces\n");
		fprintf(stderr,"Will correct for time-stamp resets mid-trial by adjusting timestamps\n");
		fprintf(stderr,"	after the reset, adding the previous timestamp + 2x the sample interval\n");
		fprintf(stderr,"	(ie. times will be made to run in order, with a small gap as a reminder\n");
		fprintf(stderr,"Automatically detemines number of channels from file header\n");
		fprintf(stderr,"Assigns channel-number to output  based on the order they appear in the \n");
		fprintf(stderr,"	input (001, 002, 003 etc.)\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input][options]\n",thisprog);
		fprintf(stderr,"	[input]: file name (CHART txt export file with header)\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-chancol: column containing channel to be extratced [%d]\n",setdatacol);
		fprintf(stderr,"		NOTE: time is in col.1, and columns may not map onto channels\n");
		fprintf(stderr,"	-channame: name of channel to extract [%s]\n",setchanname);
		fprintf(stderr,"		NOTE: overrides -chancol. Unset by default\n");
		fprintf(stderr,"	-time: output time records? (0=no, 1=yes) [%d]\n",setouttime);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -t 1\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	[base].[channel].dat : all samples from one channel\n");
		fprintf(stderr,"	[base].[channel].cmt : comments (time comment) from one channel\n");
		fprintf(stderr,"	[base].time : timestamps for all samples\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE INPUT FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-channame")==0){ sprintf(setchanname,"%s",argv[i+1]); i++;}
			else if(strcmp(argv[i],"-chancol")==0) { setdatacol=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-time")==0) { setouttime=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}
	if(setouttime!=0&&setouttime!=1) {fprintf(stderr,"\n--- Error[%s]: -time (%d) must be 0 or 1\n\n",thisprog,setouttime);exit(1);};
	if(setdatacol==1) {fprintf(stderr,"\n--- Error[%s]: -chancol cannot be set to 1: this is the time column\n\n",thisprog);exit(1);}
	if(setdatacol>0 && strlen(setchanname)>0) setdatacol=-1;

	// GENERATE BASE-NAME - STRIP PATH NAME AND ".TXT" OR OTHER EXTENTIONS OFF INFILE NAME
	for(i=strlen(infile);i>=0;i--) if(infile[i]=='/') break;
	// copy each character to basefile, up to and including the terminating NULL, unless a "." is encountered first
	for(j=(i+1),k=0;j<=strlen(infile);j++) {
		if(infile[j]=='.') { basefile[k]='\0'; break; }
		else basefile[k++]=infile[j];
	}

	/******************************************************************************/
	/* READ THE HEADER - DETERMINE CHART VERSION (OLD or NEW), GET COLUMN NAMES ETC. */
	/******************************************************************************/
	if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	/* first read the header: keep track of the last line number read starting with a non-number */
	headerlines=0; /* keep track of total number of lines in the header */
	oldchart=0; /* variable to determine if this is an old version of chart or new */
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		pline=line;
		// read first word on line - if it's a number, break
		pkey=strtok(pline," ,\t\n\r"); pline=NULL;
		if(sscanf(pkey,"%lf",&aa)==1) break;
		else headerlines++;

		/* if header contains keyword "TopValue", this is output from an old version of CHART */
		if(strcmp(pkey,"TopValue=")==0) oldchart=1;

		/* Find the specified input column number of channel name */
		if(strcmp(pkey,"ChannelTitle=")==0) {
			if(setdatacol>0) {
				// start reading next columns, tab delimited, looking for the specified column
				for(col=2;(pcol=strtok(pline,"\t\n\r"))!=NULL;col++) {
					pline=NULL;
					nchans++;
					if(col==setdatacol) {colchan=col;sprintf(setchanname,"%s",pcol);}
				}
				if(colchan<0) {fprintf(stderr,"\n--- Error[%s]: specified input column %d not found\n\n",thisprog,setdatacol);exit(1);}
			}
			else { /* this condition only met if -channame is set */
				/* look for input column name */
				for(col=2;(pcol=strtok(pline,"\t\n\r"))!=NULL;col++) {
					pline=NULL;
					nchans++;
					if(strcmp(pcol,setchanname)==0) colchan=col;
				}
				if(colchan<0) {fprintf(stderr,"\n--- Error[%s]: specified input channel name \"%s\" not found\n\n",thisprog,setchanname);exit(1);}
			}
		}
	}

	// GENERATE OUTPUT FILE-NAMES - DATA & COMMENT FILE NAMES INCLUDE CHANNEL-NUMBER
	sprintf(datafile,"%s.%03d.dat",basefile,(colchan-1));
	sprintf(timefile,"%s.time",basefile);
	sprintf(commentfile,"%s.%03d.cmt",basefile,(colchan-1));

	// SET HOW MANY COLUMNS TO LOOK FOR TIME AND DATA IN
	datcoltot=nchans+1;

	/******************************************************************************/
	/* NOW GO BACK AND READ THE DATA*/
	/******************************************************************************/
	rewind(fpin); for(i=0;i<headerlines;i++) fgets(line,MAXLINELEN,fpin);
	/* now read the real data */
	if(oldchart==0) sprintf(delimiters,"\t\n");
	if(oldchart==1) sprintf(delimiters,"#\t\n");
	n=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		pline=line;
		foundword=0; //marker to indicate if a comment was found
		colmatch=2; // number of columns to match - time plus specified channel
		sprintf(tempword,""); // initialise placeholder for comments
		for(col=1;(pcol=strtok(pline,delimiters))!=NULL;col++) { // only tabs and newlines are field separators - allows parsing of comments for each channel
			pline=NULL;
			if(col==1 && sscanf(pcol,"%lf",&aa)==1) colmatch--; // if this is the time column store time temporarily in "aa"
			if(col==colchan && sscanf(pcol,"%f",&b)==1) colmatch--; // if this is the data column, store the temporary number in "b"
			if(col==(colchan+nchans)) { // if column is appropriate for optional comments for the selected channel, save the comment
				foundword=1;break;
			}
		}
		// if fewer than time+nch columns were found containing numbers, continue
		if(colmatch>0) continue;

		// dynamically allocate memory for the time and data
		if((time=(double *)realloc(time,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((data1=(float *)realloc(data1,(n+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		// assign values to time and data for this record (timepoint)
		time[n]=aa; data1[n]=b;

		// if a comment (word) was found, store it in a list (words)
		if(foundword==1) {
			/* replace any spaces with underscores */
			pcol=xf_strsub1(pcol," ","_");
			/* allocate memory for expanded words and word-index */
			x=strlen(pcol); // not including terminating NULL
			words=(char *)realloc(words,((lenwords+x+4)*sizeofchar)); if(words==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			iword=(int *)realloc(iword,(nwords+1)*sizeofint); if(words==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			posword=(long *)realloc(posword,(nwords+1)*sizeoflong); if(words==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			/* set pointer to start position (currently, the end of the labels string) */
			iword[nwords]=lenwords;
			/* link word to position in overall data series */
			posword[nwords]=n;
			/* add new word to end of words, adding terminal NULL */
			sprintf(words+lenwords,"%s",pcol);
			/* update length, allowing for terminal NULL - serves as pointer to start of next word */
			lenwords+=(x+1);
			/* incriment nwords with check */
			nwords++;
		}
		 n++;
	}
	fclose(fpin);

	/******************************************************************************/
	/* CALCULATE THE MEDIAN INTERVAL - THE CHART HEADER FILE IS NOT ALWAYS ACCURATE
	/* AS DATA IS SOMETIMES DOWNSAMPLED DURING EXPORT TO .TXT */
	/******************************************************************************/
	/* make a copy of the time array which is actually interval */
	if((interval=(double *)realloc(interval,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(i=1;i<n;i++) interval[(i-1)]=time[i]-time[(i-1)];
	/* sort the intervals */
	z=xf_percentile1_d(interval,(n-1),result_d);
	if(z!=0) {fprintf(stderr,"\t\aError[%s]: insufficient memory for calculation of percentiles%s\n",thisprog);exit(1);}
	sample_interval=result_d[5];
	sample_freq=(float)(1.0/sample_interval);

	/******************************************************************************/
	/* CHECK THE TIMESTAMPS - ARE THERE STARTS & STOPS? */
	/******************************************************************************/
	tprev = time[0]-sample_interval; /* fake time for previous sample at start = start minus sample_interval */
	tadj = 0.0; /* adjustment to current time is zero at start, before any gaps are detected */

	for(i=0;i<n;i++) {
		tcur=time[i]+tadj;
		aa = tcur-tprev;
		if(aa<=0) {
			tadj+=(tprev-tcur)+(2.0*sample_interval); // guarantees an unusually long gap at adjustment point
			tcur=time[i]+tadj;
			fprintf(stderr,"\n--- Warning[%s]: bad time sequence at sample %d, time %g. New time=%g\n\n",thisprog,i,time[i],tcur);
		}
		if(aa>(1.5 * sample_interval)) fprintf(stderr,"\n--- Warning[%s]: Sample interval at sample %d, time %g is unusually large (%g seconds)\n\n",thisprog,i,time[i],aa);
		time[i]=tcur;
		tprev=tcur;
	}

	/******************************************************************************/
	/* OUTPUT TIME */
	/******************************************************************************/
	if(setouttime==1) {
		if((fpouttime=fopen(timefile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: cannot write to file \"%s\"\n\n",thisprog,timefile);exit(1);}
		for(i=0;i<n;i++) fprintf(fpouttime,"%.3f\n",time[i]);
		fclose(fpouttime);
	}

	/******************************************************************************/
	/* OUTPUT THE DATA */
	/******************************************************************************/
	if((fpoutdata=fopen(datafile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: cannot write to file \"%s\"\n\n",thisprog,datafile);exit(1);}
	for(i=0;i<n;i++) fprintf(fpoutdata,"%.3f\n",data1[i]);
	fclose(fpoutdata);

	/******************************************************************************/
	/* OUTPUT THE COMMENTS */
	/******************************************************************************/
	if((fpoutcomment=fopen(commentfile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: cannot write to file \"%s\"\n\n",thisprog,datafile);exit(1);}
	for(i=0;i<nwords;i++) {
		sprintf(line,words+iword[i]);
		pline=line;
		for(col=1;(pcol=strtok(pline,"#\t\n"))!=NULL;col++) { // only hash and newlines are field separators - allows parsing of multiple comments for a given timestamp
			pline=NULL;
			fprintf(fpoutcomment,"%.3f\t%s\n",time[posword[i]],pcol);
		}
	}
	fclose(fpoutcomment);

	/******************************************************************************/
	/* PRINT REPORT
	/******************************************************************************/
	aa=time[0]; bb=time[(n-1)]; cc=bb-aa;
	printf("\n");
	printf("Total_channels: %d\n",nchans);
	printf("Input_channel: %s (column %d)\n",setchanname,colchan);
	printf("Total_samples: %d\n",n);
	printf("Sample_rate: %g Hz\n",sample_freq);
	printf("Start_time: %.3f\n",aa);
	printf("End_time: %.3f\n",bb);
	printf("Duration: %.3f seconds = %.3f minutes\n",cc,(cc/60.0));
	printf("Output_files:\n");
	printf("\t%s\n",datafile);
	printf("\t%s\n",commentfile);
	if(setouttime==1) printf("\t%s\n",timefile);
	printf("\n");

	free(time); free(interval); free(data1);
	free(words); free(iword); free(posword);

	exit(0);
	}
