/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: bitset.h,v $
*	$Revision: 1.6 $
*	$Date: 2007/10/15 14:31:36 $
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
*	Source description: Define a Bitset-For the satisfaction relation in MCC.
*	Uses: Definition of bitset - bitset.h
*/

#include "macro.h"

#include <math.h>
#include "macro.h"

#ifndef BITSET_H

#define BITSET_H

//This is the type of one block in the bitset
//WARNING: We assume that "sizeof(unsigned int) = 4" !!!
#define BITSET_BLOCK_TYPE unsigned int
//This is the size of the bitset block
//WARNING! The definition should be equivalent to having:
//	#define BITSET_BLOCK_SIZE (int) (sizeof(BITSET_BLOCK_TYPE) * 8)
//We do not define it this way because then we get a significant slow down. 
#define BITSET_BLOCK_SIZE 32

/**
*This are the constant bits
*/
static const BITSET_BLOCK_TYPE BIT_ON = 1, BIT_OFF = 0;

/**
* The bitset structure
* @member n			: The number of valid bits in this bitset.
* @member blocks_num	: The number of elements (bytes) int the bytesp array
* @member bytesp		: A pointer to an array of BITSET_BLOCK_TYPE.
*/
typedef struct bitset
{
	int n;
	int blocks_num;
	BITSET_BLOCK_TYPE *bytesp;
} bitset;

/*****************************************************************************
name		: get_new_bitset
role		: get a new bitset.
@param		: int n: indicates the size of the required bitset.
@return	: bitset *: returns a pointer to a new bitset.
remark		: A bitset is composed of an array of BITSET_BLOCK_SIZE-bit unsigned integers.
******************************************************************************/
extern bitset * get_new_bitset(int n);

/**
* This function should extend the bitset. In case:
*	"isFillNewBlocksWithOne == TRUE"
* the new bitset blocks are filled with ones.
* WARNING: We do not fill with one the remainings of the old last block
* we do not extra blocks 
* We assume that the bitset size may only be increased.
* @param pBitset the bitset to increase
* @param new_size the new bitset size, should be: "new_size >= pBitset->n"
* @param isFillNewBlocksWithOne if TRUE then the new bitset blocks will be filled with one.
*/
extern void extend_bitset( bitset * pBitset, const int new_size, BOOL isFillNewBlocksWithOne );

/*****************************************************************************
name		: or
role       	: obtain the disjunction of two bitsets.
@param     	: bitset *a: One of the operands to the disjunction operation.
@param		: bitset *b: The second operand to the disjuction operation.
@return		: bitset *: returns a pointer to the result of disjunction.
remark 	        : size of the operands must be the same else a NULL pointer is
		  returned as the result.
******************************************************************************/
extern bitset * or(const bitset *a, const bitset *b);

/*****************************************************************************
name		: or_result
role       	: obtain the disjunction of two bitsets and put result in arg2.
@param     	: bitset *a: One of the operands to the disjunction operation.
@param		: bitset *b: The second operand to the disjunction operation.
******************************************************************************/
extern void or_result(const bitset *a, bitset *result);

/*****************************************************************************
name		: copy_bitset
role       	: This method just copies elements from one bitset into enother.
		: The two bitsets must have the same size.
@param     	: bitset *from_bitset: the bitset to be copied .
@param		: bitset *to_bitset: the bitset to copy to.
******************************************************************************/
void copy_bitset(const bitset *from_bitset, bitset *to_bitset);

/*****************************************************************************
name		: xor
role       	: obtain the exclusive-or of two bitsets.
@param     	: bitset *a: One of the operands to the exclusive-or operation.
@param		: bitset *b: The second operand to the exclusive-or operation.
@return		: bitset *: returns a pointer to the result of exclusive-or
remark 	        : The size of the operands must be the same else a NULL pointer is returned as the result.
******************************************************************************/
extern bitset * xor(const const bitset * a, const const bitset * b);

/*****************************************************************************
name		: xor_result
role       	: obtain the xor of two bitsets and put result in arg2.
@param     	: bitset *a: One of the operands to the x-or operation.
@param		: bitset *b: The second operand to the x-or operation.
******************************************************************************/
extern void xor_result(const bitset *a, bitset *result);

/*****************************************************************************
name		: and
role       	: obtain the conjunction of two bitsets.
@param     	: bitset *a: One of the operands to the conjunction operation.
@param		: bitset *b: The second operand to the conjunction operation.
@return		: bitset *: returns a pointer to the result of conjunction.
remark 	        : The size of the operands must be the same else a NULL pointer is returned as the result.
******************************************************************************/
extern bitset * and(const bitset * a, const bitset * b);

/*****************************************************************************
name		: and_result
role       	: obtain the conjunction of two bitsets and put result in arg2.
@param     	: bitset *a: One of the operands to the conjunction operation.
@param		: bitset *b: The second operand to the conjunction operation.
******************************************************************************/
extern void and_result(const bitset *a, bitset *result);

/*****************************************************************************
name		: not
role       	: obtain the complement of a bitset.
@param     	: bitset *a: The operand to the complement operation.
@return		: bitset *: returns a pointer to the result of complement.
remark 	        :
******************************************************************************/
extern bitset * not(const bitset * a);

/*****************************************************************************
name		: not_result
role       	: obtain the conjunction of two bitsets and put result in arg2.
@param     	: bitset *result: One of the operands to the complement operation.
@return	: the result pointer, for convenience
******************************************************************************/
extern bitset *not_result(bitset *result);

/**
* Gets the value of a specified bit of a bitset.
* @param     a One of the bits of this bitset is of interest.
* @param	i The position of the bit of interest in the bitset.
* @return	The value of type BITSET_BLOCK_TYPE which is not zero
*		 if the bit is set to 1 and zero otherwise.
* WARNING: NEVER ASSIGN IT TO A BOOL-TYPE VARIABLE!!!
*/
extern BITSET_BLOCK_TYPE get_bit_val(const bitset *, const int i);

/**
* Sets the value of a specified bit of a bitset.
* @param a One of the bits of this bitset is to be changed.
* @param i The position of the bit of interest in the bitset.
* @param val The new value for the bit of interest should be zero for 0 and positive for 1.
*/
extern void set_bit_val(const bitset *, const int i, const BITSET_BLOCK_TYPE val);

/*****************************************************************************
name		: print_bitset_states
role       	: used to print the position of the bits
		 in the given bitset whose value is 1.
@param     	: bitset *a: The bitset to be printed.
@return		: 
remark 	        :
******************************************************************************/
extern void print_bitset_states(bitset *);

/*****************************************************************************
name		: free_bitset
role       	: used to free the given bitset.
@param     	: bitset *a: The bitset to be freed.
@return		: 
remark 	        :
******************************************************************************/
extern void free_bitset(bitset *);

/**
* Fills the given bitset with one.
* WARNING: Including the unused elements at the end of the last block!
* @param a the bitset to be filled.
* @return returns a pointer to the result of the filling operation.
*/
extern void fill_bitset_one(bitset *);

/*****************************************************************************
name		: is_bitset_zero
role       	: checks if the bitset contains only zeros.
@param     	: bitset *a: The bitset to be checked.
@return		: 0: bitset contains a non-zero, 1: bitset contains only zeros.
******************************************************************************/
extern int is_bitset_zero(bitset *);

/**
* Get the Index of the next non-zero element.
* @param a the bitset to be checked.
* @param idx the Present index. If set to -1 then it is the first time call
*		for this method on the given bitset i.e. we have
*		to start searching from the very beginning
* @return the next index, or -1 if there are no more elements left
*/
extern int get_idx_next_non_zero( const bitset *, const int);

/*****************************************************************************
name		: count_non_zero
role       	: count the number of non-zero elements in the given bitset.
@param     	: bitset *: The bitset to be checked.
@return		: the count.
******************************************************************************/
extern int count_non_zero(const bitset *);

/**
* Count the number of valid bits in a bitset.
* put all the valid-bit ids in an int-array.
* the first element of the array is the count.
* the rest are the bit ids of valid (set) bits.
* @param	: bitset *toCount: the bitset in question
* @return	: the count of bits that are set and their ids.
*/
extern int * count_set(bitset *toCount);

#endif
