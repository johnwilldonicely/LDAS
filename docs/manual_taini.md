These instructions pertain to all TaiNi experiments for wireless electrophysiology

- **REFER TO** manual_ldas.md for information on:
	* terminology
	* folder structrures
	* file-types

################################################################################
## RUNNING AN EXPERIMENT (ON THE ACQUISITION WORKSTATION)


### Setting up the transmitters

1. (for new batteries)]: remove stickers and allow to "warm up" for ~5 minutes

2. Insert batteries into transmitters FLAT SIDE UP
	- try to do this smoothly, not too quickly (2 seconds?)
	- ***CAUTION!*** Use electrostatic-discharge protection!

### Preparing the Taini workstation

1. Power up the receiver

2. Power up and log in to the Taini workstation

3. Open a terminal and navigate to the data-acquisition folder. Example...
		       path  study experiment 
		        |      |       | 
		$ cd ~/Data/rSNL05/Hargreaves/

4. Create a folder for today's experiment, including an empty *[date].plan* file 

		$ xs-TAINI-tools setup --expt Hargreaves_week1

5. Change to the folder, open the .plan file and record the experimental design

		$ cd *Hargreaves_week1  
		$ gedit *.plan &  

        - NOTES: record details of the experiment
	- MAPPING: record subject-IDs for each session and the session/batch/box
		- **NOTE** you must enter valid subject IDs (dossier IDs) 
		- **NOTE** check that session/batch/box are correct for each subject 
	- TRIALS: sync-pulse-defined trial-records, identifying the batch/box the sync relates to 


### Setup recording and check signals


2. Run FFT script to test transmitter signal:

	- position and orient the transmitters for good reception
	- check for FFT-peaks on channels A-B and C-D

		$ xs-TAINI-tools fft --input ab
		$ xs-TAINI-tools fft --input cd

	- make sure each transmitter is producing a spectral peak
	- if not, try removing and re-inserting the battery
	- ***CAUTION!*** Use electrostatic-discharge protection!

2. Run TainiLive:

		$ xs-TAINI-tools live

	a. Transmitter settings:
		- select the transmitters
		- make sure devices 1&2=inputA and devices 3&4=inputB
		- ** IMPORTANT ** : enter the alias (dossier ID) for each animal
		- apply settings

	b. Live: 
		- check signals for all devices
		- if sine-waves appear:
			- TainiLive is not talking to the receiver 
			- restart receiver and TainiLive
		- if packet-loss is 100% for any device:
			- check the log: make sure each device has a unique band
			- try re-applying the transmitter settings
			- try removing and re-inserting the battery
			- ***CAUTION!*** Use electrostatic-discharge protection!

	c. Recording menu: 
		- set output directory
		- set the recording duration, if required (d:h:m:s)
		- ** IMPORTANT **:  do NOT adjust the file-name format
 
3. Put the transmitters on the animals:

	- ***CAUTION!*** Use electrostatic-discharge protection!
	- ***CAUTION!*** Do not apply pressure to the antenna enclosure

	- apply the battery enclosure - secure with adhesive if required
	- place animals in recording apparatus
	- ensure signal quality is good



### Run the experiment

**REFER TO** to manual for the required type of experiment

1. TainiLive Recording menu: start recording

2. Fill out the [date].plan file as you go:
	- add a MAPPING batch section if you stop and start recording
	- keep a paper record of trials for additional information
	- add trials as they are completed 
		- ** IMPORTANT ** resave the .plan file [CTRL-s] often  

3. Conduct periodic checks 
	- are .dat files (for all subjects) being produced? 
	- are .sync files increasing in size? (if required)
	- do the .plan TRIALS records match the .sync file contents? 

		$ xs-TAINI-tools check --batch [batch] 

4. Finish recording 
	- TainiLive Recording menu: stop recording
	- Confirm .dat and .sync files were written 
		$ xs-TAINI-tools sync --file [sync-file] 
	- Record "finish" time in NOTES section 

5. Check the signals
		$ xs-TAINI-tools plot

5. Remove transmitters, returning them to a safe location 
	- ***CAUTION!*** Use electrostatic-discharge protection!

6. Cleanup 
        - NON_ESSENTIAL .dat/.sync files must be deleted or moved to ./extra/ before copying
	- Amend table_channels_[subject].txt files in case channels are dead 
	- Merge paper record with TRIALS section of the .plan file 


################################################################################
## TRANSFERRING DATA

1. Copy the [date]_[experiment] directory to the external hard-drive

2. Right-click and safely-remove the hard-drive

3. Attach to networked PC with access to the data-storage server

4. Upload [date]_[experiment directory to the server temporary transfer directory
	- example server locations:
		- smb://[name].[region].[company]/[folder]/

################################################################################
## PRE-PROCESSING DATA (ON THE ANALYSIS WORKSTATION)

1.  Transfer the files

2. Start building the data library, notes file (+channels), copying the .dat files & extracting the .bin files

		$ xs-TAINI-preproc1 *.plan 2>&1| tee "log_xs-TAINI-preproc1"

		NOTE: this includes a call to xs-TAINI-tools to rename files starting with "TAINI_"

	...or, if we just need links to the .dat file, skip copying....	

		$ xs-TAINI-preproc1 *.plan --skip d 2>&1| tee "log_xs-TAINI-preproc1"

	...or to just update the .notes file...

		$ xs-TAINI-preproc1 *.plan --skip pdb

	...or to do a batch...

		$ cd Data_Acquired
		$ xs-TAINI-preproc1 "*.plan" --skip db --batch "dir1 dir2 etc"

	...or, if these are old v0 files in which .dat =  unsigned integers...

		$ xs-TAINI-preproc1 *.plan --conv 1 2>&1| tee "log_xs-TAINI-preproc1"



3.  **REFER TO** experiment-specific manual, pre-processing section

4. Once experimental pre-processing is completed, if .dat files are not needed they can be deleted
	- you can create links to original .dat files 
		$ p=/run/user/1000/gvfs/smb-share:server=[serverlink],share=[path]
		$ q=$p""20190103_Hargreaves_base1
		$ list=$(ls --color=never $q/*dat)
		$ for i in $list ; do ln -s $i ; done
		
5. Copy data back to server
	- renamed files
	- cleaned version of sync-files if necessary
	- log files 

6. **REFER TO** manual for LDAS: EXPERIMENTAL DESIGN 


