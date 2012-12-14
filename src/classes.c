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
        if(strcmp(name, c->methodes[pos]->name) == 0)
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
        if(strcmp(name, c->variables[pos].name) == 0)
            return pos;
    return -1;
}
