/*
 * Eve programming language
 * includes files manager header
 * check eve.h for copyright informations
 */

#ifndef EVE_INCLUDES_H
#define EVE_INCLUDES_H

#include "eve.h"
#include "lex.h"

string * included_files;
int included_files_count;

string * cimport_files;
int cimport_files_count;

string * imported_files;
int imported_files_count;

void init_inc();

void add_inc(string path);

void add_cinc(string path);

void init_imported_files();

void add_imported_file(string name);

int file_is_imported(string name);

void free_imported_files();
#endif
