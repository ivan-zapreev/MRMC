/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: macro.h,v $
*	$Revision: 1.19 $
*	$Date: 2007/10/12 14:34:31 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Ivan Zapreev
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
*	Source description: This file contains main macro definitions.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#ifdef __APPLE__
	//Smth specific for Apple
#else
	#include <malloc.h>
#endif

#ifndef MACRO_H

#define MACRO_H
	
	/**************************************/
	/*******THE SAFETY-IF ASSERTION********/
	/**************************************/
	//WARNING: SHOULD BE USED ONLY FOR THE PERFORMANCE-CRYTICAL PLACES
	//WARNING: MUST NOT BE USED TO ASSERT ON BOOLEAN EXPRESSIONS WITH
	//         SIDEEFFECTS! BECAUSE IN CASE SAFETY_FIRST IS UNDEFINED
	//         THE BOOLEAN EXPRESSION WILL NOT BE EVALUATED AT ALL!!!
	
	//Should be defined for the development phase and testing phase.
	#define SAFETY_FIRST
	
	#ifdef SAFETY_FIRST
		#define IF_SAFETY( condition ) if( condition ){
		#define ELSE_SAFETY } else {
	#else
		#define IF_SAFETY( condition )
		#define ELSE_SAFETY if( FALSE ){
	#endif
	#define ENDIF_SAFETY }
	
	/**************************************/
	/*******THE SIMPLE BOOLEAN TYPE********/
	/**************************************/
	#ifndef BOOL
		#define BOOL short
	#endif
	
	#ifndef TRUE
		#define TRUE 1
	#endif
	
	#ifndef FALSE
		#define FALSE 0
	#endif
	
	/**************************************/
	/******THE THREE-VALUED LOGIC TYPE*****/
	/**************************************/
	//For the three valued logic
	#ifndef TV_LOGIC
		#define TV_LOGIC short int
	#endif
	
	//The YES (TRUE) answer ">0"
	#ifndef TVL_TT
		#define TVL_TT TRUE
	#endif
	
	//The NO (FALSE) answer "=0"
	#ifndef TVL_FF
		#define TVL_FF FALSE
	#endif
	
	//The I do not know answer "<0"
	#ifndef TVL_NN
		#define TVL_NN -1
	#endif
	
	/**************************************/
	/******THE NULL POINTER DEFINITION*****/
	/**************************************/
	#ifndef NULL
		#define NULL 0
	#endif
	
	/**************************************/
	/******THE MIN OPERATOR DEFINITION*****/
	/**************************************/
	#ifndef MIN
		#define MIN(a,b) (a<b?a:b)
	#endif
	
	/**************************************/
	/******THE BINARY-FLAGS MANAGEMENT*****/
	/**************************************/
	//This macro definitions are used for working with bit flags
	#define set_flag(flags,bit) (flags = flags | bit)
	#define clear_flag(flags,bit) (flags = flags & ~bit)
	#define test_flag(flags,bit) (flags & bit)
	
#endif

