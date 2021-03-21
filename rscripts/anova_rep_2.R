################################################################################
# 2-WAY REPEATED-MEASURES ANOVA WITH CONTRASTS AGAINST A CONTROL GROUP

# DEPENDENCIES: R, with packages emmeans and afex

# USAGE: Rscript [this-script] [input-file] [control-group] [adjustment]
#    - [input-file] must have four columns with headers (column-name)
#         - col1= subject-id
#         - col2= within-subjects group-number - assumes group "0" is the control group
#         - col3= within-subjects time
#         - col4= result
#    - [control-group] must be one of the groups in column-2
#    - [adjustment] should be one of dunnettx,sidak,bonferroni

# EXAMPLE:
# 	awk '{print $5,$1,$2,$3}' data.txt > tempfile
#	Rscript  anova_rep_2.R  tempfile  Vehicle
################################################################################
suppressMessages(library(afex))
suppressMessages(library(emmeans))

args = commandArgs(trailingOnly=TRUE)
setinfile= args[1]
setcontrol= args[2]
setadjust= args[3]

df1= read.table(setinfile, sep="\t", header=T, row.names=NULL)
namesub=colnames(df1)[1]
namegrp=colnames(df1)[2]
nametim=colnames(df1)[3]
nameres=colnames(df1)[4]

# 1 RUN THE ANOVA
model= aov_ez(namesub, nameres, df1, within=c(namegrp,nametim), na.rm=FALSE)
# 2. GET THE ESTIMATED MARGINAL MEANS FOR EACH GROUP
emm= emmeans(model,namegrp,by=nametim,model="multivariate")
# 3. RUN THE CONTRASTS AGAINST THE CONTROL GROUP (0)
con= contrast(emm, "trt.vs.ctrl", ref=setcontrol,by=nametim,adjust=setadjust,parens=NULL)

# GENERATE FULL REPORT
cat("\nANOVA ----------------------------\n\n")
model

cat("\nCONTRASTS ------------------------\n\n")
con

# GENERATE BRIEF REPORT
brief= as.data.frame(con)
# replace "X" in time-designations - should be in column 2 of te contrast output
brief[,2]= gsub('X','',brief[,2])
# select only significant contrasts
brief= brief[brief$p.value<.05,]
# round t & p values
brief$p.value= round(brief$p.value,5)
brief$t.ratio= round(brief$t.ratio,3)
cat("\nCONTRAST-SUMMARY -----------------\n\n")
brief
