/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: bitset.c,v $
*	$Revision: 1.23 $
*	$Date: 2007/10/15 14:31:35 $
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
*	Source description: Manage a Bitset - For the satisfaction relation in MCC.
*	Uses: Definition of bitset - bitset.h
*/

#include "bitset.h"

//This macro comutes the number of bytes structure
//needed in order to hold the LENGTH bits
#define NUMBER_OF_BLOCKS(LENGTH) ( ( int ) ceil( ( ( double ) ( LENGTH ) ) / ( ( double ) BITSET_BLOCK_SIZE ) ) )
//First we compute the number of byte structures
//required to hols IDX+1 bits and then we subtract 1
//NOTE: 1. This "+1" is because indexing of states
//         and bitset elements start from 0
//      2. This "subtract 1" is because index in the
//         array of bytes starts from 0
#define IDX_TO_BLOCK_NUM(IDX) ( NUMBER_OF_BLOCKS( ( IDX ) + 1 ) - 1 )
//Simply do IDX mod BITSET_BLOCK_SIZE to compute
//the bit position within the byte structure
#define IDX_TO_IN_BLOCK_POSITION(IDX) ( ( IDX ) % BITSET_BLOCK_SIZE )

//Since 255 is the greatest value of unsigned char, having a byte set to
//255 (converted into an unsigned char) is equal to setting all its bits to 1
#define BYTE_OF_ONES 255

/**
* Get a new bitset.
* A bitset is composed of an array of BITSET_BLOCK_SIZE-bit unsigned integers.
* @param n indicates the size of the required bitset.
* @return returns a pointer to a new bitset.
*/
bitset * get_new_bitset(int n)
{
	const int d = NUMBER_OF_BLOCKS(n);

	//Allocate the bitset
	bitset * newbitset = (bitset *)calloc(1,sizeof(bitset));
	
	//Allocate the bytes array
	if( n != 0 && d != 0 ){
		newbitset->bytesp = (BITSET_BLOCK_TYPE *)calloc(d,sizeof(BITSET_BLOCK_TYPE));
	}
	
	//Init the other bitset fields
	newbitset->n = n;
	newbitset->blocks_num = d;

	return newbitset;
}


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
void extend_bitset( bitset * pBitset, const int new_size, BOOL isFillNewBlocksWithOne ){
	IF_SAFETY( pBitset != NULL )
		const int old_size = pBitset->n;
		//We check for the number of valid elements here but not the block_num
		//because we can have the same number of blocks but still less valid elements
		IF_SAFETY( old_size <= new_size )
			const int new_blocks_num = NUMBER_OF_BLOCKS( new_size );
			const int old_blocks_num = pBitset->blocks_num;
			pBitset->n = new_size;
			//Otherwise there is no need to reallocate the memory
			if( new_blocks_num > old_blocks_num ){
				//Add new blocks
				pBitset->bytesp = realloc( pBitset->bytesp, new_blocks_num * sizeof(BITSET_BLOCK_TYPE) );
				//Update the number of blocks
				pBitset->blocks_num = new_blocks_num;
				
				//Fill the new blocks with one
				if( isFillNewBlocksWithOne ){
					memset( &( pBitset->bytesp[ old_blocks_num ] ), BYTE_OF_ONES,
						sizeof(BITSET_BLOCK_TYPE) * ( new_blocks_num - old_blocks_num ) );
				}
			}
		ELSE_SAFETY
			printf("ERROR: Trying to decrease the bitset size.\n");
			exit(1);
		ENDIF_SAFETY
	ELSE_SAFETY
		printf("ERROR: Trying to extend a NULL bitset.\n");
		exit(1);
	ENDIF_SAFETY
}


/**
* Obtain the disjunction of two bitsets.
* @param a one of the operands to the disjunction operation.
* @param b the second operand to the disjuction operation.
* @return returns a pointer to the result of disjunction.
* NOTE: The size of the operands must be the same else a NULL pointer is
* returned as the result.
*/
bitset * or(const bitset *a, const bitset *b)
{
	int i, n = a->n;
	bitset *result = NULL;
	if( n == b->n ){
		result = get_new_bitset(n);
		for( i=0; i < a->blocks_num ; i++ ){
			result->bytesp[i] = a->bytesp[i] | b->bytesp[i];
		}
	}else{
		printf("WARNING: Trying to construct a disjunction of two bitsets with different lenghts.\n");
	}
	return result;
}

/**
* Obtain the disjunction of two bitsets and put result in arg2.
* @param a one of the operands to the disjunction operation.
* @param b the second operand to the disjunction operation.
*/
void or_result(const bitset *a, bitset *result)
{
	int i;
	if( a->n == result->n ){
		for( i=0; i < a->blocks_num; i++ ){
			result->bytesp[i] = a->bytesp[i] | result->bytesp[i];
		}
	}else{
		printf("WARNING: Trying to construct a disjunction of two bitsets with different lenghts.\n");
	}
}


/**
* This method just copies elements from one bitset into another.
*@param from_bitset the bitset to be copied .
*@param to_bitset the bitset to copy to.
* NOTE: The two bitsets must have the same size.
*/
void copy_bitset(const bitset *from_bitset, bitset *to_bitset)
{
	if( from_bitset->n == to_bitset->n ){
		memcpy(to_bitset->bytesp, from_bitset->bytesp, sizeof(BITSET_BLOCK_TYPE) * from_bitset->blocks_num);
	}else{
		printf("WARNING: Trying to copy bitsets with different lenghts.\n");
	}
}

/**
* Obtain the exclusive-or of two bitsets.
* @param a one of the operands to the exclusive-or operation.
* @param b the second operand to the exclusive-or operation.
* @return returns a pointer to the result of exclusive-or
* NOTE: The size of the operands must be the same else a NULL pointer is
* returned as the result.
*/
bitset * xor(const bitset *a, const bitset *b)
{
	int i, n=a->n;
	bitset *result = NULL;
	if( n == b->n ){
		result = get_new_bitset(n);
		for( i=0; i < a->blocks_num; i++ ){
			result->bytesp[i] = a->bytesp[i] ^ b->bytesp[i];
		}
	}else{
		printf("WARNING: Trying to construct an exclusive disjunction of two bitsets with different lenghts.\n");
	}

	return result;
}

/**
* Obtain the xor of two bitsets and put result in arg2.
* @param a one of the operands to the x-or operation.
* @param b the second operand to the x-or operation.
*/
void xor_result(const bitset *a, bitset *result)
{
	int i;
	if( a->n == result->n ){
		for( i = 0; i < a->blocks_num; i++ ){
			result->bytesp[i] = a->bytesp[i] ^ result->bytesp[i];
		}
	}else{
		printf("WARNING: Trying to construct an exclusive disjunction of two bitsets with different lenghts.\n");
	}
}

/**
* Obtain the conjunction of two bitsets.
* @param a: One of the operands to the conjunction operation.
* @param b: The second operand to the conjunction operation.
* @return returns a pointer to the result of conjunction.
* NOTE: The size of the operands must be the same else a NULL pointer is
* returned as the result.
*/
bitset * and(const bitset *a, const bitset *b)
{
	int i, n=a->n;
	bitset *result = NULL;
	if( n == b->n ){
		result = get_new_bitset(n);
		for( i = 0; i < a->blocks_num; i++ ){
			result->bytesp[i] = a->bytesp[i] & b->bytesp[i];
		}
	}else{
		printf("WARNING: Trying to construct a conjunction of two bitsets with different lenghts.\n");
	}

	return result;
}

/**
* Obtain the conjunction of two bitsets and put result in arg2.
* @param a one of the operands to the conjunction operation.
* @param b the second operand to the conjunction operation.
*/
void and_result(const bitset *a, bitset *result)
{
	int i;
	if( a->n == result->n ){
		for( i = 0; i < a->blocks_num; i++ ){
			result->bytesp[i] = a->bytesp[i] & result->bytesp[i];
		}
	}else{
		printf("WARNING: Trying to construct a conjunction of two bitsets with different lenghts.\n");
	}
}

/**
* Obtain the complement of a bitset.
* @param a the operand to the complement operation.
* @return	returns a pointer to the result of complement.
*/
bitset * not(const bitset *a)
{
	int i, n=a->n, d;
	bitset *result = NULL;
	bitset *dummy = get_new_bitset(n);
	
	fill_bitset_one(dummy);
	
	result = xor(a, dummy);
	
	free_bitset(dummy);
	return result;
}

/**
* Obtain the conjunction of two bitsets and put result in arg2.
* @param result one of the operands to the complement operation.
* @return the result pointer, for convenience
*/
bitset *not_result(bitset *result){
	bitset *dummy = get_new_bitset( result->n );
	fill_bitset_one(dummy);
	xor_result(dummy, result);
	free_bitset(dummy);
	return result;
}

/**
* obtain the value of a specified bit of a bitset.
* @param     a One of the bits of this bitset is of interest.
* @param	i The position of the bit of interest in the bitset.
* @return	The value of type BITSET_BLOCK_TYPE which is not zero
*		 if the bit is set to 1 and zero otherwise.
* WARNING: NEVER ASSIGN IT TO A BOOL-TYPE VARIABLE!!!
*/
BITSET_BLOCK_TYPE get_bit_val(const bitset * a, const int i){
	int d, r;
	BITSET_BLOCK_TYPE val;
	
	if( i < 0 || i >= a->n ){
		return -1;
	}
	
	//Compute the indeces
	d = IDX_TO_BLOCK_NUM(i);
	r = IDX_TO_IN_BLOCK_POSITION(i);
	
	//Create the mask for conjunction
	val = 1 << r;
	
	//Return the conjunct
	return a->bytesp[d] & val;
}

/**
* Sets the value of a specified bit of a bitset.
* @param a One of the bits of this bitset is to be changed.
* @param i The position of the bit of interest in the bitset.
* @param val The new value for the bit of interest should be zero for 0 and positive for 1.
*/
void set_bit_val(const bitset * a, const int i, const BITSET_BLOCK_TYPE val)
{
	int d;
	BITSET_BLOCK_TYPE value = BIT_ON;
	
	IF_SAFETY( i >= 0 && i < a->n )
		d = IDX_TO_BLOCK_NUM(i);
		
		value <<= IDX_TO_IN_BLOCK_POSITION(i);
		
		if( val ){
			a->bytesp[d] |= value;
		}else{
			a->bytesp[d] &= (~value);
		}
	ELSE_SAFETY
		printf("ERROR: trying to set a value to bit number %d, which is out of bound [0,%d]\n", i , a->n-1);
		exit(1);
	ENDIF_SAFETY
}

/**
* Print the position of the bits in the given bitset whose value
* is 1.
* @param a the bitset to be printed.
*/
void print_bitset_states(bitset *a)
{
	int i;
	BOOL bFirst = TRUE;
	
	if( a != NULL ){
		const int size = a->n;
		printf("{ ");
		for( i = 0; i < size; i++ ){
			if( get_bit_val(a, i) ){
				if( ! bFirst ){
					printf(", ");
				}else{
					bFirst = FALSE;
				}
				printf("%d", i+1);
			}
		}
		printf(" }");
	}else{
		printf("??\n");
		printf("WARNING: The bitset does not exist.");
	}
}

/**
* Free the given bitset.
* @param a the bitset to be freed.
*/
void free_bitset(bitset *a)
{
	if( a )
	{
		if(a->bytesp) free(a->bytesp);
		free(a);
	}
}

/**
* Fills the given bitset with one.
* WARNING: Including the unused elements at the end of the last block!
* @param a the bitset to be filled.
* @return returns a pointer to the result of the filling operation.
*/
void fill_bitset_one(bitset *a)
{
	memset(a->bytesp, BYTE_OF_ONES, sizeof(BITSET_BLOCK_TYPE) * a->blocks_num);
}

/**
* Checks if the bitset contains only zeros.
* @param a the bitset to be checked.
* @return 0: bitset contains a non-zero, 1: bitset contains only zeros.
*/
int is_bitset_zero(bitset *a)
{
	int i, flag=1;
	for( i=0; i < a->blocks_num; i++ ){
		if( a->bytesp[i] ){
			flag=0;
			break;
		}
	}
	return flag;
}

/**
* Get the Index of the next non-zero element.
* @param a the bitset to be checked.
* @param idx the Present index. If set to -1 then it is the first time call
*		for this method on the given bitset i.e. we have
*		to start searching from the very beginning
* @return the next index, or -1 if there are no more elements left
*/
int get_idx_next_non_zero(const bitset *a, const int idx){
	//If idx==-1 then we set it to 0 to make us searching
	//from the very first byte of the bitset
	const int p = IDX_TO_BLOCK_NUM(idx == -1 ? 0 : idx);
	//If idx==-1 then r will be -1 which will make us searching
	//from the very beginning of the byte
	const int r = IDX_TO_IN_BLOCK_POSITION(idx);
	//This is the number of bytes in the bitset
	const int NUMBER_OF_BYTES = a->blocks_num;
	//This is the index of the last byte in the bitset
	const int LAST_BLOCK_INDEX = NUMBER_OF_BYTES - 1;
	//This is the last valid bit index in the last byte of the bitset
	const int LAST_BIT_INDEX = IDX_TO_IN_BLOCK_POSITION( a->n - 1);

	int i, j, r_val;
	BOOL found = FALSE;
	BITSET_BLOCK_TYPE val = 1;
	BITSET_BLOCK_TYPE bits;
	
	//Set the starting bit for search in the byte number p
	j = r + 1;
	//Do search for the other non zerror elements
	for( i = p; i < NUMBER_OF_BYTES; i++ ){
		bits = a->bytesp[i] >> j;
		//Check if the current block is not empty
		if( bits ){
			//Here we should stop iterations if: 
			//1. idx was pointing to the last element of this byte subset
			//2. the rest of the byte subset are just zerro bits
			//3. If we are in the last byte of the bitset and we
			//   have checked the last VALID bit of the bitsetset i.e. LAST_BIT_INDEX
			while( j < BITSET_BLOCK_SIZE && bits && ( i < LAST_BLOCK_INDEX || j <= LAST_BIT_INDEX )){
				if( ( bits & val ) ){
					found = TRUE;
					break;
				}
				bits = bits >> 1;
				j++;
			}
			if( found ){
				//If a nonzerror bit is found then break
				break;
			}
		}
		//In the next byte we start searching from the very first bit.
		j = 0;
	}
	if( found ){
		r_val = i * BITSET_BLOCK_SIZE + j;
	}else{
		r_val = -1;
	}
	return r_val;
}

/**
* Count the number of non-zero elements in the given bitset.
* @param a the bitset to be checked.
* @return the count.
*/
int count_non_zero(const bitset *a)
{
	int i=-1, count=0;
	while( ( i = get_idx_next_non_zero(a, i) ) != -1 ) ++count;
	return count;
}

/**
* Count the number of valid bits in a bitset.
* put all the valid-bit ids in an int-array.
* the first element of the array is the count.
* the rest are the bit ids of valid (set) bits.
* @param	: bitset *toCount: the bitset in question
* @return	: the count of bits that are set and their ids.
*/
int * count_set(bitset *toCount)
{
	int size = toCount->n, i;
	int *pValidSet = (int *) calloc (1, sizeof(int));
	pValidSet[0] = 0;
	for( i=0; i < size; i++ ) {
		if( get_bit_val(toCount, i) ){
			++pValidSet[0];
			pValidSet = (int *) realloc (pValidSet, (pValidSet[0]+1) * sizeof(int));
			pValidSet[pValidSet[0]] = i;
		}
	}
	return pValidSet;
}
