/*
 * Eve programming language
 * Types handler
 * check eve.h for copyright informations
 */

#include <stdlib.h>
#include <string.h>

#include "eve.h"
#include "error.h"
#include "lex.h"
#include "parse.h"

tType * create_global_types()
{
    tType * types = (tType*)eve_malloc(6*sizeof(tType));
    types[0].BasicType = _int;
    types[1].BasicType = _str;
    types[2].BasicType = _float;
    types[3].BasicType = _char;
    types[4].BasicType = _bool;
    types[5].BasicType = _void;

    types[0].name = strdup("int");
    types[0].pointer = 0;
    types[0].pointerto = strdup("int");
    types[1].name = strdup("str");
    types[1].pointer = 0;
    types[1].pointerto = strdup("str");
    types[2].name = strdup("float");
    types[2].pointer = 0;
    types[2].pointerto = strdup("float");
    types[3].name = strdup("char");
    types[3].pointer = 0;
    types[3].pointerto = strdup("char");
    types[4].name = strdup("bool");
    types[4].pointer = 0;
    types[4].pointerto = strdup("bool");
    types[5].name = strdup("void");
    types[5].pointer = 0;
    types[5].pointerto = strdup("void");

    return types;
}

void add_basic_type(const string name, tBasicType BasicType, int maxsize, tField * fields)
{
    global_types = (tType*)eve_realloc(global_types, global_types_count*sizeof(tType));
    global_types[global_types_count].name = name;
    global_types[global_types_count].BasicType = BasicType;
    global_types[global_types_count].maxsize = maxsize;
    global_types[global_types_count].fields = fields;

    global_types_count++;
}

void register_type(tType type)
{
    global_types = (tType*)realloc(global_types, (global_types_count+1)*sizeof(tType));
    global_types[global_types_count] = type;
    global_types_count++;
}

int type_is_defined(const string name)
{
    int i;
    for (i = 0; i < global_types_count; i++)
    {
        debugf("[type_is_defined]:comparing %s with %s\n", name, global_types[i].name);
        if (strcmp(name, global_types[i].name) == 0)
            return 1;
    }
    return 0;
}

tType find_type(string name)
{
    int i;
    for (i = 0; i < global_types_count; i++)
    {
        debugf("[find_type]:comparing %s with %s\n", name, global_types[i].name);
        if (strcmp(name, global_types[i].name) == 0)
            return global_types[i];
    }
}

string find_type_pointerto(string pointed, int pointer)
{
    debugf("Looking for %s is pointer = %d\n", pointed, pointer);
    int i;
    for (i = 0; i < global_types_count; i++)
      if (strcmp(pointed, global_types[i].pointerto) == 0)
        if(global_types[i].pointer == pointer)
        {
          debugf("%s founded\n", global_types[i].name);
          return global_types[i].name;
        }

    return NULL;
}

string mod_to_str(tMod mod)
{
    switch (mod)
    {
    case _extern:
        return "extern";
    case _static:
        return "static";
    case _register:
        return "register";
    case _local:
        return "";
    case _volatile:
        "volatile";
    case _var:
        return "";
    }
}

int is_mod(token_node token)
{
    switch (token.TT)
    {
    case EXTERN:
    case STATIC:
    case REGISTER:
    case LOCAL:
    case VOLATILE:
    case CONST:
    case VAR:
        return 1;
    default:
        return 0;
    }

}

tMod token_to_mod(token_node token)
{
    switch (token.TT)
    {
    case EXTERN:
        return _extern;
    case STATIC:
        return _static;
    case REGISTER:
        return _register;
    case LOCAL:
        return _local;
    case VOLATILE:
        return _volatile;
    case VAR:
        return _var;
    }
}

