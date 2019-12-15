/************************************************************************
- determines validity and number of records in Neuralynx file
- should be called before reading data from Neuralynx files
- allows calling function to allocate sufficient memory to store records
- pass pointers to input filename and file type
- requires #include "neuralynx.h" in calling function
- returns number of records, or -1 if file is unreadable/invalid
- prints warning if file contains no records (header only)
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h> /* must be included for directory reading */
#include <sys/stat.h> /* must be included for directory reading */
#include "neuralynx.h"


void hux_error(char message[]);
int hux_getfilename(char *line);
int hux_substring(char *source, char *target, int casesensitive);

int hux_checknlx (char *infile, float *result)	
{
	char command[256];
	int size,headerlength,rectot=0;
	struct stat fileinfo; /* stat structure to hold file info */
	struct CRRec EEGRec;
	int EEGSize=sizeof(struct CRRec);
	FILE *fpin;

	headerlength=16384; /* size of ascii header on binary Neuralynx files */

	/* check that file exists and determine size (stored as fileinfo.st_size*/
	if(stat(infile,&fileinfo)==-1) {sprintf(command,"hux_checknlx: can't open \"%s\"\n\n",infile);hux_error(command);}
	else if((fileinfo.st_mode & S_IFMT) == S_IFDIR){sprintf(command,"hux_checknlx: this is a directory, not an input file: %s",infile);hux_error(command);}

	/* determine file data-type (first instnce of .xxx, not necessarily at end of file name) */
	if(hux_substring(infile,".Nvt",0)>=0) {result[0]=1.0; size = sizeof(struct VideoRec);}
	else if(hux_substring(infile,".Ncs",0)>=0) {result[0]=2.0; size = sizeof(struct CRRec);}
	else if(hux_substring(infile,".Ntt",0)>=0) {result[0]=5.0; size = sizeof(struct TTRec);}
	else if(hux_substring(infile,".Nev",0)>=0) {result[0]=6.0; size = sizeof(struct EventRec);}
	else{sprintf(command,"hux_checknlx: file has no valid Neuralynx extension (.Ncs .Ntt Nev): %s\n",infile+hux_getfilename(infile));hux_error(command);}

	if(result[0]==1.0) size = sizeof(struct VideoRec);
	if(result[0]==2.0) size = sizeof(struct CRRec);
	if(result[0]==3.0) size = sizeof(struct SERec); /* unimplimented at present - assumes file is tetrode - fix!! ???*/
	if(result[0]==4.0) size = sizeof(struct STRec); /* unimplimented at present - assumes file is tetrode - fix!! ???*/
	if(result[0]==5.0) size = sizeof(struct TTRec);
	if(result[0]==6.0) size = sizeof(struct EventRec);

	/* check that file exists and determine size (stored as fileinfo.st_size*/
	if(stat(infile,&fileinfo)==-1) {sprintf(command,"hux_checknlx: can't open file\n\t\tPath=%s\n\n",infile);hux_error(command);}
	/* (filesize minus headersize) divided by recordsize should be integer*/
	else if(((fileinfo.st_size - headerlength) % size) != 0) {
		if(result[0]==1) sprintf(command,"hux_checknlx: invalid video file: %s",infile);
		if(result[0]==2) sprintf(command,"hux_checknlx: invalid EEG file: %s",infile);
		if(result[0]==5) sprintf(command,"hux_checknlx: invalid spike file: %s",infile);
		hux_error(command);
	}
	else rectot = (fileinfo.st_size - headerlength)/size;

	/* For EEG records, multiply rectot by the number of samples per Neuralynx record */
	if(result[0]==2) {
		if((fpin=fopen(infile,"rb"))==NULL) {sprintf(command,"hux_checknlx: can't open EEG file: %s",infile);hux_error(command);}
		else {
			fseek(fpin,headerlength,SEEK_SET);
			if(fread(&EEGRec,EEGSize,1,fpin)==0) hux_error("hux_checknlx could not read EEG file"); 
			else rectot=rectot*EEGRec.dwNumValidSamples;
		}
		fclose(fpin);
	}

	result[1]=(float)rectot;
	return (rectot);
}
