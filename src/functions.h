/*
 * Eve programming language
 * Functions handler header
 * check eve.h for copyright informations
 */


#ifndef EVE_FUNCTIONS_H
#define EVE_FUNCTIONS_H

#include "eve.h"
#include "lex.h"

int func_is_defined(string name, tThreadType type);

tThread * find_func(string name, tThreadType type);

void init_functions_stack();

void register_function(tThread * func);

#endif

