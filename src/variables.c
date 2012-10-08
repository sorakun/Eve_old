/*
 * Eve programming language
 * Variables handler
 * check eve.h for copyright informations
 */

#include <stdlib.h>
#include <string.h>

#include "eve.h"
#include "error.h"
#include "lex.h"
#include "parse.h"
#include "threads.h"
#include "variables.h"
#include "types.h"

int var_is_defined(string name, tThread * thread, int check_parent)
{
    int i = 0;
    debugf("[varisdfined] in thread %s have %d variable in stack, %d variables in params\n", thread->name, thread->vcount, thread->pcount);
    for (i = 0; i < thread->vcount; i++)
    {
        debugf("i = %d\n");
        debugf("[varisdefined]:comparing %s with %s\n", name, thread->vars[i].name);
        if (strcmp(name, thread->vars[i].name) == 0)
            return 1;
    }

    debugf("[varisdefined] moving to parameters it contains %d\n", thread->pcount);
    for (i = 0; i < thread->pcount; i++)
    {

        debugf("i = %d, %s\n", i, thread->params[i].name);
        debugf("[varisdefined]:comparing %s with %s\n", name, thread->params[i].name);
        if (strcmp(name, thread->params[i].name) == 0)
            return 1;
    }
    debugf("[varisdefined] moving to parent\n");
    // if not in our level, search in higher levels
    if((thread->parent != NULL ) && (check_parent == 1))
        return var_is_defined(name, thread->parent, 1);
    else
        return 0;
}

void register_variable(string name, string type, int pointer, tMod mod, tThread * thread)
{
    debugf("[register_variable]registring variable %s at pos %d\n", name, thread->vcount);
    thread->vars = (tVar*)eve_realloc(thread->vars, (thread->vcount+1) * sizeof(tVar));
    thread->vars[thread->vcount].name = strdup(name);
    thread->vars[thread->vcount].type = type;
    thread->vars[thread->vcount].mod = mod;
    //thread->vars[thread->vcount].pointer = pointer;
    thread->vcount ++;
}

tVar find_variable(string name, tThread * thread)
{
    int i;
    for (i = 0; i < thread->vcount; i++)
    {
        debugf("[varisdefined]:comparing %s with %s\n", name, thread->vars[i].name);
        if (strcmp(name, thread->vars[i].name) == 0)
            return thread->vars[i];
    }
    for (i = 0; i < thread->pcount; i++)
    {
        debugf("[varisdefined]:comparing %s with %s\n", name, thread->params[i].name);
        if (strcmp(name, thread->params[i].name) == 0)
            return thread->params[i];
    }
    if(thread->parent !=NULL)
        return find_variable(name, thread->parent);
}
