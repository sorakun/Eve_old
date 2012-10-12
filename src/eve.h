/*
 * Eve programming language
 * (c) 2012 Chouri Soulaymen (sorakun)
 * Projected started since: 13/07/2012
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef EVE_EVE_H
#define EVE_EVE_H

#include <stdio.h>

#define string char *

//#define EVE_DEBUG

#ifdef EVE_DEBUG
#define debugf debug
#else
#define debugf ignore
#endif

#define EVE_VERSION "0.0.1"

#define EVE_CMD_OUTPUT \
"Eve programming language version %s.\
\n\
Usage: eve [-c] [-r] [-o]\n\
           [-l] [-p] [-w]\n\
\n\
-c [file]        : Compiles the file.\n\
-r [file]        : Executes the binary file after compiling it.\n\
-o [bin/dyn/obj] : The output should be Executable, dynamic lib or object file.\n\
-l [lib:         : Include a static library.\n\
-p [file]        : Change binary output\'s name\n\
-w               : Disable warnings.\n\
"

FILE * debug_file;

string compile_file;
string compiled_file;

int eve_enable_debug; /* still not working */
int eve_enable_warning;
int buildmode;   /* 0: Compile, 1: Run */
int output_type; /* 1 executable, 2 dynamic library, 3 object file */

#endif
