/******************************************************************************
Calculation of the probability (p) for a resultant length (R) z-score (Z)
Appropriate for tests of uniformity (vs. unimodal clustering) of circular data

This formula comes directly from Dr. Nicholas I. Fisher, personal 
correspondence,August 3 2005...

"
Dear John, 
I think the simplest thing is to calculate the P-value directly from the test statistic. (This is what I present in my book.). 
The P-value is the probability of obtaining the given value of the test statistic or something more extreme, assuming that the null hypothesis of uniformity is true. 
Suppose you have a sample of n data values. 

  * Calculate the mean resultant length R, say (a number between 0 and 1)
  * Then calculate the test statistic Z = n * R * R  
  * Then the probability of obtaining a value at least as large as Z is 

  P = exp(-Z) [1 + (2Z - Z*Z)/(4*n) - (24Z - 132*Z*Z + 76*Z*Z*Z - 9*Z*Z*Z*Z)/(288n*n)]

In fact, if n > 40, the approximation P = exp(-Z) is quite adequate. 
Hope that helps. Let me know if you need more info. 
Regards ... Nick 
"

Citation:
N.I. Fisher (1995), "Statistical Analysis of Circular Data". Paperback
edition. Cambridge University Press, Cambridge.
***************************************************************************/
#include <math.h>

float hux_prob_R(float R, int n)
{
        void hux_error(char error_txt[]);

		double Z;
        float p;
		
		Z=n*R*R;
		p = exp(-Z)* (1+ (2*Z - Z*Z)/(4*n) - (24*Z - 132*Z*Z + 76*Z*Z*Z - 9*Z*Z*Z*Z)/(288*n*n));
        return(p);
}

