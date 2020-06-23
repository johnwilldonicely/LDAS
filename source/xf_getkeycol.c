/*
DESCRIPTION:
	- Find the columns containing each keyword in a delimited list
	- Useful for reading the header-lines in files, and determining which columns correspond with the keywords
	- It is assumed that the user knows wich keywords should be present


DEPENDENCIES:
	xf_lineparse2()

ARGUMENTS:
	double *data : input holding data
	size_t nn : size of data array
	double interval:  the interval between samples, for integrating area
	double *result : pre-allocated array to hold results - must allow at least 6 elements
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	0 on success, -1 on error
	result array will hold statistics
	message array will hold explanatory text (if any)

SAMPLE CALL:
	x= xf_auc1_d(data, nn, interval, result, );
	if(x!=0) { fprintf(stderr,"\b\n\t%s/%s\n\n",thisprog,message); exit(1); }

<TAGS> string </TAGS>
*/

long *xf_lineparse2(char *line,char *delimiters, long *nwords);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
long *xf_getkeycol(char *line1, char *d1, char *keys1, char *d2, long *nkeys1, char *message) {

	char *thisfunc="xf_getkeycol\0";
	char *line2=NULL,*keys2=NULL,*pkey=NULL;
	int status=0;
	long ii,jj,kk,*iwords=NULL,*ikeys=NULL,*keycols=NULL,nwords,nkeys2,missing;

	/* CHECK VALIDITY OF ARGUMENTS */
	if(line1==NULL) { sprintf(message,"%s [ERROR]: invalid input line",thisfunc); status=-1; goto END; }
	if(keys1==NULL) { sprintf(message,"%s [ERROR]: invalid key-string",thisfunc); status=-1; goto END; }

	/* MAKE LOCAL COPIES TO PRESERVE THE INPUT LINES & KEYS */
	line2= strdup(line1);
	keys2= strdup(keys1);
	if(line2==NULL||keys2==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); status=-1; goto END;}

	/* PARSE THE LINE AND THE KEYS */
	iwords= xf_lineparse2(line2,d1,&nwords);
	if(nwords<0) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); status=-1; goto END;}
	ikeys= xf_lineparse2(keys2,d2,&nkeys2);
	if(nkeys2<0) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); status=-1; goto END;}

	/* ALLOCATE MEMORY FOR KEYCOL ARRAY AND INITIALIZE */
	keycols= malloc(nkeys2*sizeof(*keycols));
	if(keycols==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); status=-1; goto END;}
	for(ii=0;ii<nkeys2;ii++) keycols[ii]=-1;

	/* UPDATE THE ARRAY WITH THE COLUMN NAME MATCHING ANY WORD IN LINE*/
	missing= nkeys2;
	for(ii=0;ii<nkeys2;ii++) {
		pkey= keys2+ikeys[ii];
		for(jj=0;jj<nwords;jj++) {
			if(strcmp(pkey,(line2+iwords[jj]))==0) { keycols[ii]= jj; missing--; break; }
		}
	}
	if(missing>0) {sprintf(message,"%s [ERROR]: there are missing keys",thisfunc); status=-1; goto END;}


END:
	//TEST for(ii=0;ii<nkeys2;ii++) printf("%ld: key=%s keycol=%ld\n",ii,keys2+ikeys[ii],keycols[ii]);exit(0);
	if(line2!=NULL) free(line2);
	if(keys2!=NULL) free(keys2);
	if(iwords!=NULL) free(iwords);
	if(ikeys!=NULL) free(ikeys);

	(*nkeys1)= nkeys2;
	if(status==0) return(keycols);
	else return(NULL);

}
