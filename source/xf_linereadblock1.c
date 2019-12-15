/*
DESCRIPTION:
	- Read a block of data replicates in which specfied key-columns do not change
	- blank lines will be ignored
	- Each call extracts a single block of data
		- the unchanging key-values
		- an array of "repeated-measures" values which are allowed to change within the block
		- an array of data for the block
	- Can be called multiple times until the end of the file is reached
	- 6.May.2019 [JRH]

DEPENDENCIES:
	xf_lineparse2

ARGUMENTS:
	long *par : array of control and results parameters
		par[0] status (output): set to -9 for first call, so function initialises keyprev[]
		par[1] ndata (output): number of data-points extracted
		par[2] nkeys (input): the number of key-colmns
		par[3] repcol (input): column-number holding the repeated-measures values
		par[4] datcol (input): column-number holding the data-values
		par[5] maxdat (input): maximum allowable size of the block
	double *keycurr  : output[nkeys] of current key-values
	double *keyprev  : input[nkeys] of key-values from previous call - use par[0]=-9 to initialize
	long *keycol     : input[nkeys] to hold the column-numbers
	double *rep1     : output[maxdat], repeated-measures-values for the block
	double *dat1     : output[maxdat], data for the block
	FILE *fpin       : file-pointer to input stream - files or stdin
	char *message 	 : pre-allocated array to hold error message

RETURN VALUE: none
	- prams[0] (status) will be updated:
		<0: error (check contents of message)
		 0: end-of-file
		 1: block read successfully, there may be more
	- par[1] will contain the number of datapoints in the extracted block
	- keycurr[nkeys] will contain the fixed key-values for the block
	- keyprev[nkeys] will contain the fixed key-values for the block

TEST PROGRAM:
	nkeys= 3;
	maxdat= 1000;
	keycurr= malloc(nkeys*sizeof(*keyprev));
	keyprev= malloc(nkeys*sizeof(*keyprev));
	rep1= malloc(maxdat*sizeof(*rep1));
	dat1= malloc(maxdat*sizeof(*dat1));

	keycol[0]=5;keycol[1]=6;keycol[2]=7;
	par[0]=-9; par[1]=0; par[2]=nkeys; par[3]=repcol; par[4]=datcol; par[5]=setmaxdat;

	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while(++nn) {
		xf_linereadblock1(par,keycurr,keyprev,keycol,rep1,dat1,fpin,message);
		status= par[0]; ndata= par[1];
		if(status<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		for(ii=0;ii<ndata;ii++) {
			for(jj=0;jj<nkeys;jj++) printf("%g\t",keyprev[jj]);
			printf("%f\t%f\n",rep1[ii],dat1[ii]);
		}
		printf("\n");
		if(status==0) break; // end of file
		if(status==1) for(ii=0;ii<nkeys;ii++) keyprev[ii]= keycurr[ii];
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

<TAGS>file database</TAGS>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define XF_LINEREAD1_BUFFSIZE 1000

long *xf_lineparse2(char *line, char *delimiters, long *nwords);

void xf_linereadblock1(long *par, double *keycurr, double *keyprev, long *keycol, double *rep1, double *dat1, FILE *fpin, char *message)
{
	char *thisfunc="xf_linereadblock1\0";
	char buffer[XF_LINEREAD1_BUFFSIZE],*line=NULL,*pword=NULL;
	int newblock;
	long ii,jj,kk;
	long *iwords=NULL,nwords,nchars,maxlinelen;
	long status,nkeys,repcol,datcol,ndata,maxdat;
	double aa,bb,cc,dd;

	status= par[0];
	ndata=  par[1]= 0;
	nkeys=  par[2];
	repcol= par[3];
	datcol= par[4];
	maxdat= par[5];


	/********************************************************************************/
	/* INITIALIZE MEMORY FOR LINE STORAGE - START WITH MEMORY FOR JUST ONE BUFFER */
	/********************************************************************************/
	maxlinelen= XF_LINEREAD1_BUFFSIZE;
	line= realloc(line,maxlinelen);
	if(line==NULL) { status= -1; goto END; }


	/********************************************************************************/
	/* KEEP READING LINES UNTIL KEYS CHANGE, END-OF-FILE, OR ERROR */
	/********************************************************************************/
	while(1) {

		/********************************************************************************/
		/* GET A LINE */
		nchars=0; line[0]='\0';
		/* GET MULTIPLE BUFFFERS AND CONCATENATE THEM - NOTE THAT fgets STOPS AUTOMATICALLY AFTER A NEWLINE IS READ */
		while(fgets(buffer,XF_LINEREAD1_BUFFSIZE,fpin)!=NULL) {
			/* calculate how many characters were read, not including the terminating '\0' */
			nchars+= strlen(buffer);
			/* if more characters have been read that there is currently storage for, allocate more memory */
			/* note that nchars does not include the terminating null character which must be accommodated, so reallocation must also happen if nchars=maxlinelen */
			if(nchars>maxlinelen) {
				maxlinelen= nchars;
				line= realloc(line,maxlinelen+1);
				if(line==NULL) { status= -1; goto END; }
			}
			/* append current buffer to line */
			strcat(line,buffer);
			/* if the current buffer contains a newline, break */
			if(strstr(buffer,"\n")) break;
		}
		/* IF NOTHING WAS READ INTO THE BUFFER, FINISH THE BLOCK TERMINATING THE WHILE LOOP */
		if(nchars!=0) line[nchars]='\0';
		else {
			line[nchars]='\0';
			status= 0;
			goto END;
		}

		/********************************************************************************/
		/* WE'VE GOT ONE LINE - IF IT'S NOT A NEW BLOCK, ADD IT TO DATA */
		/* parse the newly constructed line */
		iwords= xf_lineparse2(line,"\t",&nwords);
		/* ignore lines with no words on them (blank lines) - not the same as no characters read (see above) */
		if(nwords<1) continue;
		/* if this is the first call, set up the previous key-values */
		if(status==-9) {
			status= 0;
			for(ii=0;ii<nkeys;ii++) {
				pword= line+iwords[keycol[ii]];
				if(sscanf(pword,"%lf",&cc)!=1 || !isfinite(cc)) cc= NAN;
				keyprev[ii]= cc;
		}}
		/* otherwise, save the previous key-values, and retreive rep1 and dat1 from previous storage at te end of the array */
		else if(ndata==0) {
			rep1[0]= rep1[maxdat];
			dat1[0]= dat1[maxdat];
			ndata=1;
			if(ndata>=maxdat) { status= -2; goto END; }
		}

		/********************************************************************************/
		/* NOW STORE REP1, DAT1, & KEYS - CHECK WHETHER A NEW BLOCK STARTED */
		newblock= 0;
		aa= atof(line+iwords[repcol]);          /* temporary store for rep1 */
		bb= atof(line+iwords[datcol]);          /* temporary store for dat1 */
		for(ii=0;ii<nkeys;ii++) {
			pword= line+iwords[keycol[ii]];                                     /* pointer to keyword */
			if(sscanf(pword,"%lf",&cc)!=1 || !isfinite(cc)) cc= NAN;            /* convert to number or NAN  */
			if(cc==keyprev[ii] || (isnan(cc)&&isnan(keyprev[ii]))) newblock++;  /* if cc=previous, increment */
			keycurr[ii]= cc;		/* temporary store for the current value */
		}
		/* if a new block started... */
		if(newblock<nkeys) {
			rep1[maxdat]= aa;
			dat1[maxdat]= bb;
			status= 1;
			break;
		}
		/* otherwise save the data at the current position and increment ndata */
		else {
			rep1[ndata]= aa;
			dat1[ndata]= bb;
			ndata++;
			if(ndata>=maxdat) { status= -2; goto END; }
		}

	} /* END OF PER-BLOCK LOOP */

END:
	/********************************************************************************/
	/* PRINT ERROR MESSAGE, UPDATE STATUS & NDATA, FREE MEMORY AND RETURN */
	/********************************************************************************/
	if(status==-1) sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);
	if(status==-2) sprintf(message,"%s [ERROR]: size of block (%ld) >= maximum (%ld)",thisfunc,ndata,maxdat);
	par[0]= status;
	par[1]= ndata;
	if(line!=NULL) free(line);
	return;
}
