/*
 * Eve programming language
 * Compiler
 * check eve.h for copyright informations
 */

#include <libtcc.h>
#include <stdio.h>

#include "eve.h"
#include "error.h"
#include "includes.h"

void eve_compile(char * file, int argc, char ** argv)
{
    TCCState *s;

    s = tcc_new();
    if (!s)
    {
        fprintf(stderr, "Could not create tcc state\n");
        exit(1);
    }

    /* if tcclib.h and libtcc1.a are not installed, where can we find them */
    tcc_add_sysinclude_path(s, "./clib");

#ifdef _WIN32
    tcc_set_lib_path(s, "runtime_lib");
    tcc_add_library_path(s, "runtime_lib");
    tcc_add_library(s, "libtcc1");
    tcc_add_library(s, "user32");
#endif

//    if(eve_enable_debug == 1)
//       tcc_enable_debug(s);

    int i = 0;
    for(; i<cimport_files_count; i++)
        if (tcc_add_file(s, cimport_files[i]) == -1)
            eve_warning("unable to import C file/library %s.", cimport_files[i]);

    if (argc == 2 && !memcmp(argv[1], "lib_path=",9))
        tcc_set_lib_path(s, argv[1]+9);


    if (tcc_compile_string(s, readfile(file)) == -1)
        return 1;
    switch (buildmode)
    {
    case 0:
        // compile
        tcc_set_output_type(s, output_type);
        tcc_output_file(s, compiled_file);
        break;

    case 1:
        // run
        tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
        tcc_run(s, argc, argv);
        break;
    }
    tcc_delete(s);
    return 0;
}
