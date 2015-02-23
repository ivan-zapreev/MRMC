/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: read_lab_file.c,v $
*	$Revision: 1.5 $
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
*	Source description: Read Label (.lab) file.
*	Uses: DEF: bitset.h, label.h, token.h
*		LIB: bitset.c, label.c, token.c
*		Definition of read_lab_file - read_lab_file.h
*/

# include "macro.h"

# include "bitset.h"
# include "label.h"
# include "token.h"
# include "read_lab_file.h"

/*****************************************************************************
name		: read_lab_file
role       	: reads a .lab file. puts the result in a labelling structure (label.h).
@param     	: int ns: the number of states.
@param     	: char *filename: input .lab file's name.
@return		: labelling *: returns a pointer to a labelling function.
remark 	        : 
******************************************************************************/
labelling * read_lab_file(int ns, char *filename)
{
	FILE *p;
	labelling *labellin;
	int n=0, c, state, key;
	char  s[1024], decl[13], end[5], token[MAXTOKENSIZE];
	
	const char * END_DECL = "#END";
	p = fopen(filename, "r");
	if(p==NULL){
		return NULL;
	}
	if(fgets( s, 1024, p )!=NULL){
		sscanf( s, "%s", decl);
		//Count the number of label declarations
		while(NULL != fgets( s, 1024, p )){
			//printf("Buf: '%s'\n",s);
			c=0;
			while((key=get_next_token(s,&c,token))!=EOL && (0 != strcmp(token,END_DECL))){
				//printf("Token: '%s' <> '%s'\n",token, END_DECL);
				++n;
			}
			if( 0 == strcmp(token,END_DECL) ){
				break;
			}
		}
		//printf("Labels nbr: %d\n",n);
		(void)fclose(p);

		//Reopen the label file for the second pass,
		p = fopen(filename, "r");
		if(p==NULL){
			return NULL;
		}
		if(fgets( s, 1024, p )!=NULL){
			sscanf( s, "%s", decl);
			
			//Allocate the structure for storing labels
			labellin = get_new_label(n,ns);
			
			//Read declarations once again 
			while(NULL != fgets( s, 1024, p )){
				//printf("Buf: %s\n",s);
				c=0;
				while((key=get_next_token(s,&c,token))!=EOL && ( 0 != strcmp(token,END_DECL))){
					add_label(labellin, token);
					//printf("Adding label: %s\n",token);
				}
				if(0 == strcmp(token,END_DECL)) break;
			}
		}

		//Read states/labels and fill in the structure.
		while (NULL != fgets( s, 1024, p )){
			c=0;
			key == get_next_token(s,&c,token);
			state = atoi(token);
			while((key=get_next_token(s,&c,token))!=EOL){
				set_label_bit(labellin,token,state-1);
				//printf("Adding label \'%s\' to the state %d\n",token,state);
			}
		}
	}
	(void)fclose(p);
	return labellin;
}

