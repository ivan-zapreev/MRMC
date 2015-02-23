/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: read_lab_file.h,v $
*	$Revision: 1.1 $
*	$Date: 2007/10/01 12:39:51 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Maneesh Khattri, Ivan Zapreev
*
*	This program is free software; you can redistribute it and/or
*	modify it under the terms of the GNU General Public License
*	as published by the Free Software Foundation; either version 2
*	of the License, or (at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program; if not, write to the Free Software
*	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*	Our contacts:
*		Formal Methods and Tools Group, University of Twente,
*		P.O. Box 217, 7500 AE Enschede, The Netherlands,
*		Phone: +31 53 4893767, Fax: +31 53 4893247,
*		E-mail: mrmc@cs.utwente.nl
*
*	Source description: Read Label (.lab) file.
*	Uses: DEF: bitset.h, label.h, token.h
*		LIB: bitset.c, label.c, token.c
*		Definition of read_lab_file - read_lab_file.h
*/

/*****************************************************************************
name		: read_lab_file
role       	: reads a .lab file. puts the result in a labelling structure (label.h).
@param     	: int ns: the number of states.
@param     	: char *filename: input .lab file's name.
@return		: labelling *: returns a pointer to a labelling function.
remark 	        : 
******************************************************************************/
extern labelling * read_lab_file(int, char *);
