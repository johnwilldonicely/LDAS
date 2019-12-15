#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-insert1"
#define TITLE_STRING thisprog" v 1: 22.November.2018 [JRH]"

/*
<TAGS>string</TAGS>

v 1: 22.November.2018 [JRH]
	- bugfix behaviour is start is not set
v 1: 12.November.2017 [JRH]
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char *line1=NULL,*line2=NULL;
	long ii,maxlinelen1=0,maxlinelen2=0;
	FILE *fpin1,*fpin2;
	/* program-specific variables */
	int out=0;
	/* arguments */
	char *infile1=NULL,*infile2=NULL,*setstart=NULL;
	int setpos=1;

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Insert into file1 the contents of file2\n");
		fprintf(stderr,"- default is to simply concatenate file2 to the end of file1 \n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [file1] [file2]\n",thisprog);
		fprintf(stderr,"	[file1]: file to be added to\n");
		fprintf(stderr,"	[file2]: file whose contents is to be added\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-start: insert when this quoted text is found in file1 [unset]\n");
		fprintf(stderr,"	-pos:   position to insert (-1=before,1=after) [%d]\n",setpos);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt temp.txt -s1 \"# block1\" \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	the desired block of text, sent to stdout\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME, START-SIGNAL AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile1= argv[1];
	infile2= argv[2];
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-start")==0) setstart=argv[++ii];
			else if(strcmp(argv[ii],"-pos")==0)   setpos=atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setpos!=-1 && setpos!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -pos [%d]: must be -1 or 1\n\n",thisprog,setpos);exit(1);}

	/* READ THE DATA  */
	if((fpin1=fopen(infile1,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile1);exit(1);}
	if((fpin2=fopen(infile2,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile2);exit(1);}

	while((line1=xf_lineread1(line1,&maxlinelen1,fpin1))!=NULL) {
		if(maxlinelen1==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

		/* do nothing but print the line if start was not set */
		if(setstart==NULL) printf("%s",line1);
		/* otherwise, check line for presence of the start-signal */
		else if(strstr(line1,setstart)!=NULL) {
			if(setpos==1) printf("%s",line1);
			while((line2=xf_lineread1(line2,&maxlinelen2,fpin2))!=NULL) {
				if(maxlinelen2==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
				printf("%s",line2);
			}
			if(setpos==-1) printf("%s",line1);
		}
		else printf("%s",line1);
	}
	fclose(fpin1);

	/* if start was not set, concatenate infile2 */
	if(setstart==NULL) {
		while((line2=xf_lineread1(line2,&maxlinelen2,fpin2))!=NULL) {
			if(maxlinelen2==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			printf("%s",line2);
	}}

	fclose(fpin2);

	if(line1!=NULL) free(line1);
	if(line2!=NULL) free(line2);
	exit(0);
}
