These instructions are for experiments using the TAINI wireless recording system

```


```
################################################################################  
# RUNNING AN EXPERIMENT (ACQUISITION WORKSTATION)  


--------------------------------------------------------------------------------

## 1. Initialise the TAINI transmitters

#### a) For new batteries, remove stickers and allow to "warm up" for ~5 minutes

#### b) Insert batteries into the TAINI transmitters, **FLAT SIDE UP**
- try to do this smoothly, not too quickly (2 seconds?)
- ***CAUTION!*** Use electrostatic-discharge protection!


--------------------------------------------------------------------------------

## 2. Prepare the folder for data acquisition

#### a) Power up the receiver

#### b) Power up and log in to the Taini workstation

#### c) Open a terminal and navigate to the Data_Acquisition folder.
- This is a standard LDAS directory-name inside an experiments. Regardless of the path to your data, Data_Acquisition should hold raw data. Within Data_Acquisition, create a separate folder for each day. Example:

		     Path     Study      Expt   Acquisition_Folder
		     |        |          |      |
		$ cd Projects/Alzheimers/T-Maze/Data_Acquired/

	...or you may ony have a single experiment, not part of a larger study...

		$ cd T-Maze/Data_Acquired/

#### d) Create a folder for today's experiment
- The current date will be added as prefix to the directory name

		$ xs-TAINI-tools setup --expt TMAZE-day1
		$ cd 20001231_TMAZE-day1

- A channel-table template will be copied to the folder
	- you should make sure this reflects the regions for each channel

#### e) Set up trial-tracking (specific to type of experiment)
- If your recording is not divided into trials, skip this step
- Purpose: to document the trials associated with sync-pulses
- What these pulses mean depends on the setup and the experiment
- Instructions for trial-tracking are found in the LDAS script designed for processing that type of TAINI session. Example:

		$ xs-TAINI-laser0


--------------------------------------------------------------------------------

## 3. Setup recording and check signals

#### a) Run FFT script to test transmitter signal:
- position and orient the transmitters for good reception
- check for FFT-peaks on channels A-B and C-D
		$ xs-TAINI-tools fft --input ab
		$ xs-TAINI-tools fft --input cd

- make sure each transmitter is producing a spectral peak
- if not, try removing and re-inserting the battery
- ***CAUTION!*** Use electrostatic-discharge protection!

#### b) Run TainiLive:

		$ xs-TAINI-tools live

* Transmitter settings:
	- select the transmitters
	- make sure devices 1&2=inputA and devices 3&4=inputB
	- ** IMPORTANT ** : enter the alias (dossier ID) for each animal
	- apply settings

* Complete the notes section
	- this should include the keyword experiment=
	- example:
			experiment= TMAZE_baseline

* Lives section:
	- check signals for all devices
	- if sine-waves appear:
		- TainiLive is not talking to the receiver
		- restart receiver and TainiLive
	- if packet-loss is 100% for any device:
		- check the log: make sure each device has a unique band
		- try re-applying the transmitter settings
		- try removing and re-inserting the battery
		- ***CAUTION!*** Use electrostatic-discharge protection!

* Recording menu:
	- set output directory
	- set the recording duration, if required (d:h:m:s)
	- **IMPORTANT**:  do NOT adjust the file-name format

#### c) Put the transmitters on the animals:
- ***CAUTION!*** Use electrostatic-discharge protection!
- ***CAUTION!*** Do not apply pressure to the antenna enclosure
- apply the battery enclosure - secure with adhesive if required
- place animals in recording apparatus
- ensure signal quality is good


--------------------------------------------------------------------------------

## 4. Run the experiment

**REFER TO** to manual for the required type of experiment

#### a) TainiLive Recording menu: start recording

#### b) Fill out the experiment-specific trial-tracking if required
- Example, for Hargreaves trials, update the .plan file:
	- add a MAPPING batch section if you stop and start recording
	- keep a paper record of trials for additional information
	- add trials as they are completed
- **IMPORTANT** update/save the trial-tracker often!

#### c) Conduct periodic checks
- are .dat files (for all subjects) being produced?
- are .sync files increasing in size? (if required)
- do the .plan TRIALS records match the .sync file contents?
		$ xs-TAINI-tools check --batch [batch]

#### d) Finish recording
- TainiLive Recording menu: stop recording
- Confirm .dat and .sync files were written
		$ xs-TAINI-tools sync --file [sync-file]

#### e) Check the signals
		$ xs-TAINI-tools plot

#### f) Remove transmitters, returning them to a safe location
- ***CAUTION!*** Use electrostatic-discharge protection!

#### g) Cleanup
- NON_ESSENTIAL .dat/.sync files must be deleted or moved to ./extra/ before copying
- Amend table_channels_[subject].txt files in case channels are dead
- Merge paper record with TRIALS section of the .plan file

#### h) Copy today's Data_Acquired folder to an external hard-drive or server 
- this is critical, for data-security and to free space on your acquisition workstation



################################################################################
# PRE-PROCESSING DATA (ANALYSIS WORKSTATION)

1.  Transfer the files

2. Start building the data library, notes file (+channels), copying the .dat files & extracting the .bin files

		$ xs-TAINI-preproc1 "*.dat" 2>&1| tee "log_xs-TAINI-preproc1"

	...or, if we just need links to the .dat file, skip copying....

		$ xs-TAINI-preproc1 "*.dat" --skip d 2>&1| tee "log_xs-TAINI-preproc1"

	...or to just update the .notes file...

		$ xs-TAINI-preproc1 "*.dat" --skip pdb

	...or to do a batch...

		$ cd Data_Acquired
		$ xs-TAINI-preproc1 "*.dat" --skip db --batch "dir1 dir2 etc"

	...or, if these are old TainiLive V.0 files in which .dat =  unsigned integers...

		$ xs-TAINI-preproc1 "*.dat" --conv 1 2>&1| tee "log_xs-TAINI-preproc1"

3. Once experimental pre-processing is completed, if .dat files are not needed they can be deleted
	- you may wish to create links to the original .dat files

4. Process the trials for this type of experiment
- **REFER TO** experiment-specific manuals, pre-processing section

