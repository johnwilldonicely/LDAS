# calculate the mean & SEM of column-1, for each category of column-2
# pipe two columns of output to this script 
# example: xe-cut1 data.txt result,group | Rscript /opt/LDAS/rscripts/mean_1.R


myfunc= function(x) { c( MEAN=mean(x), SEM=sd(x)/sqrt(length(x)), LEN=format(length(x),digits=0) )}

df1= read.table("stdin",header=T,sep="\t")
namedv= colnames(df1)[1]
namegrp1= colnames(df1)[2]

# remove non-numbers manually 
df1= df1[df1[1]!="-" & df1[1]!="nan" & df1[1]!="NAN" & df1[1]!="NaN" & df1[1]!="inf" & df1[1]!="INF", ]

# calculate the stats 
a= aggregate(df1[namedv],df1[namegrp1], FUN=myfunc)

# print the result, removing rownames 
print(a,row.names=F)

