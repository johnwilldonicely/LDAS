# 2-way repeated-measures ANOVA with contrasts against a control group 
# to use, pipe 4 columns with headers to this script
# column1= subject-id
# column2= within-subjects group-number - assumes group "0" is the control group 
# column3= within-subjects time
# column4= result

suppressMessages(library(afex))
suppressMessages(library(emmeans))
	
df1= read.table("stdin", sep="\t", header=T)

namesub=colnames(df1)[1]
namegrp=colnames(df1)[2]
nametim=colnames(df1)[3]
nameres=colnames(df1)[4]

# 1 run the anova
model= aov_ez(namesub, nameres, df1, within=c(namegrp,nametim), na.rm=FALSE)
cat("\nANOVA\n\n")
model

# 2. get the estimated marginal means for each group
emm= emmeans(model,namegrp,by=nametim,model="multivariate")

# 3. run the contrasts against the control group (0) 
con= contrast(emm, "trt.vs.ctrl", ref="X0",by=nametim,adjust="bon",parens=NULL)
cat("\nCONTRASTS\n\n")
con



