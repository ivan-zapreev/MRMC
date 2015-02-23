/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: transient.h,v $
*	$Revision: 1.1 $
*	$Date: 2007/10/01 12:39:55 $
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
*	Source description: Perform Transient Analysis for PCTL/CSL/PRCTL/CSRL - X, U.
*/

#include "macro.h"

#include <math.h>
#include <sys/types.h>

#include "bitset.h"
#include "sparse.h"
#include "runtime.h"

#include "transient_dtmc.h"
#include "transient_dtmrm.h"
#include "transient_ctmc.h"
#include "transient_ctmrm.h"

#ifndef TRANSIENT_H

#define TRANSIENT_H

//These macro definitions are helping us to invoke the right model checking method.
#define TIME_UNBOUNDED_FORM 1
#define TIME_INTERVAL_FORM 2

/**
* Solve next_rewards formula.
* @param phi satisfaction relation for phi formula.
* @param subi sub I (time).
* @param supi sup I (time).
* @param subj sub J (reward).
* @param supj sup J (reward).
* @return result of the next formula for all states.
* NOTE: ********without impulse rewards at present***********
* 		  ********to be tested***********
*/
double * next_rewards(bitset *phi, double subi, double supi, double subj, double supj);

/**
* Solve until formula with rewards.
* @param phi satisfaction relation for phi formula.
* @param psi satisfaction relation for psi formula.
* @param subi sub I (time).
* @param supi sup I (time).
* @param subj sub J (reward).
* @param supj sup J (reward).
* @param isIgnoreFLumping if we want the formula dependent lumping to be ignored
* @return result of the until formula for all states.
*/
double * until_rewards(bitset *phi, bitset *psi, double subi, double supi, double subj, double supj, BOOL isIgnoreFLumping);

/**
* Solve next formula.
* @param type one of: TIME_UNBOUNDED_FORM, TIME_INTERVAL_FORM
* @param phi satisfaction relation for phi formula.
* @param subi sub I (time).
* @param supi sup I (time).
* @return result of the next formula for all states.
*/
double * next(const int type, bitset *phi, double subi, double supi);

/**
* Solve until formula.
* @param type one of: TIME_UNBOUNDED_FORM, TIME_INTERVAL_FORM
* @param phi satisfaction relation for phi formula.
* @param psi satisfaction relation for psi formula.
* @param subi sub I (time).
* @param supi sup I (time).
* @param isIgnoreFLumping if we want the formula dependent lumping to be ignored
* @return result of the until formula for all states.
*/
double * until(const int type, bitset *phi, bitset *psi, double subi, double supi, BOOL isIgnoreFLumping);

#endif
