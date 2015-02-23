/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: sparse.c,v $
*	$Revision: 1.9 $
*	$Date: 2007/10/12 11:23:54 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Maneesh Khattri, Ivan Zapreev, Tim Kemna
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
*	Source description: Manage Sparse Matrix.
*	Uses: DEF: sparse.h
*	Definition of sparse matrix - sparse.h
*/

#include "sparse.h"

/*======================================================================*/
/************************************************************************/
/************************General Sparse matrix methods*******************/
/**************For matrices allocated with "init_matrix"*****************/
/*  NOTE: NOT ALL of these methods are applicable to matrices allocated
	with the "init_matrix_ncolse" method! For example you can not
	add a new row element with reallocating the row elements array!  */
/*======================================================================*/

/**
* Creates new R matrix and copies all the rows that belong to the pValidStates into it
* @param pStateSpace the initial matrix
* @param pQ the new matrix to where to copy rows
* @param pValidStates the valid states i.e. ids of the rows to be copied
*                     this array contains the number of nodes as the first element
*/
void initMatrix(const sparse * pStateSpace, sparse * pQ, const int * pValidStates)
{
	int i;
	int * pNCols = pStateSpace->ncols;
	values * pRows = pStateSpace->val;
	int valid_state;
	
	/*Form the pQ matrix*/
	for( i = 1; i <= pValidStates[0] ; i++ )
	{
		valid_state = pValidStates[i];
		set_mtx_row( pQ, valid_state, pNCols[valid_state], pRows[valid_state] );
	}
}

/*This is the zerro row*/
static const values ZERO_ROW = {NULL,NULL,NULL,0};

/**
* This method removes all the non zero rows from the Q matrix of the BSCC
* @param pQ the BSCC nodes of the diagoriginal matrix
* @param pValidStates the valid states i.e. ids of the rows which have to be cleaned
*                     this array contains the number of nodes as the first element
*/
void cleanMatrix(sparse * pQ, const int * pValidStates)
{
	/*Remove the non zero rows*/
	int i, length = pValidStates[0];
	for( i = 1; i <= length ; i++ )
	{
		set_mtx_row( pQ, pValidStates[i], 0 , ZERO_ROW );
	}
	/*NoTe: Because of the sparse matrix functions we do not need to
	restore the pStateSpace->val diagonal values as we do not modify them*/
}

/*****************************************************************************
name		: free_sparse
role       	: frees the sparse matrix.
@param		: sparse * pM the matrix to remove
remark 	        :
******************************************************************************/
void free_mtx_sparse(sparse * pM)
{
	int i,col,j, rows = pM->rows;
	if(pM)
	{
		for(i=0;i<rows;i++)
		{
			if(pM->ncols[i])
			{
				if(pM->rows==pM->cols)
					free(pM->val[i].back_set);
				free(pM->val[i].col);
				free(pM->val[i].val);
			}
		}
		free(pM->val);
		free(pM->ncols);
		//The pM->pcols structure is valid only for matrix with
		//pM->rows == pM->cols but it is allocated for all of
		//them, so it whould be cleaned also
		if ( /*pM->rows == pM->cols &&*/ pM->pcols ) free(pM->pcols);
		free(pM);
	}
}

/**
* ADD a NEW value to the matrix.
* WARNING: Should NOT be used with the matrices allocated with
* init_matrix_ncolse(,,) method!
* @param	: sparse * pM the matrix to add element to
* @param	: int row: The row number in which the value is to be set.
* @param	: int col: The column number in which the value is to be set.
* @param	: double val: The value to be set.
* @return	: int: 0: fail, 1: success
* NOTE: return 0 indicates that row/col does not meet the bounds of matrix
*/
unsigned int set_mtx_val(sparse * pM, int row, int col, double val)
{
	int idx = pM->ncols[row], idxp=0;
	if( !pM || row < 0 || row > pM->rows ||col < 0 || col > pM->cols ) return 0;
	if( val != 0 && row != col )
	{
		pM->val[row].col = (int *)realloc(pM->val[row].col,(idx+1)*sizeof(int));
		pM->val[row].val = (double *)realloc(pM->val[row].val, (idx+1)*sizeof(double));
		pM->val[row].col[idx] = col;
		pM->val[row].val[idx] = val;
		if( pM->rows == pM->cols )
		{
			idxp=pM->pcols[col];
			pM->val[col].back_set = (int *)realloc(pM->val[col].back_set, (idxp+1)*sizeof(double));
			pM->val[col].back_set[idxp]=row;
			++pM->pcols[col];
		}
		++pM->ncols[row];
	}
	else if( val != 0 && row == col )
		pM->val[row].diag = val;
	return 1;
}

/*****************************************************************************
name		: free_mtx_wrap
role       	: frees the sparse matrix. Does not clean rows.
@param		: sparse * pM the matrix to remove
******************************************************************************/
extern void free_mtx_wrap(sparse *pM)
{
	if(pM)
	{
		free(pM->val);
		free(pM->ncols);
		//The pM->pcols structure is valid only for matrix with
		//pM->rows == pM->cols but it is allocated for all of
		//them, so it whould be cleaned also
		if ( /*pM->rows == pM->cols &&*/ pM->pcols ) free(pM->pcols);
		free(pM);
	}
}

/**
* Add a value to an existing one in the matrix or inserts a new value if the is none.
* WARNING: Should NOT be used with the matrices allocated with
* init_matrix_ncolse(,,) method! Unless you are 100% sure that
* the pM[row,col] value already exists in the matrix!
* @param	: sparse * pM the matrix
* @param     : int row: The row number in which the value is to be added.
* @param	: int col: The column number in which the value is to be added.
* @param	: double val: to be added.
*/
void add_mtx_val(sparse * pM, int row, int col, double val)
{
	int ncols = pM->ncols[row], i;
	unsigned int found=0;
	if( !pM || row < 0 || row > pM->rows || col <0 || col > pM->cols )
		return;
	if ( row != col )
	{
		for( i=0; i < ncols; i++ )
		{
			if( pM->val[row].col[i] == col )
			{
				pM->val[row].val[i]+=val;
				found=1;
				break;
			}
		}
	}
	else
	{
		pM->val[row].diag+=val;
		found = 1;
	}
	if( found == 0 )
		set_mtx_val(pM, row, col, val);
}

/**
* Gets the next value in the row of a matrix.
* WARNING: Does not get the values in the diagonal - Use get_mtx_diag_val
* @param	: sparse * pM the matrix to get element from
* @param     : int row: The row number in which the value is to be set.
* @param	: int * col (by reference): The present col. (Initially points to NULL)
* @param	: double * val (by reference): The present value. (Initially points to NULL)
* @param	: int *count (by reference): The count.
* @return	: int: TRUE if there is the next value, otherwise FALSE
* NOTE: return 0 value was not found / row/col does not meet bounds.
*/
BOOL get_mtx_next_val(const sparse * pM, const int row, int *col, double *val, int *count)
{
	//printf("[DEBUG] Looking up row = %d, count = %d\n", row, *count);
	const int ncols = pM->ncols[row];
	BOOL found = FALSE;

	if( !pM || row < 0 || row > pM->rows ){
		found = FALSE;
	}
	else if( *count == 0 )
	{
		if( ncols == 0 )
			found = FALSE;
		else{
			*count = 1;
			*col = pM->val[row].col[*count-1];
			*val = pM->val[row].val[*count-1];
			found = TRUE;
		}
	}
	else
	{
		if( ( *count + 1 ) > ncols )
			found = FALSE;
		else{
			++*count;
			*col = pM->val[row].col[*count-1];
			*val = pM->val[row].val[*count-1];
			found = TRUE;
		}
	}
	//printf("[DEBUG] %s, col = %d, value = %lf\n", ( found ? "Found" : "Did not find"), (found ? *col:-1), (found ? *val:-1.0));
	return found;
}

/*****************************************************************************
name		: get_mtx_diag_val
role		: get the diag value in the row of a matrix.
@param		: sparse * pM the matrix to get element from
@param		: int row: The row number in which the value is to be set.
@param		: double * val (by reference): The value.
@return	: int: 0: fail, 1: success
remark		: return 0: value was not found / row/col does not meet bounds.
		  **Does not get the values in the diagonal**
******************************************************************************/
BOOL get_mtx_diag_val(const sparse * pM, const int row, double *val)
{
	unsigned int found = FALSE;
	if( pM->val[row].diag != 0 )
	{
		*val = pM->val[row].diag;
		found = TRUE;
	}
	return found;
}

/*****************************************************************************
name		: get_mtx_val
role       	: get a value in the matrix. Perform binary search to get a
		non-diagonal value in the matrix when the number of
		non-diagonal elements in a row is >4.
@param		: sparse * pM the matrix to get element from
@param     	: int row: The row number in which the value is to be set.
@param		: int col: The column number in which the value is to be set.
@param		: double * val (by reference): The value.
@return		: int: 0: fail, 1: success
remark 	        : return 0: value was not found / row/col does not meet bounds.
******************************************************************************/
unsigned int get_mtx_val(const sparse * pM, const int row, const int col, double *val)
{
	const int ncols = pM->ncols[row];
	int high = pM->ncols[row]-1, low = 0 , i; //high and low are used for binary search
	unsigned int found = 0;
	
	if( !pM || row < 0 || row > pM->rows || col <0 || col > pM->cols )
	{
		return found;
	}
	
	if ( row != col )
	{
		//If there are <= than 4 elements in a row then we can do a regular search
		if(high > 4){
			while(low < high){
				i = (low+high)/2;
				if( pM->val[row].col[i] == col ){
					*val = pM->val[row].val[i];
					found=1;
					break;
				}else if(col > pM->val[row].col[i]){
					low = i + 1;
				}else{
					high = i - 1;
				}
			}
			// low == high => there may be no required element in the (row,col) cell
			if( pM->val[row].col[low] == col )
			{
				*val = pM->val[row].val[low];
				found=1;
			}
		}else{
			for( i=0; i < ncols; i++ )
			{
				if( pM->val[row].col[i] == col )
				{
					*val = pM->val[row].val[i];
					found=1;
					break;
				}
			}
		}
	}
	else if( pM->val[row].diag )
	{
		*val = pM->val[row].diag;
		found = 1;
	}
	return found;
}

/*****************************************************************************
name		: get_mtx_row_sums
role       	: add the elements in each row, including the diagonal element.
@param		: sparse * pM the matrix to work with
@return		: double *: row sums for each row.
remark 	        :
******************************************************************************/
double * get_mtx_row_sums(sparse * pM)
{
	int i,j, size = pM->rows;
	double *row_sum = (double *)calloc(size, sizeof(double));
	for( i=0; i < size; i++ )
	{
		int col = pM->ncols[i];
		double result=0.0;
		for( j=0; j < col; j++ )
		{
			result += pM->val[i].val[j];
		}
		row_sum[i] = result + pM->val[i].diag;
	}
	return row_sum;
}

/*****************************************************************************
name		: get_mtx_cer_row_sums
role       	: add elements in certain rows and return the vector of values.
@param		: sparse * pM the matrix to work with
@param		: int * pIds the ids of the rows for which the sums should be computed
			the pIds[0] contains the number of row ids
@return		: the rows' sums
******************************************************************************/
double * get_mtx_cer_row_sums(sparse * pM, int * pIds)
{
	int i, j;
	int * pNCols = pM->ncols;
	values * pRows = pM->val;
	int num = pIds[0];
	
	double * pE = (double *)calloc(num, sizeof(double));
	for( i=1; i<= num; i++ )
	{
		int col = pNCols[ pIds[i] ];
		values row = pRows[ pIds[i] ];
		
		/*sum all the non diagonal elements first*/
		double result=0.0;
		for( j=0; j < col; j++ )		
			result += row.val[j];
		/*Add diagonal value*/
		pE[ i-1 ] = result + row.diag;
	}
	return pE;
}

/*****************************************************************************
name		: mult_mtx_const
role       	: multiply all elements in the matrix with a constant.
@param		: sparse * pM the matrix to multiply
@param     	: double constant: The constant value.
******************************************************************************/
void mult_mtx_const(sparse * pM, double constant)
{
	int i,j;
	int length = pM->rows;
	int * pNCols = pM->ncols;
	values * pRows = pM->val;
	values * pRow = NULL;
	double * pVal;
	
	for( i=0; i < length; i++ )
	{
		int col = pNCols[i];
		pRow = & pRows[i];
		pVal = pRow->val;
		for( j=0; j < col; j++ )		
			pVal[j] *= constant;
		pRow->diag *= constant;
	}
}

/*****************************************************************************
name		: mult_mtx_cer_const
role       	: multiply all elements in the matrix with a constant.
@param		: sparse * pM the matrix to multiply
@param     	: double constant: The constant value.
@param		: int * pValidStates the valid rows to multiply pValidStates[0]
		  is the number of states (rows) in the array
******************************************************************************/
void mult_mtx_cer_const(sparse * pM, double constant, int * pValidStates)
{
	int i, j, id;
	int length = pValidStates[0];
	int * pNCols = pM->ncols, col;
	values * pRows = pM->val;
	values * pRow = NULL;
	double * pVal;
	
	for( i=1; i <= length; i++ )
	{
		id = pValidStates[i];
		col = pNCols[ id ];
		pRow = & pRows[ id ];
		pVal = pRow->val;
		for( j=0; j < col; j++ )		
			pVal[j] *= constant;
		pRow->diag *= constant;
	}
}

/**
* Multiply all elements in all i'th rows of the matrix by a constant[i] (or 1/constant[i] ).
* @param pM the matrix to multiply
* @param constant The constant value.
* @param pValidStates the valid rows to multiply pValidStates[0]
*		  	is the number of states (rows) in the array
* @param invert if == 0 then nothing, otherwise we invert the elements of constant
*			before the multiplication.
*/
void mult_mtx_cer_const_array(const sparse * pM, const double *constant, const int * pValidStates, const int invert){
	int i, j, id, col;
	int length = pValidStates[0];
	double * pVal;
	double cons;
	
	for( i=1; i <= length; i++ ){
		id = pValidStates[i];
		col = pM->ncols[ id ];
		pVal = pM->val[id].val;
		cons = constant[id];
		if ( invert && cons ) cons = 1 / cons;
		for( j=0; j < col; j++ ){
			pVal[j] *= cons;
		}
		pM->val[id].diag *= cons;
	}
}

/**
* This method makes an "embedded" DTMC out of the rate matrix.
* WARNING: The diagonal elements remain untouched, and therefor
* should not be considered in any further operations on the resulting matrix.
* WARNING: We do not just make an embedded DTMC, but we also eliminate the self loops
* 		As if we would not have the rate matrix but the generator matrix as the original
* WARNING: The self loops on the absorbing states are not modified, i.e. the
*	proper way to identify an absorbing state is to make sure that there are
*	no non-diagonal elements in the corresponding row.
* @param pM the matrix to operate on
* @param constant the array of row sums of the rate matrix.
* @param pValidStates the valid rows of the matrix to operate on.
*		NOTE: The element pValidStates[0] should store
*		the number of states (rows) in the array.
*/
void make_embedded_dtmc_out_of_rate_mtx(const sparse * pM, const double *pRowSums, const int * pValidStates){
	int row_idx, column_idx;
	const int length = pValidStates[0];
	
	//Iterate through the allowed states
	for( row_idx = 1; row_idx <= length; row_idx++ ){
		//Obtain the allowed state index
		int id = pValidStates[row_idx];
		//Get the number of non diagonal elements in the row
		int non_zero_off_diag_columns = pM->ncols[ id ];
		
		//if there are non-diagonal elements
		if( non_zero_off_diag_columns > 0 ){
			//Get the matrix row
			values pMtxRow = pM->val[id];
			
			//Create the normalization factor, the diagonal element should be excluded
			//because we do not want to have loops in the resulting DTMC
			double normalization_factor =  pRowSums[id] - pMtxRow.diag ;
			
			//NOTE: Since non_zero_off_diag_columns is positive
			//We shall assume that normalization_factor > 0,
			//but let's do a safety check just in case
			IF_SAFETY( normalization_factor > 0 )
				//Get the non-diagonal row elements
				double *pVal = pMtxRow.val;
				for( column_idx=0; column_idx < non_zero_off_diag_columns; column_idx++ ){
					//Normalize the off-diagonal elements of the row
					pVal[column_idx] /= normalization_factor;
				}
			ELSE_SAFETY
				printf("ERROR: The sparce matrix seems to contain zero elements in row %d.\n",row_idx);
				exit(1);
			ENDIF_SAFETY
		}
	}
}

/**
* This method restores the rate matrix from the "embedded" DTMC odtained
* with the "make_embedded_dtmc_out_of_rate_mtx" method.
* WARNING: The diagonal elements remain untouched, and therefor
* we expect them to be preserved from before the matrix was turned in to an embedded DTMC.
* @param pM the matrix to operate on
* @param constant the array of row sums of the rate matrix.
* @param pValidStates the valid rows of the matrix to operate on.
*		NOTE: The element pValidStates[0] should store
*		the number of states (rows) in the array.
*/
void restore_rate_mtx_out_of_embedded_dtmc(const sparse * pM, const double *pRowSums, const int * pValidStates){
	int row_idx, column_idx;
	const int length = pValidStates[0];
	
	//Iterate through the allowed states
	for( row_idx = 1; row_idx <= length; row_idx++ ){
		//Obtain the allowed state index
		int id = pValidStates[row_idx];
		//Get the number of non diagonal elements in the row
		int non_zero_off_diag_columns = pM->ncols[ id ];
		
		//if there are non-diagonal elements
		if( non_zero_off_diag_columns > 0 ){
			//Get the matrix row
			values pMtxRow = pM->val[id];
			
			//Recreate the normalization factor, the diagonal element should be excluded
			//because we do not want to have loops in the resulting DTMC
			double normalization_factor =  pRowSums[id] - pMtxRow.diag;
			//WARNING: We assume that pMtxRow.diag contains the diagonal
			//element of the rate mtx, i.e. it was not modified
			
			//Get the non-diagonal row elements
			double *pVal = pMtxRow.val;
			for( column_idx=0; column_idx < non_zero_off_diag_columns; column_idx++ ){
				//Restore the off-diagonal elements of the row
				pVal[column_idx] *= normalization_factor;
			}
		}
	}
}

/*****************************************************************************
name		: add_mtx_diagona
role       	: add the elements of the input array to the diagonal one-one.
@param		: sparse * pM the matrix to work with
@param     	: double *diagonal: The addendum to the diagonal.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
		  size should be correct.
******************************************************************************/
void add_mtx_diagonal(sparse * pM, double *diagonal)
{
	int i, size;
	values *pRows = pM->val;
	size = pM->rows;
	for( i=0; i < size; i++ )
		pRows[i].diag += diagonal[i];
}

/*****************************************************************************
name		: sub_mtx_diagonal
role       	: sub the elements of the input array to the diagonal one-one.
@param		: sparse * pM the matrix to work with
@param     	: double *diagonal: The addendum to the diagonal.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
		  size should be correct.
******************************************************************************/
unsigned int sub_mtx_diagonal(sparse * pM, double *diagonal)
{
	int i, size;
	if(!pM) return 0;
	size = pM->rows;
	for( i=0; i < pM->rows; i++ )
	{
		pM->val[i].diag -= diagonal[i];
	}
	return 1;
}

/****************************************************************************
name		: add_mtx_cer_diagonal
role		: diag elements = cons - diag elements
@param		: sparse * pM the matrix to work with
@param		: int * pIds the ids of the rows in which diagonal elements
		  should be reduced by the correcponding pE values
		  the pIds[0] contains the number of row ids
@param		: int constant: an operand (cons)
******************************************************************************/
void add_cer_cons_diagonal_mtx(sparse *pM, int * pIds, int constant)
{
	int i;
	values * pRows = pM->val;
	int num = pIds[0];
	for( i=1; i <= num; i++ )
		pRows[ pIds[i] ].diag = constant+pRows[ pIds[i] ].diag;
}

/****************************************************************************
name		: sub_cer_cons_diagonal_mtx
role		: diag elements = cons - diag elements
@param		: sparse * pM the matrix to work with
@param		: int * pIds the ids of the rows in which diagonal elements
		  should be reduced by the correcponding pE values
		  the pIds[0] contains the number of row ids
@param		: int constant: an operand (cons)
******************************************************************************/
void sub_cer_cons_diagonal_mtx(sparse *pM, int * pIds, int constant)
{
	int i;
	values * pRows = pM->val;
	int num = pIds[0];
	for( i=1; i <= num; i++ )
	{
		pRows[ pIds[i] ].diag = constant-pRows[ pIds[i] ].diag;
	}
}

/****************************************************************************
name		: sub_mtx_cer_diagonal
role		: Subtract elements of the pE array from the predefined diagonal
		  elements of the matrix
@param		: sparse * pM the matrix to work with
@param		: int * pIds the ids of the rows in which diagonal elements
		  should be reduced by the correcponding pE values
		  the pIds[0] contains the number of row ids
@param		: double * pE: The elements to be subtracted
******************************************************************************/
void sub_mtx_cer_diagonal(sparse *pM, int * pIds, double * pE)
{
	int i;
	values * pRows = pM->val;
	int num = pIds[0];
	for( i=1; i <= num; i++ )
	{
		pRows[ pIds[i] ].diag -= pE[i-1];
	}
}

/*****************************************************************************
name		: add_mtx_cons_diagonal
role       	: add a constant to the all the elements of the diagonal.
@param		: sparse * pM the matrix to work with
@param     	: double constant: The constant value.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
******************************************************************************/
unsigned int add_mtx_cons_diagonal(sparse * pM, double cons)
{
	int i, size;
	if(!pM) return 0;
	size = pM->rows;
	for(i=0;i<pM->rows;i++)
		pM->val[i].diag += cons;
	return 1;
}

/*****************************************************************************
name		: set_mtx_row
role       	: set a row in the matrix.
		  NOTE: This function copies the values structure that is why change
		  of fields in the original row lateron will not affect the one which
		  was set by this function. For instance change of the diagonal element
		  in the original matrix will not affect the row added to the other matrix
		  by this method.
@param		: sparse * pM the matrix to work with
@param     	: int row: The row number in which the value is to be set.
@param		: int ncol: The number of valid columns (non-zero) in the given row.
@param		: values rowval: The row value to be set.
@return		: int: 0: fail, 1: success
remark 	        : return 0: indicates that row/col does not meet the bounds
		  of matrix
******************************************************************************/
unsigned int set_mtx_row(sparse * pM, int row, int ncols, values rowval)
{
	if( !pM || row < 0 || row > pM->rows )
		return 0;
	pM->ncols[row] = ncols;
	pM->val[row] = rowval;
	return 1;
}

/*****************************************************************************
name		: multiply_mtx_MV
role       	: multiply a matrix with a vector.
@param		: sparse * pM: operand matrix.
@param     	: double *vec: The operand vector.
@param		: double *res: The resulting vector.
@return		: int: 0: fail, 1: success
remark 	        : size should be correct.
******************************************************************************/
void multiply_mtx_MV(sparse * pM, double *vec, double *res)
{
	int i, j, rows = pM->rows, cols, *col, *ncols=pM->ncols;
	double *val, result;
	for(i=0;i<rows;i++)
	{
		cols = *ncols++;
		result = pM->val[i].diag*vec[i];
		if(cols!=0)
		{
			col = pM->val[i].col; val = pM->val[i].val;
			for(j=0;j<cols;j++)
				result+=vec[*col++]*(*val++);
		}
		res[i]=result;
	}
}

/*****************************************************************************
name		: multiply_mtx_cer_MV
role       	: multiply certain rows of a matrix with a vector.
@param		: sparse * pM: operand matrix.
@param     	: double *vec: The operand vector.
@param		: double *res: The resulting vector.
@param		: int num: number of valid rows.
@param		: int *valid_rows: the valid rows
@return		: int: 0: fail, 1: success
remark 	        : size should be correct.
******************************************************************************/
void multiply_mtx_cer_MV(sparse *pM, double *vec, double *res, int num, int * valid_rows)
{
	int i, j, rows = pM->rows, cols, *col, v;
	double *val, result;
	for(i=0;i<num;i++, valid_rows++)
	{
		v = *valid_rows;
		cols = pM->ncols[v];
		result = pM->val[v].diag*vec[v];
		if(cols!=0)
		{
			col = pM->val[v].col; val = pM->val[v].val;
			for(j=0;j<cols;j++)
				result+=vec[*col++]*(*val++);
		}
		res[v]=result;
	}
}

/*****************************************************************************
name		: multiplyTMV
role       	: multiply a vector to a matrix.
@param		: sparse * pM the matrix to be used
@param     	: double *vec: The operand vector.
@param		: double *res: The resulting vector.
@return		: double *: the resulting vector is also returned here for convinient usage
remark 	        : size should be correct.
******************************************************************************/
double * multiply_mtx_TMV(sparse * pM, double *vec, double *res)
{
	int i, j, rows = pM->rows, cols, *tcol;
	double  *tval, vec_i;
	/*Set the resulting array to zero values*/
	bzero( res, sizeof(double)*rows );
	/*Compute*/
	for( i=0; i < rows; i++ )
	{
		cols = pM->ncols[i];
		vec_i = vec[i];
		res[i] += vec_i * pM->val[i].diag;
		if(cols==0) continue;
		tcol = pM->val[i].col;
		tval = pM->val[i].val;
		for( j=0; j < cols; j++, tcol++, tval++)
			res[*tcol] += vec_i * *tval;
	}
}

/*****************************************************************************
name		: multiply_mtx_cer_TMV
role       	: multiply a vector to a matrix.
@param		: sparse * pM the matrix to be used
@param     	: int vec: The operand vector.
@param		: int res: The resulting vector.
@return		: double *: the resulting vector is also returned here for convinient usage
@param		: int * pIds the valid ids, pIds[0] contains the number of ids
remark 	        : size should be correct.
******************************************************************************/
double * multiply_mtx_cer_TMV(sparse * pM, double * vec, double *res, int * pIds )
{
	/*ToDo: Implement*/
	int i, j, total_length = pM->rows, cols, *pCols, * pNCols = pM->ncols, length = pIds[0], id;
	double  *pVals, vec_i;
	values * pRow, * pRows = pM->val;
	
	/*Set the resulting array to zero values*/
	bzero( res, sizeof(double)*total_length );
	
	/*Compute*/
	for( i=1; i <= length; i++ )
	{
		id = pIds[i];		/*Get the valid row id*/
		cols = pNCols[id];	/*The number of columns in a row, BUG_ID: 004*/
		vec_i = vec[id];	/*The corresponding element in the vec vector*/
		pRow = & pRows[id];	/*The row values*/
		
		res[id] += vec_i * pRow->diag;	/*First multiply the diagonal value*/
		
		if( cols == 0 ) continue; /*If there are no non diagonal elements then skip*/
		
		/*Process the rest elements of the row*/
		pCols = pRow->col;
		pVals = pRow->val;
		for( j=0; j < cols; j++, pCols++, pVals++)
			res[*pCols] += vec_i * *pVals;
	}
	return res;
}

/*======================================================================*/
/************************************************************************/
/************************An alternative MARTIX ALLOCATION****************/
/***********For matrices allocated with "init_matrix_ncolse"*************/
/*======================================================================*/

/*
	The way this sparce matrix is allocated in "init_sparse_ncolse" imposes some
	strict conditions on some operations on this matrix, such as adding a new element
*/

/*****************************************************************************
name		: free_sparse
role       	: frees the sparse matrix.
@param		: sparse * pM the matrix to remove
remark 	        :
******************************************************************************/
void free_sparse_ncolse(sparse * pM)
{
	int i,col,j, rows = pM->rows;
	if(pM)
	{
		if(pM->rows==pM->cols)
		{
			for(i=0;i<pM->rows;i++)
				free(pM->val[i].back_set);
		}
		if(pM->val && pM->val[0].col && pM->val[0].val)
			{free(pM->val[0].col); free(pM->val[0].val);}
		free(pM->val);
		free(pM->ncols);
		//The pM->pcols structure is valid only for matrix with
		//pM->rows == pM->cols but it is allocated for all of
		//them, so it whould be cleaned also
		if ( /*pM->rows == pM->cols && */ pM->pcols ) free(pM->pcols);
		free(pM);
	}
}

/*****************************************************************************
name		: set_mtx_val_ncolse
role		: set a value in the matrix.
@param		: sparse * pM the matrix to remove
@param		: int row: The row number in which the value is to be set.
@param		: int col: The column number in which the value is to be set.
@param		: double val: The value to be set.
@return	: int: 0: fail, 1: success
remark		: return 0: indicates that row/col does not meet the bounds
		  of matrix
******************************************************************************/
void set_mtx_val_ncolse(sparse * pM, int row, int col, double val)
{
	IF_SAFETY( ( pM != NULL ) && ( row >= 0 ) && ( row < pM->rows ) && ( col >= 0 ) && ( col < pM->cols ) )
		//Here pM->ncols[row] is initially 0.
		int idx = pM->ncols[row], idxp=0;
		if(val != 0 && row != col){
			pM->val[row].col[idx] = col; //Set the new non zerro element column id
			pM->val[row].val[idx] = val; //Set the new non zerro element value
			if( pM->rows == pM->cols ){   //Check if it is a square matrix
				idxp = pM->pcols[col];  //Get the number of states from which we can reach state col
				pM->val[col].back_set = (int *)realloc(pM->val[col].back_set, (idxp+1)*sizeof(double)); //increase the size of the back_set array
				pM->val[col].back_set[idxp] = row; //Add the current row to this set
				++pM->pcols[col];  //Increase the counter for the number of elements in the back_set
			}
			++pM->ncols[row]; //Increase the number of non zerro entries in the current row
		} else {
			if(val != 0 && row == col){
				pM->val[row].diag = val;  //Handle the diagonal element
			}
		}
	ELSE_SAFETY
		printf("ERROR: In set_mtx_val_ncolse, trying work with invalid parameters.\n");
		exit(1);
	ENDIF_SAFETY
}

/*****************************************************************************
name		: add_mtx_val_ncolse
role       	: add a value to an existing one in the matrix.
@param		: sparse * pM the matrix
@param     	: int row: The row number in which the value is to be added.
@param		: int col: The column number in which the value is to be added.
@param		: double val: to be added.
remark		:
******************************************************************************/
void add_mtx_val_ncolse(sparse * pM, int row, int col, double val)
{
	int ncols = pM->ncols[row], i;
	BOOL found = FALSE;
	IF_SAFETY( ( pM != NULL ) && ( row >= 0 ) && ( row < pM->rows ) && ( col >= 0 ) && ( col < pM->cols ) )
		if ( row != col ) {
			//TODO: We could be smart and make at least a binary
			//search here, as soon as all elements in pM->val[row].col
			//array are actually required to be ordered bu value
			//(increasing with the arrayindex)
			for( i=0; i < ncols; i++ )
			{
				if( pM->val[row].col[i] == col )
				{
					pM->val[row].val[i] += val;
					found = TRUE;
					break;
				}
			}
		} else {
			pM->val[row].diag += val;
			found = TRUE;
		}
		if( found == FALSE ) {
			set_mtx_val_ncolse(pM, row, col, val);
		}
	ELSE_SAFETY
		printf("ERROR: In add_val_ncolse, trying work with invalid parameters.\n");
		exit(1);
	ENDIF_SAFETY
}

/************************************************************************/
/**************SUPPLEMENTARTY METHODS USED FOR PRINTING ETC**************/
/************************************************************************/

void print_pattern_vec_double(const char * pattern, int length, double * vec)
{
	int i;
	if( vec != NULL){
		printf("( ");
		for( i = 0 ; i < length ; i++ )
		{
			printf( pattern, vec[i] );
			
			if (i != length-1) printf(", ");
		}
		printf(" )");
	}else{
		printf("NULL");
	}
}

void print_vec_double(int length, double * vec){
	print_pattern_vec_double("%1.15lf", length, vec);
}

void print_vec_int(int length, int * vec)
{
	int i;
	printf("( ");
	for( i = 0 ; i < length ; i++ )
	{
		printf( "%d", vec[i] );
		if (i != length-1) printf(", ");
	}
	printf(" )\n");
}

void print_vec_short(int length, short * vec)
{
	int i;
	if( vec != NULL){
		printf("( ");
		for( i = 0 ; i < length ; i++ )
		{
			printf( "%d", vec[i] );
			if (i != length-1) printf(", ");
		}
		printf(" )\n");
	}else{
		printf("NULL\n");
	}
}

/*****************************************************************************
name		: print_sparse
role		: print the matrix.
@param		: sparse * pM the matrix to work with
@return	: int: 0: fail, 1: success
remark		: return 0: sparse matrix is not valid.
******************************************************************************/
unsigned int print_mtx_sparse(const sparse * pM)
{
	int i,j;
	if( !pM ) return 0;
	for( i=0; i < pM->rows; i++ )
	{
		int col = pM->ncols[i];
		printf("ncols[%d]:: %d\n", i, col);
		for( j=0; j < col; j++ )
			printf("[%d][%d]=%lf\n",i,(int)pM->val[i].col[j],pM->val[i].val[j]);
	}
	for( i=0; i < pM->rows; i++)
		printf("diag:: [%d][%d]=%lf\n", i, i, pM->val[i].diag);
	if(pM->rows==pM->cols)
	{
		printf("********************* Printing Back Sets *************************************\n");
		for( i=0; i < pM->rows; i++)
		{
			printf("state: %d : BS : [ ", i+1);
			for(j=0;j<pM->pcols[i];j++)
				printf("%d, ", pM->val[i].back_set[j]+1);
			printf("]\n");
		}
		printf("******************************************************************************\n");
	}
	return 1;
}

/*****************************************************************************
name		: split_A_into_DI_LU
role		: this method splits the given A matrix into two matrixes DI and L+U
		: where DI is an inverted diagonal matrix and L+U are all the rest elements of
		  the initial matrix A.
		  NOTE: This method doesn't need the matrix to be set with the
		  set_sparse(sparse *) method it works with the patrix pA.
@param		: sparse *pA the initial matrix A
@param		: sparse *pDI the empty matrix which will be filled with the D elements
@param		: sparse *pLU the empty matrix which will be filled with the LU elements
*****************************************************************************/
void split_A_into_DI_LU(sparse* pA, sparse *pDI, sparse *pLU)
{
	int i;
	values * pD_Val = pDI->val, * pA_Val = pA->val, * pLU_Val = pLU->val;
	int * pA_Ncols = pA->ncols, * pLU_Ncols = pLU->ncols;
	for(i=0; i < pA->rows; i++)
	{
		/*Copy values to the D matrix*/
		if( pA_Val[i].diag != 0 )
			pD_Val[i].diag = 1/pA_Val[i].diag;
		
		/*Copy values to the LU matrix*/
		pLU_Ncols[i] = pA_Ncols[i];
		pLU_Val[i].col = pA_Val[i].col;
		pLU_Val[i].val = pA_Val[i].val;
	}
}

/*****************************************************************************
name		: split_A_into_D_LU
role		: this method splits the given A matrix into two matrixes DI and L+U
		: where D is a diagonal matrix and L+U are all the rest elements of
		  the initial matrix A.
		  NOTE: This method doesn't need the matrix to be set with the
		  set_sparse(sparse *) method it works with the patrix pA.
@param		: sparse *pA the initial matrix A
@param		: double *pD the empty vector which will be filled with the diagonal elements
@param		: sparse *pLU the empty matrix which will be filled with the LU elements
*****************************************************************************/
void split_A_into_D_LU(sparse* pA, double *pD, sparse *pLU)
{
	int i;
	values * pA_Val = pA->val, * pLU_Val = pLU->val;
	int * pA_Ncols = pA->ncols, * pLU_Ncols = pLU->ncols;
	for( i=0; i < pA->rows; i++ )
	{
		/*Copy values to the D vector*/
		pD[i] = pA_Val[i].diag;
		
		/*Copy values to the LU matrix*/
		pLU_Ncols[i] = pA_Ncols[i];
		pLU_Val[i].col = pA_Val[i].col;
		pLU_Val[i].val = pA_Val[i].val;
	}
}

/*****************************************************************************
name		: split_A_into_D_L_U
role		: This method splits the given A matrix into two matrixes DI and L+U
		: where D is a diagonal matrix and L+U are all the rest elements of
		  the initial matrix A.
		  NOTE: This method presumes that row elements are ordered by column id
		  and that is why it does not copy the pA elements to pL, pU but
		  simply copy pointers. Take this in mind while deleting pL and pU
		  NOTE: This method doesn't need the matrix to be set with the
		  set_sparse(sparse *) method it works with the matrix pA.
@param		: sparse *pA the initial matrix A
@param		: double *pD the empty vector which will be filled with the diagonal elements
@param		: sparse *pL the empty matrix which will be filled with the L elements
@param		: sparse *pU the empty matrix which will be filled with the U elements
*****************************************************************************/
void split_A_into_D_L_U(sparse* pA, double *pD, sparse *pL, sparse *pU)
{
	short isRFirst;
	int i, j, theNColsL, theNColsU, col;
	double val;
	
	for(i=0; i < pA->rows; i++)
	{
		/*Copy values to the pD vector*/
		pD[i] = pA->val[i].diag;
		
		/*Initialize the counter for the number of row elements of the pL and pU matrixes*/
		theNColsL = 0;
		theNColsU = 0;
		
		isRFirst = 1;
		
		/*Store possible Low diagonal values, even if there are no values
		then the number of columns will be zerro and thus this will not
		affect the entire process*/
		pL->val[i].col = pA->val[i].col;
		pL->val[i].val = pA->val[i].val;
		
		/*Copy values to the pL, pU matrixes*/
		for(j = 0; j < pA->ncols[i]; j++)
		{
			/*Get the column index of the next element in a row*/
			col = pA->val[i].col[j];
			/*Check and store element in pL or in pU*/
			if( col < i )
			{
				theNColsL++;
			}
			else
			{
				/* Case of col > i*/
				if( isRFirst )
				{
					/*Copy to the pU matrix*/
					pU->val[i].col = & pA->val[i].col[j];
					pU->val[i].val = & pA->val[i].val[j];
					isRFirst = 0;
				}
				theNColsU++;
			}
		}
		pL->ncols[i] = theNColsL;
		pU->ncols[i] = theNColsU;
	}
}

/*****************************************************************************
name		: sum_vec_vec
role		: This method is used to compute the sum of two vectors A+B=Res.
@param		: int length the length of vectors
@param		: double *pA vector A
@param		: double *pB vector B
@param		: double *pRes vector Res
*****************************************************************************/
double * sum_vec_vec(int length , double * pA, double * pB, double * pRes )
{
	int i;
	for( i = 0; i < length; i++ )
	{
		pRes[i] = pB[i] + pA[i];
	}
	return pRes;
}

/*****************************************************************************
name		: sum_vec_vec
role		: This method is used to compute the sum of two vectors A+B=Res.
@param		: double *pA vector A
@param		: double *pB vector B
@param		: double *pRes vector Res
@param		: int * pIds the valid ids, pIds[0] contains the number of ids
*****************************************************************************/
double * sum_cer_vec_vec(double * pA, double * pB, double * pRes, int * pIds )
{
	int i, length = pIds[0], id;
	for( i = 1; i <= length ; i++ )
	{
		id = pIds[i];
		pRes[ id ] = pB[ id ] + pA[ id ];
	}
	return pRes;
}

/*****************************************************************************
name		: multiply_inv_diag_D_V
role		: multiply v*diag(d)I. I.e. take vector of diagonal elements
		  create matrix diag(d) invert it and multiply by the vector v
		  from the left side.
@param		: double * pD: The diagonal elements.
@param		: double * pV: The vector to multiply with.
@param		: double * pR: The resulting vector.
@param		: int length : The length of vectors
******************************************************************************/
void multiply_inv_diag_D_V(double * pD, double * pV, double * pR, const int length)
{
	int i;
	double diag;
	for( i = 0; i < length; i++ )
	{
		diag = pD[i];
		if( diag != 0 )
		{
			pR[i] = pV[i]/diag;
		}
	}
}

/*****************************************************************************
name		: multiply_cer_inv_diag_D_V
role		: multiply v*diag(d)I. I.e. take vector of diagonal elements
		  create matrix diag(d) invert it and multiply by the vector v
		  from the left side.
@param		: double * pD: The diagonal elements.
@param		: double * pV: The vector to multiply with.
@param		: double * pR: The resulting vector.
@param		: int * pValidStates : The set of valid states
******************************************************************************/
void multiply_cer_inv_diag_D_V(double * pD, double * pV, double * pR, int * pValidStates)
{
	int i,idx;
	double diag;
	const int length = pValidStates[0];
	for( i = 1; i <= length; i++ )
	{
		idx = pValidStates[i];
		diag = pD[idx];
		if( diag != 0 )
		{
			pR[idx] = pV[idx]/diag;
		}
	}
}

/*======================================================================*/
/************************************************************************/
/*************************OBSOLETE BEGIN*********************************/
/************************************************************************/
/*======================================================================*/

/*
	THESE METHODS ARE OBSOLETE, THOUGH ARE STILL USED IN THE OLD SOURCES!
				AVOID USING THEM !!!!
	Use methods that have pM matrix as a parameter instead!
	If there is no such methods yet, then create them!
*/

static sparse *s = NULL;

/*****************************************************************************
name		: init_sparse
role		: initialise a matrix
@param		: int rows: number of rows in the matrix.
@param		: int cols: number of cols in the matrix.
@return	: the created sparse matrix
remark		: initialises a matrix for the given number of rows and columns
******************************************************************************/
sparse * init_matrix(int rows, int cols)
{
	int i;
	s = (sparse *) calloc (1, sizeof(sparse));
	s->rows = rows;
	s->cols = cols;
	s->val = (values *) calloc (s->rows,(sizeof(values)));
	s->ncols = (int *) calloc (s->rows,(sizeof(int)));
	s->pcols = (int *) calloc (s->rows,(sizeof(int))); /* NOTE:: Works only for square matrix */
	return s;
}

/*****************************************************************************
name		: init_sparse_ncolse
role		: initialise a matrix
@param		: int rows: number of rows in the matrix.
@param		: int cols: number of cols in the matrix.
@param		: int * ncolse: an awway with the number of non-zero cols in each row.
@return	: the created sparse matrix
remark 	: initialises a matrix for the given number of rows and columns
		NOTE: Matrices which were allocated with this method are faster to
		work with, at the same time they should be treated with care
		because you can not just add - reallocate rows in them
		So there is a specific set of methods to be used for this.
******************************************************************************/
sparse * init_matrix_ncolse(int rows, int cols, int *ncolse)
{
	int i, sum=0;
	int *col_val;
	double *val;
	s = (sparse *) calloc (1, sizeof(sparse));
	s->rows = rows;

	s->cols = cols;
	s->val = (values *) calloc (s->rows,(sizeof(values)));
	s->ncols = (int *) calloc (s->rows,(sizeof(int)));
	s->pcols = (int *) calloc (s->rows,(sizeof(int)));/* NOTE:: Works only for square matrix */
	
	for(i=0;i<rows;i++) sum+=ncolse[i];
	
	//Allocate the continuous arrays for all non-zero column indeces and values
	col_val = (int *)calloc(sum, sizeof(int));
	val = (double *)calloc(sum, sizeof(double));
	if(col_val==NULL || val==NULL) printf("couldn't allocate space\n");
	
	//Initialize pointers for the matrix rows
	sum=0;
	for(i=0;i<rows;i++)
	{
		s->val[i].col = &col_val[sum];
	 	s->val[i].val = &val[sum];
		sum+=ncolse[i];
	}
	return s;
}

/*****************************************************************************
name		: get_sparse
role       	: get a reference to the sparse matrix.
@return		: sparse *: pointer to the sparse matrix.
remark 	        :
******************************************************************************/
sparse * get_sparse()
{
	return s;
}

/*****************************************************************************
name		: set_sparse
role		: set a reference to the sparse matrix.
@param		: sparse * s_in
@return	:
remark 	: This method has to be called before any other operations on
		the sparse matrix.
******************************************************************************/
void set_sparse(const sparse *s_in)
{
	s = ( sparse * ) s_in;
}

/*****************************************************************************
name		: free_sparse
role		: frees the sparse matrix.
remark		:
******************************************************************************/
void free_sparse()
{
	free_mtx_sparse(s);
	s=NULL;
}

/**
* ADD a NEW value to the matrix.
* WARNING: Should NOT be used with the matrices allocated with
* init_matrix_ncolse(,,) method!
* @param     : int row: The row number in which the value is to be set.
* @param	: int col: The column number in which the value is to be set.
* @param	: double val: The value to be set.
* @return	: int: 0: fail, 1: success
* NOTE: return 0 indicates that row/col does not meet the bounds of matrix
*/
unsigned int set_val(int row, int col, double val)
{
	return set_mtx_val(s,row,col,val);
}

/*****************************************************************************
name		: get_val
role		: get a value in the matrix. Perform binary search to get a
		non-diagonal value in the matrix when the number of
		non-diagonal elements in a row is >4.
@param     	: int row: The row number in which the value is to be set.
@param		: int col: The column number in which the value is to be set.
@param		: double * val (by reference): The value.
@return		: int: 0: fail, 1: success
remark 	        : return 0: value was not found / row/col does not meet bounds.
		  SLOW 
******************************************************************************/
unsigned int get_val(int row, int col, double *val)
{
	return get_mtx_val(s, row, col, val);
}

/*****************************************************************************
name		: mult_const
role       	: multiply all elements in the matrix with a constant.
@param     	: double constant: The constant value.
******************************************************************************/
void mult_const(double constant)
{
	mult_mtx_const(s, constant);
}

/*****************************************************************************
name		: sub_diagonal
role       	: sub the elements of the input array to the diagonal one-one.
@param     	: double *diagonal: The addendum to the diagonal.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
		  size should be correct.
******************************************************************************/
unsigned int sub_diagonal(double *diagonal)
{
	return sub_mtx_diagonal(s, diagonal);
}

/*****************************************************************************
name		: add_cons_diagonal
role       	: add a constant to the all the elements of the diagonal.
@param     	: double constant: The constant value.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
******************************************************************************/
unsigned int add_cons_diagonal(double cons)
{
	return add_mtx_cons_diagonal(s, cons);
}

/*****************************************************************************
name		: set_row
role       	: set a row in the matrix.
		  NOTE: This function copies the values structure that is why change
		  of fields in the original row lateron will not affect the one which
		  was set by this function. For instance change of the diagonal element
		  in the original matrix will not affect the row added to the other matrix
		  by this method.
@param     	: int row: The row number in which the value is to be set.
@param		: int ncol: The number of valid columns (non-zero) in the given row.
@param		: values rowval: The row value to be set.
@return		: int: 0: fail, 1: success
remark 	        : return 0: indicates that row/col does not meet the bounds
		  of matrix
******************************************************************************/
unsigned int set_row(int row, int ncols, values rowval)
{
	return set_mtx_row(s, row, ncols, rowval);
}

/*****************************************************************************
name		: multiplyMV
role		: multiply a matrix with a vector.
@param		: double *vec: The operand vector.
@param		: double *res: The resulting vector.
@return	: int: 0: fail, 1: success
remark		: size should be correct.
******************************************************************************/
unsigned int multiplyMV(double *vec, double *res)
{
	multiply_mtx_MV(s, vec, res);
	return 1;
}

/*****************************************************************************
name		: multiply_cer_MV
role		: multiply certain rows of a matrix with a vector.
@param		: double *vec: The operand vector.
@param		: double *res: The resulting vector.
@param		: int num: number of valid rows.
@param		: int *valid_rows: the valid rows
@return	: int: 0: fail, 1: success
remark		: size should be correct.
******************************************************************************/
unsigned int multiply_cer_MV(double *vec, double *res, int num, int * valid_rows)
{
	multiply_mtx_cer_MV(s, vec, res, num, valid_rows);
	return 1;
}

/*****************************************************************************
name		: set_val_ncolse
role       	: set a value in the matrix.
@param     	: int row: The row number in which the value is to be set.
@param		: int col: The column number in which the value is to be set.
@param		: double val: The value to be set.
@return		: int: 0: fail, 1: success
remark 	        : return 0: indicates that row/col does not meet the bounds
		  of matrix
******************************************************************************/
unsigned int set_val_ncolse(int row, int col, double val)
{
	set_mtx_val_ncolse(s, row, col, val);
	return 1;
}

/*****************************************************************************
name		: multiplyTMV
role		: multiply a vector to a matrix.
@param		: double *vec: The operand vector.
@param		: double *res: The resulting vector.
remark		: size should be correct. CHECK FOR DIAGONALS ??
******************************************************************************/
void multiplyTMV(double *vec, double *res)
{
	multiply_mtx_TMV(s, vec, res);
}

/*****************************************************************************
name		: add_diagonal
role       	: add the elements of the input array to the diagonal one-one.
@param     	: double *diagonal: The addendum to the diagonal.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
		  size should be correct.
******************************************************************************/
unsigned int add_diagonal(double *diagonal)
{
	int i, size;
	if(!s) return 0;
	size = s->rows;
	for( i=0; i < size; i++ )
		s->val[i].diag += diagonal[i];
	return 1;
}

/*****************************************************************************
name		: sub_cons_diagonal
role       	: sub a all the elements of the diagonal from a constant.
@param     	: double constant: The constant value.
@param     	: int flag: 0: cons-diag_i, flag: 1 diag_i-cons
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
******************************************************************************/
unsigned int sub_cons_diagonal(double cons, int flag)
{
	int i, size;
	if(!s) return 0;
	size = s->rows;
	for(i=0;i<s->rows;i++)
		if(flag)
			s->val[i].diag = s->val[i].diag-cons;
		else
			s->val[i].diag = cons-s->val[i].diag;
	return 1;
}

/************************************************************************/
/**************SUPPLEMENTARTY METHODS USED FOR PRINTING ETC**************/
/************************************************************************/

/*****************************************************************************
name		: print_sparse
role       	: print the matrix.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
******************************************************************************/
unsigned int print_sparse()
{
	return print_mtx_sparse(s);
}

/*****************************************************************************
name		: read_file
role       	: reads the file puts in a new matrix. See remarks for format.
@param     	: char *filename: The name of the input file.
@return         : the matrix size (i.e. number of states)
remark 	        : Format of the file
		  no_of_states
		  (src_state dest_state value)*
******************************************************************************/
unsigned int read_file(char *filename){
	FILE *p;
	char  s[1024];
	int size, row, col;
	double val = 0.0;
	p = fopen(filename, "r");
	if(p==NULL)
	{
		printf("EROOR: Could not open .tra file \"%s\".\n",filename);
		exit(1);
	}
	
	if(fgets( s, 1024, p )!=NULL)
	{
		sscanf( s, "%d", &size );
		init_matrix(size,size);
		while (NULL != fgets( s, 1024, p ))
		{
			sscanf( s, "%d%d%lf", &row, &col, &val );
			set_val(row-1,col-1,val);
		}
	}
	(void)fclose(p);
	return size;
}

/*======================================================================*/
/**************************OBSOLETE END**********************************/
/*======================================================================*/
