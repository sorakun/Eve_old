/*
 * Eve programming language
 * Memory Manager header
 * check eve.h for copyright informations
 */

#ifndef EVE_MEMORY_H
#define EVE_MEMORY_H

#include "eve.h"

void * eve_malloc (size_t             size);

void * eve_calloc (size_t num, size_t size);

void * eve_realloc(void * p,   size_t size);


#endif
