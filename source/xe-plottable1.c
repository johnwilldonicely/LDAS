#define thisprog "xe-plottable1"
#define TITLE_STRING thisprog" v 75: 30.April.2020 [JRH]"
#define MAXLINELEN 10000
#define MAXGROUPS 32
#define MAXUSERLINES 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <ctype.h>

/*
<TAGS>plot</TAGS>

v 75: 30.April.2020 [JRH]
	- restore "-colour" option functionalty lost in last revision, and use xf_scale_l to wrap colours

v 75: 18.April.2020 [JRH]
	- allow colour-gradient palette option (-pal) to have group-id determine the colour

v 75: 9.November.2019 [JRH]
	- bugfix tic-drawing - set min and max x/y tics to prevent printing outside of actual data range

v 75: 28.September.2019 [JRH]
	- add option to automatically jitter x-values to improve visability of dense points
	- fix auto-range and tics when there is no variability in X or Y - previously failed for negative X or Y values

v 75: 16.July.2019 [JRH]
	- drop conversion of NANs to zero - simply don't consider invalid x/y values
	- this could have had unpredictable effects on calculating ranges, or give false results
	- note that NAN or INF error-bars will be converted to zero

v 75: 29.April.2019 [JRH]
	- renamed from plotpost to plottable1

v 75: 1.January.2019 [JRH]
	- correct mis-printing of tics due to floating-point imprecision
		- for integers, use %ld format instead %.0f
	- simplify and improve definition of tic-label-offsets
	- update variable names

v 75: 24.November.2018 [JRH]
	- bugfix padding settings and remove unnecessary variables - just keep setxpad & setypad

v 75: 30.September.2018 [JRH]
	- bugfix: linecount (for estimating x-value when there is no x-column) now only increments AFTER checking for comments and blank lines
	- bugfix: finally resolved issues of automatic range and interval setting
	- add verbose output option

v 75: 30.August.2018 [JRH]
	- expand palette to 32 colours

v 75: 2.June.2018 [JRH]
	- added x-error-bar option
	- made memory reservation for error-bars dependent on whether they are defined
	- some variable renaming

v 75: 20.March.2018 [JRH]
	- move pointfill definition close to this block: "% DEFINE_DATA_RELATED_VALUE"
		- allows combination of plots while preserving point types

v 75: 20.April.2017 [JRH]
	- bugfix: definition of setxint/setyint is now double to allow fractional settings

v 75: 31.March.2017 [JRH]
	- allow omission of xtics and ytics (set xint or yint to -1)

v 75: 9.January.2017 [JRH]
	- bugfix decimal precision calculation by xf_precision_d
		- needed to allow max decimal precision due to issues precisely representing floating-point numbers

v 75: 3.November.2016 [JRH]
	- add new point-type "bar"

v 75: 1.November.2016 [JRH]
	- add auto-set min/max values to zero if they are < 1/10th the range on that axis

v 75: 22.October.2016 [JRH]
	- add redefinition of xtloff (x-axis-label offset) at plot-time to facilitate modification if tics are adjusted

v 75: 21.October.2016 [JRH]
	- as with lines and points, replace "errorbar" definition with "E"

v 75: 6.September.2016 [JRH]
	- add control of tic size

v 75: 5.September.2016 [JRH]
	- simplify line and point definintions to L and P
	- define pointdraw directly (no indirect reference to fill or open functions
	- bugfix: white-fill usage

TO DOs
	- some pecular axis/tic positioning with some settings - especially with padding turned off
	- with box-plots solve why is last box to the right sometimes wider than others
	- tic size should be function of plot size, or user-set
	- add control for trim to digits on x/y-axes

GENERAL NOTES
- After translation step, the x/y axes represent postscript coordinates 0/0
- Datapoints and tics are transformed within postscript functions
- This means the raw data can be extracted from the postscript file
- x/y tics begin from xmin/ymin, and rise by xint/yint
- padding is added before the first tic on each axis
- padding is also added to the top of each scale (beyond xmax/ymax)
- tics can extend to the padded region at the top of each scale

Absolute quantities (insensitive to scaling)
- ticsize, fontsize, data linewidth, axes linewidth

*/

/* external functions start */
double xf_trimdigits_d(double number_to_trim, int digits_to_keep);
char* xf_strsub1 (char *source, char *str1, char *str2);
long xf_binpeak4(double *time, double *data , long n, double winwidth); // not currently used - to allow future downsampling
double xf_round1_d(double input, double setbase, int setdown);
int xf_precision_d(double number, int max);
double *xf_unique_d(double *data, long *nn, char *message);
double *xf_jitter1_d(double *yval, long nn, double centre, double limit, char *message);
double xf_rand1_d(double setmax);
long xf_scale1_l(long old, long min, long max);
int xf_palette7(float *red, float *green, float *blue, long nn, char *palette);
long xf_interp3_f(float *data, long ndata);
int xf_compare1_i(const void *a, const void *b);
void xf_qsortindex1_i(int *data, long *index,long nn);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char *infile=NULL,outfile[256],line[256],newline[256],*pline,*pcol,*words=NULL,message[256];
	long ii,jj,kk,nn,mm;
	int w,x,y,z,col,colsmissing=2,ngroups=0,lengroups=0;
	int  sizeofchar=sizeof(char),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double),sizeoflong=sizeof(long);
	float a,b,c,d,e;
	double aa,bb,cc,dd,ee,ff;
	FILE *fpin,*fpout;

	/* program-specific variables */
	char xlabel[256],ylabel[256],plottitle[256],*tempword=NULL;
	int grpcount[(MAXGROUPS+1)],grpcount2[(MAXGROUPS+1)],iword[(MAXGROUPS+1)],iword2[(MAXGROUPS+1)];
	int *group=NULL, *linebreak=NULL,*temp_linebreak=NULL,lb,grp,groupfound;
	int xticprecision,yticprecision,groupsareints;
	int grpc[MAXGROUPS];
	long groupindex[(MAXGROUPS+1)];
	long n1,linecount,ncolours=33;
	long *tempindex=NULL;
	long tempcolour1,tempcolour2,maxcolour;
	float xlimit=500.0,ylimit=500.0; // the postscript plotting space (pixels)- should be square and smaller than an A4 sheet (591 x 841)
	float psxmin,psxmax,psymin,psymax; // currently unused - defines bounding box, i.e. plot area including labels (currently assume A4 page instead)
	float zx=-1,zy=-1; // zero-coordinates for plot - readjust zy if scale is reset
	double xfactor,yfactor; // variables scaled by data range + padding to transform the data to x/y coordinates
	float groupshift[MAXGROUPS+1],yticmaxchar;
	double *xdata=NULL,*ydata=NULL,*edata=NULL,*fdata=NULL;
	double *temp_xdata=NULL,*temp_ydata=NULL,*temp_edata=NULL,*temp_fdata=NULL,*temp_xunique=NULL,*tempjit=NULL;
	double xmin,xmax,ymin,ymax,xrange,yrange,newx,newy,xticmin,xticmax,yticmin,yticmax;
	double winwidth=0.0;

	/* colour-palette variables */
	char *setrgbpal=NULL;
	float *red=NULL,*green=NULL,*blue=NULL;

	/* arguments */
	char plottype[16],pointtype[16],bigtic[256],*hlineword=NULL,*vlineword=NULL;
	int setverb=0,setxcol=1,setycol=2,setfcol=-1,setecol=-1,setgcol=-1,setxmin=0,setxmax=0,setymin=0,setymax=0,setyzeroline=1,setline=0,sethline=0,setvline=0,setlinebreak=0,setlegend=0;
	int setpointsize=0,boxyzero=1,setewidth=0,setelwidth=0,setebright=0;
	int pointfill=1, framestyle=3, f1=0,f2=0,f3=0,f4=0,setdatacolour=0,setmaxpoints=10000,setmid=1,setgshift=0;
	double setxminval,setxmaxval,setyminval,setymaxval,setxint=0.0,setyint=0.0,xint=0.0,yint=0.0,setxpad=-1.0,setypad=-1.0,setdown=0.0,setjitter=0.0;
	double hline[MAXUSERLINES],vline[MAXUSERLINES],hlinemin,hlinemax,vlinemin,vlinemax;
	float xscale=.3,yscale=.3; // plot scale - xlimit and ylimit are multiplied by these values
	float setticsize=-3,pointsize=5,fontsize=10.0;
	float boxwidth=0.75, ewidth=(boxwidth*0.5),lwdata=1,lwaxes=1,lwerror=0.75; // boxwidth and linewidth for drawing data and frame/tics

	sprintf(outfile,"temp_%s.ps",thisprog);
	xlabel[0]=0;
	ylabel[0]=0;
	plottitle[0]= '\0';
	sprintf(plottype,"cir");

	/* allocate memory for hlines and vlines */
	hlineword=(char *)realloc(hlineword,MAXLINELEN*sizeof(char));
	vlineword=(char *)realloc(vlineword,MAXLINELEN*sizeof(char));
	// initialize storage for list of horizontal and vertical lines and associated arrays
	vlineword[0]= '\0'; for(ii=0;ii<MAXUSERLINES;ii++) vline[ii]=NAN;
	hlineword[0]= '\0'; for(ii=0;ii<MAXUSERLINES;ii++) hline[ii]=NAN;

	setxminval=setyminval=hlinemin=vlinemin= DBL_MAX;
	setxmaxval=setymaxval=hlinemax=vlinemax= -DBL_MAX;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Produces a postscript plot of data\n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"USAGE...\n");
		fprintf(stderr,"	%s [filename] [options]\n\n",thisprog);
		fprintf(stderr,"VALID OPTIONS (default in [])...\n");
		fprintf(stderr,"	[filename]: file name or \"stdin\"\n");
		fprintf(stderr,"	-cx: x-data column (-1 to infer x from sample-number) [%d]\n",setxcol);
		fprintf(stderr,"	-cy: y-data column [%d]\n",setycol);
		fprintf(stderr,"	-ce: y-error estimate column [%d]\n",setecol);
		fprintf(stderr,"	-cf: x-error estimate column [%d]\n",setfcol);
		fprintf(stderr,"	-cg: group ID column [%d]\n",setgcol);
		fprintf(stderr,"		- can be numerical or text (no spaces)%d\n",MAXGROUPS);
		fprintf(stderr,"		- if text, group-colour assigned by order of appearance\n");
		fprintf(stderr,"	-xmin -xmax -ymin -ymax: manually set data range\n");
		fprintf(stderr,"	-xpad -ypad: pad between data range and plot axes (-1=auto)\n");
		fprintf(stderr,"	-xint -yint: interval between axis-tics (0=AUTO -1=OMIT) [%g %g]\n",setxint,setyint);
		fprintf(stderr,"	-jitter: apply this much jitter (max) to x-values [%g]\n",setjitter);
		fprintf(stderr,"		NOTE: do not use with -line option\n");
		fprintf(stderr,"	-line: draw line between data points (1=YES,0=NO)[%d]\n",setline);
		fprintf(stderr,"	-pt: plot type (squ cir tri box bar histo) [%s]\n",plottype);
		fprintf(stderr,"	-ps: point size (zero to omit) [%g]\n",pointsize);
		fprintf(stderr,"	-pf: point fill (0=no, -1=white, 1=datacolour) [%d]\n",pointfill);
		fprintf(stderr,"	-colour: colour for lowest group (-1 to %d) [%d]\n",MAXGROUPS,setdatacolour);
		fprintf(stderr,"	-ebright: adjust error-bar colours up (typically 8,16,24) [%d]\n",setebright);
		fprintf(stderr,"	-pal: colour palette (black=min, white=NAN): \n");
		fprintf(stderr,"	    NOTE: do not use with -colour or -ebright\n");
		fprintf(stderr,"	        grey: darkgrey-lightgrey\n");
		fprintf(stderr,"	        rainbow: blue-green-red\n");
		fprintf(stderr,"	        viridis: purple-green-yellow\n");
		fprintf(stderr,"	        plasma: blue-purple-yellow\n");
		fprintf(stderr,"	        magma: black-purple-cream\n");
		fprintf(stderr,"	        inferno: black-purple-orange-paleyellow\n");
		fprintf(stderr,"	-bw: box/bar width, as a fraction of xint (above) [%g]\n",boxwidth);
		fprintf(stderr,"	-ew: error-bar width, fraction of xint (above) [%g]\n",ewidth);
		fprintf(stderr,"	-bz: boxes and histograms extend to zero? (1=YES,0=NO)[%d]\n",boxyzero);
		fprintf(stderr,"	-gs: group-shift on x-axis (1=YES,0=NO) [%d]\n",setgshift);
		fprintf(stderr,"		NOTE: suitable for box bar or histo plots only\n");
		fprintf(stderr,"		NOTE: puts groups side by side centred on x-value\n");
		fprintf(stderr,"		NOTE: set -bw to 1/(#groups+1) for this to look nice\n");
		fprintf(stderr,"	-xlabel: x-axis label, in quotes [unset]\n");
		fprintf(stderr,"	-ylabel: y-ayis label, in quotes [unset]\n");
		fprintf(stderr,"	-title: plot title (enclose in quotes)\n");
		fprintf(stderr,"	-legend: display legend (0=NO, 1=bottom-left, 2=top-right)[%d]\n",setlegend);
		fprintf(stderr,"	-frame: draw frame at bottom(1) left(2) top(4) right(8) [%d]\n",framestyle);
		fprintf(stderr,"		-NOTE: these are additive, eg full box=15 [%d]\n",framestyle);
		fprintf(stderr,"	-tics: size of x- and y-tics (negative=outside frame) [%g]\n",setticsize);
		fprintf(stderr,"	-hline: CSV list of y-values for horizontal lines [unset]\n");
		fprintf(stderr,"	-vline: CSV list of x-values for vertical lines [unset]\n");
		fprintf(stderr,"		NOTE: maximum %d lines of each type\n",MAXUSERLINES);
		fprintf(stderr,"		NOTE: if > data range, plot range will be expanded\n");
		fprintf(stderr,"	-yzero: draw zero-line if y-data spans zero (0=NO 1=YES) [%d]\n",setyzeroline);
		fprintf(stderr,"	-xscale: scale plot in x dimension [%g] \n",xscale);
		fprintf(stderr,"	-yscale: scale plot in y dimension [%g] \n",yscale);
		fprintf(stderr,"	-font: base font size [%g]\n",fontsize);
		fprintf(stderr,"	-lwd: line width for data [%g]\n",lwdata);
		fprintf(stderr,"	-lwe: line width for error-bars [%g]\n",lwerror);
		fprintf(stderr,"	-lwa: line width for axes [%g]\n",lwaxes);
		fprintf(stderr,"	-lb: break lines in plot [%d]\n",setlinebreak);
		fprintf(stderr,"		(0) no line-breaks in plot\n");
		fprintf(stderr,"		(1) if there is missing data or blank lines\n");
		fprintf(stderr,"		(2) if a time-series repeats (x[ii]<x[i-1]) \n");
		fprintf(stderr,"	-zx, -zy: page offset of the plot [%g,%g]\n",zx,zy);
		fprintf(stderr,"		NOTE: -1 = default A4 top-left\n");
		fprintf(stderr,"	-out: output file name [%s]\n",outfile);
		fprintf(stderr,"	-verb: verbose output (0=NO, 1=YES) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -cx 2 -cy 3 -line 1 -title \"Sample-1\"\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	- postscript file, default name \"%s\"\n",outfile);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if(ii>=argc) break;
			if((ii+1)>=argc) {fprintf(stderr,"\n\a--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1); }
			else if(strcmp(argv[ii],"-colour")==0) 	{ setdatacolour=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-pal")==0)     { setrgbpal= argv[++ii]; }
			else if(strcmp(argv[ii],"-ebright")==0) { setebright=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-cx")==0) 	{ setxcol=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-cy")==0) 	{ setycol=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-ce")==0) 	{ setecol=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-cf")==0) 	{ setfcol=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-cg")==0) 	{ setgcol=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-gs")==0) 	{ setgshift=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-xlabel")==0) 	{ sprintf(xlabel,"%s",(argv[++ii])); }
			else if(strcmp(argv[ii],"-ylabel")==0) 	{ sprintf(ylabel,"%s",(argv[++ii])); }
			else if(strcmp(argv[ii],"-title")==0) 	{ sprintf(plottitle,"%s",(argv[++ii])); }
			else if(strcmp(argv[ii],"-legend")==0) 	{ setlegend=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-font")==0) 	{ fontsize=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-xscale")==0) 	{ xscale=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-yscale")==0) 	{ yscale=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-xmin")==0) 	{ setxminval=atof(argv[++ii]); setxmin=1; }
			else if(strcmp(argv[ii],"-ymin")==0) 	{ setyminval=atof(argv[++ii]); setymin=1; }
			else if(strcmp(argv[ii],"-xmax")==0) 	{ setxmaxval=atof(argv[++ii]); setxmax=1; }
			else if(strcmp(argv[ii],"-ymax")==0) 	{ setymaxval=atof(argv[++ii]); setymax=1; }
			else if(strcmp(argv[ii],"-xint")==0) 	{ setxint=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-yint")==0) 	{ setyint=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-jitter")==0) 	{ setjitter=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-xpad")==0) 	{ setxpad=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-ypad")==0) 	{ setypad=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-ew")==0) 	{ ewidth=atof(argv[++ii]); setewidth=1; }
			else if(strcmp(argv[ii],"-bw")==0) 	{ boxwidth=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-bz")==0) 	{ boxyzero=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-line")==0) 	{ setline=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-hline")==0) 	{ sprintf(hlineword,"%s",(argv[++ii])); sethline=1; }
			else if(strcmp(argv[ii],"-vline")==0) 	{ sprintf(vlineword,"%s",(argv[++ii])); setvline=1; }
			else if(strcmp(argv[ii],"-yzero")==0) 	{ setyzeroline=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-lwd")==0) 	{ lwdata=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-lb")==0) 	{ setlinebreak=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-lwa")==0) 	{ lwaxes=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-lwe")==0) 	{ lwerror=atof(argv[++ii]); setelwidth=1; }
			else if(strcmp(argv[ii],"-frame")==0) 	{ framestyle=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-tics")==0)    { setticsize=atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-pt")==0) 	{ sprintf(plottype,"%s",(argv[++ii])); }
			else if(strcmp(argv[ii],"-ps")==0) 	{ pointsize=atof(argv[++ii]); setpointsize=1; }
			else if(strcmp(argv[ii],"-pf")==0) 	{ pointfill=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-plot")==0) 	{ sprintf(plottype,"%s",(argv[++ii])); }
			else if(strcmp(argv[ii],"-out")==0) 	{ snprintf(outfile,256,"%s",(argv[++ii])); }
			else if(strcmp(argv[ii],"-zx")==0) 	{ zx=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-zy")==0) 	{ zy=atoi(argv[++ii]); }
			else if(strcmp(argv[ii],"-verb")==0) 	{ setverb=atoi(argv[++ii]); }
			else {fprintf(stderr,"\n\a--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1); }
	}}

	// CHECK FOR INVALID OPTIONS
	if(strcmp(plottype,"histo")!=0 && strcmp(plottype,"box")!=0 && strcmp(plottype,"bar")!=0 && strcmp(plottype,"cir")!=0 && strcmp(plottype,"squ")!=0 && strcmp(plottype,"tri")!=0) {
		{fprintf(stderr,"\n\a--- Error[%s]: invalid plot-type specified (%s) - choose cir, squ, tri, box, bar or histo\n\n",thisprog,infile);exit(1); }
	}
	if(setgshift==1 && (strcmp(plottype,"histo")!=0 && strcmp(plottype,"box")!=0) && strcmp(plottype,"bar")!=0){
		{fprintf(stderr,"\n\a--- Error[%s]: cannnot apply group shift unless plot-type (%s) is box, bar or histo\n\n",thisprog,infile);exit(1); }
	}
	if(setjitter<0) {fprintf(stderr,"\n\a--- Error[%s]: invalid -jitter (%g) - should be either >= 0 \n\n",thisprog,setjitter);exit(1); }
	if(setjitter>0 && setline>0) {fprintf(stderr,"\n\a--- Error[%s]: cannot combine jitter (-jitter %g) with line-graphs (-line %d)\n\n",thisprog,setjitter,setline);exit(1); }
	if(setxcol<1&&setxcol!=-1) {fprintf(stderr,"\n\a--- Error[%s]: invalid -cx (%d) - should be either -1 or a positive integer\n\n",thisprog,setxcol);exit(1); }
	if(setlegend<0||setlegend>2) {fprintf(stderr,"\n\a--- Error[%s]: invalid -legend (%d) - should be either 0 1 or 2\n\n",thisprog,setlegend);exit(1); }
	if(setyzeroline!=0&&setyzeroline!=1) {fprintf(stderr,"\n\a--- Error[%s]: invalid -yzero (%d) - should be either 0 or 1\n\n",thisprog,setyzeroline);exit(1); }
	if(setverb!=0&&setverb!=1) {fprintf(stderr,"\n\a--- Error[%s]: invalid -verb (%d) - should be either 0 or 1\n\n",thisprog,setverb);exit(1); }
	if(setebright<0||setebright>(MAXGROUPS-1)) {fprintf(stderr,"\n\a--- Error[%s]: invalid -ebright (%d) - should be 0-%d\n\n",thisprog,setebright,(MAXGROUPS-1));exit(1); }
	if(pointfill<0 && (strcmp(plottype,"histo")==0 || strcmp(plottype,"bar")==0)) {fprintf(stderr,"\n\a--- Error[%s]: white-fill (-pf -1) cannnot be used with histograms (-pt histo) or bars (-pt bar) \n\n",thisprog);exit(1); }
	// automatically determine error-bar width and line width if required
	if(setewidth==0) ewidth=0.5*boxwidth;
	if(setelwidth==0) lwerror=0.5*lwdata;
	if(setrgbpal!=NULL) {
		if(
		strcmp(setrgbpal,"grey")!=0
		&& strcmp(setrgbpal,"rainbow")!=0
		&& strcmp(setrgbpal,"viridis")!=0
		&& strcmp(setrgbpal,"plasma")!=0
		&& strcmp(setrgbpal,"magma")!=0
		&& strcmp(setrgbpal,"inferno")!=0
		) {fprintf(stderr,"\n\a--- Error[%s]: illegal -pal (%s)\n\n",thisprog,setrgbpal);exit(1);}
	}

	// build the list of horizontal lines
	pline=hlineword;
	for(col=0;(pcol=strtok(pline,","))!=NULL;col++)	{
		pline=NULL;
		if(col>=MAXUSERLINES) {fprintf(stderr,"\n\a--- Error[%s]: too many horizontal lines specified (-hline %s) - max %d allowed\n\n",thisprog,hlineword,MAXUSERLINES);exit(1); }
		else hline[col]=atof(pcol);
		if(col==0) hlinemin=hlinemax=hline[0];
		if(hline[col]<hlinemin) hlinemin=hline[col];
		if(hline[col]>hlinemax) hlinemax=hline[col];
	}
	// build the list of vertical lines
	pline=vlineword;
	vlinemin=vlinemax=vline[0];
	for(col=0;(pcol=strtok(pline,","))!=NULL;col++)	{
		pline=NULL;
		if(col>=MAXUSERLINES) {fprintf(stderr,"\n\a--- Error[%s]: too many vertical lines specified (-vline %s) - max %d allowed\n\n",thisprog,vlineword,MAXUSERLINES);exit(1); }
		else vline[col]=atof(pcol);
		if(col==0) vlinemin=vlinemax=vline[0];
		if(vline[col]<vlinemin) vlinemin=vline[col];
		if(vline[col]>vlinemax) vlinemax=vline[col];
	}


	/******************************************************************************/
	/******************************************************************************/
	/* READ THE DATA */
	/* open the data file - define the number of columns which should be found on each line */
	/******************************************************************************/
	/******************************************************************************/
	colsmissing=2; a=b=c; d=0; n1=0;
	if(setecol>0) colsmissing++;	// need to detect one extra column
	if(setfcol>0) colsmissing++;	// need to detect one extra column
	if(setgcol>0) colsmissing++;	// need to detect one extra column
	if(setxcol==-1) colsmissing--;	// decrease detected columns by one if "x" is inferred from sample-number
	for(ii=0;ii<=MAXGROUPS;ii++) grpcount[ii]=0;

	/* NOW READ THE FILE AND STORE THE DATA */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n\a--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1); }
	linecount= 0;
	lb= 0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(line[0]=='#') continue;
		aa= bb= cc= 0.0;
		w= colsmissing;
		groupfound= 0;
		pline= line;
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++)	{
			pline=NULL;
			if(col==setxcol) {z=sscanf(pcol,"%lf",&aa); if(z==1) w--; }  // temporarily store x-data
			if(col==setycol) {z=sscanf(pcol,"%lf",&bb); if(z==1) w--; }  // temporarily store y-data
			if(col==setecol) {z=sscanf(pcol,"%lf",&cc); if(z==1) w--; }  // temporarily store error-data
			if(col==setfcol) {z=sscanf(pcol,"%lf",&dd); if(z==1) w--; }  // temporarily store error-data
			if(col==setgcol) {tempword=pcol; groupfound=1; w--; }        // temporarily store group-label
		}
		/* if all columns found, store data and indicate this line is not a line-break */
		if(w==0) {
			/* for valid lines only, if cx is "-1" then derive x-value from line-count */
			if(setxcol==-1) aa=(double)linecount; /* sets x to sample-number */
			/* convert NAN or INF values to zero - except x-values, which MUST be defined */
			if(!isfinite(aa)) continue;
			if(!isfinite(bb)) continue;
			if(setecol>0 && !isfinite(cc)) cc= 0.0;
			if(setfcol>0 && !isfinite(dd)) dd= 0.0;
			/* determine group ID from words or numbers in the group-column (-cg) */
			z=0; // z is the unique group index
			if(groupfound==1) {
				/* check to see if a group-word already exists... */
				z=-1; for(jj=0;jj<ngroups;jj++) if(strcmp(tempword,(words+iword[jj]))==0) z=jj;
				/* ...if not, add it to a list of words */
				if(z<0) {
					/* allocate memory for expanded words and word-index */
					x= strlen(tempword); // not including terminating NULL
					words= (char *)realloc(words,((lengroups+x+4)*sizeofchar)); if(words==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1); };
					iword[ngroups]=lengroups; /* set pointer to start position (currently, the end of the labels string) */
					sprintf(words+lengroups,"%s",tempword); /* add new word to end of words, adding terminal NULL */
					lengroups+= (x+1); /* update length, allowing for terminal NULL - serves as pointer to start of next word */
					z= ngroups; /* set the group label index */
					ngroups++; /* increment ngroups with check */
					if(ngroups>(MAXGROUPS)) {fprintf(stderr,"\n\a--- Error[%s]: categories in group-column (-cg %d) exceeds maximum allowed (%d)\n\n",thisprog,setgcol,MAXGROUPS);exit(1); }
				}
			}
			else z=0;

			linecount++;
			/* only include data falling within the user-specified x-range */
			if((setxmin==0 || aa>=setxminval) && (setxmax==0 || aa<=setxmaxval)) {
				/* adjust y-values and error bars to fit within user-specified range */
				if(setymin==1) {
					if(bb<setyminval) { bb=setyminval; cc=0.0; }
					else if((bb-cc)<setyminval) { cc=bb-setyminval; }
				}
				if(setymax==1) {
					if(bb>setymaxval) { bb=setymaxval; cc=0.0; }
					else if((bb+cc)>setymaxval) { cc=setymaxval-bb; }
				}
				/* allocate memory */
				xdata= realloc(xdata,(n1+1)*sizeofdouble);
				ydata= realloc(ydata,(n1+1)*sizeofdouble);
				if(setecol>0) {
					edata= realloc(edata,(n1+1)*sizeofdouble);
					if(edata==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1); }
				}
				if(setfcol>0) {
					fdata= realloc(fdata,(n1+1)*sizeofdouble);
					if(fdata==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1); }
				}

				group= realloc(group,(n1+1)*sizeofint);
				linebreak= realloc(linebreak,(n1+1)*sizeofint);
				if(xdata==NULL||ydata==NULL||group==NULL||linebreak==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1); }

				group[n1]=z; // the unique group-index, zero-offset - the value is stored in "words"
				grpcount[z]++;
				xdata[n1]=aa;
				ydata[n1]=bb;
				if(setecol>0) edata[n1]=cc;
				if(setfcol>0) fdata[n1]=dd;
				linebreak[n1]=lb;
				lb=0;
				n1++;
			}
		}
		/* detect missing data and if required, insert a line-break-signal in the plot */
		else if(setlinebreak==1) lb=1;
		else lb=0; /* otherwise for the next line the lb signal is zero (no break) */
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* create the original group-index array - ??? not sure how useful this is...*/
	w=0; for(ii=0;ii<=MAXGROUPS;ii++) if(grpcount[ii]>0) groupindex[ii]= w++; // w=number of valid groups

	/* ALOCATE MEMORY FOR TEMPORARY ARRAYS - THESE ARE USED DURING PLOTTING TO SPLIT THE DATA INTO GROUPS */
	temp_xdata= realloc(temp_xdata,(n1+1)*sizeofdouble);
	temp_ydata= realloc(temp_ydata,(n1+1)*sizeofdouble);
	if(setecol>0) {
		temp_edata= realloc(temp_edata,(n1+1)*sizeofdouble);
		if(temp_edata==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1); }
	}
	if(setfcol>0) {
		temp_fdata= realloc(temp_fdata,(n1+1)*sizeofdouble);
		if(temp_fdata==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1); }
	}
	temp_linebreak=(int *)realloc(temp_linebreak,(n1+1)*sizeofint);
	if(temp_xdata==NULL||temp_ydata==NULL||temp_linebreak==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1); }

	/* IF GROUP COLUMN WAS UNDEFINED, MAKE A DUMMY GROUP LABEL ARRAY */
	if(setgcol<0) {
		words=(char *)realloc(words,(4*sizeofchar)); if(words==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1); }
		sprintf(words,"0");
		ngroups=1;
		iword[0]=0;
	}

	/* WARN IF PLOT IS TO BE EMPTY - MAKE SOME FAKE DATA BUT KEEP n1=0 */
	if(n1<1) {
		fprintf(stderr,"\n\a--- Warning[%s]: no data! Check input columns and whether input is non-numeric\n\n",thisprog);
		xdata= realloc(xdata,(1)*sizeofdouble);
		ydata= realloc(ydata,(1)*sizeofdouble);
		if(setecol>0) {
			edata= realloc(edata,(1)*sizeofdouble);
			if(edata==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1); }
			edata[0]=0.0;
		}
		if(setfcol>0) {
			fdata= realloc(fdata,(1)*sizeofdouble);
			if(fdata==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1); }
			fdata[0]=0.0;
		}
		group=(int *)realloc(group,(1)*sizeofint);
		linebreak=(int *)realloc(linebreak,(1)*sizeofint);
		if(xdata==NULL||ydata==NULL||group==NULL||linebreak==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1); }
		xdata[0]=0.0;
		ydata[0]=0.0;
		words=(char *)realloc(words,(1*sizeofchar)); if(words==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1); };
		words[0]= '\0';
	}
	//TEST:	for(ii=0;ii<n1;ii++) printf("group[%ld]=%d label=%s	%g %g\n",ii,group[ii],(words+iword[group[ii]]),xdata[ii],ydata[ii]);
	//TEST: for(grp=0;grp<ngroups;grp++) {printf("label[%d]=%s\n",grp,words+iword[grp]);}

	/* MAKE SURE WORD INDICES FOR EMPTY GROUPS POINT TO THE END OF THE WORD LIST */
	w=strlen(words); for(ii=0;ii<=MAXGROUPS;ii++) if(grpcount[ii]<1) iword[ii]=w;

	/* ASSIGN COLOURS TO THE GROUPS */
	/* first check if all the group-labels are integers  */
	groupsareints=1;
	for(ii=0;ii<ngroups;ii++) {
		tempword= words+iword[ii];
		kk=strlen(tempword);
		for(jj=0;jj<kk;jj++) if(!isdigit(tempword[jj])) {groupsareints=0; break; }
	}
	/* if all group labels were integers >= 0 and <= maxgroups, use the absolute numerical value as the label */
	/* this allows users to ensure a given group (number) will have a given colour in their plots */
	if(groupsareints==1) {
		int *tempint= malloc(ngroups*sizeof(int));
		for(grp=0;grp<ngroups;grp++) tempint[grp]= atoi(words+iword[grp]);
		xf_qsortindex1_i(tempint,groupindex,(long)ngroups);
		for(grp=0;grp<ngroups;grp++) grpc[grp]= groupindex[grp];
	}
	/* otherwise, use the original order in which the groups appeared in the data  */
	else {
		for(grp=0;grp<ngroups;grp++) { grpc[grp]=grp; }
	}
	//TEST:for(grp=0;grp<ngroups;grp++) {printf("label[%d]=%s\tindex=%ld\tcolour=%d\n",grp,words+iword[grp],groupindex[grp],grpc[grp]);}

	/* CHECK FOR DISCONTIGUOUS X-VALUES IF SETLINEBREAK==2 */
	if(setlinebreak==2) for(ii=1;ii<n1;ii++) if(xdata[(ii-1)]>xdata[ii]) linebreak[ii]=1;

	/* APPLY JITTER TO X-VALUES */
	/* - a little complicated because for each unique xdata, ydata must be copied and jittered xdata copied back */
	if(setjitter>0.0) {
		mm= n1;
		/* allocate memory for index */
		tempindex= realloc(tempindex,(n1*sizeoflong));
		if(tempindex==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1); }
		/* find unique data */
		temp_xunique= xf_unique_d(xdata,&mm,message); //TEST: for(ii=0;ii<mm;ii++) printf("%g\n",temp_xunique[ii]);
		if(temp_xunique==NULL) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		/* for each unique x, get the mean jitter the values */
		for(ii=0;ii<mm;ii++) {
			aa= temp_xunique[ii];
			kk= 0;
			for(jj=0;jj<n1;jj++) {
				if(xdata[jj]==aa) {
					temp_ydata[kk]= ydata[jj];
					tempindex[kk]= jj;
					kk++;
			}}
			/* now generate the jittered x-values */
			tempjit= xf_jitter1_d(temp_ydata,kk,aa,setjitter,message);
			if(tempjit==NULL) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); } //TEST: for(jj=0;jj<kk;jj++) printf("%ld\t%ld\t%g\t%g\n",jj,tempindex[jj],tempjit[jj],temp_ydata[jj]);
			/* copy the jittered x-values back to the original xdata array */
			for(jj=0;jj<kk;jj++) xdata[tempindex[jj]]= tempjit[jj];
			/*release the memory */
			free(tempjit);
		}
		free(tempindex);
		free(temp_xunique);
	}
	//TEST: for(jj=0;jj<n1;jj++) printf("%g\t%g\n",xdata[jj],ydata[jj]);


	/* DETERMINE XMIN,XMAX,YMIN,YMAX, INCLUDING X/Y-ERRORBARS */
	xmin=xmax=xdata[0];
	ymin=ymax=ydata[0];
	if(setecol>0) { ymin-=edata[0]; ymax+=edata[0]; }
	if(setfcol>0) { xmin-=fdata[0]; xmax+=fdata[0]; }
	for(ii=0;ii<n1;ii++){
		if(setecol>0) ee= edata[ii]; else ee= 0.0;
		if(setfcol>0) ff= fdata[ii]; else ff= 0.0;
		aa= ydata[ii]-ee ; if(aa<ymin) ymin=aa;
		aa= ydata[ii]+ee ; if(aa>ymax) ymax=aa;
		aa= xdata[ii]-ff ; if(aa<xmin) xmin=aa;
		aa= xdata[ii]+ff ; if(aa>xmax) xmax=aa;
	}
	/* auto-audjust if there is no variance in the x-data or y-data */
	if(xmin==xmax) { if(xmin==0) {xmin=-1; xmax=1;} else {aa=fabs(xmin/2.0); xmin=xmin-aa; xmax=xmax+aa; if(setxint==0) setxint=aa;} }
	if(ymin==ymax) { if(ymin==0) {ymin=-1; ymax=1;} else {aa=fabs(ymin/2.0); ymin=ymin-aa; ymax=ymax+aa; if(setyint==0) setyint=aa;} }

	/* MIN/MAX MANUAL OVERRIDES */
	/* 1. for box-plots and histograms, determine if bars should "radiate" from zero */
	if(strcmp(plottype,"histo")==0 || strcmp(plottype,"box")==0) {
		if(ymin>0.0 && boxyzero==1) ymin=0.0; // reset ymin to zero if necessary
		if(ymax<0.0 && boxyzero==1) ymax=0.0; // reset ymax to zero if necessary
	}
	/* 2. override min/max if either is very close to zero (less than 1/10 the range) */
	if( xmin>0.0 && xmin<((xmax-xmin)/(10.0)) ) xmin=0.0;
	if( xmax<0.0 && xmax>((xmax-xmin)/(-10.0)) ) xmax=0.0;
	if( ymin>0.0 && ymin<((ymax-ymin)/(10.0)) ) ymin=0.0;
	if( ymax<0.0 && ymax>((ymax-ymin)/(-10.0)) ) ymax=0.0;
	/* 3. override all min/max with command-line settings */
	if(setxmin==1) xmin= setxminval;
	if(setxmax==1) xmax= setxmaxval;
	if(setymin==1) ymin= setyminval;
	if(setymax==1) ymax= setymaxval;
	/* 4. override to accomodate user-lines, if required */
	if(sethline==1 && hlinemin<ymin) ymin=hlinemin;
	if(sethline==1 && hlinemax>ymax) ymax=hlinemax;
	if(setvline==1 && vlinemin<xmin) xmin=vlinemin;
	if(setvline==1 && vlinemax>xmax) xmax=vlinemax;
	/* 5. final check for invalid min-max values - possible if user defines a min or max value outside the data range? */
	if(xmin>xmax) {fprintf(stderr,"\n\a--- Error[%s]: xmin (%g) must be <= xmax (%g)\n\n",thisprog,xmin,xmax);exit(1); }
	if(ymin>ymax) {fprintf(stderr,"\n\a--- Error[%s]: ymin (%g) must be <= ymax (%g)\n\n",thisprog,ymin,ymax);exit(1); }

	/* DETERMINE RANGES, TIC-INTERVALS (OR MANUAL), TIC-PRECISION & PADDING */
	/* determine data ranges based on final decision regarding min and max values, above */
	xrange= xmax-xmin;
	yrange= ymax-ymin;
	/* set tic-intervals based on range, if not manually set */
	if(setxint>0) xint= setxint;
	else xint= xf_trimdigits_d((xrange/5.0),1);
	if(setyint>0) yint= setyint;
	else yint= xf_trimdigits_d((yrange/5.0),1);
	/* set tic-precision based on tic-intervals */
	if(xint>10) xticprecision= 0;
	else xticprecision= xf_precision_d(xint,8);
	if(yint>10) yticprecision= 0;
	else yticprecision= xf_precision_d(yint,8);
	/* determine padding (or manual override) based on tic-intervals */
	if(setxpad<0) setxpad= xint*0.5;
	if(setypad<0) setypad= yint*0.5;
	/* determine actual minimum and maximum tics - to make sure integer-precision doesn't place tics outside plot boundaries */
	xticmin= xmin; xticmax= xmax;
	if(xticprecision==0) {
		xticmin= xf_round1_d(xmin,1,0); if(xticmin<xmin) xticmin+=1.0;
		xticmax= xf_round1_d(xmax,1,0);	if(xticmax>xmax) xticmax-=1.0;
	}
	yticmin= ymin; yticmax= ymax;
	if(yticprecision==0) {
		yticmin= xf_round1_d(ymin,1,0); if(yticmin<ymin) yticmin+=1.0;
		yticmax= xf_round1_d(ymax,1,0); if(yticmax>ymax) yticmax-=1.0;
	}

	/* ADJUST PLOT SIZE AND DETERMINE FACTOR BY WHICH TO ADJUST VALUES FOR PLOTTING */
	xlimit*=xscale;
	ylimit*=yscale;
	xfactor=xlimit/(xrange+2*setxpad);
	yfactor=ylimit/(yrange+2*setypad);

	/* DETERMINE MAXIMUM CHARACTERS IN Y-AXIS TIC-LABELS, FORM MAP OFFSET */
	/* if setyint==-1, there will be no y-tics so the value stays at 0 */
	yticmaxchar=0.0;
	bigtic[0]= '\0';
	if(setyint!=-1) {
		if((ymin<=0. && ymax>0.)||(ymax>=0 && ymin<0.)) {
			for(aa=0;aa>=ymin;aa-=yint) {
				if(yticprecision==0)  sprintf(line,"%ld",(long)aa);
				else sprintf(line,"%.*f",yticprecision,aa);
				z=strlen(line); if(z>(int)yticmaxchar) {yticmaxchar=(float)z;sprintf(bigtic,"%s",line); }
			}
			for(aa=(0+yint);aa<=ymax;aa+=yint) {
				if(yticprecision==0)  sprintf(line,"%ld",(long)aa);
				else sprintf(line,"%.*f",yticprecision,aa);
				z=strlen(line); if(z>(int)yticmaxchar) {yticmaxchar=(float)z;sprintf(bigtic,"%s",line); }
			}
		}
		else {
			for(aa=ymin; aa<=ymax; aa+=yint) {
				if(yticprecision==0)  sprintf(line,"%ld",(long)aa);
				else sprintf(line,"%.*f",yticprecision,aa);
				z=strlen(line); if(z>(int)yticmaxchar) {yticmaxchar=(float)z;sprintf(bigtic,"%s",line); }
			}
		}
	}
	if( yint!=(int)yint) yticmaxchar-=0.75 ; // reduced width for inclusion of a decimal
	if( ymin<0) yticmaxchar-=0.5 ; // reduced width for the negative symbol
	//TEST:fprintf(stderr,"yticmaxchar=%g\n",yticmaxchar); fprintf(stderr,"bigtic=%s\n",bigtic); exit(0);

	/* AUTOMATICALLY SET zx AND zy TO ACCOUNT FOR AXIS LABELS */
	if(zx<0) zx= (-1.0*setticsize) + (fontsize*yticmaxchar) + (fontsize*4);
	if(zy<0) zy= 842-(ylimit + fontsize*4) ; // 842 = y-points in an A4 page

	// SET LIMITS WHICH TAKE INTO ACCOUNT TIC SIZE, AXIS VALUES AND AXIS LABELS
	psxmin=zx-setticsize-fontsize*4.0-(fontsize*2+2.0);
	psymin=zy-setticsize-fontsize*2.0-(fontsize*2.0+2.0);
	psxmax=zx+xlimit+fontsize*4.0;
	psymax=zy+ylimit+fontsize*2.0;

	// DETERMINE X-SHIFTS FOR PLOT TO ALLOW SIDE-BY SIDE PLOTTING OF GROUP DATA, IF REQUIRED
	for(ii=0;ii<=MAXGROUPS;ii++) groupshift[ii]=0.0;
	if(setgshift==1) {
 		x=(int)((float)ngroups/2.0); // number of shifts
 		a=(float)xint/(float)(ngroups+1); // shift between each group
 		b=0; if((ngroups%2)==0) b=a/2.0; // shift value for smallest group-id
		for(jj=0;jj<ngroups;jj++) {groupshift[jj]=b; b+=a; } // set initial shift values for each group index
		for(ii=0;ii<x;ii++) for(jj=0;jj<ngroups;jj++) {groupshift[jj]-=a; } // shifting the shifts (!) backwards to centre on the middle index
 	}


	/* DIAGNOSTIC OUTPUT */
	if(setverb==1) {
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"xmin= %f\n",xmin);
		fprintf(stderr,"xmax= %f\n",xmax);
		fprintf(stderr,"xrange= %f\n",xrange);
		fprintf(stderr,"xint= %f\n",xint);
		fprintf(stderr,"xticprecision= %d\n",xticprecision);
		fprintf(stderr,"xlimit= %f\n",xlimit);
		fprintf(stderr,"xfactor= %f\n",xfactor);
		fprintf(stderr,"zx= %f\n",zx);
		fprintf(stderr,"psxmin= %f\n",psxmin);
		fprintf(stderr,"psxmax= %f\n",psxmax);
		fprintf(stderr,"\n");
		fprintf(stderr,"ymin= %f\n",ymin);
		fprintf(stderr,"ymax= %f\n",ymax);
		fprintf(stderr,"yrange= %f\n",yrange);
		fprintf(stderr,"yint= %f\n",yint);
		fprintf(stderr,"yticprecision= %d\n",yticprecision);
		fprintf(stderr,"xlimit= %f\n",xlimit);
		fprintf(stderr,"yfactor= %f\n",yfactor);
		fprintf(stderr,"zy= %f\n",zy);
		fprintf(stderr,"psymin= %f\n",psymin);
		fprintf(stderr,"psymax= %f\n",psymax);
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
	}

	/******************************************************************************/
	/******************************************************************************/
	/*  NOW PRODUCE THE POSTSCRIPT OUTPUT */
	/******************************************************************************/
	/******************************************************************************/

	/* OPEN POSTSCRIPT FILE */
	if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\n\a--- Error[%s]: file \"%s\" not found\n\n",thisprog,outfile);exit(1); }

	/* PRINT BASIC POSTSCRIPT FILE HEADER */
	fprintf(fpout,"%%!\n");

	/* DEFINE BOUNDING BOX FOR PRINTING, PDF CONVERSION ETC */
	//fprintf(fpout,"%%%%BoundingBox: %g %g %g %g\n",psxmin,psymin,psxmax,psymax);
	fprintf(fpout,"%%%%BoundingBox: 0 0 595 842\n"); // this assumes an A4 page
	/* basic variables */
	fprintf(fpout,"/Helvetica findfont 12.00 scalefont setfont\n");
	fprintf(fpout,"/basefontsize {%g} def\n",fontsize);
	fprintf(fpout,"/ticsize {%g} def\n",setticsize);

	/* DEFINE COLOUR SET */
	fprintf(fpout,"\n%% DEFINE_COLOUR\tGROUP-LABEL\n");
	/* ...if a colour-palette is defined... */

	if(setrgbpal!=NULL) {
		setdatacolour= 0;
		setebright= 0;
		ncolours= ngroups;
		/* adjust number of colours slightly for palettes where the top is close to white */
		if(strcmp(setrgbpal,"magma")==0||strcmp(setrgbpal,"inferno")==0) ncolours= ncolours+1;
		red= realloc(red,ncolours*sizeof(*red));
		green= realloc(green,ncolours*sizeof(*green));
		blue= realloc(blue,ncolours*sizeof(*blue));
		if(red==NULL||green==NULL||blue==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		for(ii=0;ii<ncolours;ii++) red[ii]=green[ii]=blue[ii]=NAN;
		x= xf_palette7(red,green,blue,ncolours,setrgbpal);
		for(ii=0;ii<ngroups;ii++) fprintf(fpout,"/c%ld {%.4f %.4f %.4f} def\t%% %s\n",ii,red[ii],green[ii],blue[ii],words+iword[ii]);
	}
	else {
		ncolours= 32;
		red= realloc(red,ncolours*sizeof(*red));
		green= realloc(green,ncolours*sizeof(*green));
		blue= realloc(blue,ncolours*sizeof(*blue));
		if(red==NULL||green==NULL||blue==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		red[0]=.00; green[0]=.00; blue[0]=.00;
		red[1]=.60; green[1]=.10; blue[1]=.20;
		red[2]=.50; green[2]=.25; blue[2]=.50;
		red[3]=.00; green[3]=.25; blue[3]=.60;
		red[4]=.20; green[4]=.70; blue[4]=.70;
		red[5]=.20; green[5]=.50; blue[5]=.10;
		red[6]=.80; green[6]=.80; blue[6]=.20;
		red[7]=.50; green[7]=.25; blue[7]=.00;

		red[8]=.30; green[8]=.30; blue[8]=.30;
		red[9]=1.0; green[9]=.00; blue[9]=.00;
		red[10]=1.0; green[10]=.00; blue[10]=1.0;
		red[11]=.00; green[11]=.00; blue[11]=1.0;
		red[12]=.00; green[12]=1.0; blue[12]=1.0;
		red[13]=.00; green[13]=1.0; blue[13]=.00;
		red[14]=1.0; green[14]=1.0; blue[14]=.00;
		red[15]=1.0; green[15]=.40; blue[15]=.10;

		red[16]=.50; green[16]=.50; blue[16]=.50;
		red[17]=1.0; green[17]=.50; blue[17]=.50;
		red[18]=1.0; green[18]=.50; blue[18]=1.0;
		red[19]=.50; green[19]=.60; blue[19]=1.0;
		red[20]=.50; green[20]=1.0; blue[20]=1.0;
		red[21]=.50; green[21]=1.0; blue[21]=.50;
		red[22]=1.0; green[22]=1.0; blue[22]=.50;
		red[23]=1.0; green[23]=.70; blue[23]=.30;

		red[24]=.75; green[24]=.75; blue[24]=.75;
		red[25]=1.0; green[25]=.75; blue[25]=.75;
		red[26]=1.0; green[26]=.75; blue[26]=1.0;
		red[27]=.75; green[27]=.80; blue[27]=1.0;
		red[28]=.75; green[28]=1.0; blue[28]=1.0;
		red[29]=.75; green[29]=1.0; blue[29]=.75;
		red[30]=1.0; green[30]=1.0; blue[30]=.75;
		red[31]=.90; green[31]=.90; blue[31]=.90;

		for(ii=0;ii<ncolours;ii++) {
			fprintf(fpout,"/c%ld {%.4f %.4f %.4f} def\t%% %s\n",ii,red[ii],green[ii],blue[ii],words+iword[ii]);
		}
	}
	fprintf(fpout,"/cw {1. 1. 1.} def   %% open_points_fill\n");
	fprintf(fpout,"/cf {.0 .0 .0} def   %% frame_colour\n");
	fprintf(fpout,"/ct {.5 .5 .5} def   %% title colour\n");


	/* DEFINE AXIS FUNCTIONS - TICS, LABELS, TITLES */
	fprintf(fpout,"\n%% DEFINE_AXIS_FUNCTIONS\n");
	/* tics */
	fprintf(fpout,"ticsize 0 lt {/ticout ticsize def} {/ticout 0 def} ifelse \n");
	fprintf(fpout,"/xtic { newx dup 0 exch 0 moveto 0 ticsize rlineto stroke xticlabel} def\n");
	fprintf(fpout,"/ytic { newy 0 exch 1 index 1 index moveto ticsize 0 rlineto stroke yticlabel } def\n");
	/* tics-labels: offsets determined by font and tic-size, and whether tics are positive or negative */
	if(setxint==-1) fprintf(fpout,"/xtloff { basefontsize -0.6 mul } def\n");
	else            fprintf(fpout,"/xtloff { basefontsize -1.0 mul ticout add } def\n");
	if(setyint==-1) fprintf(fpout,"/ytloff { basefontsize -0.15 mul } def\n");
	else            fprintf(fpout,"/ytloff { (%s) stringwidth pop -.5 mul ticout add } def\n",bigtic);
	fprintf(fpout,"/xticlabel { moveto dup stringwidth pop 2 div neg xtloff rmoveto show } def\n");
	fprintf(fpout,"/yticlabel { moveto dup stringwidth pop neg ytloff add -0.25 basefontsize mul rmoveto show } def\n");
	/* axis-labels */
	fprintf(fpout,"/xaxislabel { moveto dup stringwidth pop 2 div neg 2 xtloff mul rmoveto show } def\n");
	fprintf(fpout,"/yaxislabel { 90 rotate moveto dup stringwidth pop 2 div neg ytloff -3 mul rmoveto show -90 rotate } def\n");
	fprintf(fpout,"/f_plottitle { ct setrgbcolor basefontsize .5 mul add moveto ytloff 4 mul  basefontsize 2 div rmoveto show cf setrgbcolor } def\n");

	/* DEFINE LEGEND FUNCTION */
	/* plot-legend function - includes a coloured square and the label for the group */
	/* - make sure a complete function is generated, even if technically no legend is required */
	fprintf(fpout,"/f_plotlegend {\n");
	fprintf(fpout,"	setrgbcolor\n");
	fprintf(fpout,"	2 copy\n");
	fprintf(fpout,"\n");
	if(setlegend==0) fprintf(fpout,"\tytloff 4 mul xtloff 2 mul moveto\n");
	if(setlegend==1) fprintf(fpout,"\tytloff 4 mul xtloff 2 mul moveto\n");
	if(setlegend==2) fprintf(fpout,"\t%g %g moveto\n",xlimit,ylimit);
	fprintf(fpout,"\tbasefontsize mul -1 mul rmoveto\n");
	fprintf(fpout,"\t0 pointsize rlineto\n");
	fprintf(fpout,"\tpointsize 0 rlineto\n");
	fprintf(fpout,"\t0 pointsize neg rlineto\n");
	fprintf(fpout,"\tpointdraw\n");
	fprintf(fpout,"\n");
	if(setlegend==0) fprintf(fpout,"\tytloff 4 mul pointsize 2 mul add xtloff 2 mul moveto\n");
	if(setlegend==1) fprintf(fpout,"\tytloff 4 mul pointsize 2 mul add xtloff 2 mul moveto\n");
	if(setlegend==2) fprintf(fpout,"\t%g basefontsize add %g moveto\n",xlimit,ylimit);
	fprintf(fpout,"	basefontsize mul -1 mul rmoveto\n");
	fprintf(fpout,"	show\n");
	fprintf(fpout,"	cf setrgbcolor\n");
	fprintf(fpout,"} def\n");


	/* DEFINE_DATA_DRAWING_FUNCTIONS */
	fprintf(fpout,"\n%% DEFINE_DATA_DRAWING_FUNCTIONS\n");
	/* define plot commands for lines connecting points */
	fprintf(fpout,"/line {\n");
	fprintf(fpout,"	newpoint lineto\n");
	fprintf(fpout,"	} def \n");
	fprintf(fpout,"\n");
	/* define plot commands for histogram (vertical bars) */
	fprintf(fpout,"/histo {\n");
	fprintf(fpout,"	newpoint moveto\n");
	fprintf(fpout,"	currentpoint pop boxyzero lineto\n");
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"	} def \n");
	fprintf(fpout,"\n");
	/* define plot commands for horizontal bar */
	fprintf(fpout,"/bar	{\n");
	fprintf(fpout,"	newpoint moveto\n");
	fprintf(fpout,"	boxwidth 2 div neg 0 rmoveto boxwidth 0 rlineto\n");
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"	} def\n");
	fprintf(fpout,"\n");
	/* define plot commands for boxplot - filled or open */
	fprintf(fpout,"/box	{\n");
	fprintf(fpout,"	newpoint moveto\n");
	fprintf(fpout,"	boxwidth 2 div neg 0 rmoveto boxwidth 0 rlineto\n");
	fprintf(fpout,"	currentpoint pop boxyzero lineto\n");
	fprintf(fpout,"	boxwidth neg 0 rlineto\n");
	fprintf(fpout,"	pointdraw\n");
	fprintf(fpout,"	} def\n");
	fprintf(fpout,"\n");
	/* define plot commands for white-filled boxplot */
	fprintf(fpout,"/boxw	{\n");
	fprintf(fpout,"	newpoint moveto\n");
	fprintf(fpout,"	boxwidth 2 div neg 0 rmoveto boxwidth 0 rlineto\n");
	fprintf(fpout,"	currentpoint pop boxyzero lineto\n");
	fprintf(fpout,"	boxwidth neg 0 rlineto\n");
	fprintf(fpout,"	closepath\n");
	fprintf(fpout,"	gsave\n");
	fprintf(fpout,"	1.0 setgray fill\n");
	fprintf(fpout,"	grestore\n");
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"	} def\n");
	fprintf(fpout,"\n");
	/* define plot commands for square - filled or open */
	fprintf(fpout,"/squ	{\n");
	fprintf(fpout,"	newpoint moveto\n");
	fprintf(fpout,"	pointsize 2 div neg pointsize 2 div neg rmoveto\n");
	fprintf(fpout,"	0 pointsize rlineto\n");
	fprintf(fpout,"	pointsize 0 rlineto\n");
	fprintf(fpout,"	0 pointsize neg rlineto\n");
	fprintf(fpout,"	pointdraw\n");
	fprintf(fpout,"	} def\n");
	fprintf(fpout,"\n");
	/* define plot commands for white-filled square */
	fprintf(fpout,"/squw	{\n");
	fprintf(fpout,"	newpoint moveto\n");
	fprintf(fpout,"	pointsize 2 div neg pointsize 2 div neg rmoveto\n");
	fprintf(fpout,"	0 pointsize rlineto\n");
	fprintf(fpout,"	pointsize 0 rlineto\n");
	fprintf(fpout,"	0 pointsize neg rlineto\n");
	fprintf(fpout,"	closepath\n");
	fprintf(fpout,"	gsave\n");
	fprintf(fpout,"	1.0 setgray fill\n");
	fprintf(fpout,"	grestore\n");
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"	} def\n");
	fprintf(fpout,"\n");
	/* define plot commands for triangle - filled or open */
	fprintf(fpout,"/tri {\n");
	fprintf(fpout,"	newpoint moveto\n");
	fprintf(fpout,"	pointsize 2 div neg pointsize 2 div neg rmoveto\n");
	fprintf(fpout,"	pointsize 2 div pointsize rlineto\n");
	fprintf(fpout,"	pointsize 2 div pointsize neg rlineto\n");
	fprintf(fpout,"	pointdraw\n");
	fprintf(fpout,"	} def\n");
	fprintf(fpout,"\n");
	/* define plot commands for white-filled triangle */
	fprintf(fpout,"/triw {\n");
	fprintf(fpout,"	newpoint moveto\n");
	fprintf(fpout,"	pointsize 2 div neg pointsize 2 div neg rmoveto\n");
	fprintf(fpout,"	pointsize 2 div pointsize rlineto\n");
	fprintf(fpout,"	pointsize 2 div pointsize neg rlineto\n");
	fprintf(fpout,"	closepath\n");
	fprintf(fpout,"	gsave\n");
	fprintf(fpout,"	1.0 setgray fill\n");
	fprintf(fpout,"	grestore\n");
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"	} def\n");
	fprintf(fpout,"\n");
	/* define plot commands for circle - filled or open */
	fprintf(fpout,"/cir {\n");
	fprintf(fpout,"	newpoint pointsize 2 div 0 360 arc\n");
	fprintf(fpout,"	pointdraw\n");
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"	} def\n");
	fprintf(fpout,"\n");
	/* define plot commands for white-filled circle */
	fprintf(fpout,"/cirw {\n");
	fprintf(fpout,"	newpoint pointsize 2 div 0 360 arc\n");
	fprintf(fpout,"	closepath\n");
	fprintf(fpout,"	gsave\n");
	fprintf(fpout,"	1.0 setgray fill\n");
	fprintf(fpout,"	grestore\n");
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"	} def\n");
	fprintf(fpout,"\n");

	/* define plot commands for y-error-bars */
	fprintf(fpout,"/E	{\n");
	fprintf(fpout,"	newpath\n");
	fprintf(fpout,"	newpoint moveto\n");    // move to x/y coordinates - e remains
	fprintf(fpout,"	dup\n");                // duplicate z (height of error bar)
	fprintf(fpout,"	0 exch rmoveto\n");     // move to top of errorbar (takes 1 of the e-values, leaves 1)
	fprintf(fpout,"	ewidth 2 div neg 0 rmoveto ewidth 0 rlineto\n"); // draw top of error bar
	fprintf(fpout,"	ewidth 2 div neg 0 rmoveto\n"); // return to x-middle of errorbar
	fprintf(fpout,"	0 exch neg 2 mul rlineto\n"); // draw vertical line using remaining e-value
	fprintf(fpout,"	ewidth 2 div neg 0 rmoveto ewidth 0 rlineto\n");  // draw bottom of errorbar
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"	} def\n");
	/* define plot commands for x-error-bars */
	fprintf(fpout,"/F	{\n");
	fprintf(fpout,"	newpath\n");
	fprintf(fpout,"	newpoint moveto\n");    // move to x/y coordinates - f remains
	fprintf(fpout,"	dup\n");                // duplicate f (width of error)
	fprintf(fpout,"	neg 0 rmoveto\n");     // move to left of errorbar (takes 1 of the z-values, leaves 1)
	fprintf(fpout,"	fwidth 2 div neg 0 exch rmoveto 0 fwidth rlineto\n"); // draw left of error bar
	fprintf(fpout,"	fwidth 2 div neg 0 exch rmoveto\n"); // return to x-middle of errorbar
	fprintf(fpout,"	2 mul 0 rlineto\n"); // draw horizontal line using remaining f-value
	fprintf(fpout,"	fwidth 2 div neg 0 exch rmoveto 0 fwidth rlineto\n");  // draw right of errorbar
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"	} def\n");


	/* SHIFT COORDINATES FOR THIS PLOT */
	fprintf(fpout,"\n%% SHIFT_COORDINATES\n");
	fprintf(fpout,"%.14g %.14g translate\n",zx,zy);

 	fprintf(fpout,"\n%% PLOT_CODE_START\n");
	/* DEFINE PLOT-SPECIFIC PARAMETERS */
	fprintf(fpout,"\n%% DEFINE_PLOT_PARAMETERS\n");
	fprintf(fpout,"/xoffset {%.14g} def\n",(setxpad-xmin)*xfactor);
	fprintf(fpout,"/yoffset {%.14g} def\n",(setypad-ymin)*yfactor);
	fprintf(fpout,"/newx {%.14g mul xoffset add } def\n",xfactor);
	fprintf(fpout,"/newy {%.14g mul yoffset add } def\n",yfactor);
	fprintf(fpout,"/newpoint {newy exch newx exch} def\n");
	fprintf(fpout,"/xlabel {(%s)} def\n",xlabel);
	fprintf(fpout,"/ylabel {(%s)} def\n",ylabel);
	fprintf(fpout,"/plottitle {(%s)} def\n",plottitle);

	/* DEFINE DATASET-RELATED PARAMETERS */
	fprintf(fpout,"\n%% DEFINE_DATA_RELATED_VALUES\n");
	fprintf(fpout,"/linewidth_data {%.14g} def\n",lwdata);
	fprintf(fpout,"/linewidth_error {%.14g} def\n",lwerror);
	fprintf(fpout,"/linewidth_axes {%.14g} def\n",lwaxes);
	fprintf(fpout,"/L { line } def \n");
	if(pointfill==-1) fprintf(fpout,"/P { %s%c } def\n",plottype,'w');
	else fprintf(fpout,"/P { %s } def\n",plottype);
	fprintf(fpout,"\n");
	fprintf(fpout,"/pointsize {%.14g} def\n",pointsize);
	if(pointfill==0) fprintf(fpout,"/pointdraw { closepath } def\n"); // points are not filled (transparent centre)
	else fprintf(fpout,"/pointdraw { fill } def\n");       // points filled with colour, -1 = white, +1 = data colour
	fprintf(fpout,"/ewidth { %.14g } def \n",ewidth*xint*xfactor);
	fprintf(fpout,"/fwidth { %.14g } def \n",ewidth*yint*yfactor);
	fprintf(fpout,"/boxwidth { %.14g } def \n",boxwidth*xint*xfactor);
	fprintf(fpout,"/setboxwidth { %.14g } def \n",boxwidth*xint);
	if(boxyzero==0) fprintf(fpout,"/boxyzero {%.14g} def\n",(lwaxes/2.0)); // boxes and histograms run to x-axis, plus width of frame
	else if(boxyzero==1) fprintf(fpout,"/boxyzero {0 newy} def\n");     // boxes and histograms run to zero


	/********************************************************************************/
	/* PLOT DATA */
	/********************************************************************************/
	if(setrgbpal!=NULL) maxcolour= ngroups-1;
	else maxcolour= 32-1;

	/* FOR EACH GROUP */
	for(grp=0;grp<MAXGROUPS;grp++) {
		if(grpcount[grp]<1) continue;

		/* determine colours for data & lines (tempcolour1) and errorbars (tempcolour2) */
		kk= grpc[grp]+setdatacolour;
		tempcolour1= xf_scale1_l(kk,0,maxcolour); // ensure colours stay within range, even if modified by -colour
		tempcolour2= tempcolour1+setebright;
		if(tempcolour2>maxcolour)  tempcolour2-=setebright-8;

		fprintf(fpout,"\n%% PLOT_VALUES_GROUP_%d\n",grp);
		/* make a temporary copy of x,y,z and group variables */
		nn=0;for(ii=0;ii<n1;ii++) {
			aa=xdata[ii];
			bb=ydata[ii];
			if(group[ii]==grp) {
				temp_xdata[nn]=aa+groupshift[groupindex[grp]];
				temp_ydata[nn]=bb;
				if(setecol>0) temp_edata[nn]= edata[ii];
				if(setfcol>0) temp_fdata[nn]= fdata[ii];
				temp_linebreak[nn]= linebreak[ii];
				nn++;
		}}

		fprintf(fpout,"\t%% PLOT_Y-ERRORBARS\n");
		if(setecol>0) {
			/* set colour for error-bars - adjust to brighter if required */
			fprintf(fpout,"\tc%ld setrgbcolor\n",tempcolour2);
			fprintf(fpout,"	linewidth_error setlinewidth\n");
			fprintf(fpout,"	newpath\n");
			// first go to first in-range data point
			fprintf(fpout,"\t%.14g %.14g newpoint moveto\n",temp_xdata[0],temp_ydata[0]);
			// now plot the rest
			for(ii=0;ii<nn;ii++) fprintf(fpout,"\t%.14g %.14g %.14g E\n",temp_edata[ii]*yfactor,temp_xdata[ii],temp_ydata[ii]);
			fprintf(fpout,"	stroke\n");
			fprintf(fpout,"	closepath\n");
		}
		fprintf(fpout,"\t%% PLOT_X-ERRORBARS\n");
		if(setfcol>0) {
			/* set colour for error-bars - adjust to brighter if required */
			fprintf(fpout,"\tc%ld setrgbcolor\n",tempcolour2);
			fprintf(fpout,"	linewidth_error setlinewidth\n");
			fprintf(fpout,"	newpath\n");
			// first go to first in-range data point
			fprintf(fpout,"\t%.14g %.14g newpoint moveto\n",temp_xdata[0],temp_ydata[0]);
			// now plot the rest
			for(ii=0;ii<nn;ii++) fprintf(fpout,"\t%.14g %.14g %.14g F\n",temp_fdata[ii]*xfactor,temp_xdata[ii],temp_ydata[ii]);
			fprintf(fpout,"	stroke\n");
			fprintf(fpout,"	closepath\n");
		}

		fprintf(fpout,"\t%% PLOT_LINES\n");
		/* set colour for data */
		fprintf(fpout,"\tc%ld setrgbcolor\n",tempcolour1);
		if(setline>0) {
			fprintf(fpout,"\tlinewidth_data setlinewidth\n");
			fprintf(fpout,"\tnewpath\n");
			// first go to first in-range data point
			fprintf(fpout,"\t%.14g %.14g newpoint moveto\n",temp_xdata[0],temp_ydata[0]);
			// now plot the rest
			for(ii=0;ii<nn;ii++) {
				if(temp_linebreak[ii]==1) {
					fprintf(fpout,"	stroke\n");
					fprintf(fpout,"	closepath\n");
					fprintf(fpout,"	newpath\n");
					fprintf(fpout,"\t%.14g %.14g newpoint moveto\n",temp_xdata[ii],temp_ydata[ii]);
				}
				fprintf(fpout,"\t%.14g %.14g L\n",temp_xdata[ii],temp_ydata[ii]);
				dd=temp_xdata[ii];
			}
			fprintf(fpout,"	stroke\n");
			fprintf(fpout,"	closepath\n");
		}

		fprintf(fpout,"	%% PLOT_POINTS\n");
		/* set colour for data */
		fprintf(fpout,"\tc%ld setrgbcolor\n",tempcolour1);
		if(pointsize>0) {
			fprintf(fpout,"	linewidth_data setlinewidth\n");
			fprintf(fpout,"	newpath\n");
			// plot the points (no need to move to start position first
			for(ii=0;ii<nn;ii++) fprintf(fpout,"\t%.14g %.14g P\n",temp_xdata[ii],temp_ydata[ii]);
			fprintf(fpout,"	stroke\n");
			fprintf(fpout,"	closepath\n");
		}

	}

	/* DRAW FRAME */
	fprintf(fpout,"\n%% DRAW_FRAME\n");
	fprintf(fpout,"cf setrgbcolor\n");
	fprintf(fpout,"linewidth_axes setlinewidth\n");
	if(framestyle>0) {
		if ((int)(framestyle&1)>0) f1=1;
		if ((int)(framestyle&2)>0) f2=1;
		if ((int)(framestyle&4)>0) f3=1;
		if ((int)(framestyle&8)>0) f4=1;
		fprintf(fpout,"newpath\n");
		if (f1==1) {
			fprintf(fpout,"%.14g 0 moveto 0 0 lineto\n",xlimit);
			if (f2==0) fprintf(fpout,"stroke\n");
		}
		if (f2==1) {
			if(f1==0) fprintf(fpout,"newpath 0 0 moveto\n");
			fprintf(fpout,"0 %.14g lineto\n",ylimit);
			if(f3==0) fprintf(fpout,"stroke\n");
		}
		if (f3==1) {
			if(f2==0) fprintf(fpout,"newpath 0 %.14g moveto\n",ylimit);
			fprintf(fpout,"%.14g %.14g lineto\n",xlimit,ylimit);
			if(f4==0) fprintf(fpout,"stroke\n");
		}
		if (f4==1){
			if(f3==0) fprintf(fpout,"newpath %.14g %.14g moveto\n",xlimit,ylimit);
			fprintf(fpout,"%.14g 0 lineto \n",xlimit);
			if(f1==0) fprintf(fpout,"stroke\n");
			else fprintf(fpout,"0 0 lineto stroke\n");
		}
		fprintf(fpout,"stroke\n");
	}


	/* DRAW YZERO-LINE IF REQUIRED */
	fprintf(fpout,"\n%% DRAW_YZERO_LINE\n");
	if(setyzeroline==1 && ymin<0&&ymax>0) {
		fprintf(fpout,"\n%% DRAW_YZERO_LINE\n");
		fprintf(fpout,"cf setrgbcolor\n");
		fprintf(fpout,"	linewidth_axes setlinewidth\n");
		fprintf(fpout,"newpath 0 0 newy moveto %.14g 0 rlineto\n",xlimit);
		fprintf(fpout,"stroke\n");
		fprintf(fpout,"\n");
	}

	/********************************************************************************/
	/* DRAW TICS AND TIC-LABELS */
	/********************************************************************************/
	fprintf(fpout,"\n%% DRAW_TICS_AND_LABELS\n");
	fprintf(fpout,"/Helvetica findfont basefontsize scalefont setfont\n");
	fprintf(fpout,"cf setrgbcolor\n");
	fprintf(fpout,"	linewidth_axes setlinewidth\n");

	// XTICS: if zero is in the range, make sure zero is included as a tic mark
	if(setxint>=0) {
		fprintf(fpout,"newpath\n");
		if((xmin<=0 && xmax>0)||(xmax>=0 && xmin<0)) {
			for(aa=0;aa>=(xticmin-setxpad);aa-=xint) {
				if(xticprecision==0) fprintf(fpout,"(%ld) %ld xtic\n",(long)aa,(long)aa);
				else fprintf(fpout,"(%.*f) %.*f xtic\n",xticprecision,aa,xticprecision,aa);
			}
			for(aa=(0+xint);aa<=(xticmax+setxpad);aa+=xint) {
				if(xticprecision==0) fprintf(fpout,"(%ld) %ld xtic\n",(long)aa,(long)aa);
				else fprintf(fpout,"(%.*f) %.*f xtic\n",xticprecision,aa,xticprecision,aa);
			}
		}
		else for(aa=xticmin; aa<=(xticmax+setxpad); aa+=xint) {
			if(xticprecision==0) fprintf(fpout,"(%ld) %ld xtic\n",(long)aa,(long)aa);
			else fprintf(fpout,"(%.*f) %.*f xtic\n",xticprecision,aa,xticprecision,aa);
		}
		fprintf(fpout,"stroke\n");
	}
	// YTICS: if zero is in the range, make sure zero is included as a tic mark
	if(setyint>=0) {
		fprintf(fpout,"newpath\n");
		yticmaxchar=0.0;
		if((ymin<=0 && ymax>0)||(ymax>=0 && ymin<0)) {
			/* do numbers <0 */
			for(aa=0;aa>=(yticmin-setypad);aa-=yint) {
				if(yticprecision==0) fprintf(fpout,"(%ld) %ld ytic\n",(long)aa,(long)aa);
				else fprintf(fpout,"(%.*f) %.*f ytic\n",yticprecision,aa,yticprecision,aa);
			}
			/* do numbers >0 */
			for(aa=(0+yint);aa<=(yticmax+setypad);aa+=yint) {
				if(yticprecision==0) fprintf(fpout,"(%ld) %ld ytic\n",(long)aa,(long)aa);
				else fprintf(fpout,"(%.*f) %.*f ytic\n",yticprecision,aa,yticprecision,aa);
			}
		}
		else {
			for(aa=yticmin; aa<=(yticmax+setypad); aa+=yint) {
				if(yticprecision==0) fprintf(fpout,"(%ld) %ld ytic\n",(long)aa,(long)aa);
				else fprintf(fpout,"(%.*f) %.*f ytic\n",yticprecision,aa,yticprecision,aa);
			}
		}
		fprintf(fpout,"stroke\n");
	}
	fprintf(fpout,"\n");



	/* DRAW AXIS LABELS - 2 font sizes larger than basefont, centred on each axis */
	fprintf(fpout,"\n%% DRAW_AXIS_LABELS\n");
	fprintf(fpout,"/Helvetica-Bold findfont basefontsize 2 add scalefont setfont\n");
	fprintf(fpout,"xlabel %.14g 0 xaxislabel\n",xlimit/2.0);
	fprintf(fpout,"ylabel %.14g 0 yaxislabel\n",ylimit/2.0);

	/* DRAW PLOT TITLE - 4 font sizes larger than basefont */
	fprintf(fpout,"\n%% DRAW_PLOT_TITLE\n");
	fprintf(fpout,"/Helvetica-Bold findfont basefontsize 4 add scalefont setfont\n");
	fprintf(fpout,"plottitle 0 %.14g f_plottitle\n",ylimit);

	/* DRAW PLOT LEGEND - lower-left or upper-right of plot */
	fprintf(fpout,"\n%% DRAW_PLOT_LEGEND\n");
	if(setlegend>0) {
		fprintf(fpout,"/Helvetica-Bold findfont basefontsize 0.75 mul scalefont setfont\n");
		for(z=grp=0;grp<=MAXGROUPS;grp++) {
			if(grpcount[grp]<1) continue;
			fprintf(fpout,"(%s) 0 %d c%d f_plotlegend\n",(words+iword[grp]),++z,tempcolour1);
	}}

	/* DRAW USER-DEFINED HORIZONTAL LINES, IF REQUIRED */
	/* Note: this uses the xe-plottable1 "line" function to position at the correct coordinates*/
	fprintf(fpout,"\n%% PLOT_USERLINE_HORIZONTAL\n");
	if(sethline==1) {
		fprintf(fpout,"linewidth_axes setlinewidth\n");
		for(ii=0;ii<MAXUSERLINES;ii++) {
			if(isfinite(hline[ii])) {
				fprintf(fpout,"newpath\n");
				fprintf(fpout,"%.14g %.14g newpoint moveto\n",(xmin-setxpad),hline[ii]);
				fprintf(fpout,"%.14g %.14g line\n",(xmax+setxpad),hline[ii]);
				fprintf(fpout,"[3] 0 setdash stroke\n");
				fprintf(fpout,"closepath\n\n");
	}}}



	/* DRAW USER-DEFINED VERTICAL LINES, IF REQUIRED */
	/* Note: this uses the xe-plottable1 "line" function to position at the correct coordinates*/
	fprintf(fpout,"\n%% PLOT_USERLINES_VERTICAL\n");
	if(setvline==1) {
		fprintf(fpout,"linewidth_axes setlinewidth\n");
		for(ii=0;ii<MAXUSERLINES;ii++) {
			if(isfinite(vline[ii])) {
				fprintf(fpout,"newpath\n");
				fprintf(fpout,"%.14g %.14g newpoint moveto\n",vline[ii],(ymin-setypad));
				fprintf(fpout,"%.14g %.14g line\n",vline[ii],(ymax+setypad));
				fprintf(fpout,"[3] 0 setdash stroke\n");
				fprintf(fpout,"closepath\n\n");
	}}}



	fprintf(fpout,"\n%% PLOT_CODE_END\n");
 	fprintf(fpout,"\n%% SHOW_PAGE\n");
	fprintf(fpout,"showpage\n");
	fclose(fpout);

	free(xdata);
	free(ydata);
	free(edata);
	free(fdata);
	free(group);
	free(temp_xdata);
	free(temp_ydata);
	free(temp_edata);
	free(temp_fdata);
	free(linebreak);
	free(temp_linebreak);
	free(words);
	free(hlineword);
	free(vlineword);

	if(red!=NULL)free(red);
	if(green!=NULL) free(green);
	if(blue!=NULL) free(blue);

	exit(0);
}
