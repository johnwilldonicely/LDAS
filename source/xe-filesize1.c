#define thisprog "xe-filesize1"
#define TITLE_STRING thisprog" v 1: 24.March.2019 [JRH]"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <yaml.h>

/*
<TAGS> file time </TAGS>
v 1: 24.March.2019 [JRH]
*/


/* external functions start */
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL,message[1000];
	long int ii,jj,kk,ll,mm,nn;
	int v,w,x,y,z;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin,*fpout;
	size_t iii,jjj,kkk,lll,nnn,mmm;
	/* program-specific variables */
	off_t datasize=0,filebytes=0,recordbytes=0,nrecords=0;
	off_t ndays=0,nhours=0,nminutes=0,nmicro=0;
	double nsectot=0.0,nseconds=0.0;
	/* arguments */
	char *infile=NULL;
	int setverb=0,setdatatype=1;
	long sethead=0,setnch=1;
	double setsf=1.0;


	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Determine binary file size and/or record-count, duration etc\n");
		fprintf(stderr,"USAGE: %s [infile] [options]\n",thisprog);
		fprintf(stderr,"	[input]: binary file name\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-sf: sample frequency (Hz) [%g]\n",setsf);
		fprintf(stderr,"	-dt: data type [%d]\n",setdatatype);
		fprintf(stderr,"		0: unsigned char\n");
		fprintf(stderr,"		1: signed char\n");
		fprintf(stderr,"		2: unsigned short\n");
		fprintf(stderr,"		3: signed short\n");
		fprintf(stderr,"		4: unsigned int\n");
		fprintf(stderr,"		5: signed int\n");
		fprintf(stderr,"		6: unsigned long\n");
		fprintf(stderr,"		7: signed long\n");
		fprintf(stderr,"		8: float\n");
		fprintf(stderr,"		9: double\n");
		fprintf(stderr,"	-head: header-bytes to be excluded from byte-count [%ld]\n",sethead);
		fprintf(stderr,"	-nch: number of interlaced channels [%ld]\n",setnch);
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -t 1\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	1st column: lower limit of each bin\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-dt")==0)   setdatatype= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-sf")==0)   setsf= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-head")==0) sethead= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-nch")==0)  setnch= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb != 999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0,1, or 999\n\n",thisprog,setverb);exit(1);}
	if(setnch<1) { fprintf(stderr,"\n--- Error [%s]: invalid -nch [%ld] - must be >=1\n\n",thisprog,setnch);exit(1);}
	if(setsf<0.0) { fprintf(stderr,"\n--- Error [%s]: invalid -sf [%g] - must be >=0\n\n",thisprog,setsf);exit(1);}

	/************************************************************/
	/* DEFINE THE KEY PARAMETERS */
	/************************************************************/
	if(setdatatype==0||setdatatype==1) datasize=(off_t)sizeof(char);
	else if(setdatatype==2||setdatatype==3) datasize=(off_t)sizeof(short);
	else if(setdatatype==4||setdatatype==5) datasize=(off_t)sizeof(int);
	else if(setdatatype==6||setdatatype==7) datasize=(off_t)sizeof(long);
	else if(setdatatype==8) datasize=(off_t)sizeof(float);
	else if(setdatatype==9) datasize=(off_t)sizeof(double);
	else {fprintf(stderr,"\n--- Error[%s]: data type (-t %d) must be 0-9\n\n",thisprog,setdatatype); exit(1);}


	/********************************************************************************
	OPEN THE FILE & DETERMINE SIZE IN BYTES
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) {fprintf(stderr,"\n--- Error[%s]: this program does not accept stdin as input\n\n",thisprog,infile);exit(1);}
	else {
		fpin= fopen(infile,"rb");
		if(fpin==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	}
	fseek(fpin, 0L, SEEK_END);
	filebytes= ftello(fpin);
	fclose(fpin);

	/********************************************************************************
	CALCULATE OTHER VALUES
	********************************************************************************/
	recordbytes= datasize*setnch; /* bytes per multi-channel record */
	iii= filebytes-sethead; /* bytes-free for storing records after accounting for header */
	if(iii%recordbytes!=0) {fprintf(stderr,"\n--- Error[%s]: corrupt file or incorrect dt/head/nch combination\n\n",thisprog);exit(1);}
	nrecords= iii/recordbytes; /* number of records */
	nsectot= aa= (double)nrecords/setsf; /* total number of seconds */
	nmicro= aa*1000000; /* microseconds, as off_t - so all decimal maths for the next few steps */

	/* calculate days/hours/minutes/seconds */
	/* - subtracting the integer-value of each from nmicro at each step */
	/* - in this way, the remainder is used to calculate the next-smaller component */
	ndays=    nmicro/86400000000; nmicro-= ndays*86400000000;
	nhours=   nmicro/3600000000;  nmicro-= nhours*3600000000;
	nminutes= nmicro/60000000;    nmicro-= nminutes*60000000;
	nseconds= (double)nmicro/1000000.0;


	printf("bytes= %ld\n",filebytes);
	printf("Kbytes= %.3f\n",((double)filebytes/1000.0));
	printf("Mbytes= %.3f\n",((double)filebytes/1000000.0));
	printf("Gbytes= %.3f\n",((double)filebytes/1000000000.0));
	printf("datasize= %ld\n",datasize);
	printf("nrecords= %ld\n",nrecords);
	printf("nseconds= %.6f\n",nsectot);
	printf("d:h:m:s= %ld:%02ld:%02ld:%g\n",ndays,nhours,nminutes,nseconds);

goto END;

/********************************************************************************/
/* CLEANUP AND EXIT */
/********************************************************************************/
END:
	if(line!=NULL) free(line);
	exit(0);
}
