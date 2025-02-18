/***************************************************************************
 *  file                 :  pars.h                                         *
 *  copyright            : (C) 2001 by Dirk Nolting	                   *
 *  email                : nolting@uni-duesseldorf.de                      *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PARS_H_
#define _PARS_H_

class TIPC ;

/// Part of the IPC package
class TIPC_PARS
	{
	public :
	TIPC_PARS ( TIPC *i = NULL ) ;
	
	private :
 	friend class TIPC ;
    int pars_peptid(const char *formel);
    int pars_chem_form(const char *formel);
    int is_symbol(const char *probe);
    int add_component(const char *symbol, const int number);
    int pars_amino_acid(const char *formel);
    int add_amino_acid(const char acid);
    int print_sum(void);

 	TIPC *ipc ;
    int MAX_DIGITS ;
    int MAX_PEP_LINE ;
	} ;    

/*
#define MAX_DIGITS 10
#define MAX_PEP_LINE 81

#include <string.h>
#include <ctype.h>

int pars_peptid(char *formel);
int pars_chem_form(char *formel);
int is_symbol(char *probe);
int add_component(char *symbol,int number);
int pars_amino_acid(char *formel);
int add_amino_acid(char acid);
int print_sum(void);
*/

#endif
