/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: read_tra_file.c,v $
*	$Revision: 1.4 $
*	$Date: 2007/10/03 09:44:29 $
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
*	Source description: Read transition (.tra) file.
*	Uses: DEF: sparse.h
*		LIB: sparse.c
*		Definition of read_tra_file - read_tra_file.h
*/

# include "macro.h"

#include "bitset.h"
#include "sparse.h"
#include "read_tra_file.h"

/**
* This method does the one pass thrue the .tra file and computes
* the number of non zero elements in each row. Then it stores this
* values into the ncolse array.
*/
int * make_first_pass(char *filename)
{
	FILE *p;
	char  s[1024], states[7], transitions[12];
	int size=0, row=0, col=0, nnz=0, *ncolse=NULL;
	double val=0.0;
	p = fopen(filename, "r");
	if(p==NULL) return NULL;
	
	if(fgets( s, 1024, p )!=NULL)
	{
		sscanf( s, "%s%d", states, &size);
		if(fgets( s, 1024, p )!=NULL)
		{
			sscanf( s, "%s%d", transitions, &nnz);
			ncolse=(int *)calloc(size, sizeof(int));
			while (NULL != fgets( s, 1024, p ))
			{
				sscanf( s, "%d%d%lf", &row, &col, &val );
				if(row != col)
					++ncolse[row-1];
			}
		}
	}
	(void)fclose(p);
	return ncolse;
}

void print_read_mtx(sparse *sp){
	if(sp == NULL){
		printf("ERROR: The state space matrix seems to be unallocated!\n");
		exit(1);
	}
	
	FILE *of;
	of = fopen("matrix_test_read.tra", "w");
	fprintf(of, "STATES %d\n",sp->rows);
	
	int nnz = 0, i = 0;
	for(i = 0; i < sp->rows; i++){
		nnz += sp->ncols[i];
		if(sp->val[i].diag != 0)
			nnz ++;
	}
	fprintf(of, "TRANSITIONS %d\n", nnz);
	int j = 0;
	BOOL printed_diag = FALSE;
	for(i = 0; i < sp->rows; i++){
		printed_diag = FALSE;
		for(j = 0; j < sp->ncols[i]; j++){
			if( ( i < sp->val[i].col[j] ||
			    ( i > sp->val[i].col[j] && j+1 == sp->ncols[i]) ) &&
			    sp->val[i].diag != 0 &&
			    printed_diag == FALSE){
				fprintf(of, "%d %d %.12lg\n", i+1, i+1, sp->val[i].diag);
				printed_diag = TRUE;
			}
			fprintf(of, "%d %d %.12lg\n", i+1, sp->val[i].col[j]+1, sp->val[i].val[j]);
		}
		if(sp->ncols[i] == 0 && sp->val[i].diag != 0, sp->val[i].diag){
			//NOTE: sp->ncols[i] == 0 even if sp->val[i].diag != 0
			fprintf(of, "%d %d %.12lg\n", i+1, i+1, sp->val[i].diag);
		}
	}
	(void)fclose(of);
}

/*****************************************************************************
name		: read_tra_file
role       	: reads a .tra file. puts the result in a sparse matrix (sparse.h).
@param     	: char *filename: input .tra file's name.
@return		: sparse *: returns a pointer to a sparse matrix.
remark 	        : 
******************************************************************************/
sparse * read_tra_file(char *filename)
{
	FILE *p;
	char  s[1024],states[7],transitions[12];
	int size, row, col, nnz,i, *ncolse;
	double val = 0.0;
	sparse *sp = NULL;
	
	ncolse = make_first_pass(filename);
	
	p = fopen(filename, "r");
	if(p==NULL) return NULL;
	if(fgets( s, 1024, p )!=NULL)
	{
		sscanf( s, "%s%d", states, &size);
		if(fgets( s, 1024, p )!=NULL)
		{
			sscanf( s, "%s%d", transitions, &nnz);
			printf("States=%d, Transitions=%d\n", size, nnz);
			init_matrix_ncolse(size, size, ncolse);
			while (NULL != fgets( s, 1024, p ))
			{
				sscanf( s, "%d%d%lf", &row, &col, &val );
				set_val_ncolse(row-1,col-1,val);
			}
		}
	}
	(void)fclose(p);
	sp = get_sparse();

	//print_read_mtx(sp);

	free(ncolse);
	return sp;
}
