/***********************************************************************
Calculate an array of path-length values using Bob Muller's "cord" method
- define size of window
- position data is reduced to a series of cords connecting points at start & end of the window
- if style = "jump", windows are non-overlapping
- if style = "sliding", windows are overlapping with single-sample increments 
- NOTE: jump method is probably better for calculating path lengths
- path length at each position sample = distance travelled since last sample
	- jump method: path is the same fixed fraction of the length of the cord for all samples in that cord
	- sliding method: each sample has a unique cord, & path is a fraction of that cord length
************************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

void hux_error(char message[]);
int hux_pospath (
						 double *pos_time,	/* array of position sample timestamps */ 
						 float *pos_x,		/* array of x-position values*/
						 float *pos_y,		/* array of y-position values*/
						 float *pos_path,	/* target array of path segment values */
						 int postot,		/* total number of position samples */
						 int winsize,		/* size (samples) of window to integrate position */
						 char *style,		/* behaviour of window, "sliding" or "jump" */
						 int posinvalid,	/* invalid value of position data to be ignored */
						 int pathinvalid	/* invalid value for path array (usually same as posinvalid) */
						 )
{ 
	int i,y,winstart,jumpsize,pathtot=0;
	float factor;
	double a,b,c,d,e;

	/* error if window size = 1 sample only or the same size as entire position record */
	if(winsize<2) hux_error("hux_pospath: integration window is less than 2 samples");
	if(winsize>=postot) hux_error("hux_pospath: position integration window exeeds no. of position samples");

	if(strcmp(style,"sliding")==0)   {jumpsize=0; factor=(float)winsize;} /* amount to divide path segemnts by depends on integration method */
	else if(strcmp(style,"jump")==0) {jumpsize=winsize;	factor=(float)(winsize+1.0);}
	else hux_error("hux_pospath: \"style\" argument must be either \"sliding\" or \"jump\"");

	for(i=0;i<postot;i++) pos_path[i]=posinvalid; /* initialise path values to invalid */

	for(i=winsize;i<postot;i++) { /* start at n=winsize samples into position record */
		winstart=i-winsize; /* winstart = sample corresponding with end of window: cannot exceed postot */
		if(pos_x[i]==posinvalid || pos_x[winstart]==posinvalid) continue; /* if sample at either end of window is invalid, skip ahead one sample */
		else {
			a = pos_x[i] - pos_x[winstart]; /* change in x */
			b = pos_y[i] - pos_y[winstart]; /* change in y */
			c = sqrt(a*a + b*b); /* path segment */
			d = c/factor; /* portion of path segment attributable to single sample in window (eg: if winsize=5, "d" is assigned to 6 samples) */
			/* fill window (type=jump) or current sample (type=sliding) with this velocity */
			for(y=i;y>=i-jumpsize;y--) pos_path[y] = d;
			pathtot+=(jumpsize+1); /* total number of valid path segments calculated */
			i += jumpsize; /* increment counter by winsize, if type=jump (if type=sliding, window is sliding) */
		}
	}
	return(pathtot);
}

