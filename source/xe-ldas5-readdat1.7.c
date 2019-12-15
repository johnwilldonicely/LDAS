#define thisprog "xe-ldas5-readdat1"
#define TITLE_STRING thisprog"v 7: 12.September.2016 [JRH]"

#define MAXLINELEN 1000
#define CHANNELMAX 256

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h> /* to get maximum possible short value */
#include <sys/stat.h>  /* this and next header allow testing file exists using stat() function */
#include <errno.h>

/* <TAGS> file signal_processing filter </TAGS> */

/************************************************************************
v 7: 12.September.2016 [JRH]
	- add ability to define a reference channel

v 7: 2.September.2016 [JRH]
	- add ability to apply a multiplier to the data

v 7: 28.March.2016 [JRH]
	- correct mistake in instructions suggesting "all" could be entered to output all channels
		- this was wrong - to output all channels, the -ch option should be left unset

v.7: 22 January 2016 [JRH]
	- allow decimation of input (-dec) - sets block size
	- bugfix: variable "adjust" was not previously initialized to zero

v 7: 2.November.2015 [JRH]
	- binary write speed improvement
		- previously was writing buffers containing only setchtot values
		- now writes blocks containing all transformed channels selected for output

//??? but there is an error!!

v 6: 18.September.2015 [JRH]
	- simplify calculation of adjustments for unsigned-int input

v 5: 6.August.2015 [JRH]
	- add scaling for the fact that input might actually have a lower bit range than 16bit int

v 4: 4.August.2015 [JRH]
	- add interpolation across invalid values (SHRT_MIN or SHRT_MAX)

v 3: 3.August.2015 [JRH]
	- allow conversion from unsigned to signed
	- allow flipping of the data (-ive-going signals become +ive-going & vice versa)

v 2: 24.July.2015 [JRH]
	- this version uses xf_readbin1_v
		- keeps the file open for multiple read calls
		- compatible with STDIN and extremely large files
		- probably not as fast as other methods?

v 1: 24.July.2015 [JRH]
	- based on now-retired xe-readbinary2 (which was not in use)
	- remove ASCII read option
	- rely on function xf_readbin3_v to read binary file, skip header etc
		- should be faster chunk-read
		- however the function does an fseek() on every call to determine file size - unnecessary
		- alternative version needed which keeps the file open?

*************************************************************************/

/* external functions start */
int xf_readbin1_v(FILE *fpin, void *buffer1, off_t *params, char *message);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],message[256],*pline,*pcol;
	long int ii,jj,kk,nn=0,mm;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	struct stat sts;

	/* program-specific variables */
	void *buffer1=NULL;
	short *data=NULL;
	short *buffer2=NULL,*prevgood=NULL,realmin,realmax,bval,bval2,flipval,tempdata,adjust;
	int *chout=NULL; // array of set channel numbers
	int datasize,itemsread,itemstoread,blocksread;
	long *chindex=NULL,nchout=0,nchoutm1; // array of indices to each element of setchout, and total elements in chout
	long headerbytes=0,maxread;
	off_t params[4]={0,0,0,0},block,nread,nreadtot;
	double *bindata=NULL,*bintime=NULL,timeprev=0.0,interval;
	double scale;
	/* arguments */
	char datatype[256];
	char *setchout=NULL; // pointer to argv if it defines a channel list character array
	int setout=0,setchtot=1,setverb=0,setflip=0,setuint=0,setbad=0,setrep=0,setadj=0,setdecimate=0,setref=-1;
	short setadd=0;
	float setfreq=0.0;
	double setmul=1.0;
	off_t setheaderbytes=0,setstart=0,setntoread=0,setblocksize=0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract values from an interlaced binary dat file\n");
		fprintf(stderr,"	- reads chunks at a time - suitable for very large files\n");
		fprintf(stderr,"	- can extract particular channel(s) or a block of data\n");
		fprintf(stderr,"	- can invert data and convert from unsigned to signed values\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"		- samples are 16-bit short integers\n");
		fprintf(stderr,"		- a sample refers to a multi-channel set of data\n");
		fprintf(stderr,"		- channel and sample indices are zero-offset\n");
		fprintf(stderr,"		- eg. samp2/ch5 of a 16ch input is indexed 2*16+5\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-h: file header size (bytes) [%ld]\n",setheaderbytes);
		fprintf(stderr,"	-b: read-block size (multi-chan. samples) (0=auto ~64KiB) [%d]\n",setblocksize);
		fprintf(stderr,"		NOTE: overridden by -dec\n");
		fprintf(stderr,"	-s: start reading at this sample (zero-offset) [%ld]\n",setstart);
		fprintf(stderr,"	-n: number of samples to read (0=all) [%ld]\n",setntoread);
		fprintf(stderr,"	-nch: total number of channels [%d]\n",setchtot);
		fprintf(stderr,"	-ch: CSV list of channels to extract: 0-(nch-1) [unset= all]\n");
		fprintf(stderr,"	-u: convert all data from unsigned (0=NO 1=YES) [%ld]\n",setuint);
		fprintf(stderr,"	-adj: adjust non-16-bit data (max-bits, or 0=skip) [%ld]\n",setadj);
		fprintf(stderr,"		- NOTE: only applies to data converted from unsigned\n");
		fprintf(stderr,"		- e.g. if input was derived from 12bit numbers, then\n");
		fprintf(stderr,"		  shift the data to maintain the original \"zero\"\n");
		fprintf(stderr,"	-bad: identify an invalid value (0=none -1=-1, 1=max) [%d]\n",setbad);
		fprintf(stderr,"		- protects invalid values from -u conversion\n");
		fprintf(stderr,"		- min/max refer to pre-conversion (-u) values\n");
		fprintf(stderr,"		- min/max: system-defned values for short integers\n");
		fprintf(stderr,"	-rep: replace bad values by most recent good (0=NO 1=YES) [%d]\n",setrep);
		fprintf(stderr,"	-f: flip good data values (0=NO 1=YES) [%ld]\n",setflip);
		fprintf(stderr,"	-add: arbitrary value to add to all data [%hd]\n",setadd);
		fprintf(stderr,"	-mul: arbitrary value to multiply all data by [%g]\n",setmul);
		fprintf(stderr,"	-dec: decimate to every nth sample (0=NO)[%d]\n",setdecimate);
		fprintf(stderr,"	-ref: reference channel to be subtracted (-1=none) [%d]\n",setref);
		fprintf(stderr,"	-out: output format [%d]:\n",setout);
		fprintf(stderr,"		0= ASCII\n");
		fprintf(stderr,"		1= binary interlaced file\n");
		fprintf(stderr,"		NOTE: to write channels to separate files, call this\n");
		fprintf(stderr,"		      program multiple times\n");
		fprintf(stderr,"	-verb: verbose output (0=low,1=high,2=highest) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -nch 64 -ch 0,1,2\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -nch 64 -ch 62,63\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	/************************************************************/
	sprintf(infile,"%s\0",argv[1]);
	if(strcmp(infile,"stdin")!=0 && stat(infile,&sts)==-1 && errno == ENOENT) {
		fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);
		exit(1);
	}
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-nch")==0)   setchtot=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-ch")==0)    setchout=argv[++ii];
			else if(strcmp(argv[ii],"-bad")==0)   setbad=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-rep")==0)   setrep=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-u")==0)     setuint=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-f")==0)     setflip=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-adj")==0)   setadj=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-add")==0)   setadd=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-mul")==0)   setmul=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-dec")==0)   setdecimate=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-ref")==0)   setref=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-h")==0)     setheaderbytes=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-b")==0)     setblocksize=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-s")==0)     setstart=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-n")==0)     setntoread=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)   setout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)  setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setchtot<1||setchtot>CHANNELMAX) {fprintf(stderr,"\n--- Error[%s]: invalid -nch (%d) : must be >0 and <%d\n\n",thisprog,setchtot,CHANNELMAX);exit(1);}
	if(setbad<-1||setbad>1) {fprintf(stderr,"\n--- Error[%s]: invalid -bad (%d) : must be -1, 0, or 1 \n\n",thisprog,setbad);exit(1);}
	if(setrep<0 || setrep>1) {fprintf(stderr,"\n--- Error[%s]: invalid -rep (%d) : must be 0-1\n\n",thisprog,setrep);exit(1);}
	if(setout<0 || setout>1) {fprintf(stderr,"\n--- Error[%s]: invalid -out (%d) : must be 0-1\n\n",thisprog,setout);exit(1);}
	if(setuint<0 || setuint>1) {fprintf(stderr,"\n--- Error[%s]: invalid -u (%d) : must be 0-1\n\n",thisprog,setuint);exit(1);}
	if(setflip<0 || setflip>1) {fprintf(stderr,"\n--- Error[%s]: invalid -i (%d) : must be 0-1\n\n",thisprog,setflip);exit(1);}
	if(setverb<0 || setverb>2) {fprintf(stderr,"\n--- Error[%s]: invalid -verb (%d) : must be 0-2\n\n",thisprog,setverb);exit(1);}
	if(setadj!=0 && (setadj<2 || setadj>15)) {fprintf(stderr,"\n--- Error[%s]: invalid -adj (%d) : must be 0 or 2-15\n\n",thisprog,setadj);exit(1);}
	if(setblocksize<0) {fprintf(stderr,"\n--- Error[%s]: invalid -b (%d) : must be >= 0\n\n",thisprog,setblocksize);exit(1);}
	if(setdecimate<0) {fprintf(stderr,"\n--- Error[%s]: invalid -dec (%d) : must be >= 0\n\n",thisprog,setdecimate);exit(1);}
	if(setref<-1) {fprintf(stderr,"\n--- Error[%s]: invalid -ref (%d) : must be >= -1\n\n",thisprog,setref);exit(1);}
	if(setref>=setchtot) {fprintf(stderr,"\n--- Error[%s]: invalid -ref (%d): must be less than total channels (%d)\n\n",thisprog,setref,setchtot);exit(1);}
	if(setblocksize>0&&setdecimate>0) {fprintf(stderr,"\n--- Error[%s]: cannot set both -b (%g) and -dec (%g), because -dec overrides\n\n",thisprog,setblocksize,setdecimate);exit(1);}

	datasize=sizeof(short);
	adjust=0.0;

	/* convert flags to modifier values for data */
	if(setuint==0) {
	 	if(setbad==-1) bval=-1;
		if(setbad==1)  bval=SHRT_MAX;
		bval2= bval;
	}
	else {
	 	if(setbad==-1) bval=0;
		if(setbad==1)  bval=SHRT_MAX-SHRT_MIN;
		bval2= SHRT_MAX/2 - SHRT_MIN/2;
		if(setadj==0) adjust=SHRT_MIN;
		if(setadj!=0) adjust=powl(2,(setadj-1)) * -1;
	}

 	if(setflip==0) setflip=1;
	else setflip=-1;

	adjust+= setadd*setflip;
	scale= setmul*(double)setflip;

	if(setverb==1) {
		fprintf(stderr,"adjust= %hd\n",adjust);
		fprintf(stderr,"signed_bval= %hd\n",bval);
		fprintf(stderr,"unsigned_bval= %hu\n",bval);
		fprintf(stderr,"output_bval2= %hd\n",bval2);
		fprintf(stderr,"setuint= %hd\n",setuint);
		fprintf(stderr,"modified_setflip= %d\n",setflip);
		fprintf(stderr,"scale= %g\n",scale);
	}

	/************************************************************/
	/* IF A SUBSET OF THE CHANNELS IS TO BE OUTPUT, PARSE THE SETCHOUT STRING */
	/************************************************************/
	if(setchout!=NULL) {
		chindex= xf_lineparse2(setchout,",",&nchout);
		if(nchout>setchtot) {fprintf(stderr,"\n--- Error[%s]: number of channels selected (%d) cannot exceed channel total (%d)\n\n",thisprog,nchout,setchtot,CHANNELMAX);exit(1);}
		if((chout=(int *)realloc(chout,(nchout)*sizeof(int)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<nchout;ii++) {
			chout[ii]=atol(setchout+chindex[ii]);
			if(chout[ii]>=setchtot) {fprintf(stderr,"\n--- Error[%s]: channel number (%d) must be less than channel total (%d)\n\n",thisprog,chout[ii],setchtot);exit(1);}
			if(chout[ii]<0) {fprintf(stderr,"\n--- Error[%s]: channel %d is less than zero\n\n",thisprog);exit(1);}
	}}
	else {
		nchout=setchtot;
		if((chout=(int *)realloc(chout,(nchout)*sizeof(int)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<setchtot;ii++) chout[ii]=ii;
	}

	/************************************************************/
	/* DETERMINE OPTIMAL (~64KiB) BLOCK SIZE (MULTI_CHANNEL SAMPLES) */
	/* overridden by -b or -dec */
	/************************************************************/
	if(setblocksize==0){
		if(setdecimate!=0) setblocksize=setdecimate;
		else {
			ii= datasize*setchtot; // number of bytes per multi-channel data chunk
			aa= pow(2.0,16.0)/(double)ii; // number of chunks per 64KiB
			setblocksize= (off_t)pow(2,(log2(aa))); // find the next lowest power of two
		}
	}

	/************************************************************/
	/* ALLOCATE MEMORY */
	/************************************************************/
	maxread= setblocksize*setchtot*datasize;
	if((buffer1=  realloc(buffer1,maxread))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((buffer2=  realloc(buffer2,maxread))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((prevgood= realloc(prevgood,nchout*datasize))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(ii=0;ii<nchout;ii++) prevgood[ii]=bval2;
	//for(ii=0;ii<nchout;ii++) fprintf(stderr,"%d	%d	%d\n",ii,chout[ii],prevgood[ii]);

	/************************************************************/
	/* OPEN THE INPUT */
	/************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"rb"))==0) {fprintf(stderr,"\n--- Error[%s]: could not open file: %s\n\n",thisprog,infile);exit(1);}

	/************************************************************/
	/* SKIP THE REQUIRED NUMBER OF BYTES - HEADER PLUS DATA TO SKIP */
	/************************************************************/
	ii= setheaderbytes+(setstart*setchtot*datasize);
	if(ii>0) {
		if(fseeko(fpin,ii,SEEK_CUR)!=0) {
				fprintf(stderr,"\n--- Error[%s]:  problem reading binary file (errno=%d)\n\n",thisprog,ferror(fpin));
				return(0);
	}}

	/************************************************************/
	/* READ THE DATA */
	/************************************************************/
	if(setverb==2) fprintf(stderr,"\treading block: ");
	block=0;

	params[0]=datasize*setchtot; // ensures an error if bytes read do not match datasize and channel count
	params[1]=setblocksize;
	nreadtot=0;
	z=0; // break flag
	nchoutm1=nchout-1;

	while(!feof(fpin)) {
		if(setverb==2) fprintf(stderr,"%9ld\b\b\b\b\b\b\b\b\b",block);
		/* read a block of data */
		x= xf_readbin1_v(fpin,buffer1,params,message);
		/* check for error (fail to read, bad number of bytes read) */
		if(x<0)	{fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
		/* get the number of multi-channel data read */
		nread=params[2];
		/* if no data was read this time, that's the end of the file! */
		if(nread==0) break;
		/* if too much data was read, adjust the nread variable to reduce the amount of output and set the break flag */
		ii=nreadtot+nread;
		if(setntoread>0) { if(ii>=setntoread) { nread=setntoread-nreadtot; z=1; }}
		/* if decimation was set, only the first sample counts */
		if(setdecimate>0) nread=1;
		/* set a pointer to the buffer1 */
		data=(short *)buffer1;

		/* SUBTRACT A REFERENCE-CHANNEL FROM THE DATA, PROVIDED THE CURRENT SAMPLE IS NOT INVALID */
		if(setref>-1) {
			for(ii=0;ii<nread;ii++) {
				kk=ii*setchtot;
				if(setbad==0||data[kk]!=bval) {
					for(jj=0;jj<nchoutm1;jj++) {
						if(chout[jj]!=setref) data[kk+chout[jj]]-= data[kk+setref];
					}
					data[kk+setref]=0;;
		}}}

		/* OUTPUT-TYPE-0: ASCII */
		if(setout==0) {
			for(ii=0;ii<nread;ii++) {
				kk=ii*setchtot;
				/* do the initial channels, with tabs */
				for(jj=0;jj<nchoutm1;jj++) {
					tempdata= data[kk+chout[jj]];
					if(setbad==0) tempdata=(tempdata+adjust)*scale;
					else {
						if(tempdata==bval) {
							if(setrep==1) tempdata=prevgood[jj];
							else tempdata= bval2;
						}
						else {
							tempdata= (tempdata+adjust)*scale;
							prevgood[jj]= tempdata;
						}
					}
					printf("%hd\t",tempdata);
				}
				/* now do the last channel, with newline */
				tempdata= data[kk+chout[jj]];
				if(setbad==0) tempdata=(tempdata+adjust)*scale;
				else {
					if(tempdata==bval) {
						if(setrep==1) tempdata=prevgood[jj];
						else tempdata= bval2;
					}
					else {
						tempdata= (tempdata+adjust)*scale;
						prevgood[jj]= tempdata;
					}
				}
				printf("%hd\n",tempdata);
		}}

		/* OUTPUT TYPE-1: BINARY */
		else if(setout==1) {
			for(ii=0;ii<nread;ii++) {
				kk=ii*setchtot; /* index to input  */
				mm=ii*nchout;	/* index to output */
				for(jj=0;jj<nchout;jj++) {
					tempdata=data[kk+chout[jj]];
					if(setbad==0||tempdata!=bval) {
						tempdata= (tempdata+adjust)*scale;
						prevgood[jj]=tempdata;
					}
					else if(setrep==1) tempdata=prevgood[jj];
					else tempdata= bval2;
					buffer2[mm+jj]= tempdata;
				}
			}
			fwrite(buffer2,datasize*nchout,nread,stdout);
		}

		/* increment the total amount read */
		nreadtot+=nread;
		block++;
		/* once the last bit of data is read, z should have been set to 1 : if so, break */
		if(z!=0) break;
	}

	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(setverb==1) fprintf(stderr,"\n\tread %ld multi-channel records\n",nreadtot);

	if(prevgood!=NULL) free(prevgood);
	if(buffer1!=NULL) free(buffer1);
	if(buffer2!=NULL) free(buffer2);
	if(chindex!=NULL) free(chindex);
	if(chout!=NULL) free(chout);
	exit(0);

}
