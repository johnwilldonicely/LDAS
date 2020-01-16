#!/bin/bash

# PANDOC MAY ALSO REQUIRE LATEX DEPENDENCIES? - SEE IF THERE IS A NO-LATED MD-to-PDF CONVERSION

# <TAGS>programming ldas</TAGS>
thisprog=`basename "$0"`
tempfile="temp_"$thisprog
tempfolder="/home/$USER/temp_LDAS_install"

# colours for use with optional error messages
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color
bar="############################################################"

################################################################################
# INTIAL VARIABLE DEFINITIONS
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
setscope="local"
setdest="/home/$USER/bin/"
setrc="/home/$USER/.bashrc"
filezip="LDAS-master.zip"
setfilezip=""
setverb="1"
setclean="1"

################################################################################
# DEFINE PERMISSION FUNCTION: CALL FORMAT: status=$(func_permission $path yes|no)
################################################################################
function func_permission () {
	# READ THE ARGUMENTS
	zpath=$1 # the path to check
	zerror=$2 # "yes" if fail triggers error message + exit
	# DETERMINE BASIC INFO
	zaccess="no"
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
	# HANDLE ERROR MESSAGES IF REQUIRED
	if [ "$zaccess" == "no" ] && [ "$zerror" == "yes" ] ; then
		echo -e "$RED--- Warning ["$thisprog"]: $USER has no write-permission for $zpath"
		echo -e "\tConsider using \"--scope local\" to install to the user's bin folder"
		echo -e "\t... or, run this script as superuser"
		echo -e "\t... or, use sudo to temporarily give write access"
		echo -e "$NC"
		exit
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
	echo $thisprog": LDAS installation script"
	echo "	- full install from GitHub (default) or a .zip archive"
	echo "	- checks operating system & permissions"
	echo "	- checks dependencies"
	echo "	- compiles the C source-code"
	echo "	- updates the \$PATH variable"
	echo "	- updates manuals and PROGTAG.html"
	echo ""
	echo "USAGE: $thisprog [scope] [options]"
	echo "	[scope]: local (current user) or global (all users)"
	echo ""
	echo "VALID OPTIONS (defaults in []):"
	echo "	--zip: install LDAS from this zip-file [$setfilezip]"
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
setscope=$1 ; shift
if [ "$setscope" != "local" ] && [ "$setscope" != "global" ] ; then
	echo -e "$RED\n--- Error ["$thisprog"]: invalid --scope ($setscope) -  must be \"local\" or \"global\"\n$NC" ;  exit;
fi

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

# DETERMINE DESTINATION AND PATH-UPDATE OPTIONS, DEPENDING ON SELECTED SCOPE
# local= current user only, global= all users
if [ "$setscope" == "local" ] ; then
	setdest="/home/$USER/bin"
	setrc="/home/$USER/.bashrc"
	mkdir -p $setdest # ??? check for errors?
elif [ "$setscope" == "global" ] ; then
	setdest="/opt"
	setrc="/etc/profile"
	# check permissions
#???	func_permission $setdest "yes"
#???	func_permission $setrc "yes"
fi


# DETERMINE LINUX VERSION AND REPORT
distro=$(lsb_release -a | grep "Distributor ID:" | cut -f 2)
release=$(lsb_release -a | grep "Release:" | cut -f 2)
echo -e "- Linux distro: $distro $release"


# CHECK IF LDAS IS ALREADY INSTALLED
echo "- requested install path: $setdest/LDAS"
prevpath=$(which xs-template 2>/dev/null | rev | cut -f 2- -d / |rev)
if [ "$prevpath" != "" ] ; then

	echo "- previous install path: $prevpath"
	# determine previous-install scope - make sure path is valid
	if [[ $prevpath =~ "/opt/LDAS" ]] ; then prevscope="global" ; prevrc="/etc/profile"
	elif [[ $prevpath =~ "/home/$USER/bin/LDAS" ]] ; then prevscope="local" ; prevrc="/home/$USER/.bashrc"
	else echo -e "\n--- Error ["$thisprog"]: LDAS previously installed in invalid location ($prevpath) - remove and retry\n" ; exit
	fi

	echo -e "$GREEN"
	read -p  "--- WARNING: this will first remove previous installation - proceed? [y/n] " answer
	echo -en "$NC"
	while true ; do case $answer in [yY]* ) break ;; *) echo ; exit ;; esac ; done
	echo -en "$GREEN"
	backup="/home/$USER/LDAS_backup_"$(date +'%Y%m%d')".zip"
	read -p  "--- BACKUP $prevpath to $backup? [y/n] " answer
	echo -en "$NC"
	while true ; do case $answer in [yY]* ) echo -e "$NC\t...backing up..." ; zip -qr $backup $prevpath ; break ;; *) break ;; esac ; done

	grep -v "LDAS" $prevrc 2>/dev/null > $tempfile.rc
	grep -v "LDAS" /etc/nanorc 2>/dev/null > $tempfile.nanorc

	if [ "$prevscope" == "local" ]; then
		echo -e "\t...cleaning $prevrc"
		mv $tempfile.rc $prevrc
		echo -e "\t...cleaning /etc/nanorc"
		mv $tempfile.nanorc /etc/nanorc
		echo -e "\t...removing $prevpath"
		rm -rf $prevpath
	else
		echo -e "\t...cleaning $prevrc"
		sudo mv $tempfile.rc $prevrc
		echo -e "\t...cleaning /etc/nanorc"
		sudo mv $tempfile.nanorc /etc/nanorc
		echo -e "\t...removing $prevpath"
		sudo rm -rf $prevpath
	fi

fi

# BUILD THE TEMPLATE PATH-UPDATE FILE
echo -e "# LDAS path updates $bar" > $tempfile".path"
echo -e "PATH=\$PATH:$setdest/LDAS" >> $tempfile".path"
echo -e "PATH=\$PATH:$setdest/LDAS/bin" >> $tempfile".path"


########################################################################################
# INSTALL
########################################################################################
# CHECK IF DEPENDENCIES ARE INSTALLED
echo "--------------------------------------------------------------------------------"
echo -e "CHECKING DEPENDENCIES..."
dep="git" ; if [ "$(command -v $dep)" == "" ] ; then
	echo -e "\t--- Warning:$GREEN$dep$NC not installed- install or updates might fail"
	echo -e "\t\t - attempting to install $dep as sudo..."
	sudo yum install $dep -y
fi
dep="gs" ; if [ "$(command -v $dep)" == "" ] ; then
	echo -e "\t--- Warning:$GREEN$dep$NC not installed- graphics handling might fail"
	echo -e "\t\t - attempting to install $dep as sudo..."
	sudo yum install $dep -y
fi
de="dos2unix"; if [ "$(command -v $dep)" == "" ] ; then
	echo -e "\t--- Warning:$GREEN$dep$NC not installed- some scripts might fail"
	echo -e "\t\t - attempting to install $dep as sudo..."
	sudo yum install $dep -y
fi
dep="nano" ; if [ "$(command -v $dep)" == "" ] ; then
	echo -e "\t--- Warning:$GREEN$dep$NC not installed- manuals might not be viewable"
	echo -e "\t\t - attempting to install $dep as sudo..."
	sudo yum install $dep -y
fi
dep="pandoc" ; if [ "$(command -v $dep)" == "" ] ; then
	echo -e "\t--- Warning: $GREEN$dep$NC not installed- some manuals might not be rendered"
	echo -e "\t\t - attempting to install $dep as sudo..."
	sudo yum install $dep -y
fi

# RETRIEVE OR EXTRACT LDAS TO TEMP FOLDER
if [ "$setclean" == "1" ] && [ "$tempfolder" != "" ] ; then rm -rf $tempfolder ; fi
mkdir -p $tempfolder

echo "--------------------------------------------------------------------------------"
if [ "$setfilezip" != "" ] ; then
	echo -e "EXTRACTING LDAS FROM $setfilezip ..."
	if [ ! -e "$setzipfile" ] ; then { echo -e "\n--- Error ["$thisprog"]: missing zip-file $setzipfile\n" ;  exit; } ; fi

	unzip -oq $setfilezip -d $tempfolder
	# find the most recent LDAS folder created
	z=$(ls -dt1 $tempfolder/*LDAS*/ | head -n 1 | awk -F / '{print $(NF-1)}' )
	# rename to LDAS
	mv $tempfolder/$z $tempfolder/LDAS
else
	echo -e "GIT-CLONING LDAS FROM $setsource ..."
	rm -f $tempfile".error"
	git clone $setsource $tempfolder/LDAS 2>&1 | tee $tempfile".error"
	z=$(grep fatal: $tempfile".error")
	if [ "$z" ] ; then { echo -e "$RED--- Error ["$thisprog"]: $z\n"$NC ; rm -f $tempfile".error" ;  exit ; } fi
fi
# make the directory for the compiled output
mkdir -p $tempfolder/LDAS/bin


echo "--------------------------------------------------------------------------------"
echo -e "MOVING LDAS TO DESTINATION FOLDER $setdest ..."
if [ $setscope == "local" ] ; then
	if [ -d $setdest/LDAS ] ; then rm -rf $setdest/LDAS ; fi
	mv $tempfolder/LDAS $setdest/
else
	if [ -d $setdest/LDAS ] ; then sudo rm -rf $setdest/LDAS ; fi
	sudo mv $tempfolder/LDAS $setdest/
fi

echo "--------------------------------------------------------------------------------"
echo -e "MAKING SCRIPTS EXECUTABLE..."
chmod a+x $setdest/LDAS/xs-*
chmod a+x $setdest/LDAS/xp-*
chmod a+x $setdest/LDAS/xr-*

echo "--------------------------------------------------------------------------------"
echo -e "UPDATING \$PATH VARIABLE ($setrc)..."
z=$(grep -s '$PATH:'$setdest/LDAS $setrc | head -n 1)
if [ "$z" == "" ] ; then
	if [ $setscope == "local" ] ; then
		cat $tempfile.path >> $setrc
	else
		sudo sh -c "cat $tempfile.path >> $setrc"
	fi
fi

echo "--------------------------------------------------------------------------------"
echo -e "CONFIGURING NANO SYNTAX-HIGHLIGHTING FOR MARKDOWN FILES..."
z=$(grep -s LDAS /etc/nanorc | head -n 1)
if [ "$z" == "" ] ; then
	template=$setdest"/LDAS/docs/templates/ldas_nanorc.txt"
	sudo sh -c "cat $template >> /etc/nanorc"
fi


echo "--------------------------------------------------------------------------------"
echo -e "COMPILING C-CODE..."
inpath=$setdest"/LDAS/source"
outpath=$setdest"/LDAS//bin"
if [ ! -d "$inpath" ] ; then { echo -e "$RED\n--- Error ["$thisprog"]: missing directory $inpath\n\t- consider re-installing LDAS$NC\n" ;  exit; } ; fi
if [ ! -d "$outpath" ] ; then sudo mkdir -p $outpath ; fi
func_permission $inpath

rm -f $outpath/xe-*
cd $inpath
xs-progcompile "xe-*.c"


echo "--------------------------------------------------------------------------------"
echo "UPDATING TAGS-SUMMARY FILE..."
xs-progtag html | awk '{print "\t"$0}'

echo "--------------------------------------------------------------------------------"
echo "BUILDING HTML VERSIONS OF MANUALS..."
if [ "$(command -v pandoc)" != "" ] ; then
	list=$(xs-manual | xe-cut2 stdin available manuals: -s4 1 | tail -n +2 | xe-delimit stdin)
	for i in $list ; do xs-manual $i --make html 2>/dev/null | awk '{print "\t"$0}' ; done
else
	echo -e "\n--- Warning ["$thisprog"]: pandoc is not installed on this machine: cannot create HTML versions of manuals\n"
fi

################################################################################
# REPORT, CLEANUP AND EXIT
################################################################################
if [ "$setverb" == "1" ] ; then
	end_time=$(date +'%s.%3N')
	s=$(echo $end_time $start_time | awk '{print $1-$2}' )
	m=$(echo $s | awk '{print ($1/60)}')
	echo -e "\n\tTime to finish job: "$s" seconds = "$m" minutes\n"
fi
if [ "$setclean" == "1" ] ; then
	if [ "$tempfile" != "" ] ; then rm -f $tempfile"_"* ; fi
fi
exit
