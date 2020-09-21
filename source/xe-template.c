#define thisprog "xe-template"
#define TITLE_STRING thisprog" v 1: 14.April.2014 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <time.h>
// #include <yaml.h>

/*
<TAGS>programming LDAS</TAGS>

v 1: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/

/* external functions start */
void xf_err1(char *setfunc, char *setmsg, int space);
long *xf_getkeycol(char *line1, char *d1, char *keys1, char *d2, long *nkeys1, char *message);
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_stats2_d(double *data, long n, int varcalc, double *result_d);
float *xf_morletwavelet1_f(double f, double Fs, int width, size_t *nwavelet);
complex float *xf_morletwavelet2_f(double f, double Fs, int width, size_t *nwavelet);
float *xf_readbin2_f(char *infile, off_t *parameters, char *message);
int xf_readbin1_f(FILE *fpin,off_t *parameters,float *data1,char *message);
int xf_rollbuffer1_f(float *data, size_t nbuff, size_t offset, int direction, char *message);
long *xf_parselist1_l(char *line, char *delimiters, long min, long max, long *nitems, char *message);
int xf_compare1_d(const void *a, const void *b);
char *xf_strcat1(char *string1,char *string2,char *delimiter);
int xf_strkey1(char *line,char *key, int word, char *output);
int xf_filter_mingood2_s(short *data0, size_t nn, size_t nchan, size_t mingood, short setbad, char *message);
int xf_bin1b_d(double *data, long *setn, long *setz, double setbinsize, char *message);
int xf_blockrealign2(long *samplenum, long nn, long *bstart, long *bstop, long nblocks, char *message);
double *xf_jitter1_d(double *yval, long nn, double centre, double limit, char *message);
double xf_rand1_d(double setmax);
/* external functions end */


int main (int argc, char *argv[]) {

	/* line-reading and word/column-parsing */
	char *line=NULL,*templine=NULL,*pline=NULL,*pword=NULL;
	long *keycols=NULL,nkeys=0,*iword=NULL,nlines=0,nwords=0,maxlinelen=0,prevlinelen=0;

	/*  common-use variables */
	char message[MAXLINELEN];
	long int ii,jj,kk,ll,mm,nn;
	int v,w,x,y,z,col,colmatch;
	int vector[] = {1,2,3,4,5,6,7};
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin,*fpout;
	size_t iii,jjj,kkk,lll,nnn,mmm;
	size_t sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	size_t sizeofx,sizeofy;
	size_t sizeoftempline=sizeof(*templine);

	/* program-specific variables */
	int *count,grp,bin,bintot,setrange=0,colx=1,coly=2;
	long *start=NULL,*start1=NULL,*stop1=NULL,*list=NULL;
	long *matrix=NULL;
	off_t sizeofdata,datasize,startbyte,ntoread,nread,bytestoread,parameters[8];
	float *data1=NULL;
	float *xdatf=NULL,*ydatf=NULL;
	double *xdat=NULL,*ydat=NULL;
	char timestring[32];
	time_t t1;
	struct tm *tstruct1;

	/* arguments */
	char *infile=NULL,*outfile=NULL,*setkeys=NULL;
	int setformat=1,setbintot=25,coldata=1,setverb=0,sethead=0;
	long setcolx=1,setcoly=2;
	float setlow=0.0,sethigh=0.0,setbinwidth=0.0;

	if((line=(char *)realloc(line,6))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((templine=(char *)realloc(templine,MAXLINELEN))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	// JITTER test
	// double *xval=NULL, yval[100];
	// nn=100;
	// for(ii=0;ii<nn;ii++) yval[ii]=(double)ii;
	// xval= xf_jitter1_d(yval,nn,5,0.25,message);
	// if(xval==NULL) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	// for(ii=0;ii<nn;ii++) printf("%g\t%g\n",xval[ii],yval[ii]);
	// exit(0);

	// FILE *fh = fopen("~/temp.yaml", "r");
	// yaml_parser_t parser;
	// /* Initialize parser */
 	// if(!yaml_parser_initialize(&parser)) fputs("Failed to initialize parser!\n", stderr);
	// if(fh == NULL) fputs("Failed to open file!\n", stderr);
	// /* Set input file */
	// yaml_parser_set_input_file(&parser, fh);
	// /* Cleanup */
	// yaml_parser_delete(&parser);
	// fclose(fh);
	// exit(0);

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Template program source-code\n");
		fprintf(stderr,"Assumes one valid numeric value per input line\n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"USAGE: %s [infile] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-t(ype): 1(counts) 2(range 0-1) or 3(probability)\n");
		fprintf(stderr,"	-list: comma-separated list of numbers\n");
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -t 1\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 3\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	1st column: lower limit of each bin\n");
		fprintf(stderr,"	2nd column: value (eg. counts) in that bin\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		/* test error-display and exit */
		xf_err1(thisprog,"this is the error message",1);
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-cx")==0)   setcolx=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-cy")==0)   setcoly=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-t")==0)    setformat=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-keys")==0) setkeys=argv[++ii];
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb != 999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0,1, or 999\n\n",thisprog,setverb);exit(1);}
	setcolx--;
	setcoly--;



/********************************************************************************/
/* STORING DATA  */
/********************************************************************************/

	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	sizeofdata= sizeof(*data1);
	nlines=mm=nn=0; // nlines= total lines read (for reporting), mm= nonblank/noncomment lines, nn= data stored

	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		// increment line-counter, for reporting
		nlines++;
		// preserve leading comments and blank lines if required
		if(sethead==1) { if(line[0]=='#'||strlen(line)<=1) { printf("%s",line); continue;}}
		// increment non-comment/blank line counter, to detect column-header line
		mm++;

		/* OPTIONAL: MAKE A TEMPORARY COPY OF THE LINE BEFORE PARSING IT */
		if(maxlinelen>prevlinelen) {
			prevlinelen= maxlinelen;
			templine= realloc(templine,(maxlinelen+1)*sizeoftempline);
			if(templine==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		}
		strcpy(templine,line);

		/* ASSIGN KEYCOL NUMBERS: THIS IS FOR KEYWORD-SELECTION OF COLUMNS */
		if(mm==1) {
			keycols= xf_getkeycol(line,"\t",setkeys,",",&nkeys,message);
			if(keycols==NULL) { fprintf(stderr,"\b\n\t%s/%s\n\n",thisprog,message); exit(1); }
			if(setverb==999) for(ii=0;ii<nkeys;ii++) printf("%ld: keycols=%ld setkeys=%s\n",ii,keycols[ii],setkeys);
			//TEST: printf("LINE: %s SETKEYS: %s\n",line,setkeys);
			continue;
		}

		/* PARSE NON-HEADER LINES (TWO OPTIONS) */
		// iword= xf_lineparse1(line,&nwords); // whitespace delimited, multiple delimiters treated as one
		iword= xf_lineparse2(line,"\t",&nwords); // user-defined delimited
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		/* make sure required columns are present */
		if(nwords<nkeys) continue;

		/* STORE A DATUM FROM THE COLUMN DEFINED BY THE FIRST KEY (OR OTHER ANY OTHER USeR VARIABLE IF YOU PREFER) */
		/* this example requires the column contents to nbe numeric */
		/* make sure content in x- and y-columns is numeric */
		if(sscanf(line+iword[keycols[0]],"%f",&a)!=1 || !isfinite(a)) continue;
		/* dynamically allocate memory */
		data1= realloc(data1,(nn+1)*sizeofdata);  // if not using preallocated size, use sizeof(*data1)
		if(data1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		/* store the value */
		data1[nn]= a;

		/* INCREMENT THE STORED-DATA */
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	//TEST
	printf("nn=%ld\n",nn);
	for(ii=0;ii<nn;ii++) printf("data1[%ld]= %g\n",ii,data1[ii]);
	goto END;


	/* STORE DATA METHOD 1a - newline-delimited single float */
	/* in this example, interpolate bad points to preserve line-count */
//  	fprintf(stderr,"START ASCII READ\n");
// 	if(strcmp(infile,"stdin")==0) fpin=stdin;
// 	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
// 	n=nbad=0;
// 	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
// 		if(sscanf(line,"%f",&a)!=1 || !isfinite(a)) { a=NAN; nbad++;}
// 		if((data1=(float *)realloc(data1,(n+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
// 		data1[n++]=a;
// 	}
// 	if(strcmp(infile,"stdin")!=0) fclose(fpin);
// 	/* interpolate if some data is bad */
// 	if(nbad>0) i= xf_interp3_f(data1,n);
// 	if(i<0) {fprintf(stderr,"\n--- Error[%s]: input \"%s\" contains no finite numbers\n\n",thisprog,infile);exit(1);};
//  	fprintf(stderr,"STOP\n");
// 	exit(0);




	// /* STORE DATA METHOD 1b - newline-delimited single double */
	// if(strcmp(infile,"stdin")==0) fpin=stdin;
	// else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	// nn=0;
	// while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		// if(sscanf(line,"%lf",&aa)!=1) continue;
		// if(isfinite(aa)) {
			// if((xdat=(double *)realloc(xdat,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			// xdat[nn]=aa;
			// nn++;
	// }}
	// if(strcmp(infile,"stdin")!=0) fclose(fpin);

	// jj=0;
	// z= xf_bin1b_d(xdat,&nn,&jj,5,message);
	// if(jj!=0) {fprintf(stderr,"*** %s\n",message); exit(1);}

	// for(ii=0;ii<nn;ii++) printf("%g\n",xdat[ii]);

	// exit(0);



	/* STORE DATA METHOD 1b - newline-delimited pairs of data */
// 	if(strcmp(infile,"stdin")==0) fpin=stdin;
// 	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
// 	n=0;
// 	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
// 		if(sscanf(line,"%lf %lf",&aa,&bb)!=2) continue;
// 		if(isfinite(aa) && isfinite(bb)) {
// 			if((xdat=(double *)realloc(xdat,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
// 			if((ydat=(double *)realloc(ydat,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
// 			xdat[n]=aa;
// 			ydat[n]=bb;
// 			n++;
// 	}}
// 	if(strcmp(infile,"stdin")!=0) fclose(fpin);



	/* STORE DATA METHOD 2 - newline delimited input  from user-defined columns */
// 	if(strcmp(infile,"stdin")==0) fpin=stdin;
// 	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
// 	nn=0;
// 	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
// 		if(line[0]=='#') continue;
// 		pline=line; colmatch=2; // number of columns to match
// 		for(col=1;(pword=strtok(pline," ,\t\n\r"))!=NULL;col++) {
// 			pline=NULL;
// 			if(col==colx && sscanf(pword,"%f",&a)==1) colmatch--; // store value - check if input was actually a number
// 			if(col==coly && sscanf(pword,"%f",&b)==1) colmatch--; // store value - check if input was actually a number
// 		}
// 		if(colmatch!=0 || !isfinite(aa) || !isfinite(bb)) continue;
// 		if((xdatf=(float *)realloc(xdatf,(nn+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
// 		if((ydatf=(float *)realloc(ydatf,(nn+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
// 		xdatf[nn]=a;
// 		ydatf[nn]=b;
// 		nn++;
// 	}
// 	if(strcmp(infile,"stdin")!=0) fclose(fpin);
//

/* STORE DATA METHOD 4 - .BIN FORMAT, STDIN NOT PERMITTED, UNKNOWN-TO-FLOAT CONVERSION */
//  	fprintf(stderr,"START BINARY READ\n");
// 	if(strcmp(infile,"stdin")!=0) {
// 		parameters[0]= 2; /* data type (float in this case) */
// 		parameters[1]= 0; /* header-bytes to skip */
// 		parameters[2]= 0; /* numbers to skip */
// 		parameters[3]= 0; /* numbers to read (zero = read all) */
//
// 		data1= xf_readbin2_f(infile,parameters,message);
//
// 		if(data1!=NULL) n=parameters[3];
// 		else { fprintf(stderr,"\b\n\t--- Error[%s]: %s\n\n",thisprog,message); exit(1); }
// 	}
//  	fprintf(stderr,"STOP\n");
// 	exit(0);
//
// 	for(i=0;i<n;i++) printf("%d:	%g	%g\n",i,xdat[i],ydat[i]);
// 	xf_stats2_d(xdat,n,1,result_d);
//

/*
// BINARY READ TEST
ntoread=240000; // datatpoints to read
parameters[0]=3; // set input datattype to short int
parameters[1]=ntoread; // set number of values to read
parameters[2]=0; // pre-fill number of bytes read (not necessary actually - function does this)
if((data1=(float *)realloc(data1,ntoread*sizeof(float)))==NULL) { fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog); exit(1); }
sprintf(infile,"%s",argv[1]);
if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
while(1) {
	x= xf_readbin1_f(fpin,parameters,data1,message);
	nread=parameters[2];
	//TEST:fprintf(stdout,"x=%d\n",x);
	if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	else if(nread==0)break;
	else for(ii=0;ii<nread;ii++) printf("%g\n",data1[ii]);
}
fclose(fpin);
free(data1);
exit(0);
*/



	// // TEST MINIMUM-GOOD FILTER
	// nn=20;
	// short datas[100];
	// for(ii=0;ii<nn;ii++) datas[ii]=999;
	//
	// jj=0; kk=3; for(ii=jj;ii<(jj+kk);ii++) datas[ii]=ii;
	// jj=7; kk=2; for(ii=jj;ii<(jj+kk);ii++) datas[ii]=ii;
	// jj=11; kk=4; for(ii=jj;ii<(jj+kk);ii++) datas[ii]=ii;
	// jj=18; kk=2; for(ii=jj;ii<(jj+kk);ii++) datas[ii]=ii;
	// for(ii=0;ii<nn;ii++) printf("%ld	%d\n",ii,datas[ii]);
	// x= xf_filter_mingood2_s(datas,nn,1,3,999,message);
	// printf("\n");
	// for(ii=0;ii<nn;ii++) printf("%ld	%d\n",ii,datas[ii]);
	// exit(0);

	// // TEST strkey1 AND strcat FOR FINDING A KEYWORD VALUE IN A FILE
	// char *line1=NULL,line2[MAXLINELEN],line3[MAXLINELEN];
	// if(strcmp(infile,"stdin")==0) fpin=stdin;
	// else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	// while(fgets(line2,MAXLINELEN,fpin)!=NULL) {
	// 	line1= xf_strcat1(line1,line2,"");
	// }
	// if(strcmp(infile,"stdin")!=0) fclose(fpin);
	// printf("LINE1=\n%s[END]\n",line1);
	//
	// x= xf_strkey1(line1,"dark",-1,line3);
	// printf("KEY-VALUE=%s\n",line3);
	// free(line1);
	// exit(0);

/********************************************************************************/
/* MEMORY ALLOCATION  */
/********************************************************************************/

	// if((data= calloc(nn,sizeof(*data)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	// if((data= realloc(data,nn*sizeof(*data)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};

/********************************************************************************/
/* STRING OPERATIONS */
/********************************************************************************/

	// /* SIMPLE STRING CONCATENATION */
	// line=xf_strcat1(line,"newword","\n");

	// /* UNIQUE STRING CONCATENATION (EVERY WORD UNIQUE) - REPORTS WHICH WORD-IN-LINE MATCHED NEW WORD  */
	// line= xf_strcat2(line,word,'\t',&match,message);

	// /* PARSE A LINE INTO WORDS */
	// iwords= xf_lineparse2(line,"\t ,",&nwords);

	// /* PARSE A COMMA-SEPARATED COMAND LINE LIST INTO AN ARRAY */
	// iword= xf_lineparse2(setkeys,",",&nwords);
	// xdat=realloc(xdat,nwords*sizeof(float));
	// if(xdat==NULL) {{fprintf(stderr,"\n--- Error[%s]: memory allocation error\n\n",thisprog);exit(1);}}
	// for(ii=0;ii<nwords;ii++) xdat[ii]=atof(setkeys+iword[ii]);
	// for(ii=0;ii<nwords;ii++) printf("xdat[%ld]=%g\n",ii,xdat[ii]);

	// /* FIND (EXACT-MATCH) A WORD IN A LINE */
	// match= xf_strstr2(haystack,needle,'\t');

	// /* BUILD A LIST USING A SET OF INDIVIDUAL NUMBERS AND RANGES */
 	// list= xf_parselist1_l("1,21,30-40",",",min,max,&nn,message);

	// /* REPORTING ERRORS USING THE MESSAGE STRING */
	// if(x!=0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

/********************************************************************************/
/* MATHS OPERATIONS */
/********************************************************************************/

	// /* FIND THE NEAREST POWER OF TWO */
	// printf("nearest power=%ld\n",log2f(atof(argv[1])));exit(1);

	// /* MODULUS FUNCTION TEST */
	// 	off_t aaa=15,bbb=4,ccc;
	// 	if((aaa%bbb)!=0) fprintf(stderr,"ERROR: indivisible\n");
	// 	else fprintf(stderr,"divisible\n");


	// /* BUILDING TIMESTAMPS */
 	// // generate the initial timestamp
 	// t1 = time(NULL);
 	// // convert to local time and place into a structure
	// tstruct1 = localtime(&t1);
	// // create timestamp string
	// strftime(timestring,sizeof(timestring),"%Y%m%d%H%M%S",tstruct1);
	// fprintf(stderr,"\trecord %ld timestamp %s\n",ii,timestring);
	// // increment the time by a number of seconds - next time localtime() is called it will correctly change minutes, hours etc
	// t1+=200;




	/* TEST interpolation designed for block-read functions */
// 	short data_s[20]; nn=20; for(ii=0;ii<nn;ii++) data_s[ii]=ii;
// 	long params[9]={99,0,0,0};
// 	params[1]=atoi(argv[1]);
// 	data_s[0]=99;data_s[1]=99;data_s[2]=99;
// 	data_s[13]=99;data_s[14]=99;data_s[15]=99;
// 	data_s[18]=99;data_s[19]=99;
// 	x= xf_interp4_s(data_s, nn, params);
// 	for(ii=0;ii<nn;ii++) printf("%u\n",data_s[ii]);
// 	fprintf(stderr,"first good= %ld\n",params[2]);
// 	fprintf(stderr,"last good= %ld\n",params[3]);
// 	fprintf(stderr,"total good points= %ld\n",params[4]);
// 	fprintf(stderr,"final status= %d\n",x);
// 	exit(0);


// 	/* TEST xf_blockrealign2 : FUNCTION TO REALIGN TIMESTAMPS FOR BLOCK-CUT DATASETS */
//  	long nblocks=10;
// 	long blocklen=10,blockint=100;
//  	nn= 100;
// 	list= realloc(list,nn*sizeof(*list));
// 	start1= realloc(start1,nblocks*sizeof(*start1));
// 	stop1= realloc(stop1,nblocks*sizeof(*stop1));
// 	if(list==NULL||start1==NULL||stop1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
// 	// // build blocks
// 	// for(ii=0;ii<nblocks;ii++) {start1[ii]=blockint*ii; stop1[ii]=start1[ii]+blocklen;}
// 	// for(ii=0;ii<nblocks;ii++) printf("block %ld: %ld	%ld\n",ii,start1[ii],stop1[ii]);printf("\n");
// 	// // build fake timestamps - presumably post-chunking
// 	// for(ii=0;ii<nn;ii++) list[ii]=ii*blocklen+0.5*blocklen;
// 	// list[nn-1]=99;
// 	// for(ii=0;ii<nn;ii++) printf("new: %ld\n",list[ii]); printf("\n");
// 	// // realign
// 	// x= xf_blockrealign2(list,nn,start1,stop1,nblocks,message);
// 	// if(x!=0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
// 	// for(ii=0;ii<nn;ii++) printf("original: %ld\n",list[ii]);
//
// 	nn=4;
// 	nblocks=2;
// 	for(ii=0;ii<6;ii++) list[ii]=ii;
// 	start1[0]=100; stop1[0]=100;
// 	start1[1]=102; stop1[1]=106;
// 	for(ii=0;ii<nn;ii++) printf("new: %ld\n",list[ii]);
// 	x= xf_blockrealign2(list,nn,start1,stop1,nblocks,message);
// 	if(x!=0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
// 	for(ii=0;ii<nn;ii++) printf("old: %ld\n",list[ii]);
//
// 	exit(0);



	/* CIRCULAR BUFFER ROLL TEST */
// 	// fill the buffer
// 	for(ii=0;ii<ntoread;ii++) data1[ii]=(float)ii;
// 	// roll the buffer three places
// 	x= xf_rollbuffer1_f(data1,ntoread,3,1,message);
// 	if(x!=0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
// 	// print the rolled buffer
// 	for(ii=0;ii<10;ii++) printf("%ld	%g\n",ii,data1[ii]);
// 	exit(0);

	/* MATRIX DENSITY TEST - NEEDS "STORE DATA METHOD 2" */
// 	jj=3;kk=5; /* desired dimensions of matrix */
// 	mm=jj*kk; /* tot matrix elements */
// 	matrix= xf_densitymatrix1_l(xdatf,ydatf,nn,jj,kk,message);
// 	if(matrix==NULL)  { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
// 	for(ii=0;ii<mm;ii++) {
// 		printf("%ld\t",matrix[ii]);
// 		if((ii+1)%jj==0) printf("\n");
// 	}

goto END;

/********************************************************************************/
/* CLEANUP AND EXIT */
/********************************************************************************/
END:
	if(data1!=NULL) free(data1);
	if(line!=NULL) free(line);
	if(templine!=NULL) free(templine);
	if(keycols!=NULL) free(keycols);
	if(iword!=NULL) free(iword);
	exit(0);
}
