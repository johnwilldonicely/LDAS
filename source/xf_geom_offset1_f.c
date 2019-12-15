/********************************************************************************
<TAGS>math</TAGS>
DESCRIPTION:
	Calculate the vertical offset of a point from the line joining two flanking points

USES:
	- surrogate for amplitude or AUC
	- useful for detecting amplitude of "local" peaks

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float x1      : line-point1, x
	float y1      : line-point1, y
	float x2      : line-point2, x
	float y2      : line-point2, y
	float x3      : point, x
	float x3      : point, y
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	offset on success, NAN error
	char array will hold message (if any)

SAMPLE CALL:

********************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double xf_geom_offset1_f(float x1,float y1,float x2,float y2, float x3, float y3, char *message) {

	char *thisfunc="xf_geom_yoff1_f\0";
	double aa,slope,inter,yoff;

	/* initialize offset to invalid value */
	yoff= NAN;
	/* make sure inputs are finite */
	aa= x1*y1*x2*y2*x3*y3;
	if(!isfinite(aa)) { sprintf(message,"%s [ERROR]: one of the arguments is non-finite",thisfunc); return(yoff); }
	/* make sure x1!=x2 */
	if(x1==x2) { sprintf(message,"%s [ERROR]: x1 and x2 are equal (%g)",thisfunc,x1); return(yoff); }
	/* make sure x3 falls between x1 and x2 */
	if((x1<x2 && x3<x1)||(x1>x2 && x3<x2)) { sprintf(message,"%s [ERROR]: x3(%g) is not between x1(%g) and x2(%g)",thisfunc,x3,x1,x2); return(yoff); }
	/* determine the slope & intercept */
	if(y1==y2) slope=0;
	else slope= (double)(y2-y1) / (double)(x2-x1);
	inter= (double)y1-(slope*(double)x1);
	/* calculate the relative peak */
	yoff= y3 - (slope*x3+inter);
	/* return results */
	return(yoff);
}
