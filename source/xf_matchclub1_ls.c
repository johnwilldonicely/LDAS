/*
<TAGS>signal_processing</TAGS>
DESCRIPTION:
	- remove clusters from a club/t record which do not match the ID's specified in a list

USES:
	- reducing the size of a cluster record to make processing more efficient

DEPENDENCY TREE:
	long *xf_lineparse2(char *line,char *delimiters, long *nwords);

ARGUMENTS:
	char *list1 : pointer to CSV list of clusters to keep
	long *clubt : input array of cluster timestamps
	short *club : input array of cluster-IDs
	long nn     : total number of items in the clubt[] and club[]
	char *message : pre-allocated array to hold error message


RETURN VALUE:
	on success:
		- the new number of clubt[] and club[] records
		- the clubt[] and club[] arrays themselves will be adjusted accordingly
	on failure:
		-1

SAMPLE CALL:


*/

# include <stdlib.h>
# include <string.h>
# include <stdio.h>

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
/* external functions end */

long xf_matchclub1_ls(char *list1, long *clubt, short *club, long nn, char *message) {

	char *thisfunc="xf_matchclub1_ls\0";
	short *clukeep=NULL;
	long ii,jj,kk;
	long *index1=NULL,nclulist;

	/* PARSE THE LIST OF CLUSTER-ID'S TO KEEP */
	index1= xf_lineparse2(list1,",",&nclulist);
	if(index1==NULL) { sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(-1); }

	/* FIND THE LARGEST CLUSTER ID AND ALLOCATE MEMORY FOR THE CLUKEEP[] ARRAY */
	kk=-1; for(ii=0;ii<nn;ii++) if(club[ii]>kk) kk=club[ii]; kk++;
	clukeep= calloc(kk,sizeof(*clukeep));
	if(clukeep==NULL) { sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); free(index1); return(-1); }

	/* SET THE CLUKEEP FLAG FOR CLUSTERS THAT ARE MENTIONED IN THE LIST */
	for(ii=0;ii<nclulist;ii++) clukeep[ atol(list1+index1[ii]) ]= 1;

	/* SCAN THE RECORD AND COMPRESS TO ONLY INCLUDE CLUSTERS IN THE LIST */
	for(ii=kk=0;ii<nn;ii++) {
		if(clukeep[club[ii]]==1) {
			club[kk]=club[ii];
			clubt[kk]=clubt[ii];
			kk++;
		}
	}
	nn= kk;

	/* FREE MEMORY AND RETURN NEW SPIKE-COUNT */
	free(index1);
	free(clukeep);
	return(kk);
}
