![logo](https://raw.githubusercontent.com/johnwilldonicely/LDAS/master/docs/figures/LDAS_logo.png)

# Contents
[* INTRODUCTION](#introduction) [* INSTALLATION](#installation) [* QUICKSTART TEST](#quickstart-test) [* MANUALS](#manuals-and-program-tags) [* GUIDELINES](#use-guidelines) [* EXPERIMENTAL DESIGN](#experimental-design) [* FILE TYPES](#file-types) [* PLOTS ](#plots)

![examples](https://raw.githubusercontent.com/johnwilldonicely/LDAS/master/docs/figures/LDAS_Sample_Plots_2.png)

# Introduction
* There are plenty of data-analysis tools available, but LDAS was designed to provide several advantages:
	* coded in C for very high-speed processing
	* minimal dependencies for quick and easy installation
	* modular design allows custom scripting for complex tasks

* Capabilities include:
	* data visualization (smart-postscript)
	* spectral power analysis (FFT)
	* coherence and correlation
	* phase-amplitude coupling
	* filters (IIR, FIR, notch)
	* large-scale automated batch-processing for entire experiments

	...and more.



################################################################################
# INSTALLATION

## Installation on Windows (estimated: 15 minutes)
**This section explains how to set up Windows 10 to run Linux in the Windows Subsystem for Linux. Once you complete this, proceed to the "Installation on Linux" section**. While LDAS is designed for maximum speed on Ubuntu/Fedora/Redhat Linux, you can now configure any Windows 10 machine to run a Linux "subsystem". 

### 1. Enable the Windows Subsystem for Linux (WSL-1)
Open Windows Powershell as Administrator and  run this command:
```
	dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
```
...then restart your PC.  

**Note:** LDAS will run under the existing (WSL-1) version of the subsystem. If you want even better fperformance, you can join the Windows Insider program to update Windows and try WSL-2. Complete instructions are **[here](https://docs.microsoft.com/en-us/windows/wsl/install-win10)**

### 2. Install the free Ubuntu distribution from Microsoft Store 
This is suprisngly simple, as Microsoft Store provides pre-tested "plug & play" versions which should install and run seamlessly undwer Windows 10. When installation cometes you'll see an Ubuntu icon in the Start Menu (recently added section) .

https://www.microsoft.com/store/apps/9N9TNGVNDL3Q

### 3. Initialise the distribution
Basically, click on the Ubuntu icon to launch, and follow these steps to set up your username and password.
https://docs.microsoft.com/en-us/windows/wsl/user-support

### 4. [optional] Configure display for handling by Xming
- LDAS will run fine without this, but if you want graphical output to be displayed in windows, you will need to install an X-windows server like Xming (see **Installation on Windows** section), and run the following commands once:
```
		$ echo '[ -z localhost:0 ] && export DISPLAY=127.0.0.1:0.0' >> ~/.bashrc 
		$ dbus-launch --exit-with-x11 s
		$ sudo dbus-uuidgen --ensure 
```

### 5. Now proceed with the **Installation on Linux**  instructions below. 


A few extra notes:  

* Your Windows drives will be accessible from /mnt/c, /mnt/d, etc. 

* To enable graphical tools like the evince document-viewer and gedit text-editor, install the free Xming server using the default options: http://www.straightrunning.com/XmingNotes/

* To view LDAS plots on Windows you will need the free ghostscript and GS-View tools
	* http://www.ghostscript.com/download/gsdnld.html
	* http://www.ghostgum.com.au/software/gsview.htm

* Running LDAS in the Windows Subsystem for Linux might be slower than running on a Linux workstation



## Installation on Linux

### 1. Make sure your Linux distribution is updated and git is installed
It's always a good idea to make sure your Linux components are up to date before starting, especially if you are freshly installing on the Windows Subsystem for Linux. Git should come with Linux, but if not...
``` 
	For Ubuntu:    $ sudo apt-get update
	Other distros: $ sudo yum update -y
```

### 2. Download the install script: *do this in your home directory*:
```
 	$ wget https://raw.github.com/johnwilldonicely/LDAS/master/LDAS_INSTALL.sh -O LDAS_INSTALL.sh
```

### 3. Make the script executable:
```
	$ chmod a+x LDAS_INSTALL.sh
```

### 4. Run the install script, specifying operating mode
* local: for the current user only  
* global: for all users, superuser priviledges will be required  
* update: for an existing installation  
	
Example install commands:
```
	$ ./LDAS_INSTALL.sh local
	$ ./LDAS_INSTALL.sh local --zip LDAS-master.zip
	$ ./LDAS_INSTALL.sh global
	$ ./LDAS_INSTALL.sh global --zip LDAS-2020_01_20.zip
	$ ./LDAS_INSTALL.sh update
```
Note that in two of the examples you do have the option to install from a previously-saved zip archive. This can be useful if you need to "roll back" to a previous LDAS version, or if you need to install on a machine which is not networked. You can download a zipped archive of LDAS here: https://github.com/johnwilldonicely/LDAS/archive/master.zip

### 4. Check for Warnings and Errors
* Are you using the most up-to-date installer?
* Do you have sudo priviledges (required for GLOBAL installs and for installing missing dependencies)
* Is there a Firewall preventing access to remote repositories?

### 5. Log out and back in again
- updates the $PATH variable so the system can find LDAS programs
- if the install ws sucessful, you may want to delete the installer in your home directory, as a new version will now be in the install location


## Extra notes on installation : 
* local: installation is for the **current user only**:
	* any user can install
	* LDAS installed in /home/$USER/bin/LDAS
	* $PATH variable updated in /home/$USER/.bashrc

* global: installation is for **all users**:
	* sudo or super-user priviledges required during installation
	* LDAS installed in /opt/LDAS/
	* $PATH variable updated in /etc/profile

* update: updates existing installation, regardless of installation scope:
	* for gloabl installs, sudo or super-user priviledges required
	* existing programs will be removed first
	* dependencies will not be checked
	* $PATH variable is not altered

#### Dependencies
* Most LDAS dependencies will come with your Linux distribution.
* If not, the LDAS_INSTALLER.sh script will have attempted to install them 
* Otherwise, they can be installed by the superuser or users with sudo-access.
	* Before installing LDAS, install dependencies with the appropriate command:
```
	Fedora/Redhat: $ sudo yum install -y [program]
	Ubuntu: 	   $ sudo apt install -y [program]
```

### Essential
LDAS will not install or will fail to run properly without these programs, but they should come with your Linux distribution. If not, use the appropriate install command.

- wget - for downloading the installer
- zip - for zipping archives
- unzip - for unzipping archives
- gcc - compiler used for C-source code
- dos2unix - required for correcting DOS-style line-breaks
- gs - ghostscript - essential for dealing with LDAS graphics
- nano - simple text editor - used for showing manuals
- evince - for viewing LDAS postscript reports

### Optional (most functionality does not require these)

#### pandoc
	- document converter, used for creating manuals
	- determine the actual version by refrring to the pandoc download page: https://github.com/jgm/pandoc/releases
```
	Ubuntu: 
		$ sudo apt install pandoc -y

	Other distros:  
		$ version=2.9.1.1
		$ tarname="pandoc-"$version"-linux-amd64.tar.gz"
		$ dest=/opt/pandoc/
		$ wget "https://github.com/jgm/pandoc/releases/download/"$version"/"$tarname
		$ sudo tar xvzf $tarname --strip-components 1 -C $dest
```
	- for pandoc to generate PDF output, addtional dependencies may be required:
		- texlive-latex-base
		- texlive-fonts-recommended


#### python3 + hdf5 support 
	- required for some of the MEA scripts
```
		$ sudo yum install python3-y libffi-devel
		$ sudo yum install -y openssl-devel
		$ python3 -m pip install h5py numpy pandas requests mne matplotlib --user
```

#### R statistical package
	- for the xs-R_* statistics scripts (ANOVA, Multiple regression, etc)
	- dependenceies here: https://mirrors.sonic.net/epel/7/x86_64/Packages/r/
```
		$ sudo yum install -y R
```

#### libreoffice 
	- for some scripts, required to convert Excell spreadsheets to CSV files
```
	Ubuntu: 
		$ sudo apt install libreoffice-common

	for a Fedora install using an RPM tarball:
		$ version=6.3.4
		$ rpmname="LibreOffice_"$version"_Linux_x86-64_rpm"
		$ tarname=$rpmname".tar.gz"
		$ urlbase="https://www.mirrorservice.org/sites/download.documentfoundation.org/tdf/libreoffice/stable/"
		$ wget $urlbase"/"$version"/rpm/x86_64/"$tarname
		$ tar -zxvf $tarname
		$ cd $rpmname/RPMS
		$ sudo yum install *.rpm
```


################################################################################
# QUICKSTART TEST

To get a feel for how LDAS works, let's make some sample data and analyse it.
When typing the following commands, omit the "$", as this represents the command-prompt

## make some data and plot it
```
$ xs-makesignal1 5 1000
$ xe-plottable1 "temp_xs-makesignal1" -xscale 1 -line 1 -ps 0
```
....use evince to view the postscript plot...
```
$ evince temp_xe-plottable1.ps &
```
## try a power spectral analysis
* Use the built-in linux command **cut** to extract the second column from the data file (exclude the timestamps), use **xe-fftpow2** to calculate the mean spectrum, and plot using **xe-plottable1**.
* Here we take advantage of the Linux pipe (|) to pass the output from one program to the next. This powerful feature of Linux eliminates the need to create intermediate files.
* Note that for LDAS programs, piped-data is read by specifying "stdin" as the filename (standard-input)

```
$ cut -f 2 temp_xs-makesignal1 |
	xe-fftpow2 stdin |
	xe-plottable1 stdin -line 1 -ps 0 -xscale 1
```
...now view the postscript plot...
```
	$ evince temp_xe-plottable1.ps &
```

## make a longer dataset with gamma-events

* 60-second dataset with massively increased gamma power every 10 seconds

```
$ xs-makesignal1 60 1000 -B g -E "-ei 10 -ed 5 -et 1 -ea 300"
```
## make a time-course heat-map for spectral power
* here we generate the fft-output ats a time-series matrix (option: -o 1), rotate and smooth the matrix, and then plot the results

```
$ cut -f 2 temp_xs-makesignal1 |
	xe-fftpow2 stdin -w 1000 -min 2 -max 150 -o 1 |
	xe-matrixmod1 stdin -r -90 -sx 10 -sy 2 -w 100 -h 100 |
	xe-plotmatrix1 stdin -xmax 60 -ymax 150  -xlabel "Time(s)" -ylabel "Frequency (Hz)"

```
...now view the postscript plot...
```
	$ evince temp_xe-plotmatrix1.ps &
```
![plotmatrix](https://raw.githubusercontent.com/johnwilldonicely/LDAS/master/docs/figures/sample_plotmatrix.jpg)

################################################################################
# USE GUIDELINES

This sections covers the ground-rules for using LDAS. A lot of this is simply good-practice when working in a Linux environment, but may not be obvious if you are used to working in Windows. This dsection does not cover manuals for individual components of LDAS (se the MANUALS section).

## File-names: no spaces, please!

File and directory (folder) names should **never** contain spaces. If you feel tempted, use an underscore or a hyphen instead. If you copy files from another system which have spaces in them, rename them using **xs-rename**.

Point-and-click operating systems like Windows have relaxed rules about spaces, but it's a disaster when trying to type commands - don't do it!

## Running commands

Every command you type in Linux begins with the name of the program you want to run. Many Linux built-in programs like "ls" will run just by typing their name and pressing [ENTER].  

Most programs also have options, often called **arguments**, which come after the program name, separated by a space. The name of the option will begin with a single or double-hyphen (- or --) - it's important to know which. For example, to list all of the contents of a directory in date-order, type:
```
	$ ls -t
```

In LDAS, the typical format for a command is:  

		[program] [input] [options]  

Here, the input will be the name of the file you want the program to work on, and the options come in name-value pairs. This means the options require a value to be set. For example, to make a scatterplot of a file called "temp.dat", adding the title "MyPlot", you would type:
```
	$ xe-plottable1 temp.dat -title "MyPlot"
```
* the program is xe-plottable1
* the input is temp.dat
* the option is -title (beginning with a hyphen)
* the value for -title is "MyPlot"

## Interrupting commands

When you type a command and press [ENTER] to execute it, normally control of the terminal passes to the program you're running until it finishes. If the proram finishes quickly you may not even notice! However if a program takes a long time to run, anything you type will be ignored until the command prompt ($) returns.

To stop a program before it finishes to regain control of the terminal:
```
	$ [CONTROL]-c
```

To get control back without stopping the current command, use this sequence:
```
	$ [CONTROL]-z
	$ bg
```
Here "bg" tells the system to keep running the last command in the background,


To run a program in the background from the outset, so you don't **have** to interrupt it, type the command and add "&" before pressing [ENTER]. For example:
```
	$ xe-plottable1 temp.dat &
```

## Running programs for long periods

***NOTE***: if you log-out of the Linux workstation, any jobs you have running will, ordinarily, be terminated. This is not a **recommended** way of stopping a program, but one that people often use accidentally.

If you are running a very long job, and don't want to sit and wait for it to finish, use "&" in combination with **nohup**. This will allow you to log-out and go home for the evening - your work will continue so long as an administraotr doesn't terminate it, and so long as the machine is not shut down. For example, our plot command, run using nohup, looks like this:

```
	$ nohup xe-plottable1 temp.dat &
```

## File-name versus "stdin"

You tell an LDAS program to work on a file either by specifying the name, or, if you are **piping** the data to the program, you substitute the filename for "stdin". Both of the commands below achieve the same thing.
```
	$ xe-plottable mydata.txt
	$ cat mydata.txt | xe-plottable1 stdin
```
For LDAS programs only, "stdin" is a special file-name for **piped** data. If you don't know what the **pipe** is in Linux, read this:

* https://opensource.com/article/18/8/introduction-pipes-linux


## Types of data LDAS works on

LDAS programs usually read one of two types of files:

* plain text
* binary

### Plain text files
Plain text files are readable by any text editor, and LDAS typically accepts one of the following types:
	* table - with our without headers
	* matrix - a 2D array of numbers

Note that with tables, if (say) your data is from two groups, **do not** put the data int two columns. Instead, you need a second column which defines the group on each row.  

For example, rather than this:
```
	g1	g2
	1	4
	2	5
	3	6
```
Your data sould look instead like this:
```
	grp	data
	1	1
	1	2
	1	3
	2	4
	2	5
	2	6
```
The second, "correct" format is what is refrred to as a data-frame in the R statistical programming language, and this is the format LDAS expects.

***NOTE***: Windows programs use a different sequence of invisible characters for the end of each line in text files. This can cause Linux programs (including LDAs) to fail to read them properly. If you are working with files that were edited in Windows, always run **dos2unix** on the file before trying to work with it.

### Binary data files

Binary data is **NOT** readable in text-editors, so you have to know the format before you process it. Binary files are usually used for large datasets because they are much more compact and much faster to read and write.

#### Simple is best!
In LDAS, the philosophy is that simpler = better. Binary file formats created by LDAS are presumed to contain a stream of numbers, all of a single type. There is no file header, no hidden text in the files, and no mixing of data types.

There are components in LDAS to read more complex binary files like HDF5, but the general strategy is to convert these to a more simple file structure, and to use a **.notes** file to describe the contents in a standard way.

#### Single-channel is fastest!
While data acquisition systems often interlace data from multiple channels, which may be necessary for efficient data writing at the time of capture, multi-channel files are never as efficient to read as a single-channel file, if you are only looking at one channel at a time.

For this reason, LDAS happily accepts ".dat" files, which we define as an interlaced file containing multiple channels of **the same type of data**, acquired in parallel. However, for the most efficient processing, LDAS prefers to break the data down into a separate file for each channel for later processing. The storage space on-disk is identical, but the gain in speed is significant.



################################################################################
# MANUALS AND PROGRAM-TAGS

1. To print the instructions for any LDAS executable to the screen, type the name of the program and press [RETURN], with no options

2. the PROGTAG.html document (see below) has a summary of all the onscreen-manuals generated by method (1) above.

3. More general instructions, including this document - can displayed at any time using **xs-manual**. Run **xs-manual** with no arguments for a guide on use.

4. Linux built-in commands like "ls" and "pwd" usually have manuals which you can access by typing "man" followed by the name of the command. For example:
```
	$ man ls
```

### Program-type tags

* There are three different types of code in LDAS, identifiable by their prefixes (xs-, xe-, xf-):
	* xe- : executable C-programs
	* xs- : executable shell-scripts written in native Bash
	* xf_ : C-functions which form components of xe- programs

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
	- holds any files describing recordng prarameters and describing trials

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

Sample listing from 14 Dec. 2018, session 0, subject 31229:

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

## [base].xyd  
- binary file: 32-bit float triplet
- records x,y and direction information
- found in the Data_Library/[base]/ folders

## [base].xydt  
- binary file: 64-bit long integer  
- records timestamps (samples) for the similarly named .xyd file  
- found in the Data_Library/[base]/ folders  


## [base].notes
- plain text file
- experimental meta-data for the current [base]
	- keyword definitions such as...
		- experiment= hargreaves_week4
		- sample_rate= 20000 Hz
	- XML section *CHANNELS* describing each channel
		- derived from the table_channels_[subject].txt files
	- XML section *TRIALS* describing each trial
		- derived using experiment-specific scripts  
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

################################################################################
# PLOTS

## XE-PLOTTABLE
- this is LDAS's base plotting tool for table (or data-frame) input, where the each column represents a different variable and each row is a case to be plotted.

### Colour palettes
#### The default colour palette
The default palette is a 32-colour repeating scale with 8 colours in each repeat - black-red-magenta-blue-cyan-green-yellow-orange.
- There are 4 repeats in this scale, with colours becoming lighter in each repeat
- colours 0-7 are dark grey(black) to dark orange (brown)
- colours 8-15 are the primary colours (8=dark grey)
- colours 16-23 are lighter
- colours 24-31 are pastel

#### The grey palette (-pal grey)
- for groups 0-max, this ranges from very dark to very light grey

#### The rainbow palette (-pal rainbow)
- this is the default LDAS palette for heat-maps, where low values are "cool" colours and high values are "hot"
- most colours except purple are represented
- for groups 0-max, colours range from blue-cyan-green-yellow-orange-red

#### The Viridis palettes (-pal viridis, plasma, magma or inferno)
- [Link to source](https://cran.r-project.org/web/packages/viridis/vignettes/intro-to-viridis.html)
- these are palettes specially designed to be perceived as a continuous scale based on luminance
- viridis: purple-blue-green-yellow
- plasma: blue-purple-yellow
- magma: black-purple-cream
- plasma: black-purple-yellow

### How colours are assigned
Colours are assigned based on the group for each case, which is determined by the user-defined group-column (option -cg). If no group-column is defined, the group for all data is taken as "zero", and the colour for the data is typically black. When the data contains multiple groups, colour definition depends on the palette.

1. for the default palette, colours are constrained to the 0-31 range as follows, depending on what the groups *are*:

	a. all integers: colour= exactly the group-ID  
	b. all numbers: colour= the group-ID rank  
	c. everything else: colour= order of appearance in the data  

> NOTE: the -colour option can be used to shift the starting point in the default palette. for example, "-colour 1" will make group 0=red, group 1-purple and so on.

 2. for all other palettes, assignment is as per b or c, above. The -colour option is disabled.







...
[END]
