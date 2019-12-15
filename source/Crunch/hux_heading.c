/***********************************************************************
 - calculate: heading from change in x-position & y-position
 - requires: xchange (x), ychange (y)
 - returns: angle (0-359, 0=east, 90=north, 180=west, 270=south)
************************************************************************/
#include <math.h>
float hux_heading (float delta_x, float delta_y) {

	float angle;

	if(delta_x==0) {
	        if(delta_y==0) return(0);
	        else if(delta_y>0) return(90);
	        else return(270);
	}

	else if(delta_y==0) {
		if(delta_x<0) return(180);
		else return(0);
	}

	else {
		angle=atanf(delta_y/delta_x)*57.29577951308232087685; /* 57.295... = 180/pi */
		if(delta_x<0) angle+=180.0;
		else if (delta_x>0&&delta_y<0) angle+=360.0;
		return(angle);
	}
}
