/*
 * Eve programming language
 * File io and other utilities
 * check eve.h for copyright informations
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "eve.h"
#include "error.h"
#include "memory.h"

long flengthh(FILE* file)
{
    long cur = ftell(file), len;
    fseek(file, 0, SEEK_END);
    len = ftell(file);
    fseek(file, cur, SEEK_SET);
    return len;
}

/* Get file length from a file name. */
long flength(const char* fname)
{
    FILE* file = fopen(fname, "rb");
    long flen = 0;
    if (file)
    {
        flen = flengthh(file);
        fclose(file);
    }
    return flen;
}

char* readfile(const char* fname)
{
    char* buf;
    FILE* fp;
    int flen = flength(fname);
    if ((buf=eve_calloc(flen+1, sizeof(char))) == NULL)
        return NULL;
    fp = fopen(fname, "rb");
    if (fp == NULL)
    {
        free(buf);
        eve_custom_error(EVE_UNABLE_TO_READ_FILE, "Unable to read file \"%s\".", fname);
    }
    fread(buf, sizeof(char), flen, fp);
    fclose(fp);
    buf[flen] = 0;
    return buf;
}

char * extract_name(char * src)
{
    char * tmp = (char*)eve_malloc(sizeof(char));
    int i =0;
    for (; i<strlen(src) && src[i] != '.'; i++)
    {
        tmp = (char*)eve_realloc(tmp, (i+1)*sizeof(char));
        tmp[i] = src[i];
    }
    tmp[i] ='\0';
#ifdef _WIN32
    strcat(tmp, ".exe");
#endif
    return tmp;
}

char dir_sep()
{
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

char * extract_dir(char * src)
{
    char *str;
    strcpy(str, src);
    int i;
    for(i = strlen(str) - 1; i>0; --i)
    {
        if( str[i] == dir_sep() )
        {
            str[i] = '\0';
            break;
        }
    }
    return str;
}

int file_exists(const char * filename)
{
    FILE * file = NULL;
    if ((file = fopen(filename, "r")) != NULL)
    {
        debugf("files exists\n");
        //fclose(file);
        return 1;
    }
    return 0;
}

#ifndef POSIX
string strdup(const string src)
{
    char * p = malloc(strlen(src) + 1);
    if (p != NULL)
        strcpy(p, src);
    return p;
}
#endif
