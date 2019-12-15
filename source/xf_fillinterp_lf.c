/*
<TAGS>signal_processing</TAGS>

DESCRIPTION:
	- Fill target array (B) with interpolated values from source array (A)
	- Uses timestamps for each array to find A's which fall before & after each B
	- Assigns interpolated value of A to B
	- Invalid data points only assigned to B where it is out of range of A (beginning & end)
	- NOT appropriate for data with large number of invalid samples

	- derived from hux_fillinterp.c and xf_fillinterp_itime.c
		- however, this function insists interpolation must use samples either side of the target, not the previous sample and one which coincidentally has the same timestamp as the target
		- also, user does not define an invalid value - NAN is taken as invalid

USES:
	- assigning low-sample-rate position values to high-sample-rate action-potiential data

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:

	long *Atime : input holding timestamps (sample-numbers) for array A
	long *Btime : input holding timestamps (sample-numbers) for array B
	float *Aval	: the data for array A (source)
	float *Bval : the data for array B (destination)
	long Atot : total elements in array A
	long Btot : total elements in array B
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	0 on success, -1 on error
	result array will hold statistics
	char array will hold message (if any)

SAMPLE CALL:


*/

# include <string.h>
# include <stdio.h>
#include <math.h>

long xf_fillinterp_lf(long *Atime, long *Btime, float *Aval, float *Bval, long Atot, long Btot, long maxinvalid, char *message) {

	char *thisfunc="xf_fillinterp_lf\0";
	long Aindex,Bindex,first,second,ninvalid;
	double aa;

	Aindex=Bindex=0;

	/* DETERMINE FIRST VALID SOURCE RECORD */
	while( !isfinite(Aval[Aindex]) && Aindex<Atot) Aindex++;
	/* check for no valid data */
	if(Aindex>=Atot) { sprintf(message,"%s [ERROR]: no valid data",thisfunc); return(-1); }


	/* DETERMINE B-RECORD JUST AFTER FIRST VALID A-RECORD  */
	/* Invalidate all B-records before (and up to exactly the same time as) the first valid A-record */
	/* Avoiding equality of time[Aindex] and time[Bindex] is important to avoid potential division-by-zero problems below */
	/* Put another way, all B-records must have A-records before AND after them for interpolation to proceed */
	while(Btime[Bindex] <= Atime[Aindex] && Bindex<Btot) {
		Bval[Bindex]= NAN;
		Bindex++;
	}

	/* SCAN THROUGH THE REMAINING B-RECORDS */
	first=second=Aindex;
	ninvalid=0;
	for(Bindex=Bindex;Bindex<Btot;Bindex++) {

		/* find valid A-records falling either side of the current B-record */
		/* "first" and "second" are holders for the A-record index before and after the current B, respectively */
		/* also don't stop looking until another valid A-record is found */
		while(Atime[second] <= Btime[Bindex] || !isfinite(Aval[second])) {
			if(!isfinite(Aval[second])) ninvalid++;
			else {
				first=second;
				ninvalid=0;
			}
			/* if we reach the end of the A-records, no further interpolation is possible */
			if(++second>=Atot) {
				for(Bindex=Bindex;Bindex<Btot;Bindex++) Bval[Bindex]= NAN;
				return(0);
			}
		}

		/* perform fill-interpolation */
		if(ninvalid<=maxinvalid) {
			/* calc. proportion of time between first and second A elapsed at the current B */
			aa = (double)(Btime[Bindex]-Atime[first]) / (double)(Atime[second]-Atime[first]);
			/* use this time proportion to calculate proportions of other variables elapsed */
			Bval[Bindex] = Aval[first] + (long)((Aval[second]-Aval[first]) * aa);
		}
		else Bval[Bindex]= NAN;
	}

	return(0);
}
