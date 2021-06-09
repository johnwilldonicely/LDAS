/*
<TAGS> dt.matrix stats </TAGS>
DESCRIPTION:
	extract parameters for a rectangular region of a matrix
	NAN and INF values will be ignored

USES:
DEPENDENCY TREE:

ARGUMENTS:
	double *matrix1 : input array (a matrix)
	long nx         : width of matrix`
	long ny         : height of matrix
	double range[4] : matrix ranges (x1,x2,y1,y2) - if NULL, values are taken as 0,(nx-1),0,(ny-1)
	double box      : ranges defining box - as above, but must fit within the matrix - note that top left of matrix represents the lowest values
	int stat        - the statistic required (1=sum,2=mean)
	char *message   : array to hold error message

RETURN VALUE:
	on success: the requested statistic
	on failure: NAN

SAMPLE CALL:
	aa= xf_matrixbox1_d(matrix1,nrows,ncols,range,box,message);
	if(!isfinite(aa))  { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); exit(1); }

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double xf_matrixbox1_d(double *matrix1, long nx, long ny, double *range, double *box, char *message) {

	char *thisfunc="xf_matrixbox1_d\0";
	long ii,jj,kk,x1,x2,y1,y2,nbox,nsums,index;
	double aa,mx1,mx2,my1,my2,bx1,bx2,by1,by2;
	double *tempdat=NULL,result=NAN;

	// SIMPLIFY REFERENCE TO COORDINATES FOR MATRIX AND BOX
	mx1=range[0]; mx2=range[1]; my1=range[2]; my2=range[3];
	bx1=box[0]; bx2=box[1]; by1=box[2]; by2=box[3];


	/* CHECK VALIDITY OF ARGUMENTS */
	if(nx<=0||ny<=0) { sprintf(message,"%s [ERROR]: invalid size of input matrix (%ld x %ld)",thisfunc,nx,ny); result=NAN; goto END; }
	if(bx1<mx1 || bx2>mx2 || by1<my1 || by2>my2 ) { sprintf(message,"%s [ERROR]: box out of range of matrix",thisfunc); result=NAN; goto END; }
	if(mx1>mx2) { sprintf(message,"%s [ERROR]: matrix range x-values, minimum is larger than maximum",thisfunc); result=NAN; goto END; }
	if(my1>my2) { sprintf(message,"%s [ERROR]: matrix range y-values, minimum is larger than maximum",thisfunc); result=NAN; goto END; }
	if(bx1>bx2) { sprintf(message,"%s [ERROR]: box x-values, minimum is larger than maximum",thisfunc); result=NAN; goto END; }
	if(by1>by2) { sprintf(message,"%s [ERROR]: box y-values, minimum is larger than maximum",thisfunc); result=NAN; goto END; }

	/* CALCULATE BOX COORDINATES : proportion of range mutiplied by total elements */
	x1= nx * (bx1 - mx1) / (mx2 - mx1);
	x2= nx * (bx2 - mx1) / (mx2 - mx1);
	y1= ny * (by1 - my1) / (my2 - my1);
	y2= ny * (by2 - my1) / (my2 - my1);

	/* ALLOCATE MEMORY FOR TEMP ARRAY */
	nbox= (x2-x1)*(y2-y1);
	tempdat= malloc( nbox * sizeof(*tempdat) );
	if(tempdat==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); result=NAN; goto END; }

	/* STORE THE DATA FROM THE MATRIX-AREA CORRESPONDING TO THE BOX */
	nbox=0; // reset nbox to zero - we will only be keeping valid points
	for(ii=y1;ii<=y2;ii++) {
		index= ii*nx; // set index to the start of the row
		for(jj=x1;jj<=x2;jj++) {
			aa= matrix1[index+jj];
			if(isfinite(aa)) tempdat[nbox++]= aa;
	}}

	/* CALCULATE THE STATS FOR THE BOX DATA */

result=666;

	/* RETURN THE ERROR CODE */
END:
	if(tempdat!=NULL) free(tempdat);
	return(result);
}
