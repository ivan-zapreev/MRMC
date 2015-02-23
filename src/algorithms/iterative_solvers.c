/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: iterative_solvers.c,v $
*	$Revision: 1.13 $
*	$Date: 2007/10/11 16:15:27 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Maneesh Khattri, Ivan Zapreev, Christina Jansen
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
*	Source description: This file contains functions' for the iterative solving
*	   of the systems of linear equations.
*/

#include "iterative_solvers.h"

/**
* This method normalizes solution i.e. makes the sum of its elements to be 1
* @param length the length of vector pSolution if pValidStates == NULL
* @param pSolution the solution vector
* @param pValidStates the valid states of the solution,
*                     pValidStates[0] == the number of valid states
*                     pValidStates == NULL if all states are valid
* @param in_any_case if TRUE then the normalization is performed even if sum < 1
*/
void normalizeSolution( const int length, double * pSolution,
			int * pValidStates, BOOL in_any_case )
{
	int i;
	double sum = 0;
	if( pValidStates )
	{
		/*If not all states are valid*/
		int size = pValidStates[0];
		for( i = 1; i <= size ; i++ )
		{
			sum += pSolution[ pValidStates[i] ];
		}
		if( ( sum > 0 ) && (in_any_case || sum > 1) )
		{
			/*If we can make a solution smaller*/
			for( i = 1; i <= size ; i++ )
			{
				pSolution[ pValidStates[i] ] /= sum;
			}
		}
	}
	else
	{
		/*If all the states are valid*/
		for( i = 0; i < length ; i++ )
		{
			sum += pSolution[i];
		}
		if( ( sum > 0 ) && (in_any_case || sum > 1) )
		{
			/*If we can make a solution smaller*/
			for( i = 0; i < length ; i++ )
			{
				pSolution[i] /= sum;
			}
		}
	}
}

/**
* This method checks the convergence of solution
* @param pX1 the first vector
* @param pX2 the second vector
* @param err the allowed difference between corresponding vector elements
* @param max_iterations the max number of iterations
* @param pValidStates this array contains the number of nodes as the first
*		element
*        all the other elements are the node ids, if it is NULL then
*        all the nodes from the A matrix are valid
* @return TRUE if vectors are close enough otherwise FALSE;
*/
static BOOL stopGJIC(double * pX1, double * pX2, double err, int * pValidStates)
{
	BOOL result = TRUE;
	int i, id;
	const int size = pValidStates[0];
	for( i = 1; i <= size; i++ )
	{
		id = pValidStates[i];
		if( fabs( pX1[ id ]- pX2[ id ] ) > err )
		{
			result = FALSE;
			break;
		}
	}
	return result;
}

/**
* This method checks the convergence of solution
* @param size the length of vectors
* @param pX1 the first vector
* @param pX2 the second vector
* @param err the allowed difference between corresponding vector elements
* @param max_iterations the max number of iterations
* @return TRUE if vectors are close enough otherwise FALSE;
*/
static BOOL stopGJI(const int size, double * pX1, double * pX2, double err)
{
	BOOL result = TRUE;
	int i;
	for( i = 0; i < size; i++ )
	{
		if( fabs( pX1[ i ]- pX2[ i ] ) > err )
		{
			result = FALSE;
			break;
		}
	}
	return result;
}

/**
* If the pB vector is non zero result=DI(b+(-LU)x), and pValidStates are available
*/
static double * solveGaussJacobi_V_B(sparse *pDI, sparse *pLU, double *pIntermediate, double *pX, double *pB, double err,
					int max_iterations, int *pValidStates, int *validStates, const int N_STATES)
{
	double *pResult = (double *) calloc(N_STATES,sizeof(double));
	int i=0;
	double *pTmp;
	
	//Copy the initial vector into the results.
	//pX is initialized to 1 for states that already fulfill the unbounded until formula or reach only 
	//states which fulfill the formula with probability 1, therefore no further probability has to be calculated
	//and for this states result = initial value
	//analog for states which reach only states fulfilling the formula with probability 0
	memcpy(pResult, pX, sizeof(double) * N_STATES);
	
	while(TRUE)
	{
		i++;
		/* intermediate = (-LU)*x */
		multiply_mtx_cer_MV(pLU,pX,pIntermediate,*pValidStates,validStates);
		
		/* intermediate = b + intermediate */
		sum_vec_vec(N_STATES,pB,pIntermediate,pIntermediate );
		
		/* result = DI * intermediate */
		multiply_mtx_cer_MV(pDI,pIntermediate,pResult,*pValidStates,validStates );
		
		/* Stop if we need or can(b+(-LU)x) */
		if( stopGJIC( pResult, pX, err, pValidStates ) || ( i > max_iterations ) )
			break;
		
		/* Switch values between pX and pResult */
		pTmp = pX;
		pX = pResult;
		pResult = pTmp;
	}
	free(pX);
	
	printf("Gauss Jacobi V_B: The number of Gauss-Jacobi iterations %d\n",i);
	if( i > max_iterations ) printf("ERROR: %s\n", METHOD_DIVERGENCE_MSG);
	
	return pResult;
}

/**
* If the pB vector is zero result=DI((-LU)x), and pValidStates are available
*/
static double * solveGaussJacobi_V_0(sparse *pDI, sparse *pLU, double * pIntermediate, double * pX, double err,
					int max_iterations, int *pValidStates, int *validStates, const int N_STATES)
{
	double *pResult = (double *) calloc( N_STATES,sizeof(double));
	int i=0;
	double *pTmp;
	
	//Copy the initial vector into the results.
	//pX is initialized to 1 for states that already fulfill the unbounded until formula or reach only 
	//states which fulfill the formula with probability 1, therefore no further probability has to be calculated
	//and for this states result = initial value
	//analog for states which reach only states fulfilling the formula with probability 0
	memcpy(pResult, pX, sizeof(double) * N_STATES);
	
	while(TRUE)
	{
		i++;
		/* intermediate = (-LU)*x */
		multiply_mtx_cer_MV( pLU, pX, pIntermediate, *pValidStates, validStates );
		
		/* result =  DI * intermediate */
		multiply_mtx_cer_MV( pDI, pIntermediate, pResult, *pValidStates, validStates );
		
		/* Stop if we need or can */
		if( stopGJIC( pResult, pX, err, pValidStates) || ( i > max_iterations ) )
			break;
		
		/* Switch values between pX and pResult */
		pTmp = pX;
		pX = pResult;
		pResult = pTmp;
	}
	free(pX);
	
	printf("Gauss Jacobi V_0: The number of Gauss-Jacobi iterations %d\n",i);
	if( i > max_iterations ) printf("ERROR: %s\n", METHOD_DIVERGENCE_MSG);
	
	return pResult;
}

/**
* Solves the system of linear equations Ax=b using the Gauss-Jacobi method
* @param pA the A matrix
* @param pX the initial x vector
*           NOTE: Is possibly freed inside by solveGaussJacobi method
* @param pB the b vector
* @param err the difference between two successive x vector lavues which 
*	     indicates when we can stop iterations
* @param max_iterations the max number of iterations
* @param pValidStates this array contains the number of nodes as the first
*         element all the other elements are the node ids, if it is NULL then
*                     all the nodes from the A matrix are valid
* @return the solution of the system
*/
static double * solveGaussJacobi(sparse * pA, double * pX, double * pB,
				  double err, int max_iterations, int * pValidStates)
{
	sparse *pDI, *pLU;
	int *validStates = pValidStates;
	const int N_STATES = pA->rows;
	double *pResult = NULL;
	double *pIntermediate=(double*)calloc(N_STATES,sizeof(double));
	char c;
	++validStates;
	/* Init the pDI matrix */
	pDI = init_matrix( N_STATES, pA->cols );
	/* Init the pLU matrix */
	pLU = init_matrix( N_STATES, pA->cols );
	/* Get D inverted and L+U */
	split_A_into_DI_LU( pA, pDI, pLU );
	
	/* Turn LU into -LU */
	mult_mtx_const( pLU, -1.0 );
	
	/*Solve system iteratively*/
	if( pValidStates )
	{
		if( pB )
		{
			/* If there is a non zero pB vector result=DI(b+(-LU)x) */
			pResult = solveGaussJacobi_V_B(pDI, pLU, pIntermediate, pX, pB, err,
							max_iterations, pValidStates, validStates, N_STATES);	
		}
		else
		{
			/* If the pB vector is zero result=DI((-LU)x) */
			pResult = solveGaussJacobi_V_0(pDI, pLU, pIntermediate, pX, err, max_iterations,
						       pValidStates, validStates, N_STATES);
		}
	}
	else
	{
		if( pB )
		{
			/*ToDo: Implement*/
			printf("ERROR: solveGaussJacobi_NULL_B is not implemented.\n");
		}
		else
		{
			/*ToDo: Implement*/
			printf("ERROR: solveGaussJacobi_NULL_0 is not implemented.\n");
		}
	}
	
	/* Turn -LU into LU */
	mult_mtx_const( pLU, -1.0 );
	
	free(pIntermediate);
	free_mtx_wrap(pDI);
	free_mtx_wrap(pLU);
	
	return pResult;
}

/**
* Multiply nRow row of pL by the elem vector and add result to the pLUx[nRow]
*/
static multiplyLRowByVecAndAddToLUx(const int nRow, sparse *pL, double *elem, double * pLUx)
{
	const int LENGTH = pL->ncols[nRow];
	int i;
	double result = 0;
	int *pColRef = pL->val[nRow].col;
	double *pValRef = pL->val[nRow].val;
	for( i = 0; i < LENGTH; i++)
	{
		result += pValRef[i] * elem[pColRef[i]];
	}
	pLUx[nRow] += result;
}

/**
* This function is used to order an array of valid states
* @param pValidStates this array contains the number of nodes as the first element
*                     all the other elements are the node ids, if it is NULL then
*                     all the nodes from the A matrix are valid
*/
static int compare_int(const void *e1, const void *e2 )
{
	return *((int*)e1) - * ((int *)e2);
}

static orderValidStates(int *pValidStates)
{
	qsort(pValidStates+1, pValidStates[0], sizeof(int), compare_int);
}

/**
* If the pB vector is non zero result=(b+(-LU)x))DI, and pValidStates are available
*/
static void solveGaussSeidel_V_B(sparse *pL, sparse *pU, double *pD,
			double * pX, double * pB, double err, int max_iterations,
			int * pValidStates, double * pLUx, int * validStates)
{
	int i=0, j=0, cur_id;
	double tmp;
	BOOL endOfIterations;
	const int LENGTH = pValidStates[0];
	
	while(TRUE)
	{
		i++;
		
		/*If no state is valid, stop computation, initial vector is result*/
		if( pValidStates[0] == 0 ) break;
		
		/*Compute the non changeable part of each iteration*/
		multiply_mtx_cer_MV( pU, pX, pLUx, *pValidStates, validStates );
		endOfIterations = TRUE;
		
		/*Start computing the Xi+1 solution element by element
		Perform the first iteration here as far as it is simpler then others*/
		cur_id = pValidStates[1];
		if( cur_id != 0 ) multiplyLRowByVecAndAddToLUx(cur_id, pL, pX, pLUx);
		/*xi+1 = (b+(-LU)x)DI*/
		tmp = (pB[cur_id] - pLUx[cur_id]) / pD[cur_id];
		endOfIterations = endOfIterations && fabs( tmp - pX[ cur_id ] ) <= err;
		pX[cur_id] = tmp;
		/*Perform all the rest iterations*/
		for( j = 2; j <= LENGTH; j++)
		{
			cur_id = pValidStates[j];
			multiplyLRowByVecAndAddToLUx(cur_id, pL, pX, pLUx);
			/*xi+1 = (b+(-LU)x)DI*/
			tmp = (pB[cur_id] - pLUx[cur_id]) / pD[cur_id];
			endOfIterations = endOfIterations && fabs( tmp - pX[ cur_id ] ) <= err ;
			//printf("new = %1.15le - old = %1.15le == %1.15le\n",tmp,pX[ cur_id ],tmp - pX[ cur_id ]);
			pX[cur_id] = tmp;
		}
		/*Stop if we need or can*/
		if( endOfIterations || ( i > max_iterations ) ) break;
	}
	
	printf("Gauss Seidel V_B: The number of Gauss-Seidel iterations %d\n",i);
	if( i > max_iterations ) printf("ERROR: %s\n", METHOD_DIVERGENCE_MSG);
}

/**
* If the pB vector is zero result=((-LU)x)DI, and pValidStates are available
*/
static void solveGaussSeidel_V_0(sparse *pL, sparse *pU, double *pD,
			double * pX, double err, int max_iterations,
			int * pValidStates, double * pLUx, int * validStates)
{
	int i=0, j=0, cur_id;
	double tmp;
	BOOL endOfIterations;
	const int LENGTH = pValidStates[0];
	
	while(TRUE)
	{
		i++;
	
		/*If no state is valid, stop computation, initial vector is result*/
		if( pValidStates[0] == 0 ) break;
	
		/*Compute the non changeable part of each iteration*/
		multiply_mtx_cer_MV( pU, pX, pLUx, *pValidStates, validStates );
		endOfIterations = TRUE;
		
		/*Start computing the Xi+1 solution element by element
		Perform the first iteration here as far as it is simpler then others*/
		cur_id = pValidStates[1];
		if( cur_id != 0 ) multiplyLRowByVecAndAddToLUx(cur_id, pL, pX, pLUx);
		/*xi+1 = ((-LU)x)DI*/
		tmp =  - pLUx[cur_id] / pD[cur_id];
		endOfIterations = endOfIterations && fabs( tmp - pX[ cur_id ] ) <= err;
		//printf("new = %1.15le - old = %1.15le == %1.15le\n",tmp,pX[ cur_id ],tmp - pX[ cur_id ]);
		pX[cur_id] = tmp;
		/*Perform all the rest iterations*/
		for( j = 2; j <= LENGTH; j++)
		{
			cur_id = pValidStates[j];
			multiplyLRowByVecAndAddToLUx(cur_id, pL, pX, pLUx);
			/*xi+1 = ((-LU)x)DI*/
			tmp =  - pLUx[cur_id] / pD[cur_id];
			endOfIterations = endOfIterations && fabs( tmp - pX[ cur_id ] ) <= err ;
			//printf("new = %1.15le - old = %1.15le == %1.15le\n",tmp,pX[ cur_id ],tmp - pX[ cur_id ]);
			pX[cur_id] = tmp;
		}
		/*Stop if we need or can*/
		if( endOfIterations || ( i > max_iterations ) ) break;
	}
	
	printf("Gauss Seidel V_0: The number of Gauss-Seidel iterations %d\n",i);
	if( i > max_iterations ) printf("ERROR: %s\n", METHOD_DIVERGENCE_MSG);
}

/**
* Solves the system of linear equations Ax=b using the Gauss-Seidel method
* @param pA the A matrix
* @param pX the initial x vector
* @param pB the b vector
* @param err the difference between two successive x vector lavues which indicates
*	     when we can stop iterations
* @param max_iterations the max number of iterations
* @param pValidStates this array contains the number of nodes as the first element
*                     all the other elements are the node ids, if it is NULL then
*                     all the nodes from the A matrix are valid
* @return the solution of the system
*/
static double * solveGaussSeidel(sparse * pA, double * pX, double * pB,
			  double err, int max_iterations, int * pValidStates)
{
	int N_STATES = pA->rows;
	int *validStates = pValidStates;
	sparse *pL, *pU;
	double *pD = (double*)calloc(N_STATES,sizeof(double));
	double * pLUx = (double *) calloc( N_STATES, sizeof(double) );
	
	/* Init the pL matrix */
	pL = init_matrix( N_STATES, pA->cols );
	/* Init the pU matrix */
	pU = init_matrix( N_STATES, pA->cols );
	
	/*Get D, L and U, A = L + diag(D) + U*/
	split_A_into_D_L_U( pA, pD, pL, pU );
	
	++validStates;
	
	/*solve system iteratively*/
	if( pValidStates )
	{
		/*NOTE: The pValidStates states should be ordered, otherwise we will get a wrong solution
		I.e. for example instead of {5,9,3} there should be {3,5,9}*/
		orderValidStates(pValidStates);
		if( pB )
		{
			/*If the pB vector is non zero result=(b+(-LU)x)DI*/
			solveGaussSeidel_V_B(pL, pU, pD, pX, pB, err, max_iterations, 
						pValidStates, pLUx, validStates);
		}
		else
		{
			/*If the pB vector is zero result=(-LU)xDI*/
			solveGaussSeidel_V_0(pL, pU, pD, pX, err, max_iterations, pValidStates, 
						pLUx, validStates);
		}
	}
	else
	{
		if( pB )
		{
			/*If the pB vector is non zero result=(b+(-LU)x)DI*/
			printf("ERROR: solveGaussSeidel_NULL_B is not implemented.\n");
		}
		else
		{
			/*If the pB vector is zero result=(-LU)xDI*/
			printf("ERROR: solveGaussSeidel_NULL_0 is not implemented.\n");
		}
	}
	
	free(pD);
	free(pLUx);
	free_mtx_wrap(pL);
	free_mtx_wrap(pU);
	
	/*Return the result as it is modified internaly and finaly contains the resulting solution*/
	return pX;
}

/**
* If there is a non zero pB vector result=(b+x(-LU))DI, and pValidStates are available
*/
static double *solveGaussJacobiInverted_V_B(sparse *pLU, double *pD, double * pIntermediate,
					   double * pX, double * pB, double err, int max_iterations,
					   int * pValidStates, const int N_STATES)
{
	double * pResult = (double *) calloc( N_STATES, sizeof(double) );
	int i=0;
	double * pTmp;
	while( TRUE )
	{
		i++;
		/*intermediate = x*(-LU)*/
		multiply_mtx_cer_TMV( pLU, pX, pIntermediate, pValidStates );
		/*intermediate = b + intermediate*/
		sum_cer_vec_vec( pB, pIntermediate, pIntermediate, pValidStates );
		/*result = intermediate * DI*/
		multiply_cer_inv_diag_D_V( pD, pIntermediate, pResult, pValidStates );
		/*Stop if we need or can*/
		if(stopGJIC(pResult,pX,err,pValidStates)||(i>max_iterations))break;
		
		/*Improve the convergence
		if ( i % 10 == 0) normalizeSolution( 0, pResult, pValidStates, FALSE );
		
		Switch values between pX and pResult*/
		pTmp = pX;
		pX = pResult;
		pResult = pTmp;
	}
	free(pX);
	
	printf("Gauss Jacobi Inverted V_B: The number of Gauss-Jacobi iterations %d\n",i);
	if( i > max_iterations ) printf("ERROR: %s\n", METHOD_DIVERGENCE_MSG);
	
	return pResult;
}

/**
* If the pB vector is zero result=x(-LU)DI, and pValidStates are available
*/
static double *solveGaussJacobiInverted_V_0(sparse *pLU, double *pD,  double * pIntermediate,
					   double * pX, double err, int max_iterations,
					   int * pValidStates, const int N_STATES)
{
	double * pResult = (double *) calloc( N_STATES, sizeof(double) );
	int i=0;
	double * pTmp;
	while(TRUE)
	{
		i++;
		/*intermediate = x*(-LU)*/
		multiply_mtx_cer_TMV( pLU, pX, pIntermediate, pValidStates );
		/*result = intermediate * DI*/
		multiply_cer_inv_diag_D_V( pD, pIntermediate, pResult, pValidStates );
		/*Stop if we need or can*/
		if(stopGJIC(pResult,pX,err,pValidStates)||(i>max_iterations))break;
		/*Improve the convergence
		if ( i % 10 == 0) normalizeSolution( 0, pResult, pValidStates, FALSE );
		Switch values between pX and pResult */
		pTmp = pX;
		pX = pResult;
		pResult = pTmp;
	}
	free(pX);
	
	printf("Gauss Jacobi Inverted V_0: The number of Gauss-Jacobi iterations %d\n",i);
	if( i > max_iterations ) printf("ERROR: %s\n", METHOD_DIVERGENCE_MSG);
	
	return pResult;
}

/**
* If there is a non zero pB vector result=(b+x(-LU))DI and pValidStates are unavailable
*/
static double *solveGaussJacobiInverted_NULL_B(sparse *pLU, double *pD, double * pIntermediate,
					      double * pX, double * pB, double err, int max_iterations,
					      const int N_STATES)
{
	double * pResult = (double *) calloc( N_STATES, sizeof(double) );
	int i=0;
	double * pTmp;
	while( TRUE )
	{
		i++;
		/*intermediate = x*(-LU)*/
		multiply_mtx_TMV( pLU, pX, pIntermediate);
		/*intermediate = b + intermediate*/
		sum_vec_vec(N_STATES, pB, pIntermediate, pIntermediate);
		/*result = intermediate * DI*/
		multiply_inv_diag_D_V( pD, pIntermediate, pResult, N_STATES );
		/*Stop if we need or can*/
		if( stopGJI( N_STATES, pResult, pX, err ) || ( i > max_iterations ) ) break;
		/*Improve the convergence
		if ( i % 10 == 0) normalizeSolution( N_STATES, pResult, NULL, FALSE );
		Switch values between pX and pResult*/
		pTmp = pX;
		pX = pResult;
		pResult = pTmp;
	}
	free(pX);
	
	printf("Gauss Jacobi Inverted NULL_B: The number of Gauss-Jacobi iterations %d\n",i);
	if( i > max_iterations ) printf("ERROR: %s\n", METHOD_DIVERGENCE_MSG);
	
	return pResult;
}

/**
* If the pB vector is zero result=x(-LU)DI and pValidStates are unavailable
*/
static double * solveGaussJacobiInverted_NULL_0(sparse *pLU, double *pD, double * pIntermediate,
					      double * pX, double err, int max_iterations,
					      const int N_STATES)
{
	double * pResult = (double *) calloc( N_STATES, sizeof(double) );
	int i=0;
	double * pTmp;
	while( TRUE )
	{
		i++;
		/*intermediate = x*(-LU)*/
		multiply_mtx_TMV( pLU, pX, pIntermediate );
		/*result = intermediate * DI*/
		multiply_inv_diag_D_V( pD, pIntermediate, pResult, N_STATES );
		/*Stop if we need or can*/
		if( stopGJI( N_STATES, pResult, pX, err ) || ( i > max_iterations )) break;
		/*Improve the convergence
		if ( i % 10 == 0) normalizeSolution( N_STATES, pResult, NULL, FALSE );
		Switch values between pX and pResult*/
		pTmp = pX;
		pX = pResult;
		pResult = pTmp;
	}
	free(pX);
	
	printf("Gauss Jacobi Inverted NULL_0: The number of Gauss-Jacobi iterations %d\n",i);
	if( i > max_iterations ) printf("ERROR: %s\n", METHOD_DIVERGENCE_MSG);
	
	return pResult;
}

/**
* Solves the system of linear equations xA=b using the Inverted Gauss-Jacobi method
* @param pA the A matrix
* @param pX the initial x vector
* @param pB the b vector
* @param err the difference between two successive x vector lavues which indicates
*	     when we can stop iterations
* @param max_iterations the max number of iterations
* @param pValidStates this array contains the number of nodes as the first element
*                     all the other elements are the node ids, if it is NULL then
*                     all the nodes from the A matrix are valid
* @return the solution of the system
*/
static double * solveGaussJacobiInverted(sparse * pA, double * pX, double * pB,
				  double err, int max_iterations, int * pValidStates)
{
	const int N_STATES = pA->rows;
	sparse *pLU;
	double *pD = (double *) calloc( N_STATES, sizeof(double) );
	double * pResult = NULL;
	double * pIntermediate = (double *) calloc( N_STATES, sizeof(double) );
	
	/*Init the pLU matrix*/
	pLU = init_matrix( N_STATES, pA->cols );
	/*Get D and L+U, A = L + diag(D) + U*/
	split_A_into_D_LU( pA, pD, pLU );
	/*Solve system iteratively*/
	if( pValidStates )
	{
		/*If not all states are valid
		Turn LU into -LU*/
		mult_mtx_cer_const( pLU, -1.0, pValidStates );
		if( pB )
		{
			/*If there is a non zero pB vector result=(b+x(-LU))DI*/
			pResult = solveGaussJacobiInverted_V_B(pLU, pD, pIntermediate,
						     pX, pB, err, max_iterations, pValidStates, N_STATES);
		}
		else
		{
			/*If the pB vector is zero result=x(-LU)DI*/
			pResult = solveGaussJacobiInverted_V_0(pLU, pD, pIntermediate,
						     pX, err, max_iterations, pValidStates, N_STATES);
		}
		/*Turn -LU into LU*/
		mult_mtx_cer_const( pLU, -1.0, pValidStates );
	}
	else
	{
		/*If all states are valid
		Turn LU into -LU*/
		mult_mtx_const( pLU, -1.0 );
		if( pB )
		{
			/*If there is a non zero pB vector result=(b+x(-LU))DI*/
			pResult = solveGaussJacobiInverted_NULL_B(pLU, pD, pIntermediate,
							pX, pB, err, max_iterations, N_STATES);
		}
		else
		{
			/*If the pB vector is zero result=x(-LU)DI*/
			pResult = solveGaussJacobiInverted_NULL_0(pLU, pD, pIntermediate,
							pX, err, max_iterations, N_STATES);
		}
		/*Turn -LU into LU*/
		mult_mtx_const( pLU, -1.0 );
	}
	
	free(pIntermediate);
	free(pD);
	free_mtx_wrap(pLU);
	
	return pResult;
}

/*
* Multiply nRow row of pU by the elem value and add resulting vector to the pLUx
*/
static multiplyUrowByConstAndAddToLUx(const int nRow, sparse *pU, double elem, double * pLUx)
{
	const int LENGTH = pU->ncols[nRow];
	int i, *pColRef = pU->val[nRow].col;
	double *pValRef = pU->val[nRow].val;
	for( i = 0 ; i < LENGTH; i++)
	{
		pLUx[ pColRef[i] ] += elem * pValRef[i];
	}
}

/**
* If the pB vector is zero result=x(-LU)DI, and pValidStates are available
* NOTE: The pValidStates states should be ordered, otherwise we will get a wrong solution
*       I.e. for example instead of {5,9,3} there should be {3,5,9}
*/
static void solveGaussSeidelInverted_V_0(sparse *pL, sparse *pU, double *pD,
						double * pX, double err, int max_iterations,
						int * pValidStates, double * pLUx)
{
	int i=0, j=0, prev_id, cur_id;
	double tmp;
	BOOL endOfIterations;
	const int LENGTH = pValidStates[0];
	//print_mtx_sparse(pU);
	//print_mtx_sparse(pL);
	//print_vec_double(pU->rows, pD);
	//print_vec_double(pU->rows, pX);
	while(TRUE)
	{
		i++;
		/*Compute the non changeable part of each iteration*/
		multiply_mtx_cer_TMV( pL, pX, pLUx, pValidStates );
		
		endOfIterations = TRUE;
		
		/*Start computing the Xi+1 solution element by element
		Perform the first iteration here as far as it is simpler then others*/
		cur_id = pValidStates[1];
		tmp =  - pLUx[cur_id] / pD[cur_id];
		endOfIterations = endOfIterations && fabs( tmp - pX[ cur_id ] ) <= err;
		//printf("new = %1.15le - old = %1.15le == %1.15le\n",tmp,pX[ cur_id ],tmp - pX[ cur_id ]);
		pX[cur_id] = tmp;
		prev_id = cur_id;
		/*Perform all the rest iterations*/
		for( j = 2; j <= LENGTH; j++)
		{
			cur_id = pValidStates[j];
			multiplyUrowByConstAndAddToLUx(prev_id, pU, pX[prev_id], pLUx);
			tmp =  - pLUx[cur_id] / pD[cur_id];
			endOfIterations = endOfIterations && fabs( tmp - pX[ cur_id ] ) <= err ;
			//printf("new = %1.15le - old = %1.15le == %1.15le\n",tmp,pX[ cur_id ],tmp - pX[ cur_id ]);
			pX[cur_id] = tmp;
			prev_id = cur_id;
		}
		/*Stop if we need or can*/
		if( endOfIterations || ( i > max_iterations ) ) break;
		/*Improve the convergence*/
		if ( i % 10 == 0) normalizeSolution( 0, pX, pValidStates, FALSE );
		//print_vec_double(pU->rows, pX);
	}
	
	printf("Gauss Seidel Inverted V_0: The number of Gauss-Seidel iterations %d\n",i);
	if( i > max_iterations ) printf("ERROR: %s\n", METHOD_DIVERGENCE_MSG);
	
}

/**
* If the pB vector is zero result=x(-LU)DI and pValidStates are unavailable
*/
static void solveGaussSeidelInverted_NULL_0(sparse *pL, sparse *pU, double *pD,
						double * pX, double err, int max_iterations,
						const int N_STATES, double * pLUx)
{
	int i=0, j=0;
	double tmp;
	BOOL endOfIterations;
	while(TRUE)
	{
		i++;
		/*Compute the non changeable part of each iteration*/
		multiply_mtx_TMV( pL, pX, pLUx );
		endOfIterations = TRUE;
		/*Start computing the Xi+1 solution element by element
		Perform the first iteration here as far as it is simpler then others*/
		tmp =  - pLUx[0] / pD[0];
		endOfIterations = endOfIterations && fabs( tmp - pX[ 0 ] ) <= err;
		pX[0] = tmp;
		/*Perform all the rest iterations*/
		for( j = 1; j < N_STATES; j++)
		{
			multiplyUrowByConstAndAddToLUx(j-1, pU, pX[j-1], pLUx);
			tmp =  - pLUx[j] / pD[j];
			endOfIterations = endOfIterations && fabs( tmp - pX[ j ] ) <= err ;
			pX[j] = tmp;
		}
		/*Stop if we need or can*/
		if( endOfIterations || ( i > max_iterations ) ) break;
		/*Improve the convergence*/
		if ( i % 10 == 0) normalizeSolution( N_STATES, pX, NULL, FALSE );
	}
	
	printf("Gauss Seidel Inverted NULL_0: The number of Gauss-Seidel iterations %d\n",i);
	if( i > max_iterations ) printf("ERROR: %s\n", METHOD_DIVERGENCE_MSG);
	
}

/**
* Solves the system of linear equations xA=b using the Inverted Gauss-Seidel method
* @param pA the A matrix
* @param pX the initial x vector
* @param pB the b vector
* @param err the difference between two successive x vector lavues which indicates
*	     when we can stop iterations
* @param max_iterations the max number of iterations
* @param pValidStates this array contains the number of nodes as the first element
*                     all the other elements are the node ids, if it is NULL then
*                     all the nodes from the A matrix are valid
* @return the solution of the system
*/
static double * solveGaussSeidelInverted(sparse * pA, double * pX, double * pB,
				  double err, int max_iterations, int * pValidStates)
{
	const int N_STATES = pA->rows;
	sparse *pL, *pU;
	double *pD = (double *) calloc( N_STATES, sizeof(double) );
	double * pLUx = (double *) calloc( N_STATES, sizeof(double) );

	/*Init the pL matrix*/
	pL = init_matrix( N_STATES, pA->cols );
	/*Init the pU matrix*/
	pU = init_matrix( N_STATES, pA->cols );
	
	/*Get D, L and U, A = L + diag(D) + U*/
	split_A_into_D_L_U( pA, pD, pL, pU );
	
	/*Solve system iteratively*/
	if( pValidStates )
	{
		/*NOTE: The pValidStates states should be ordered, otherwise we will get a wrong solution
		I.e. for example instead of {5,9,3} there should be {3,5,9}*/
		orderValidStates(pValidStates);
		if( pB )
		{
			/*ToDo: Implement*/
			printf("ERROR: solveGaussSeidelInverted_V_B is not implemented.\n");
		}
		else
		{
			/*If the pB vector is zero result=x(-LU)DI*/
			solveGaussSeidelInverted_V_0(pL, pU, pD, pX, err,
						max_iterations, pValidStates, pLUx);
		}
	}
	else
	{
		if( pB )
		{
			/*ToDo: Implement*/
			printf("ERROR: solveGaussSeidelInverted_NULL_B is not implemented.\n");
		}
		else
		{
			/*If the pB vector is zero result=x(-LU)DI*/
			solveGaussSeidelInverted_NULL_0(pL, pU, pD, pX, err,
							max_iterations, N_STATES, pLUx);
		}
	}
	
	free(pD);
	free(pLUx);
	free_mtx_wrap(pL);
	free_mtx_wrap(pU);
	
	/*Return the result as it is modified internaly and finaly contains the resulting solution*/
	return pX;
}

/**
* Simple printing info method
*/
void print_info(char * method, double err, int max_iterations)
{
	printf("%s\n", method);
	printf("\tError: %le\n", err);
	printf("\tMax iter: %d\n", max_iterations);
}

/**
* Solves the system of linear equations xA=b using one of the Inverted methods
* @param type the type of the method to use (GAUSS_JACOBI,GAUSS_JACOBI_INV,GAUSS_SEIDEL,...)
* @param pA the A matrix
* @param pX the initial x vector
*           NOTE: Is possibly freed inside by a solver method (like solveGaussJacobi)
* @param pB the b vector if NULL then b = (0,...,0)
* @param err the difference between two successive x vector lavues which indicates
*	     when we can stop iterations
* @param max_iterations the max number of iterations
* @param pValidStates this array contains the number of nodes as the first element
*                     all the other elements are the node ids, if it is NULL then
*                     all the nodes from the A matrix are valid
* @return the solution of the system
*/
static double * solve(int type, sparse * pA, double * pX, double * pB,
		double err, int max_iterations, int * pValidStates)
{
	double * result = NULL;
	printf("Solving the system of linear equations:\n\tMethod: ");
	switch(type)
	{
		case GAUSS_JACOBI:
			print_info("GAUSS-JACOBI", err, max_iterations);
			result = solveGaussJacobi(pA, pX, pB, err, max_iterations, pValidStates);
			break;
		case GAUSS_SEIDEL:
			print_info("GAUSS-SEIDEL", err, max_iterations);
			result = solveGaussSeidel(pA, pX, pB, err, max_iterations, pValidStates);
			break;
		case GAUSS_JACOBI_INV:
			print_info("GAUSS-JACOBI-INVERTED", err, max_iterations);
			result = solveGaussJacobiInverted(pA, pX, pB, err, max_iterations, pValidStates);
			break;
		case GAUSS_SEIDEL_INV:
			print_info("GAUSS-SEIDEL-INVERTED", err, max_iterations);
			result = solveGaussSeidelInverted(pA, pX, pB, err, max_iterations, pValidStates);
			break;
		default:
			printf("Bug: The method to solve a system of linear equations is not defined.\n");
			exit(1);
	}
	return result;
}

/**
* This method uses one of the method to iteratively solve the system of linear equations.
* The initial X value is computed inside this method
* @param pQ the matrix to be solved
* @param pX an optional initial vector, if not provided the one is created internally.
* @param pB the b vector if NULL then b = (0,...,0)
* @param err the difference between two successive x vector lavues which indicates
*            when we can stop iterations
* @param max_iterations the max number of iterations
* @param pValidStates the valid states i.e. ids of the rows which are valid and for which
*                     the system is solved, this array contains the number of nodes as the
*                     first element
*/
double * solve_initial(int type, sparse * pQ, double * pX, double * pB, double err,
				int max_iterations, int * pValidStates)
{
	//If a specific initial vector is not provided.
	//For example it is provided for the unbounded until operator (PCTL, CSL)
	if(pX == NULL){
		/*Create the initial vector*/
		pX = (double *) calloc( pQ->rows, sizeof( double ) );
		double init_value = 0.0;
		/*Initialize the initial X vector*/
		int i, length;
		if( pValidStates )
		{
			length = pValidStates[0];
			init_value = 1.0/length;
			for( i = 1; i <= length; i++)
			{
				pX[ pValidStates[i] ] = init_value;
			}
		}
		else
		{
			length = pQ->rows;
			init_value = 1.0/length;
			for( i = 0; i < length; i++)
			{
				pX[i] = init_value;
			}
		}
	}
	
	/*Solve system of linear equations*/
	return solve(type, pQ, pX, pB, err, max_iterations, pValidStates);
}
