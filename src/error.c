/*
 * Eve programming language
 * Error manager
 * check eve.h for copyright informations
 */

#include <stdio.h>
#include <stdarg.h>

#include "eve.h"
#include "lex.h"
#include "error.h"

eve_enable_warning = 1;

extern FILE * debug_file;

void ignore(const char * msg, ...)
{

}

void init_debugger(string file)
{
#ifdef EVE_DEBUG
    debug_file = fopen("debug.txt", "w");
    fprintf(debug_file, "debugging file %s.\n", file);
    fclose(debug_file);
#endif
}

void debug(const char * msg, ...)
{
    char temp[1024];
    va_list vl;
    va_start(vl, msg);
    vsprintf(temp, msg, vl);
    va_end(vl);
    fprintf(stderr, temp);
    fprintf(stderr, "\n");
    debug_file = fopen("debug.txt", "a");
    fprintf(debug_file, temp);
    fprintf(debug_file, "\n");
    fclose(debug_file);
}

string getErrorType(int type)
{
    switch(type)
    {
    case 0:
        return "Memory allocation error";
    case 1:
        return "Invalid float number";
    case 2:
        return "Invalid hex number";
    case 3:
        return "Wrong token expected";
    case 4:
        return "Statement expected";
    case 5:
        return "Identifier redefinition";
    case 6:
        return "Unkown data type";
    case 7:
        return "End of file with missing syntax";
    case 8:
        return "Invalid use of nil";
    case 9:
        return "Identifier already defined";
    case 10:
        return "Wrong mod expected";
    case 11:
        return "Undefined data type";
    case 12:
        return "Invalid data type";
    case 13:
        return "Invalid procedure call";
    case 14:
        return "Invalid parameters number";
    case 15:
        return "Undefined identifier";
    case 16:
        return "Included file not found";
    case 17:
        return "Wrong call of \"return\"";
    case 18:
        return "Wrong call of \"break\"/\"continue\"";
    case 19:
        return "Unable to read file stream.";
    case 20:
        return "Wrong function definition";
    case 21:
        return "Key redefinition";
    case 22:
        return "Invalid class operand";
    case 23:
        return "Invalid class method";
    case 24:
        return "Method already defined";
    case 25:
        return "Wrong method definition";
    case 26:
        return "Definition expected";
    case 27:
        return "Invalid operation";
    case 28:
        return "Variable already have a read function";
    case 29:
        return "Variable already have a write function";
    case 30:
        return "Invalid use of read/write";
    }
}

int eve_error(string msg, int type)
{
    char fmsg[1024];
    sprintf(fmsg, "Fatal error: %s, %s.\n",getErrorType(type), msg);
    fprintf(stderr, fmsg);
    exit(type);
}

int eve_syntax_error(int type, string file, long line, long pos)
{
    char fmsg[1024];
    sprintf(fmsg, "Syntax Error: file: '%s' line: %d pos: %d (%s).\n", file, line, pos, getErrorType(type));
    fprintf(stderr, fmsg);
    exit(type);
}

int eve_syntax_error_expected(int type, string file, long line, long pos, tTokenType excepted)
{
    char fmsg[1024];
    sprintf(fmsg, "Syntax Error: file: '%s' line: %d pos: %d, \"%s\" expected. (%s)\n", file, line, pos, token_get_string(excepted), getErrorType(type));
    fprintf(stderr, fmsg);
    exit(type);
}

int eve_syntax_error_expected_before(int type, string file, long line, long pos, tTokenType excepted, string wrongToken)
{
    char fmsg[1024];
    sprintf(fmsg, "Syntax Error: file: '%s' line: %d pos: %d, \"%s\" expected before \"%s\". (%s)\n", file, line, pos, token_get_string(excepted), wrongToken, getErrorType(type));
    fprintf(stderr, fmsg);
    exit(type);
}

void eve_custom_error(int type, const char* msg, ...)
{
    char temp[1024];
    va_list vl;
    va_start(vl, msg);
    vsprintf(temp, msg, vl);
    va_end(vl);
    fprintf(stderr, "Error: %s\n", temp);
    exit(type);
}

void eve_warning(const char* msg, ...)
{
    if (eve_enable_warning)
    {
        char temp[1024];
        va_list vl;
        va_start(vl, msg);
        vsprintf(temp, msg, vl);
        va_end(vl);
        fprintf(stderr, "Warning: %s\n", temp);
    }
}
