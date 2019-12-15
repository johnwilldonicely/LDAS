/******************************************************************************
Identify runs through place field - all entrances and exits
This version of getruns does not filter any runs - it tags them for...
	- runstart (start time of run, seconds)
	- runend (end time of run, seconds)
	- runlen (cm)
	- runspikes (n)
	- rundir (mean angle)
	- rundirdelta (angular difference between entry and exit)
	- runcentre - does run include field centre? 0=no, 1=yes
	- runstop - does run include times when the rat violates the behavioural filter settings (eg. min speed)? 0=no, 1=yes
- filtered for run speed and direction (within tolerance)
- direction (if "360") is set to direction upon field entry
- runstart array idicates first position sample after field entry
- runend array indicates first position sample after field exit
- runlen (run length) includes paths between runstart and runend + path just prior to entry 
	(ie. runs span more than the field on both sides)

int postot		total number of position samples
float *pos_x		array of x-positions
float *pos_y		array of y-positions
float *pos_dir		array of heading values for each position sample
float *pos_vel		array of velocity values for each position sample
float *pos_avel		array of angular velocity values for each position sample (+ive = CCW -ive = CW)
float *pos_path		array of path-lengths for each position sample (how far has rat travelled since last sample)
int posinvalid		invalid position value
int dirinvalid		invalid direction value
int avelinvalid		invalid angular velocity value
float velmin		behavioural filter from calling function - velocity below which animal is said to have "stopped" (see "runstop")
int *maprate,		array indicating map firing rates
int *mapfield,		array indicating if firing rate bins are outside field (0), inside field (1), in field centre (2), or excluded (<0)
int mapbins,		width (bins) of firing rate map 
float mapbinsize		size of firing rate map bins (cm) 
int *runstart		result-array of pos sample numbers which will represent run starts 
int *runend		result-array of pos sample numbers which will represent run ends 
float *runlen		result-array of run length values 
float *rundir		result-array indicating direction at start of run (all the same if r_dirchoice != 360)
int *runcentre		result-array indicating if rat enters middle of field (1) or not (0) 
int *runmaxbin		result-array indicating the rate-map bin the rat runs through with the highest firing rate 
int *runstop		result-array indicating if rat stopped in field (1) or not (0) 
int max_runs		AMOUNT OF MEMORY RESERVED FOR RUNS ! 
******************************************************************************/
#include<math.h>
#include<stdio.h>
float hux_angleshift (float angle1, float angle2);
void hux_error(char message[]);
void hux_circmean(float* array,int arraysize,int missing,float *result);

int hux_getruns(
	int postot,float *pos_x,float *pos_y,float *pos_dir,float *pos_vel,float *pos_avel,float *pos_path,
	int posinvalid,int dirinvalid,int avelinvalid, float velmin,
	float *maprate, int *mapfield,int mapbins,float mapbinsize,
	int *runstart, int *runend, float *runlen, float *rundir, float *rundirsd, float *runavel, 
	int *runcentre, int *runmaxbin, int *runstop, int max_runs)
{
	char command[256];
	int i,w,xbin,ybin,start=0,run=-1,infield=0,maxbin=-1,badbehave=0,spoiltrun=0,c_pass=0,r_stop=0,r_samples=0;
	float runlentemp=0.0, runaveltemp=0.0,maxrate=-1.0,result[32];

	for(i=0;i<=postot;i++) {
		/* check for loss of tracking - if so treat as spoilt but still in-field
		- in this case run is aborted & nothing happens until another good sample is found 
		- first good sample will not be processed because infield will be -1 until the end of the loop iteration - then infield is set to 0 or 1
		- then, if good sample was infield, when rat exits again run will not be incrimented
		- in other words, when field is entered again, run stats will be overwritten
		*/
		if(pos_x[i]==posinvalid) {spoiltrun=1;infield=1;continue;} 

		/* get map index to bin */		
		ybin = (int) (pos_y[i]/mapbinsize); 
		xbin = (int) (pos_x[i]/mapbinsize); 
		w=ybin*mapbins+xbin;

		/* 1. was rat outfield and now infield? if so, start of run */
		if(infield==0 && mapfield[w]>0) {
			start=i;
			infield=1; c_pass=0; runlentemp=0.0; runaveltemp=0.0; r_stop=0; r_samples=0; spoiltrun=0; maxrate=-1.0; /* this is where previously spoilt runs inside field get reset */
		}
		/* 2. If rat was previously in field (or tracking was lost), or if rat has just entered field this iteration... */
		if(infield==1) {
			if(mapfield[w]>0) {	/* 2a. ...and still in field, then increment run length & samples */
				r_samples++;
				if(pos_path[i]!=posinvalid) runlentemp += pos_path[i]; /* this will include some distance from outside field */
				if(pos_avel[i]!=avelinvalid) runaveltemp += pos_avel[i];
				if(mapfield[w]>1) c_pass=1;
				if(maprate[w]>maxrate) {maxrate=maprate[w];maxbin=w;}
				if(pos_vel[i]<velmin) r_stop=1;
				
			}
			else if(mapfield[w]<=0) { /* 2b. ... or in field but now out of field, then end this run & store variables */
				infield=0;
				if(spoiltrun==0) { /* only actually update runs data if tracking was not lost */
					if(pos_path[i]!=posinvalid) runlentemp += pos_path[i];
					if(pos_avel[i]!=avelinvalid) runaveltemp += pos_avel[i];
					run++;
					if(run>max_runs) {sprintf(command,"hux_getruns: run count exceeds MAX_RUNS (%d)",max_runs);hux_error(command);}
					runstart[run]=start;
					runend[run]=i;
					runlen[run]=runlentemp;
					runcentre[run]=c_pass;
					runmaxbin[run]=maxbin;
					runstop[run]=r_stop;
					hux_circmean(pos_dir+start, r_samples, dirinvalid,result);
					rundir[run]=result[2];
					rundirsd[run]=result[3];
					runavel[run]=runaveltemp/(float)r_samples;
				}
			} /* end of if previously infield and now outfield */
		} /* end of if infield */
		if(mapfield[w]>0) infield=1;
		else infield=0;
	}/* end of for loop */
	run++;
	return(run);
}
