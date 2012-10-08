/*
 * Eve programming language
 * Variables handler header
 * check eve.h for copyright informations
 */

#ifndef EVE_VARIABLES_H
#define EVE_VARIABLES_H

#include "eve.h"
#include "lex.h"

int var_is_defined(string name, tThread * thread, int check_parent);
void register_variable(string name, string type, int pointer, tMod mod, tThread * thread);
tVar find_variable(string name, tThread * thread);

#endif
