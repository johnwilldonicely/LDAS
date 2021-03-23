################################################################################
# 2-WAY REPEATED-MEASURES ANOVA WITH CONTRASTS AGAINST A CONTROL GROUP

# DEPENDENCIES: R, with packages emmeans and afex

# USAGE: cat [input-file] | Rscript [this-script] [control-group] [adjustment]
#    - [input-file] must have four columns with headers (column-name)
#         - col1= subject-id
#         - col2= within-subjects group-number - assumes group "0" is the control group
#         - col3= within-subjects time
#         - col4= result
#    - [control-group] must be one of the groups in column-2
#    - [adjustment] should be one of dunnettx,sidak,bonferroni

# EXAMPLE:
#     cat data.txt | Rscript  /opt/LDAS/rscripts/anova_rep_2.R  Vehicle sidak
################################################################################
suppressMessages(library(afex))
suppressMessages(library(emmeans))

args = commandArgs(trailingOnly=TRUE)
setcontrol= args[1]
setadjust= args[2]

df1= read.table("stdin", sep="\t", header=T, row.names=NULL)
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
cat("\nANOVA --------------------------------------------------------------------------\n")
model

# GENERATE BRIEF REPORT
cat("\nSIGNIFICANT-CONTRASTS ----------------------------------------------------------\n")
brief= as.data.frame(con)
# replace spaces beteween contrast levels
brief[,1]= gsub(' - ','_-_',brief[,1])
# replace "X" in time-designations - should be in column 2 of te contrast output
brief[,2]= gsub('X','',brief[,2])
# select only significant contrasts
brief= brief[brief$p.value<.05,]
# round t & p values
brief$p.value= round(brief$p.value,5)
brief$t.ratio= round(brief$t.ratio,3)
print(brief[c(1,2,7)],row.names=FALSE)

cat("\nALL-CONTRASTS ------------------------------------------------------------------\n")
con
