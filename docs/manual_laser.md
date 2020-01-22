################################################################################  
# OVERVIEW

This manual is for laser-stimulation trials run with the TAINI transmitter  

- Assumes 4 subjects are run at a time, one in each box  
- Assumes activation of the laser generates a sync-pulse for all 4 subjects  
- Hence a worksheet is required to track the box receiving each stimulus  
	- template: /opt/LDAS/docs/templates/template_taini_worksheet_laser.xlsx

One worksheet is filled per recording session, initally on paper
**NOTE: for each successful stiumulation, fill in a box in the worksheet **  

- record the paw targeted: FL, FR, BL, BR  

The worksheet is designed to track the total stims each subject receives  

- assumes that after 48 good stimuli, there should be a 10-minute break
- these blocks of 48-trials can all occur within one session
- the TAINI system can continue acquiring during the break
- this allows the sbjects to remain undisturbed 

Note that a sync pulse may also be generated when no stimulus is delivered  

- the user might miss the target
- the laser may be activated without the safety pedal being depressed
- **NOTE: for these "miss" trials, enter  an "x" instead of the targeted paw **
- for a given box, you can enter multiple x-s
- these will be recorded as "miss" trials
- for each block, the digital version of the worksheet will record the total Xs
- Example - two missed stimuli preceeding a hit to the back-left paw:
```
		XX BL
```
 **WARNING! the total boxes filled and total Xs must match the total sync-pulses for the sesion!**

- After the recording session is finished, the sheet should be scanned
- Transcribe the sheet to a digital copy of the form 

	- this file should be named according to the date and session
	- **be sure to record the name of TAINI .yaml config file for the session**


################################################################################  
## RUNNING EXPERIMENTS

### Initial setup for TAINI

1. **REFER TO** manual_taini.md: PREPARING AN EXPERIMENT
2. **REFER TO** manual_taini.md: RUNNING AN EXPERIMENT

### Documenting your laser-stimulation resorcing session

- record the session (pen & paper) using the Excel worksheet template
- after the session, complete the worksheet digitally 
- generate one worksheet for each session
- name worksheets according to the date and session 

```
	$ path="/opt/LDAS/docs/templates/"
	$ template="template_taini_worksheet_laser.xlsx"
	$ cp $path/$template ./worksheet_[yyyymmdd]-[session].xlsx 
```

- As you work from box to box with the laser, enter the paw 


3. Run the laser :  
	a. record the box each stim was delivered to
	b. 
	c.  
		$ xs-TAINI-tools check


4. **REFER TO** manual_taini.md: TRANSFERRING DATA



################################################################################
## PRE-PROCESSING DATA

1. **REFER TO**  manual_taini.txt: PRE-PROCESSING DATA

2. Extract LASER trials and add <TRIALS> section to .notes file



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

