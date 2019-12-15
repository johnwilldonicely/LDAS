#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define thisprog "xe-ldas-txt2clb1"
#define TITLE_STRING thisprog" v 2: 6.October.2014 [JRH]"
#define MAXLINELEN 1000

/* <TAGS> file SCORE </TAGS> */

/*
v 3: 28.April.2015 [JRH]
	- fix aspects of the CLB header:
		- the XML specification states the <?xml> element must be the very first thing in the document
		- in the metadata, the "SampleFreq" element should be "sampleRate"
		- there appears to be a line ending at the beginning of the XML metadata which displeases the XML parser

	- add options for CLB v.2.0 compliance
		- change the major version in the binary header to 0x00000002.
		- change the version in the XML metadata to "2.0"
		- add an element after the "recordTimeLength" element named "recordTimeIncrement"
			- for waveforms, the value should simply match the "recordTimeLength" value

v 2: 6.October.2014 [JRH]
	- change argument sr (sampling rate) to sf (sampling frequency) for consistency with other programs
*/

int main (int argc, char *argv[]) {

	/* general variables */
	char line[MAXLINELEN],message[MAXLINELEN];
	long int i,j,k,n;
	int v,w,x,y,z;
	float a,b,c,d;
	double aa,bb,cc,dd,ee;
	FILE *fpin,*fpout;
	size_t ii,jj,kk,nn,mm;
	size_t sizeofchar=sizeof(char);
	size_t sizeofshort=sizeof(short),sizeofint=sizeof(int),sizeoflong=sizeof(long);
	size_t sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);

	/* program-specific variables */
	char *metadata=NULL,m0[128],m1[64],m2[64],m3[64],m4[64],m5[64],m6[64],m7[64],m8[64],m9[64],m10[64],m11[64];
	char timestring[20];
	short *data1=NULL,*pdata=NULL;
	int version_major,version_minor,MetaLength;
	float version_xml;
	off_t BitDepth,ByteDepth,SamplesPerRecord,BytesPerRecord;
	size_t nrecords;
	time_t t1;
	struct tm *tstruct1;

	/* arguments */
	char infile[256],outfile[256];
	int setversion=1.0;
	off_t SampleFreq=400,RecordTimeLength=10;

	sprintf(outfile,"stdout");

	/********************************************************************************/
	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	/********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Convert ASCII input stream to 3EG .clb file (16-bit short int)\n");
		fprintf(stderr,"Assumes one valid numeric value per input line\n");
		fprintf(stderr,"Assumes the data type is \"waveform\" (continuously sampled data)\n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"Timestamps assigned to records reflect time of conversion\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-sf: sample rate (Hz) [%ld]\n",SampleFreq);
		fprintf(stderr,"	-dur: duration (seconds) of each record [%ld]\n",RecordTimeLength);
		fprintf(stderr,"	-out: output file name, or \"stdout\" [%s]\n",outfile);
		fprintf(stderr,"	-version: define CLB/XML version to use [%d]\n",setversion);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -sf 400 -dur 60 -out data.clb\n",thisprog);
		fprintf(stderr,"	cat data.txt | %s stdin -sf 100 -dur 20 -out stdout\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/********************************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	/********************************************************************************/
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-sf")==0) SampleFreq=atoi(argv[++i]);
			else if(strcmp(argv[i],"-dur")==0) RecordTimeLength=atoi(argv[++i]);
			else if(strcmp(argv[i],"-out")==0) sprintf(outfile,"%s",argv[++i]);
			else if(strcmp(argv[i],"-version")==0) setversion=atoi(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(setversion!=1&&setversion!=2) {fprintf(stderr,"\n--- Error[%s]: invalid -version (%d) : must be 1 or 2 \n\n",thisprog,setversion); exit(1);}




	/********************************************************************************/
	/* CREATE METADATA STRING */
	/********************************************************************************/
	ByteDepth=sizeof(short);
	BitDepth=ByteDepth*8;
	SamplesPerRecord=SampleFreq*RecordTimeLength;
	BytesPerRecord=SamplesPerRecord*ByteDepth;

	if(setversion==1) {
		version_major=1;
		version_minor=0;
		version_xml=1.0;
		sprintf(m0,"<\?xml version=\"%.1f\" encoding=\"utf-8\"\?>\n<clb version=\"%d.%d\">\n\t<standard>\n",version_xml,version_major,version_minor);
		sprintf(m1,"\t\t<subjectId>00000</subjectId>\n");
		sprintf(m2,"\t\t<dataTypeId>Score.HippocampusLfp</dataTypeId>\n");
		sprintf(m3,"\t\t<dataFormatId>Standard.Waveform</dataFormatId>\n");
		sprintf(m4,"\t\t<dataByteLength>%ld</dataByteLength>\n",BytesPerRecord);
		sprintf(m5,"\t\t<recordTimeLength>%ld</recordTimeLength>\n",RecordTimeLength);
		sprintf(m6,"\t\t<waveform>\n");
		sprintf(m7,"\t\t\t<bitDepth>%ld</bitDepth>\n",BitDepth);
		sprintf(m8,"\t\t\t<sampleRate>%ld</sampleRate>\n",SampleFreq);
		sprintf(m9,"\t\t\t<samplesPerRecord>%ld</samplesPerRecord>\n",SamplesPerRecord);
		sprintf(m10,"\t\t</waveform>\n\t</standard>\n</clb>\n");

		MetaLength=strlen(m0)+strlen(m1)+strlen(m2)+strlen(m3)+strlen(m4)+strlen(m5)+strlen(m6)+strlen(m7)+strlen(m8)+strlen(m9)+strlen(m10);
		if((metadata=(char *)realloc(metadata,(MetaLength+1)*sizeofchar))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		sprintf(metadata,"%s%s%s%s%s%s%s%s%s%s%s",m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,m10);
	}

	if(setversion==2) {
		version_major=2;
		version_minor=0;
		version_xml=2.0;
		sprintf(m0,"<\?xml version=\"%.1f\" encoding=\"utf-8\"\?>\n<clb version=\"%d.%d\">\n\t<standard>\n",version_xml,version_major,version_minor);
		sprintf(m1,"\t\t<subjectId>00000</subjectId>\n");
		sprintf(m2,"\t\t<dataTypeId>Score.HippocampusLfp</dataTypeId>\n");
		sprintf(m3,"\t\t<dataFormatId>Standard.Waveform</dataFormatId>\n");
		sprintf(m4,"\t\t<dataByteLength>%ld</dataByteLength>\n",BytesPerRecord);
		sprintf(m5,"\t\t<recordTimeLength>%ld</recordTimeLength>\n",RecordTimeLength);
		// extra field in version 2: for waveforms, same value as RecordTimeLength
		sprintf(m6,"\t\t<recordTimeIncrement>%ld</recordTimeIncrement>\n",RecordTimeLength);
		sprintf(m7,"\t\t<waveform>\n");
		sprintf(m8,"\t\t\t<bitDepth>%ld</bitDepth>\n",BitDepth);
		sprintf(m9,"\t\t\t<sampleRate>%ld</sampleRate>\n",SampleFreq);
		sprintf(m10,"\t\t\t<samplesPerRecord>%ld</samplesPerRecord>\n",SamplesPerRecord);
		sprintf(m11,"\t\t</waveform>\n\t</standard>\n</clb>\n");

		MetaLength=strlen(m0)+strlen(m1)+strlen(m2)+strlen(m3)+strlen(m4)+strlen(m5)+strlen(m6)+strlen(m7)+strlen(m8)+strlen(m9)+strlen(m10)+strlen(m11);
		if((metadata=(char *)realloc(metadata,(MetaLength+1)*sizeofchar))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		sprintf(metadata,"%s%s%s%s%s%s%s%s%s%s%s%s",m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11);
	}



	//fprintf(stderr,"%s",metadata);
	fprintf(stderr,"header_length: %ld bytes\n",(MetaLength+19));



	/********************************************************************************/
	/* STORE DATA as short integers */
	/********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf",&aa)!=1) continue;
		if(isfinite(aa)) {
			if((data1=(short *)realloc(data1,(nn+1)*sizeofshort))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			data1[nn++]=(short)aa;
	}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(nn<=0) {fprintf(stderr,"\n--- Error[%s]: input file \"%s\" has no numeric content\n\n",thisprog,infile); exit(1);}

	fprintf(stderr,"total_samples: %ld\n",nn);
	//TEST: for(ii=0;ii<10;ii++) fprintf(stderr,"%d\n",data1[ii]); exit(0);


	/********************************************************************************/
	/* SET UP WRITE PARAMETERS */
	/********************************************************************************/
	/* calculate number of records */
	if(nn%SamplesPerRecord!=0) {fprintf(stderr,"\n--- Error[%s]: total_samples (%ld) not a multiple of record-size (%ld seconds at %ld Hz = %ld samples)\n\n",thisprog,nn,SampleFreq,RecordTimeLength,SamplesPerRecord);exit(1);}
	nrecords= nn/SamplesPerRecord;
	fprintf(stderr,"total_records: %ld\n",nrecords);

	/* generate the initial timestamp */
	t1 = time(NULL);
	/* reset seconds to zero */
	t1=60*(size_t)(t1/60);
	/* set up a pointer to the initial data */
	pdata=data1;

	/********************************************************************************/
	/* WRITE DATA */
	/********************************************************************************/
	if(strcmp(outfile,"stdout")==0) fpout=stdout;
	else if((fpout=fopen(outfile,"wb"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile);exit(1);}

	/* write the 19-byte header */
	fwrite("CLB3EG\0",7,1,fpout);
	fwrite(&version_major,4,1,fpout);
	fwrite(&version_minor,4,1,fpout);
	fwrite(&MetaLength,4,1,fpout);
	/* write the metadata */
	fwrite(metadata,MetaLength,1,fpout);

	/* write the data one record at a time */
	for(ii=0;ii<nrecords;ii++) {

		/* create timestamp string */
		tstruct1 = localtime(&t1);
		strftime(timestring,sizeof(timestring),"%Y%m%d%H%M%S",tstruct1);
		fprintf(stderr,"\trecord %ld timestamp %s\n",ii,timestring);

		/* write the timestamp */
		jj= fwrite(timestring,14,1,fpout);

		/* write the current data record - note that data1 pointer is incremented for each record */
		kk= fwrite(data1,BytesPerRecord,1,fpout);

		/* check for errors */
		if(jj!=1 || kk!=1) {
			fprintf(stderr,"\n--- Error[%s]: problem writing record %ld to binary file %s (errno=%d)\n\n",thisprog,ii,outfile,ferror(fpout));
			if(strcmp(outfile,"stdout")!=0) fclose(fpout);
			exit(1);
		}

		/* increment data pointer */
		data1+=SamplesPerRecord;
		/* increment the time in seconds */
		t1+=RecordTimeLength;
	}


	/* close the file if not standard output */
	if(strcmp(outfile,"stdout")!=0) fclose(fpout);

	/* reset the data1 pointer so that the memory can be properly freed */
	data1=pdata;

	/* free memory*/
	if(data1!=NULL) free(data1);
	if(metadata!=NULL) free(metadata);

	exit(0);
}
