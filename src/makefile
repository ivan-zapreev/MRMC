#
#	WARNING: Do Not Remove This Section
#
#	$RCSfile: makefile,v $
#	$Revision: 1.31 $
#	$Date: 2007/10/24 13:39:48 $
#	$Author: zapreevis $
#	
#	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
#	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
#	automated verification of properties concerning long-run and instantaneous rewards
#	as well as cumulative rewards.
#	
#	Copyright (C) The University of Twente, 2004-2006.
#	Authors: Maneesh Khattri, Ivan Zapreev
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
#	Source description: Make file for src directory of the MRMC tool
#

include $(MRMC_HOME_DIR)/makefile.def

SRC = runtime.c $(STORAGE_SRC_DIR)/bitset.c $(STORAGE_SRC_DIR)/sample_vec.c $(STORAGE_SRC_DIR)/kjstorage.c $(STORAGE_SRC_DIR)/label.c $(STORAGE_SRC_DIR)/path_graph.c $(STORAGE_SRC_DIR)/sparse.c $(STORAGE_SRC_DIR)/stack.c $(IO_SRC_DIR)/read_tra_file.c $(IO_SRC_DIR)/read_lab_file.c $(IO_SRC_DIR)/read_rewards.c $(IO_SRC_DIR)/read_impulse_rewards.c $(IO_SRC_DIR)/token.c $(ALGORITHMS_SRC_DIR)/bscc.c $(ALGORITHMS_SRC_DIR)/foxglynn.c $(ALGORITHMS_SRC_DIR)/iterative_solvers.c $(STORAGE_SRC_DIR)/splay.c $(LUMPING_SRC_DIR)/lump.c $(LUMPING_SRC_DIR)/partition.c $(MODELCHECKING_SRC_DIR)/prctl.c $(MODELCHECKING_SRC_DIR)/steady.c $(MODELCHECKING_SRC_DIR)/transient.c $(MODELCHECKING_SRC_DIR)/transient_ctmc.c $(MODELCHECKING_SRC_DIR)/transient_dtmc.c $(MODELCHECKING_SRC_DIR)/transient_common.c $(MODELCHECKING_SRC_DIR)/transient_ctmrm.c $(MODELCHECKING_SRC_DIR)/transient_dtmrm.c $(RANDOM_NUMBERS_SRC_DIR)/rng_gsl.c $(RANDOM_NUMBERS_SRC_DIR)/rng_ymer.c $(RANDOM_NUMBERS_SRC_DIR)/rng_ciardo.c $(RANDOM_NUMBERS_SRC_DIR)/rng_prism.c $(RANDOM_NUMBERS_SRC_DIR)/rng_app_crypt.c $(RANDOM_NUMBERS_SRC_DIR)/rand_num_generator.c $(MODELCHECKING_SRC_DIR)/simulation.c $(MODELCHECKING_SRC_DIR)/simulation_common.c $(MODELCHECKING_SRC_DIR)/simulation_utils.c $(MODELCHECKING_SRC_DIR)/simulation_ctmc.c 

PARSER_OBJ=$(PARSER_SRC_DIR)/parser_to_core.o $(PARSER_SRC_DIR)/core_to_core.o $(PARSER_SRC_DIR)/parser_to_tree.o $(PARSER_SRC_DIR)/mrmc_grammar.tab.o $(PARSER_SRC_DIR)/lex.yy.o

MAIN_SRC  = mcc.c

OBJ = $(SRC:.c=.o)

all: parser lib ${EXEC}

lib: $(LIB_A)

$(LIB_A): $(OBJ) $(PARSER_OBJ)
	 @ar crv $(LIB_A) $?
	 @case x$(HASRANLIB) in xt) echo ranlib; ranlib $(LIB_A);; esac 

parser:
	cd $(PARSER_SRC_DIR); make all;

${EXEC} : ${SRC}
	$(CC) $(FLAGS) $(INCLUDES) -o $(MRMC_BIN_DIR)/${EXEC} ${MAIN_SRC} $(LDFLAGS)

clean:
	cd $(PARSER_SRC_DIR); make clean;
	$(RM) -f $(MRMC_HOME_DIR)/include/*~ */*~ *~ $(OBJ)
