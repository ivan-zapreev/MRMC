/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: token.c,v $
*	$Revision: 1.2 $
*	$Date: 2006/10/26 16:49:02 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Maneesh Khattri, Ivan S Zapreev
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
*	Source description: A simple tokenizer.
*	Uses: DEF: token.h
*/

# include "token.h"

//Some required constant arrays definitions with the special symbols

//NOTE: The 3d symbol is '\0' which is present implicitly.
static const char *END_OF_LINE_SYMBOLS = "\n\r";
//NOTE: Here the last symbol i.e. '\0' does not count
static const char *WITHIN_LINE_DELIMITER_SYMBOLS = " \t";

static inline BOOL isEndOfLineSymbol(char *src, int *start){
	int index = 0;
	BOOL result = FALSE;
	//NOTE: We also test for the last (invisible)
	//symbol '\0' in the string, that is why we have <= and not <
	for(  ; index <= strlen(END_OF_LINE_SYMBOLS); index++ ){
		if( src[*start] == END_OF_LINE_SYMBOLS[index] ){
			result = TRUE;
		}
	}
	return result;
}

static inline BOOL isWithinLineDelimiter(char *src, int *start){
	int index = 0;
	BOOL result = FALSE;
	for(  ; index < strlen(WITHIN_LINE_DELIMITER_SYMBOLS); index++ ){
		if( src[*start] == WITHIN_LINE_DELIMITER_SYMBOLS[index] ){
			result = TRUE;
		}
	}
	return result;
}

/*****************************************************************************
name		: scan_number
role       	: get the next token: scan for a number.
@param		: char *src: the source string to be tokenized.
@param		: int *start: the index from which to commence search for a token in
		  	 the source string.
@param		: char *token: the token in which the new token is to be put.
@param		: int *idx: the index from which to commence adding in the token.
remark 	        : If while scanning an alphabet is encountered then scan for a
		  string subsequently.
******************************************************************************/
int scan_number(char *src, int *start, char *token, int *idx)
{
	int key = NUMBER;

	while( *idx < MAXTOKENSIZE && isNumeric(src[*start]) && ! isWithinLineDelimiter( src, start ) && ! isEndOfLineSymbol( src, start ) ){
		token[(*idx)++] = src[(*start)++];
	}

	if( ! isNumeric(src[*start]) && ! isWithinLineDelimiter( src, start ) && ! isEndOfLineSymbol( src, start ) ){
		key = ALPHANUMERIC;
		while( *idx < MAXTOKENSIZE && ! isWithinLineDelimiter( src, start ) && ! isEndOfLineSymbol( src, start ) ){
			token[(*idx)++] = src[(*start)++];
		}
	}
	return key;
}

/*****************************************************************************
name		: scan_string
role       	: get the next token: scan for a string.
@param		: char *src: the source string to be tokenized.
@param		: int *start: the index from which to commence search for a token in
		  	 the source string.
@param		: char *token: the token in which the new token is to be put.
@param		: int *idx: the index from which to commence adding in the token.
remark 	        : 
******************************************************************************/
int scan_string(char *src, int *start, char *token, int *idx)
{
	int key = ALPHANUMERIC;
	//printf("ENTER: \\%d\n",(int) '\n');
	//printf("Reading: '");
	while(*idx < MAXTOKENSIZE && ! isWithinLineDelimiter( src, start ) && ! isEndOfLineSymbol( src, start )){
		//printf("\\%d",(int) src[*start]);
		token[(*idx)++] = src[(*start)++];
	}
	//printf("'\n");
	return key;
}

/*****************************************************************************
name		: get_next_token
role       	: get the next token.
@param		: char *src: the source string to be tokenized.
@param		: int *start: the index from which to commence search for a token in
		  	 the source string.
@param		: char *token: the token in which the new token is to be put.
remark 	        : 
******************************************************************************/
int get_next_token(char *src, int *start, char *token)
{
	int idx=0, key;
	//printf("Skipping: '");
	while( isWithinLineDelimiter( src, start ) && ! isEndOfLineSymbol( src, start ) ){
		//printf("%c",src[*start]);
		(*start)++;
	}
	//printf("'\n");
	if( isNumeric( src[*start] ) ){
		key = scan_number(src,start,token,&idx);
	}else{
		key = scan_string(src,start,token,&idx);
	}
	token[idx] = '\0';
	if(idx==0){
		key=EOL;
	}

	return key;
}

