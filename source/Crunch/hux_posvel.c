/***********************************************************************
Calculate an array of velocity values using Bob Muller's "cord" method
	- define size of window
	- position data is reduced to a series of cords connecting points at start & end of the window
	- if style = "jump", windows are non-overlapping
	- if style = "sliding", windows are overlapping with single-sample increments 
	- NOTE: sliding method is recommended for velocity calculations
************************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

void hux_error(char message[]);
int hux_posvel (
						 double *pos_time,	/* array of position sample timestamps */ 
						 float *pos_x,		/* array of x-position values*/
						 float *pos_y,		/* array of y-position values*/
						 float *pos_vel,	/* target array of path segment values */
						 int postot,		/* total number of position samples */
						 int winsize,		/* size (samples) of window to integrate position */
						 char *style,		/* behaviour of window, "sliding" or "jump" */
						 int posinvalid,	/* invalid value of position data to be ignored */
						 int velinvalid		/* invalid velocity value */
						 )
{ 
	int i,y,winstart,jumpsize,veltot=0;
	float factor;
	double a,b,c,d;

	/* error if window size = 1 sample only or the same size as entire position record */
	if(winsize<2) hux_error("hux_posvel: integration window is less than 2 samples");
	if(winsize>=postot) hux_error("hux_posvel: position integration window exeeds no. of position samples");

	if(strcmp(style,"sliding")==0)   {jumpsize=0; factor=(float)winsize;} /* amount to divide path segemnts by depends on integration method */
	else if(strcmp(style,"jump")==0) {jumpsize=winsize;	factor=(float)(winsize+1.0);}
	else hux_error("hux_posvel: \"style\" argument must be either \"sliding\" or \"jump\"");

	for(i=0;i<postot;i++) pos_vel[i]=posinvalid; /* initialise velocity values to invalid */

	for(i=winsize;i<postot;i++) { /* start at n=winsize samples into position record */
		winstart=i-winsize; /* winstart = sample corresponding with end of window: cannot exceed postot */
		if(pos_x[i]==posinvalid || pos_x[winstart]==posinvalid) continue; /* if sample at either end of window is invalid, skip ahead one sample */
		else {
			a = pos_x[i] - pos_x[winstart]; /* change in x */
			b = pos_y[i] - pos_y[winstart]; /* change in y */
			c = sqrt(a*a + b*b); /* path segment */
			d = c/(pos_time[i] - pos_time[winstart]); /* velocity */
			for(y=i;y>=i-jumpsize;y--) pos_vel[y] = d; /* fill window (type=jump) or current sample (type=sliding) with this velocity */
			veltot+=(jumpsize+1); /* total number of valid path segments calculated */
			i += jumpsize; /* increment counter by winsize, if type=jump (if type=sliding, window is sliding) */
		}
	}
	return(veltot);
}

