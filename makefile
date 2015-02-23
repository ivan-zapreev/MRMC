#
#	WARNING: Do Not Remove This Section
#
#	$RCSfile: makefile,v $
#	$Revision: 1.8 $
#	$Date: 2007/11/12 12:37:53 $
#	$Author: zapreevis $
#	
#	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
#	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
#	automated verification of properties concerning long-run and instantaneous rewards
#	as well as cumulative rewards.
#	
#	Copyright (C) The University of Twente, 2004-2006.
#	Authors: Katoen, Maneesh Khattri, Ivan Zapreev
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
#	Source description: The main Markov Reward Model Checker m a k e f i l e.
#

include makefile.def

export MRMC_HOME_DIR =$(PWD)
export MRMC_BIN_DIR = $(MRMC_HOME_DIR)/bin
export TILDA_TMP_FILES = *~

error:
	@echo "+-----------------------------------------------------------------+"
	@echo "|                                                                 |"
	@echo "|          Install Markov Reward Model Checker (MRMC)             |"	
	@echo "|                                                                 |"
	@echo "|       Usage: make all    - installs MRMC                        |"
	@echo "|              make clean  - cleans *.o and other executables     |"
	@echo "|                                                                 |"
	@echo "|  Make sure the system-specific makefile.def has been edited     |"
	@echo "|  to reflect your system configuration.                          |"
	@echo "|                                                                 |"
	@echo "+-----------------------------------------------------------------+"

bin:
	mkdir bin;

lib:
	mkdir lib;

src: bin lib
	cd $(MRMC_HOME_DIR)/src; make all;

all: src
	@echo "+-----------------------------------------------------------------+"
	@echo "|              Markov Reward Model Checker installed.             |"
	@echo "+-----------------------------------------------------------------+"

clean:
	cd $(MRMC_HOME_DIR)/src; make clean;
	$(RM) -rf $(MRMC_HOME_DIR)/bin $(MRMC_HOME_DIR)/lib $(TILDA_TMP_FILES);
	$(RM) -f $(INCLUDE_DIR)/$(TILDA_TMP_FILES) $(INCLUDE_DIR)/$(ALGORITHMS_SRC_DIR)/$(TILDA_TMP_FILES) $(INCLUDE_DIR)/$(LUMPING_SRC_DIR)/$(TILDA_TMP_FILES) $(INCLUDE_DIR)/$(MODELCHECKING_SRC_DIR)/$(TILDA_TMP_FILES) $(INCLUDE_DIR)/$(STORAGE_SRC_DIR)/$(TILDA_TMP_FILES) $(INCLUDE_DIR)/$(IO_SRC_DIR)/$(TILDA_TMP_FILES) $(INCLUDE_DIR)/$(PARSER_SRC_DIR)/$(TILDA_TMP_FILES) $(INCLUDE_DIR)/$(RANDOM_NUMBERS_SRC_DIR)/$(TILDA_TMP_FILES)

done:
