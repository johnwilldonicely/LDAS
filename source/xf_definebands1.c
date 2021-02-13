/*
DESCRIPTION:
	- build a matching set of frequency-band arrays by parsing an input string
	- input string = comma-separated triplets of name,start,stop
		- example: delta,.5,4,theta,4,12)
	NOTE: 16-band maximum
USES:
	- define band names and boundaries for programs analysing frequency bands

DEPENDENCIES:
	None

ARGUMENTS:
	char *setbands   : input string of CSV triplets e.g. "delta,0.5,4,theta,4,12" - will be modified
	float[16] bstart : output, preallocated array to hold 64 band start-values
	float[16] bstop  : output, preallocated array to hold 64 band start-values
	long *btot       : output, value holding the number of bands (max 16)
	char *message    : pre-allocated array to hold error message

RETURN VALUE:
	Success:
		- return pointer to array of indices to band-names
		- setbands array will be compressed to include only the bandnames, not the values
		- bstart[16] and bstop[16] will be filled with the start & stop values for each band
		- btot will be updated to reflect the number of bands

	Failure: NULL, message array will hold explanatory text (if any)

SAMPLE CALL:
	char setbands[]= "theta,.5,4,theta,4,12,beta,12,30,gamma,30,100";
	long btot=0,*ibands=NULL;
	float bstart[16],bstop[16];

	ibands= xf_definebands1(setbands,bstart,bstop,&btot,messsage);
	if(ibands==NULL) { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); exit(1); }
	for(ii=0;ii<btot;ii++) printf("%s\t%g\t%g\n",(setbands+iband[ii]),bstart[ii],bstop[ii]);
	...
	if(ibands!=NULL) free(ibands);

<TAGS> string spectra </TAGS>
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
/* external functions end */


long *xf_definebands1(char *setbands,float *bstart,float *bstop, long *btot, char *message) {

	char *thisfunc="xf_definebands\0",*newbands=NULL;;
	int status=0;
	long ii=0,jj=0,kk=0,mm=0,nn=0;
	long *iword=NULL,nwords=0,lensetbands=0;

	if(setbands==NULL) { snprintf(message,256,"%s [ERROR]: setbands is NULL",thisfunc); status=-1; goto END; }
	lensetbands= strlen(setbands);
	if(lensetbands==0) { snprintf(message,256,"%s [ERROR]: setbands has no elements",thisfunc); status=-1; goto END; }
	iword= xf_lineparse2(setbands,",",&nwords);
	if(nwords<0) { snprintf(message,256,"%s [ERROR]: lineparse function encountered insufficient memory",thisfunc); status=-1; goto END; }
	if(nwords%3!=0) { snprintf(message,256,"%s [ERROR]: setbands has %ld elements - must be a multiple of three",thisfunc,nwords); status=-1; goto END; }
	if((nwords/3)>16) { snprintf(message,256,"%s [ERROR]: setbands has %ld triplets - max bands is 16",thisfunc,(nwords/3)); status=-1; goto END; }

	/* SAVE THE NUMBER OF BANDS */
	nn= nwords/3;

	/* MAKE A NEW STRING TO HOLD NEW BANDS NAMES */
	newbands= realloc(newbands,(lensetbands+1*sizeof(char)));
	if(newbands==NULL) { snprintf(message,256,"%s [ERROR]: lineparse function encountered insufficient memory",thisfunc); status=-1; goto END; }

	/* SAVE THE START AND STOP VALUES FOR EACH BAND */
	for(ii=0;ii<nn;ii++) {
		bstart[ii]= atof((setbands+iword[ii*3+1]));
		bstop[ii]=  atof((setbands+iword[ii*3+2]));
	}

	/* BUILD THE NEWBANDS ARRAY (NAMES ONLY, SEPARATED BY NULLS)  */
	mm=0; // first word to insert - this will increment in threes
	for(ii=iword[0];ii<lensetbands;ii++) {
		/* this  will be the new start of iword[(mm/3)] */
		kk=ii;
		/* copy names-only to newbands */
		for(jj=iword[mm];jj<lensetbands;jj++) {
			newbands[ii]=setbands[jj];
			if(newbands[ii]=='\0') break;
			if(++ii>=lensetbands) break;
		}
		if(ii>=lensetbands) break;
		/* build new indices to each of nbands - this will be returned */
		iword[(mm/3)]= kk;
		mm+=3;
		if(mm>=nwords) break;
	}
	newbands[jj]='\0';
	//TEST: for(ii=0;ii<nn;ii++) printf("%s\t%f\t%f\n",(newbands+iword[ii]),bstart[ii],bstop[ii]);

	/* UPDATE THE SETBANDS STRING  */
	for(ii=0;ii<lensetbands;ii++) setbands[ii]= newbands[ii];

	/* UPDATE THE TOTAL NUMBER OF BANDS */
	*btot= nn;

END:

	if(newbands==NULL) free(newbands);

	/* RETURN THE SET (N=BTOT) OF LONG INDICES TO THE NEW BAND-NAMES NOW IN SETBANDS */
	if(status==0) return(iword);
	else return(NULL);

}
