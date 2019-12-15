/************************************************************************************
<TAGS>math</TAGS>
DESCRIPTION:
	Take two cartesian coordinates and return distance (result[0]) and angle (result[1])
************************************************************************************/
#include <math.h>
#include <stdio.h>
void xf_geom_distangle(float x1,float y1,float x2,float y2,float *result)
{
float dx,dy,angle;
dx=x2-x1; dy=y2-y1;

result[0]= sqrt(dx*dx+dy*dy); /* distance between coordinates */

if(dx==0) {
	if(dy==0) result[1]=0.0;
	else if(dy>0) result[1]=90.0;
	else result[1]=270.0;
}
else if(dy==0) {
	if(dx<0) result[1]=180.0;
	else result[1]=0.0;
}

else {
	result[1]= atanf(dy/dx)*57.295779513082320876798154814105;	/* convert to degrees - number is 180/Pi*/;
	if(dx<0) result[1]+=180.0;
	else if(dx>0&&dy<0) result[1]+=360.0;
}

return;
}
