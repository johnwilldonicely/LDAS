#define thisprog "xe-matchlist"
#define TITLE_STRING thisprog" v 2.0: JRH, 6.June.2010"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> /* must be included for directory reading */
#include <sys/stat.h> /* must be included for directory reading */
#define MAXLINELEN 1000
#define MAXCOLS 16


/*
<TAGS>database screen</TAGS>
*/

/* function prototype definitions */
void hux_error(char error_txt[]);
int hux_getpath(char *line);

int main (int argc, char *argv[]) {

	/* general and file type variables */
	char line[MAXLINELEN],templine[MAXLINELEN],*pline,*pcol,*tempcol;
	char temp_str[256],path_prog[256],path_data[256];
	char rcfile[256],datafile[256],listfile[256],outfile[256]="stdout",outfile2[256];
	char dcstring[64]="1",lcstring[64]="1";
	int i,j,k,w,x,y,z,n,skip,output,col,colmatch,systemtype,allfiletot=0;
	int dctot,lctot,dc[MAXCOLS],lc[MAXCOLS];
	float a,b,c,tempfa1[MAXCOLS],*match;
	double aa,bb,cc;
	struct stat fileinfo; /* stat structure to hold file info */
	FILE *fpin,*fpout;
	/* temp_str line variables */
	int setpause=0;

	/* Print instructions if only one argument */
	if(argc<3) {
		fprintf(stderr,"\n**************************************************************************\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"- Match data-file columns against list-file columns (numeric only, max 15)\n");
		fprintf(stderr,"- Use this to select lines from large data files\n");
		fprintf(stderr,"- USAGE: %s [datafile] [listfile] [arguments]\n",thisprog);
		fprintf(stderr,"- Arguments - defaults in ()...\n");
		fprintf(stderr,"	-dc [col1,col2...]: datafile columns which must match (%s)\n",dcstring);
		fprintf(stderr,"	-lc [col1,col2...]: listfile columns defining \"good\" values (%s)\n",lcstring);
		fprintf(stderr,"- Example: match \"a.txt\" cols 1,4&5 against \"list.txt\" cols 1,2&3 (respectively):\n");
		fprintf(stderr,"	%s  a.txt  list.txt -dc 1,4,5  -lc 1,2,3\n",thisprog);
		fprintf(stderr,"\n");
		exit(0);
	}

	/******************************************************************************
	Read file names: arguments up to the first argument beginning with "-"
	******************************************************************************/
	sprintf(datafile,"%s",argv[1]);
	sprintf(listfile,"%s",argv[2]);
	for(i=3;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if(i>=argc) break;
			else if(strcmp(argv[i],"-dc")==0)	{strcpy(dcstring,argv[i+1]);i++;}
			else if(strcmp(argv[i],"-lc")==0)	{strcpy(lcstring,argv[i+1]);i++;}
			else {fprintf(stderr,"\nError[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	/* process strings representing lists of columns to be compared */
	pline=dcstring; for(col=0;(pcol=strtok(pline,",\n"))!=NULL&&i<MAXCOLS;col++) {pline=NULL;dc[col]=atoi(pcol);};dctot=col;
	pline=lcstring; for(col=0;(pcol=strtok(pline,",\n"))!=NULL&&i<MAXCOLS;col++) {pline=NULL;lc[col]=atoi(pcol);};lctot=col;

	/* check for illegal numbers of files or duplication of spike file names */
	/* Print user defined options */
	if(lctot!=dctot) {fprintf(stderr,"\nError[%s]: unequal no.of columns to match (%d versus %d)\n\n",thisprog,lctot,dctot);exit(1);}


	/* initialize temp arraay */
	for(i=0;i<MAXCOLS;i++) tempfa1[i]=0.0;

	/* Open list-file to determine memory for matches */
	if((fpin=fopen(listfile,"r"))==NULL) {fprintf(stderr,"\nError[%s]: can't open \"%s\"\n\n",thisprog,listfile);exit(1);}
	i=0;while(fgets(line,MAXLINELEN,fpin)!=NULL) i++;
	/* allocate memory for match array */
	if((match=(float *) malloc((i*lctot+1)*sizeof(float)))==NULL) {fprintf(stderr,"\nError[%s]: insufficient memory\n\n",thisprog);exit(1);}
	rewind(fpin); n=0;
	/* read listfile and store possible match values for each column */
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(line[0]==35) continue;
		pline=line; colmatch=0;
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			for(i=0;i<lctot;i++) if(col==lc[i]) {match[n*lctot+i]=atof(pcol);colmatch++;}
		}
		if(colmatch==lctot) n++;
	}
	fclose(fpin);


	/* Read data file and output matching lines*/
	if((fpin=fopen(datafile,"r"))==NULL) {fprintf(stderr,"\nError[%s]: can't open \"%s\"\n\n",thisprog,datafile);exit(1);}

	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(line[0]==35) {printf("%s",line);continue;}
		strcpy(temp_str,line);
		pline=line;
		colmatch=0;
		/* temporarily store column values */
		z=0;
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			for(j=0;j<dctot;j++) if(col==dc[j]) {tempfa1[j]=atof(pcol); z++;}
		}
		if(z<dctot) continue;
		/* compare column values against good matches from listfile */
		for(i=0;i<n;i++) { // for each line in the original list file...
			colmatch=0;
			for(j=0;j<dctot;j++) { // for each data column to be matched
				if(tempfa1[j]==match[i*lctot+j]) colmatch++;
			}
			// if any set of columns from the list file is matched, print the data line and break
			if(colmatch==lctot) {printf("%s",temp_str); break; }
		}
	}

	fclose(fpin);
	free(match);
	exit(0);
	}
