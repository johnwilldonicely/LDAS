/*
<TAGS>math</TAGS>
DESCRIPTION:
	Sort an array of long integers using a quick-sort algorithm
	Modifies the input data array
	This function also modifies an array of indices so the user can know the original position of the sorted numbers

USES:
	rearranging data in numerical order

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long *data: array holding the data
	long *index: array of numbers representing the original order of the data, typically 0 to (n-1)
	long n: size of the array

RETURN VALUE:
	None

NOTE!
	- behaviour for non-numeric values, Inf and Nan is not specified
	- arrays with only one element will be unaltered
*/

void xf_qsortindex1_l(long *data, long *index,long n) {

	long ii,jj,kk,mm,left=0,right=(n-1),tempindex;
	long first,last,pivot;
	long tempdatum,pivotvalue;

	/* if data is less than 2 elements long, do nothing */
	if( right<=left ) return;
	/* choose the middle element of the data for the initial pivot */
	pivot=(long)((left+right)/2);
	/* check if pivot=left, which could cause endless recursion	*/
	if(pivot==left) pivot=right;
	/* define pivot value (saves indexing time in loop below) */
	pivotvalue=data[pivot];

	/* do an initial swap to move the pivot out of the way */
	tempdatum = data[left]; data[left] = data[pivot]; data[pivot] = tempdatum;
	tempindex = index[left]; index[left] = index[pivot]; index[pivot] = tempindex;

	/* define end of the search (to be modified in the loop below) */
	last=left;

	/* scan from first to last for values out of sequence: data[first]<= pivotvalue <data[last] */
	for(first=left+1; first<=right; first++) {
		if(data[first]<pivotvalue) {
			last++;
			tempdatum = data[first]; data[first] = data[last]; data[last] = tempdatum;
			tempindex = index[first]; index[first] = index[last]; index[last] = tempindex;
	}}

	/* move the pivot to the adjusted last position */
	tempdatum = data[left]; data[left] = data[last]; data[last] = tempdatum;
	tempindex = index[left]; index[left] = index[last]; index[last] = tempindex;

	/* recursively sort the left-side */
	ii=left; jj=last-1; kk=1+(jj-ii); xf_qsortindex1_l((data+ii),(index+ii),kk);

	/* recursively sort the right-side */
	ii=last+1; jj=right;  kk=1+(jj-ii); xf_qsortindex1_l((data+ii),(index+ii),kk);
}
