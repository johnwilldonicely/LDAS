/***********************************************************************
- calculate degrees of angular shift (-180 to 180)
- polar coordinate system defines as follows...
- negative shifts are clockwise
- positive shifts are counterclockwise
************************************************************************/
float hux_angleshift (
	float angle1,	/* first measure */ 
	float angle2	/* second measure */
	)
{
float shift;
shift = angle2 - angle1;
if(shift>180) shift-=360.0;
if(shift<-180) shift+=360.0;
return(shift);
}
