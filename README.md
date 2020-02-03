![logo](https://raw.githubusercontent.com/johnwilldonicely/LDAS/master/docs/figures/LDAS_logo.png)

# Contents
* [INTRODUCTION](#introduction) * [INSTALLATION](#installation) * [MANUALS](#manuals) * [PROGRAM TYPE TAGS](#program-type-tags) * [EXPERIMENTAL DESIGN](#experimental-design) * [FILE TYPES](#file-types) * [DEPENDENCIES](#dependencies)

# Introduction
* LDAS is a modular suite of data-analysis tools, designed for high-speed and batch processing run on Linux systems.
* LDAS programs are executed on the command-line, either directly, or remotely via a terminal connection.
* Capabilities include: 
	* data visualization (smart-postscript)
	* spectral power analysis (FFT)
	* coherence and correlation
	* phase-amplitude coupling
	* filters (IIR, FIR, notch)  

	...and more. 

* There are three principal types of code in LDAS, identifiable by their prefixes: 

	* xe- : executable C-programs  
	* xs- : executable Bash shell-scripts  
	* xf_ : C-functions which form components of xe- programs  

Intensive processing is performed by the C-programs, which are optimized for speed and low memory-usage.  The shell-scripts are used to link the C-programs together to perform more complicated jobs, and also to perform some of the file-management operations.

################################################################################
# INSTALLATION 

## Preamble 
LDAS is intended for use on Linux systems, and should run equally well on Ubuntu or Redhat/Fedora distributions. In principal LDAS should also run on Unix and OS-X systems, although this has not been tested. 

#### Installation scope: local or global

* Local installation is for the current user only:
	* any user should be able to install
	* LDAS installed in /home/$USER/bin/LDAS
	* $PATH variable updated in /home/$USER/.bashrc

* Global installation makes LDAS usable for all users:
	* sudo or super-user priviledges required during installation
	* LDAS installed in /opt/LDAS/
	* $PATH variable updated in /etc/profile

#### Installation mode: git or zip

By default, the installation script provided with LDAS installs by using the program "git" to clone the repository. This is fast and ensures you have the latest version of LDAS. If you do not have git installed on your machine, you can install it like this: 

* for Ubuntu
```
		$ sudo apt-get install git 
```

* for other flavours of Linux: 
```
		$ sudo yum install git 
```

Alternatively, you can install using a previously downloaded zipped-archive of the LDAS repo. This does not require git, and is one way of keeping backup copies of LDAS should you want to roll-back the installation. Download the latest zip-archive here:

	https://github.com/johnwilldonicely/LDAS  

## Steps to install LDAS

### 1. Download the install script:  
This should be performed in your home or download directory  
```
 		$ wget https://raw.github.com/johnwilldonicely/LDAS/master/LDAS_INSTALL.sh -O LDAS_INSTALL.sh
```

### 2. Make the script executable:  
```
		$ chmod a+x LDAS_INSTALL.sh
```

### 3. Run the script, specifying the scope of the installation.
Examples:  
```
		$ ./LDAS_INSTALL.sh local 
		$ ./LDAS_INSTALL.sh local --zip LDAS-master.zip 
		$ ./LDAS_INSTALL.sh global 
		$ ./LDAS_INSTALL.sh global --zip LDAS-2020_01_20.zip 
```

### 4. [optional] - delete the INSTALLER 
You might wnat to keep the installer if the installation was not successful. But once it is, a new copy of INSTALL_LDAS.sh will be in the installation directory and accessible from anywhere on the system. 


################################################################################
# MANUALS

* Run any LDAS executable (xe- or xs-) without arguments to print the instructions to screen.  
* Instructions - including this document - can displayed at any time using **xs-manual**. This script will show a given manual  in one of three ways:

1. to the terminal
2. as an html document (requires pandoc and Firefox to be installed)
3. as a pdf file (requires the evince document viewer)

For example:  
```  
		$ xs-manual ldas  
		$ xs-manual ldas --make html  
		$ xs-manual ldas --veiw pdf
```  

################################################################################
# PROGRAM TYPE TAGS

The code for LDAS programs and scripts usually has an XML "tag" section defining the category to which the program belongs. Tags are lowercase by default. You can use the **xs-progtag** script to find or list programs scripts or functions with a particular tag. For example...  
```
		$ xs-ldas-progtag find "plot"  
```

A full description of all LDAS components can be found in docs/PROGTAG.html

* [click to preview](https://htmlpreview.github.io/?https://raw.github.com/johnwilldonicely/LDAS/master/docs/PROGTAG.html)
* [right-click to download](https://raw.github.com/johnwilldonicely/LDAS/master/docs/PROGTAG.html)


### Preprocessing scripts for particular acquisition systems

Most LDAS scripts and programs are designed as general tools, but some were designed to work with specific acquisition systems, or assume your data is organized according to the LDAS file/folder experiment conventions (see EXPERIMENTAL DESIGN, below). Here are some examples of patform-specific modules: 

* xs-TAINI- : data generated by TAINI wireless electrophysiology systems  
* xs-O2-    : oxygen amperometry data from CHART  
* xs-SLICE- : slice-electrophysiology data from WinLTP  
* xs-MEA-   : from Multi-Electrode-Array (MEA) systems  


################################################################################
# EXPERIMENTAL DESIGN

LDAS is not just a suite of analysis tools - it is also a system of organizing experimental data for analysis. Most LDAS programs can be used in almost any context, but some assume that data is organized in this way.


### Terminology

	[date] the date a recording session started
		- should be in [yyyymmdd] format, e.g. 20001231

	[study] the over-arching study involving a cohort of subjects
		- a study may involve multiple experiments on the same cohort

	[experiment] a particular type of test conducted in the study
		- example 24-hour, Hargreaves, Reactivation, etc
		- experiments may overlap across days
			- hence a separate directory for each experiment!

	[session] the recording-session for a group of subjects
		- triple-digit incrementing (from 000) part of the file name

	[trial] subdivision of a session
		- this could be a replicate (trial1, trial2, trial3, etc)
		- could also be a stage (FAM1, SLEEP1, NOVEL1, SLEEP2, etc)
		- how the session is divided into trials is defined in a .notes file
			-see APPENDIX: FILE TYPES

	[subject] the unique subject ID (e.g. dossier ID).  
		- must be numeric

	[channel] the channel-number
		- triple-digit incrementing from 000

	[base] folder-names and file-prefices
		- typically [date]-[session]_[subject]
		- e.g. 20001231-002-00025 (session 2 for subject 25, 2000/12/31


### Folder Structure

LDAS assumes that in a given experiment, on a given day, for a given subject, there should be consistent experimental parameters which apply to all the acquired data. These parameters include things like the type of experiment, the sampling-rate, the location of recording electrodes, the times defining independent trials, and so on.

Based on this assumption, the base-name [base] (see above) is the foundation of file- and directory-naming within an experiment. Each experiment should have the folowing four sub-directories...

1. *Analysis*
	- batch-analysis control and output
	- holds database (db*.txt) files defining paths to Data_Working

2. *Data_Acquired*
	- holds raw-data in [day] sub-directories
	- holds a .plan file describing the experimental parameters

3. *Data_Library*
	- holds renamed, processed data files and corresponding .notes
	- these are held in [base] sub-directories

4. *Data_Working*
	- holds links to the Data_Library files in [base] sub-directories
	- using links avoids the risk of accidentally altering the data
	- analyses are performed in the sub-directories
		- indexed by database files in Analysis
	- these folders also hold the local output of analysis

The folders are nested as follows:
```
* [date]-[study]/[experiment]/
	- file:  README.txt
	- files: table_channels_[subject].txt
	* Analysis/
		- file:  table_groups.txt
		- file:  table_groupnames.txt
		- files: db[name].txt
		- files: collated results from batch analyses
	* Data_Acquired/
 		* [date]_[description]/
 			- files: [base].dat
 			- files: [base].sync
 			- file:  [base].plan
	* Data_Library/
 		* [base]/
 			- files: [base]-[channel].bin
 			- file:  [base].dat
 			- file:  [base].sync
 			- file:  [base].notes
 			- file:  [base]-lost.ssp
	* Data_Working/
 		* [base]/
 			- links: Data_Library/[base]*
			- files: local output from analyses
```

Sample listing:  

```
	$ ls Data_Working/20181214-000_31229/  
	20181214-000_31229-000.bin  20181214-000_31229-010.bin  
	20181214-000_31229-001.bin  20181214-000_31229-011.bin  
	20181214-000_31229-002.bin  20181214-000_31229-012.bin  
	20181214-000_31229-003.bin  20181214-000_31229-013.bin  
	20181214-000_31229-004.bin  20181214-000_31229-014.bin  
	20181214-000_31229-005.bin  20181214-000_31229-015.bin  
	20181214-000_31229-006.bin  20181214-000_31229.dat  
	20181214-000_31229-007.bin  20181214-000_31229-lost.ssp  
	20181214-000_31229-008.bin  20181214-000_31229.notes  
	20181214-000_31229-009.bin  20181214-000_31229.sync  
```

### Preparing an experiment (on the data-server)

1. Create LDAS space: [date]-[study]

2. Create subfolders:
	- Analysis
	- Data_Acquired
	- Data_Library
	- Data_Working

3. Add tables to *Analysis* directory  
		
```
		$ cd [study]/[experiment]  
		$ echo -e "subject\\tgroup" > Analysis/table_groups.txt  
		$ echo -e "group\\tname" > Analysis/table_groupnames.txt  
```
		
4. Build links to files in *Data_Library* in *Data_Working*: 
	- the working-data directories are where analyses are perfomed
	- this protects the original files in the Data_Library  
	
```
		$ cd [study]/[experiment]  
		$ xs-makelink1 Data_Library Data_Working --patterns BASE  
```
		
5. Make a database file:  
	- defines the [date]_[subject] paths to *Data_Working* folders  
	- also defines the group-id for each path  
	- **NOTE**: LDAS groups must be integers (0,1,2,3 etc).  
	- subject-groups can be defined in *table_groups.txt*, if available  
	- group-names may be defined in *table_groupnames.txt* if available  
	- these names may be incorporated into the db-file header  
	
```
		$ cd [study]/[experiment]/Analysis  
		$ t1=table_groups.txt  
		$ t2=table_groupnames.txt  
		$ opts="--xml PATHS --groups $t1 --names $t2 --expt HARGREAVES"  
		$ xs-dbmake1 ../Data_Working/ $opts > db_all.txt  
```
		
	- see *APPENDIX FILE TYPES / db_[name].txt* for an example db-file


### Batch analysis

LDAS has specialized scripts to perform complex analyses. These scripts often have a batch-script compliment, typically of the same name, but ending in "b". Example:  

```  
		$ xs-ldas5-XHAR1 [base] [options] : Hargreaves analysis for one folder
		$ xs-ldas5-XHAR1b [db] [options]  : batch analyse entire experiment
```  
		
These batch scripts require a database [db] file to specify which *Data_Working* directories to use for the analyses. As described above, this db-file includes group-ids originally defined in a separate *table_groups.txt* file, and may also incorporate roup-names from the *table_groupnames.txt* file. 
However, any db-file can be modified in order to...

	- remove or comment-out (#) paths which should be exluded from analysis 	
	- change group-ids to allow for date-specific subject treatments
	- change group-names originally derived from *table_groupnames.txt*

################################################################################
## ANALYSES 

### Aligning data to trials or events

There are three ways of aligning recorded data to trials or evens using LDAS:  

1. The <TRIALS> section in the .notes file  
	- best for defining large blaocks of data defining a phase of an experiment  
	- typically defines:
		- trial(number)
		- start(sample)
		- stop(sample)
		- seconds(duration)
		- name(text) 

2. A .ssp file in the data folder  
	- all events in a given .ssp file are of the same type  

3. A .cmt file in the data folder  
	- each event can be of a diffeerent type  

################################################################################
# FILE TYPES  

These are some basic filetypes used by LDAS.

## [base].dat  
- binary, 16-bit (2-byte) signed integer
- multi-channel (interlaced) acquisition data
	- for TAINI systems:
		- 16 channels
		- 12-bit range, uv_per_unit= 3.1002
		- offset= 537 units, approximately
		- sample_rate= 19531.25 Hz
- found in Data_Acquired/[date]/ and Data_Library/[base]/ folders  

## [base]-[channel].bin  
- binary, 32-bit (4-byte) floating-point values
- single-channel, extracted from .dat file
	- anti-aliased at 2KHz
	- de-meaned with a 100s window
	- downsampled to 1KHz
- filename suffix indicates the channel-number (zero-offset)
	- e.g. 20181214-000_31229-015.bin
- sample_rate= 1000 Hz , typically
- found in the Data_Library/[base]/ folders

## [base].cmt
	- a way of recording the time of multipe types of events
	- very useful for behavioural experiments
	- an alternative to defining many "trials" in the .notes file
	- format: <timestamp> <description>
	- <description> should be a single word with underscores separating fields
	- e.g.: 
```
		1000	STIM_ON_LEFT
		2000	STIM_OFF_LEFT
```
		
	- timestamps are usually in seeconds for O2 experiments

## [base].notes
- plain text file
- experimental meta-data for the current [base]
	- keyword definitions such as...
		- experiment= hargreaves_week4
		- sample_rate= 20000 Hz
	- XML section *CHANNELS* describing each channel
		- derived from the table_channels_[subject].txt files
	- XML section *TRIALS* describing each trial
		- derived from the [dat].plan file in Data_Acquired/[date] directories
		- includes trial number & name
		- includes start- & stop- samples (relative to .dat file)
- found in the Data_Library/[base]/ folders

## .ssp
- binary, 64-bit (8-byte) signed long integers
- start-stop pairs (SSP) of values
	- start and stop sample-numbers for a block of data
	- sample-numbers refer to the original .dat file
	- the stop-sample is not considered part of the block
- found in the Data_Library/[base]/ folders

## [base]-lost.ssp
- a .ssp file as described above, capturing the periods of lost-packets in a .dat file
- found in the Data_Library/[base]/ folders

## table_groups.txt
- plain text file with a header-line specifying "subject" and "group"
- defines group membership for each subject (if applicable)
- used in the generation of database files
- found in the [Analysis] folder
- example:
```
	subject	group  
	31229	0  		
	31230	1  
	31231	2  
	31232	0  
	31233	1  
	31234	2  
```

## table_groupnames.txt
- plain text file with a header-line specifying "group" and "name"
- groups in LDAS must be numbers - ideally integers
- therefore, this file defines the name for each group
- by convention, group-zero is the control-group
- used in the generation of database files, applying plot legends
- found in the [Analysis] folder
- example:
```
	group	name
	0	SHAM
	1	LESION
```

## db_[name].txt
- plain text file with a header-line specifying "path" and "group"
- used to control batch-analysis of data
	- points to paths containing data to be analysed
	- defines the group for that date/subject
	- lines can be commented out or deleted to control analysis
	- group-values can be modified to allow (eg) sub-groups by date
- found in the [Analysis] folder
- example
```
	# experiment= HARGREAVES
	# group_0= SHAM
	# group_1= LESION
	<PATHS>
	path	group
	../Data_Working/20181214-000_31229	0
	../Data_Working/20181214-000_31230	1
	#../Data_Working/20181214-000_31231	0
	../Data_Working/20181214-000_31232	1
	../Data_Working/20181214-000_31233	0
	../Data_Working/20181214-000_31234	1
	</PATHS>
```
...
[END]



################################################################################
# DEPENDENCIES
* Most LDAS dependencies will come with your Linux distribution.  
* If not, they can be installed by the superuser or users with sudo-access.  
* Before installing LDAS, install dependencies with the appropriate command:
```
	Fedora/Redhat: 	$ sudo yum install -y [program] 
	Ubuntu: 	$ sudo apt-get install -y [program] 
```

Most of the examples below describe a Fedora/Redhat installation. You may have to look online for specific install instructions for different versions of Linux


## Essential
LDAS will not install or will fail to run properly without these programs, but they should come with your Linux distribution. If not, use the appropriate install command. 

- wget - for downloading the installer  
- zip - for zipping archives  
- unzip - for unzipping archives  
- gcc - compiler used for C-source code  
- dos2unix - required for correcting DOS-style line-breaks  
- gs - ghostscript - essential for dealing with LDAS graphics  
- nano - simple text editor - used for showing manuals  

## Optional (most functionality does not require these)

- pandoc - document converter, used for creating manuals 
```
		$ version=2.9.1.1
		$ tarname="pandoc-"$version"-linux-amd64.tar.gz" 
		$ dest=/opt/pandoc/
		$ wget "https://github.com/jgm/pandoc/releases/download/"$version"/"$tarname
		$ sudo tar xvzf $tarname --strip-components 1 -C $dest
```

- git - Useful for install-management of LDAS
```
		$ sudo yum install -y git 
```

- python3 + hdf5 support (required for some of the MEA scripts)
```
		$ sudo yum install -y libffi-devel
		$ sudo yum install -y openssl-devel
 		$ v=$(lsb_release -a | grep Release: | awk '{print $2}' | cut -f 1 -d .)
		$ if [ $v == "7" ] ; then p="3.7.3" ; else p="3.5.7" ; fi 
		$ wget https://www.python.org/ftp/python/$p/Python-$p.tgz
		$ tar -xvf Python-$p.tgz
		$ cd Python-$p/
		$ ./configure
		$ make
		$ make install - this doesn't work without sudo trace?
		$ python3 -m pip install h5py numpy pandas requests
		$ python3 -m pip install mne
		$ python3 -m pip install matplotlib
```

- R - for some of the xr-* statistics scripts (ANOVA, Multiple regression, etc)
		... dependenceies here: https://mirrors.sonic.net/epel/7/x86_64/Packages/r/  
```
		$ sudo yum install -y zvbi-fonts-0.2.35-1.el7.noarch.rpm
		$ sudo yum install -y tre-common-0.8.0-18.20140228gitc2f5d13.el7.noarch.rpm
		$ sudo yum install -y tre-0.8.0-18.20140228gitc2f5d13.el7.x86_64.rpm
		$ sudo yum install -y tre-devel-0.8.0-18.20140228gitc2f5d13.el7.x86_64.rpm
		$ sudo yum install -y libRmath-3.5.2-2.el7.x86_64.rpm
		$ sudo yum install -y libRmath-devel-3.5.2-2.el7.x86_64.rpm
		$ sudo yum install -y openblas-Rblas-0.3.3-2.el7.x86_64.rpm
		$ sudo yum install -y R-core-3.5.2-2.el7.x86_64.rpm
		$ sudo yum install -y R-core-devel-3.5.2-2.el7.x86_64.rpm
		$ sudo yum install -y R-java-3.5.2-2.el7.x86_64.rpm
		$ sudo yum install -y R-java-devel-3.5.2-2.el7.x86_64.rpm
		$ sudo yum install -y R-devel-3.5.2-2.el7.x86_64.rpm
```

- libreoffice - for some LASER scripts, required to convert Excell spreadsheets to CSV files  
	- this example is for a Fedora install using an RPM tarball 
```
		$ version=6.3.4
		$ rpmname="LibreOffice_"$version"_Linux_x86-64_rpm" 
		$ tarname=$rpmname".tar.gz" 
		$ urlbase="https://www.mirrorservice.org/sites/download.documentfoundation.org/tdf/libreoffice/stable/"
		$ wget $urlbase"/"$version"/rpm/x86_64/"$tarname
		$ tar zxvf $tarname 
		$ cd $rpmname
		$ cd RPMS
		$ sudo yum install *.rpm
```

## The following are not required, but can be useful

- atom ... optional editor for managing LDAS: https://atom.io/download/rpm
```
		$ sudo yum install -y atom.x86_64.rpm
```
