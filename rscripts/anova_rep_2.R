################################################################################
# 2-WAY REPEATED-MEASURES ANOVA WITH CONTRASTS AGAINST A CONTROL GROUP

# DEPENDENCIES: R, with packages emmeans and afex

# USAGE:
#    - Pipe 4 columns with headers to Rscript, calling this script
#         - col1= subject-id
#         - col2= within-subjects group-number - assumes group "0" is the control group
#         - col3= within-subjects time
#         - col4= result
#    - Groups are assumed to be numeric and group 0 is taken as the control

# EXAMPLE:
# 	awk '{print $5,$1,$2,$3}' data.txt | Rscript [this script]
################################################################################
suppressMessages(library(afex))
suppressMessages(library(emmeans))

df1= read.table("stdin", sep="\t", header=T)
namesub=colnames(df1)[1]
namegrp=colnames(df1)[2]
nametim=colnames(df1)[3]
nameres=colnames(df1)[4]

# 1 RUN THE ANOVA
model= aov_ez(namesub, nameres, df1, within=c(namegrp,nametim), na.rm=FALSE)
cat("\nANOVA\n\n")
model
# 2. GET THE ESTIMATED MARGINAL MEANS FOR EACH GROUP
emm= emmeans(model,namegrp,by=nametim,model="multivariate")
# 3. RUN THE CONTRASTS AGAINST THE CONTROL GROUP (0)
con= contrast(emm, "trt.vs.ctrl", ref="X0",by=nametim,adjust="bon",parens=NULL)


# GENERATE BRIEF REPORT
cat("\nCONTRASTS\n\n")
brief= as.data.frame(con)
# replace "X" in time-designations - should be in column 2 of te contrast output
brief[,2]= gsub('X','',brief[,2])
# select only significant contrasts
brief= brief[brief$p.value<.05,]
# round t & p values
brief$p.value= round(brief$p.value,5)
brief$t.ratio= round(brief$t.ratio,3)

model
brief
