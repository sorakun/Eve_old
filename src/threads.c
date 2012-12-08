/*
 * Eve programming language
 * Threads handler
 * check eve.h for copyright informations
 */

#include <stdlib.h>
#include <string.h>

#include "eve.h"
#include "error.h"
#include "lex.h"
#include "parse.h"
#include "threads.h"

char * dupstr(const char *s)
{
    if (s == NULL)
        return NULL;
    char *const result = malloc(strlen(s) + 1);
    if (result != NULL)
    {
        strcpy(result, s);
    }
    return result;
}


tThread * create_thread(tThreadType type, tThread * parent, string name, string return_type, tVar * params, int pcount, int body_defined, int unlimited_args, int cdef)
{
    tThread * thread = (tThread*)eve_malloc(sizeof(tThread));
    thread->parent = parent;
    thread->name = dupstr(name);
    thread->type = type;
    thread->return_type = dupstr(return_type);
    thread->global_types = global_types;
    //thread->global_functions = global_functions;
    thread->instructions = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->icount = 0;
    thread->vcount = 0;
    thread->pcount = pcount;
    thread->params = params;
    thread->vars = (tVar*)eve_malloc(sizeof(tVar));
    thread->if_coumpound = 0;
    thread->cdef = cdef;

    thread->body_defined = body_defined;
    thread->unlimited_args = unlimited_args;
    return thread;
}
