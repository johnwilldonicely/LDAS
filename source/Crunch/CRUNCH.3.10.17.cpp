#define THIS_PROG "CRUNCH.exe"
#define TITLE_STRING THIS_PROG" v 3.10.17: 3.June.2016 [JRH]"

/* 
TO DO:
	- switch to new xf_filter_bworth3 function
		- used for EEG so:
			1) ensure invalid points are detected in EEG (readEEG functions)
			2) if any detectd, apply interpolation
			3) call filter function


TO COMPILE:  
	 xs-progcompile CRUNCH.3.10.17.cpp -c g++ -f ./ -o /opt/LDAS/bin/

VERSION HISTORY
	v 3.10.17: 3.June.2016 [JRH]
		- retire use of .rc file, as the function is now served by custom invocation scripts 
		
	v 3.10.16: 1.December.2013 [JRH]
		- update butterworth filter function to xf_filter-bworth3_f
		- include interpolation step to remove invalid points

	v 3.10.15: 5.May.2013 [JRH]
		- remove all dependencies on old hux_sort functions within other functions (mullerize, linearstats)
		- these functions now call qsort directly
		- compare functions have been provided to support qsort instead
		- update mullerize functions and spatial information calculation ( no real numerical change to results) 
		
 	v 3.10.14: 26.March.2012 [JRH]
		- changed output headers to files for consistency and clarity - may cause issues for some programs
 
	v 3.10.13: 15.March.2012 [JRH]
	 	- removed some diagnostic comments I'd forgotten the purpose of 
	v 3.10.11: August.22.2011 [JRH]"
		- new history reporting
		- removed unused reference to hux_interpf
	3.10.9 (Jan. 26 2011) 
		- BUGFIX - related to previous bugfix - for linear data, must use different peak-detect criteria!
	3.10.9 (Jan. 24 2011) 
		- BUGFIX: expand map-peak requirement - must have at least 6 adjacent visited pixels
	3.10.8 (Jan. 21 2011) 
		- BUGFIX: map peaks now exclude pixels that are isolated or only connected by diagonals
		- this ensures that place fields are not single-bin-sized if rates in isolated bins are unusually high
	3.10.7 (Jan. 15 2011) 
		- BUGFIX: crunch_cells.txt output now indicates SPKS=0 (not RBASE=0) when a cluster has zero spikes
	3.10.6 (Jan. 3 2011) 
		- created a filetype 2.1 for .eegh files - no functional consequences yet
	3.10.5 (Jan. 1 2011) 
		- BUGFIX - check for maptype needed to cast absolute (eg. 2.1) as float
		- made changes to xf_psratemap as well - had been using clunky multiply-by-10 and compare as int method
	3.10.4 (Nov. 11 2010) 
		- changed EEG filter section and output - includes rms power calculation and improved efficiency
	3.10.3 (Nov.7.2010)
		- use new mapping functions xf_psratemap/xf_mullerize
		- map types are now 0 (heatmap) 1.0 (Muller) 1.1 (Muller heatmap) 2.0 (y-line) 2.1 (x-line)
	3.10.2
		- fixed summary statistic POS_IMMOBILITY to use data filetered using start/end criteria
	3.10.0
		- changed theta-fitting function to allow peak or trough detection (new function - xf-detectcycles)
		- removed arbitrary phase-adjust option -eegadjust - no longer needed given above
		- implimented new Butterworth filter xf_filter_bworth2
		- removed eeg-smooth option - replaced with low-pass Butterworth filtering
		- removed the half-fit option to only fit a half-sinusoid to the data - this was never used
	3.9.10
		- function definition now compaitble with xs-compile: xs-compile CRUNCH.cpp -f ../functions/ -o ./
		- remove hard exit if no valid position records in position input - just print warning instead
		- changed behaviour of -setverb variable: 1 prints to local file, 2 prints to screen
	3.9.8
		- bugfix: field is now defined before spike in-field status is determined!
		- bugfix: field size for data where x or y positions are collapsed is now correct
	3.9.7
		- added PLOT_CODE_START/END tags to postscript summary to permit pulling files out 
	3.9.6
		- fixed empty RUNS, FDWELL and FSPIKES columns
		- enhance and rename cells output: RBASE(10%) RMEAN(spikes/adjusted time) RPEAK (97.5%)
		- drop "DWELL" output in cells file - not very usefull
	3.9.5
		- revised position & eeg verbout output to standard KEY VALUE format
		- fixed bug in eeg cycles - segmentation fault if eeg cycles extend beyond limits of position record
	3.9.4
		- fixed longstanding miscalculation of fieldsize
			- previous versions included unvisited pixels due to smoothing
			- smoothing extended field beyond visited pixels
			- problem was worst for linear tracks
	3.9.3
		- changed to allow probe zero (no extension on .res files)
		- changed function listing to be compatible with xs-compile 
			- still not quite working though - need sub-functions defined too!
			
	3.8.3 October 8 2008
		* BUGFIX: for newer versions of Neuralynx the video resolution can now be read properly from the header file (Neuralynx changed the header format)
		* BUGFIX: for Neuralynx video files the LED can now be red, green or blue, or neither
		- introduced controls (for Neuralynx files only at present) for the number of LED's to be detected (-nleds) and which LED is the "principal" (-pled). This required addition of two more arguments for the hux_readpos_nlx.cpp funtion.
	3.8.2 August 19 2008
		- addition of -setburst variable to define minimum ISI for spike to be considered part of a burst (default 0.006)
	3.8.1 July 22 2008
		- addition of -spikefreq variable (Csicsvari files only) - sampling frequency for spike time files (.res)
	3.8.0 July 6 2008
		- calculation of ISIs , instantaneous firing rate and burstiness are made faster by ignoring clusters zero and (depending on the data format) one.
	3.7.9 July 5 2008
		* BUGFIX - further refinement of cycle detection and output
			- previously cycle start/end times were reconstructed from estimates of the cycle frequencies
			- this led to "impossible" frequencies when back-converting from start/stop times
			- cycle start and end times are now taken directly from a new hux_cyclefit function
			- crunch_eegcycles.txt now has filter tag in the tird column, not running speed
	3.7.8 July 3 2008
		- For Csicsvari position files (whl/whd), position values <0 are invalid and converted to -1
        - previously a "-2" would not be detected as invalid, causing an "out of camera range" error 
		* BUGFIX - corrected loss of cycles in the crunch_eegcycles.txt output 
			- this was due to a bug in the hux_ifreq function
			- instantaneous frequencies were not calculated if a cycle bordered an invalid cycle
			- cycle-times were only output if the cycle had a valid frequency
			- this has been corrected
	3.7.7 June 26 2008
		- CRUNCH now issues a warning if a spike file has no records, but does not quit with an error unless there are no valid spike files at all

	3.7.6 June 24 2008
		- introduced -mapfill option to output ratemaps with unvisited pixels left filled if smoothing does so
		- NOTE: should modify manual to state that field definition and runs are based on "filled" maps
	3.7.4 April 29 2008
		- introduced -efreq argument to specify eeg sampling frequency for Csicsvary eeg files
		- improved Csicsvari eeg file read speed by introducing block-reading
	3.7.2 April 12 2008
		* BUGFIX: crunch_pos.txt file header-line now begins with a "#"
	3.7.1 April 2 2008
		* BUGFIX: upper limit of detected oscillation range now properly implimented, so CRUNCH can detect different oscillations
	3.7 April 1 2008
		- can now output a dwelltime matrix even if there is no spike analysis
	3.6.8 January 25 2008
		- interspike intervals are dropped from the crunch_spike.txt output
		- in place of isi, a unique cell id number is included. This matches the cell id for each probe/cell combination in teh crunch_cells.txt file
	3.6.5 January 3 2008
		* BUGFIX: Fixed bug in coherence calculation which cut short the number of bins used in the calculation of the average of bin-rates around each bin if an unvisited bin was encountered partway through
		- modified line plot to ensure lines do not cross the bounding box
		- added plot type 2 (y-axis line plot) 
		- added stipulation that line plots can only be used in conjuction with -nox or -noy
			
			
TO DO:
	- change to read .clu file and reconstruct .res name, instead of vice versa!

	- add EEG filtering for Csicsvari & Axona files
	- alter tempbuf so it reads/writes exactly the right number of bytes
	- do this for both hux_readeeg_csicsvari and hux_filtereeg_csicsvari
- NOTE that for now pos_headdir and spike_headdir are output but only initialised to -1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h> /* must be included for directory reading */
#include <sys/stat.h> /* must be included for directory reading */
#include "neuralynx.h" /* Neuralynx header includes structure definitions */

#define PI 3.14159265358979323846
#define HALFPI 1.57079632679489661923
#define PIUNDER180 57.29577951308232087685
#define PIOVER180 0.01745329251994329577
#define MAXPROBES 32 /* max number of tetrodes */
#define MAXFILES MAXPROBES+3 /* one file per probe plus one each of video, EEG, and events */
#define MAXCLUSTERS 512 /* max number of clusters per probe: should be appropriate for all file types handled */
#define MAXRUNS 5000 /* max number of runs per cell */
#define EVENTSIZE 128 /* max size of an individual event record */

/* external functions start */
float hux_angleshift (float angle1, float angle2);
void hux_angletocart (float angle, float dist, float *result);
int hux_autocorr_array(double *time,int n,int *count, int bintot,int winsize,char *mode);
int hux_checkaxona (char *infile, float *result);
int hux_checknlx (char *posfile, float *result);	
int hux_checkcsicsvari (char *posfile, int channeltot, float *result);	
void hux_centroid (float *rate,	int *field, int bintot, float mapbinsize,float *result);
void hux_circmean(float* array, int arraysize, int missing, float *result);
int hux_correlate(float *x,float *y,int N,int invalid,float *result);
int hux_definefield(float *rate, int *field, int bintot, int maxbinx, int maxbiny, float fieldthresh, float peakthresh);
int hux_dejump (double *pos_time, float *pos_x, float *pos_y, int tot, int invalid, float jumpthresh);
void hux_error(char error_txt[]);
void hux_ratestats (float *dwell,float *rate,int bintot,float mapdwelltot,int ztransp,int maprateinvalid, float *result);
int hux_fillinterp(double *A_time,double *B_time,float *A_val,float *B_val,int Atot,int Btot,int invalid,char *type);
int hux_fillprev (double *A_time,double *B_time,float *A_val,float *B_val,int Atot,int Btot, int invalid);
int hux_fillprev_char (double *A_time,double *B_time, char *A_val, char *B_val,int Atot,int Btot,char invalid);
int hux_filtereeg_nlx (char *infile,char *outfile,double *pos_time,int postot,char *pos_filter);
int hux_getext(char *line);
int hux_getfilename(char *path);
int hux_getfileheader(FILE *fpin,char *header,char *header_stop,int headermax);
int hux_getpath(char *line);
int hux_getruns(int postot,float *pos_x,float *pos_y,float *pos_dir,float *pos_vel,float *pos_avel,float *pos_path,int posinvalid,int dirinvalid,int avelinvalid, float velmin,float *maprate, int *mapfield,int mapbins,float mapbinsize,int *runstart, int *runend, float *runlen, float *rundir, float *rundirsd, float *runavel,int *runcentre, int *runmaxbin, int *runstop, int max_runs);
int hux_getword(char *line,char *trigger, int word, char *target);
float hux_halfsinefit(float *data, int total);
float hux_heading (float delta_x, float delta_y);
void hux_irate (int cellid,long int spiketot,int *spike_id, double *spike_time, float set_burst, float *spike_isi,int *spike_burst,float *spike_irate);
void hux_linearstats (float *array,int arraysize,float missing,float *result);
int hux_posvel (double *pos_time, float *pos_x, float *pos_y,float *pos_vel,int postot,int winsize,char *style,int posinvalid,int velinvalid);
int hux_pospath (double *pos_time, float *pos_x, float *pos_y,float *pos_path,int postot,int winsize,char *style,int posinvalid,int pathinvalid);
float hux_prob_R(float R, int n);
void hux_pshistograms(double *time,int n,int probe,int cluster,char *path);
void hux_psratemap_assemble (unsigned int *spikecount,int clusterlimit,int probelimit,int probemax,float vidratio,float fontsize,int datatype,char *filebase1,char *filebase2,char *filebase3,int system,int mapcompact,int clean);
int hux_readeeg_axona (char *infile, double *eeg_time, float *eeg_val, float eegscale, float *result);
int hux_readeeg_csicsvari (char *infile, double *eeg_time, float *eeg_val, float eegscale, int channelchoice, int channeltot, float eegsampfreq, float *result);
int hux_readeeg_nlx (char *eegfile, double *eeg_time, float *eeg_val, float eegscale, float *result);
int hux_readevent_nlx (char *infile, double *event_time, char *event_label, float *result);
int hux_readpos_axona (char *posfile, double *pos_time, float *pos_x, float *pos_y, float videoresolution, float *result);
int hux_readpos_nlx (char *infile, int nleds, int ledcolour, double *pos_time, float *pos_x, float *pos_y,  float *pos_leddis, float vidres, float *result) ;
int hux_readpos_csicsvari_asc (char *infile, double *pos_time, float *pos_x, float *pos_y,  float *pos_headdir, float *pos_leddis, float vidres, int vidxmax, int vidymax, float posfreq, float *result) ;
int hux_readpos_xyd (char *infile, double *pos_time, float *pos_x, float *pos_y, float *pos_headdir, float *pos_leddis, float vidres, int vidxmax, int vidymax, int timebase, float *result) ;
int hux_readspike_nlx (char *spikefile[], int spikefiletot, int maxprobes, int maxclusters, double *spike_time, int *spike_probe, int *spike_clust, unsigned int *spikecount, int *probe_file_id);	
int hux_readspike_csicsvari_asc (char *spikefile[], int spikefiletot, int maxprobes, int maxclusters, double *spike_time, int *spike_probe, int *spike_clust, unsigned int *spikecount, int *probe_file_id, int *probe_maxcluster, float spikefreq);	
int hux_readspike_axona (char *spikefile[],int spikefiletot,int maxprobes,int maxclusters,double *spike_time,int *spike_probe,int *spike_clust,unsigned int *spikecount,int *probe_file_id);
void hux_smooth_float (float *original, float *smoothed, int arraysize, int windowsize, int invalid);
void hux_smooth_map (int *mapspikes, float *mapdwell, float *maprate,int mapbintot, int smooth,int smoothtype,int invalid);
int hux_substring(char *source, char *target, int casesensitive);
void hux_winpath(char *line);

int xf_detectcycles(int ndat,double *time,float *dat,float *phase,int *cstart,int *cend,float fmin,float fmax,float fitmax,int invalid,int method);
int xf_filter_bworth1_f(float *data, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
float xf_fitsine(float *data, int total, int alignment);
long xf_interp3_f(float *data, long ndata);
float xf_prob_F(float F,int df1,int df2);
void xf_psratemap(float maptype, float *map_rate, int width, float binsize, float peakrate, int peakx, int peaky, float map_scale, float vidratio, char *map_title, float fontsize, char *outfile);
void xf_mullerize_f(float *rate,int bintot,float *result);
int xf_compare1_f(const void *a, const void *b);
/* external functions end */

/**************************************************************************/


main (int argc, char *argv[])
{
/* position data */
char *posfile;
int posfiletot=0,postot=0,vidxmax=720,vidymax=576,headdirpresent=0,nleds=3,pled=1;
int set_filter=1,posinvalid=-1,posinvalidcount=0,velinvalid=-1,dirinvalid=-1,angleinvalid=-1,avelinvalid=999999;
int f_start=0,f_end=999999,f_dirchoice=360,f_dirtol=180,f_diroff;
float pos_centx=-1,pos_centy=-1,possmooth=0.4,pshiftt=0.0,pshiftd=0.0,posfreq=25.0,posinterval,posdur=0.0,jumpthresh=200,vidres=1.0,pinterp=0.4,vidratio;
float f_velmin=5.0,f_velmax=999999.0,f_avel=0; /* filter limits */
float posxorigin=0.0,posyorigin=0.0,posxlimit,posylimit,posxmin,posxmax,posymin,posymax,velmean=-1.0;
float *pos_x,*pos_y,*pos_leddis,*pos_vel,*pos_dir,*pos_headdir,*pos_angle,*pos_avel,*pos_path;
double *pos_time;
char *pos_filter, filterinvalid=99;

/* eeg data */
char *eegfile;
int eegfiletot=0,eegchannel=1,eegchantot=64,eegtot=0,cycletot=0,edetect=1;
int  *cycle_start,*cycle_end, eeginvalid=999999,phaseinvalid=-1;
float thetamin= 4.0, thetamax= 12, eeglowcut=2.0, eeghighcut=8;
float eegsampfreq=1250.0,eegdur=0.0,eegscale=1.0,eegfitmax=0.6;
float *eeg_val,*eeg_phase,*eeg_tfreq,*eeg_vel; 
double *eeg_time,eegsetstart=0.0,eegsetdur=3.0;
char *eeg_filter; /* eeg_filter holds position data for filtering EEG if necessary*/

/* spike data */
char *spikefile[MAXPROBES+1],dummy_filename[64],*spike_event;
unsigned int *spikecount;
int probe,cluster,cellid[MAXPROBES+1][MAXCLUSTERS+1],spikeinvalid=-1;
int *spike_probe,*spike_clust,*spike_id,*spike_field,*spike_burst,*spike_run;
int spikefiletot=0,spiketot=0,phasetot=0,filespikes[MAXPROBES+1],probe_goodspikes[MAXPROBES+1],probe_maxcluster[MAXPROBES+1],probe_file_id[MAXPROBES+1],clustart;
float *spike_x,*spike_y,*spike_isi,*spike_irate,*spike_dir,*spike_headdir,*spike_vel,*spike_avel,*spike_angle,*spike_phase,*phase,*spike_peakdis,*spike_peakangle,*spike_ratedis,*spike_rundis;
float spikedur=0.0,spikefreq=20000, setburst=0.006;
double *spike_time, *spike_runtime;
char *spike_filter;

/* temporary spike data for per-cluster calculations */
int spiketot2;
double *spike_time2;
float *spike_x2,*spike_y2;

/* map and per-cell variables */
char map_title[256];
int xbin,ybin,mappeakxbin=-1,mappeakybin=-1,mapbins=64,mapsmooth=7,mapsmoothtype=3,mapfill=0;
int mapmindwell=2,maprateinvalid=-1,mapthresh=10,mappeakzone=75,mapnums=0,mapcompact=1,clean=1;
int *mapspikes,*mapfield, mapfieldspikes=0,nox=0,noy=0;;
float *mapdwell,*maprate, maptype=0.0, mapfielddwell=0.0,duration=0.0;
float mapbinsize=2.5, maporigin_x=-1.0,maporigin_y=-1.0,mapdwelltot,mappeakx,mappeaky,mappeakangle,mapscale=0.16,mapbinscale=1.0,fontsize=20.0,set_mpr=-1;
float mapx,mapy,mapcoherence,mapinfo,mapsparsity,mapratebase=0.0,mapratepeak=-1.0,mapratemean=0.0,mapratesd=0.0,mapratemedian=0.0,maprateskew=0.0,maprate975=0.0,mapfieldsize=0.0;
float cellphasemean,cellphasesd,cellphasevector,cellphasesig,cellburstiness;

/* runs variables*/
int run=0,runtot=0,runstart[MAXRUNS],runend[MAXRUNS],runcentre[MAXRUNS],runmaxbin[MAXRUNS],runstop[MAXRUNS],runspikes[MAXRUNS];
float runlen[MAXRUNS],runvel[MAXRUNS],rundir[MAXRUNS],rundirsd[MAXRUNS],runavel[MAXRUNS];

/* event data */
char *eventfile;
int eventfiletot=0,eventtot=0;
double *event_time;
char *event_label;
char *event_filter;

/* Just for fun, output bitfield to determine output of program */
struct OutputBitField { /* define elements of the 32-bit "target" field (right to left) and size in bits */ 
		unsigned int irate : 1;
		unsigned int pos : 1;
		unsigned int eeg : 1;
		unsigned int spike : 1;
		unsigned int matrix: 1;
		unsigned int map : 1;
		unsigned int field : 1;
		unsigned int runs : 1;
		unsigned int isi : 1;
		unsigned int pad : 23;
} output;
output.eeg = output.field = 0;
output.irate = output.pos = output.spike = output.matrix = output.map = output.runs = output.isi = 0;

/* general file type variables */
int allfiletot=0;
char *tempfile[MAXFILES+1];
char path_prog[256],path_data[256],infile[256],infile1[256],infile2[256],outfile[256],verbout[256];
float result[32]; /* general purpose storage array for passing data between functions */
FILE *fpin,*fpin1,*fpin2,*fpout,*fpoutcells,*fpoutruns,*fpoutrate,*fpoutmatrix,*fpverbout;

/* general purpose variables */
char suffix[256],temp_str[256];
char *settingpath="/home/jhuxter/resources/readnlx/";
char *settingfile="readnlx_settings.txt";
char line[1000],*tempcol,*pline,*pcol;
long int i,j,k,l,m;
int w,x,y,z,n,N,skip,col,systemtype=1,datatype=2, trackingtype=1, setpause=0, setverb=2;
float a,b,c,d;
float *tempfa1,*tempfa2,*tempfa3; /* temp float arrays */
double aa,bb,cc,dd;

/*******************************************************************************
Determine program path, resource file and plot file names from first argument
*******************************************************************************/
strcpy(path_prog,*argv);			/* read full-path filename from 1st argument */
systemtype=hux_getpath(path_prog);	/* determine directory where program is and current system type */
if(systemtype<1) systemtype=0; /* presence of no slashes or forward slashes in path identifies file system as unix/linux */
else systemtype=1; /* presence of backslashes in path identifies file system as Windows */

/*******************************************************************************
Print instructions if only one argument
*******************************************************************************/
if(argc==1)
	{
	fprintf(stderr,"\n");
	fprintf(stderr,"--------------------------------------------------------------\n");
	fprintf(stderr,"%s\n",TITLE_STRING);
	fprintf(stderr,"\n");
	fprintf(stderr,"- Analyse EEG, position and spike data & generate place maps\n");
	fprintf(stderr,"- Drag files to program icon or include as command line arguments\n");
	fprintf(stderr,"- Default arguments are read from CRUNCH.rc file in program directory\n");
	fprintf(stderr,"- defaults can be overridden on command line or in batch file\n");
	fprintf(stderr,"- example...\n");
	fprintf(stderr,"	%s jc39-0207-0105.whl jc39-0207-0105.res.* -vidres 3.5\n",THIS_PROG);

	/* Print user-defined options */
	fprintf(stderr,"\nCRUNCH settings (defaults)...\n");
	fprintf(stderr,"--------------------------------------------------------------\n");
	fprintf(stderr,"-datatype %d -pause %d -clean %d -setverb %d -suffix %s ",
		datatype,setpause,clean,setverb,suffix);
	fprintf(stderr,"-vidres %.3f -vidxmax %d -vidymax %d -cx %.2f -cy %.2f -nox %d -noy %d -psmooth %.3f -pshiftt %.3f -pshiftd %.3f -pinterp %.3f -dejump %.2f -nleds %d -pled %d -posfreq %.4f -spikefreq %.4f -setburst %.3f ",
		vidres,vidxmax,vidymax,pos_centx,pos_centy,nox,noy,possmooth,pshiftt,pshiftd,pinterp,jumpthresh,nleds,pled,posfreq,spikefreq,setburst);
	fprintf(stderr,"-echannel %d -echantot %d -efreq %.4f -estart %.2f -edur %.2f -emin %.2f -emax %.2f -elowcut %.2f -ehighcut %.3f -efit %.2f -escale %.2f -edetect %d ",
		eegchannel,eegchantot,eegsampfreq,eegsetstart,eegsetdur,thetamin,thetamax,eeglowcut,eeghighcut,eegfitmax,eegscale,edetect);
	fprintf(stderr,"-mapbinsize %.2f -maptype %g -mapsmooth %d -mapsmtype %d -mapfill %d -mapmindwell %d -mapratepeak %.2f -mapthresh %d -mappeakzone %d -mapnums %d -mapcompact %d ",
		mapbinsize,maptype,mapsmooth,mapsmoothtype,mapfill,mapmindwell,set_mpr,mapthresh,mappeakzone,mapnums,mapcompact);
	fprintf(stderr,"-set_filter %d -f_start %d -f_end %d -f_velmin %.2f -f_velmax %.2f -f_dirchoice %d -f_dirtol %d -f_avel %.2f ",
		set_filter,f_start,f_end,f_velmin,f_velmax,f_dirchoice,f_dirtol,f_avel);
	fprintf(stderr,"-outpos %d -outeeg %d -outspikes %d -outmatrix %d -outmap %d -outfield %d -outruns %d -outisi %d ",
		output.pos,output.eeg,output.spike,output.matrix,output.map,output.field,output.runs,output.isi);
	fprintf(stderr,"\n");
	fprintf(stderr,"\n");
	if(systemtype==1) hux_error("");
	exit(0);
	}


/*******************************************************************************
READ COMMAND LINE ARGUMENTS & PRINT INSTRUCTIONS IF NECESSARY
- these arguments will overwrite any defaults previously set
- first arguments, NOT preceded by "-",  are presumed file names
	- classed as spike, eeg or video records depending on extention
- subsequent arguments beginning with "-" are treated as default overrides
*******************************************************************************/
*argv++; /* increment pointer to argument #2 */
/* read file names: arguments up to the first argument beginning with "-" */
for (x=1;x<argc;x++) {
	if(*argv[0]=='-') break;
	else {
		sprintf(path_data,"%s\0",*argv); hux_getpath(path_data); /* Designate path for output */
		if(allfiletot>=MAXFILES) {sprintf(temp_str,"%d input files exceeds maximum (%d)",allfiletot,MAXFILES); hux_error(temp_str);}
		else {tempfile[allfiletot]=*argv; allfiletot++; *argv++;}}
}

/* read command line (default-override) arguments: identifier starting with "-" followed by a value */
*argv--; /* wind back one argument, and start looking for commands */
if(argc-1>allfiletot) { // that is, if there are more arguments to be read...
	skip=0;
	z=x-1;
	for (x=z;x<argc-1;x++) {
		argv++;
		if(skip==0 && (*argv)[0]=='-') {skip=1;strcpy(line,*argv+1);continue;}
		if(skip==1)	{
			skip=0;
			if(strcmp(line,"datatype")==0) datatype=atoi(*argv);	
			else if(strcmp(line,"pause")==0) setpause=atoi(*argv);	
			else if(strcmp(line,"psmooth")==0) possmooth=atof(*argv);
			else if(strcmp(line,"pshiftt")==0) pshiftt=atof(*argv);
			else if(strcmp(line,"pshiftd")==0) pshiftd=atof(*argv);
			else if(strcmp(line,"pinterp")==0) pinterp=atof(*argv);
			else if(strcmp(line,"dejump")==0) jumpthresh=atof(*argv);
			else if(strcmp(line,"nleds")==0) nleds=atoi(*argv);
			else if(strcmp(line,"pled")==0) pled=atoi(*argv);
			else if(strcmp(line,"ehighcut")==0) eeghighcut=atof(*argv);
			else if(strcmp(line,"echannel")==0) eegchannel=atoi(*argv);
			else if(strcmp(line,"echantot")==0) eegchantot=atoi(*argv);
			else if(strcmp(line,"efreq")==0) eegsampfreq=(float)atof(*argv);
			else if(strcmp(line,"elowcut")==0) eeglowcut=(float)atof(*argv);
			else if(strcmp(line,"estart")==0) eegsetstart=(float)atof(*argv);
			else if(strcmp(line,"emin")==0) thetamin=(float)atof(*argv);
			else if(strcmp(line,"emax")==0) thetamax=(float)atof(*argv);
			else if(strcmp(line,"edur")==0) eegsetdur=(float)atof(*argv);
			else if(strcmp(line,"efit")==0) eegfitmax=(float)atof(*argv);
			else if(strcmp(line,"escale")==0) eegscale=(float)atof(*argv);
			else if(strcmp(line,"edetect")==0) edetect=atoi(*argv);
			else if(strcmp(line,"vidres")==0) vidres=(float)atof(*argv);
			else if(strcmp(line,"vidxmax")==0) vidxmax=atoi(*argv);
			else if(strcmp(line,"vidymax")==0) vidymax=atoi(*argv);
			else if(strcmp(line,"cx")==0) pos_centx=(float)atof(*argv);
			else if(strcmp(line,"cy")==0) pos_centy=(float)atof(*argv);
			else if(strcmp(line,"nox")==0) nox=atoi(*argv);
			else if(strcmp(line,"noy")==0) noy=atoi(*argv);
			else if(strcmp(line,"posfreq")==0) posfreq=(float)atof(*argv);
			else if(strcmp(line,"spikefreq")==0) spikefreq=(float)atof(*argv);
			else if(strcmp(line,"setburst")==0) setburst=(float)atof(*argv);
			else if(strcmp(line,"mapbinsize")==0) mapbinsize=(float)atof(*argv);
			else if(strcmp(line,"maptype")==0) maptype=atof(*argv);
			else if(strcmp(line,"mapsmooth")==0) mapsmooth=atoi(*argv);
			else if(strcmp(line,"mapsmtype")==0) mapsmoothtype=atoi(*argv);
			else if(strcmp(line,"mapfill")==0) mapfill=atoi(*argv);
			else if(strcmp(line,"mapmindwell")==0) mapmindwell=atoi(*argv);
			else if(strcmp(line,"mapratepeak")==0) set_mpr=(float)atof(*argv);
			else if(strcmp(line,"mapthresh")==0) mapthresh=atoi(*argv);
			else if(strcmp(line,"mappeakzone")==0) mappeakzone=atoi(*argv);
			else if(strcmp(line,"mapnums")==0) mapnums=atoi(*argv);
			else if(strcmp(line,"mapcompact")==0) mapcompact=atoi(*argv);
			else if(strcmp(line,"set_filter")==0) set_filter=atoi(*argv);
			else if(strcmp(line,"f_start")==0) f_start=atoi(*argv);
			else if(strcmp(line,"f_end")==0) f_end=atoi(*argv);
			else if(strcmp(line,"f_velmin")==0) f_velmin=(float)atof(*argv);
			else if(strcmp(line,"f_velmax")==0) f_velmax=(float)atof(*argv);
			else if(strcmp(line,"f_dirchoice")==0) f_dirchoice=atoi(*argv);
			else if(strcmp(line,"f_dirtol")==0) f_dirtol=atoi(*argv);
			else if(strcmp(line,"f_avel")==0) f_avel=(float)atof(*argv);
			else if(strcmp(line,"outpos")==0) {i=atoi(*argv); if(i==0||1==1) output.pos = i;}
			else if(strcmp(line,"outeeg")==0) {i=atoi(*argv); if(i==0||1==1) output.eeg = i;}
			else if(strcmp(line,"outspikes")==0) {i=atoi(*argv); if(i==0||1==1) output.spike = i;}
			else if(strcmp(line,"outmatrix")==0) {i=atoi(*argv); if(i==0||1==1) output.matrix = i;}
			else if(strcmp(line,"outmap")==0) {i=atoi(*argv); if(i==0||1==1) output.map = i;}
			else if(strcmp(line,"outfield")==0) {i=atoi(*argv); if(i==0||1==1) output.field = i;}
			else if(strcmp(line,"outruns")==0) {i=atoi(*argv); if(i==0||1==1) output.runs = i;}
			else if(strcmp(line,"outisi")==0) {i=atoi(*argv); if(i==0||1==1) output.isi = i;}
			else if(strcmp(line,"setverb")==0) setverb=atoi(*argv);
			else if(strcmp(line,"suffix")==0) strcpy(suffix,*argv);
			else if(strcmp(line,"clean")==0) clean=atoi(*argv);
			else {sprintf(temp_str,"command line contains an invalid argument,  \"%s\"",line); hux_error(temp_str);}
		}
	}
}

/* Determine whether diagnostic text goes to common file, local file, or screen */
if(setverb==1) {sprintf(verbout,"%scrunch_verbout.txt",path_data); fpverbout=fopen(verbout,"w");}
else if(setverb==2) fpverbout=stdout;
else {fprintf(stderr,"\n--- Error [%s]: invalid value for -setverb argument (%d)\n\n",THIS_PROG,setverb);exit(1);}
if(fpverbout==NULL){sprintf(temp_str,"can't open verbose output file %s",verbout);hux_error(temp_str);}

if(setverb>0) {
	fprintf(fpverbout,"\n");
	fprintf(fpverbout,"--------------------------------------------------------------\n");
	fprintf(fpverbout,"%s\n",TITLE_STRING);
}


/***********************************************************************
- read temporary input file neames
- check that they have valid file extentions according to data type
- assign each file a type (video, spikes, eeg)
- check that each file is a valid file of that type
- build spike file array
/***********************************************************************/
/* Check for valid specification of data type in .rc file */
if(datatype!=1&&datatype!=2&&datatype!=3) {sprintf(temp_str,"\"%d\" is not a valid data type: choose \"1\" (neuralynx) \"2\" (csicsvari) or \"3\" (Axona)",datatype); hux_error(temp_str);}
fprintf(fpverbout,"	Checking input files...\n");

sprintf(dummy_filename,"dummy\0");
for(i=0;i<=MAXPROBES;i++) spikefile[i]=dummy_filename; /* by default all spike filenames point to "dummy" filename */

for(i=0;i<allfiletot;i++) {
	if(setverb==2) printf("\t%s\n",tempfile[i]+hux_getfilename(tempfile[i]));
	if(datatype==1)			hux_checknlx(tempfile[i],result);
	else if(datatype==2)	hux_checkcsicsvari(tempfile[i],eegchantot,result);
	else if(datatype==3)	hux_checkaxona(tempfile[i],result);

	if(result[1]==0) fprintf(fpverbout,"--- Warning: file contains no records: %s",tempfile[i]);
	if(result[1]!=0 && result[0]>=1.0 && result[0]<2.0) {
		if(++posfiletot>1) hux_error("more than one video file specified"); 
		posfile=tempfile[i];
		postot=(int)result[1];
		if(result[0]==(float)1.0) {trackingtype=1;headdirpresent=0;} // .whl file
		if(result[0]==(float)1.5) {trackingtype=2;headdirpresent=1;} // .whd file
		if(result[0]==(float)1.7) {trackingtype=3;headdirpresent=1;} // .xyd file
	}
	if(result[1]!=0 && (int)result[0]==2) {
		if(++eegfiletot>1) hux_error("more than one eeg file specified"); 
		eegfile=tempfile[i];
		eegtot=(int)result[1];
	}
	if(result[1]!=0 && result[0]==5) {
		if(spikefiletot>=MAXPROBES) {
			sprintf(temp_str,"number of spike files exceeds maximum (%d)",MAXPROBES);
			hux_error(temp_str);
		}
		spikefile[spikefiletot]=tempfile[i]; /* filenames point to "tempfile" - not temp at all - memory must be preserved */
		filespikes[spikefiletot]=(int)result[1];
		spikefiletot++; 
		spiketot+=(int)result[1];
	}
}

if(posfiletot<1&&spikefiletot<1&&eegfiletot<1) hux_error("No position, EEG or spike files specified");

/* Declare data type & define first valid cluster number for data-type (omitting artefact and unsorted cluster */
if(datatype==1) {fprintf(fpverbout,"	Data format: Neuralynx\n");clustart=1;}
if(datatype==2) {fprintf(fpverbout,"	Data format: Csicsvari\n");clustart=2;}
if(datatype==3) {fprintf(fpverbout,"	Data format: Axona\n");clustart=1;}
fprintf(fpverbout,"	Data directory: %s\n",path_data);

// currently unused?
/* if origin-adjustment for rate maps is set to auto, set value as zero */
if(maporigin_x==-1) maporigin_x=0.0; 
if(maporigin_y==-1) maporigin_y=0.0;


/* CHECK FOR ILLEGAL VALUES, NUMBERS OF FILES OR DUPLICATION OF SPIKE FILE NAMES */
if(datatype==2&&posfreq==-1) {sprintf(temp_str,"Csicsvari files require definition of -posfreq");hux_error(temp_str);}
if(datatype==2 && (vidxmax==-1||vidymax==-1)) {sprintf(temp_str,"Csicsvari files require specification of -vidxmax and -vidymax"); hux_error(temp_str);}
if(pshiftt*pshiftd!=0) {sprintf(temp_str,"one of -pshiftt (%.3f) and -pshiftd (%.3f) must be zero",pshiftt,pshiftd);hux_error(temp_str);}
if(pshiftd!=0 && headdirpresent!=1) {sprintf(temp_str,"-pshiftd cannot be applied if head-direction data is unavailable");hux_error(temp_str);}
if(setpause!=0&&setpause!=1) {sprintf(temp_str,"-pause of %d is invalid: must be \"0\" or \"1\"",setpause);hux_error(temp_str);}
if(set_mpr<0&&set_mpr!=-1) hux_error("-mapratepeak must be positive or -1 (auto)");
if(nox!=0 && nox!=1) hux_error("-nox (collapse in x-dimension) must be 0 or 1");
if(noy!=0 && noy!=1) hux_error("-noy (collapse in y-dimension) must be 0 or 1");
if(nox==1&&noy==1) hux_error("-nox and -noy both set to 1: cannot collapse in both dimensions");
if(maptype==(float)2.0 && nox!=1) hux_error("for -maptype 2.0 (y-lineplot) -nox must be set to 1)");
if(maptype==(float)2.1 && noy!=1) hux_error("for -maptype 2.1 (x-lineplot) -noy must be set to 1)");
if(pos_centx<0&&pos_centx!=-1) hux_error("-cx (x-centroid definition) must be positive number or -1");
if(pos_centy<0&&pos_centy!=-1) hux_error("-cy (y-centroid definition) must be positive number or -1");
if(posfiletot>1) hux_error("More than one video tracker file specified");
if(eegfiletot>1) hux_error("More than one EEG file specified");
if(mapthresh<0||mapthresh>100) {sprintf(temp_str,"invalid -mapthresh (%d) - must be 0-100",mapthresh);hux_error(temp_str);}
if(mapthresh<0||mapthresh>100) {sprintf(temp_str,"invalid -mapthresh (%d) - must be 0-100",mapthresh);hux_error(temp_str);}
if(mapsmoothtype<1||mapsmoothtype>3) {sprintf(temp_str,"invalid -mapsmoothtype (%d) - must be 1-3",mapsmoothtype);hux_error(temp_str);}

z=(int)(maptype*10);
if(z!=0&&z!=10&&z!=11&&z!=20&&z!=21) 
	{fprintf(stderr,"MAPTYPE=%g\n",maptype); hux_error("Invalid maptype specified - should be 0.0 1.0 1.1 2.0 or 2.1");}
if(mapnums!=0&&mapnums!=1) hux_error("Invalid mapnums specified - should be 0 or 1");
if(mapfill!=0&&mapfill!=1) hux_error("Invalid mapfill specified - should be 0 or 1");
if(mapcompact!=0&&mapcompact!=1) hux_error("Invalid mapcompact specified - should be 0 or 1");
for(x=0;x<spikefiletot;x++) 
	for(y=x+1;y<spikefiletot;y++)
		if(strcmp(spikefile[x],spikefile[y])==0) hux_error("Duplicate spike record files specified");
if(f_dirchoice<0||f_dirchoice>360) {sprintf(temp_str,"f_dirchoice %d is invalid - chose 0-360",f_dirchoice); hux_error(temp_str);}
if(f_dirtol<0||f_dirtol>180) {sprintf(temp_str,"f_dirtol %d is invalid - chose 0-180 (180=any direction)",f_dirtol); hux_error(temp_str);}
if(clean!=0&&clean!=1) {sprintf(temp_str,"-clean %d is invalid - chose 1 (clean) or 0 (no)",clean); hux_error(temp_str);}

/* PRINT WARNINGS FOR LESS-SERIOUS CONFLICT OF SETTINGS */
if((int)maptype==1 && mapsmooth>0) {mapsmooth=0; fprintf(fpverbout,"--- Warning: -mapsmooth reset to zero for -maptype %g\n",maptype);}

if(eegfiletot<1 && output.eeg==1) fprintf(fpverbout,"--- Warning: Can't output EEG data if no EEG file is included\n");
if(posfiletot<1) {
	if(set_filter>0) {set_filter=0;fprintf(fpverbout,"--- Warning: Behavioural filtering requires a video file\n");}
	if(output.pos==1) {output.pos=0;fprintf(fpverbout,"--- Warning: Can't output position data if no video file is included\n");}
	if(output.map==1) {output.map=0;fprintf(fpverbout,"--- Warning: Can't output place maps if no video file is included\n");}
	if(output.field==1) {output.field=0;fprintf(fpverbout,"--- Warning: Can't output place field representations if no video file is included\n");}
	if(output.runs==1) {output.runs=0;fprintf(fpverbout,"--- Warning: Can't output runs through place fields if no video file is included\n");}
	if(output.matrix==1) {output.matrix=0;fprintf(fpverbout,"--- Warning: Can't output dwell/spike matrix if no video file is included\n");}
}
if(spikefiletot<1) {
	if(output.spike==1) {output.spike=0;fprintf(fpverbout,"--- Warning: Can't output spike data if no spike file is included\n");}
	if(output.map==1) {output.map=0;fprintf(fpverbout,"--- Warning: Can't output place maps if no spike file is included\n");}
	if(output.field==1) {output.field=0;fprintf(fpverbout,"--- Warning: Can't output place field representations if no spike file is included\n");}
	if(output.runs==1) {output.runs=0;fprintf(fpverbout,"--- Warning: Can't output runs through place fields if no spike file is included\n");}
	if(output.isi==1) {output.isi=0;fprintf(fpverbout,"--- Warning: Can't calculate isi, burstiness etc. if no spike file is included\n");}
}
if(output.runs==1&&mapsmooth<=0) fprintf(fpverbout,"--- Warning: runs analysis not recommended for unsmoothed maps\n");


/* PRINT FINAL USER-DEFINED OPTIONS */
fprintf(fpverbout,"\nUser settings...\n----------------\n");
fprintf(fpverbout,"-pause %d -clean %d -setverb %d -suffix %s ",setpause,clean,setverb,suffix);
fprintf(fpverbout,"-vidres %.3f -vidxmax %d -vidymax %d -cx %.2f -cy %.2f -nox %d -noy %d -psmooth %.3f -pshiftt %.3f -pshiftd %.3f -pinterp %.3f -dejump %.2f -nleds %d -pled %d -posfreq %.4f -spikefreq %.4f -setburst %.3f ",
	vidres,vidxmax,vidymax,pos_centx,pos_centy,nox,noy,possmooth,pshiftt,pshiftd,pinterp,jumpthresh,nleds,pled,posfreq,spikefreq,setburst);
fprintf(fpverbout,"-echannel %d -echantot %d -efreq %.4f -estart %.2f -edur %.2f -emin %.2f -emax %.2f -elowcut %.2f -ehighcut %.3f -efit %.2f -escale %.2f -edetect %d ",
	eegchannel,eegchantot,eegsampfreq,eegsetstart,eegsetdur,thetamin,thetamax,eeglowcut,eeghighcut,eegfitmax,eegscale,edetect);
fprintf(fpverbout,"-mapbinsize %.2f -maptype %g -mapsmooth %d -mapsmtype %d -mapfill %d -mapmindwell %d -mapratepeak %.2f -mapthresh %d -mappeakzone %d -mapnums %d -mapcompact %d ",
	mapbinsize,maptype,mapsmooth,mapsmoothtype,mapfill,mapmindwell,set_mpr,mapthresh,mappeakzone,mapnums,mapcompact);
fprintf(fpverbout,"-set_filter %d -f_start %d -f_end %d -f_velmin %.2f -f_velmax %.2f -f_dirchoice %d -f_dirtol %d -f_avel %.2f ",
	set_filter,f_start,f_end,f_velmin,f_velmax,f_dirchoice,f_dirtol,f_avel);
fprintf(fpverbout,"-outpos %d -outeeg %d -outspikes %d -outmatrix %d -outmap %d -outfield %d -outruns %d -outisi %d ",
	output.pos,output.eeg,output.spike,output.matrix,output.map,output.field,output.runs,output.isi);
fprintf(fpverbout,"\n");


/* PARSE SUFFIX STRING, REPLACING \T WITH TABS */ 
x=0;
for(i=0;i<=strlen(suffix);i++) {
	if(suffix[i]!='\\') y=suffix[i];
	else {
		i++; if(i>=strlen(suffix)) hux_error("-suffix should not end in backslash");
		if(suffix[i]=='t') y='	';
		else hux_error("-suffix accepts only \"\t\" as a control character");
	}
       temp_str[x]=y; x++;
}
for(i=0;i<=strlen(temp_str);i++) suffix[i]=temp_str[i];

fprintf(fpverbout,"\n");

/****************************************************************************
READ VIDEO DATA AND PROCESS POSITION INFO
- this calls routines to read Neuralynx data types
- however, calls to hux_checkpos_nlx and hux_readpos_nlx could be conditional
- other parts of this portion of program are input-type independent
****************************************************************************/
if(posfiletot<1) goto bookmark_EEG;
bookmark_pos:

fprintf(fpverbout,"\nReading video file \"%s\"  ...\n",posfile+hux_getfilename(posfile));

/* assign memory for position data arrays */
pos_time = (double *) calloc(postot+1,sizeof(double));
pos_x = (float *) calloc(postot+1,sizeof(float));
pos_y = (float *) calloc(postot+1,sizeof(float));
pos_leddis = (float *) calloc(postot+1,sizeof(float));
pos_vel = (float *) calloc(postot+1,sizeof(float));
pos_dir = (float *) calloc(postot+1,sizeof(float));
pos_headdir = (float *) malloc((postot+1)*sizeof(float));
pos_angle = (float *) calloc(postot+1,sizeof(float));
pos_avel = (float *) calloc(postot+1,sizeof(float));
pos_path = (float *) calloc(postot+1,sizeof(float));
pos_filter = (char *) calloc(postot+1,sizeof(char));
tempfa1 = (float *) calloc(postot+1,sizeof(float));
tempfa2 = (float *) calloc(postot+1,sizeof(float));
tempfa3 = (float *) calloc(postot+1,sizeof(float));
if(pos_time==NULL||pos_x==NULL||pos_y==NULL||pos_vel==NULL||pos_dir==NULL||pos_headdir==NULL||pos_angle==NULL||pos_avel==NULL||
pos_path==NULL||pos_filter==NULL||tempfa1==NULL||tempfa2==NULL){
	hux_error("not enough memory for video data");
}
/*initialise pos_headdir to -1, as this variable may not be available, depending on data type */
for(i=0;i<postot;i++) pos_headdir[i]=-1.0;

/* store video data */
/* NOTE: posfreq is calculated for Neuralynx files, but read from user input for Csicsvari files */
fprintf(fpverbout,"Storing & converting video data  ...\n");
if(datatype==1) {
	x=hux_readpos_nlx(posfile,nleds,pled,pos_time,pos_x,pos_y,pos_leddis,vidres,result);
}
else if(datatype==2) {
	// for csicsvari files, tracking types 1,2,3 = .whl, .whd, and .xyd respectively
	if(trackingtype==1||trackingtype==2) {
		x=hux_readpos_csicsvari_asc(posfile,pos_time,pos_x,pos_y,pos_headdir,pos_leddis,vidres,vidxmax,vidymax,posfreq,result);
	}
	if(trackingtype==3) {
		x=hux_readpos_xyd(posfile,pos_time,pos_x,pos_y,pos_headdir,pos_leddis,vidres,vidxmax,vidymax,(int)spikefreq,result);
		// now fill in variables which are not defined in the .xyd file but derived from user input
		result[1]=(float)vidxmax;
		result[2]=(float)vidymax;
		result[3]=posfreq;
		result[4]=(float)(1.0/posfreq);
}}
else if(datatype==3) {
	x=hux_readpos_axona(posfile,pos_time,pos_x,pos_y,vidres,result);
}

if(x==0) {fprintf(stderr,"--- Warning [%s]: no valid tracking points in position file\n\n",THIS_PROG);}

/* calculate aspect ratio and required scaling factor for maps */
postot= (int) result[0]; 	// total position samples, including "invalid" samples where tracking is lost
vidratio = result[1]/result[2];	// video camera aspect ratio x/y
posxlimit = result[1]/vidres; 	// theoretical maximum x-value in cm 
posylimit = result[2]/vidres;	// theoretical maximum y-value in cm
posfreq= result[3];
posinterval = result[4];
fprintf(fpverbout,"\tVID_SAMPLES %d\n",postot);
fprintf(fpverbout,"\tVID_SAMPLERATE %.2f Hz\n",posfreq);
fprintf(fpverbout,"\tVID_RESOLUTION %.2f\n",vidres);
fprintf(fpverbout,"\tVID_XMAX %g pixels %g cm\n",result[1],posxlimit);
fprintf(fpverbout,"\tVID_YMAX %g pixels %g cm\n",result[2],posylimit);
fprintf(fpverbout,"\tVID_MISTRACK %d\n",(int)result[5]);

/* make user-defined forward/backward correction for LED misplacement (only if headdirpresent==1)  */ 
if(pshiftd!=0) {
	for(i=0;i<postot;i++) {
		if(pos_headdir[i]!=dirinvalid) { // no adjustment if head direction is invalid (or, by default, x or y)
            hux_angletocart(pos_headdir[i],pshiftd,result); // calculate x/y translation for movement phiftd cm at pos_headdir degrees
            pos_x[i]+=result[0]; // apply correction to x-values
			pos_y[i]+=result[1]; // apply correction to y-values
		}}}

/* remove jumpy position records */
x=0; if(jumpthresh!=-1) { x = hux_dejump(pos_time,pos_x,pos_y,postot,posinvalid,jumpthresh); }
fprintf(fpverbout,"\tPOS_JUMPYPOINTS %d\n",x);

/* interpolate missing position samples */
/* doing this AFTER dejumping means there should be no ridiculous interpolation */
/* it also means that gaps in the record will be filled by straight lines */
if(pinterp!=-1) {
	for(i=0;i<postot;i++) {if(pos_x[i]!=posinvalid) {z=0;k=i;break;}} // go to first valid position record - set last-good-sample marker
	for(i=i+1;i<postot;i++) { /// now start from NEXT position sample
		if(pos_x[i]==posinvalid) {z++;continue;} // count number of invalid positions to fill
		if(z>0 && ( (pos_time[i]-pos_time[k]) < pinterp) ) { // only interpolate across appropriately small gap
			a=pos_x[k]; // starting x-pos
			b=pos_y[k]; // starting y-pos
			c = (pos_x[i]-a) / (z+1.0); // amount to incriment xpos each step
			d = (pos_y[i]-b) / (z+1.0); // amount to incriment ypos each step
			for(j=k+1;j<i;j++) {a+=c; pos_x[j]=a; b+=d; pos_y[j]=b;}
		}
		z=0;k=i;
}}
else fprintf(fpverbout,"\tposition interpolation disabled)\n");

/* Find video ranges and position centroid */
i=0;while(pos_x[i]==posinvalid) i++; /* find first valid position record */
posxmin = posxmax = pos_x[i]; /* initialize value for smallest and largest x position */
posymin = posymax = pos_y[i]; /* initialize value for smallest and largest y position */
posinvalidcount=0;
for(i=1;i<postot;i++) {
	if(pos_x[i]!=posinvalid){
		if(pos_x[i] > posxmax) posxmax = pos_x[i];
		if(pos_x[i] < posxmin) posxmin = pos_x[i];
		if(pos_y[i] > posymax) posymax = pos_y[i];
		if(pos_y[i] < posymin) posymin = pos_y[i];
	}
	else {
		posinvalidcount++;
}}

if(posxmax>posxlimit) {sprintf(temp_str,"x-position %g exceeds theoretical x-limit of %g",posxmax,posxlimit); hux_error(temp_str);} 
if(posymax>posylimit) {sprintf(temp_str,"y-position %g exceeds theoretical y-limit of %g",posymax,posylimit); hux_error(temp_str);} 

posdur =  pos_time[postot-1]-pos_time[0];
/* calculate centre of environment based on position range, unless centre value is preset  */
if(pos_centx==-1.0) pos_centx=posxmin+(posxmax-posxmin)/2.0; /* centroid calc for angular position */
if(pos_centy==-1.0) pos_centy=posymin+(posymax-posymin)/2.0; /* centroid calc for angular position */

a=100*(float)posinvalidcount/(float)postot;
fprintf(fpverbout,"\tPOS_INVALID %d (%.2f%%)\n",posinvalidcount,((float)posinvalidcount/(float)postot));
fprintf(fpverbout,"\tPOS_START %g seconds\n",pos_time[0]);
fprintf(fpverbout,"\tPOS_END %g seconds\n",pos_time[postot-1]);
fprintf(fpverbout,"\tPOS_XRANGE %g cm (%g to %g)\n",(posxmax-posxmin),posxmin,posxmax);
fprintf(fpverbout,"\tPOS_YRANGE %g cm (%g to %g)\n",(posymax-posymin),posymin,posymax);
fprintf(fpverbout,"\tPOS_XCENTROID %.1f\n",pos_centx);
fprintf(fpverbout,"\tPOS_YCENTROID %.1f\n",pos_centy);
if(headdirpresent==0) fprintf(fpverbout,"\tPOS_HEADDIR not_present\n");
if(headdirpresent==1) fprintf(fpverbout,"\tPOS_HEADDIR present\n");

/************************************************************************
- calculate direction, angular position and angular velocity
- use smoothing for direction and angular velocity
- smoothing should produce a full-size smoothing window of about 0.4s
- therefore, half-window is 0.2s, which at 25 Hz sampling = about 5 samples
************************************************************************/

/* initialise first two values - VERY IMPORTANT FOR LATER INTERPOLATION TO SPIKE ARRAY (hux_fillinterp)*/
pos_angle[0] = (float) angleinvalid; /* only first value for this variable, as it is not integrated*/
pos_vel[0] = pos_vel[1] = (float) velinvalid;
pos_dir[0] = pos_dir[1] = (float) dirinvalid;
pos_headdir[0] = pos_headdir[1] = (float) dirinvalid;
pos_avel[0] = pos_avel[1] = (float) avelinvalid;

/* convert possmooth from time(seconds) to samples */
x = (int) (possmooth*posfreq + 0.5); /* size of smoothing window in samples */
if(x%2==0) x++; /* add one if x is an even number */
fprintf(fpverbout,"\tPOS_SMOOTH %.3f seconds ( %d samples )\n",possmooth,x);
if(x<3) {possmooth=0.0; fprintf(fpverbout,"--- Warning: position smoothing cancelled: window is too small\n");}
else possmooth=(x-1.0)/2.0; /* integer result = size of half-window for smoothing function */

hux_smooth_float (pos_x,tempfa1,postot,(int)possmooth,posinvalid); /* create smoothed x-array, passing half-window size */
hux_smooth_float (pos_y,tempfa2,postot,(int)possmooth,posinvalid); /* create smoothed y-array, passing half-window size */
hux_pospath (pos_time,pos_x,pos_y,pos_path,postot,(int)(possmooth*2.0+1.0),"jump",posinvalid,posinvalid); /* calc. path lengths using end-to-end cord method */
hux_posvel (pos_time,pos_x,pos_y,pos_vel,postot,(int)(possmooth*2.0+1.0),"sliding",posinvalid,velinvalid); /* calc. velocity using overlapping cord method */

/* loop - calculate angular position (from unsmoothed data), angular velocity (from smoothed data), and heading (from smoothed data) */
for(i=1;i<postot;i++) { /* starts at one - no valid calc for most values until after first sample, except pos_angle, but we can live without the first value */
	y=i-1; /* calculate once to save time */
	pos_dir[i]=(float)dirinvalid; pos_angle[i]=(float)angleinvalid; pos_avel[i]=(float)avelinvalid; /* initialize to invalid */ 
	if (pos_x[i]!=posinvalid)  { /* following require current position value to be valid */
		pos_angle[i]=hux_heading((pos_x[i]-pos_centx),(pos_y[i]-pos_centy)); /* angular position value to keep */
		tempfa3[i]=hux_heading((tempfa1[i]-pos_centx),(tempfa2[i]-pos_centy)); /* smoothed angular position, to calc. pos_avel */
		if(pos_x[y]!=posinvalid) { /* the following require both current & previous raw positions to be valid, but are based on smoothed values */
			a = tempfa1[i] - tempfa1[y]; /* change in x */
			b = tempfa2[i] - tempfa2[y]; /* change in y */
			pos_avel[i] = hux_angleshift(tempfa3[y],tempfa3[i])/(pos_time[i]-pos_time[y]); /* angular vel. based indirectly on smoothed position */
			if(a!=0&&b!=0) pos_dir[i] = hux_heading(a,b); /* calculate heading, if position has changed  */
			else pos_dir[i] = pos_dir[y]; /* if position has not changed, heading is unchanged */
}}}


/* shift position along path by user-defined amount (or as far as first invalid sample) */
if(pshiftt>0) {
	for(i=0;i<postot;i++) {
		if(pos_x[i]!=posinvalid) {
			a=0; 
			for(j=i+1;j<postot;j++) {
               	if(pos_path[j]==posinvalid) break; 
				else a+=pos_path[j]; 
               	if(a>pshiftt) break;
			}
			pos_x[i] = pos_x[j];
			pos_y[i] = pos_y[j];
			pos_angle[i] = pos_angle[j];
		}}}
if(pshiftt<0) {
	for(i=postot-1;i>0;i--) {
		if(pos_x[i]!=posinvalid) {
			a=0; 
			for(j=i-1;j>=0;j--) {
				if(pos_path[j+1]==posinvalid) break; 
				else a-=pos_path[j+1]; 
				if(a<pshiftt) break;
			}
			pos_x[i] = pos_x[j];
			pos_y[i] = pos_y[j];
			pos_angle[i] = pos_angle[j];
		}}}


/*  BEHAVIOURAL FILTER MODULE - uses video data from above calculations
- define video data samples which pass filter criteria
- later, eeg phase values will be invalidated and spikes dumped to cluster zero on this basis */
z=0; 
if(set_filter==1) {
	fprintf(fpverbout,"\tImplimenting behavioural filter  ...\n");

	/* filter for first position sample must be set to "1" - impossible to gauge speed, etc.*/
	pos_filter[0]=1;

	/* Adjust start/end time filters to beginning of position record */
	f_start += (int) pos_time[0];
	f_end += (int) pos_time[0];
	for(i=1;i<postot;i++){
		/* calculate direction offset */
		if(f_dirchoice==360)f_diroff = 0;
		else if(pos_dir[i]==dirinvalid) f_diroff=f_dirtol+1;
		else f_diroff = (int) fabsf(hux_angleshift(pos_dir[i],(float)f_dirchoice));
		if (
			pos_x[i]==posinvalid ||
			pos_time[i]<f_start || pos_time[i]>f_end || pos_vel[i]<f_velmin || pos_vel[i]>f_velmax || pos_vel[i]==velinvalid ||	f_diroff>f_dirtol ||
			(f_avel!=0&&(pos_avel[i]==avelinvalid || pos_avel[i]*f_avel <0))	/* this condition met if avel and filter are of opposite sign */
			) {
				pos_filter[i]=1;
				posdur -= posinterval;
				z++;
			}
	}
	fprintf(fpverbout,"\t\tPOS_FAIL %d (%.1f %%) samples failed behavioural criteria \n",z,100*((float)z/(float)postot));
	fprintf(fpverbout,"\t\tPOS_DUR %.2f seconds (adjusted duration)\n",posdur);
}

duration=posdur; /* assign position-recording duration as overall duration of recording */

/* Calculate mean velocity (a), duration of immobility (b), and total path length (c)*/
a=c=d=0.0; aa=bb=0.0; x=y=0;
for(i=1;i<postot;i++){
	if(pos_filter[i]==0){ /* this will be true if either set_filter=0 or filter conditions were met */
		if(pos_vel[i]!=velinvalid) { /* must check for valid velocity because filter may not include velocity */
			x++; a += pos_vel[i]; /* velocity count and sum */
		}
		if(pos_path[i]!=posinvalid) c += pos_path[i]; // path length
	}
	/* determine cumulative time spent motionless or failing speed criterion */
	if((set_filter==1 && pos_vel[i]<=f_velmin && pos_time[i] >=f_start&&pos_time[i]<=f_end ) || (set_filter==0 && pos_vel[i] == 0)) {
		bb += (pos_time[i]-pos_time[i-1]);
	}
}
velmean = a/(float)x; /* mean velocity */
c = c/100.0; /* convert path length from cm to meters */
if(set_filter==1) fprintf(fpverbout,"\tPOS_IMMOBILITY %.2f seconds (speed < %.2f cm/s)\n",bb,f_velmin);
else fprintf(fpverbout,"\tPOS_IMMOBILITY %.2f seconds (speed = 0 cm/s)\n",bb);
fprintf(fpverbout,"\tPOS_VELOCITY %.2f cm/s\n",velmean);
fprintf(fpverbout,"\tPOS_PATHLENGTH %.2f metres\n",c);


/* COLLAPSE DATA TO ONE DIMENSION IF REQUIRED
NOTE!!!: this comes AFTER calculation of velocity BUT BEFORE assignment of spikes to positions and place map generation */
if(nox==1) for(i=0;i<postot;i++) if(pos_x[i]!=posinvalid) pos_x[i] = pos_x[i]/posxlimit;
if(noy==1) for(i=0;i<postot;i++) if(pos_y[i]!=posinvalid) pos_y[i] = pos_y[i]/posylimit;

/* DIAGNOSTIC PLOT  - to see smoothed data, copy smoothed temp float arrays to original first */
if(output.pos==1) {
	sprintf(outfile,"%scrunch_pos.txt\0",path_data);
	fprintf(fpverbout,"\tProducing position output file \"%s\"\n",outfile+hux_getfilename(outfile));
	fpout=fopen(outfile,"w");
	if(fpout==NULL) hux_error("can't save crunch_pos.txt - is the file open in another application?");

	fprintf(fpout,"#TIME	XPOS	YPOS	LEDDIST VEL DIR HDIR	ANGLE	AVEL	FILTER\n");
	for(i=0;i<postot;i++) 
		fprintf(fpout,"%.5f	%.3f	%.3f	%.3f	%.3f	%.3f	%.3f	%.3f	%.3f	%d\n",
			pos_time[i],pos_x[i],pos_y[i],pos_leddis[i],pos_vel[i],pos_dir[i],pos_headdir[i],pos_angle[i],pos_avel[i],pos_filter[i]);
	fclose(fpout);
}
free(tempfa1);
free(tempfa2);
free(tempfa3);

/* Prepare for generation of place firing maps */

/* set map scaling to the larger of either X- or Y-dimensions, to fit within 256x256 map grid */
if(posxlimit>posylimit) {mapbinscale= 256.0/posxlimit; mapbins= (int)(posxlimit/mapbinsize);}
else {mapbinscale= 256.0/posylimit; mapbins= (int)(posylimit/mapbinsize);}
fprintf(fpverbout,"\tScale position data by %.2f for 256x256 point postscript map\n",mapbinscale);

fprintf(fpverbout,"\tAllocating memory for %d x %d bin firing rate maps\n",mapbins,mapbins);
/* allocate memory for map variables */
mapdwell = (float *) calloc (mapbins*mapbins+1,sizeof(float)); /* dwelltime map */ 
mapfield = (int *) malloc ((mapbins*mapbins+1) * sizeof(int)); /* infield/outfield map */
mapspikes = (int *) malloc ((mapbins*mapbins+1) * sizeof(int)); /* spikecount map */
maprate = (float *) malloc ((mapbins*mapbins+1) * sizeof(float)); /* dwelltime-corrected rate map */
if(mapdwell==NULL || mapspikes==NULL || mapfield==NULL || maprate==NULL) hux_error("insufficent memory for map arrays");

/* create behaviourally filtered dwelltime map - only has to be done once */
for(i=0;i<postot;i++) {
	if(pos_x[i]==posinvalid||pos_filter[i]==1) continue;
	ybin = (int) (pos_y[i]/mapbinsize); 
	xbin = (int) (pos_x[i]/mapbinsize); 
	mapdwell[ybin*mapbins+xbin] ++;
}

/* filter for minimum number of samples and convert sample numbers to dwelltimes */
mapdwelltot=0.0;
for(ybin=0;ybin<mapbins;ybin++) for(xbin=0;xbin<mapbins;xbin++) {
	w=ybin*mapbins+xbin;
	if(mapdwell[w]<mapmindwell) mapdwell[w]=0;
	else {
		mapdwell[w] = mapdwell[w]/posfreq;
		mapdwelltot += mapdwell[w]; /* calculate total valid time in map */
	}
}
/* output dwelltime matrix */
if(output.matrix > 0) {
	sprintf(outfile,"%scrunch_matrix.txt\0",path_data);
	fprintf(fpverbout,"\tProducing matrix output file \"%s\"\n",outfile+hux_getfilename(outfile));
	fpoutmatrix=fopen(outfile,"w"); if(fpoutmatrix==NULL) {fprintf(fpverbout,"\n--- Error: can't open \"%s\"\n\n",outfile);exit(0);}
	fprintf(fpoutmatrix,"# Dwell map: %d bins wide\n",mapbins); 
	for(ybin=0;ybin<mapbins;ybin++) {
		fprintf(fpoutmatrix,"\n"); 
		for(xbin=0;xbin<mapbins;xbin++) {
			fprintf(fpoutmatrix,"%.4f ",mapdwell[ybin*mapbins+xbin]);
		}
	}
	fprintf(fpoutmatrix,"\n");
	fclose(fpoutmatrix);
}



/****************************************************************************
READ EEG DATA
- data is smoothed, highpass filtered, and theta-fit
- theta fitting is performed using negative zero-crossing as cycle start7
****************************************************************************/
bookmark_EEG:
if(eegfiletot<1) goto bookmark_event;

fprintf(fpverbout,"\nReading EEG file \"%s\"\n",eegfile+hux_getfilename(eegfile));

fprintf(fpverbout,"Allocating memory...");
/* allocate memory for eeg records */
eeg_time = (double *) malloc ((eegtot+1)*sizeof(double));
eeg_val = (float *) malloc ((eegtot+1)*sizeof(float));
eeg_phase = (float *) malloc ((eegtot+1)*sizeof(float));
eeg_vel = (float *) calloc((eegtot+1),sizeof(float));
eeg_filter = (char *) malloc ((eegtot+1)*sizeof(char));
tempfa1 = (float *) malloc ((eegtot+1)*sizeof(float));
cycle_start=(int *) malloc ((eegtot+1)*sizeof(int));
cycle_end=(int *) malloc ((eegtot+1)*sizeof(int));
if(eeg_time==NULL||eeg_val==NULL||eeg_phase==NULL||cycle_start==NULL||cycle_end==NULL||tempfa1==NULL) {hux_error("not enough memory for eeg data");}

fprintf(fpverbout,"Initialising variables...");
/* re-initialize phase values to invalid value - VERY IMPORTANT!*/
for(x=0;x<eegtot;x++) eeg_phase[x] = (float) phaseinvalid;

fprintf(fpverbout,"Storing data...\n");
/* read EEG file */ 
if(datatype==1) x = hux_readeeg_nlx(eegfile,eeg_time,eeg_val,eegscale,result);
else if(datatype==2) x = hux_readeeg_csicsvari(eegfile,eeg_time,eeg_val,eegscale,eegchannel,eegchantot,eegsampfreq,result); /* data for 1 channel only */
else if(datatype==3) x = hux_readeeg_axona(eegfile,eeg_time,eeg_val,eegscale,result); /* data for 1 channel only */

if(x==0) hux_error("Problem reading EEG data - no records?");	
eegtot =(int)result[0]*(int)result[1]; /* multiply number of records by number of channels  */
eegdur =  eeg_time[eegtot-1]-eeg_time[0];
eegsampfreq=result[2];
fprintf(fpverbout,"\t%d EEG records at %d samples/record = total %d samples\n",(int)result[0],(int)result[1],eegtot);
fprintf(fpverbout,"\tEEG_SAMPLERATE %.2f Hz\n",eegsampfreq);
fprintf(fpverbout,"\tEEG_START %.2f seconds\n",eeg_time[0]);
fprintf(fpverbout,"\tEEG_END %.2f seconds\n",eeg_time[eegtot-1]);
if(posfiletot<=0) duration=eegdur;

/* BUTTERWORTH FILTER TO REMOVE LOW & HIGH FREQUENCIES */
fprintf(fpverbout,"\tInterpolating and filtering EEG...\n");
// linear interpolation to remove NAN or INF values
xf_interp3_f(eeg_val,eegtot); 
// keep a copy of the original EEG
for(i=0;i<eegtot;i++) tempfa1[i]=eeg_val[i]; 
//butterworth_hp(eeg_val, eegtot, eegsampfreq, eeglowcut);
xf_filter_bworth1_f(eeg_val,(size_t)eegtot,eegsampfreq,eeglowcut,eeghighcut,1.0,line); 


/* FIT CYCLES TO FILTERED EEG - OUTPUT IS IN EEG_PHASE */
fprintf(fpverbout,"\tOscillation fitting...\n");
cycletot= xf_detectcycles(eegtot,eeg_time,eeg_val,eeg_phase,cycle_start,cycle_end,thetamin,thetamax,eegfitmax,phaseinvalid,edetect);
if(cycletot==-2) {fprintf(stderr,"--- Error [%s]: problem allocating memory for cycle start/end arrays",THIS_PROG);exit(1);}

/* COPY BEHAVIOURAL FILTER VALUES FOR EEG DATA */
if(set_filter==1) { // this can only happen if posfiletot > 0
    fprintf(fpverbout,"\tValidating EEG data based on behavioural filters...\n");
    z = hux_fillprev_char (pos_time, eeg_time, pos_filter, eeg_filter, postot, eegtot, filterinvalid);
}
/* ASSIGN VELOCITY VALUES TO EEG RECORDS FOR DIAGNOSTIC OUTPUT */
if(posfiletot > 0) z = hux_fillinterp(pos_time, eeg_time, pos_vel, eeg_vel, postot, eegtot, velinvalid,"linear");

/* CALCULATE CYCLE STATS AND OUTPUT, ALONG WITH FILTER INFORMATION */
sprintf(outfile,"%scrunch_eegcycles.txt\0",path_data);
if((fpout=fopen(outfile,"w"))==NULL) {sprintf(temp_str,"can't save %s - is the file open in another application?",outfile); hux_error(temp_str);}
fprintf(fpverbout,"\t\tWriting cycle times file \"%s\"\n",outfile+hux_getfilename(outfile));
fprintf(fpout,"#START	END	FREQ	RMS_POWER	FILTER\n");
j=0; z=0; cc=0.0; // j=poscounter, z="good" cycles, cc=cycle frequency average
for(i=0;i<cycletot;i++) {
	x=0; // marker for if the cycle is filtered
	j=cycle_start[i]; 
	k=cycle_end[i];
	
	// calculate momentary frequency
	aa=1.0/(eeg_time[k]-eeg_time[j]);
	// calculate root-meansquare power (rms) for each cycle, & check if cycle includes filtered data
	n=0;cc=0.0; for(l=j;l<k;l++) {cc+=(eeg_val[l]*eeg_val[l]);n++; if(eeg_filter[l]>=1) x=1;}; cc=sqrt(cc/(double)n);
	// output per-cycle stats to file
	fprintf(fpout,"%f	%f	%.3f	%f	%d\n",eeg_time[j],eeg_time[k],aa,cc,x);
	
	//calculate trial-averaged stats for unfiltered data
	if(x<=0) { z++; bb+=aa; dd+=cc; }
}
fclose(fpout);
bb=bb/(double)z; dd=dd/(double)z;

fprintf(fpverbout,"\t\tEEG_FREQBANDMIN %.2f Hz\n",thetamin);
fprintf(fpverbout,"\t\tEEG_FREQBANDMAX %.2f Hz\n",thetamax);
fprintf(fpverbout,"\t\tEEG_NCYCLES %d ( %d pass behav.criteria)\n",cycletot,z);
fprintf(fpverbout,"\t\tEEG_FREQMEAN %.4f Hz\n",bb);
fprintf(fpverbout,"\t\tEEG_POWERMEAN %f \n",dd);


/* INVALIDATE EEG VALUES AND PAHASES IF FILTER=1 */
if(set_filter==1) { // this can only happen if posfiletot > 0
	for(i=0;i<eegtot;i++) {
		if(eeg_filter[i]>=1) {
			eeg_val[i]=tempfa1[i]=(float)eeginvalid;
			eeg_phase[i]=(float)phaseinvalid;
}}}

if(output.eeg==1) {
	/* OUTPUT COMPLETE ASCII EEG TRACE */
	sprintf(outfile,"%scrunch_eeg.txt\0",path_data);
	fprintf(fpverbout,"\t\tWriting ASCII EEG record \"%s\"\n",outfile+hux_getfilename(outfile));
	if((fpout=fopen(outfile,"w"))==NULL) {sprintf(temp_str,"can't save %s - is the file open in another application?"); hux_error(temp_str);}
	fprintf(fpout,"#TIME	RAW	SMOOTH	PHASE\n");
	for(i=0;i<eegtot;i++) {
		if(eeg_filter[i]>=1) fprintf(fpout,"%f	-	-	-\n",eeg_time[i]);
		else fprintf(fpout,"%f	%.3f	%.3f	%.3f\n",
			eeg_time[i],tempfa1[i],eeg_val[i],eeg_phase[i]);
	}
	fclose(fpout);
}

/* DIAGNOSTIC PLOTS */

/* OUTPUT DIAGNOSTIC SAMPLE EEG TRACES AND THETA FITTING */
sprintf(outfile,"%scrunch_eegdiag.txt\0",path_data);
fprintf(fpverbout,"\t\tWriting diagnostic EEG-fit (%.2f-%.2f seconds) to file \"%s\"\n",
eegsetstart,(eegsetstart+eegsetdur),outfile+hux_getfilename(outfile));
eegsetstart=eegsetstart+eeg_time[0]; /* adjust start time to compensate for the fact that eeg times do not start at zero */
if(eegsetstart>eeg_time[eegtot-1]) hux_error("EEG output start time is beyond end of EEG record");
if((fpout=fopen(outfile,"w"))==NULL) {sprintf(temp_str,"can't save %s - is the file open in another application?"); hux_error(temp_str);}
fprintf(fpout,"#TIME	RAW SMOOTH	PHASE	TSPEED\n");
aa=eegsetstart+eegsetdur;
for(i=0;i<eegtot;i++){
	if(eeg_time[i]<eegsetstart) continue;
	if(eeg_time[i]>aa) break;
	if(eeg_filter[i]>=1) fprintf(fpout,"%f	-	-	-	%.3f\n", eeg_time[i],eeg_vel[i]);
	else fprintf(fpout,"%f\t%.2f\t%.2f\t%.2f\t%.2f\n", eeg_time[i],tempfa1[i],eeg_val[i],eeg_phase[i],eeg_vel[i]);
}
fclose(fpout);

/* CALCULATE AND OUTPUT AVERAGE DETECTED EEG WAVEFORM. NOTE: ONLY VALID CYCLES INCLUDED (I.E. ALL INVALID VOLTAGE VALUES EXCLUDED)	*/
int phaseN[360]; float phasesum[360],phasesqsum[360];
for(z=0;z<360;z++) {phasesum[z] = phasesqsum[z] = 0.0;	phaseN[z]=0;}
for(i=0;i<eegtot;i++) if(eeg_phase[i]!=phaseinvalid) {z=(int)eeg_phase[i];phaseN[z]++;phasesum[z]+=eeg_val[i];phasesqsum[z]+=eeg_val[i]*eeg_val[i];}
/* find min (a) and max (b) in waveform to determine theta amplitude */
a=999999.0;b=-999999.0; for(z=0;z<360;z++) if(phaseN[z]>=1) {c=phasesum[z]/(float)phaseN[z]; if(c<a) a=c; if(c>b) b=c;}
fprintf(fpverbout,"\t\tEEG_AMPLITUDE %.3f units, peak to trough\n",(b-a));
sprintf(outfile,"%scrunch_eegmean.txt\0",path_data);
fprintf(fpverbout,"\t\tWriting mean detected cycle file \"%s\"\n",outfile+hux_getfilename(outfile));
if((fpout=fopen(outfile,"w"))==NULL) {sprintf(temp_str,"can't save %s - is the file open in another application?"); hux_error(temp_str);}
fprintf(fpout,"#PHASE	N	MEAN	SEM\n");
for(z=0;z<360;z++) {
	fprintf(fpout,"%d	",z);
	if(phaseN[z]<1) {fprintf(fpout,"0	-	-\n"); continue;}
	fprintf(fpout,"%d	%f	",phaseN[z],(phasesum[z]/phaseN[z]));
	if(phaseN[z]<2) {fprintf(fpout,"0\n"); continue;}
	d = (phaseN[z]*phasesqsum[z]-(phasesum[z]*phasesum[z])) / (phaseN[z]*(phaseN[z]-1)); /* variance */
	fprintf(fpout,"%f\n",((sqrt(d))/sqrt((float)phaseN[z])) ); /* standard error of the mean */
}
fclose(fpout);

/* CREATE GNUPLOT COMMAND FILE TO BE EXECUTED AFTERWARDS TO PRODUCE DIAGNOSTIC OUTPUT (SAMPLE FITTING AND MEAN CYCLE) */
sprintf(outfile,"%scrunch_eeg.plt\0",path_data);
fprintf(fpverbout,"\t\tWriting EEG plot command file \"%s\"\n",outfile+hux_getfilename(outfile));
fpout=fopen(outfile,"w");
if(fpout==NULL) {sprintf(temp_str,"can't save EEG plot command file \n\t\t\"%s\"",outfile); hux_error(temp_str);}
fprintf(fpout,"set nokey\n");
fprintf(fpout,"set size 1,0.3\n");
fprintf(fpout,"set tics scale 0.5\n");
fprintf(fpout,"set xtics 0.5\n");
fprintf(fpout,"set mxtics 5\n");
fprintf(fpout,"set xlabel \"Time (s)\"\n");
fprintf(fpout,"set ylabel \"A/D Volts\"\n");
fprintf(fpout,"set term post color solid portrait \"Helvetica\" 8\n");
fprintf(fpout,"set output \"crunch_eegfit.ps\"\n");
fprintf(fpout,"plot \"crunch_eegdiag.txt\" using 1:2 w l lt 2 lw 1,  \"\" using 1:3 w l lt 1 lw 1, \"\" using 1:($4>=0?$3:1/0) w l lt 3 lw 2, \"\" using 1:($5*10) with lines lt 4, 0 lt -1 lw 0.5 \n");
fprintf(fpout,"set bar 0\n");
fprintf(fpout,"set xtics 45\n");
fprintf(fpout,"set xlabel \"Phase (degreees)\"\n");
fprintf(fpout,"set ylabel \"A/D Volts\"\n");
fprintf(fpout,"set term post color solid portrait \"Helvetica\" 8\n");
fprintf(fpout,"set output \"crunch_eegmean.ps\"\n");
fprintf(fpout,"plot \"crunch_eegmean.txt\" using 1:3:4 w yerrorbars pt 7 ps 0.3\n");
fprintf(fpout,"print \"\"\n");
fprintf(fpout,"print \"output sent to crunch_eegfit.ps and crunch_eegmean.ps\"\n\n");
fprintf(fpout,"print \"\"\n");
fclose(fpout);

/****************************************************************************
READ EVENT DATA
****************************************************************************/
bookmark_event:

/* set first event record data to time zero and "-" to fill spike records if no valid event records */
if(eventfiletot<1) goto bookmark_spikes;

fprintf(fpverbout,"Event file contains %d records\n",eventtot);
x=(eventtot+1)*EVENTSIZE;
fprintf(fpverbout,"Memory allocated for %d characters in all\n",x);

fprintf(fpverbout,"Allocating memory  ...\n");
/* allocate memory for event records */
event_time = (double *) malloc ((eventtot+1)*sizeof(double));
event_label = (char *) malloc ((eventtot+1)*EVENTSIZE*sizeof(char));
event_filter = (char *) malloc ((eventtot+1)*sizeof(char));
if(event_time==NULL||event_label==NULL||event_filter==NULL)	{
	hux_error("not enough memory for event data");
}

/* read event file */ 
fprintf(fpverbout,"Storing events  ...\n");
if(hux_readevent_nlx(eventfile,event_time,event_label,result)==0) hux_error("Problem reading event data - no records?");	
eventtot =(int)result[0];

fprintf(fpverbout,"\t%d event records read\n",eventtot);
a = event_time[eventtot-1] - event_time[0];
fprintf(fpverbout,"\tSpike time range %.0f to %.0f seconds\n",event_time[0],event_time[eventtot-1]);
fprintf(fpverbout,"\tSpike record duration = %.0f seconds = %.1f minutes\n",a,a/60);


/****************************************************************************
READ SPIKE DATA 
****************************************************************************/
bookmark_spikes:
if(spikefiletot<1) goto bookmark_end;

fprintf(fpverbout,"\nReading spike files (total=%d)\n",spikefiletot);
fprintf(fpverbout,"\tAllocating memory... ");

/* allocate memory for spike records */
spike_time = (double *) malloc ((spiketot+1)*sizeof(double)); if(spike_time==NULL) hux_error("not enough memory for spike data");
spike_time2 = (double *) malloc ((spiketot+1)*sizeof(double)); if(spike_time2==NULL) hux_error("not enough memory for spike data");
spike_probe = (int *) malloc ((spiketot+1)*sizeof(int)); if(spike_probe==NULL) hux_error("not enough memory for spike data");
spike_clust = (int *) malloc ((spiketot+1)*sizeof(int)); if(spike_clust==NULL) hux_error("not enough memory for spike data");
spike_id = (int *) malloc ((spiketot+1)*sizeof(int)); if(spike_id==NULL) hux_error("not enough memory for spike data");
spike_isi = (float *) calloc ((spiketot+1),sizeof(float)); if(spike_isi==NULL) hux_error("not enough memory for spike data");
spike_burst = (int *) calloc ((spiketot+1),sizeof(int)); if(spike_burst==NULL) hux_error("not enough memory for spike data");
spike_irate = (float *) calloc ((spiketot+1),sizeof(float)); if(spike_irate==NULL) hux_error("not enough memory for spike data");
spikecount = (unsigned int *) calloc(MAXPROBES*MAXCLUSTERS+1,sizeof(unsigned int)); if(spikecount==NULL) hux_error("not enough memory for spike data");
tempfa2 = (float *) malloc ((spiketot+1)*sizeof(float));  if(tempfa2==NULL) hux_error("not enough memory for spike data");
tempfa3 = (float *) malloc ((spiketot+1)*sizeof(float));  if(tempfa3==NULL) hux_error("not enough memory for spike data");

if(posfiletot>0){
	spike_x = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_x==NULL) hux_error("not enough memory for spike data");
	spike_y = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_y==NULL) hux_error("not enough memory for spike data");
	spike_x2 = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_x2==NULL) hux_error("not enough memory for spike data");
	spike_y2 = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_y2==NULL) hux_error("not enough memory for spike data");
	spike_angle = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_angle==NULL) hux_error("not enough memory for spike data");
	spike_dir = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_dir==NULL) hux_error("not enough memory for spike data");
	spike_headdir = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_headdir==NULL) hux_error("not enough memory for spike data");
	spike_vel = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_vel==NULL) hux_error("not enough memory for spike data");
	spike_avel = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_avel==NULL) hux_error("not enough memory for spike data");
	spike_field = (int *) malloc ((spiketot+1)*sizeof(int)); if(spike_field==NULL) hux_error("not enough memory for spike data");
	spike_run = (int *) malloc ((spiketot+1)*sizeof(int)); if(spike_run==NULL) hux_error("not enough memory for spike data");
	spike_runtime = (double *) malloc ((spiketot+1)*sizeof(double)); if(spike_runtime==NULL) hux_error("not enough memory for spike data");
	spike_rundis = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_rundis==NULL) hux_error("not enough memory for spike data");
	spike_peakdis = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_peakdis==NULL) hux_error("not enough memory for spike data");
	spike_peakangle = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_peakangle==NULL) hux_error("not enough memory for spike data");
	spike_ratedis = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_ratedis==NULL) hux_error("not enough memory for spike data");
	spike_filter = (char *) malloc ((spiketot+1)*sizeof(char)); if(spike_filter==NULL) hux_error("not enough memory for spike data");
}
if(eegfiletot>0) {
    spike_phase = (float *) malloc ((spiketot+1)*sizeof(float)); if(spike_phase==NULL) hux_error("not enough memory for spike data");
    phase = (float *) malloc ((spiketot+1)*sizeof(float));if(phase==NULL) hux_error("not enough memory for spike data");
}
if(eventfiletot>0) {
	spike_event = (char *) malloc ((spiketot+1)*EVENTSIZE*sizeof(char)); if(spike_event==NULL) hux_error("not enough memory for spike data");
}

/* Initialise probe-stats and spike file IDs - will be set when the spike file is actually read */
/* probe_file_id[x] is the probe number for the xth spike file */

for(i=0;i<=MAXPROBES;i++) {
	probe_maxcluster[i] = 0;	/* largest probe number amongst spike files sent to program */
	probe_goodspikes[i]=0;		/* number of spikes, not including unclustered spikes */
	probe_file_id[i]=MAXPROBES; /* NOTE: probe_file_id is zero-offset so MAXPROBES is a known invalid probe number */
	for(j=0;j<=MAXCLUSTERS;j++) cellid[i][j]=-1;
}

/* read spike records and put data into arrays */
fprintf(fpverbout,"Storing spike data...\n");
if	(datatype==1) {spiketot = hux_readspike_nlx(spikefile,spikefiletot,MAXPROBES,MAXCLUSTERS,spike_time,spike_probe,spike_clust,spikecount,probe_file_id);}
else if	(datatype==2) {spiketot = hux_readspike_csicsvari_asc(spikefile,spikefiletot,MAXPROBES,MAXCLUSTERS,spike_time,spike_probe,spike_clust,spikecount,probe_file_id,probe_maxcluster,spikefreq);}
else if	(datatype==3) {spiketot = hux_readspike_axona(spikefile,spikefiletot,MAXPROBES,MAXCLUSTERS,spike_time,spike_probe,spike_clust,spikecount,probe_file_id);}
fprintf(fpverbout,"\tTotal spikes: %d\n",spiketot);

spikedur=spike_time[spiketot-1]-spike_time[0];
if(posfiletot<=0&&eegfiletot<=0) duration=spikedur;

/*******************************************************************************
- SPIKE DATA PROCESSING
- insert data based on position, eeg, & events into spike data structure
- produce spike.txt file (one line per spie)
- calculate place field statistics, if position data is included
- print cell summary to screen
********************************************************************************/
bookmark_processing:
/* add up clustered spikes and maximum cluster number in each probe */
x=0; 
for(probe=0;probe<MAXPROBES;probe++) {
	for(cluster=clustart;cluster<MAXCLUSTERS;cluster++) {
		x=spikecount[probe*MAXCLUSTERS+cluster];
		probe_goodspikes[probe]+=x;
		if(x>0 && cluster>probe_maxcluster[probe]) probe_maxcluster[probe]=cluster; /* this should never happen for Csicsvari files */
}}

/* assign a unique cell-id to each probe and cluster */ 
i=clustart; for(probe=0;probe<MAXPROBES;probe++) {
	if(probe_goodspikes[probe]<1) {for(cluster=clustart;cluster<=probe_maxcluster[probe];cluster++) i++; continue;}
	else 	{for(cluster=clustart;cluster<=probe_maxcluster[probe];cluster++) cellid[probe][cluster]=i++;}
}
for(i=0;i<spiketot;i++) spike_id[i]= cellid[spike_probe[i]][spike_clust[i]];

/* Add total clustered spikes & check for invalid probe number (zero) if Csicsvari data type */
z=0; for(probe=0;probe<MAXPROBES;probe++) {
	z+=probe_goodspikes[probe];
//	if(datatype==2&&probe==0&&z>0) hux_error("Csicsvari data should have no probe-zero. Check for correct spike file extentions (.1 to .32)");
}
if(z<=0) {
	sprintf(temp_str,"	--- Warning: no clustered spikes in any probes: no cell output will be produced\n");
	if(setverb!=2) printf(temp_str); 
	fprintf(fpverbout,temp_str);
	spikefiletot=0;
	goto bookmark_end;
}

if(spikefiletot<1) goto bookmark_end;

/* Calculate instantaneous firing rate (irate), interspike intervals (ISI), and presence of burst firing */
/* irate calculated using using "2xtheta" averaging window (8Hz, total 0.250 ms) */
/* NOTE: this MUST be done before any reassignment of spikes to cluster zero due to behavioural filter */
/* NOTE: cell burstiness is calculated post-behavioural-filtering */
if(output.isi==1) {
	fprintf(fpverbout,"\tCalculating instantaneous firing rate...\n");
	for(probe=0;probe<MAXPROBES;probe++){
		if(filespikes[probe_file_id[probe]]<=0) continue;
		for(cluster=clustart;cluster<=probe_maxcluster[probe];cluster++) {
			if(spikecount[probe*MAXCLUSTERS+cluster]>0){
				if(cellid[probe][cluster]>=0) hux_irate(cellid[probe][cluster],spiketot,spike_id,spike_time,setburst,spike_isi,spike_burst,spike_irate);
}}}}


/* incorporate theta phase values */
if(eegfiletot>0) z = hux_fillprev (eeg_time, spike_time, eeg_phase, spike_phase, eegtot, spiketot, phaseinvalid);

if(posfiletot>0) {
	/* Incorporate position & movement data into spike arrays */
	z = hux_fillinterp (pos_time,spike_time,pos_x,spike_x,postot,spiketot,posinvalid,"linear");
	z = hux_fillinterp (pos_time,spike_time,pos_y,spike_y,postot,spiketot,posinvalid,"linear");
	z = hux_fillinterp (pos_time,spike_time,pos_angle,spike_angle,postot,spiketot,angleinvalid,"circular");
	z = hux_fillinterp (pos_time,spike_time,pos_vel,spike_vel,postot,spiketot,velinvalid,"linear");
	z = hux_fillinterp (pos_time,spike_time,pos_dir,spike_dir,postot,spiketot,posinvalid,"linear");
	z = hux_fillinterp (pos_time,spike_time,pos_headdir,spike_headdir,postot,spiketot,posinvalid,"linear");
	z = hux_fillinterp (pos_time,spike_time,pos_avel,spike_avel,postot,spiketot,avelinvalid,"linear");
	/* copy behavioural filter settings (pos_filter, 0=ok, 1=filter) to spike_filter - based on last position record before each spike */
	z = hux_fillprev_char (pos_time,spike_time,pos_filter,spike_filter,postot,spiketot,1);

	/* if required, assign cluster to zero if spike occurs during "bad behaviour" */
	/* NOTE that due to interpolation (above), spikes may be included which have speed, dir (etc) values which violate filter settings */
	if(set_filter==1) {
		fprintf(fpverbout,"\tApplying behavioural filter to spikes\n");
		x=0;
		for(i=0;i<spiketot;i++) {
			if(spike_filter[i]>=1 && spike_clust[i]!=0) {
				spikecount[spike_probe[i]*MAXCLUSTERS+spike_clust[i]]-=1;
				spikecount[spike_probe[i]*MAXCLUSTERS]+=1;
				spike_clust[i]=0;
				x++; 
			} 
		}
		a=100*(float)posinvalidcount/(float)postot;

	fprintf(fpverbout,"\t%d spikes (%.1f %c) failed behavioural criteria: reset to cluster 0\n",x,100*((float)x/(float)spiketot),37);
	}
}
fprintf(fpverbout,"\n");


/*******************************************************************************************************
Calculate per-cell field and spike statistics
	- unique place cell ids
	- place fields, firing patterns, phase data
	- note that cell summary output is default and non-optional
********************************************************************************************************/
sprintf(outfile,"%scrunch_cells.txt\0",path_data);
fprintf(fpverbout,"Producing cell stats output file \"%s\"\n",outfile+hux_getfilename(outfile));
fpoutcells=fopen(outfile,"w");
if(fpoutcells==NULL) hux_error("can't save crunch_cells.txt - is the file open in another application?");
fprintf(fpoutcells,"#PROBE	CLUST	ID	BURST	SPKS	RBASE	RMEAN	RPEAK	FDWELL	FSPKS	FSIZE	RUNS	X	Y	ANGLE	INFO	SPARS	COH	PHASE	VECT	PROB	CODES");

if(output.runs==1) {
	sprintf(outfile,"%scrunch_runs.txt\0",path_data);
	fprintf(fpverbout,"Producing runs output file \"%s\"\n",outfile+hux_getfilename(outfile));
	fpoutruns=fopen(outfile,"w");
	if(fpoutruns==NULL) {sprintf(temp_str,"can't open output file \"%s\"",outfile+hux_getfilename(outfile)); hux_error(temp_str);}
	fprintf(fpoutruns,"#PROBE	CLUST	RUN	START	END	DUR	LEN	VEL	DIR	DIRSD	AVEL	SPKS	RATE	CENTRE	MAXBIN	STOP	CODES\n");
}

/*******************************************************************
PER-PROBE LOOP BEGINS HERE 
********************************************************************/
N=mapbins*mapbins;
fprintf(fpverbout,"Calculating per-cell statistics...\n",x);
/* Re-open matrix file to append results for rate maps to dwell map */
if(output.matrix>0) {
	sprintf(outfile,"%scrunch_matrix.txt\0",path_data);
	fpoutmatrix=fopen(outfile,"a"); if(fpoutmatrix==NULL) {fprintf(fpverbout,"\n--- Error: can't open \"%s\"\n\n",outfile);exit(0);}
	for(i=0;i<spiketot;i++) { /* initialise spike variables */
		spike_field[i] = -1;
		if(output.runs==1) { spike_run[i] = -1; spike_runtime[i] = -1.0;	spike_rundis[i] = -1.0; }
		spike_peakdis[i] = (float)avelinvalid; /*typically 999999*/ 
		spike_peakangle[i] = -1.0; 
		spike_ratedis[i] = -1.0; 
	}
}
for(probe=0;probe<MAXPROBES;probe++) {
	if(strcmp(spikefile[probe_file_id[probe]],dummy_filename)==0) continue; // if there was no spike file in the first place...
	fprintf(fpoutcells,"\n");
	if(probe_goodspikes[probe]<1) {
		fprintf(fpverbout,"\tprobe %02d has no clustered spikes\n",probe);
		fprintf(fpoutcells,"# probe %02d has no clustered spikes\n",probe);
		continue;
    }
	else fprintf(fpverbout,"	probe %02d cluster    ",probe);
	/* PER-CLUSTER LOOP BEGINS HERE */
	/* determine start cluster for analysis */
	for(cluster=clustart;cluster<=probe_maxcluster[probe];cluster++) { 
		if(spikecount[probe*MAXCLUSTERS+cluster]<=0) {
			fprintf(fpoutcells,"%02d\t%02d\t%04d\t-\t0\t-\t-\t-\t-\t-\t-\t-\t-\t-\t-\t-\t-\t-\t-\t-\t-\t%s\n",probe,cluster,cellid[probe][cluster],suffix);
			continue;
		}
		fprintf(fpverbout,"\b\b\b%03d",cluster);

		/*initialise values - this is important especially if some data types are missing */
		mapx=mapy=cellburstiness=-1;
		cellphasemean = cellphasesd = cellphasevector = cellphasesig = -1.0;
		mapinfo = mapsparsity = mapcoherence = mapratepeak = mappeakx = mappeaky = mappeakangle = mapfieldsize = -1.0;
		mappeakxbin = mappeakybin = -1;
		runtot=-1;

		/* Create temporary arrays for maps, & phase - NOTE: spikes already filtered for behaviour - can't calculate ISIs here */
		/* Calculate burstiness while we're at it */
		y=z=0;
		for(i=0;i<spiketot;i++) {
			if(spike_probe[i]==probe && spike_clust[i]==cluster) {
				if(spike_burst[i]>0) y++; /* Incriment bursty spike counter */
				spike_time2[z]=spike_time[i];
				if(posfiletot>0) {spike_x2[z]=spike_x[i];spike_y2[z]=spike_y[i];}
				if(eegfiletot>0) tempfa3[z]=spike_phase[i];
				z++; /*incriment spikes-in-cluster counter */
			}
		}
		spiketot2=z;
		if(y>0) cellburstiness = (float)y/(float)z;
		else cellburstiness = 0;

		/* Produce postscript ISI and autocorrelation histograms */
		if(output.map==1) hux_pshistograms(spike_time2,spiketot2,probe,cluster,path_data);

		/* calculate mean EEG firing phase from temp spike arrays - NOTE: this is not affected by runs analysis */
		if(eegfiletot>0) {
			hux_circmean(tempfa3,spiketot2,phaseinvalid,result); 
			cellphasemean=result[2]; cellphasesd=result[3];	cellphasevector=result[1]; cellphasesig=result[5];
		}
		/* Create firing rate map and runs through field - these map-related procedures must be done before runs analysis */
		if(posfiletot>0) {
			x=0;
			for(i=0;i<N;i++) {mapspikes[i]=0; mapfield[i]=0;}
			for(i=0;i<spiketot2;i++) if(spike_x2[i]!=posinvalid) {
				ybin=(int)(spike_y2[i]/mapbinsize); 
				xbin=(int)(spike_x2[i]/mapbinsize); 
				mapspikes[ybin*mapbins+xbin]++; x++;
			}
			/* Create unsmoothed rate map - N=mapbins*mapbins */
			for(i=0;i<N;i++) {
				if(mapdwell[i]>0) maprate[i]=mapspikes[i]/mapdwell[i];
				else maprate[i]=(float)maprateinvalid;
			}

			/* calculate cell rate-array statistics - SHOULD BE DONE BEFORE MAP SMOOTHING! */
			hux_ratestats(mapdwell,maprate,mapbins,mapdwelltot,11,maprateinvalid,result);
			mapratemean = result[1];
			mapratesd = result[2];
			mapratemedian = result[3];
			maprateskew = result[4];
			maprate975 = result[5];
			mapinfo=result[6]; 
			mapsparsity=result[7]; 
			mapcoherence=result[8];
			mapratebase=result[11]; // 10th percentile

			/* Smooth map - NOTE: this may assign values to bins previously set to "maprateinvalid" (ie. unvisited pixels */
			/* Smoothed map is used to determine position and extent of field - not firing rate cutoffs */
			if(mapsmooth>0) hux_smooth_map(mapspikes,mapdwell,maprate,mapbins,mapsmooth,mapsmoothtype,maprateinvalid);

			/* find map peak firing rate and identify peak position (bins & cm - visited bins only) */ 
			mapratepeak=-1.0;z=mappeakxbin=mappeakybin=-1; 


			for(y=0;y<mapbins;y++) {
			for(x=0;x<mapbins;x++) {
					w = y*mapbins+x; if(mapdwell[w]<=0.0) continue;
					// check that at least 2/3rds of the bins around the peak  are visited pixels - otherwise the peak is suspect
					if((int)maptype==2) skip=3; else skip=6; // obviously criterion is lower for linear maps
					for(i=y-1;i<=y+1;i++) {
						if(i<0 || i>=mapbins) continue;
						for(j=x-1;j<=x+1;j++) {
							if(j<0 || j>=mapbins) continue;
							if(mapdwell[(i*mapbins+j)]>0.0) skip--;
					}}
					if(skip<=0 && maprate[w]>mapratepeak) { mapratepeak=maprate[w];z=w;} // z holds the position of the peak bin
			}}
			if(z==-1) fprintf(fpverbout,"\n--- Warning: empty rate map for probe %d cluster %d\n",probe,cluster);
			else {
				mappeakybin=(int)(z/mapbins); 
				mappeakxbin=(int)(z%mapbins);
				mappeakx = mapbinsize*(mappeakxbin+0.5); 
				mappeaky = mapbinsize*(mappeakybin+0.5);
				mappeakangle=hux_heading((mappeakx-pos_centx),(mappeaky-pos_centy));
			}
			/* redefine map peak rate, but not peak position, if manual override was set*/
			if(set_mpr!=-1) mapratepeak=(float)set_mpr;

			/* if you don't want to keep smoothed rates over unvisited pixels, remove them now */
			if(mapfill==0) {for(i=0;i<N;i++) if(mapdwell[i]<=0) maprate[i]=maprateinvalid;}	
	
			/* Define field and peak zone based on smoothed rate map corrected by dwelltime */
			mapfieldsize =-1.0; 
			a=mapratepeak*mapthresh/100.0; b=mappeakzone/100.0;
			if(nox==1||noy==1) mapfieldsize = mapbinsize * hux_definefield(maprate,mapfield,mapbins,mappeakxbin,mappeakybin,a,b);
			else mapfieldsize = mapbinsize*mapbinsize * hux_definefield(maprate,mapfield,mapbins,mappeakxbin,mappeakybin,a,b);

			/* Calculate infield statistics and output firing rate array for this cell */ 
			mapfielddwell=0.0; mapfieldspikes=0;
			if(output.map==1) {	for(i=0;i<N;i++) {if(mapfield[i]>0) {mapfielddwell +=mapdwell[i]; mapfieldspikes +=mapspikes[i];}}}

			/* Calculate spike "field-status" and distance from field centre */
			for(i=0;i<spiketot;i++) {
				if(spike_probe[i]!=probe || spike_clust[i]!=cluster || spike_x[i]==posinvalid) continue;
				aa = mappeakx-spike_x[i]; 
				bb = mappeaky-spike_y[i]; 
				/* calculate angle separating trajectory and heading-to-fieldpeak (heading to peak minus trajectory): 
					angle is positive if running to right of peak, negative if running to left 
					if |angle| is <90, rat is running towards field peak
					*/
				spike_peakangle[i] = hux_angleshift(spike_dir[i],hux_heading(aa,bb));
				if(fabsf(spike_peakangle[i])<90.0) a=-1.0; else a=1.0;
				/* distance, adjusted for field peak vector, so approach field = -ive, leaving = +ive */
				spike_peakdis[i] = a*sqrt(aa*aa + bb*bb); 
				/* calculate infield status and firing rate zone (again, corrected for movement relative to peak */
				ybin=(int)(spike_y[i]/mapbinsize); xbin=(int)(spike_x[i]/mapbinsize); /* determine current bin position */
				spike_field[i]= mapfield[ybin*mapbins+xbin];
				spike_ratedis[i] = a*(1.0-(maprate[ybin*mapbins+xbin]/mapratepeak)); /* distance as a function of firing rate in current bin */
			}

			/* Do runs-analysis  */
			if(output.runs==1) {
				runtot=0; 
				/* Define runs through field using position and mapfield data */
				runtot = hux_getruns(postot,pos_x,pos_y,pos_dir,pos_vel,pos_avel,pos_path,posinvalid,dirinvalid,avelinvalid,f_velmin,maprate,mapfield,mapbins,mapbinsize,runstart,runend,runlen,rundir,rundirsd,runavel,runcentre,runmaxbin,runstop,MAXRUNS);
				if(runtot>0) {
					/* Calculate mean velocity of each run (total path length / total time elapsed) */
					for(run=0;run<runtot;run++) {
						runvel[run]=runlen[run]/(pos_time[runend[run]]-pos_time[runstart[run]]);
						runspikes[run]=0;
					}
					/* Assign run and field statistics to spike records for current cell */
					run=0;
					for(i=0;i<spiketot;i++) {
						if(spike_probe[i]!=probe || spike_clust[i]!=cluster) continue;
						if(spike_time[i]<pos_time[runstart[run]]) continue;
						/* keep incrimenting run until current spiketime is less than run end */ 
						else while (spike_time[i]>pos_time[runend[run]]) if(++run>=runtot) break;
						if(run>=runtot) break;	
						if(spike_time[i]>=pos_time[runstart[run]]) { /* only get here if spike occurs within current run */ 
							spike_run[i] = run;
							runspikes[run]++;
							spike_runtime[i] = spike_time[i] - pos_time[runstart[run]];
							/* calculate distance into run at which spike occurred, not including next position sample past spike */
							for(z=runstart[run];spike_time[i]<=pos_time[z];z++) if(pos_path[z]!=posinvalid) spike_rundis[i] += pos_path[z];
							/* now, add distance between current spike and last position sample */
							z-=1; if(spike_x[i]!=posinvalid&&pos_x[z]!=posinvalid) {
				                a = spike_x[i]-pos_x[z]; 
								b = spike_y[i]-pos_y[z]; 
								spike_rundis[i] += sqrt(a*a + b*b);
				}}}}
				/* write runs data to files */
				for(run=0;run<runtot;run++) {
					a=pos_time[runend[run]]-pos_time[runstart[run]]; /* run duration */
					b=maprate[runmaxbin[run]];
					c=b/mapratepeak;
					fprintf(fpoutruns,"%02d	%02d	%d	%f	%f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%d	%.2f	%d	%.3f	%d	",
						probe,cluster,run,pos_time[runstart[run]],pos_time[runend[run]],
						a,runlen[run],runvel[run],rundir[run],rundirsd[run],runavel[run],runspikes[run],((float)runspikes[run]/a),runcentre[run],c,runstop[run]);
					fprintf(fpoutruns,"%s",suffix); /* print user-defined suffix codes */
					fprintf(fpoutruns,"\n");
			}} 

			/* output rate matrix */
			if(output.matrix>0) {
				fprintf(fpoutmatrix,"\n\n# Cell %d Probe %02d Cluster %02d \n",cellid[probe][cluster],probe,cluster); 
				if(mapfill==0) for(i=0;i<N;i++) { 	// only output smoothed rates if dwelltime in bin >0
					if(i%mapbins==0) fprintf(fpoutmatrix,"\n"); 
					fprintf(fpoutmatrix,"%.4f ",maprate[i]);
				}
				else for(i=0;i<N;i++) {		// output smoothed rates, regardless of whether 
					if(i%mapbins==0) fprintf(fpoutmatrix,"\n"); 
					fprintf(fpoutmatrix,"%.4f ",maprate[i]);
			}}

			/* output postscript file */
			if(output.map==1) {
				if(mapnums==0) sprintf(map_title,"%02d  %02d  n:%d  top:%.2f\0", probe,cluster,spikecount[probe*MAXCLUSTERS+cluster],maprate975);
				if(mapnums==1) sprintf(map_title,"%04d  n:%d  top:%.2f\0", cellid[probe][cluster],spikecount[probe*MAXCLUSTERS+cluster],maprate975);
				if(output.field==0) {
					sprintf(outfile,"%scrunch_map_P%02d_C%02d.ps\0",path_data,probe,cluster);
					xf_psratemap(maptype,maprate,mapbins,mapbinsize,mapratepeak,mappeakxbin,mappeakybin,mapbinscale,vidratio,map_title,fontsize,outfile);
				}
				if(output.field==1) {
					/* output representation of field */
					sprintf(outfile,"%scrunch_field_P%02d_C%02d.ps\0",path_data,probe,cluster);
					for(ybin=0;ybin<mapbins;ybin++) for(xbin=0;xbin<mapbins;xbin++) {w=ybin*mapbins+xbin;maprate[w]= (float)mapfield[w];} 
					xf_psratemap(maptype,maprate,mapbins,mapbinsize,2,mappeakxbin,mappeakybin,mapbinscale,vidratio,map_title,fontsize,outfile);
			}}

		} /* END OF CONDITION "IF POSFILETOT>0")*/
		
		/* OUTPUT CELL RESULTS */
		z=spikecount[probe*MAXCLUSTERS+cluster];
		fprintf(fpoutcells,"%02d	%02d	%04d	",probe,cluster,cellid[probe][cluster]);
		fprintf(fpoutcells,"%.2f	%d	%.2f	%.2f	%.2f	",cellburstiness,z,mapratebase,((float)z/duration),maprate975);
		fprintf(fpoutcells,"%.2f	%d	%.2f	",mapfielddwell,mapfieldspikes,mapfieldsize);
		fprintf(fpoutcells,"%d	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.3f	%.4f	",
			runtot,mappeakx,mappeaky,mappeakangle,mapinfo,mapsparsity,mapcoherence,cellphasemean,cellphasevector,cellphasesig);
		fprintf(fpoutcells,"%s\n",suffix); /* print user-defined suffix codes */
	} /* END OF FOR CLUSTER = 1 to MAXCLUSTERS LOOP */
	fprintf(fpverbout,"\n");
} /* END OF FOR PROBE = 0 to MAXCLUSTERS LOOP */


free(tempfa2);
free(tempfa3);
fclose(fpoutcells);
if(output.runs==1) fclose(fpoutruns);
if(output.matrix==1) fclose(fpoutmatrix);


/* 
MAIN SPIKE OUTPUT !! spike positions, phase etc
******************************************************************************************************************************/
if(output.spike==1) {
	sprintf(outfile,"%scrunch_spikes.txt\0",path_data);
	fprintf(fpverbout,"Producing spike file \"%s\"\n",outfile+hux_getfilename(outfile));
	fpout=fopen(outfile,"w");
	if(fpout==NULL) hux_error("can't open spike file");
	fprintf(fpout,"#TIME\tPROBE\tCLUST\tCELLID\tBURST\tIRATE\tTPHASE\tXPOS\tYPOS\tANGLE\tDIR\tHDIR\tVEL\tAVEL\tRUN\tRUNTIME\tRUNDIST\tFIELD\tPEAKANGLE\tPEAKDIST\tRATEDIST\n");

	if(posfiletot>0 && eegfiletot>0) {
		for(i=0;i<spiketot;i++) {
			if(spike_clust[i]<clustart) continue;	
			fprintf(fpout,"%.7f	%d	%d	%d	%d	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%d	%.2f	%.2f	%d	%.2f	%.2f	%.3f\n", 
				spike_time[i],spike_probe[i],spike_clust[i],spike_id[i],spike_burst[i],spike_irate[i],spike_phase[i],spike_x[i],spike_y[i],spike_angle[i],spike_dir[i],spike_headdir[i],spike_vel[i],spike_avel[i],spike_run[i],spike_runtime[i],spike_rundis[i],spike_field[i],spike_peakangle[i],spike_peakdis[i],spike_ratedis[i]);
	}}

	if(posfiletot>0 && eegfiletot<=0) {
		for(i=0;i<spiketot;i++) {
			if(spike_clust[i]<clustart) continue;	
			fprintf(fpout,"%.7f	%d	%d	%d	%d	%.2f	-	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%d	%.2f	%.2f	%d	%.2f	%.2f	%.3f\n",
				spike_time[i],spike_probe[i],spike_clust[i],spike_id[i],spike_burst[i],spike_irate[i],spike_x[i],spike_y[i],spike_angle[i],spike_dir[i],spike_headdir[i],spike_vel[i],spike_avel[i],spike_run[i],spike_runtime[i],spike_rundis[i],spike_field[i],spike_peakangle[i],spike_peakdis[i],spike_ratedis[i]);
	}}

	if(posfiletot<=0 && eegfiletot>0) {
		for(i=0;i<spiketot;i++) {
			if(spike_clust[i]<clustart) continue;	
			fprintf(fpout,"%.7f	%d	%d	%d	%d	%.2f	%.2f	-	-	-	-	-	-	-	-	-	-	-	-	-\n",
				spike_time[i],spike_probe[i],spike_clust[i],spike_id[i],spike_burst[i],spike_irate[i],spike_phase[i]);
	}}

	if(posfiletot<=0 && eegfiletot<=0) {
		for(i=0;i<spiketot;i++) {
			if(spike_clust[i]<clustart) continue;
			if(spike_clust[i]>probe_maxcluster[spike_probe[i]]) hux_error("MAXCLUSTER EXCEEDED");
			fprintf(fpout,"%.7f	%d	%d	%d	%d	%.2f	-	-	-	-	-	-	-	-	-	-	-	-	-	-\n",
				spike_time[i],spike_probe[i],spike_clust[i],spike_id[i],spike_burst[i],spike_irate[i]);
	}}

	fclose(fpout);
}

/* Assemble postscript files */
if(spikefiletot>0 && posfiletot>0 && output.map==1) {
	if(output.field==1) sprintf(infile1,"%scrunch_field_",path_data);
	else sprintf(infile1,"%scrunch_map_",path_data);
	sprintf(infile2,"%scrunch_isi_",path_data);
	sprintf(outfile,"%scrunch_SUMMARY_",path_data);
	j = 0; 
	for(probe=0;probe<MAXPROBES;probe++) {
		if(probe_goodspikes[probe]>0) j=probe; /* find highest probe number */
	}
	hux_psratemap_assemble(spikecount,MAXCLUSTERS,MAXPROBES,j,vidratio,fontsize,datatype,infile1,infile2,outfile,systemtype,mapcompact,clean);
}

/************************************************************************
Free memory and exit with no errors
*/
bookmark_end:

if(setverb<2) fclose(fpverbout);

if(spikefiletot>0){   
	free(spike_time);
	free(spike_time2);
	free(spike_probe);
	free(spike_clust);
	free(spike_isi);
	free(spike_id);
	free(spike_burst);
	free(spike_irate);
	free(spikecount);
	if(posfiletot>0) {
		free(spike_x);
		free(spike_y);
		free(spike_x2);
		free(spike_y2);
		free(spike_dir);
		free(spike_headdir);
		free(spike_angle);
		free(spike_vel);
		free(spike_avel);
		free(spike_run);
		free(spike_runtime);
		free(spike_rundis);
		free(spike_field);
		free(spike_peakdis);
		free(spike_peakangle);
		free(spike_ratedis);
		free(spike_filter);
	}
	if(eegfiletot>0) {
		free(spike_phase);
		free(phase);
	}
	if(eventfiletot>0) free(spike_event);
}
if(posfiletot>0) {
    free(pos_time);
	free(pos_x);
	free(pos_y);
	free(pos_leddis);
	free(pos_dir);
	free(pos_headdir);
	free(pos_vel);
	free(pos_angle);
	free(pos_avel);
	free(pos_path);
	free(pos_filter);
	free(mapspikes);
	free(mapfield);
	free(mapdwell);
	free(maprate);
}

if(eegfiletot>0) {
    free(eeg_time);
	free(eeg_val);
	free(eeg_phase);
	free(eeg_vel);
	free(eeg_filter);
	free(cycle_start);
	free(cycle_end);
}

if(eventfiletot>0) {
	free(event_time); 
	free(event_label);
	free(event_filter);
}

if(setpause==1) hux_error("");
exit(0);
}


