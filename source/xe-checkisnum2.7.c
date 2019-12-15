#define thisprog "xe-checkisnum2"
#define TITLE_STRING thisprog" v 7: 6.March.2017 [JRH]"
#define MAXLINELEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
<TAGS>string filter</TAGS>

v 7: 6.March.2017 [JRH]
	- allow for comments & header-line to pass
	- bugfix passing of blank lines
	- enable exceptions for cases where only certain columns must match

v 7: 13.November.2017 [JRH]
	- clarify treatment of "missing values" in instructions
v 7: 18.July.2017 [JRH]
	- update long integer variable names to newer convention (ii,jj,kk)
	- recognize that integer-test will fail for 64-bit numbers
		- new solution looks for any non-numeric values (including decimal) in each scanned word
		- consequently, numbers like 1.0 will not be detected as an integer

v 7: 1.April.2014 [JRH]
	- don't pre-allocate memory for line - unnecessary (let xf_lineread function handle it)
	- bugfix: allocate extra byte of memory for line & templine

v 6: 1.March.2013 [JRH]
	- now uses function _readline1 to read lines of unknown length

v 5: 2.August.2012 [JRH]
	- update instructions to fix errors

v 2.4: JRH, 18 August 2011
	- remove requirements for extra functions xf_strtol, strtod etc.
	- simpler calls to isfinite and sscanf
*/


/* external functions start */
char *xf_lineread1(char *line,long *maxlinelen,FILE *fpin);
/* external functions end */


int main (int argc, char *argv[]) {

	char *line=NULL,*templine=NULL,*pline,*pcol;
	long int ii,jj,kk,nn,maxlinelen=0;
	int v,w,x,y,z,n,col,colmatch,badcols;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	long double aaa,bbb,ccc;
	long int result_l[1]; float result_f[1]; double result_d[1]; long double result_ld[1];
	FILE *fpin,*fpout;

	/* program-specific variables */
	char message[256],dec;
	long prevlen=maxlinelen;

	/* arguments */
	char infile[256],missingval[256],fieldlist[256];
	int incol[256], ncols;
	int numtype=2 ; // options are integer (0) float (1)
	int setblank=0,setcomment=0,sethead=0;
	sprintf(missingval,"-");
	ncols=1, incol[0]=0;

	if(argc<=1) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Output lines from a file only if specified fields are numbers\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: filename or \"stdin\"):\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS, defaults in []: \n");
		fprintf(stderr,"	-t: valid number-types [%d]\n",numtype);
		fprintf(stderr,"		0= integer (no decimals accepted - eg. 1.0 fails)\n");
		fprintf(stderr,"		1= float or integer\n");
		fprintf(stderr,"		2= any, including INF or NAN)\n");
		fprintf(stderr,"	-f: fields to check, comma-separated [all]\n");
		fprintf(stderr,"...exceptions... \n");
		fprintf(stderr,"	-b: output blank lines? 0=no, 1=yes, [%d]\n",setblank);
		fprintf(stderr,"	-h: allow header line? (0=NO 1=YES) [%d]\n",sethead);
		fprintf(stderr,"	-c: allow \"#\"comments) (0=NO 1=YES) [%d]\n",setcomment);
		fprintf(stderr,"	-m: non-numeric missing-value placeholder to accept [%s]\n",missingval);
		fprintf(stderr,"		-NOTE: this will be output as if it were numeric\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLE: output lines if columns 1 3 and 5 contain integers:\n");
		fprintf(stderr,"	%s datafile.txt -t 0 -f 1,3,5 \n",thisprog);
		fprintf(stderr,"	cat datafile.txt | %s datafile.txt -t 0 -f 1,3,5 \n",thisprog);
		fprintf(stderr,"EXAMPLE: output lines if all columns contain any number:\n");
		fprintf(stderr,"	%s datafile.txt -t 2 \n",thisprog);
		fprintf(stderr,"\n");
		fprintf(stderr,"CAUTION: strings like 25abc will be interpreted as a number (25) \n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-t")==0) numtype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-b")==0) setblank=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-h")==0) sethead=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-c")==0) setcomment=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-m")==0) strcpy(missingval,argv[++ii]);
			else if(strcmp(argv[ii],"-f")==0) {
				strcpy(fieldlist,argv[++ii]);
				pline=fieldlist;
				ncols=0;
				for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {pline=NULL; incol[ncols++]=atoi(pcol);}
			}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(numtype<0||numtype>2) {fprintf(stderr,"\n--- Error[%s]: \"%d\" is an invalid value for \"type\" - should be 0,1 or 2\n\n",thisprog,numtype); exit(1);}
	if(setblank!=0&&setblank!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -b (%d) - must be 0 or 1\n\n",thisprog,setblank); exit(1);}
	if(sethead!=0&&sethead!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -b (%d) - must be 0 or 1\n\n",thisprog,sethead); exit(1);}
	if(setcomment!=0&&setcomment!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -b (%d) - must be 0 or 1\n\n",thisprog,setcomment); exit(1);}

	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	/* if all columns must be matched... */
	if(ncols==1 && incol[0]==0)	{
		nn=0;
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1) {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			if(maxlinelen>prevlen) {
				templine=(char *)realloc(templine,(maxlinelen+1)*sizeofchar);
				if(templine==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n",thisprog);exit(1);}
			}

			/* exceptions */
			if(++nn==1 && sethead==1) { printf("%s",line); continue; }
			if(line[0]=='#' && setcomment==1) { printf("%s",line); continue; }
			if(line[0]=='\n' && setblank==1) { printf("%s",line); continue; }

			strcpy(templine,line); pline=line; badcols=0;
			for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
				pline=NULL;
				if(strcmp(pcol,missingval)==0) continue;
				if(numtype==0 && (sscanf(pcol,"%ld%c",&ii,&dec)!=1)) badcols++;
				if(numtype==1 && (sscanf(pcol,"%lf",&aa)!=1 || !isfinite(aa)) ) badcols++;;
				if(numtype==2 && (sscanf(pcol,"%lf",&aa)!=1)) badcols++;;
			}
			if(badcols==0 && col>1) printf("%s",templine);
			prevlen=maxlinelen;
		}
	}
	/* otherwise, if only certain columns need to match... */
	else	{
		nn=0;
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1) {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			if(maxlinelen>prevlen) {
				templine=(char *)realloc(templine,(maxlinelen+1)*sizeofchar);
				if(templine==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n",thisprog);exit(1);}
			}

			/* exceptions */
			if(++nn==1 && sethead==1) { printf("%s",line); continue; }
			if(line[0]=='#' && setcomment==1) { printf("%s",line); continue; }
			if(line[0]=='\n' && setblank==1) { printf("%s",line); continue; }

			strcpy(templine,line);
			pline=line; colmatch=ncols; // number of columns to match
			for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
				pline=NULL;
				if(numtype==0) for(jj=0;jj<ncols;jj++) if(col==incol[jj]) if(strcmp(pcol,missingval)==0 || sscanf(pcol,"%ld%c",&ii,&dec)!=1) colmatch--;
				if(numtype==1) for(jj=0;jj<ncols;jj++) if(col==incol[jj]) if(strcmp(pcol,missingval)==0 || (sscanf(pcol,"%lf",&aa)==1 && isfinite(aa))) colmatch--;
				if(numtype==2) for(jj=0;jj<ncols;jj++) if(col==incol[jj]) if(strcmp(pcol,missingval)==0 || sscanf(pcol,"%lf",&aa)==1) colmatch--;
			}
			if(colmatch==0) printf("%s",templine);
			prevlen=maxlinelen;
		}
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(line!=NULL) free(line);
	if(templine!=NULL) free(templine);

	exit(0);
}
