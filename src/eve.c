/*
 * Eve programming language
 * All rights reserved
 * Read eve.h for copyright informations.
 */

#include <stdio.h>
#include <stdlib.h>

#include "eve.h"
#include "lex.h"
#include "parse.h"
#include "error.h"

void parse_cmds(int argc, char **argv)
{
    int i = 1;
    eve_enable_warning = 1;
    eve_enable_debug = 0;
    output_type = 1;
    for(i; i < argc; i++)
    {
        if( strcmp( argv[i], "-c" ) == 0 )
        {
            compile_file = argv[i+1];
            buildmode = 0;
            i++;
            continue;
        }
        if( strcmp( argv[i], "-r" ) == 0 )
        {
            compile_file = argv[i+1];
            buildmode = 1;
            i++;
            continue;
        }
        if( strcmp( argv[i], "-p" ) == 0 )
        {
            compiled_file = argv[i+1];
            i++;
            continue;
        }
        if( strcmp( argv[i], "-o" ) == 0 )
        {
            if(strcmp(argv[i+1], "bin"))
                output_type = 1;
            else if(strcmp(argv[i+1], "dyn"))
                output_type = 2;
            else if(strcmp(argv[i+1], "obj"))
                output_type = 3;
            else
                eve_custom_error(0, "Invalid argument of -o, expected \"bin\", \"dyn\" or \"obj\".");
            i++;
            continue;
        }
        if( strcmp( argv[i], "-w" ) == 0 )
        {
            eve_enable_warning = 0;
            continue;
        }
        if( strcmp( argv[i], "-d" ) == 0 )
        {
            eve_enable_debug = 1;
            continue;
        }
        eve_custom_error(0, "Invalid argument number %d, \"%s\".", i+1, argv[i]);
    }
    if(compiled_file == NULL)
        compiled_file = extract_name(compile_file);
}

int main(int argc, char **argv)
{
    int i;
    init_debugger(argv[1]);
    for (i = 0; i < argc; i++)
        debugf("argv[%d] = \"%s\"\n", i, argv[i]);
    lib_path = strdup("lib/");

    if (argc < 2)
    {
        printf(EVE_CMD_OUTPUT, EVE_VERSION);
        return 0;
    }
    parse_cmds(argc, argv);
    global_types = create_global_types();
    global_types_count = 6;
    init_inc();
    init_functions_stack();
    LexInfo li;
    li = Lex_Begin(compile_file, readfile(compile_file));
    global_thread = create_thread(_main, NULL, "__main__", "void", NULL, 0, 0, 0, 0);
    start_parse(&li);
    debugf("parsing ended successfully.\n");
    string src = strdup(gen_code(compile_file, global_thread));
    eve_compile(src, argc, argv);
    return 0;
}
