################################################################################
# MEA analysis - multi-electrode-array data 

- data is collected using the MultiChannel Systems MC-Rack software  
- orignal sample rate is typically 10KHz for LFP recordings  
- this is increased to 50KHz for spike analysis datasets  
	- MC-Rack  spike detection gives timestamps microseconds  
	- hence:  
		- sample_rate for spikes is always 1 MHz  
		- bin_decimation for spikes is always 1000 Hz (assuming 1KHz .bin sample rate)  


- events or spikes are stored in binary .club/t file-pairs  

	- [base].club : channel (map-position)  
		- to convert to original .hdf5 channel, use *channel_list=* in [base].notes  
		- alternatively you can use the CHANNELS section in [base].notes  

	- [base].clubt: timestamps for events or spikes (sample-number)  
		- to convert to seconds, divide by *sample_rate=* in [base].notes  


################################################################################
## Naming conventions 

### folder-name 
- folder names define the base-name for subsequent merged files 
- should be [yyyymmdd]_[n] or [yyyymmdd]_[n]-spikes

### file names
- format: [yyyymmdd]-[n]_[name]_[sess].raw
- example: 20151126-3_aCSF_31oC_001.raw


################################################################################
## Spike workflow
1. Acquire at 50KHz 
2. Run spike detection and save times & waveforms as .hdf5 file 
3. Downsample and save .raw files as for LFP 
4.
	$ infile=[infile]
	$ xp-hdf5_parse1.py metadata $infile

	$ xs-MEA-hdf5 [$infile]
xs-MEAhdf5


################################################################################
## LFP workflow
1. Acquire data
2. Downsample and save .raw files
3. Copy to LDAS
	- folders named [yyymmdd]-[session]_[subject]
	- example
		20181230-001_123456

4. Create a master database file
	cd Analysis
	$ xs-dbmake1 ../Data_Library/ > db_all.txt

5. Merge the sessions and extract the channels
	$ xs-MEA-merge1 .raw
	...or...
	$ xs-batch1 db_all.txt "xs-MEA-merge1 .raw" --out log_xs-MEAmerge1

6. Run detection
	$ xs-MEA-detect3 20160108_000123 table_mapchans1.txt -s +1

7. Run rate analysis 
	$ xs-MEA-rate1 $base map_channels_good.txt -g 10 -b 40 -N aCSF -n 4


8. run power analysis


################################################################################
## Extra visualization 

### Making a flip-book of detected events
	$ xs-MEA-plotflip $base --chan 44 --plotopts "-xscale 1 -ymin -10 -ymax 10" --win 4 --trial 2 --max -1


### Visualize a block of channels of raw data
	$ list=$(xe-dbmatch1 jj1 2 14,24,34,44,54,64,74,84 -cn 1 -oc 1 | tr '\n' ',')
	$ xs-rega2 $base.dat 60 1000 

### Plot a chunk of data 
	$ xs-ldas5-plotdata1 *dat -S 1500 -s 795.54 -o 1.5 -l 5 -h 20 -w 2 -P "-lwa .25 -yint -1 -ypad 0 "



*highlight folder names* 
**note**
***warning***
