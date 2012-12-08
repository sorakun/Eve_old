/*
 * Eve programming language
 * Parser header
 * check eve.h for copyright informations
 */

#ifndef EVE_PARSE_H
#define EVE_PARSE_H

#include "eve.h"
#include "lex.h"

typedef enum tMod
{
    _extern, _static, _register, _local, _volatile, _const, _none, _var // var in procedures
} tMod;

typedef enum tBasicType
{
    _int, _str, _float, _char, _bool, _array, _record, _class, _void
} tBasicType;

struct tType;

typedef struct tField
{
    string name;
    struct tType * type;
} tField;

typedef enum tType_kind
{
    __array, __enum, __class, __none, __struct
} tType_kind;

typedef struct tType
{
    tType_kind type_kind;
    string name;
    tBasicType BasicType;
    int maxsize; // size in case of array
    // fields in case of record
    tField * fields;
    int fcount; // # of fields

    char ** enums; // list of enums
    int ecount;    // enums count

    // methods in case of class
    // TODO.
    int pointer; // is it a pointer?
    string pointerto;
} tType;

typedef struct tVar
{
    string name;
    string type;
    tMod mod;
    token_node info;
} tVar;

typedef enum tThreadType
{
    _main, _func, _proc, _block, _repeat, _while, _if, _elif, _else, _for, _foreach
} tThreadType;

struct tThread;

typedef struct tStatementNode
{
    struct tStatementNode * left, * right, * unary, // unary and binary operators
            ** args, // functions
            * condition, // for if-else and do-while statements
            * elif, *else_, // next condition statement
            * from, *to, * step; // for .. to .. do
    token_node type;
    // args for functions with multiple parameters
    struct tThread * thread, // a pointer to thread, usually it's a code block.
            * parent_thread; // The parent thread if the current instruction.
    tVar variable; // is it a variable/identifier?
    struct tStatementNode * parent;

    int acount; // # of arguments
} tStatementNode;

typedef struct tThread
{
    int cdef;
    string name; // name is case of proc/func
    tThreadType type;
    string return_type; // a function's returning type
    tStatementNode ** instructions;
    int icount; // # of instructions
    tVar * vars;
    int vcount; // # of constant variables,
    int unlimited_args; // does it support unlimited arguments ex: printf(...)?
    tType * global_types; // pointer to types. Types are always global
    struct tThread * global_functions; // pointer to global parameters
    int body_defined; // defenition or declaration of func/proc
    tVar * params;
    int pcount;
    int if_coumpound; // # of compound if-elif-else

    struct tThread * parent; // a link  between a thread inside an instruction, and the parent thread.
} tThread;

int global_types_count;
tType * global_types;
#define BASIC_TYPES_COUNT 6
char* lib_path;
tThread ** global_functions;
int global_functions_count; // # of defined parameters in case of function/procedure
void start_parse(LexInfo * li);
tThread * global_thread;

#endif
