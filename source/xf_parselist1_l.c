/*
<TAGS>string</TAGS>
DESCRIPTION:

	Convert a character string containing a list of positive numbers and/or ranges into an array of numbers
	Preserves input order
	Will not remove redundant entries
	Will not work with negative numbers

	Sample acceptable list formats:
		1,2,5,6,10
		1,3,5,7-20
		10-1,11,14,20-25


USES:
	Build a list of columns to extract from a table

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *line       : character array holding list
	char *delimiters : delimiters (quoted) for each item in the list
	long min       : minimum accepted value
	long max       : maximum accepted value
	long *nitems   : number of items in resulting list (pass as address to variable)
	char *message    : holds error message

RETURN VALUE:
	pointer to the list array, or NULL on error
	value for nitems will be overwritten

SAMPLE CALLS:

	char line[1000],message[1000];
	long *list=NULL,i,n;

	sprintf(line,"1,2,3,4,5");
	list= xf_parselist1_l(line,",",1,999,&n,message);
	if(list==NULL) {fprintf(stderr,"\b\n\t--- Error [%s] %s\n\n",thisprog,message); exit(1); }
	for(i=0;i<n;i++) printf("%ld\n",list[i]);
	free(list);

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long *xf_parselist1_l(char *line, char *delimiters, long min, long max, long *nitems, char *message) {

	char *thisfunc="xf_parselist1_d\0";
	char *pch,*word=NULL;
	int sizeoflong=sizeof(long);
	long *list=NULL,numleft,numright;
	long ii,jj,kk,nn,mm,nchars,nwords,nnums;
	long *start1=NULL,*start2=NULL;

	nchars=strlen(line);
	*nitems=0;
	sprintf(message,"%s: success",thisfunc);

	/* CHECK MIN AND MAX */
	if(min>max) {sprintf(message,"%s [ERROR]: min (%ld) should not be greater than max (%ld)",thisfunc,min,max); return(NULL);}

	/* REALLOCATE MEMORY FOR START1 ARRAY - START POSITION OF EACH WORD IN THE LIST */
	start1= realloc(start1,(nchars*sizeof(*start1)));
	if(start1==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(NULL);}
	start1[0]=0;

	/* REALLOCATE MEMORY FOR WORD ARRAY _ TEMPORARILY HOLDS EACH LIST ITEM  */
	word= realloc(word,(nchars*sizeof(*word)));
	if(word==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(NULL);}
	word[0]=0;

	/* REALLOCATE MEMORY FOR START2 ARRAY - START POSITION OF EACH NUMBER IN THE WORD */
	/* ALLOWS FOR HYPHEN-SEPARATED ITEMS */
	start2= realloc(start2,(nchars*sizeof(*start2)));
	if(start2==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(NULL);}
	start2[0]=0;


	/* FIND THE LOCATIONS OF THE DELIMITERS */
	pch= strpbrk(line,delimiters);
	nwords= 1;
	while(pch != NULL) {
		ii=pch-line;
		start1[nwords++]=ii;
		pch=strpbrk(pch+1,delimiters);
	}

	/* RESET DELIMITERS TO '\0' */
	for(ii=1;ii<nwords;ii++) line[start1[ii]]='\0';
	/* SET START1 POSITIONS FOR EACH WORD TO ONE PAST THE DELIMITER */
	for(ii=0;ii<nwords;ii++) if(start1[ii]!=0) start1[ii]+=1;
	/* IF THE LAST WORD CONTAINS A NEWLINE, REPLACE IT WITH A NULL */
	ii=start1[(nwords-1)];
	while(line[ii]!='\0') {
		if(line[ii]=='\n' || line[ii]=='\r') line[ii]='\0';
		ii++;
	}

	/* NOW ANALYZE EACH WORD AND BUILD THE LIST */
	for(ii=0;ii<nwords;ii++) {
		strcpy(word,(line+start1[ii]));
		/* find hyphens and count words - will always be at least 1 */
		pch= strpbrk(word,"-");
		nnums= 1;
		while(pch != NULL) {
			jj= pch-word;
			start2[nnums++]= jj;
			pch= strpbrk(pch+1,"-");
		}

		/* CASE1: SINGLE NUMBER */
		if(nnums==1) {
			if(strlen((word+start2[0]))>0) {
				list= realloc(list,((*nitems+1)*sizeoflong));
				list[*nitems]= atol(word);
				if(list[*nitems]<min||list[*nitems]>max) {
					sprintf(message,"%s: number (%ld) is outside limits (%ld to %ld)",thisfunc,list[*nitems],min,max);
					free(start1);free(start2);free(word);free(list);list=NULL;
					return(list);
				}
				(*nitems)++;
			}
			else {
				sprintf(message,"%s: corrupt number format \"%s\" (no content)",thisfunc,word);
				free(start1);free(start2);free(word);free(list);list=NULL;
				return(list);
			}
		}

		/* CASE1: A RANGE SPECIFIED USING A HYPHEN (e.g. 2-7) - CAN ALSO LEAVE MIN OR MAX UNSPECIFIED */
		else if(nnums==2) {
			/* reset delimiters to '\0' */
			for(jj=1;jj<nnums;jj++) word[start2[jj]]='\0';
			/* set start2 positions for each word to one past the delimiter */
			for(jj=0;jj<nnums;jj++) if(start2[jj]!=0) start2[jj]+=1;
			/* process each number - if either is missing, substitute the min or max argument as appropriate */
			if(strlen((word+start2[0]))>0) numleft=atol(word+start2[0]);
			else numleft=min;
			if(strlen((word+start2[1]))>0) numright=atol(word+start2[1]);
			else numright=max;
			/* make sure number on the left side of the hyphen is within range */
			if(numleft<min||numright>max) {
				sprintf(message,"%s: number in %s (%ld) is outside limits (%ld to %ld)",thisfunc,word,numleft,min,max);
				free(start1);free(start2);free(word);free(list);list=NULL;
				return(list);
			}
			/* make sure number on the right side of the hyphen is within range */
			if(numright<min||numright>max) {
				sprintf(message,"%s: number in %s (%ld) is outside limits (%ld to %ld)",thisfunc,word,numright,min,max);
				free(start1);free(start2);free(word);free(list);list=NULL;
				return(list);
			}
			/* build list for ascending numbers */
			if(numleft<=numright) {
				for(kk=numleft;kk<=numright;kk++) {
					list= realloc(list,((*nitems+1)*sizeoflong));
					list[*nitems]=kk;
					(*nitems)++;
				}
			}
			/* build list for descending numbers */
			else {
				for(kk=numleft;kk>=numright;kk--) {
					list= realloc(list,((*nitems+1)*sizeoflong));
					list[*nitems]=kk;
					(*nitems)++;
				}
			}
		} // END OF CONDITION IF(NWORDS==2)

		/* if there are more than 3 numbers (ie. more than 1 hyphen) reject number format */
		else {
			sprintf(message,"%s: corrupt number format \"%s\" (more than one hyphen)",thisfunc,word);
			free(start1);free(start2);free(word);free(list);list=NULL;
			return(list);
		}

	} // END OF LOOP PROCESSING EACH WORD
	free(start1);
	free(start2);
	free(word);

	return (list);
}
