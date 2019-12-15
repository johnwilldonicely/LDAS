/*
<TAGS>misc</TAGS>
DESCRIPTION:
	"roll" the data in a circular buffer, typically to prepare for the addition of additional data

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	FILE *fpin : pointer to input stream/file
	void **buffer1 : pointer to circular buffer to be rolled
	size_t nbuff : number of elements in the buffer
	size_t offset : the amount (number of elements) by which to roll the buffer
	int type : the type of rolling...
		1) forward, entire buffer
		2) forward, dont fill the top with the bottom - use if the top is later to be filled with new data
		3) forward, only fill the top with the bottom - use if the bottom is later to be filled with new data
		4) backward, entire buffer
		6) backward, don't fill the bottom with the top - use if the bottom is later to be filled with new data
		6) backward, only fill the bottom with the top - use if the top is later to be filled with new data
	char *message : an array to hold diagnostic messages on return

RETURN VALUE:
	0 on success, -1 on error

SAMPLE CALL:

	nn=13; offset=3; type=1;
	for(ii=0;ii<nn;ii++) data[ii]=(float)ii;
	x= xf_rollbuffer1(data,nn,offset,type,message);
	if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	for(ii=0;ii<13;ii++) printf("%ld\t%g\n",ii,data_f[ii]);

	0	10
	1	11
	2	12
	3	0
	4	1
	5	2
	6	3
	7	4
	8	5
	9	6
	10	7
	11	8
	12	9


--------------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int xf_rollbuffer1_f(float *buffer1, size_t nbuff, size_t offset,int type, char *message) {

	char *thisfunc="xf_rollbuffer1_f\0";
	size_t ii,jj;
	float *temp;

	/* MAKE SURE OFFSET IS SMALLER THAN THE SIZE OF THE ARRAY */
	if(offset>=nbuff) {
		sprintf(message,"%s [Error]: offset (%ld) must be less than nbuff(%ld)",thisfunc,offset,nbuff);
		return(-1);
	}


	/* OPTIONS FOR ROLLING DATA FORWARD */

	if(type==1) {
		/* allocate memory */
		if((temp=(float *)malloc(offset*sizeof(float)))==NULL) { sprintf(message,"%s [Error]: insufficient memory",thisfunc); return(-1); }
		/* save the very bottom of the array */
		jj=(nbuff-offset); for(ii=0;ii<offset;ii++) temp[ii]=buffer1[jj++];
		/* roll the bottom of the array, starting at the bottom and working back */
		ii=nbuff-1; jj=ii-offset; for(ii=ii;ii>=offset;ii--) buffer1[ii]=buffer1[jj--];
		/* copy the old bottom of the array to the top */
		for(ii=0;ii<offset;ii++) buffer1[ii]=temp[ii];
		/* free memory */
		free(temp);
	}
	else if(type==2) {
		/* roll the bottom of the array, starting at the bottom and working back */
		ii=nbuff-1; jj=ii-offset; for(ii=ii;ii>=offset;ii--) buffer1[ii]=buffer1[jj--];
	}
	else if(type==3) {
		/* copy the very bottom of the array to the top */
		jj=(nbuff-offset); for(ii=0;ii<offset;ii++) buffer1[ii]=buffer1[jj++];
	}


	/* OPTIONS FOR ROLLING DATA BACKWARD */

	else if(type==4) {
		/* allocate memory */
		if((temp=(float *)malloc(nbuff*sizeof(float)))==NULL) { sprintf(message,"%s [Error]: insufficient memory",thisfunc); return(-1); }
		/* save the very top of the array */
		for(ii=0;ii<offset;ii++) temp[ii]=buffer1[ii];
		/* roll the top of the array */
		jj=offset; for(ii=0;jj<nbuff;ii++) buffer1[ii]=buffer1[jj++];
		/* copy the old top of the array to the bottom */
		jj=(nbuff-offset); for(ii=0;ii<offset;ii++) buffer1[jj++]=temp[ii];
		/* free memory */
		free(temp);
	}
	else if(type==5) {
		/* roll the top of the array */
		jj=offset; for(ii=0;jj<nbuff;ii++) buffer1[ii]=buffer1[jj++];
	}
	else if(type==6) {
		/* copy the very top of the array to the bottom */
		jj=(nbuff-offset); for(ii=0;ii<offset;ii++) buffer1[jj++]=buffer1[ii];
	}

	else {
		sprintf(message,"%s [Error]: invalid roll-type %d (must be 1-4)",thisfunc,type);
		return(-1);
	}

	return(0);
}
