/*
 * Eve programming language
 * Memory Manager
 * check eve.h for copyright informations
 */

#include <stdlib.h>
#include "eve.h"
#include "error.h"

void * eve_malloc(size_t size)
{
    void * mem;
    if (NULL == (mem = malloc(size)))
        eve_error("not enough memory (malloc)", EVE_MEMORY_ALLOCATION_ERROR);
    return mem;
}

void * eve_calloc(size_t num, size_t size)
{
    void * mem;
    if (NULL == (mem = calloc(num, size)))
        eve_error("not enough memory (calloc)", EVE_MEMORY_ALLOCATION_ERROR);
    return mem;
}

void * eve_realloc(void * p, size_t size)
{
    void * mem;
    if (NULL == (mem = realloc(p, size)))
        eve_error("not enough memory (realloc)", EVE_MEMORY_ALLOCATION_ERROR);
    return mem;
}
