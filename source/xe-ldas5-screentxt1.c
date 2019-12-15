#define thisprog "xe-ldas5-screentxt1"
#define TITLE_STRING thisprog" v 1: 29.July.2016 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <time.h>

/*
<TAGS> file screen time</TAGS>

v 1: 29.July.2016 [JRH]
	- original
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],*line=NULL,*templine=NULL,word[256],*pline,*pcol,message[MAXLINELEN];
	long ii,jj,kk,ll,mm,nn,nbad,nchars=0,maxlinelen=0,prevlinelen=0;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin,*fpout;
	size_t sizeofchar=sizeof(char),sizeoflong=sizeof(long);

	/* program-specific variables */
	char *words=NULL;
	int lenwords=0,*count,grp,bin,bintot,setrange=0,colx=1,coly=2;
	long nwords=0,*iword=NULL,*start=NULL,*block=NULL,*blockn=NULL,*list=NULL;

	long *index=NULL,*start1=NULL,*stop1=NULL,nlist;

	/* arguments */
	char *setscreenfile=NULL,*setscreenlist=NULL;
	long setcoltime=1;

	if((line=(char *)realloc(line,6))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((templine=(char *)realloc(templine,MAXLINELEN))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract lines if timestamp falls between start-stop-pairs (SSPs)\n");
		fprintf(stderr,"- NOTE: it is assumed timestamps are long-integers (sample-numbers)\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-scrf: screen-file (binary ssp) defining inclusion bounds []\n");
		fprintf(stderr,"	-scrl: screen-list (CSV) defining inclusion bounds []\n");
		fprintf(stderr,"	-ct: column containing long-integer timestamps [%ld]\n",setcoltime);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -scrf times_immobile.ssp\n",thisprog);
		fprintf(stderr,"	cat data.txt | %s stdin -scrl 0,1000,30000,31000\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	all lines with timestamps falling within one of the SSPs\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}



	/* READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-scrf")==0) setscreenfile=argv[++ii];
			else if(strcmp(argv[ii],"-scrl")==0) setscreenlist=argv[++ii];
			else if(strcmp(argv[ii],"-ct")==0)   setcoltime=atol(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setscreenfile==NULL && setscreenlist==NULL) { fprintf(stderr,"\n--- Error [%s]: must define one of -scrl or -scrf\n\n",thisprog);exit(1);}
	if(setcoltime<1) { fprintf(stderr,"\n--- Error [%s]: time-column (-ct %ld) must be >0\n\n",thisprog,setcoltime);exit(1);}


	/* adjust time column so that it is zero-offset */
	setcoltime--;

	/************************************************************
	READ THE SCREENING LIST OR FILE
	/************************************************************/
	if(setscreenlist!=NULL) {
		index= xf_lineparse2(setscreenlist,",",&nlist);
		if((nlist%2)!=0) {fprintf(stderr,"\n--- Error[%s]: screen-list does not contain pairs of numbers: %s\n\n",thisprog,setscreenlist);exit(1);}
		nlist/=2;
		if((start1=(long *)realloc(start1,nlist*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((stop1=(long *)realloc(stop1,nlist*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<nlist;ii++) {
			start1[ii]=atol(setscreenlist+index[2*ii]);
			stop1[ii]= atol(setscreenlist+index[2*ii+1]);
		}
		if(nlist<1) {fprintf(stderr,"\n--- Error[%s]: screening list is empty\n\n",thisprog);exit(1);}

	}
	else if(setscreenfile!=NULL) {
		nlist = xf_readssp1(setscreenfile,&start1,&stop1,message);
		if(nlist==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		if(nlist<1) {fprintf(stderr,"\n--- Error[%s]: screening file \"%s\" is empty\n\n",thisprog,setscreenfile);exit(1);}
	}
	//TEST: for(jj=0;jj<nlist;jj++) printf("%ld	%ld	%ld\n",jj,start1[jj],stop1[jj]);free(start1);free(stop1);exit(0);


	/************************************************************
	 READ THE DATA TO BE SCREENED
	************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

		/* make a temporary copy of the line before parsing it */
		if(maxlinelen>prevlinelen) { if((templine=(char *)realloc(templine,(maxlinelen+1)*sizeofchar))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}}
		strcpy(templine,line);
		/* create an index to the words on the line */
		iword= xf_lineparse2(line,"\t",&nwords);
		/* generate the timestamp for the line based on the user-defined column containing the timestamps */
		kk=atol(line+iword[setcoltime]);
		/* see if that timestamp falls between any of the start-stop pairs in the screening list */
		for(ii=0;ii<nlist;ii++) if(kk>=start1[ii]&&kk<stop1[ii]) break;
		/* if so, the counter would not have reached nlist: print the copy of the line */
		if(ii<nlist) printf("%s",templine);
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	if(line!=NULL) free(line);
	if(templine!=NULL) free(templine);
	if(iword!=NULL) free(iword);
	if(index==NULL) free(index);
	if(start1==NULL) free(start1);
	if(stop1==NULL) free(stop1);
	exit(0);
}
