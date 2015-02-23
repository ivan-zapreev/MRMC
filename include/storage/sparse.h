/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: sparse.h,v $
*	$Revision: 1.3 $
*	$Date: 2007/10/12 11:24:07 $
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
*	Source description: Manage Sparse Matrix.
*	Uses: LIB: sparse.c
*/

#include "macro.h"

#include <stdio.h>
#include <stdlib.h>
#include "bitset.h"

#ifndef SPARSE_H

#define SPARSE_H

/*****************************************************************************
			STRUCTURE
name		: values
purpose	: a row values in a sparse matrix.
@member col	: column indices of non-zero elements.
@member val	: values of non-zero elements.
@member back_set: states from which this state is reachable.
@member diag	: The value of the diagonal element in this row.
remark		: There is a one-one relation between col and val. For instance
		  col[0] indicates the column index of val[0]. This structure
		  is similar to the compressed-row storage for sparse matrices.
******************************************************************************/
typedef struct values
{
	int *col;    //the column ids
	double *val; //the corresponding values
	int *back_set;
	double diag;
}values;

/*****************************************************************************
			STRUCTURE
name			: sparse
@member rows		: column indices of non-zero elements.
@member val		: this is a vector of rows, each element is a structure,
			containing values of non-zero elements.
@member ncols	: The number of non-zero columns in a corresponding row not including
			the diagonal element.
@member pcols	: the number of elements in the back_set array of the corresponding
			values structure, in other words it represents the number of states
			from which we can reach
remark			: There is a one-one relation between col and val. For instance
			col[0] indicates the column index of val[0]. This structure
			is similar to the compressed-row storage for sparse matrices.
******************************************************************************/
typedef struct sparse
{
	int rows;
	int cols;
	int *ncols;
	int *pcols;
	values *val;
}sparse;

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
extern void initMatrix(const sparse * pStateSpace, sparse * pQ, const int * pValidStates);

/**
* This method removes all the non zero rows from the Q matrix of the BSCC
* @param pQ the BSCC nodes of the diagoriginal matrix
* @param pValidStates the valid states i.e. ids of the rows which have to be cleaned
*                     this array contains the number of nodes as the first element
*/
extern void cleanMatrix(sparse * pQ, const int * pValidStates);

/*****************************************************************************
name		: free_mtx_sparse
role       	: frees the sparse matrix.
@param		: sparse * pM the matrix to remove
remark 	        :
******************************************************************************/
extern void free_mtx_sparse(sparse *);

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
extern unsigned int set_mtx_val(sparse * pM, int row, int col ,double val);

/*****************************************************************************
name		: free_mtx_wrap
role       	: frees the sparse matrix. Does not clean rows.
@param		: sparse * pM the matrix to remove
******************************************************************************/
extern void free_mtx_wrap(sparse *);

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
extern void add_mtx_val(sparse * pM, int row, int col, double val);

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
extern BOOL get_mtx_next_val(const sparse *, const int, int *, double *, int *);

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
extern BOOL get_mtx_diag_val(const sparse *, const int, double *);

/*****************************************************************************
name		: get_mtx_val
role       	: get a value in the matrix.
@param		: sparse * pM the matrix to get element from
@param     	: int row: The row number in which the value is to be set.
@param		: int col: The column number in which the value is to be set.
@param		: double * val (by reference): The value.
@return		: int: 0: fail, 1: success
remark 	        : return 0: value was not found / row/col does not meet bounds.
******************************************************************************/
extern unsigned int get_mtx_val(const sparse * pM, const int row, const int col, double *);

/*****************************************************************************
name		: get_mtx_row_sums
role       	: add the elements in each row.
@param		: sparse * pM the matrix to work with
@return		: double *: row sums for each row.
remark 	        :
******************************************************************************/
extern double * get_mtx_row_sums(sparse * pM);

/*****************************************************************************
name		: get_mtx_cer_row_sums
role       	: add elements in certain rows and return the vector of values.
@param		: sparse * pM the matrix to work with
@param		: int * pIds the ids of the rows for which the sums should be computed
			the pIds[0] contains the number of row ids
@return		: the rows' sums
******************************************************************************/
extern double * get_mtx_cer_row_sums(sparse * pM, int * pIds);

/*****************************************************************************
name		: mult_mtx_const
role       	: multiply all elements in the matrix with a constant.
@param		: sparse * pM the matrix to multiply
@param     	: double constant: The constant value.
******************************************************************************/
extern void mult_mtx_const(sparse * pM, double constant);

/*****************************************************************************
name		: mult_mtx_cer_const
role       	: multiply all elements in the matrix with a constant.
@param		: sparse * pM the matrix to multiply
@param     	: double constant: The constant value.
@param		: int * pValidStates the valid rows to multiply pValidStates[0]
		  is the number of states (rows) in the array
******************************************************************************/
extern void mult_mtx_cer_const(sparse * pM, double constant, int * pValidStates);

/**
* Multiply all elements in all i'th rows of the matrix by a constant[i] (or 1/constant[i] ).
* @param pM the matrix to multiply
* @param constant The constant value.
* @param pValidStates the valid rows to multiply pValidStates[0]
*		  	is the number of states (rows) in the array
* @param invert if == 0 then nothing, otherwise we invert the elements of constant
*			before the multiplication.
*/
extern void mult_mtx_cer_const_array(const sparse *, const double *, const int *, const int);

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
void make_embedded_dtmc_out_of_rate_mtx(const sparse * pM, const double *pRowSums, const int * pValidStates);

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
void restore_rate_mtx_out_of_embedded_dtmc(const sparse * pM, const double *pRowSums, const int * pValidStates);

/*****************************************************************************
name		: add_mtx_diagona
role       	: add the elements of the input array to the diagonal one-one.
@param		: sparse * pM the matrix to work with
@param     	: double *diagonal: The addendum to the diagonal.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
		  size should be correct.
******************************************************************************/
extern void add_mtx_diagonal(sparse * pM, double *diagonal);

/*****************************************************************************
name		: sub_diagonal
role       	: sub the elements of the input array to the diagonal one-one.
@param		: sparse * pM the matrix to work with
@param     	: double *diagonal: The addendum to the diagonal.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
		  size should be correct.
******************************************************************************/
extern unsigned int sub_mtx_diagonal(sparse * pM, double *);

/****************************************************************************
name		: add_mtx_cer_diagonal
role		: diag elements = cons - diag elements
@param		: sparse * pM the matrix to work with
@param		: int * pIds the ids of the rows in which diagonal elements
		  should be reduced by the correcponding pE values
		  the pIds[0] contains the number of row ids
@param		: int constant: an operand (cons)
******************************************************************************/
extern void add_cer_cons_diagonal_mtx(sparse *, int *, int);

/****************************************************************************
name		: sub_mtx_cer_diagonal
role		: diag elements = cons - diag elements
@param		: sparse * pM the matrix to work with
@param		: int * pIds the ids of the rows in which diagonal elements
		  should be reduced by the correcponding pE values
		  the pIds[0] contains the number of row ids
@param		: int constant: an operand (cons)
******************************************************************************/
extern void sub_cer_cons_diagonal_mtx(sparse *pM, int * pIds, int constant);

/*****************************************************************************
name		: sub_mtx_cer_diagonal
role		: Subtract elements of the pE array from the predefined diagonal
		  elements of the matrix
@param		: sparse * pM the matrix to work with
@param		: int * pIds the ids of the rows in which diagonal elements
		  should be reduced by the correcponding pE values
		  the pIds[0] contains the number of row ids
@param		: double * pE: The elements to be subtracted
******************************************************************************/
extern void sub_mtx_cer_diagonal(sparse *, int *, double *);

/*****************************************************************************
name		: add_mtx_cons_diagonal
role       	: add a constant to the all the elements of the diagonal.
@param		: sparse * pM the matrix to work with
@param     	: double constant: The constant value.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
******************************************************************************/
extern unsigned int add_mtx_cons_diagonal(sparse * pM, double cons);

/*****************************************************************************
name		: set_row
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
extern unsigned int set_mtx_row(sparse * pM, int,int,values);

/*****************************************************************************
name		: multiply_mtx_MV
role       	: multiply a matrix with a vector.
@param		: sparse * pM: operand matrix.
@param     	: double *vec: The operand vector.
@param		: double *res: The resulting vector.
@return		: int: 0: fail, 1: success
remark 	        : size should be correct.
******************************************************************************/
extern void multiply_mtx_MV(sparse *pM, double *vec, double *res);

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
extern void multiply_mtx_cer_MV(sparse *, double *, double *, int , int *);

/*****************************************************************************
name		: multiply_mtx_TMV
role       	: multiply a vector to a matrix.
@param		: sparse * pM the matrix to be used
@param     	: double *vec: The operand vector.
@param		: double *res: The resulting vector.
@return		: double *: the resulting vector is also returned here for convinient usage
remark 	        : size should be correct.
******************************************************************************/
extern double * multiply_mtx_TMV(sparse * pM, double *, double *);

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
extern double * multiply_mtx_cer_TMV(sparse * pM, double *, double *, int * pIds );

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
name		: 	free_sparse_ncolse();
free_sparse
role       	: frees the sparse matrix.
@param		: sparse * pM the matrix to remove
remark 	        :
******************************************************************************/
extern void free_sparse_ncolse(sparse *);

/*****************************************************************************
name		: set_mtx_val_ncolse
role		: set a value in the matrix.
@param		: sparse * pM the matrix to add element to
@param		: int row: The row number in which the value is to be set.
@param		: int col: The column number in which the value is to be set.
@param		: double val: The value to be set.
@Return	: int: 0: fail, 1: success
remark		: return 0: indicates that row/col does not meet the bounds
		  of matrix
******************************************************************************/
extern void set_mtx_val_ncolse(sparse * pM, int row, int col, double val);

/*****************************************************************************
name		: add_mtx_val_ncolse
role		: add a value to an existing one in the matrix.
@param		: sparse * pM the matrix
@param		: int row: The row number in which the value is to be added.
@param		: int col: The column number in which the value is to be added.
@param		: double val: to be added.
remark		:
******************************************************************************/
extern void add_mtx_val_ncolse(sparse * pM, int row, int col, double val);

/************************************************************************/
/**************SUPPLEMENTARTY METHODS USED FOR PRINTING ETC**************/
/************************************************************************/

extern void print_pattern_vec_double(const char * pattern, int length, double * vec);

extern void print_vec_double(int length, double * vec);

extern void print_vec_int(int length, int * vec);

extern void print_vec_short(int length, short * vec);

/*****************************************************************************
name		: print_sparse
role       	: print the matrix.
@param		: sparse * pM the matrix to work with
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
******************************************************************************/
extern unsigned int print_mtx_sparse(const sparse * pM);

/*****************************************************************************
name		: split_A_into_DI_LU
role		: this method splits the given A matrix into two matrixes DI and L+U
		: where DI is an inverted diagonal matrix and L+U are all the rest elements of
		  the initial matrix A.
		  NOTE: This method doesn't need the matrix to be set with the
		  set_sparse(sparse *) method it works with the patrix pA.
@param 		: sparse *pA the initial matrix A
@param 		: sparse *pDI the empty matrix which will be filled with the D elements
@param 		: sparse *pLU the empty matrix which will be filled with the LU elements
*****************************************************************************/
extern void split_A_into_DI_LU(sparse* pA, sparse *pDI, sparse *pLU);

/*****************************************************************************
name		: split_A_into_D_LU
role		: this method splits the given A matrix into two matrixes DI and L+U
		: where D is a diagonal matrix and L+U are all the rest elements of
		  the initial matrix A.
		  NOTE: This method doesn't need the matrix to be set with the
		  set_sparse(sparse *) method it works with the patrix pA.
@param 		: sparse *pA the initial matrix A
@param 		: double *pD the empty vector which will be filled with the diagonal elements
@param 		: sparse *pLU the empty matrix which will be filled with the LU elements
*****************************************************************************/
extern void split_A_into_D_LU(sparse* pA, double *pD, sparse *pLU);

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
@param 		: sparse *pA the initial matrix A
@param 		: double *pD the empty vector which will be filled with the diagonal elements
@param 		: sparse *pL the empty matrix which will be filled with the L elements
@param 		: sparse *pU the empty matrix which will be filled with the U elements
*****************************************************************************/
extern void split_A_into_D_L_U(sparse* pA, double *pD, sparse *pL, sparse *pU);

/*****************************************************************************
name		: sum_vec_vec
role		: This method is used to compute the sum of two vectors A+B=Res.
@param		: int length the length of vectors
@param 		: double *pA vector A
@param 		: double *pB vector B
@param 		: double *pRes vector Res
*****************************************************************************/
extern double * sum_vec_vec(int length , double * pA, double * pB, double * pRes );

/*****************************************************************************
name		: sum_cer_vec_vec
role		: This method is used to compute the sum of two vectors A+B=Res.
@param 		: double *pA vector A
@param 		: double *pB vector B
@param 		: double *pRes vector Res
@param		: int * pIds the valid ids, pIds[0] contains the number of ids
*****************************************************************************/
extern double * sum_cer_vec_vec(double * pA, double * pB, double * pRes, int * pIds );

/*****************************************************************************
name		: multiply_inv_diag_D_V
role       	: multiply v*diag(d)I. I.e. take vector of diagonal elements
		  create matrix diag(d) invert it and multiply by the vector v
		  from the left side.
@param     	: double * pD: The diagonal elements.
@param     	: double * pV: The vector to multiply with.
@param     	: double * pR: The resulting vector.
@param     	: int length : The length of vectors
******************************************************************************/
extern void multiply_inv_diag_D_V(double * pD, double * pV, double * pR, const int length);

/*****************************************************************************
name		: multiply_cer_inv_diag_D_V
role       	: multiply v*diag(d)I. I.e. take vector of diagonal elements
		  create matrix diag(d) invert it and multiply by the vector v
		  from the left side.
@param     	: double * pD: The diagonal elements.
@param     	: double * pV: The vector to multiply with.
@param     	: double * pR: The resulting vector.
@param     	: int * pValidStates : The set of valid states
******************************************************************************/
extern void multiply_cer_inv_diag_D_V(double * pD, double * pV, double * pR, int * pValidStates);

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

/*****************************************************************************
name		: get_sparse
role       	: get a reference to the sparse matrix.
@return		: sparse *: pointer to the sparse matrix.
remark 	        :
******************************************************************************/
extern sparse * get_sparse();

/*****************************************************************************
name		: set_sparse
role       	: set a reference to the sparse matrix.
@param     	: sparse * s_in
@return		:
remark 	        : This method has to be called before any other operations on
		  the sparse matrix.
******************************************************************************/
extern void set_sparse( const sparse *);

/*****************************************************************************
name		: free_sparse
role       	: frees the sparse matrix.
remark 	        :
******************************************************************************/
extern void free_sparse();

/*****************************************************************************
name		: init_sparse
role       	: initialise a matrix
@param		: int rows: number of rows in the matrix.
@param		: int cols: number of cols in the matrix.
@return		: the created sparse matrix
remark 	        : initialises a matrix for the given number of rows and columns
******************************************************************************/
extern sparse * init_matrix(int,int);

/*****************************************************************************
name		: init_sparse_ncolse
role       	: initialise a matrix
@param		: int rows: number of rows in the matrix.
@param		: int cols: number of cols in the matrix.
@return		: the created sparse matrix
remark 	        : initialises a matrix for the given number of rows and columns
******************************************************************************/
extern sparse * init_matrix_ncolse(int,int,int *);

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
extern unsigned int set_val(int,int,double);

/*****************************************************************************
name		: set_val_ncolse
role		: set a value in the matrix.
@param		: int row: The row number in which the value is to be set.
@param		: int col: The column number in which the value is to be set.
@param		: double val: The value to be set.
@Return	: int: 0: fail, 1: success
remark		: return 0: indicates that row/col does not meet the bounds
		  of matrix
******************************************************************************/
extern unsigned int set_val_ncolse(int row, int col ,double val);

/*****************************************************************************
name		: get_val
role       	: get a value in the matrix.
@param     	: int row: The row number in which the value is to be set.
@param		: int col: The column number in which the value is to be set.
@param		: double * val (by reference): The value.
@return		: int: 0: fail, 1: success
remark 	        : return 0: value was not found / row/col does not meet bounds.
******************************************************************************/
extern unsigned int get_val(int, int, double *);

/*****************************************************************************
name		: mult_const
role       	: multiply all elements in the matrix with a constant.
@param		: sparse * pM the matrix to work with
@param     	: double constant: The constant value.
*********************************************************************/
extern void mult_const(double);

/*****************************************************************************
name		: add_diagonal
role       	: add the elements of the input array to the diagonal one-one.
@param     	: double *diagonal: The addendum to the diagonal.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is m not valid.
		  size should be correct.
******************************************************************************/
extern unsigned int add_diagonal(double *);

/*****************************************************************************
name		: sub_diagonal
role       	: sub the elements of the input array to the diagonal one-one.
@param     	: double *diagonal: The addendum to the diagonal.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
		  size should be correct.
******************************************************************************/
extern unsigned int sub_diagonal(double *);

/*****************************************************************************
name		: add_cons_diagonal
role       	: add a constant to the all the elements of the diagonal.
@param     	: double constant: The constant value.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
******************************************************************************/
extern unsigned int add_cons_diagonal(double);

/*****************************************************************************
name		: sub_cons_diagonal
role       	: sub a all the elements of the diagonal from a constant.
@param     	: double constant: The constant value.
@param     	: int flag: 0: cons-diag_i, flag: 1 diag_i-cons
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
******************************************************************************/
extern unsigned int sub_cons_diagonal(double, int);

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
extern unsigned int set_row(int,int,values);

/*****************************************************************************
name		: multiplyMV
role       	: multiply a matrix with a vector.
@param     	: double *vec: The operand vector.
@param		: double *res: The resulting vector.
@return		: int: 0: fail, 1: success
remark 	        : size should be correct.
******************************************************************************/
extern unsigned int multiplyMV(double *, double *);

/*****************************************************************************
name		: multiply_cer_MV
role       	: multiply certain rows of a matrix with a vector.
@param     	: double *vec: The operand vector.
@param		: double *res: The resulting vector.
@param		: int num: number of valid rows.
@param		: int *valid_rows: the valid rows
@return		: int: 0: fail, 1: success
remark 	        : size should be correct.
******************************************************************************/
extern unsigned int multiply_cer_MV(double *, double *, int, int *);

/*****************************************************************************
name		: multiplyTMV
role       	: multiply a vector to a matrix.
@param     	: double *vec: The operand vector.
@param		: double *res: The resulting vector.
remark 	        : size should be correct.
******************************************************************************/
extern void multiplyTMV(double *, double *);

/*****************************************************************************
name		: mult_const
role       	: multiply all elements in the matrix with a constant.
@param     	: double constant: The constant value.
******************************************************************************/
extern void mult_const(double constant);

/************************************************************************/
/**************SUPPLEMENTARTY METHODS USED FOR PRINTING ETC**************/
/************************************************************************/

/*****************************************************************************
name		: print_sparse
role       	: print the matrix.
@return		: int: 0: fail, 1: success
remark 	        : return 0: sparse matrix is not valid.
******************************************************************************/
extern unsigned int print_sparse();

/*****************************************************************************
name		: read_file
role       	: reads the file puts in a new matrix. See remarks for format.
@param     	: char *filename: The name of the input file.
@return		: int: 0: fail, 1: success
remark 	        : Format of the file
		  no_of_states
		  (src_state dest_state value)*
******************************************************************************/
extern unsigned int read_file(char * filename);

/*======================================================================*/
/**************************OBSOLETE END**********************************/
/*======================================================================*/
#endif

