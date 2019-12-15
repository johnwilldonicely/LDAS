#define thisprog "xe-delimit"
#define TITLE_STRING thisprog" v 1: 23.October.2018 [JRH]"
#define MAXLINELEN 10000
#define MAXWORDLEN 256
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>string</TAGS>

v 1: 23.October.2018 [JRH]
	- update style and variable names, add TAGS section
v 1: 14.March.2016 [JRH]
	- increased MAXLINELEN to 10,000 to better deal with large tables - still not guaranteed though!
v 1: 28.April.2010 [JRH]
*/

/* external functions start */
/* external functions end */

int main(int argc, char *argv[]) {

	char line[MAXLINELEN],*pline,*pcol;
	char old_delimiters[10],new_delimiter[5];
	int x,y,z;
	long ii,jj,kk,col;
	FILE *fpin,*fpout;
	/* arguments */
	char *infile=NULL,outfile[MAXWORDLEN]="stdout\0",name[MAXWORDLEN]="tab\0";
	int dcomment=1;

	strcpy(old_delimiters," ,\t\n\r");
	strcpy(new_delimiter,"0");

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2){
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
	 	fprintf(stderr,"Delimits columns of data in a text file with a delimiter of your choice\n");
		fprintf(stderr,"- Replaces multiple spaces tabs and commas\n");
		fprintf(stderr,"- Eliminates leading white-spaces\n");
		fprintf(stderr,"- maximum line length= %ld characters\n",(long)MAXLINELEN);
		fprintf(stderr,"USAGE: %s [infile] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-d: delimiter- tab,space,comma,colon,semicolon, or newline [%s]\n",name);
		fprintf(stderr,"	-c: delimit comment lines (#) as well? 0=NO, 1=YES [%d]\n",dcomment);
		fprintf(stderr,"	-out: name of output file [stdout = output to screen] [%s]\n",outfile);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s a.txt -d tab -out temp.txt \n",thisprog);
		fprintf(stderr,"	cat a.txt | %s stdin -d comma\n",thisprog);
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-d")==0)   strncpy(name,argv[++ii],MAXWORDLEN);
			else if(strcmp(argv[ii],"-c")==0)   dcomment= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0) strncpy(outfile,argv[++ii],MAXWORDLEN);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	/* create new delimiter based on specified name */
	if(strcmp(name,"space")==0) strcpy(new_delimiter," ");
	if(strcmp(name,"comma")==0) strcpy(new_delimiter,",");
	if(strcmp(name,"tab")==0)   strcpy(new_delimiter,"\t");
	if(strcmp(name,"colon")==0) strcpy(new_delimiter,":");
	if(strcmp(name,"semicolon")==0) strcpy(new_delimiter,";");
	if(strcmp(name,"newline")==0) strcpy(new_delimiter,"\n");
	if(strcmp(new_delimiter,"0")==0) {fprintf(stderr,"\n--- Error[%s]: invalid delimiter: \"%s\"\n\n",thisprog,name); exit(1);}

	/******************************************************************************
	OPEN INPUT AND OUTPUT
	******************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin= stdin;
	else fpin= fopen(infile,"r");
	if(fpin==0) {fprintf(stderr,"Error[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}

	if(strcmp(outfile,"stdout")==0) fpout= stdout;
	else fpout= fopen(outfile,"w");
	if(fpout==0) {fprintf(stderr,"Error[%s]: unable to open file \"%s\" for writing\n",thisprog,outfile);exit(1);}

	/******************************************************************************
	START DELIMITING
	******************************************************************************/
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(dcomment==0 && line[0]=='#') {fprintf(fpout,"%s",line);continue;}
		pline= line;
		/* print first column with no preceding delimiter */
		pcol= strtok(pline,old_delimiters);
		if(pcol!=NULL) {
			fprintf(fpout,"%s",pcol);
			pline= NULL;
		}
		else {
			fprintf(fpout,"\n");
			continue;
		}
		/* print remaining columns with preceding delimiter */
		for(col=2;(pcol=strtok(pline,old_delimiters))!=NULL;col++) {
			pline= NULL;
			fprintf(fpout,"%s%s",new_delimiter,pcol);
		}
		fprintf(fpout,"\n");
	}

	/******************************************************************************
	CLEANUP AND EXIT
	******************************************************************************/
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(strcmp(outfile,"stdout")!=0) fclose(fpout);
	exit(0);
}
