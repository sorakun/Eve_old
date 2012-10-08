/*
 * Eve programming language
 * includes files manager
 * check eve.h for copyright informations
 */

#include <stdlib.h>
#include <string.h>

#include "eve.h"
#include "error.h"
#include "lex.h"
#include "parse.h"
#include "threads.h"
#include "includes.h"

void init_inc()
{
    included_files_count = 0;
    included_files = (string*)eve_malloc(sizeof(string));
    cimport_files_count = 0;
    cimport_files = (string*)eve_malloc(sizeof(string));
}

/* add include, if it does not exist */
void add_inc(string path)
{
    int i = 0;
    for (; i<included_files_count; i++)
      if(strcmp(path, included_files[i]) == 0)
        {
            eve_warning("File %s was already imported (it wont be imported twice).", included_files[i]);
            return;
        }
    included_files_count++;
    included_files = (string)eve_realloc(included_files, included_files_count*sizeof(char));
    included_files[included_files_count-1] = strdup(path);
}

void add_cinc(string path)
{
    int i = 0; int x = strlen(path)-2;
    string path2=(string)malloc(x*sizeof(char));
    strncpy(path2, path+1, x);
    path2[x] = '\0';
    for (; i<cimport_files_count; i++)
      if(strcmp(path2, cimport_files[i]) == 0)
        {
            eve_warning("C file %s was already imported (it wont be imported twice).", cimport_files[i]);
            return;
        }
    cimport_files_count++;
    cimport_files = (string)eve_realloc(cimport_files, cimport_files_count*sizeof(char));
    cimport_files[cimport_files_count-1] = strdup(path2);
}
