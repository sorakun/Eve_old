/*
 * Eve programming language
 * Threads handler header
 * check eve.h for copyright informations
 */

#ifndef EVE_THREADS_H
#define EVE_THREADS_H

#include "eve.h"
#include "lex.h"

tThread * create_thread(tThreadType type, tThread * parent, string name, string return_type, tVar * params, int pcount, int body_defined, int unlimited_args, int cdef);

#endif
