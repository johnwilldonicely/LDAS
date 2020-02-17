
################################################################################
## RUNNING EXPERIMENTS

1. **REFER TO** manual_taini.md: PREPARING AN EXPERIMENT
2. **REFER TO** manual_taini.md: RUNNING AN EXPERIMENT

3. Run the Hargreaves task:
	a. record the box each stim was delivered to in the .plan file (TRIALS)
	b. record trial-durations on paper
	c. periodically make sure sync records match the .plan TRIALS 
		$ xs-TAINI-tools check

4. **REFER TO** manual_taini.md: TRANSFERRING DATA



################################################################################
## PRE-PROCESSING DATA

1. **REFER TO**  manual_taini.txt: PRE-PROCESSING DATA

2. Extract Hargreaves trials and add <TRIALS> section to .notes file

		$ cd [study]/[experiment]/Data_Acquired/
		$ xs-TAINI-hargreaves0 [plan] 2>&1| tee "log_xs-TAINI-hargreaves1"

	...or, in batch mode...

		$ cd Data_Acquired
		$ j=$(pwd)
		$ list=$(ls *Hargreaves* -d --color=never)
		$ for i in $list ; do cd $i ; xs-TAINI-hargreaves0 *.plan --verb 1 2>&1|tee log_xs-TAINI-hargreaves0.txt ; cd $j ; done


################################################################################
## ANALYSING THE DATA

1. **REFER TO** manual_ldas.txt: DATABASE CREATION

2. Make sure the [experiment]/table_groups.txt file is completed
	* requires subject, group & name columns

3. Make the database files... eg...
	$ xs-dbmake1 ../Data_Library/ --xml PATHS --table ../table_groups.txt --expt HARGREAVES > db_hargreaves_all.txt
	$ xs-dbmake1 ../Data_Library/ --xml PATHS --table ../table_groups.txt --expt HARGREAVES_baseline > db_hargreaves_baseline.txt
	$ xs-dbmake1 ../Data_Library/ --xml PATHS --table ../table_groups.txt --expt HARGREAVES_wk2 > db_hargreaves_wk2.txt
	...etc.

4. analysis 1 - all trials

	$ opts="--align stop --pre -60 --post 60 --theta 6,12 --gamma 45,65"
	$ xs-TAINI-hargreaves1b db_hargreaves_all.txt --verb 1 --opt1 "$opts" 2>&1 |tee log_xs-TAINI-hargreaves1.txt

		$ xs-TAINI-hargreaves1 [base]
		- determines trials meeting the timeout criteria

			$ xs-ldas-pow1/coh1 (list)

				$ xs-ldas5-getttrials
				- adjustment start-stop using a 3rd column
				- align trials to start or stop
				- apply --pre and --post



	...subesequently...

	$ xs-TAINI-hargreaves1 db_hargreaves_[week].txt --skip xc

	...output...

		HARGREAVES1_packetloss.txt
		HARGREAVES1_BEHAV_summary.txt

		HARGREAVES1_COH_[reg1]_[reg2]_bands.txt
		HARGREAVES1_COH_[reg1]_[reg2]_matrix.txt
		HARGREAVES1_COH_summary_avg.txt
		HARGREAVES1_COH_summary_bands.txt
		HARGREAVES1_COH_summary_noise.txt
		HARGREAVES1_COH_summary.ps

		HARGREAVES1_POW_[region]_bands.txt
		HARGREAVES1_POW_[region]_matrix.txt
		HARGREAVES1_POW_summary_avg.txt
		HARGREAVES1_POW_summary_bands.txt
		HARGREAVES1_POW_summary_noise.txt
		HARGREAVES1_POW_summary.ps
