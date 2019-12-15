#define thisprog "xe-keyupdate1"
#define TITLE_STRING thisprog" v 1.1: JRH, 24.September.2012"
#define MAXLINELEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>database</TAGS>

*/

/* external functions start */
char* xf_strsub1 (char *source, char *str1, char *str2);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],*pcol,*pline;
	int x,y,z;
	long int i,j,k,col;
	FILE *fpin,*fpout;

	/* program-specific variables */
	int found=0;
	char setkeyword[256],setvalue[256],*newword,line2[MAXLINELEN],oldvalue[MAXLINELEN],*pline2;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<4) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Update the value following a key-word in a file\n");
		fprintf(stderr,"If the key-word is not found it will be added\n");
		fprintf(stderr,"Overwrites the original file\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [filename] [key] [value]\n",thisprog);
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	[filename]: file name or \"stdin\"\n");
		fprintf(stderr,"	[key]: the setkeyword to be matched (any word on the line)\n");
		fprintf(stderr,"	[value]: the value to be added or updated\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt RATE 24000\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin PHONE 123-456-7890\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	updated file\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	sprintf(outfile,"temp_%s",thisprog);
	sprintf(setkeyword,"%s",argv[2]);
	sprintf(setvalue,"%s",argv[3]);

	/* READ THE INPUT AND OUPUT MATCHING setkeyword VALUES */
	if(strcmp(infile,"stdin")==0) {
		fpin=stdin;
		fpout=stdout;
		}
	else {
		if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"Error[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}
		if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"Error[%s]: file \"%s\" coule not be written to\n",thisprog,outfile);exit(1);}
	}
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {

		// copy line to line2, omitting any newline before the NULL at the end of the string
		z=strlen(line); for(i=0;i<z;i++) { line2[i]=line[i]; if(line[i]=='\n'||line[i]=='\0') {line2[i]='\0';break;} }

		pline=line;
		pline2=line2;
		x=z=0;
		for(col=1;(pcol=strtok(pline," \t\n"))!=NULL;col++) {
			pline=NULL;
			// if the keyword is found, mark the next word as the one containing the value to be replaced
			if(strcmp(pcol,setkeyword)==0) {found=1; z=col+1;}
			// if the keyword was found and this is the the next word, save the value to be replaced
			if(col==z) { sprintf(oldvalue,"%s",pcol); x=1; break; }
		}
		// if the keyword was found....
		if(z>0) {
			// if an existing value (next word) was found, print the line with that word replaced by setvalue
			if(x==1) fprintf(fpout,"%s\n",xf_strsub1(pline2,oldvalue,setvalue));
			// otherwise, if no value was present (ie. the keyword was the last word on the line) print the line, then setvalue
			if(x==0) fprintf(fpout,"%s %s\n",line2,setvalue);
		}
		else fprintf(fpout,"%s\n",line2);
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	// if the keyword was not found in the file, print a line containing the keyword followed by setvalue
	if(found==0) fprintf(fpout,"%s %s\n",setkeyword,setvalue);

	rename(outfile,infile);

	exit(0);
	}
