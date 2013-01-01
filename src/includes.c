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
            eve_warning("File '%s' was already imported (it wont be imported twice).", included_files[i]);
            return;
        }
    included_files_count++;
    included_files = (string)eve_realloc(included_files, included_files_count*sizeof(char));
    included_files[included_files_count-1] = strdup(path);
}

void add_cinc(string path)
{
    int i = 0;
    int x = strlen(path)-2;
    string path2=(string)malloc(x*sizeof(char));
    strncpy(path2, path+1, x);
    path2[x] = '\0';
    for (; i<cimport_files_count; i++)
        if(strcmp(path2, cimport_files[i]) == 0)
        {
            eve_warning("C file '%s' was already imported (it wont be imported twice).", cimport_files[i]);
            return;
        }
    cimport_files_count++;
    cimport_files = (string)eve_realloc(cimport_files, cimport_files_count*sizeof(char));
    cimport_files[cimport_files_count-1] = strdup(path2);
}

void init_imported_files()
{
    imported_files = (string*)eve_malloc(sizeof(string));
    imported_files_count = 0;
}

void add_imported_file(string name)
{
    imported_files = (string*)eve_realloc(imported_files, sizeof(string));
    imported_files[imported_files_count] = strdup(name);
    imported_files_count++;
}

int file_is_imported(string name)
{
    int i = 0;
    for (; i<imported_files_count; i++)
        if (strcmp(name, imported_files[i]) ==0)
            return 1;
    return 0;
}

void free_imported_files()
{
    int i = 0;
    for (; i<imported_files_count; i++)
        free(imported_files[i]);
    free(imported_files);
}
