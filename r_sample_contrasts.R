library(nlme)
library(multcomp)

# READ THE RELEVANT DATA COLUMNS INTO VARIABLES
datamatrix <- read.table('r_sample_data.txt', header = FALSE)
attach(datamatrix)
data<-(V5)
subjects<-factor(V1)
repeated<-factor(V3)
col_2<-factor(V2)

# CREATE A DATA FRAME FROM THE VARIABLES 
gDATA=data.frame(data=data, subjects=subjects, repeated=repeated, between=col_2)

# GROUP THE DATA ACCORDINGLY
gDATA<-groupedData( data ~ 1 | subjects,data = as.data.frame( gDATA ))

# MODEL THE DATA FRAME
MODEL.lme=lme(data~repeated*between, random = ~1 | subjects,data=gDATA) 

# PERFORM THE OVERALL ANOVA
anova(MODEL.lme)

# LIST THE POSSIBLE CONTRASTS
colnames(coef(MODEL.lme))

# CREATE A MATRIX (K) FOR WHICH EACH LINE SPECIFIES A CONTRAST BETWEEN  
K <- rbind(c(0,1,-1,rep(0,11)),c(0,1,0,-1,rep(0,10)),c(0,0,1,-1,rep(0,10))) 

# PRINT THE COEFFICIENTS
K

# PERFORM THE CONTRAST
CONTRAST=glht(MODEL.lme,K) 
summary(CONTRAST)

# PERFORM POST-HOC TESTS ON DATA
POSTHOC=glht(MODEL.lme, linfct = mcp(repeated = "Tukey", between="Tukey"))
summary(POSTHOC)

POSTHOC=glht(MODEL.lme, linfct = mcp(between = "Tukey"))
summary(POSTHOC)
