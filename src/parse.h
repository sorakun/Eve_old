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
struct tThread;
struct tVar;

typedef struct tField
{
    string name;
    struct tType * type;
} tField;



typedef enum tType_kind
{
    __array, __enum, __class, __none, __struct, __pclass, __ctype// pointer to class
} tType_kind;

typedef struct class_
{
    char * name;
    // the name of the pointer to this class ex: class Player; pointer_name = PPlayer = ^ Player.
    char * pointer_name;
    string type; // list of types, for templates
    int key_index; // is the key variable already defined
    struct tVar * variables;
    int vcount; // number of variables
    struct tThread ** methodes;
    int mcount; // number of methods
}class_;

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
    char * pointerto;

    class_ class_info;
} tType;

typedef struct tVar
{
    string name;
    string type;
    tMod mod;
    token_node info;
    struct tThread * reads, * writes; // OOP
    int is_property;
} tVar;

typedef enum tThreadType
{
    _main, _func, _proc, _block, _repeat, _while, _if, _elif, _else, _for, _foreach
} tThreadType;

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

    int member_func; // if = 1, it's a func, if 0 it's a variable. (for OOP)
    string gen_name;
} tStatementNode;

typedef struct tThread
{
    int cdef;
    string name; // name in case of proc/func
    string gen_name; // name of the generated func (OOP)
    tThreadType type;
    string return_type; // a function's returning type
    tStatementNode ** instructions;
    int icount; // # of instructions
    tVar * vars;
    int vcount; // # of constant variables,
    int unlimited_args; // does it support unlimited arguments like printf(...)?
    tType * global_types; // pointer to types. Types are always global
    int body_defined; // defenition or declaration of func/proc
    tVar * params;
    int pcount;
    int if_coumpound; // # of compound if-elif-else

    struct tThread * parent; // a link  between a thread inside an instruction, and the parent thread.
    class_ * parent_class; // parent class, to find member variables.
    token_node info;
} tThread;

int global_types_count;
tType * global_types;
#define BASIC_TYPES_COUNT 6
char* lib_path;
tThread ** global_functions;
int global_functions_count; // # of defined parameters in case of function/procedure
tThread * global_thread;

#endif
