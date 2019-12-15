/********************************************************************************
<TAGS>math</TAGS>
DESCRIPTION:
	Return the linear equation for two Cartesian coordinates
	- equation: y= slope*x + intercept
	- result[0]= slope (or NAN for vertical lines)
	- result[1]= intercept (or NAN for vertical lines)
********************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void xf_geom_linear1_f(float x1,float y1,float x2,float y2,double *result_d) {

	/* determine the slope */
	if(x1==x2) result_d[0]= NAN;
	else if(y1==y2) result_d[0]=0;
	else result_d[0]= (double)(y2-y1) / (double)(x2-x1);

	/* determine the intercept */
	if(x1==x2) result_d[1]= NAN;
	else result_d[1]= (double)y1-(result_d[0]*(double)x1);

	return;
}
