#define thisprog "xe-ldas2-makecmt1"
#define TITLE_STRING thisprog" v 10: 8.October.2012 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000
# define TZMAX 5
# define RZMAX 5

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS> EV </TAGS>

.EVtcv: conditioned version of an Ethovision trial-control record
	- three columns: time, variable, value
	- quotation marks removed
	- spaces converted to underscores
	- semicolon delimiters converted to tabs
	- this conditioning performed by xs-estt-extract
.EVtrk: conditioned version of an Ethovision track file
			- time,x,y,dir,dist, and in/out status for trigger and reward zones

v 10: 8.October.2012 [JRH]
	- eliminate use of track file to detect zone-exits - unnecessary now that nose-point is used for entries and exits
	- add option to use START_TRACK keyword to indicate true time-zero for the record
	- ignore repeated entries into the same zone 
	- change so that the following events trigger comments:
			- CORRECT_CHOICE when R_WAITING is set to 1
			- REWARD_GIVEN when NREWARDS increments
			- REWARD_RETRIEVED when R_WAITING is set to 0
	- bugfix: eliminate outdated use of fscanf to read data from file - use fgets to read line and then sscanf to safely deal with "-" or "." entries

v 9: 7.September.2012 [JRH]
	- change variable-detection so the "v_" prefix is not required - more compatible with older data files where vairable names might not have started with "v_"
		- note that in any case, when the .EVtcv file is made the variable names are all extracted from a particular column, regardless of the name of the variable
	- allow reward delivery to be detected by variables called either NREWARDS (new sytnax) or NPELLETS (old syntax)
	
v 7: 18.July.2012 [JRH]
	- now adjusts timestamps so that setstarttime becomes zero
		- critical for aligning events to other hardware using a sync signal

v 6: 9.July.2012 [JRH]
	- update to use new-format variable names v_NREWARDS (replaces v_NPELLETS) and v_R_WAITING (replaces v_RWAITING)
	- also v_TZ_IN and v_RZ_IN replace vTZ_IN and vRZ_IN
	
v 5: 20.April.2012 [JRH]
	- separate variables to track previous reqward and trigger zone status
	- indicate if zone departure is from the active TZ or RZ

*/


/* external functions start */
int xf_stats2_d(double *data, long n, int varcalc, double *result_d);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],word[MAXLINELEN],*matchstring=NULL,*pline,*pcol;
	long int i,j,k,n;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d; 
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */ 
	char tcvfile[256],trkfile[256];
	char prev_type[3];
	long ntrk=0;
	int bint,tz_active=-1,rz_active=-1,ntz=0,nrz=0,trk_timecol=0,tzidindex[TZMAX],rzidindex[RZMAX],tzcol[TZMAX],rzcol[RZMAX];
	int *trk_tzin=NULL,*trk_rzin=NULL,tz_in=0,rz_in=0,grp,bin,bintot,pellet_waiting=0;
	int prev_id_tz=-1,prev_id_rz=-1,npellets=0;
	double *trk_time=NULL,prev_time_tz=-1.0,prev_time_rz=-1.0,starttime=0.0;
	FILE *fptcv,*fptrk; 
	/* arguments */
	double setstarttime=0.0;

	sprintf(prev_type,"XX");

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Creates a comment (.cmt) file from a .EVtcv file\n");
		fprintf(stderr,"Program indicates key events such as zone entry/exit, reward etc.\n");
		fprintf(stderr,"Tracks changes in active trigger zone (TZ) or reward-zone (RZ)\n");
		fprintf(stderr,"Looks for changes in zone-occpancy (TZ_IN or RZ_IN)\n");
		fprintf(stderr,"Ignores repeated entries into the same zone\n");
		fprintf(stderr,"START_TRACK keyword used to re-zero times & ignore initializations\n");
		fprintf(stderr,"R_WAITING set to 1 results in CORRECT_CHOICE output\n");
		fprintf(stderr,"R_WAITING set to 0 results in REWARD_RETRIEVED output\n");
		fprintf(stderr,"NREWARDS keyword results in REWARD_DELIVERED output\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [EVtcv]\n",thisprog);
		fprintf(stderr,"	[EVtcv]: conditioned version of an Ethovision trial-control record\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-s: start time (-1 to use START_TRACK) [%g]\n",setstarttime);
		fprintf(stderr,"		Used to ignore variable initializations\n");
		fprintf(stderr,"		NOTE: timestamps are also aligned so this becomes zero\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s 028-120420_02.EVtcv -s 2.0\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	1st column: time\n");
		fprintf(stderr,"	2nd column: event\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(tcvfile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-s")==0) 	{ setstarttime=atof(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(setstarttime>=0) starttime=setstarttime;
	else starttime=0.0; // if the program is to look for START_TRACK keyword, starttime is set to 0.0 by default in case it is not found

	/***************************************************************************************************/
	/***************************************************************************************************/
	/* READ THE TRIAL CONTROL DATA */
	/***************************************************************************************************/
	/***************************************************************************************************/
	if((fptcv=fopen(tcvfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	n=0;w=1;
	while(fgets(line,MAXLINELEN,fptcv)!=NULL) { 
		if(sscanf(line,"%lf %s %f",&aa,&word,&b)!=3) continue;
		bint=(int)b;
		
		//printf("%g\t%s\t%g\n",aa,word,b);

		/* HAS TRACKING BEGUN ? START_TRACK KEYWORD, IF PRESENT, SHOULD ALWAYS BE FIRST LINE OF THE INPUT FILE */
		if(strstr(word,"START_TRACK")!=NULL && setstarttime<0.0) starttime=aa;

		/* DOES TZ_ACTIVE CHANGE ? */
		if(strstr(word,"TZ_ACTIVE")!=NULL) tz_active=bint;

		/* DOES RZ_ACTIVE CHANGE ? */
		if(strstr(word,"RZ_ACTIVE")!=NULL) rz_active=bint;

		/* CHECK IF TIME IS GREATER THAT SET START TIME */
		aa-=starttime;
		if(aa>0.0) {

			/* DOES SUBJECT ENTER A TRIGGER ZONE */
			if(strstr(word,"TZ_IN")!=NULL && bint>0 && bint!=prev_id_tz) { 
				if(bint==tz_active) { printf("%.6f\tENTER_TZ_%d_ACTIVE\n",aa,bint); tz_in=2;}
				if(bint!=tz_active) { printf("%.6f\tENTER_TZ_%d\n",aa,bint); tz_in=1;}
				prev_id_tz=bint;
				prev_id_rz=0;
				rz_in=0;
			} 
			/* DOES SUBJECT EXIT A TRIGGER ZONE ? */
			if(strstr(word,"TZ_IN")!=NULL && bint<1 && tz_in>0) { 
				if(tz_in==1) printf("%.6f\tEXIT_TZ_%d\n",aa,prev_id_tz); 
				if(tz_in==2) printf("%.6f\tEXIT_TZ_%d_ACTIVE\n",aa,prev_id_tz); 
				tz_in=0;
			} 
			/* DOES SUBJECT ENTER A REWARD ZONE ? */
			if(strstr(word,"RZ_IN")!=NULL && bint>0 && bint!=prev_id_rz) { 
				if(bint==rz_active) printf("%.6f\tENTER_RZ_%d_ACTIVE\n",aa,bint); 
				if(bint!=rz_active) printf("%.6f\tENTER_RZ_%d\n",aa,bint); 
				prev_id_rz=bint;
				prev_id_tz=0;
				rz_in=1;
				tz_in=0;
			}
			/* DOES SUBJECT EXIT A REWARD ZONE ? */
			if(strstr(word,"RZ_IN")!=NULL && bint==0  && rz_in>0) { 
				if(rz_in==1) printf("%.6f\tEXIT_RZ_%d\n",aa,prev_id_rz); 
				if(rz_in==2) printf("%.6f\tEXIT_RZ_%d_ACTIVE\n",aa,prev_id_rz); 
				rz_in=0;
			} 

			/* DOES SUBJECT ENTER THE CORRECT TRIGGER ZONE ? */
			if(strstr(word,"R_WAITING")!=NULL && bint==1) {
				printf("%.6f\tCORRECT_CHOICE\n",aa);
				tz_in=2;
			}
			/* WAS A REWARD DELIVERED ? */
			if(strstr(word,"NREWARDS")!=NULL) {
				printf("%.6f\tREWARD_GIVEN_%d\n",aa,bint);
				npellets=bint;
				rz_in=2;
			}
			/* WAS A REWARD RETRIEVED ? */
			if(strstr(word,"R_WAITING")!=NULL && bint==0) {
				printf("%.6f\tREWARD_RETRIEVED_%d\n",aa,npellets);
			}

		n++;
		}
	}
	fclose(fptcv);
	
	free(trk_time);
	free(trk_tzin);
	free(trk_rzin);

	exit(0);
	}

