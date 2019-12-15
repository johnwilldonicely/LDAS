
/***********************************************************************************************
Smooth a float array of size "arraysize" using boxcar averaging method.
Memory must be allocated for the original and smoothed arras by the calling function
Invalid data points in original will be replaced by mean of valid points in smoothing window 
This function DOES NOT rerwrite the original data
The function is written not to be pretty, but to maximize speed, as follows.... 

1. As many variables are calculated outside the loops as possible. 
2. The smoothing is conducted in 3 steps (beginning, middle, end) so as to avoid 
having to check for smoothing window exceeding begining or end of the array except in the 
portions of the array where it is necessary - ie. two conditional checks removed from bulk of 
processing. 
3. A running sum is used to calculate the average instead of seeking forward and backward at 
each sample. This is faster, even for small smoothing window sizes, & makes calculation speed 
insensitive to the size of the window. 
************************************************************************************************/
#include<stdlib.h>
#include<stdio.h>
void hux_smooth_float (
					   float *original, /* pointer to data to be smoothed (memory must be pre-allocated) */
					   float *smoothed, /* pointer to an array set up to hold the result */
					   int arraysize,	/* number of elements in original array */
					   int winmax,		/* half-size of smoothing window (boxcar) - full size = 2*winmax +1 */
					   int invalid		/* invalid data value to be excluded from smoothing */
					   )
{
if(winmax<1) return;
int z,buff_n,winmin,start,end,trailing,leading;
float buff_sum;

winmin=(-1*winmax)-1; /* value just outside trailing edge of smoothing window */
buff_sum = 0.00; /* intitialise buffer for STEP 2 */
buff_n = 0; /* initialize buffer counter for STEP 2 */

/* STEP 1: pre-fill sum-buffer */
start=0; end=winmax-1;
for(z=start;z<=end;z++) {if(original[z]!=invalid) {buff_n++ ; buff_sum+= original[z];}} /* fill buffer */

/* STEP 2 - smooth beginning of array using dynamically calculated sliding sum-buffer*/
start=0; end=winmax;
for(z=start;z<=end;z++)
	{
	leading= z+winmax; /* calculate position of array value to be added */
	if(original[leading]  !=invalid) {buff_n ++; buff_sum += original[leading];}  /* update sum-buffer step 2*/
	if(buff_n>0) smoothed[z] = buff_sum/buff_n; 
	else smoothed[z]=(float)invalid;
	}
/* STEP 2 - smooth middle of array using dynamically calculated sliding sum-buffer*/
start=winmax+1; end=arraysize-winmax-1;
for(z=start;z<=end;z++)
	{
	trailing= z+winmin; /* calculate position of array value to be dropped */
	leading=  z+winmax; /* calculate position of array value to be added */
	if(original[trailing] !=invalid) {buff_n --; buff_sum -= original[trailing];} /* update sum-buffer step 1*/
	if(original[leading]  !=invalid) {buff_n ++; buff_sum += original[leading];}  /* update sum-buffer step 2*/
	if(buff_n>0) smoothed[z] = buff_sum/buff_n; 
	else smoothed[z]=(float)invalid;
	}
/* STEP 3: smooth end of array  - same conditions as step 1, but out of upper range must be checked */
start=arraysize-winmax; end=arraysize-1;
for(z=start;z<=end;z++)
	{
	trailing= z+winmin; /* calculate position of array value to be dropped */
	if(original[trailing] !=invalid) {buff_n --; buff_sum -= original[trailing];} /* update sum-buffer step 1*/
	if(buff_n>0) smoothed[z] = buff_sum/buff_n; 
		else smoothed[z]=(float)invalid;
	}
return;
}
