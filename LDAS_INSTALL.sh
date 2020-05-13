#!/bin/bash

# <TAGS>programming ldas</TAGS>
thisprog=`basename "$0"`
tempfile="temp_"$thisprog
tempdir="/home/$USER/temp_LDAS_install"

# colours for use with optional error messages
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color
bar="############################################################"

################################################################################
# INITIAL VARIABLE DEFINITIONS
################################################################################
# for most scripts...
thisprog=`basename "$0"`
progpath=$(dirname $(readlink -f "$0"))
progbase1=$(echo $thisprog | awk -F - '{print toupper($NF)}')
allopts=$@

tempfile="temp_"$thisprog #"."$$"."
startdir=$(pwd)
start_time=$(date +'%s.%3N')
date0=$(date)
let previnstall=0

setsource="https://github.com/johnwilldonicely/LDAS/"
setwget="https://raw.github.com/johnwilldonicely/LDAS/master/"
listdep="zip unzip wget gcc git ghostscript dos2unix nano pandoc"

setscope="local"
setdest="/home/$USER/bin/"
setrc="/home/$USER/.bashrc"
setfilezip=""
setupdate="0"
setverb="1"
setclean="1"


################################################################################
# DETECT OS, AND, IF IT IS LINUX, DETECTS WHICH LINUX DISTRIBUTION
################################################################################
function func_os () {

	OS=`uname -s`
	REV=`uname -r`
	MACH=`uname -m`

	if [ "${OS}" = "SunOS" ] ; then
		OS=Solaris
		ARCH=`uname -p`
		OSSTR="${OS} ${REV}(${ARCH} `uname -v`)"

	elif [ "${OS}" = "AIX" ] ; then
		OSSTR="${OS} `oslevel` (`oslevel -r`)"

	elif [ "${OS}" = "Linux" ] ; then
		KERNEL=`uname -r`
		if [ -f /etc/os-release ] ; then
			DIST=`cat /etc/os-release | tr -d '"' | awk -F = '$1=="NAME"{print $2}'`
			PSEUDONAME=`cat /etc/os-release | awk -F = '$1=="PRETTY_NAME"{print $2}'`
			REV=`cat /etc/os-release | tr -d '"' | awk -F = '$1=="VERSION_ID"{print $2}'`
		elif [ -f /etc/redhat-release ] ; then
			DIST=`cat /etc/redhat-release | tr "\n" ' '| sed s/VERSION.*//`
			PSEUDONAME=`cat /etc/redhat-release | sed s/.*\(// | sed s/\)//`
			REV=`cat /etc/redhat-release | sed s/.*release\ // | sed s/\ .*//`
		elif [ -f /etc/SuSE-release ] ; then
			DIST=`cat /etc/SuSE-release | tr "\n" ' '| sed s/VERSION.*//`
			REV=`cat /etc/SuSE-release | tr "\n" ' ' | sed s/.*=\ //`
		elif [ -f /etc/mandrake-release ] ; then
			DIST='Mandrake'
			PSEUDONAME=`cat /etc/mandrake-release | sed s/.*\(// | sed s/\)//`
			REV=`cat /etc/mandrake-release | sed s/.*release\ // | sed s/\ .*//`
		elif [ -f /etc/debian_version ] ; then
			DIST="Debian `cat /etc/debian_version`"
			REV=""
		fi

		if [ -f /etc/UnitedLinux-release ] ; then
		        DIST="${DIST}[`cat /etc/UnitedLinux-release | tr "\n" ' ' | sed s/VERSION.*//`]"
		fi

		OSSTR="${OS}#${DIST}#${REV}#${PSEUDONAME}#${KERNEL}#${MACH}"
	fi

	echo ${OSSTR}
}


################################################################################
# DEFINE PERMISSION FUNCTION: CALL FORMAT: status=$(func_permission $path yes|no)
################################################################################
function func_permission () {
	# READ THE ARGUMENTS
	zpath=$1 # the path to check
	zerror=$2 # "yes" if fail triggers error message + exit
	# DETERMINE BASIC INFO
	zaccess="no"
	if [ ! -e $zpath ] ; then
		zaccess="missing"
	else
		zinfo=( $(stat -L -c "0%a %G %U" $zpath) ) # Use -L to get information about the target of a symlink, not the link itself
		zperm=${zinfo[0]}
		zgroup=${zinfo[1]}
		zowner=${zinfo[2]}
		# DETERMINE WRITE-ACCESS LEVEL
		if (( ($zperm & 0002) != 0 )); then 	# everyone has access
			zaccess="yes everyone"
		elif (( ($zperm & 0020) != 0 )); then 	# some group has write access. Is setuser in that group?
			gs=( $(groups $USER) )
			for g in "${gs[@]}"; do
				if [[ $zgroup == $g ]]; then zaccess="yes group" ; break ; fi
			done
		elif (( ($zperm & 0200) != 0 )); then # The owner has write access. Does the setuser own the file?
			[[ $USER == $zowner ]] && zaccess="yes owner"
		fi
	fi
	# HANDLE ERROR MESSAGES IF REQUIRED
	if [ "$zerror" == "yes" ] ; then
		if [ "$zaccess" == "missing" ] ; then
			echo -e "--- Warning ["$thisprog"]: $zpath does not exist"
			exit
		elif [ "$zaccess" == "no" ] ; then
			echo -e "--- Warning ["$thisprog"]: $USER has no write-permission for $zpath"
			echo -e "\tsudo access will be required during \"global\" installation"
			echo -e "\t... or, run using \"local\" scope"
			echo -e "\t... or, run this script as superuser"
			exit
		fi
	else
		echo "permission= "$zaccess
	fi
}


################################################################################
# PRINT INSTRUCTIONS IF NO ARGUMENTS ARE GIVEN
################################################################################
if [ $# -lt 1 ]; then
	echo
	echo "--------------------------------------------------------------------------------"
	echo $thisprog": LDAS installation/update  script"
	echo "	- full install or update, from GitHub (default) or a .zip archive"
	echo "		install: "
	echo "			- checks operating system & permissions"
	echo "			- checks dependencies"
	echo "			- updates the \$PATH variable"
	echo "			- extracts & compiles the C source-code"
	echo "			- updates manuals and PROGTAG.html"
	echo "		update: "
	echo "			- extracts & compiles the C source-code"
	echo "			- updates manuals and PROGTAG.html"
	echo ""
	echo "USAGE: $thisprog [mode] [options]"
	echo ""
	echo "	[mode]: local,global, or update"
	echo "		- local:  install, current user only (local scope)"
	echo "		- global: install, all users (global scope)"
	echo "		- update: update existing LDAS"
	echo ""
	echo "VALID OPTIONS (defaults in []):"
	echo "	--zip: install/update  LDAS from this zip-file [$setfilezip]"
	echo "		- if unset, use \"git clone\" to get the latest repo from GitHub"
	echo "	--verb: verbose output (0=NO 1=YES) [$setverb]"
	echo "	--clean: remove temporary files (0=NO 1=YES) [$setclean]"
	echo ""
	echo "EXAMPLE: "
	echo "	"$thisprog" global --zip LDAS_master.zip"
	echo "--------------------------------------------------------------------------------"
	echo
	exit
fi
echo -e "\n################################################################################"
echo -e "LDAS INSTALLER..."
echo -e "################################################################################"

########################################################################################
# ARGUMENT HANDLING
########################################################################################
setmode=$1 ; shift

if [ "$setmode" == "local" ] ; then setscope="local" ; setupdate="0" ;
elif [ "$setmode" == "global" ] ; then setscope="global" ; setupdate="0"
elif [ "$setmode" == "update" ] ; then setupdate="1"
else echo -e "$RED\n--- Error ["$thisprog"]: invalid mode ($setmode) -  must be \"local\" or \"global\" or \"update\"\n$NC" ; exit ; fi

# OPTIONAL ARGUMENT HANDLING
vs="v:c:" ; vl="zip:,verb:,clean:"
y=$(getopt -o $vs -l $vl -n "" -- "$@" 2>&1 > /dev/null)
if [ "$y" != "" ] ; then { echo -e "\n--- Error ["$thisprog"]"$y"\n" ; exit ; }
else eval set -- $(getopt -o $vs -l $vl -n "" -- "$@") ; fi
while [ $# -gt 0 ] ; do
	case $1 in
		--zip ) setfilezip=$2 ; shift ;;
		-v | --verb ) setverb=$2 ; shift ;;
		-c | --clean ) setclean=$2 ; shift ;;
		-- ) shift ; break ;;
		* ) ;;
	esac
	shift
done
if [ "$setverb" != "0" ] && [ "$setverb" != "1" ] ; then { echo -e "\n--- Error ["$thisprog"]: invalid --verb ($setverb) -  must be 0 or 1\n" ;  exit; } ; fi
if [ "$setclean" != "0" ] && [ "$setclean" != "1" ] ; then { echo -e "\n--- Error ["$thisprog"]: invalid --clean ($setclean) -  must be 0 or 1\n" ;  exit; } ; fi


################################################################################
# PRELIMINARY SETUP AND ERROR CHECKS
################################################################################
# DETERMINE LINUX VERSION AND REPORT
z=$(func_os)
if [ "$z" != "" ] ; then
	distro=$(echo $z | awk -F '#' '{print $2}')
	version=$(echo $z | awk -F '#' '{print $3}')
	pseudoname=$(echo $z | awk -F '#' '{print $4}')
fi


# CHECK IF LDAS IS ALREADY INSTALLED - CHECK PATH
prevdest=$(which xs-template 2>/dev/null | rev | cut -f 2- -d / |rev)
if [ "$prevdest" == "" ] ; then {
	# if not installed, update cannot be performed
	if [ "$setupdate" == "1" ] ; then
		echo -e "$RED\n--- Error ["$thisprog"]: LDAS not installed, can't use \"update\" mode\n\tPerform full install instead\n$NC" ; exit
	fi
}
# determine previous-install scope - make sure path is valid
elif [[ $prevdest =~ "/opt/LDAS" ]] ; then
	prevscope="global"
	prevrc="/etc/profile"
	prevnano="/etc/nanorc"
elif [[ $prevdest =~ "/home/$USER/bin/LDAS" ]] ; then
	prevscope="local" ;
	prevrc="/home/$USER/.bashrc"
	prevnano="/home/$USER/.nanorc"
else
	echo -e "$RED\n--- Error ["$thisprog"]: LDAS previously installed in invalid location\n\t- path: $prevdest\n\t- manually remove and retry\n$NC" ; exit
fi

# for updates, use the existing scope
if [ "$setupdate" == "1" ] ; then setscope=$prevscope ; fi

# DETERMINE DESTINATION, PATH-DEFINITION, & NANORC OPTIONS, DEPENDING ON SELECTED SCOPE
# local= current user only, global= all users
if [ "$setscope" == "local" ] ; then
	setdest="/home/$USER/bin"
	setrc="/home/$USER/.bashrc"
	setnano="/home/$USER/.nanorc"
	mkdir -p $setdest
elif [ "$setscope" == "global" ] ; then
	setdest="/opt"
	setrc="/etc/profile"
	setnano="/etc/nanorc"
fi


# REPORT ON SYSTEM AND CURRENT INSTALL
echo -e "- Linux distro: $pseudoname"
if [ "$setupdate" == 0 ] ; then
	if [ "$prevdest" != "" ]  ; then echo -e "- previous install path: $prevdest" ; fi
	echo -e "- proposed install path: $setdest/LDAS"
else
	echo -e "- proposed update to LDAS: $setdest/LDAS"
fi

# CHECK WHETHER PROGRAM IS BEING RUN IN DEST OR TEMP DIRECTORY
x=$(pwd)
y=$setdest"/LDAS"
case $x in *$y*) echo -e $RED"\n--- Error ["$thisprog"]: do not run installer in the destination directory\n\t - $y will be overwritten\n"$NC ; exit ;; esac
y=$tempdir
case $x in *$y*) echo -e $RED"\n--- Error ["$thisprog"]: do not run installer in the temp directory\n\t - $y will be overwritten\n"$NC ; exit ;; esac

# FOR FULL-INSTALL, CHECK WHETHER INSTALLER BEING USED IS FROM THE PREVPATH
# because really a fresh install should use the latest installer
if [ $prevdest != "" ] && [ "$setupdate" == "0" ] ; then
	x=$(which $0)
	y=$prevdest
	case $x in *$y*)
		echo -e "\n"$RED
		echo -e "--- Error ["$thisprog"]: using an \"installed\" version of the installer"
		echo -e "\t- this should only be used for updating (--update 1)"
		echo -e "\t- for full installs, get the latest installer & run it locally (./)\n"
		echo -e "\t- use the following commands:\n"
		echo -e "cd ~/"
		echo -e "url=\"$setwget\""
		echo -e "wget \$url/$thisprog -O $thisprog"
		echo -e "chmod a+x ./$thisprog"
		echo -e "./$thisprog $setscope"
		echo -e $NC"\n"
		exit
	;;
	esac
fi

# CHECK CRITICAL WRITE-PERMISSIONS
if [ $(func_permission ./ "no" | cut -f 2 -d ' ') == "no" ] ; then
	echo -e $RED"\n--- Error ["$thisprog"]: cannot write to the current directory\n"$NC ; exit
fi
if [ $(func_permission $tempdir | cut -f 2 -d ' ') == "no" ] ; then
	echo -e $RED"\n--- Error ["$thisprog"]: cannot write to temp directory ($tempdir)\n"$NC ; exit
fi

# CHECK SUDO-REQUIRED WRITE-PERMISSIONS
rm -f $tempfile".1"
for x in  $prevdest $setdest $prevrc $setrc $prevnano $setnano  ; do
	if [ $(func_permission $x "no" | cut -f 2 -d ' ') == "no" ] ; then echo $x >> $tempfile".1" ; fi
done
if [ -e $tempfile".1" ] ; then
	echo -e $GREEN"--- Warning ["$thisprog"]: $USER has no write-permission for:"$NC
	cat $tempfile".1" | awk '{print "\t\t"$0}'
	echo -e $GREEN"\tsudo access will be required during \"global\" installation"
	echo -e "\t... or, run using \"local\" scope"
	echo -e "\t... or, run this script as superuser\n"$NC
fi


################################################################################
# CHECK TO PROCEED AND BACKUP EXISTING INSTALL
################################################################################
if [ "$prevdest" != "" ] ; then

	# CHECK FOR PROCEEDING
	if [ "$setupdate" == "0" ] ; then
		echo -e "\n--- SYSTEM WILL BE CONFIGURED FOR $setscope INSTALLATION"$GREEN
		read -p  "--- WARNING: this will overwrite the previous install - proceed? [y/n] " answer
		echo -en "$NC"
		while true ; do case $answer in [yY]* ) break ;; *) echo ; exit ;; esac ; done
	elif [ "$setupdate" == "1" ] ; then
		echo -e "$GREEN"
		read -p  "--- WARNING: this will overwrite the previous install - proceed? [y/n] " answer
		echo -en "$NC"
		while true ; do case $answer in [yY]* ) break ;; *) echo ; exit ;; esac ; done
	fi

	# OFFER TO BACKUP
	backup="/home/$USER/LDAS_backup_"$(date +'%Y%m%d')".zip"
	echo -en "$GREEN"
	read -p  "--- BACKUP $prevdest to $backup? [y/n] " answer
	echo -en "$NC"
	while true ; do case $answer in [yY]* ) echo -e "$NC\t...backing up..." ; zip -qr $backup $prevdest ; break ;; *) break ;; esac ; done

fi

########################################################################################
# CHECK DEPENDENCIES - FULL INSTALL ONLY
########################################################################################
if [ "$setupdate" == "0" ] ; then
	# DEFINE THE INSTALL COMMAND 
	if [ "$distro" == "Ubuntu" ] ; then
		command="apt-get -y install"
	else
		command="yum -y install"
	fi

	rm -f $tempfile".2"
	for x in  $listdep ; do
		if [ "$(command -v $x)" == "" ] ; then echo $x >> $tempfile".2" ; fi
	done
	if [ -e $tempfile".2" ] ; then
		echo -e $GREEN
		echo -e "\n--------------------------------------------------------------------------------"
		echo -e "--- Warning ["$thisprog"]: missing dependencies:"
		cat $tempfile".2" | awk '{print "\t\t"$0}'
		echo -e "\tWill attempt to install these using $NC $command <program>"$GREEN
		echo -e "\t... or, run this script as superuser\n"
		echo -e "\t... or, ask your superuser to install the dependencies"
		echo -e $NC"\n"

		for dep in $(cat $tempfile".2") ; do 
			echo -e $GREEN"\t\t - attempting to install $dep..."$NC
			sudo $command $dep
		fi
	fi
fi


################################################################################
# RETRIEVE OR EXTRACT LDAS TO TEMP FOLDER
################################################################################
if [ "$setclean" == "1" ] && [ "$tempdir" != "" ] ; then rm -rf $tempdir/* ; fi
mkdir -p $tempdir
echo -e "\n--------------------------------------------------------------------------------"
if [ "$setfilezip" != "" ] ; then
	echo -e "EXTRACTING LDAS FROM $setfilezip ..."
	if [ ! -e "$setzipfile" ] ; then { echo -e $RED"\n--- Error ["$thisprog"]: missing zip-file $setzipfile\n"$NC ;  exit; } ; fi

	unzip -oq $setfilezip -d $tempdir
	# find the most recent LDAS folder created
	z=$(ls -dt1 $tempdir/*LDAS*/ | head -n 1 | awk -F / '{print $(NF-1)}' )
	# rename to LDAS
	mv $tempdir/$z $tempdir/LDAS
else
	echo -e "GIT-CLONING LDAS FROM $setsource ..."
	rm -f $tempfile".error"
	git clone $setsource $tempdir/LDAS 2>&1|tee $tempfile".error"
	z=$(grep fatal: $tempfile".error")
	if [ "$z" ] ; then { echo -e $RED"\n--- Error ["$thisprog"]: $z\n"$NC ; rm -f $tempfile".error" ;  exit ; } fi
fi
# make the directory for the compiled output
mkdir -p $tempdir/LDAS/bin

echo -e "\n--------------------------------------------------------------------------------"
echo -e "MAKING SCRIPTS EXECUTABLE..."
chmod a+x $tempdir/LDAS/xs-*
chmod a+x $tempdir/LDAS/xp-*

################################################################################
# REMOVE PREVIOUS INSTALL
################################################################################
if [ "$prevdest" != "" ] ; then

	echo -e "\n--------------------------------------------------------------------------------"
	echo -e "REMOVING PREVIOUS INSTALL..."

	# for full install, remove both the repo and the $PATH and nano configurations
	if [ "$setupdate" == "0" ] ; then
		grep -v "LDAS" $prevrc 2>/dev/null > $tempfile.rc
		grep -v "LDAS" $prevnano 2>/dev/null > $tempfile.nano
		if [ "$prevscope" == "local" ]; then
			echo -e "\t...cleaning $prevrc"
			mv $tempfile.rc $prevrc
			echo -e "\t...cleaning $prevnano"
			mv $tempfile.nano $prevnano
			echo -e "\t...removing $prevdest"
			rm -rf $prevdest
		else
			echo -e "\t...cleaning $prevrc"
			sudo mv $tempfile.rc $prevrc
			echo -e "\t...cleaning $prevnano"
			sudo mv $tempfile.nano $prevnano
			echo -e "\t...removing $prevdest"
			sudo rm -rf $prevdest
		fi
	# for local install, just remove the repo
	elif [ "$setupdate" == "1" ] ; then
		if [ "$prevscope" == "local" ]; then
			echo -e "\t...removing $prevdest"
			rm -rf $prevdest
		else
			echo -e "\t...removing $prevdest"
			sudo rm -rf $prevdest
		fi
	fi
fi

################################################################################
# PROCEED WITH INSTALL
################################################################################

echo -e "\n--------------------------------------------------------------------------------"
echo -e "MOVING LDAS TO DESTINATION FOLDER $setdest ..."
if [ $setscope == "local" ] ; then
	if [ -d $setdest/LDAS ] ; then rm -rf $setdest/LDAS ; fi
	mv $tempdir/LDAS $setdest/
else
	if [ -d $setdest/LDAS ] ; then sudo rm -rf $setdest/LDAS ; fi
	sudo mv $tempdir/LDAS $setdest/
fi


if [ "$setupdate" == "0" ] ; then
	echo "--------------------------------------------------------------------------------"
	echo -e "UPDATING \$PATH VARIABLE ($setrc)..."
	# build the template path-update file
	echo -e "# LDAS path updates $bar" > $tempfile".path"
	echo -e "PATH=\$PATH:$setdest/LDAS" >> $tempfile".path"
	echo -e "PATH=\$PATH:$setdest/LDAS/bin" >> $tempfile".path"
	# update the appropriate file with the path
	z=$(grep -s '$PATH:'$setdest/LDAS $setrc | head -n 1)
	if [ "$z" == "" ] ; then
# ??? should add test here for whether LDAS path is defined somewhere else!
		if [ $setscope == "local" ] ; then
			cat $tempfile.path >> $setrc
		else
			sudo sh -c "cat $tempfile.path >> $setrc"
		fi
	fi

	# update $PATH by sourceing rc-file, regardless of whether $PATH already referred to LDAS
	# this helps prevent errors due to a previous failed install that wasn't followed by a re-login
	source $setrc

	echo "--------------------------------------------------------------------------------"
	echo -e "CONFIGURING NANO SYNTAX-HIGHLIGHTING FOR MARKDOWN FILES ($setnano)..."
	z=$(grep -s LDAS /etc/nanorc | head -n 1)
	if [ "$z" == "" ] ; then
		y=$setdest"/LDAS/docs/templates"
		echo -e "## LDAS markdown definitions for nano: to append to ~/.nanorc or /etc/nanorc" > $tempfile
		echo -e "include $y/nano_md.nanorc" >> $tempfile
		echo -e "include $y/nano_ldas.nanorc">> $tempfile
		if [ $setscope == "local" ] ; then
			cat $tempfile >> $setnano
		else
			sudo sh -c "cat $tempfile >> $setnano"
		fi
	# ??? should add test here for whether LDAS is defined in the other (LOCAL or GLOBAL) location!
	fi
fi


echo -e "\n--------------------------------------------------------------------------------"
inpath=$setdest"/LDAS/source"
outpath=$setdest"/LDAS//bin"
echo -e "COMPILING C SOURCE-CODE ($inpath)..."
if [ ! -d "$inpath" ] ; then { echo -e "$RED\n--- Error ["$thisprog"]: missing directory $inpath\n\t- consider re-installing LDAS$NC\n" ;  exit; } ; fi
if [ ! -d "$outpath" ] ; then sudo mkdir -p $outpath ; fi
rm -f $outpath/xe-*
cd $inpath
xs-progcompile "xe-*.c" --warn 0

inpath=$setdest"/LDAS/source/regaamc"
cd $inpath
gcc regaamc8.c xnsubs.c -o ../../bin/regaamc8 -lm -lX11 -L /usr/X11R6/lib -w



echo -e "\n--------------------------------------------------------------------------------"
echo -e "UPDATING TAGS-SUMMARY FILE..."
xs-progtag html | awk '{print "\t"$0}'

echo -e "\n--------------------------------------------------------------------------------"
echo -e "BUILDING HTML VERSIONS OF MANUALS..."
if [ "$(command -v pandoc)" != "" ] ; then
	list=$(xs-manual | xe-cut2 stdin available manuals: -s4 1 | tail -n +2 | xe-delimit stdin)
	for i in $list ; do xs-manual $i --make html 2>/dev/null | awk '{print "\t"$0}' ; done
else
	echo -e "\n--- Warning ["$thisprog"]: pandoc is not installed on this machine: cannot create HTML versions of manuals\n"
fi

################################################################################
# REPORT, CLEANUP AND EXIT
################################################################################
end_time=$(date +'%s.%3N')
s=$(echo $end_time $start_time | awk '{print $1-$2}' )
m=$(echo $s | awk '{print ($1/60)}')
echo -e "\n--------------------------------------------------------------------------------"
echo -e "FINISHED!"
echo -e "\t- Time to finish job: "$s" seconds = "$m" minutes\n"
echo -e $GREEN"\t- please log out and back in to complete the process\n\n"$NC

if [ "$setclean" == "1" ] ; then
	if [ "$tempfile" != "" ] ; then rm -f $tempfile"_"* ; fi
fi
exit
