/************************************************************************************
<TAGS>math</TAGS>
DESCRIPTION:
	Take two cartesian coordinates and return distance
************************************************************************************/
#include <math.h>
#include <stdio.h>
float xf_geom_dist1(float x1,float y1,float x2,float y2)
{
	float dx,dy;
	dx=x2-x1;
	dy=y2-y1;
	return(sqrt(dx*dx+dy*dy)); /* distance between coordinates */
}
