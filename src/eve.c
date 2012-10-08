/*
 * Eve programming language
 * All rights reserved
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "eve.h"
#include "lex.h"
#include "parse.h"
#include "error.h"

FILE * debug_file;


int main(int argc, char **argv)
{
    int i;
    init_debugger(argv[1]);
    for (i = 0; i < argc; i++)
        debugf("argv[%d] = \"%s\"\n", i, argv[i]);
    lib_path = strdup("");

    if (argc < 2)
    {
        printf("Eve programming language version %s.\n\n\
Usage: Eve\n\
\t-coutput (generate c file)\n\
\t-output=[EXE/DLL/OBJ] (executable, dynamic library or object file)\n\
\t-l=[lib] (link a static library)\n", EVE_VERSION);
        return 0;
    }

    global_types = create_global_types();
    global_types_count = 6;
    init_inc();
    init_functions_stack();
    LexInfo li;
    li = Lex_Begin(argv[1], readfile(argv[1]));
    global_thread = create_thread(_main, NULL, "__main__", "void", NULL, 0, 0, 0, 0);
    start_parse(&li);
    string src = strdup(gen_code(argv[1], global_thread));
    eve_compile(src, argc, argv);
    return 0;
}
