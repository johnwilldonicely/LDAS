/***********************************************************************
Convert a movement in a given direction to cartesian translation
************************************************************************/
#include <math.h>
void hux_angletocart (
	float angle,	/* direction of motion */ 
	float dist,	/* distance in direction to be converted */ 
	float *result	/* array to hold results */
)
{
float xshift,yshift;
angle*=0.01745329251994329577; /* multiply by pi/180 to convert to radians */
result[0] = dist * cos(angle); /* x-shift */
result[1] = dist * sin(angle); /* y-shift */
return;
}
