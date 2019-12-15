/* 
- John R. Huxter - October 2005 - Define place field 
- Initialise field array elements to 0 in advance, or <0 if they should be excluded from field detection
- After field detection...
	-1 = omitted elements
	 0 = out of field
	 1 = in field
	 2 = in field peak
- To detect multiple fields, set results >0 to -1, and call function again 
- Sample definition in calling function: int hux_definefield(float *rate, int *field, int matrixwidth, int x, int y, float thresh, float thresh2);  
- Sample call: fieldsize = hux_definefield(ratearray,fieldarray,64, 12,5, 1.70,15.76);
- Returns field size i number of bins
- Warning - field can propogate to non-visited bins if firing rate map is smoothed - use smoothing judiciously 
*/

#include <stdio.h>
#include <stdlib.h>

/* external functions */
void hux_error(char error_txt[]);
int xf_compare1_f(const void *a, const void *b); 


int hux_definefield(
				 float *rate,	/* pointer to zero-offset firing rate array */
				 int *field,	/* pointer to zero-offset field definition array - should be initialised to "0" to start, -1 to ignore */
				 int matrixwidth,	/* width of rate array (total elements = matrixwidth*matrixwidth) */
				 int peakxbin,	/* peak bin number (x) */
				 int peakybin,	/* peak bin number (y) */
				 float thresh,	/* firing rate threshold for inclusion in field */
				 float peakpercentile /* in-field firing rate percentile (eg. 0.80) for inclusion in field-centre */
				 )
{
int i,N,n,x,y,x1,x2,y1,y2,p1,radius,field_size=0;
float *temprate,peakzonerate;

if(peakpercentile>=1) hux_error("hux_definefield: argument #7 should be < 1");

N=matrixwidth*matrixwidth;
field[peakybin*matrixwidth+peakxbin]=1; /* start by defining peak bin as zero - this is added to field in "iteration zero" by default*/

/* define firing field */
for(radius=1;radius<matrixwidth;radius++) {
	y1=peakybin-radius; y2=peakybin+radius; x1=peakxbin-radius; x2=peakxbin+radius;
	for(y=y1;y<=y2;y++)	for(x=x1;x<=x2;x++)	{
		if(x<0||y<0||x>=matrixwidth||y>=matrixwidth) continue;
		p1=y*matrixwidth+x;
		if(rate[p1]>=thresh) {	/* if rate exceeds threshold... */
			if(	/* ...and an adjacent bin was previously added... */ 
				field[((y-1)*matrixwidth)+x]==radius || 
				field[((y+1)*matrixwidth)+x]==radius || 
				field[(y*matrixwidth)+(x-1)]==radius || 
				field[(y*matrixwidth)+(x+1)]==radius
				)   {
					field[p1]=radius+1; /* ... which is at least 1 */
					}		
}}}

/* sort temporary array to get peakzonerate (lower bound for pekpercentile cutoff) within the field */
temprate = (float *) calloc(N,sizeof(float));
for(i=n=0;i<N;i++) if(field[i]>0) temprate[n++] = rate[i]; /* build temporary array */

/* sort the temporary array */

qsort(temprate,(long)N,sizeof(float),xf_compare1_f);

peakzonerate=temprate[(int)(peakpercentile*n)];
free(temprate);

/* set all >zero field values to "1", and determine field "peak" (value set to "2") */
for(y=0;y<matrixwidth;y++) {
	for(x=0;x<matrixwidth;x++)	{
		p1=y*matrixwidth+x;
		if(field[p1]>0)	{
			field_size++;	/* calculate field size in bins*/
			field[p1]=1;	/* make all greater-than-zero values = 1 */
			if(rate[p1]>=peakzonerate) field[p1]=2;	/* code for field centre = 2 */
}}}

return(field_size);
}
