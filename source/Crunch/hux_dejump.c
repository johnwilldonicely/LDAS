/********************************************************************************
hux_dejump: John Huxter, May 2005
- remove jumpy points from position record
- requires... 
	- pointers to 1st element of x (float),y (float), and time (double) arrays
	- size of position arrays
	- invalid sample value (int) 
	- maximum allowable speed (jumpthresh)
- action...
	- changes jumpy points in x and y arrays to invalid value
- assumptions...
	- time=seconds, x & y = cm, if x is invalid, y is invalid too
- two reasons for "jumpy" points
	1) momentary mistracking caused by reflections etc.
	2) valid tracking following a period of lost tracking
- therefore this function only labels "jumpy" points invalid until enough time has 
	elapsed for the subject to have theoretically moved to the "jumpy" position
- returns number of jumpy points invalidated
*********************************************************************************/
#include <math.h>
int hux_dejump (double *t, float *x, float *y, int tot, int invalid, float jumpthresh)
{
	int i=0,jumps=0;
	float prevx,prevy,dx,dy;
	double prevt,dt,speed;
	while(x[i]==invalid) i++; /* seek to first valid position sample (first reference)*/
	prevx=x[i]; prevy=y[i]; prevt=t[i];

	while(i<tot) {
		i++;
		if(x[i]==invalid) continue;
		else {
			/* calculate delta values for x,y and time */
			dx=x[i]-prevx; dy=y[i]-prevy;	dt=t[i]-prevt;
			/* calculate momentary speed based on time & distance from last valid position */
			speed=sqrt(dx*dx + dy*dy)/dt;
			if(speed>jumpthresh){
				x[i]=y[i]=(float) invalid; /* invalidate current sample if it is "jumpy */
				jumps++;
			}
			else {
				/* define new reference sample if current point is not jumpy */
				prevx=x[i]; prevy=y[i]; prevt=t[i];
	}}}
	return (jumps);
}
