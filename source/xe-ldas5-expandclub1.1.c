#define thisprog "xe-ldas5-expandclub1"
#define TITLE_STRING thisprog" v 1: 1.May.2016 [JRH]"
#define MAXLINELEN 1000
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>  /* this and next header allow testing file exists using stat() function */
#include <errno.h>

/* <TAGS>LDAS dt.spikes</TAGS> */

/************************************************************************
v 1: 19.July.2016 [JRH]
	- change argument names to match convention (-scrf and -scrl)
v 1: 1.May.2016 [JRH]
	- added boundary test to blockalign function
v 1: 15.March.2016 [JRH]
	- bugfix initial threshold in xf_blockrealign2
v 1: 26.February.2016 [JRH]
*************************************************************************/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
off_t xf_readbin2_v(FILE *fpin, void **data, off_t startbyte, off_t bytestoread, char *message);
long xf_readclub1(char *infile1, char *infile2, long **clubt, short **club, char *message);
int xf_blockrealign2(long *data1, long nn, long *bstart, long *bsize, long nblocks, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile1[256],infile2[256],outfile1[256],outfile2[256],message[256];
	int v,w,x,y,z,sizeofshort=sizeof(short),sizeoflong=sizeof(long);
	long int ii,jj,kk,mm,nn;
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[64];
	FILE *fpin,*fpout1,*fpout2;
	struct stat sts;

	/* program-specific variables */
	void *buffer1=NULL,*buffer2=NULL;
	short *club=NULL,*clukeep;
	long *clubt=NULL,*start1=NULL,*stop1=NULL,*index1=NULL,*index2=NULL;
	int datasize,blocksread;
	long headerbytes=0,maxread,blocksize;
	off_t params[4]={0,0,0,0},block,nread,nreadtot,nout,nlist,nclu;

	/* arguments */
	char *setscrfile=NULL,*setscrlist=NULL,*setclulist=NULL;
	int setout=-1,setverb=0,setscreen=0;
	off_t setheaderbytes=0;

	sprintf(outfile1,"temp_%s.clubt",thisprog);
	sprintf(outfile2,"temp_%s.club",thisprog);

	/************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	************************************************************/
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Restore .club(t) files with sections removed using an SSP file\n");
		fprintf(stderr,"- output is either converted to ASCII or kept in binary form\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [clubt] [club] [options]\n",thisprog);
		fprintf(stderr,"	[clubt]: binary file containing cluster-times (long int)\n");
		fprintf(stderr,"	[club]: binary file containing cluster-IDs (short int)\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-scrf: binary ssp-file defining bounds for kept data [unset]\n");
		fprintf(stderr,"	-scrl: list (CSV) defining bounds for kept data\n");
		fprintf(stderr,"	-out: output format [%d]:\n",setout);
		fprintf(stderr,"		-1= ASCII, one timestamp-id pair per line\n");
		fprintf(stderr,"		 0= binary files x2 (long,short)\n");
		fprintf(stderr,"		 	%s\n",outfile1);
		fprintf(stderr,"		 	%s\n",outfile2);
		fprintf(stderr,"	-verb: set verbocity of output (0=low, 1=high) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.clubt data.club -s 1 -sl 100,200,1500,1600\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	************************************************************/
	sprintf(infile1,"%s",argv[1]);
	sprintf(infile2,"%s",argv[2]);
	if(strcmp(infile1,"stdin")!=0 && stat(infile1,&sts)==-1 && errno == ENOENT) {
		fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile1);
		exit(1);
	}
	if(strcmp(infile2,"stdin")!=0 && stat(infile2,&sts)==-1 && errno == ENOENT) {
		fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile2);
		exit(1);
	}
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-scrf")==0) setscrfile=argv[++ii];
			else if(strcmp(argv[ii],"-scrl")==0) setscrlist=argv[++ii];
			else if(strcmp(argv[ii],"-out")==0)  setout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setout!=-1 &&setout!=0) {fprintf(stderr,"\n--- Error[%s]: invalid -out (%d) : must be -1 or 0\n\n",thisprog,setout);exit(1);}
	if(setverb<0 || setverb>1) {fprintf(stderr,"\n--- Error[%s]: invalid -verb (%d) : must be 0-1\n\n",thisprog,setverb);exit(1);}
	if(setscrfile==NULL && setscrlist==NULL) {fprintf(stderr,"\n--- Error[%s]: must define an expansion list (-scrl) or file (-scrf)\n\n",thisprog);exit(1);}
	if(setscrfile!=NULL && setscrlist!=NULL) {fprintf(stderr,"\n--- Error[%s]: cannot define both an expansion list (-scrl %s) and file (-scrf %s)\n\n",thisprog,setscrlist,setscrfile);exit(1);}

	/************************************************************
	READ THE INCLUDE OR EXCLUDE LIST
	/************************************************************/
	if(setscrlist!=NULL) {
		index1= xf_lineparse2(setscrlist,",",&nlist);
		if((nlist%2)!=0) {fprintf(stderr,"\n--- Error[%s]: screen-list does not contain pairs of numbers: %s\n\n",thisprog,setscrlist);exit(1);}
		nlist/=2;
		if((start1=(long *)realloc(start1,nlist*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((stop1=(long *)realloc(stop1,nlist*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<nlist;ii++) {
			start1[ii]=atol(setscrlist+index1[2*ii]);
			stop1[ii]= atol(setscrlist+index1[2*ii+1]);
	}}
	else if(setscrfile!=NULL) {
		nlist = xf_readssp1(setscrfile,&start1,&stop1,message);
		if(nlist==-1) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	//for(jj=0;jj<nlist;jj++) printf("%ld	%ld	%ld\n",jj,start1[jj],stop1[jj]);free(start1);free(stop1);exit(0);

	/************************************************************
	READ THE CLUSTER TIMESTAMPS AND IDs
	***********************************************************/
 	nn= xf_readclub1(infile1,infile2,&clubt,&club,message);
 	if(nn<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}


	/************************************************************
	APPLY THE CORRECTION TO THE SAMPLE NUMBERS USING THE SSPs
	***********************************************************/
 	x= xf_blockrealign2(clubt,nn,start1,stop1,nlist,message);
 	if(x!=0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

	/************************************************************
	OUTPUT THE DATA, SCREENING IF REQUIRED
	***********************************************************/
	if(setout==-1) {
		for(ii=0;ii<nn;ii++) printf("%ld\t%hd\n",clubt[ii],club[ii]);
	}
	if(setout==0) {
		if(setverb==1) {
			fprintf(stderr,"writing file %s\n",outfile1);
			fprintf(stderr,"writing file %s\n",outfile2);
		}
		if((fpout1=fopen(outfile1,"wb"))==0) {fprintf(stderr,"\n--- Error[%s]: could not open file %s for writing\n\n",thisprog,outfile1);exit(1);}
		if((fpout2=fopen(outfile2,"wb"))==0) {fprintf(stderr,"\n--- Error[%s]: could not open file %s for writing\n\n",thisprog,outfile2);exit(1);}
		for(ii=0;ii<nn;ii++) { fwrite(clubt+ii,sizeoflong,1,fpout1);fwrite(club+ii,sizeofshort,1,fpout2);}

		fclose(fpout1);
		fclose(fpout2);
	}

 	if(club!=NULL) free(club);
 	if(clubt!=NULL) free(clubt);
 	if(index1!=NULL) free(index1);
 	if(index2!=NULL) free(index2);
 	if(start1!=NULL) free(start1);
 	if(stop1!=NULL) free(stop1);
 	exit(0);

}
