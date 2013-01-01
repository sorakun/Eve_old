/*
 * Eve programming language
 * Functions handler
 * check eve.h for copyright informations
 */

#include <stdlib.h>
#include <string.h>

#include "eve.h"
#include "error.h"
#include "lex.h"
#include "parse.h"

int func_is_defined(string name)
{
    int i;
    debugf("[funcisdefined] working on %s\n", name);
    for (i = 0; i < global_functions_count; i++)
    {
        debugf("[funcisdefined]:comparing %s with %s\n", name, global_functions[i]->name);
        if (strcmp(name, global_functions[i]->name) == 0)
        {
            // it should be func not proc
            debugf("[func_is_defined] %s is defined, but is it proc or func\n", name);
            if(global_functions[i]->type == _func)
            {
                debugf("[func_is_defined] %s is defined\n", name);
                return 1;
            }
            else
                return 0;
        }
    }
    return 0;
}

int proc_is_defined(string name)
{
    int i;
    debugf("[proc_is_defined] working on %s\n", name);
    for (i = 0; i < global_functions_count; i++)
    {
        debugf("[proc_is_defined]:comparing %s with %s with type %s\n", name, global_functions[i]->name, global_functions[i]->return_type);
        if (strcmp(name, global_functions[i]->name) == 0)
        {
            // it should be func not proc
            debugf("[proc_is_defined] %s is defined, but is it proc or func\n", name);
            if(global_functions[i]->type == _proc)
            {
                debugf("[proc_is_defined] %s is defined\n", name);
                return 1;
            }
            else
                return 0;
        }
    }
    return 0;
}

// this function is unsafe and should be called after making sure
// that the function exists.
tThread * find_func(string name, tThreadType type)
{
    int i;
    debugf("[find_func] working on %s\n", name);
    for (i = 0; i < global_functions_count; i++)
    {
        debugf("[find_func]:comparing %s with %s with type %s\n", name, global_functions[i]->name, global_functions[i]->return_type);
        if (strcmp(name, global_functions[i]->name) == 0)
        {
            // it should be func not proc
            debugf("[find_func] %s is defined, but is it proc or func\n", name);
            if(global_functions[i]->type == type)
            {
                debugf("[func_is_defined] %s is defined\n", name);
                return global_functions[i];
            }
        }
    }
    return NULL;
}


void init_functions_stack()
{
    global_functions_count = 0;
    global_functions = (tThread**)eve_malloc(sizeof(tThread*));
}

void register_function(tThread * func)
{
    debugf("[registerfunction] registering %s at pos %d\n", func->name, global_functions_count);
    global_functions_count++;
    global_functions = (tThread**)eve_realloc(global_functions, global_functions_count*sizeof(tThread*));
    global_functions[global_functions_count-1] = func;
}
