/*
 * Eve programming language
 * Types  handler header
 * check eve.h for copyright informations
 */

#ifndef EVE_TYPES_H
#define EVE_TYPES_H

#include "eve.h"
#include "lex.h"

tType * create_global_types();

void add_type(const string name, tBasicType BasicType, int maxsize, tField * fields);

int type_is_defined(const string name);

tType find_type(const string name);

int is_mod(token_node token);

tMod token_to_mod(token_node token);

string mod_to_str(tMod mod);

string find_type_pointerto(tType typ);

#endif
