/*
 * Eve programming language
 * Classes utilities
 * check eve.h for copyright informations
 */

#include <stdlib.h>
#include <string.h>

#include "eve.h"
#include "error.h"
#include "lex.h"
#include "parse.h"

/*
 * return the position in thread, if exists
 * else returns -1
 */
int is_member_func(char * name, class_ * c)
{
    int pos = 0;
    for(; pos < c->mcount; pos ++)
    {
        debugf("looking for class func %s vs %s (%s)\n", name, c->methodes[pos]->name, c->methodes[pos]->gen_name);
        if(match_str(name, c->methodes[pos]->name))
            return pos;
    }
    return -1;
}

/*
 * return the position in thread, if exists
 * else returns -1
 */
int is_member_data(char * name, class_ * c)
{
    debugf("checking existance of %s in class %s, have %d data\n", name, c->name, c->vcount);
    int pos = 0;
    for(; pos < c->vcount; pos++)
        if(match_str(name, c->variables[pos].name))
            return pos;
    return -1;
}

/*
 * Checks if a given string is a valid method's name
 * returns 1 if func, 2 if proc
 * 0 if it's invalid
 */

int is_class_func(char * name)
{
    int i = 0;
    int pos = 0;
    for(; i< global_types_count; i++)
    {
        if (global_types[i].type_kind == __class)
            if(is_member_func(name, &global_types[i].class_info) != -1)
                return 1;
    }
    return 0;
}

tThread * get_member_func(char * name, class_ * c)
{
{
    int pos = 0;
    for(; pos < c->mcount; pos ++)
    {
        debugf("looking for class func %s vs %s (%s)\n", name, c->methodes[pos]->name, c->methodes[pos]->gen_name);
        if(match_str(name, c->methodes[pos]->name))
            return c->methodes[pos];
    }
    return NULL;
}
}
