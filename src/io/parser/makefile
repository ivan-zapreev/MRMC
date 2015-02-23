#
#	WARNING: Do Not Remove This Section
#
#	$RCSfile: makefile,v $
#	$Revision: 1.8 $
#	$Date: 2007/10/24 13:39:49 $
#	$Author: zapreevis $
#	
#	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
#	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
#	automated verification of properties concerning long-run and instantaneous rewards
#	as well as cumulative rewards.
#	
#	Copyright (C) The University of Twente, 2004-2007.
#	Authors: Maneesh Khattri
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
#	Source description: make file for parser
#

include $(MRMC_HOME_DIR)/makefile.def

LEXSRC		= mrmc_tokenizer.l
YACCSRC		= mrmc_grammar.y
YACCH		= mrmc_grammar.tab.h
SRCS_PARSER	= mrmc_grammar.tab.c lex.yy.c
SRCS_P2CORE	= parser_to_tree.c core_to_core.c parser_to_core.c
SRCS		= $(SRCS_PARSER) $(SRCS_P2CORE)
OBJ		= $(SRCS:.c=.o)

all: 	yacc move lex $(OBJ)

clean:
	$(RM) -f $(SRCS_PARSER) $(OBJ) $(EXECS) *~

wipe: 
	$(RM) -f $(SRCS_PARSER) $(OBJ) $(EXECS) out *.out

yacc:
	$(YACC) -d $(YACCSRC)

lex:	
	$(LEX) $(LEXSRC)

move: 
	$(MV) $(YACCH) $(INCLUDE_DIR)

mrmc_grammar.tab.o : mrmc_grammar.tab.c
lex.yy.o : lex.yy.c
