/*
<TAGS>synthetic_data</TAGS>
DESCRIPTION:
	Calculate a double-precision random number from zero to [setmax]
	- calls function random(), which returns a float, normalized by RAND_MAX (unsigned long int)
	- a correction is made if setmax > RAND_MAX

	Note that the calling function should set the random seed if more than one call is to be made on a given date
	This is necessary to avoid producng the same random sequency multiple times

USES:
	Many

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double setmax : largest possible value desired

RETURN VALUE:
	The random number

SAMPLE CALL:

	#include <stdlib.h>
	#include <stdio.h>
	#include <time.h>   // needed for time() function
	#include <unistd.h>	// needed for getpid() function

	double xf_rand1_d(double setmax);

	int main (int argc, char *argv[]) {
		// set the random seed using the current time and process-ID
		srand(time(NULL) + getpid());
		// generate 10 random numbers from zero to 100
		for(int i=0;i<10;i++) printf("%d: %f\n",i,xf_rand1f(100.0));
	}

*/

#include <stdlib.h>
double xf_rand1_d(double setmax) {

	if(RAND_MAX>=setmax) {
		return(random()/(RAND_MAX/setmax));
	}

	else {
		return(random()/(setmax/RAND_MAX));
	}

}
