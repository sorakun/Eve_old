/*
 * Eve programming language header.
 * Eve data types and core functions implementation.
 *
 */

#include <stdlib.h>
#include <stdio.h>

void mem_alloc(void * p, size_t mem_size)
{
	p = malloc(mem_size);
}
