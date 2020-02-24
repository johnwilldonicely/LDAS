/*
<TAGS>file SLICE</TAGS>
DESCRIPTION:
	- Read WinLTP files (.PO or .APO) containing sweeps for slice electrophysiology


DEPENDENCIES:
	long *xf_lineparse1(char *line,long *nwords);

ARGUMENTS:
	char *setinfile  : input, name of the file to read
	char *setchan    : input, name of channel to read (typically AD0 or AD1)
	float **data1    : output, dynamically allocated array to hold data - calling function passes &data1 to this function
	double *result_d : output, array of info related to the file
	char *message    : output, a string array to hold the status message

RETURN VALUE:
	on success: nsamples, = the number of samples read for the channel
	on failure: -1
	- data1[] will be assigned memory and filled [nsamples] datapoints
	- result_d[] will hold information on the data
		result_d[0]= sample-interval (ms)
		result_d[1]= sample-rate (Hz)
		result_d[2]= duration of pre-stimulus baseline (ms)

SAMPLE CALL:

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* external functions start */
long *xf_lineparse1(char *line,long *nwords);
/* external functions end */


long xf_readwinltp1_f(char *setinfile, char *setchan, float **data1, double *result_d, char *message) {

	char *thisfunc="xf_readwinltp1_f\0";
	char tempchan[256],line[1000],*pline;
	long ii,jj,kk,nn=-1,mm=0,incol=-1,sizeofdata;
	long *start=NULL,nwords,maxlinelen=0;
	double sampint=-1,baseline=-1;
	float aa,*tempdata=NULL;
	FILE *fpin;


	sizeofdata=sizeof(*tempdata);

	/********************************************************************************/
	/* MAKE A VERSION OP THE CHANNEL-NAME WITH QUOTES (WINLTP FIELDS ARE ALWAYS QUOTED) */
	/********************************************************************************/
	snprintf (tempchan,250,"\"%s\"",setchan);

	/********************************************************************************/
	/* OPEN THE INPUT FILE  */
	/********************************************************************************/
	if(strcmp(setinfile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(setinfile,"r"))==NULL) {	sprintf(message,"%s [ERROR]: could not open file %s",thisfunc,setinfile); return(-1); }

	/********************************************************************************/
	/* READ THE FILE HEADER */
	/********************************************************************************/
	while(fgets(line,1000,fpin)!=NULL) {
		start= xf_lineparse1(line,&nwords);
		if(nwords<1) continue;
		if(strstr("\"NumSamples\"",(line+start[0]))!=NULL) nn= atol((line+start[1]));
		if(strstr("\"SampleInterval_ms\"",(line+start[0]))!=NULL) sampint= atof((line+start[1]));
		if(strstr("\"S0_PrePulseDur_ms\"",(line+start[0]))!=NULL) baseline= atof((line+start[1]));
		/* assumes data headers are AD0,AD1 etc */
		if(strstr("\"AD0\"",(line+start[0]))!=NULL) {
			/* find the column of interest */
			for(ii=0;ii<nwords;ii++) if(strstr(tempchan,(line+start[ii]))!=NULL) incol= ii;
			/* read an extra line (just specifies the units, typically mv) */
			if(fgets(line,1000,fpin)==NULL) { sprintf(message,"%s [ERROR]: no AD data in file %s",thisfunc,setinfile); return(-1);}
			break ;
		}
	}
	if(nn<0) { sprintf(message,"%s [ERROR]: NumSamples not specified in header for file %s",thisfunc,setinfile); return(-1);}
	if(sampint<0) { sprintf(message,"%s [ERROR]: SampleInterval not specified in header for file %s",thisfunc,setinfile); return(-1);}
	if(incol<0) { sprintf(message,"%s [ERROR]: specified AD channel %s not found in file %s",thisfunc,setchan,setinfile); return(-1);}
	//TEST:	fprintf(stderr,"nn=%ld\tsampint=%g\tbaseline=%g\tincol=%ld\n",nn,sampint,baseline,incol);


	/********************************************************************************/
	/* STORE THE DATA */
	/********************************************************************************/
	while(fgets(line,1000,fpin)!=NULL) {
		start= xf_lineparse1(line,&nwords);
		if(nwords<1) continue;
		if(sscanf((line+start[incol]),"%f",&aa)!=1) aa=NAN;
		if((tempdata= realloc(tempdata,(mm+1)*sizeofdata))==NULL) { sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(-1); }
		tempdata[mm]= aa;
		mm++;
	}
	if(strcmp(setinfile,"stdin")!=0) fclose(fpin);
	//TEST: for(ii=0;ii<mm;ii++) printf("%ld	%g\n",ii,tempdata[ii]);	printf("mm=%ld\n",mm);
	if(nn!=mm) { sprintf(message,"%s [ERROR]: NumSamples (%ld) does not match actual lines of data (%ld) in file %s",thisfunc,nn,mm,setinfile); return(-1);}

	/********************************************************************************/
	/* FILL THE RESULTS ARRAY */
	/********************************************************************************/
	result_d[0]= sampint;    // sample-interval (ms)
	result_d[1]= 1000.0/sampint;  // sample-rate (Hz)
	result_d[2]= baseline;   // duration of pre-stimulus baseline (ms)

	/********************************************************************************/
	/* ASSIGN NEWLY RESERVED MEMORY TO THE INPUT POINTERS */
	/********************************************************************************/
	(*data1)= tempdata;

	/********************************************************************************/
	/* FREE MEMORY AND RETURN THE NUMBER OF SAMPLES */
	/********************************************************************************/
	if(start!=NULL) free(start);
	return(nn);
}
