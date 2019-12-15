#define thisprog "xe-progdep"
#define TITLE_STRING thisprog" v 5: 26.January.2018 [JRH]"
#define MAXLINELEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>programming</TAGS>

v 5: 26.January.2018 [JRH]
	- tweak to linecomment function

v 5: 21.May.2016 [JRH]
	- switch to new long-int version of strstr1

v 5: 11.July.2013 [JRH]
	- bugfix: complete reworking of comment/quote removal, now moved to function xf_linecomment1
		- previously, the presence of "#" or "//" automatically terminated the rest of the line
		- quotes within quotes used to be counted
		- quotes within comments were previously counted
	- bugfix: xs- xe- or xf_ now do not have to be the first character in a word for it to be recognized as a dependency
		- this means defining a variable as a program name, provided it is not enclosed in quotes, will now make it appear as a dependency

v 4: 18.June.2013 [JRH]
	- bugfix - should now ignore quoted text (single or double quotes)

v 3: 7.June.2013 [JRH]
	- script basenames can now include "."

v 1.2: 9.March.2013 [JRH]
	- update to use xf_strstr1 - a more general function that allows detection of the first or last of a series of characters in a string
	- the old function was xf_strstr
*/


/* external functions start */
int xf_linecomment1(char *line, int *skip1);
long xf_strstr1(char *haystack,char *needle,int setcase,int firstlast);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],line[MAXLINELEN],*pline,*pcol;
	long i,j,k,n,zz;
	int v,w,x,y,col;
	int sizeofchar=sizeof(char),sizeofint=sizeof(int);
	FILE *fpin;
	/* program-specific variables */
	char *words=NULL, path[256],basename[256],tempword[256],prev;
	int *iword=NULL, lenwords=0,nwords=0, basetype=0,skip=0,q1=0,q2=0,q3=0,q4=0;
	/* arguments */

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Check dependencies in a script or program\n");
		fprintf(stderr,"Looks for any mention of a word containing xs-, xe-, or xf_ \n");
		fprintf(stderr,"	- allows definition of dependency (e.g. prog=[progname) to be \n");
		fprintf(stderr,"	detected, PROVIDED quotes are not used\n");
		fprintf(stderr,"	- any other reference to a script/prog/function may be falsely \n");
		fprintf(stderr,"	detected, if NOT enclosed in quotes\n");
		fprintf(stderr,"Ignores additional references to the same script/program/function\n");
		fprintf(stderr,"Ignores comments:\n");
		fprintf(stderr,"	- all text between /* and */ \n");
		fprintf(stderr,"	- everything after the first # or // on a given line\n");
		fprintf(stderr,"Ignores quoted text (between single or double-quotes)\n");
		fprintf(stderr,"Word delimiters: |(){}; \"\'\\t\\n\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [file-name]\n",thisprog);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s xs-myscript\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s\0",argv[1]);

	/* OPEN THE INPUT FILE USING FULL SPECIFIED PATH */
	if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	n=0;

	/* REMOVE PATH DESIGNATION (IF PRESENT) FROM INPUT FILE NAME - IT'S NOT NEEDED AT THIS POINT */
	pcol=strrchr(infile,'/');
	if(pcol!=NULL) sprintf(basename,"%s\0",++pcol);
	else sprintf(basename,"%s\0",infile);

	/* DETERMINE INFILE TYPE - STRIP EVERYTHING AFTER THE FIRST "." FOR  xe- and xf_ FILES */
	basetype=0;
	if(xf_strstr1(basename,"xs-",1,1)>=0) basetype=1;
	else {
		if(xf_strstr1(basename,"xe-",1,1)>=0) basetype=2;
		if(xf_strstr1(basename,"xf_",1,1)>=0) basetype=3;
		zz=xf_strstr1(basename,".",1,1);
		if(zz>=0) basename[zz]='\0';
	}

	/* READ THE FILE LINE-BY LINE LOOKING FOR WORDS SATRING WITH xs- xe- or xf_ */
	skip=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {

		/* negate commented or quoted text */
		xf_linecomment1(line,&skip);

		/* search for xs- xe- or xf_, indicators of script program or function dependencies  */
		pline=line;
		for(col=1;(pcol=strtok(pline,"|(){}; \t\n"))!=NULL;col++) {

			pline=NULL;
			zz=-1; /* flag indicating a script, program or function name has been found */
			if((zz=xf_strstr1(pcol,"xs-",1,1))>=0)  	{
				pcol+=zz;
				if(strcmp(pcol,basename)==0) continue; /* this is a recursive call - do not report */
			}
			else if((zz=xf_strstr1(pcol,"xe-",1,1))>=0) {
				pcol+=zz;
				if(strcmp(pcol,basename)==0) continue; /* this is a recursive call - do not report */
				if(basetype==2) {fprintf(stderr,"\n--- Warning[%s]: program %s appears to call another program %s\n\n",thisprog,infile,pcol);continue;}
				if(basetype==3) {fprintf(stderr,"\n--- Warning[%s]: function %s appears to call a program %s\n\n",thisprog,infile,pcol); continue;}
			}
			else if((zz=xf_strstr1(pcol,"xf_",1,1))>=0) {
				pcol+=zz;
				if(strcmp(pcol,basename)==0) continue; /* this is a recursive call - do not report */
				if(basetype==1) {fprintf(stderr,"\n--- Warning[%s]: script %s appears to call a function %s\n\n",thisprog,infile,pcol); continue;}
			}
			else continue; /* indicates no xs- xe- or xf- were found */


			/* check to see if label already exists - if so, ignore it */
			zz=0; for(j=0;j<nwords;j++) if(strcmp(pcol,(words+iword[j]))==0) zz=1; if(zz==1) continue;
			/* allocate memory for expanded words and word-index */
			x=strlen(pcol); // not including terminating NULL
			words=(char *)realloc(words,((lenwords+x+4)*sizeofchar)); if(words==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			iword=(int *)realloc(iword,(nwords+1)*sizeofint); if(words==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			/* set pointer to start position (currently, the end of the labels string) */
			iword[nwords]=lenwords;
			/* add new word to end of words, adding terminal NULL */
			sprintf(words+lenwords,"%s\0",pcol);
			/* update length, allowing for terminal NULL - serves as pointer to start of next word */
			lenwords+=(x+1);
			/* incriment nwords with check */
			nwords++;
		}
	}
	fclose(fpin);

	for(i=0;i<nwords;i++) printf("%s\n",(words+iword[i]));

	free(words);
	free(iword);
	exit(0);
	}
