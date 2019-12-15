/************************************************************************
- determines validity and number of records in Csicsvari lab format file
- must be called before reading data from these files
- result[0] = file type 
	1 = video (accepts 3 types: .whl .whd or .xyd)
	2 = eeg
	3 = electrode
	4 = stereortrode
	5 = tetrode,6=event)
- result[1] = number of records
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h> /* must be included for directory reading */
#include <sys/stat.h> /* must be included for directory reading */

void hux_error(char message[]);
int hux_getfilename(char *line);
int hux_substring(char *source, char *target, int casesensitive);

int hux_checkcsicsvari (
	char *infile,	/* name of input file */
	int channeltot,	/* for eeg, how many channels? (e.g. 33=32+sync for 8 tetrodes, 64 for 16 tetrodes)*/
	float *result	/* resul[0]=type,result[1]=rectot */
	)	
{
	char command[256],cfile[256],line[256];
	long int time;
	int i,j,x,y,d,words_per_record,size,headerlength=0,rectot=0,clustertot=0;
	float xpos,ypos,hdir;
	struct stat fileinfo; /* stat structure to hold file info */
	FILE *fpin;

	/* check that file exists and determine size (stored as fileinfo.st_size*/
	if(stat(infile,&fileinfo)==-1) {sprintf(command,"hux_checkcsicsvari: can't open \"%s\"\n\n",infile);hux_error(command);}
	else if((fileinfo.st_mode & S_IFMT) == S_IFDIR){sprintf(command,"hux_checkcsicsvari: this is a directory, not an input file: %s",infile);hux_error(command);}

	/* determine file data-type (first instnce of .xxx, not necessarily at end of file name) */
	if(hux_substring(infile,".whl",0)>=0) result[0]=1.0; // position records: x,y
	else if(hux_substring(infile,".whd",0)>=0) result[0]=(float)1.5; // position+direction records, x,y,dir
	else if(hux_substring(infile,".xyd",0)>=0) result[0]=(float)1.7; // time+position+direction records: time,x,y,dir
	else if(hux_substring(infile,".eegh",0)>=0) result[0]=(float)2.1;
	else if(hux_substring(infile,".eeg",0)>=0) result[0]=(float)2.0;
	else if(hux_substring(infile,".res",0)>=0) result[0]=(float)5.0;
	else if(hux_substring(infile,".aaaaaa",0)>=0) result[0]=(float)6.0;
	else{sprintf(command,"hux_checkcsicsvari: file has no valid Csicsvari extension (.whl/.whd .eeg .eegh .res): %s\n",infile+hux_getfilename(infile));hux_error(command);}

	/* for ascii video (.whl .whd or .xyd) files, simply count number of lines  */
	/* assumes file contains only lines with two three or four values each - x and y (and dir for .whd, and time for .xyd)*/
	if(result[0]>=1.0 && result[0]<2.0){
		fpin=fopen(infile,"r"); 
		if(fpin==NULL){sprintf(command,"hux_checkcsicsvari: can't open video file\n\t\tPath=%s\n\n",infile);hux_error(command);}
		else {
			if(result[0]==(float)1.0) while(!feof(fpin)) {if(fscanf(fpin,"%f %f",&xpos,&ypos)!=2) break; else rectot++;}
			if(result[0]==(float)1.5) while(!feof(fpin)) {if(fscanf(fpin,"%f %f %f",&xpos,&ypos,&hdir)!=3) break; else rectot++;}
			if(result[0]==(float)1.7) while(fgets(line,1000,fpin)!=NULL) {if(line[0]!='#' && sscanf(line,"%d %d %d %d",&time,&x,&y,&d)==4) rectot++;}
		}
		fclose(fpin);
	}

	/* for continuous records (.eeg or .eegh files), check binary data data format against file size */ 
	else if((int)result[0]==2) {
		size = sizeof(short int)*(channeltot); /* channeltot should include sync channel !! */ 
		rectot = (fileinfo. st_size - headerlength)/size;
		if((fileinfo.st_size - headerlength) % size != 0) { 
			printf("\n\tfilesize=%d,headerlength=%d,recordsize=%d,channeltot=%d\n",fileinfo.st_size,headerlength,size,channeltot);
			sprintf(command,"hux_checkcsicsvari: \n\t\t\"%s\" is an invalid EEG file\n\t\t\t- perhaps %d is the wrong number of channels?\n", infile,channeltot);
			hux_error(command);
		}}

	/* for ascii tetrode (.res) files, count number of lines with one number on them, check vs. matching .clu file */
	else if(result[0]==(float)5.0){
		words_per_record = 1;
		fpin=fopen(infile,"r"); 
		if(fpin==NULL){sprintf(command,"hux_checkcsicsvari: can't open spike file\n\t\tPath=%s\n\n",infile);hux_error(command);}
		else while(!feof(fpin)) {
			if(fscanf(fpin,"%ld",&time)!= words_per_record) break;  /* increment counter so long as 1 item can be read (time) */ 
			else rectot++;
		}
		fclose(fpin);
		strcpy(cfile,infile);
		i = hux_substring(cfile,".res",0);
		cfile[i+1]='c';	cfile[i+2]='l';	cfile[i+3]='u';
		fpin=fopen(cfile,"r");
		if(fpin==NULL){sprintf(command,"hux_checkcsicsvari: can't open associated cluster file %s",cfile);hux_error(command);}
		else while(!feof(fpin)) {
			if(fscanf(fpin,"%d",&i)!= words_per_record) break;  /* incriment counter so long as 1 items can be read */ 
			else clustertot++;
		}
		fclose(fpin);
		if(rectot!=clustertot-1){sprintf(command,"hux_checkcsicsvari: .res and .clu files do not have same number of records (%d vs %d)",rectot,(clustertot-1));hux_error(command);}
	}

	else {sprintf(command,"hux_checkcsicsvari: processing this file type unimplimented at present: %s",cfile);hux_error(command);}
	result[1]=(float)rectot;

	return (rectot);
}
