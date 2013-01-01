/*
 * Eve programming language
 * (c) 2012 Chouri Soulaymen (sorakun)
 * Projected started since: 13/07/2012
 *
 * Copyright:
 *
 * What follows is the Modified BSD License.
 * See also http://www.opensource.org/licenses/bsd-license.php
 *
 * Copyright (c) 2012, Chouri Soulaymen (sorakun). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *
 *   3. Neither the names of the authors nor the names of other contributors may
 *      be used to endorse or promote products derived from this software without
 *      specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * ********************************************************************************************
 *
 * For something as essential as git one would should write the classes and method from scratch
 * in order to ensure encapsulation, manageability, modularity, and last but not least maintain
 * ability. These are key factors in software engineering.
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

size_t allocated_mem;

#endif
