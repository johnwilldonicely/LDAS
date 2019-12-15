#define thisprog "xe-transpose3"
#define TITLE_STRING thisprog" v 2: 9.July.2012 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>transform</TAGS>

v 2: 9.July.2012 [JRH]
	- bugfix - previously failed to recognize non-numerical data (eg nan or inf or "-") - resulting in misattribution of group labels
*/


/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],word[256],*matchstring=NULL,*pline,*pcol;
	long int i,j,k,n;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	int *label=NULL,prevx;
	long ntemp=0,ntot=0,group=0,ngroups=0,nelements=-1;
	double *tempdata=NULL,*newdata=NULL;
	/* arguments */
	int setformat=1,setbintot=25,colxdat=1;
	float setlow=0.0,sethigh=0.0,setbinwidth=0.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"- Actually de-transpose a column of data into multiple columns\n");
		fprintf(stderr,"- For 2-column input <group><datum>, output a column for each <group>\n");
		fprintf(stderr,"- Assumes data from a given group appears on successive rows (not mixed)\n");
		fprintf(stderr,"- Max input line length = %d\n",MAXLINELEN);
		fprintf(stderr,"NOTE: group numbers are converted to integers\n");
		fprintf(stderr,"Number of elements in first group is used as the default number\n");
		fprintf(stderr,"	- if subsequent groups have more elements they will be ignored\n");
		fprintf(stderr,"	- if subsequent groups have fewer elements the column is paded\n");
		fprintf(stderr,"Number of elements in first group is used as the default number\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\" in format <group><datum>\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt \n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	1st row: integer group numbers as defined in column-1 of input\n");
		fprintf(stderr,"	subsequent rows: data from each group in a separate column\n");
		fprintf(stderr,"	...so...\n");
		fprintf(stderr,"		100\t1\n");
		fprintf(stderr,"		100\t2\n");
		fprintf(stderr,"		100\t3\n");
		fprintf(stderr,"		200\t4\n");
		fprintf(stderr,"		200\t5\n");
		fprintf(stderr,"		200.5\t6\n");
		fprintf(stderr,"		300\t7\n");
		fprintf(stderr,"		300\t8\n");
		fprintf(stderr,"	...becomes...\n");
		fprintf(stderr,"		100	200	300	\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"		1	4	7\n");
		fprintf(stderr,"		2	5	8\n");
		fprintf(stderr,"		3	6	nan\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-t")==0) 	{ setformat=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}


	/* ALTERNATIVE METHOD - SPECIFIC COLUMNS HOLD xdat  */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	/* initialize variables */
	ntot=0;	ntemp=0; ngroups=0; nelements=-1; prevx=0; // prevx setting is just to avoid compile warnings
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(line[0]=='#') continue;
		pline=line; colmatch=2; // number of columns to match
		for(col=1;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
			pline=NULL;
			if(col==1 && sscanf(pcol,"%d",&x)==1) colmatch--; // store value - check if input was actually a number
			if(col==2) {
				colmatch--;
				if(sscanf(pcol,"%lf",&aa)!=1) aa=NAN;
			}
		}
		if(colmatch!=0) continue;

 		if(ntot>0 && x!=prevx) {
 			if(ngroups==0) nelements=ntemp; /* number of elements in first groups becomes the template for all others */
			group=ngroups;
			ngroups++;
 			if((newdata=(double *)realloc(newdata,ngroups*nelements*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
 			if((label=(int *)realloc(label,ngroups*nelements*sizeofint))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			label[group]=prevx;
 			if(nelements<=ntemp) {
				for(i=0;i<nelements;i++) newdata[group*nelements+i]=tempdata[i];
			}
			else {
				for(i=0;i<ntemp;i++) newdata[group*nelements+i]=tempdata[i];
				for(i=ntemp;i<nelements;i++) newdata[group*nelements+i]=NAN;
			}
 			ntemp=0;
 		}

		/* BUILD A TEMPORARY ARRAY FOR THE DATA FROM THE CURRENT GROUP */
		if((tempdata=(double *)realloc(tempdata,(ntemp+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		tempdata[ntemp]=aa;
		prevx=x;
		ntemp++;
		ntot++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* STORE THE DATA FROM THE LAST GROUP, WHICH WILL NOT HAVE BEEN DETECTED BY A CHANGE IN GROUP NUMBER */
	if(ntemp>0) {
		if(ngroups==0) nelements=ntemp;
		group=ngroups;
		ngroups++;
		if((newdata=(double *)realloc(newdata,ngroups*nelements*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((label=(int *)realloc(label,ngroups*nelements*sizeofint))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		label[group]=prevx;
 		if(nelements<=ntemp) {
			for(i=0;i<nelements;i++) newdata[group*nelements+i]=tempdata[i];
		}
		else {
			for(i=0;i<ntemp;i++) newdata[group*nelements+i]=tempdata[i];
			for(i=ntemp;i<nelements;i++) newdata[group*nelements+i]=NAN;
		}
	}

	for(i=0;i<ngroups;i++) printf("%d\t",label[i]); printf("\n\n");
	for(i=0;i<nelements;i++) {
 		printf("%g",newdata[0*nelements+i]); for(j=1;j<ngroups;j++) printf("\t%g",newdata[j*nelements+i]); printf("\n");
 	}

	free(tempdata);
	free(newdata);
	free(label);
	exit(0);
	}
