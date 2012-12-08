/*
 * Eve programming language
 * includes files manager header
 * check eve.h for copyright informations
 */

#ifndef EVE_INCLUDES_H
#define EVE_INCLUDES_H

#include "eve.h"
#include "lex.h"

string * included_files;
int included_files_count;

string * cimport_files;
int cimport_files_count;

#endif
