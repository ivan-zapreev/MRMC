/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: foxglynn.c,v $
*	$Revision: 1.4 $
*	$Date: 2007/05/03 13:37:56 $
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
*	Source description: Calculate left/right truncation pts. and weights based on Fox&Glynn algorithm.
*	Uses: Definition of foxglynn - foxglynn.h
*	Reference: "Computing Poisson Probabilities", Bennett L. Fox and
*			Peter W. Glynn,
*			Communications of the ACM , vol. 31, p. 440-445 (1988 ).
*/

# include "foxglynn.h"

/*The pi constant*/
static const double pi = 3.14159265358979323846264;
static const double lambda_25 = 25.0;
static const double lambda_400 = 400.0;

/*****************************************************************************
Name		: finder
Role       	: The FINDER function from the Fox-Glynn algorithm
@param		: int m: the middle point from weighter.
@param     	: double lambda: (rate of uniformization)*(mission time)
@param	   	: double tau: underflow
@param		: double omega: overflow
@param		: double epsilon: error bound
@param		: double * pw_m: pointer to the initial value in the array of sets, will be set here
@param		: FoxGlynn *: return by reference
@return		: TRUE if everything is fine, otherwise FALSE.
              This is the F parameter of Fox-Glynn finder function.
remark 	    : 
******************************************************************************/
static BOOL finder(const int m, const double lambda, const double tau, const double omega,
                   const double epsilon, double * pw_m, FoxGlynn *pFG)
{	
	const double sqrt_2_pi = sqrt( 2.0 * pi );
	const double sqrt_2 = sqrt(2.0);
	const double sqrt_lambda = sqrt(lambda);
	
	/*Simple bad cases, when we quit*/
	if( lambda == 0.0 )
	{
	  printf("ERROR: Fox-Glynn: lambda = 0, terminating the algorithm\n");
	  return FALSE;
	}

	if( 0.0 < lambda && lambda < lambda_25 )
	{
		if( exp( -lambda ) <= tau )
		{
			printf("ERROR: Fox-Glynn: 0 < lambda < 25, underflow. The results are UNRELIABLE.\n");
//			return FALSE;
		}
		/*Set left truncation point to 0, as the above check did not pass*/
		pFG->left = 0;
	}
	
	/****Compute pFG->right truncation point****/
	/*According to Fox-Glynn, if lambda < 400 we should take lambda = 400,
	  otherwise use the original value. This is for computing the right truncation point*/
	const double lambda_max = ( lambda > lambda_400 ? lambda : lambda_400);
	
	/*The starting point for looking for the right truncation point*/
	const double start_k = 3.0;
	/*Here we choose the max possible value of k from, corollary 1*/
	const double stop_k = sqrt( lambda_max ) / ( 2.0 * sqrt_2 ) ;
	
	/*Start looking for the right truncation point*/
	double k, k_rtp;
	/*For lambda >= 25 we can use the upper bound for a_lambda and d_k_lambda,
	here lambda is always at least 400:
	a_lambda_sup = 1.57;
	a_lambda = ( 1.0 + 1.0 / lambda ) * exp( 1.0 / 16.0 ) * sqrt_2 <= a_lambda_sup
	d_k_lambda_sup = 1.007; for k >= 3
	d_k_lambda = 1.0 / ( 1.0 - exp( - 2.0 / 9.0 * ( k * sqrt( 2.0 * lambda ) + 3.0 / 2.0 ) ) )<= d_k_lambda_sup
	In reality lambda in this formulas should be lambda_max for this case*/
	const double a_lambda_sup = 1.57;
	const double d_k_lambda_sup = 1.007;
	for(k = start_k; k <= stop_k; k = k + 1 ) 
	{
		if( a_lambda_sup * d_k_lambda_sup * exp( - k*k / 2.0 )/ (k * sqrt( 2.0 * pi ) ) < epsilon / 2.0)
		{
			k_rtp = k;
			break;
		}
	}
	pFG->right = (int) ceil( m + k_rtp * sqrt( 2.0 * lambda_max ) + 3.0 / 2.0 );
	
	/****Compute pFG->left truncation point****/
	if( lambda >= lambda_25 )
	{
		/*The starting point for looking for the left truncation point*/
		const double start_k = 1.0 / ( sqrt( 2.0 * lambda ) );
		/*Here we choose the max possible value of k such that
		  (m - k * sqrt_lambda - 3/2) >= 0*/
		const double stop_k = ( m - 3.0/2.0 ) / sqrt_lambda;
		
		/*Start looking for the left truncation point*/
		double k, k_ltp = 0;
		/*For lambda >= 25 we can use the upper bound for b_lambda,
		here lambda is always at least 400:
		b_lambda_sup = 1.05;
		b_lambda = ( 1.0 + 1.0 / lambda ) * exp( 1.0 / ( 8.0*lambda ) ) <= a_lambda_sup*/
		const double b_lambda_sup = 1.05;
		for( k = start_k; k <= stop_k; k = k + 1 )
			if( b_lambda_sup * exp(- 1.0 / 2.0 * k*k) / ( k * sqrt_2_pi ) < epsilon / 2.0 )
			{
				k_ltp = k;
				break;
			}
		
		/*Finally the left truncation point is found*/
		pFG->left = (int) floor( m - k_ltp * sqrt_lambda - 3.0 / 2.0 );
		
		/*Perform the underflow check*/
		const double k_prime = k_ltp + 3.0 / (2.0 * sqrt_lambda);
		/*We take the c_m_inf = 0.02935 / sqrt( m ), as for lambda >= 25
		 c_m = 1 / ( sqrt( 2.0 * pi * m ) ) * exp( m - lambda - 1 / ( 12.0 * m ) ) => c_m_inf*/
		const double c_m_inf = 0.02935 / sqrt( m ); 
		double result = 0.0;
		if( 0.0 < k_prime && k_prime <= sqrt_lambda / 2.0 )
		{
			result = c_m_inf * exp( - pow(k_prime,2.0) / 2 - pow(k_prime, 3.0) / (3.0 * sqrt_lambda) );
		}else{
			if( k_prime <= sqrt( m + 1.0 ) / m )
			{
				double result_1 = c_m_inf * pow(1.0 - k_prime / sqrt( m + 1.0 ), k_prime * sqrt( m + 1 ) );
				double result_2 = exp( - lambda );
				/*Take the maximum*/
				result = ( result_1 > result_2 ? result_1 : result_2);
			}else{/*NOTE: It will be an underflow error*/;}
		}
		if ( result * omega / ( 1.0e+10 * ( pFG->right - pFG->left ) ) <= tau )
		{
			printf("ERROR: Fox-Glynn: lambda >= 25, underflow. The results are UNRELIABLE.\n");
//			return FALSE;
		}
	}
	
	/*We still have to perform an underflow check for the right truncation point when lambda >= 400*/
	if( lambda >= lambda_400 )
	{
		const double k_prime = k_rtp * sqrt_2 + 3.0 / 2.0 * sqrt_lambda;
		/*We take the c_m_inf = 0.02935 / sqrt( m ), as for lambda >= 25
		 c_m = 1 / ( sqrt( 2.0 * pi * m ) ) * exp( m - lambda - 1 / ( 12.0 * m ) ) => c_m_inf*/
		const double c_m_inf = 0.02935 / sqrt( m ); 
		double result = c_m_inf * exp( - pow( k_prime + 1.0 , 2 ) / 2.0 );
		if( result * omega / ( 1.0e+10 * ( pFG->right - pFG->left ) ) <= tau)
		{
			printf("ERROR: Fox-Glynn: lambda >= 400, underflow. The results are UNRELIABLE.\n");
//			return FALSE;
		}
	}
	
	/*Time to set the initial value for weights*/
	*pw_m = omega / ( 1.0e+10 * ( pFG->right - pFG->left ) );
	
	return TRUE;
}

/*****************************************************************************
Name		: finder
Role       	: The WEIGHTER function from the Fox-Glynn algorithm
@param     	: double lambda: (rate of uniformization)*(mission time)
@param	   	: double tau: underflow
@param		: double omega: overflow
@param		: double epsilon: error bound
@param		: FoxGlynn *: return by reference
@return		: TRUE if everything is fine, otherwise FALSE.
              This is the F parameter of Fox-Glynn finder function.
remark 	    : 
******************************************************************************/
static BOOL weighter(const double lambda, const double tau, const double omega, const double epsilon, FoxGlynn *pFG)
{
	/*The magic m point*/
	const int m = (int)floor(lambda);
	double w_m = 0;

	if( ! finder( m, lambda, tau, omega, epsilon, &w_m, pFG ) )
		return FALSE;
	
	/*Allocate space for weights*/
	pFG->weights = (double*) calloc( pFG->right - pFG->left + 1, sizeof( double ) );
	/*Set an initial weigh*/
	pFG->weights[ m - pFG->left ] = w_m;
	
	/*Fill the left side of the array*/
	int j;
	for( j = m; j > pFG->left; j-- )
		pFG->weights[ ( j - pFG->left ) - 1  ] = ( j / lambda ) * pFG->weights[ j - pFG->left ];
	
	/*Fill the right side of the array, have two cases lambda < 400 & lambda >= 400*/
	if( lambda < lambda_400 )
	{
		/*Perform the underflow check, according to Fox-Glynn*/
		if( pFG->right > 600 )
		{
			printf("ERROR: Fox-Glynn: pFG->right > 600, underflow is possible\n");
			return FALSE;
		}
		/*Compute weights*/
		for( j = m; j < pFG->right; j++ )
		{
			double q = lambda / ( j + 1 );
			if( pFG->weights[ j - pFG->left ] > tau / q )
			{
				pFG->weights[ ( j - pFG->left ) + 1  ] = q * pFG->weights[ j - pFG->left ];
			}else{
				pFG->right = j;
				break; /*It's time to compute W*/
			}
		}
	}else{
		/*Compute weights*/
		for( j = m; j < pFG->right; j++ )
			pFG->weights[ ( j - pFG->left ) + 1  ] = ( lambda / ( j + 1 ) ) * pFG->weights[ j - pFG->left ];
	}
	
	/*It is time to compute the normalization weight W*/
	pFG->total_weight = 0.0;
	int s = pFG->left;
	int t = pFG->right;

	while( s < t )
	{
		if( pFG->weights[ s - pFG->left ] <= pFG->weights[ t - pFG->left ] )
		{
			pFG->total_weight += pFG->weights[ s - pFG->left ];
			s++;
		}else{
			pFG->total_weight += pFG->weights[ t - pFG->left ];
			t--;
		}
	}
	pFG->total_weight += pFG->weights[ s - pFG->left ];

	//printf("Fox-Glynn: ltp = %d, rtp = %d, w = %10.15le \n", pFG->left, pFG->right, pFG->total_weight);

	return TRUE;
}

/*****************************************************************************
Name		: fox_glynn
Role		: get poisson probabilities.
@param		: double lambda: (rate of uniformization)*(mission time)
@param		: double tau: underflow
@param		: double omega: overflow
@param		: double epsilon: error bound
@param		: FoxGlynn **: return a new FoxGlynn structure by reference
@return	: TRUE if it worked fine, otherwise false
remark	    	:
******************************************************************************/
BOOL fox_glynn(const double lambda, const double tau, const double omega, const double epsilon, FoxGlynn **ppFG)
{
	//printf("Fox-Glynn: lambda = %3.3le, epsilon = %1.8le\n",lambda, epsilon);

	*ppFG = (FoxGlynn *) calloc (1, sizeof(FoxGlynn));
	(*ppFG)->weights = NULL;

	return weighter(lambda, tau, omega, epsilon, *ppFG);
}


/**
* Fries the memory allocated for the FoxGlynn structure
* @param fg the structure to free
*/
void freeFG(FoxGlynn * fg)
{
	if( fg ){
		if( fg->weights )
			free(fg->weights);
		free(fg);
	}
}

