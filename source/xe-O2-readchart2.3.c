#define thisprog "xe-O2-readchart2"
#define TITLE_STRING thisprog" v 3: 19.October.2015 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
<TAGS> file O2 </TAGS>

v 3: 19.October.2015 [JRH]
	- allow detection of manual comments beginning with #* - these apply to all channles

v.2: 20.April.2015 [JRH]
	- drop determination of output filename - use standard temp file output instead
	- this allows calling script to determine output names
	- calling script must also determine integority of file name and header comments

v.1: 17.April.2015 [JRH]
	- needed to account for CHART files with data from multiple animals
	- needed to derive subject number from the file header
	- needed to assume comments apply to the channel-number following the "#" comment field separator
	- derived from xe-ldas-readchart1.17.c
		- specifically disallow input from stdin
		- specify -ch (channel number 1-16) instead of -chancol, the column in file containing the channel of interest
		- specify -chname in stead of -channame
		- allow specification of the date from a file-name field (hyphen-separated)
		- subject ID is now defined by the channel label
		- comment format: [time]<tab>[ch]_[MEDbox]_[comments]


*/

/* external functions start */
long xf_interp3_f(float *data, long ndata);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long *xf_lineparse1(char *line,long *nwords);
char *xf_strcat1(char *string1,char *string2,char *delimiter);
int xf_percentile1_d(double *data, long n, double *result);
int xf_compare1_d(const void *a, const void *b);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],line[MAXLINELEN],templine[MAXLINELEN],*pline,*pcol;
	long i,j,k,l,n;
	int v,w,x,y,z,col,colmatch;
	int sizeoflong=sizeof(long),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[32];
	FILE *fpin;

	/* program-specific variables */
	char *comments=NULL,comment[MAXLINELEN];
	char basefile[64],datafile[64],commentfile[64],timefile[64];
	int headerlines,nchans=0,colchan=-1,foundcomments;
	long *start1=NULL,*start2=NULL,*poscom=NULL,nwords1,nwords2,ncom,nbad;
	float *data1=NULL;
	double *time=NULL,*interval=NULL,tcur,tprev,tadj,sample_freq;
	double sample_interval=-1.0;
	FILE *fpoutdata, *fpouttime, *fpoutcomment;

	/* arguments */
	char setchanname[256];
	int filttype=1; // this specifies a lowpass filter, provided setfilthigh is also set
	int setdatefield=3;
	int setouttime=1,setchannum=1,settx=2,setinterp=0,setmcmt=1;

	sprintf(setchanname,"");

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"Read biosensing data output from Chart program\n");
		fprintf(stderr,"Calculates sample frequency based on median sample-interval\n");
		fprintf(stderr,"Will handle comments and channel names with spaces\n");
		fprintf(stderr,"Corrects time-stamp irregularities from stopping and re-starting recording\n");
		fprintf(stderr,"	- timestamps may temporarily jump backwards (jumpback)\n");
		fprintf(stderr,"	- timestamp interval may exceed median interval (delays)\n");
		fprintf(stderr,"Automatically detemines number of channels from file header\n");
		fprintf(stderr,"Channel-numbers assigned according to column-order (001,002,003 etc.)\n");
		fprintf(stderr,"Assumptions:\n");
		fprintf(stderr,"	- input header specifies ChannelTitle for each column\n");
		fprintf(stderr,"	- input format: time<tab>ch1<tab>ch2...<tab>allcomments\n");
		fprintf(stderr,"	- multiple comments separated by \" #\" may be in the last field\n");
		fprintf(stderr,"		- example multi-comment for channels 1,5 and 9 (single timestamp): \n");
		fprintf(stderr,"		#1 1_TASK ON #5 2_TASK ON #9 3_TASK ON \n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input][options]\n",thisprog);
		fprintf(stderr,"	[input]: file name (CHART txt export file with header)\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-ch: channel number (1-16) to be extratced [%d]\n",setchannum);
		fprintf(stderr,"	-chname: name of channel to extract, overrides -ch [%s]\n",setchanname);
		fprintf(stderr,"	-time: output time records? (0=no, 1=yes) [%d]\n",setouttime);
		fprintf(stderr,"	-tx: correct timestamps? (0=no, 1=jumpbacks, 2=jumpbacks + delays) [%d]\n",settx);
		fprintf(stderr,"	-int: interpolate non-finite values? (0=no, 1=yes) [%d]\n",setinterp);
		fprintf(stderr,"	-mcmt: detect manual comments applied to all channels (0=no, 1=yes) [%d]\n",setmcmt);
		fprintf(stderr,"		- these comments will begin with #*\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s 001-004-991231.txt -ch 5\n",thisprog);
		fprintf(stderr,"\n");
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	temp_"thisprog".time : timestamps \n");
		fprintf(stderr,"	temp_"thisprog".dat : data samples for specified channel\n");
		fprintf(stderr,"	temp_"thisprog".cmt : times & comments for specified channel\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE INPUT FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-ch")==0)      setchannum=atoi(argv[++i]);
			else if(strcmp(argv[i],"-chname")==0)  { sprintf(setchanname,"%s",argv[++i]); setchannum=-1; }
			else if(strcmp(argv[i],"-time")==0)    setouttime=atoi(argv[++i]);
			else if(strcmp(argv[i],"-tx")==0)      settx=atoi(argv[++i]);
			else if(strcmp(argv[i],"-int")==0)     setinterp=atoi(argv[++i]);
			else if(strcmp(argv[i],"-mcmt")==0)     setmcmt=atoi(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}
	if(setouttime!=0&&setouttime!=1) {fprintf(stderr,"\n--- Error[%s]: -time (%d) must be 0 or 1\n\n",thisprog,setouttime);exit(1);};
	if(setchannum<1 && strlen(setchanname)<1) {fprintf(stderr,"\n--- Error[%s]: -chancol must be >0\n\n",thisprog);exit(1);}
	if(strcmp(infile,"stdin")==0) {fprintf(stderr,"\n--- Error[%s]: this program does not support input from stdin - provide a file name instead\n\n",thisprog);exit(1);}
	if(setdatefield<1) {fprintf(stderr,"\n--- Error[%s]: -df (%d) must be >0\n\n",thisprog,setdatefield);exit(1);}
	if(setinterp<0||setinterp>1) {fprintf(stderr,"\n--- Error[%s]: interpolation (-int %d) must be 0 or 1\n\n",thisprog,setinterp);exit(1);};
	if(setmcmt<0||setmcmt>1) {fprintf(stderr,"\n--- Error[%s]: -mcmt (%d) must be 0 or 1\n\n",thisprog,setmcmt);exit(1);};
	if(settx<0||settx>2) {fprintf(stderr,"\n--- Error[%s]: time-corrrection (-tx %d) must be 0,1 or 2\n\n",thisprog,settx);exit(1);};

	// user enters 1,2 etc, but for convenience this is reduced to zero-offset for line-parsing
	setdatefield--;


	/******************************************************************************
	READ THE HEADER
	- determine the number of header lines
	- get the name of the channel (or the channel corresponding to the name)
	- determine the subject ID from the channel name
	- determine the total number of channels
	******************************************************************************/
	if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	/* first read the header: keep track of the last line number read starting with a non-number */
	headerlines=0; /* keep track of total number of lines in the header */
	z=0; // here this is used to count the number of columns found matching -chname - should be maximum of 1

	while(fgets(line,MAXLINELEN,fpin)!=NULL) {

		//save the channel title line for later - to determine the within-subject channel-number
		if(strstr(line,"ChannelTitle=")!=NULL) strcpy(templine,line);
		pline=line;

		// read first column on line - if it doesn't contain an "=", then it marks the end of the header
		pcol=strtok(pline," \t\n\r"); pline=NULL;
		if(strstr(pcol,"=")==NULL) break;
		else headerlines++;

		/* find the specified input column number (zero-offset) of channel name */
		if(strcmp(pcol,"ChannelTitle=")==0) {
			nchans=0;
			colchan=-1;
			if(setchannum>0) {
				// start reading next columns, tab delimited, looking for the specified column
				for(col=1;(pcol=strtok(pline,"\t\n\r"))!=NULL;col++) {
					pline=NULL;
					nchans++;
					if(col==setchannum) {colchan=col;sprintf(setchanname,"%s",pcol);}
				}
				if(colchan<0) {fprintf(stderr,"\n--- Error[%s]: specified input column %d not found\n\n",thisprog,setchannum);exit(1);}
			}
			else { /* look for input column name - this condition only met if -channame is set */
				for(col=1;(pcol=strtok(pline,"\t\n\r"))!=NULL;col++) {
					pline=NULL;
					nchans++;
					if(strcmp(pcol,setchanname)==0) { colchan=col; z++; }
				}
				if(colchan<0) {fprintf(stderr,"\n--- Error[%s]: specified input channel name \"%s\" not found\n\n",thisprog,setchanname);exit(1);}
				if(z>1) {fprintf(stderr,"\n--- Error[%s]: specified input channel name \"%s\" is not unique - %d instances in header\n\n",thisprog,setchanname,z);exit(1);}
	}}}



	/******************************************************************************
	GENERATE OUTPUT FILE-NAMES
	******************************************************************************/
	sprintf(basefile,"temp_%s",thisprog);
	sprintf(datafile,"%s.dat",basefile);
	sprintf(timefile,"%s.time",basefile);
	sprintf(commentfile,"%s.cmt",basefile);



	/******************************************************************************
	NOW GO BACK AND READ THE DATA
	- store the timestamps and the data
	- detect lines with an extra field (comments) in addition to time and nchans
	- parse comments and store individual comments relating to the chosen channel
	******************************************************************************/
	rewind(fpin); for(i=0;i<headerlines;i++) fgets(line,MAXLINELEN,fpin);
	n=ncom=nbad=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		pline=line;
		foundcomments=0; //marker to indicate if a comment was found
		colmatch=2; // number of columns to match - time plus specified channel
		for(col=0;(pcol=strtok(pline,"\t\n"))!=NULL;col++) { // only tabs and newlines are field separators - allows parsing of comments for each channel
			pline=NULL;
			if(col==0 && sscanf(pcol,"%lf",&aa)==1) { // if this is the time column store time temporarily in "aa"
				colmatch--;
			}
			if(col==colchan && sscanf(pcol,"%f",&b)==1) { // if this is the data column, store the temporary number in "b"
				colmatch--;
				if(!isfinite(b)) { b=NAN; nbad++; }
			}
			if(col>nchans) { foundcomments=1;break; } // if this is beyond the column-number associtated with time and data, it must be a comment
		}
		// if fewer than time+nch columns were found containing numbers, continue
		if(colmatch!=0) continue;

		// dynamically allocate memory for the time and data
		if((time=(double *)realloc(time,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((data1=(float *)realloc(data1,(n+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		// assign values to time and data for this record (timepoint)
		time[n]=aa;
		data1[n]=b;

		// COMMENT HANDLING...
		if(foundcomments==1) {
			// parse the multiple comments (pointed to by the last pcol) at the "#" symbols separating individual comments
			// because the previous parsing of input lines was tab-based, we know there can be no tabs in the comments field
			strcpy(line,pcol);
			start1= xf_lineparse2(line,"#",&nwords1);

			// for each comment except the first... (which is just the nothing before the first # delimiter)
			for(i=1;i<nwords1;i++) {
				// store the current comment for later use
				strcpy(comment,(line+start1[i]));
				// parse a copy of the current comment using whitespace
				strcpy(templine,comment);
				start2= xf_lineparse1(templine,&nwords2);

				// check if the first field (CHART-generated channel-number) matches our specified channel
				// if setmcmt is set, match is assumed if the channel-number in the input file is "*"
				if(setmcmt==1 && templine[start2[0]]=='*') z=colchan;
				else z=atoi(templine+start2[0]);
				if(z==colchan) {
					/* replace spaces in the original unparsed current-comment with underscores and drop the tailing space if there is one */
					k=strlen(comment);
					for(j=0;j<k;j++) { if(comment[j]==' ') { if(j==(k-1)) comment[j]='\0' ; else comment[j]='_'; }}
					// add the comment to a list of comments for this channel, separated by tabs
					comments= xf_strcat1(comments,comment,"\t");
					if(comments==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
					// remember the record number at which the comment was found - for applying timestamps later
					poscom=(long *)realloc(poscom,(ncom+1)*sizeoflong);
					if(poscom==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
					poscom[ncom++]=n;
		}}}
		n++;
	}
	fclose(fpin);

	/* interpolate if some data is bad */
	if(setinterp>0 && nbad>0) xf_interp3_f(data1,n);



	/******************************************************************************
	CALCULATE THE MEDIAN INTERVAL
	- the chart header file is not always accurate
	- this is because data is often downsampled during export to .txt
	- this interval will be used to check and normalize the interval in the output
	******************************************************************************/
	/* make a copy of the time array which is actually interval */
	if((interval=(double *)realloc(interval,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(i=1;i<n;i++) interval[(i-1)]=time[i]-time[(i-1)];
	/* sort the intervals */
	z=xf_percentile1_d(interval,(n-1),result_d);
	if(z!=0) {fprintf(stderr,"\t\aError[%s]: insufficient memory for calculation of percentiles\n",thisprog);exit(1);}
	sample_interval=result_d[5];
	sample_freq=(1.0/sample_interval);




	/******************************************************************************
	CHECK SEQUENTIAL TIMESTAMPS AGAINST THE MEDIAN INTERVAL
	- are there starts & stops? (timestamp goes backwards)
	- are there long intervals? (>1.5 median intervals)
	- apply correction as specified
	******************************************************************************/
	tprev = time[0]-sample_interval; /* fake time for previous sample at start = start minus sample_interval */
	tadj = 0.0; /* adjustment to current time is zero at start, before any gaps are detected */
	for(i=0;i<n;i++) {
		tcur=time[i]+tadj;
		aa = tcur-tprev;
		if(aa<=0) {
			if(settx>1){
				tadj+=(tprev-tcur)+(1.0*sample_interval); // guarantees a normal gap at adjustment point
				tcur=time[i]+tadj;
			}
			fprintf(stderr,"\n--- Warning[%s]: bad time sequence at sample %ld, time %g. New time=%g\n\n",thisprog,i,time[i],tcur);
		}
		if(aa>(1.5 * sample_interval)) {
			if(settx>1){
				tadj+=(tprev-tcur)+(1.0*sample_interval); // guarantees a normal gap at adjustment point
				tcur=time[i]+tadj;
			}
			fprintf(stderr,"\n--- Warning[%s]: Sample interval at sample %ld, time %g is unusually large (%g seconds)\n\n",thisprog,i,time[i],aa);
		}
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
	/* OUTPUT THE COMMENTS WITH APPROPRIATE (NOW CORRECTED) TIMESTAMPS */
	/******************************************************************************/
	if((fpoutcomment=fopen(commentfile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: cannot write to file \"%s\"\n\n",thisprog,datafile);exit(1);}
	if(ncom>0) {
		start1= xf_lineparse2(comments,"\t",&nwords1);
		for(i=0;i<nwords1;i++) {
			fprintf(fpoutcomment,"%.3f\t%s\n",time[poscom[i]],(comments+start1[i]));
		}
	}
	else {
		fprintf(fpoutcomment,"\n");
	}
	fclose(fpoutcomment);

	/******************************************************************************/
	/* PRINT REPORT */
	/******************************************************************************/
	aa=time[0];
	bb=time[(n-1)];
	cc=bb-aa;
	if(nbad==0) dd= 100.0; else dd= 100.0*(1.0 - ((double)nbad/(double)n));

	printf("\n");
	printf("Input_channel: %d (%s)\n",colchan,setchanname);
	printf("Total_channels: %d\n",nchans);
	printf("Total_samples: %ld\n",n);
	printf("Total_bad: %ld\n",nbad);
	printf("Total_good: %ld\n",(n-nbad));
	printf("Percent_good: %.2f %%\n",dd);
	printf("Total_comments: %ld\n",ncom);
	printf("Sample_rate: %g Hz\n",sample_freq);
	printf("Start_time: %.3f\n",aa);
	printf("End_time: %.3f\n",bb);
	printf("Duration: %.3f seconds = %.3f minutes\n",cc,(cc/60.0));
	printf("Output_files:\n");
	printf("\t%s\n",datafile);
	printf("\t%s\n",commentfile);
	if(setouttime==1) printf("\t%s\n",timefile);
	printf("\n");

	free(time);
	free(interval);
	free(data1);
	free(comments);
	free(poscom);

	exit(0);
	}
