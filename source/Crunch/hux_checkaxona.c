/************************************************************************
- Determines validity and number of records in DACQ file
- Should be called before reading data from DACQ files
- Allows calling function to allocate sufficient memory to store records
- pass pointers to input filename and file type
- returns number of records, or -1 if file is unreadable/invalid
- prints warning if file contains no records (header only)

- Read file header - leave file pointer at first point binary data point
- at the end of the file are 12 bytes: 0x0d, 0x0a, data_end, 0x0d, 0x0a
- file length = header_length + 12 + (num_records * record_size)

*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h> /* must be included for directory reading */
#include <sys/stat.h> /* must be included for directory reading */

#define HEADERMAX 1000

void hux_error(char message[]);
int hux_getfileheader(FILE *fpin, char *header,	char *header_stop, int headermax);
int hux_getfilename(char *line);
int hux_getword(char *line, char *trigger, int word, char *target);
int hux_substring(char *source, char *target, int casesensitive);


int hux_checkaxona (char *infile, float *result)	
{
	char line[1000], temp_str[256], header[HEADERMAX], header_stop[64];
	int i,x,y,z,headerlength=0,footerlength=0,rectot=0,recsize=0;
	int nprobes=0,bytes_per_sample=0, samples_per_spike=0,bytes_per_timestamp=0,num_colours=0;
	struct stat fileinfo; /* stat structure to hold file info */
	FILE *fpin;

	result[0]=result[1]=0.0;
	footerlength=12; // this is the number of bytes at the bottom of Axona binary files, including the "data_end" message

	/* check that file exists and determine size (stored as fileinfo.st_size*/
	if(stat(infile,&fileinfo)==-1) {sprintf(temp_str,"hux_checkaxona: can't open \"%s\"\n\n",infile);hux_error(temp_str);}
	else if((fileinfo.st_mode & S_IFMT) == S_IFDIR){sprintf(temp_str,"hux_checkaxona: this is a directory, not an input file: %s",infile);hux_error(temp_str);}

	/* use filename extention to determine file data-type */
	if(hux_substring(infile,".pos",0)>=0) result[0]=1.0;
	else if(hux_substring(infile,".eeg",0)>=0) result[0]=2.0;
	else if(hux_substring(infile,".eg2",0)>=0) result[0]=2.0;
	else{
		/* otherwise, see if its a number from 0 to MAXPROBES */
		for(i=0;i<64;i++) {
			sprintf(temp_str,".%d\0",i); 
			x=(hux_substring(infile,temp_str,0)>=0);
			if(hux_substring(infile,temp_str,0)>=0) result[0]=3.0;
		} 
		if(result[0]<=0.0) {sprintf(temp_str,"hux_checkaxona: file has no valid Axona extension (.pos .eeg .[1-64]): %s\0",infile);hux_error(temp_str);}
	}

	/* Open file and read file header: file pointer will be poised at next byte */
	fpin=fopen(infile,"r+b");
	if(fpin==NULL) {sprintf(temp_str,"hux_checkaxona: can't open file\n\t\tPath=%s\n\n",infile);hux_error(temp_str);}
	sprintf(header_stop,"data_start\0");
	headerlength = hux_getfileheader(fpin,header,header_stop,HEADERMAX) * sizeof(char);
	if(headerlength==-1) {sprintf(temp_str,"hux_checkaxona: no file header ending in %s",header_stop); hux_error(temp_str);}
	fclose(fpin);

	if(result[0]==1.0) {
		hux_getword(header,"num_pos_samples",1,line); rectot=atoi(line);
		hux_getword(header,"bytes_per_timestamp",1,line); bytes_per_timestamp=atoi(line);
		hux_getword(header,"num_colours",1,line); num_colours=atoi(line);
		hux_getword(header,"bytes_per_coord",1,line); bytes_per_sample=atoi(line);
		recsize = bytes_per_timestamp + num_colours*2*bytes_per_sample;
	}
	if(result[0]==2.0) {
		hux_getword(header,"num_EEG_samples",1,line); rectot=atoi(line);
		hux_getword(header,"num_chans",1,line); nprobes=atoi(line);
		hux_getword(header,"bytes_per_sample",1,line); bytes_per_sample=atoi(line);
		recsize = nprobes*bytes_per_sample;
	}
	if(result[0]==3.0) { /* determine what type of electrode this actually is - starts with assumption it's a single electrode */
		hux_getword(header,"num_spikes",1,line); rectot=atoi(line); 
		hux_getword(header,"bytes_per_timestamp",1,line); bytes_per_timestamp=atoi(line);
		hux_getword(header,"num_chans",1,line); nprobes=atoi(line);
		hux_getword(header,"bytes_per_sample",1,line); bytes_per_sample=atoi(line);
		hux_getword(header,"samples_per_spike",1,line); samples_per_spike=atoi(line);
		recsize = nprobes*bytes_per_timestamp + nprobes*samples_per_spike*bytes_per_sample;
		if(nprobes==1) result[0]=3.0;
		if(nprobes==2) result[0]=4.0;
		if(nprobes==4) result[0]=5.0;
	}

	y = fileinfo.st_size; z = headerlength + rectot*recsize + footerlength;
	if(y != z) {sprintf(temp_str,"hux_checkaxona: file size (%d) doesn't match expected (%d)\n\t\theader size: %d\n\t\trecords: %d\n\t\trecordsize: %d\n\t\tfooter size: %d",y,z,headerlength,rectot,recsize,footerlength);hux_error(temp_str);}
	if(rectot == 0) hux_error("hux_checkaxona: file has no records");
	result[1]=(float)rectot;
	return (rectot);
}
