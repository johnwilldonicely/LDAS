/***********************************************************************
<TAGS>math</TAGS>
DESCRIPTION:
        Calculate angle "A" based on length of sides a,b,c
	    B
	 c / \a
          /   \
         A-----C
            b
************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

float xf_geom_angle1_f(float a, float b, float c)
{
// convert to degrees - number is 180/Pi
return(acos((a*a-(b*b+c*c))/(2.0*b*c))*57.295779513082320876798154814105);
}
