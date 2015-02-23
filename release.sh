#!/bin/sh

#
#	WARNING: Do Not Remove This Section
#
#	$RCSfile: release.sh,v $
#	$Revision: 1.8 $
#	$Date: 2007/08/21 10:16:53 $
#	$Author: zapreevis $
#	
#	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
#	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
#	automated verification of properties concerning long-run and instantaneous rewards
#	as well as cumulative rewards.
#	
#	Copyright (C) The University of Twente, 2004-2006.
#	Authors: Ivan Zapreev
#
#	This program is free software; you can redistribute it and/or
#	modify it under the terms of the GNU General Public License
#	as published by the Free Software Foundation; either version 2
#	of the License, or (at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
#	Our contacts:
#		Formal Methods and Tools Group, University of Twente,
#		P.O. Box 217, 7500 AE Enschede, The Netherlands,
#		Phone: +31 53 4893767, Fax: +31 53 4893247,
#		E-mail: mrmc@cs.utwente.nl
#
#	Source description: Is used for automated distribution
#
#	@param ${1}	'mrmc' or 'test'
#	@param ${2}	version, like '1.3' or '2.1b'
#

PWD=`eval pwd`
HOME_FOLDER=${PWD}
WORKDIR=/tmp

#Help message
#The number of script arguments
MRMC_NUM_OF_ARGUMENTS="3"
TESTS_NUM_OF_ARGUMENTS="2"

#What is distributed
TARGET_TYPE_MRMC="mrmc"
TARGET_TYPE_TESTS="test"

#In what format it is distributed
TARGET_TYPE_DISTRIB_SRC="src"
TARGET_TYPE_DISTRIB_WIN="win"
TARGET_TYPE_DISTRIB_LIN="lin"
TARGET_TYPE_DISTRIB_MAC="mac"

usage(){
	echo -e "USAGE: ${1} #1 #2 #3"
	echo "This is an automated distribution script, where:"
	echo -e "\t#1 - the distribution target, one of: {${TARGET_TYPE_MRMC}, ${TARGET_TYPE_TESTS}}"
	echo -e "\t#2 - a version, can be something like '1.3' or '2.1b'."
	echo -e "\t#3 - the distribution kind, one of: {${TARGET_TYPE_DISTRIB_SRC}, ${TARGET_TYPE_DISTRIB_WIN}, ${TARGET_TYPE_DISTRIB_LIN}, ${TARGET_TYPE_DISTRIB_MAC}}"
	echo "NOTE: The parameter #3 has to be provided only when #1=${TARGET_TYPE_MRMC}"
	exit 1
}

#Get the script arguments
SCRIPT_NAME=${0}
TARGET_TYPE=${1}
TARGET_VERSION=${2}
TARGET_DISTRIB_KIND=${3}

echo -n "INFO: Verifying the script parameters .... "

#Check for the proper number of arguments
#Define the release-files file name and the tmp folder name
if [ "${TARGET_TYPE}" = "${TARGET_TYPE_TESTS}" -a $# -eq ${TESTS_NUM_OF_ARGUMENTS} ]; then
	RELEASE_FILES=release.${TARGET_TYPE}.files
	TARGET_FOLDER_NAME="${TARGET_TYPE_MRMC}_${TARGET_TYPE}_v${TARGET_VERSION}"
elif [ "${TARGET_TYPE}" = "${TARGET_TYPE_MRMC}" -a $# -eq ${MRMC_NUM_OF_ARGUMENTS} ]; then
	TARGET_FOLDER_NAME="${TARGET_TYPE}_${TARGET_DISTRIB_KIND}_v${TARGET_VERSION}"
	#Check the distribution type
	if [ "${TARGET_DISTRIB_KIND}" = "${TARGET_TYPE_DISTRIB_SRC}" ]; then
		RELEASE_FILES=release.${TARGET_TYPE}.src.files
	elif [ "${TARGET_DISTRIB_KIND}" = "${TARGET_TYPE_DISTRIB_LIN}" -o "${TARGET_DISTRIB_KIND}" = "${TARGET_TYPE_DISTRIB_WIN}" -o "${TARGET_DISTRIB_KIND}" = "${TARGET_TYPE_DISTRIB_MAC}" ]; then
		RELEASE_FILES=release.${TARGET_TYPE}.bin.files
	else
		echo "FALIED"
		echo "ERROR: Unknown distribution kind: '${TARGET_DISTRIB_KIND}'"
		usage ${SCRIPT_NAME};
	fi
else
	echo "FALIED"
	echo "ERROR: Incorrect script arguments."
	usage ${SCRIPT_NAME};
fi

#Check the distribution target
if [ "${TARGET_TYPE}" != "${TARGET_TYPE_MRMC}" -a "${TARGET_TYPE}" != "${TARGET_TYPE_TESTS}" ]; then
	echo "FALIED"
	echo "ERROR: Unknown distribution target: '${TARGET_TYPE}'"
	usage ${SCRIPT_NAME};
fi

echo "DONE"

#Compile MRMC if needed
if [ "${TARGET_TYPE}" != "${TARGET_TYPE_TESTS}" -a "${TARGET_DISTRIB_KIND}" != "${TARGET_TYPE_DISTRIB_SRC}" ]; then
	echo -n "INFO: Rebuilding MRMC binary .... "
	make clean 2>&1 1>/dev/null
	make all 2>&1 1>/dev/null
	if ! test -f ${HOME_FOLDER}/bin/${TARGET_TYPE}*; then
		echo "FALIED"
		echo "ERROR: Could not build the MRMC binary"
		exit 1
	fi
	echo "DONE"
fi

#The files and folders that have to be excluded for this distribution target
EXCLUDE_FILE_LIST="exclude.${TARGET_TYPE}.lst"
#Choose the zip file name
TARGET_ZIP_FILE="${TARGET_FOLDER_NAME}.zip"

echo -n "INFO: Checking the release data files .... "
if test -f ${RELEASE_FILES}; then
	if test -f ${RELEASE_FILES}; then
		echo "DONE"
		TARGET_FOLDER="${WORKDIR}/${TARGET_FOLDER_NAME}"
		#Create a work folder
		rm -rf ${TARGET_FOLDER}
		mkdir ${TARGET_FOLDER}
		
		echo -n "INFO: Copying the files .... "
		#Copy the release files to the tmp directory
		for FD_NAME in `eval cat ${RELEASE_FILES}`
		do
			cp -R ${FD_NAME} ${TARGET_FOLDER}
		done
		echo "DONE"
		
		echo -n "INFO: Creating the release archive .... "
		cd ${WORKDIR}
		zip -r -o -9 -u -q ${TARGET_ZIP_FILE} ${TARGET_FOLDER_NAME} -x@${HOME_FOLDER}/${EXCLUDE_FILE_LIST}
		mv ${TARGET_ZIP_FILE} ${HOME_FOLDER}
		cd ${HOME_FOLDER}
		
		rm -rf ${TARGET_FOLDER}
		echo "DONE"
	else
		echo "FAILED"
		echo "ERROR: The required release file '${RELEASE_FILES}' does not exist."
	fi
else
	echo "FAILED"
	echo "ERROR: The required release file '${RELEASE_FILES}' does not exist."
fi
