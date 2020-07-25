#define thisprog "xe-axona2dat"
#define TITLE_STRING thisprog" v 11: 21.October.2014 [JRH]"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h> /* must be included for directory reading */
#include <sys/stat.h> /* must be icncluded for directory reawding */
#include <ctype.h>

#define MAXLINELEN 10000
#define BLOCKSIZE 1000

/*
COMPILE: compile with option -D_FILE_OFFSET_BITS=64
NOTE: for Windows compilation, replace fseeko and ftello with _fseeki64 and _ftelli64, respectively

<TAGS>signal_processing file</TAGS>

CHANGES

v 10: 10.October.2014 [JRH]

	- add option to output sync-pulse times
	- useful if sync pulses are used for identifying times of auxilliary input

v 1.10: 25 February.2013 (JRH)
	- remove reference to hux_error function
	- remove unused hux_getpath function and path_data variable
	- replace use of hux_substring with strstr
	- replace use of hux_fillinterp_itime with xf_fillinterp_itime
		- some improvements to the interpolation function were made

- added -g option to specify the minimum gap in sync records at start or end of .bin file
	to be used for alignment of video data
*/

/* external functions start */
void xf_pause1(char message[]);
int xf_fillinterp_itime (long unsigned int *timeA,long unsigned int *timeB,float *valA,float *valB,long totA,long totB,float invalid,int max_invalid,char *type);
char *xf_strcut1(char *input, char delimiter, int firstlast, int prepost);
off_t xf_filesize (FILE *fpin);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general and file type variables */
	struct stat fileinfo; /* stat structure to hold file info */
	FILE *fpin=NULL,*fpout=NULL;
	char temp_str[256],*basename=NULL,outfile[256],*perror;
	int w,x,y,z,skip,fail,col,systemtype,allfiletot=0;
	unsigned long int i,j,k,n;
	float a,b,c;
	double aa,bb,cc;
	off_t aaa,bbb,ccc; /* extra-large integers for holding 64-bit file addresses (>2GB file size) */

	/* per-line temporary variables */
	char line[MAXLINELEN],templine[MAXLINELEN],*pline,*pcol,*tempcol;
	int tempprobe=-1,tempcell=-1, tempspikes=-1;
	float tempx=-1.0,tempy=-1.0, temprate=0.0,tempcoh=0.0,tempspars=0.0,tempsize=0.0;

	/* command line arguments */
	int setpause=0, outdat=1, outpos=1, outsync=1, syncpin=1, set_align=1, maxinv=0, setled=0;
	float  samprate=24000.0, camrate=25.0, whdrate=50.0, whdres=0.5;

	/* program-specific variable */
	char binfile[256],trackfile[256];
	short int  *syncval;
	unsigned long int *frametime,*lostsync,*whdtime,start,end,prev,n_bin=0,n_dat=0,frametot=0,framevalid=0,binsynctot=0,whdtot=0,whdvalid=0,lostbinsynctot=0;
	int binfiletot=0,trackfiletot=0,align=0,minutes=0,seconds=0,syncframedif=0;
	int framesamps,camxmax=768, camymax=576;  /* maximum possible x/y coordinates for camera used (typically 768/576)  */
	int size_packet, size_tracking, size_out, size_syncval, blocksread,bitshift,bitmask, truncdat;
	int remap[192]={ /* initialize array defining order in which Axona reads 3x64 channels  - first block of 64 only */
		32,33,34,35, 36,37,38,39, 0,1,2,3, 4,5,6,7,
		40,41,42,43, 44,45,46,47, 8,9,10,11, 12,13,14,15,
		48,49,50,51, 52,53,54,55, 16,17,18,19, 20,21,22,23,
		56,57,58,59, 60,61,62,63, 24,25,26,27, 28,29,30,31};
	float *framex,*framey,*framed, *whdx,*whdy,*whdd;
	float sampint,syncrate,syncint,syncwarning,camint,whdint,whdratio; // interval between samples for raw data, sync-signals, and framegrabber card
	float gapstart=0.0,gapend=0.0,gapmin=0.5;

	/* Axona data packet structure */
	struct AxonaRawPacket {
		char id[4];	// Record ID - "ADU1" = standard packet, "ADU2" = tracker data present
		unsigned int n;	// The number of the packet (4 bytes)
		unsigned int digio;	// Digital I/O and sync inputs (4 bytes, 32 bits)
		char track[20];	// Tracking info - 20 bytes
		short int data[192];// Data: 3 sequential 2-byte samples from 64 channels (1-64, 1-64, 1-64)
 		char funkey;	// Keystroke record - function key code
		char keycode;	// Keystroke record - key code
		char pad[14];	// Unused bytes
	} axraw[BLOCKSIZE];
	size_packet=sizeof(struct AxonaRawPacket); /* define size of data packet */

	/* Tracking system data packet structure */
	struct trackPacket {
		int n;
		int redx ; int redy ; int redpix;
		int greenx ; int greeny ; int greenpix;
		int bluex ; int bluey ; int bluepix;
		int hd ; int x ; int y;
		int start; int end;
		// the following are extra diagnostic values not to be used for tracking per se
		int pad[6];
	} tracking;
	size_tracking=sizeof(struct trackPacket); /* define size of tracking packet */

	/* csicsvari data structure: 3 sequential 2-byte samples from 64 channels (1-64, 1-64, 1-64)*/
	struct CsicsvariDat { short int data[192]; } datblock[BLOCKSIZE];
	size_out=sizeof(struct CsicsvariDat); /* define size of output packet */
	size_syncval=sizeof(short int);

	/* fill remap array with same remapping values for next 2 blocks of 64 */
	for(i=0;i<64;i++) {remap[i+64]=remap[i]+64 ; remap[i+128]=remap[i]+128; }

	/*********************************************************************************************************/
	/* Print instructions if only one argument */
	/*********************************************************************************************************/
	if(argc==1) {
		fprintf(stderr,"\n******************************************************************************\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"******************************************************************************\n");
		fprintf(stderr,"1. Create a .dat (Csicsvari raw data file) from an Axona .bin file\n");
		fprintf(stderr,"2. Create a .whd & .xyd file from a .bin and a .trk (video tracking) file\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"The .trk file is binary file with multiple integers per record\n");
		fprintf(stderr,"Making the .whd/.xyd files requires a sync signal in the .bin file\n");
		fprintf(stderr,"This should be on one of the Axona digital I/O pins\n");
		fprintf(stderr,"If there are more camera frames than sync signals, do frame-alignment\n");
		fprintf(stderr,"This requires a gap at the start or end of the sync records\n");
		fprintf(stderr,"If there is no gap, alignment cannot be performed\n");
		fprintf(stderr,"Valid arguments - defaults in []\n");
		fprintf(stderr,"	-g: minimum gap (s) in sync signals to allow video data alignment [%g]\n",gapmin);
		fprintf(stderr,"	-outdat: output dat file? 0=no, 1=yes [%d]\n",outdat);
		fprintf(stderr,"	-outpos: output whd file? 0=no, 1=yes [%d]\n",outpos);
		fprintf(stderr,"	-outsync: output values on sync pin? 0=no, 1=yes [%d]\n",outsync);
		fprintf(stderr,"	-camrate: frame capture rate of video camera [%.0f]\n",camrate);
		fprintf(stderr,"	-samprate: sampling rate of binary Axona input [%.0f]\n",samprate);
		fprintf(stderr,"	-whdrate: output sampling rate for whd file [%.3f]\n",whdrate);
		fprintf(stderr,"	-whdres: stepdown for x,y resolution (x*whdres,y*whdres) [%.3f]\n",whdres);
		fprintf(stderr,"	-syncpin: Axona digital I/O pin (bit) carrying sync signal [%d]\n",syncpin);
		fprintf(stderr,"	-align: realign video frames if there are fewer sync crecords [%d]\n",set_align);
		fprintf(stderr,"		0: disallow - number of sync records and video frames must match\n");
		fprintf(stderr,"		1: allow - align frames to start or end sync records\n");
		fprintf(stderr,"	-maxinv: max invalid points across which to interpolate whd data [%d]\n",maxinv);
		fprintf(stderr,"	-led: using only one LED (no head direction) 1=red, 2=green, 3=blue [%d]\n",setled);
		fprintf(stderr,"	-pause on completion: 0=no, 1=yes [%d]\n",setpause);
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE: %s [.bin file] [.trk file] [arguments]\n",thisprog);
		fprintf(stderr,"\n");
		exit(0);
	}

	/*********************************************************************************************************/
	/* read file names: arguments up to the first argument beginning with "-" */
	/*********************************************************************************************************/
	allfiletot=0;
	for (i=1;i<argc;i++) {
		if( *(argv[i]+0) == '-' ) break;
		else {
			stat(argv[i],&fileinfo);
			if((fileinfo.st_mode & S_IFMT) == S_IFDIR){ fprintf(stderr,"\n--- Error[%s]: %s is a directory\n\n",thisprog,argv[i]);exit(1); }
			allfiletot++;
			if(strstr(argv[i],".bin")!=NULL || strstr(argv[i],".BIN")!=NULL) {strncpy(binfile,argv[i],256); binfiletot++;}
			if(strstr(argv[i],".trk")!=NULL || strstr(argv[i],".TRK")!=NULL) {strncpy(trackfile,argv[i],256); trackfiletot++;}
	}}
	if(binfiletot!=1) {fprintf(stderr,"Error[%s]: exactly one raw Axona file (.bin) must be specified\n",thisprog); exit(1);}
	if(trackfiletot>1) {fprintf(stderr,"Error[%s]: at most one tracking file (.trk) can be specified\n",thisprog); exit(1);}

	/*********************************************************************************************************/
	/* read remaining command line arguments: identifier starting with "-" followed by a value */
	/*********************************************************************************************************/
	for(i=i;i<argc;i++) {
		if( *(argv[i]+0) == '-' && i < argc-1) {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			if(strcmp(argv[i],"-pause")==0)         setpause=atoi(argv[++i]);
			else if(strcmp(argv[i],"-g")==0)        gapmin=atof(argv[++i]);
			else if(strcmp(argv[i],"-samprate")==0) samprate=atof(argv[++i]);
			else if(strcmp(argv[i],"-camrate")==0)  camrate=atof(argv[++i]);
			else if(strcmp(argv[i],"-whdrate")==0)  whdrate=atof(argv[++i]);
			else if(strcmp(argv[i],"-whdres")==0)   whdres=atof(argv[++i]);
			else if(strcmp(argv[i],"-outdat")==0)   outdat=atoi(argv[++i]);
			else if(strcmp(argv[i],"-outpos")==0)   outpos=atoi(argv[++i]);
			else if(strcmp(argv[i],"-outsync")==0)  outsync=atoi(argv[++i]);
			else if(strcmp(argv[i],"-syncpin")==0)  syncpin=atoi(argv[++i]);
			else if(strcmp(argv[i],"-align")==0)    set_align=atoi(argv[++i]);
			else if(strcmp(argv[i],"-maxinv")==0)   maxinv=atoi(argv[++i]);
			else if(strcmp(argv[i],"-led")==0)      setled=atoi(argv[++i]);
			else {fprintf(stderr,"Error[%s]: invalid command line argument \"%s\"\n",thisprog,argv[i]); exit(1);}
	}}

	printf("\n");
	printf("bin_file: %s\n",binfile);
	if(trackfiletot>0) printf("track_file: %s\n",trackfile);
	else outpos=0;

	syncrate=samprate/3.0;
	sampint=1.0/samprate;
	camint=1.0/camrate;
	syncint=1.0/syncrate;
	whdint=1.0/whdrate;
	whdratio=samprate/whdrate;

	if(whdratio!=(int)whdratio) {fprintf(stderr,"Error[%s]: -whdrate should be an integer ratio of -samprate\n",thisprog); exit(1);}


	/*********************************************************************************************************/
	/* define basename for building other filenames */
	/*********************************************************************************************************/
	basename=xf_strcut1(binfile, '.', 1,1);

	/*********************************************************************************************************/
	/* create bit-mask: a logical AND of a value combined with a bit-shift will yield "1" if the bit is set  */
	/*********************************************************************************************************/
	bitshift= syncpin-1; // bit index and amount to shift bits to convert a match to "1"
	bitmask= (int)pow(2,syncpin-1); // mask to identify if the bit (syncpin) is set


	/*********************************************************************************************************/
	/* DETERMINE NUMBER OF .BIN RECORDS AND ALLOCATE MEMORY */
	/*********************************************************************************************************/
	/* create output filename, open binfile & outfile */
	if((fpin=fopen(binfile,"rb"))==NULL) {fprintf(stderr,"Error[%s]: can't open input file \"%s\"\n",thisprog,binfile); exit(1);}
	if(outdat==1) {
		sprintf(outfile,"%s.dat",basename);
		fpout=fopen(outfile,"wb");
		if(fpout==NULL) {fprintf(stderr,"Error[%s]: can't open output file \"%s\"\n",thisprog,outfile); exit(1);}
		printf("output_dat_file: %s\n",outfile);
	}
	/* determine size of .bin file and whether the number of bytes is appropriate */
	aaa=xf_filesize(fpin); bb=(float)aaa/(float)size_packet;
	if(aaa % size_packet != 0) {fprintf(stderr,"Error![%s]: \"%s\" may be corrupt: \n\t%ld bytes & packet size %d suggests %.2f records)\n",thisprog,binfile,aaa,size_packet,bb);exit(1);}
	else n_bin=aaa/size_packet;

	/* reserve memory for n+1 sync records (1 byte each) and frame capture times (unsigned int) */
	if((syncval=(short int *) malloc((n_bin+1)*sizeof(short int)))==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog); exit(1);}
	/* print .bin summary */
	minutes=(int)((n_bin*3)/samprate/60.0);
	seconds=(int)((n_bin*3)/samprate-(minutes*60));
	printf("bin_filesize=%ld\n",aaa);
	printf("bin_sample_rate: %.2f\n",samprate);
	printf("bin_samples: %ld\n",n_bin*3);
	printf("bin_duration: %d minutes %d seconds\n",minutes,seconds);

	/* In order to create eeg files that are exactly 1/16th the size of the .dat file, the .dat file must be divisible by 16 */
	truncdat= n_bin-(n_bin%16);

	/*********************************************************************************************************/
	/* READ AXONA .BIN FILE AND OUTPUT .DAT FILE, TRUNCATING TO A MULTIPLE OF 16 BLOCKS */
	/*********************************************************************************************************/
	n_bin=n_dat=0; // record counter, incriments by the number of blocks read each read-iteration
	blocksread=0; // counter for the number of data blocks read (BLOCKSIZE determines the number of blocks attempted to be read)
	syncwarning=1; // flag to indicate if none of the digital i/o pins ever deviate from zero - this would suggest no input on any pins
	rewind(fpin);
	while((blocksread = fread(&axraw,size_packet,BLOCKSIZE,fpin)) == BLOCKSIZE) {
		/* the following will only happen if a full BLOCKSIZE number of data packets were read */
		z=0;
		for(i=0;i<BLOCKSIZE;i++) {
			/* store video sync data as 0 or 1, based only on whether the appropriate digital I/O bit was set */
			syncval[n_bin]= ( (axraw[i].digio & bitmask) >> bitshift );
			if(axraw[i].digio!=0) syncwarning=0;
			if(outdat==1) for (j=0;j<192;j++) datblock[i].data[j] = axraw[i].data[remap[j]]+1; // sort original data into correct probe sequence
			if(n_bin<truncdat) z++; // z will incriment up to BLOCKSIZE if total data records does not exceed truncdat
			n_bin++;
		}
		n_dat+=z;
		if(outdat==1) fwrite(datblock,size_out,z,fpout); // "z" = blocks read before truncdat was exceded
	}
	/* Now output remaining records (<BLOCKSIZE) which were read in the final (unassigned) iteration in the above loop */
	z=0;
	for(i=0;i<blocksread;i++) {
		syncval[n_bin]= ( (axraw[i].digio & bitmask) >> bitshift ); // store video sync data as 0 or 1
		if(outdat==1) for (j=0;j<192;j++) datblock[i].data[j] = axraw[i].data[remap[j]]+1; // sort original data into correct probe sequence
		if(n_bin<truncdat) z++;
		n_bin++;
	}
	fclose(fpin);
	n_dat+=z;
	if(outdat==1) { fwrite(datblock,size_out,z,fpout); fclose(fpout); } // "z" = blocks read before truncdat was exceded

	if(syncwarning==1) {fprintf(stderr,"--- Warning: [%s]: digital input status remains zero on all pins\n",thisprog);}

	/* print .dat summary */
	minutes=(int)((n_dat*3)/samprate/60.0);
	seconds=(int)((n_dat*3)/samprate-(minutes*60));
	printf("dat_truncate %ld datablocks to be dropped to make file divisible by 16\n",(n_bin%16));
	printf("dat_samples: %ld\n",n_dat*3);
	printf("dat_duration: %d minutes %d seconds\n",minutes,seconds);


	/*********************************************************************************************************/
	/* OUTPUT THE SYNC TIMES (SAMPLE-NUMBER) */
	/*********************************************************************************************************/
	if(outsync==1) {
		sprintf(outfile,"%s.sync",basename);
		fpout=fopen(outfile,"wb");
		if(fpout==NULL) {fprintf(stderr,"Error[%s]: can't open output file \"%s\"\n",thisprog,outfile); exit(1);}
		printf("output_sync_file: %s\n",outfile);
		for(i=2;i<n_bin;i++) { // start at i=2 because first sync value in .binfile is often erroneously set to "1"
			/* detect a sync step from 0 to 1 */
			if(syncval[i-1]==0 && syncval[i]==1) fprintf(fpout,"%ld\n",i*3);
		}
		fclose(fpout);
	}

	//TEST:	for(i=2;i<n_dat;i++) if(syncval[i-1]==0 && syncval[i]==1) printf("%d\n",syncval[i]); exit(0);

	/*********************************************************************************************************/
	/* STOP HERE IF WHD FILE IS NOT TO BE PRODUCED! */
	/*********************************************************************************************************/
	if(outpos==0) {free(syncval); printf("\n"); if(setpause==1) xf_pause1("Press ENTER to finish"); exit(0); }

	/*********************************************************************************************************/
	/* READ TRACKING FILE */
	/*********************************************************************************************************/
	if((fpin=fopen(trackfile,"rb"))==NULL) {fprintf(stderr,"Error[%s]: can't open input file \"%s\"\n",thisprog,trackfile); exit(1);}

	/* determine size of .trk file and whether the number of bytes is appropriate */
	aaa=xf_filesize(fpin) ; frametot= aaa/size_tracking ;
	if(aaa % size_tracking != 0) {fprintf(stderr,"Error![%s]: \"%s\" may be corrupt: %ld bytes suggests %g records)\n",thisprog,trackfile,aaa,bb); exit(1);}

	/* reserve memory for maximum possible frames (derived from video tracking record) */

	if((frametime=(unsigned long int *) malloc((frametot+1)*sizeof(unsigned long int)))==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog); exit(1);}
	if((lostsync=(unsigned long int *) malloc((frametot+1)*sizeof(unsigned long int)))==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog); exit(1);}
	if((framex=(float *) malloc((frametot+1)*sizeof(float)))==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog); exit(1);}
	if((framey=(float *) malloc((frametot+1)*sizeof(float)))==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog); exit(1);}
	if((framed=(float *) malloc((frametot+1)*sizeof(float)))==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog); exit(1);}
	i=0; j=0;
	/* If 3 LED's are to be used to extract position... */
	if(setled==0) while(fread(&tracking,size_tracking,1,fpin)==1) {
		framed[i]=(float)tracking.hd;
		framex[i]=(float)tracking.x;
		framey[i]=(float)tracking.y;
		i++;}
	/* If only the red LED is be used to extract position... */
	else if(setled==1) while(fread(&tracking,size_tracking,1,fpin)==1) {
		framed[i]=-1.0;
		framex[i]=(float)tracking.redx;
		framey[i]=(float)tracking.redy;
		i++;}
	/* If only the green LED is be used to extract position... */
	else if(setled==2) while(fread(&tracking,size_tracking,1,fpin)==1) {
		framed[i]=-1.0;
		framex[i]=(float)tracking.greenx;
		framey[i]=(float)tracking.greeny;
		i++;}
	/* If only the blue LED is be used to extract position... */
	else if(setled==3) while(fread(&tracking,size_tracking,1,fpin)==1) {
		framed[i]=-1.0;
		framex[i]=(float)tracking.bluex;
		framey[i]=(float)tracking.bluey;
		i++;}
	/* now deal with invalid tracking points */
	framevalid=0;
	for(i=0;i<frametot;i++) {
		if(framex[i]<2||framex[i]>camxmax||framey[i]<2||framey[i]>camymax) {
			framed[i]=framex[i]=framey[i]=-1.0;
		}
		else if(framed[i]<0) framed[i]=-1;
		else if(framed[i]>=360) framed[i]-=360;
		else framevalid++;
	}

	printf("camera_capture_rate: %g\n",camrate);
	printf("trk_total_video_frames: %ld\n",frametot);
	printf("trk_valid_video_frames: %ld (%.02f %%) \n",framevalid,(100.0*framevalid/(float)frametot));

	// DIAGNOSTIC - check pre-interpolation frame data
	// for(i=0;i<frametot;i++) {printf("%f	%f\n",framex[i],framey[i]);}

	/*********************************************************************************************************/
	/* DETERMINE THE SYNC-SIGNAL GAP AT THE START AND END OF THE BIN FILE */
	/*********************************************************************************************************/
	// calculate empty space at start of the file
	// start at sample 2 because for some reason syncval[0] is often set to "1" but following values are "0"
	for(i=2;i<n_bin;i++) if(syncval[i]!=syncval[i-1]) break;
	if(i<n_bin) gapstart=(i*syncint);
	// calculate empty space at end of the file
	for(i=n_bin-1;i>0;i--) if(syncval[i]!=syncval[i-1]) break;
	if(i>=0) gapend=(n_bin*syncint)-(i*syncint);

	/*********************************************************************************************************/
	/* COUNT THE SYNC SIGNALS AND STORE THE SYNC TIMES */
	/*********************************************************************************************************/
	framesamps= (int)(syncrate*camint); // typical number of samples corresponding with one frame capture
	prev=0; binsynctot=0; lostbinsynctot=0;
	for(i=2;i<n_bin;i++) { // start at i=2 because first sync value in .binfile is often erroneously set to "1"
		if(syncval[i-1]==0 && syncval[i]==1) { // detect a sync step from 0 to 1
			frametime[binsynctot]=i*3; // convert time to higher (3x) "samprate" base, instead of "syncfreq" base
			lostsync[binsynctot]=0;
			if(binsynctot>0 && (i-prev)>(1.5*framesamps)) {
				lostsync[binsynctot]=(unsigned long int)(-1+((i-prev)+(0.5*framesamps))/framesamps);
				lostbinsynctot += lostsync[binsynctot];
			}
			binsynctot++;
			prev=i;
	}}
	printf("bin_total_video_syncs: %ld\n",binsynctot);
	printf("bin_missing_syncs: %ld (%.02f %%)\n",lostbinsynctot,(100.0*(float)lostbinsynctot/(float)binsynctot));
	printf("bin_first_video_sync: %.3f seconds from start\n",gapstart);
	printf("bin_last_video_sync: %.3f seconds from end\n",gapend);
	printf("bin_video_sync_duration: %.3f seconds total\n",(frametime[binsynctot-1]-frametime[0])/samprate);

	if(binsynctot==0) {fprintf(stderr,"Error[%s]: no sync signals were detected in .bin file\n",thisprog); exit(1);}

	if(binsynctot>frametot) {fprintf(stderr,"Error[%s]: more sync signals in .bin file (%ld) than video frames in .trk file (%ld) \n",thisprog,binsynctot,frametot);exit(1);
	}

	/*********************************************************************************************************
	- align the sync signal timeseries with the x/y/lheaddirection data in the tracking records
	- remember that assuming the data can be aligned, frame "0" now corresponds with the first sync signal
	- note that this does not pad the beginning or end of the data record before (or after) tracking started
	*********************************************************************************************************/
	if(binsynctot!=frametot) {
		if(set_align==0) {fprintf(stderr,"Error[%s]: missing sync signals but realignment disallowed (-align 0)\n",thisprog); exit(1);}
		if(gapstart<gapmin&&gapend<gapmin) {fprintf(stderr,"Error[%s]: missing sync signals but no gap at start or end of BIN file - realignment impossible\n",thisprog); exit(1);}
		// if there is a gap at the start, align=1 (whd output needs to be padded with extra -1s at the start)
		if(gapstart>=gapmin) align=1;
		// if the only gap is at the end, assume the last sync = last frame, and shift frame pointers to data forward
		else if(gapend>=gapmin) {
			align=2;
			syncframedif=(frametot-binsynctot); // calculate the number of missing sync signals
			framex=framex+syncframedif; // advance pointer to frame x-values
			framey=framey+syncframedif; // advance pointer to frame y-values
			framed=framed+syncframedif; // advance pointer to frame direction-values
	}}
	if(align==0) printf("video_realignment: none\n");
	if(align==1) printf("video_realignment: start\n");
	if(align==2) printf("video_realignment: end\n");

	/*********************************************************************************************************/
	/* CREATE XYD FILE OUTPUT */
	/*********************************************************************************************************/
	sprintf(outfile,"%s.xyd",basename);
	if((fpout=fopen(outfile,"w"))==NULL) {fprintf(stderr,"Error[%s]: can't open output file \"%s\"\n",thisprog,outfile); exit(1);}
	framevalid=0;
	for(i=binsynctot;i<=frametot;i++) lostsync[i]=0; // initialise remaining lostsync values - this is important for padding step below
	for(i=0;i<binsynctot;i++) {
		if(framex[i]!=-1) framevalid++; // count valid frames (x/y values are >=0)
		fprintf(fpout,"%ld	%g	%g	%g\n",frametime[i],framex[i],framey[i],framed[i]);
		for(j=0;j<lostsync[i+1];j++) {fprintf(fpout,"%ld\t-2\t-2\t-2\n",(frametime[i]+framesamps*(j+1)*3));} // padd missing values between this and next sync record
	}
	fclose(fpout);
	printf("xyd_file: %s\n",outfile);
	printf("xyd_total_records: %ld (%ld + %ld lost syncs tagged \"-2\"))\n",binsynctot+lostbinsynctot,binsynctot,lostbinsynctot);


	/*********************************************************************************************************/
	/* CREATE WHD FILE OUTPUT */
	/*********************************************************************************************************/
	// determine how many whd records do we need to make? whdratio is the ratio of sampling frequencies - bin/whd
	whdtot=(long unsigned int) ((n_bin*3)/whdratio); // this will underestime the number of .whd records required by some fractional amount
	// allocate appropriate amount of memory
	if((whdtime=(unsigned long int *) malloc((whdtot+1)*sizeof(unsigned long int)))==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog); exit(1);}
	if((whdx=(float *) malloc((whdtot+1)*sizeof(float)))==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog); exit(1);}
	if((whdy=(float *) malloc((whdtot+1)*sizeof(float)))==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog); exit(1);}
	if((whdd=(float *) malloc((whdtot+1)*sizeof(float)))==NULL) {fprintf(stderr,"Error[%s]: insufficient memory\n",thisprog); exit(1);}
	// fill the whd time array - these indicate where in the original data each whd record falls (sample number)
	for(i=0;i<whdtot;i++) whdtime[i]=(long unsigned int)(i*whdratio);
	// fill whd arrays with interpolated data from frame records
	xf_fillinterp_itime(frametime,whdtime,framex,whdx,binsynctot,whdtot,(float)-1,maxinv,"linear");
	xf_fillinterp_itime(frametime,whdtime,framey,whdy,binsynctot,whdtot,(float)-1,maxinv,"linear");
	xf_fillinterp_itime(frametime,whdtime,framed,whdd,binsynctot,whdtot,(float)-1,maxinv,"circular");

	sprintf(outfile,"%s.whd",basename);
	if((fpout=fopen(outfile,"w"))==NULL) {fprintf(stderr,"Error[%s]: can't open output file \"%s\"\n",thisprog,outfile); exit(1);}

//	for(i=0;i<whdtot;i++) fprintf(fpout,"%d	%.2f	%.2f	%.2f\n",whdtime[i],whdx[i],whdy[i],whdd[i]);
	for(i=0;i<whdtot;i++) {
		if(whdx[i]!=-1) a = whdx[i]*whdres; else a=-1.0;
		if(whdy[i]!=-1) b = whdy[i]*whdres; else b=-1.0;
		fprintf(fpout,"%g	%g	%g\n",a,b,whdd[i]);
		if(a!=-1&&b!=-1) whdvalid++;
	}

	printf("whd_file: %s\n",outfile);
	printf("whd_sample_rate: %g\n",whdrate);
	printf("whd_resolution_stepdown: %g\n",whdres);
	printf("whd_total_records: %ld\n",whdtot);
	printf("whd_valid_records: %ld (%.02f %%) \n",whdvalid,(100.0*whdvalid/(float)whdtot));
	printf("\n");

	// reset pointers to previously adjusted frames variables - ESSENTIAL if "free" is to work properly
	if(syncframedif!=0) {
		framex=framex-syncframedif;
		framey=framey-syncframedif;
		framed=framed-syncframedif;
	}

	free(syncval); free(frametime); free(lostsync); free(framex); free(framey);free(framed);
	free(whdtime); free(whdx); free(whdy);free(whdd);
	if(basename!=NULL) free(basename);

	if(setpause==1) xf_pause1("Press ENTER to finish");
	exit(0);
}
