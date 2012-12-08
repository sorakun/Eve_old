/*
 * Eve programming language
 * Lexer header
 * check eve.h for copyright informations
 */

#ifndef EVE_LEX_H
#define EVE_LEX_H

#include "eve.h"

#define MAX_TOKEN_LEN 1024

#define TokensCount sizeof(Tokens)/sizeof(tToken)

typedef enum tTokenType
{
    /* reserved keywords. */
    /* enums starts from -100 because we use the ASCII code of single char tokens to identify them */

    AND = -100, AS, AUTO, BEGIN, BREAK, CASE, CATCH, CLASS, CONST, CONTINUE, DEFAULT, DO,  ELSE,
    ELIF, END, EXTENDS, EXTERN, FALSE, FOR , FUNC, FOREACH, FROM, IF, _IN, INCLUDE, IS,
    LOCAL, NIL, NOT, OR, PROC, PRIVATE, PROTECTED, PUBLIC, REGISTER, REQUIRE,
    RETURN,REPEAT, STEP, STATIC, SWITCH, THROW, TRUE, THEN, TRY, TO, TYPE, UNTIL, USE,
    VAR,  VOLATILE, WHILE, WITH, XOR, OF,

    /* types */
    ARRAY, STRUCT, ENUM,
    /* Others. */
    HEX, NODATA, BOOLEAN, CHAR, CDEF, CIMPORT,
    IDENTIFIER,   INTEGER,  FLOATING,   LITERAL,  COMMENT,   SPACE,  DOUBLEDOT,
    TRIPLEDOT, INC, DEC,  PREINC, POSTINC, PREDEC, POSTDEC, PASS, MASS, MULASS,
    DASS, MODASS, LSASS, RSASS, BAASS, BOASS, BXASS, LEQ, GEQ, EQUAL, NOTEQUAL,
    TCONSTRUCT,  LSHIFT,  RSHIFT, ENDOFFILE, UNBOUNDEDCOMMENT,
    UNBOUNDEDLITERAL,    SCOPE,         INDEX,        UNDEFINED,       UKERROR,

    /* data types */
} tTokenType;

typedef struct tToken
{
    int type;
    string str;
} tToken;

static const tToken Tokens[] =
{
    {AND, "and"},
    {AS, "as"},
    {ARRAY, "array"},
    {AUTO, "auto"},
    {BEGIN, "begin"},
    {BREAK, "break"},
    //{CASE, "case"},
    //{CATCH, "catch"},
    {CDEF, "cdef"},
    {CIMPORT, "cimport"},
    //{CLASS, "class"},
    {CONST, "const"},
    {CONTINUE, "continue"},
    //{DEFAULT, "default"},
    {DO, "do"},
    {ELSE, "else"},
    {ELIF, "elif"},
    {END, "end"},
    {ENUM, "enum"},
    //{EXTENDS, "extends"},
    //{EXTERN, "extern"},
    {FALSE, "false"},
    {FOR, "for"},
    {FUNC, "func"},
    {FOREACH, "foreach"},
    {FROM, "from"},
    {IF, "if"},
    {_IN, "in"},
    {IS, "is"},
    {INCLUDE, "import"},
    {NIL, "nil"},
    {NOT, "not"},
    {OR, "or"},
    {PROC, "proc"},
    //{PRIVATE, "private"},
    //{PROTECTED, "protected"},
    //{PUBLIC, "public"},
    {REPEAT, "repeat"},
    //{REGISTER, "register"},
    {LOCAL, "local"},
    {RETURN, "return"},
    {STEP, "step"},
    {STATIC, "static"},
    //{SWITCH, "switch"},
    {STRUCT, "struct"},
    //{THROW, "throw"},
    {THEN, "then"},
    {TRUE, "true"},
    {TRY, "try"},
    {TO, "to"},
    {TYPE, "type"},
    {UNTIL, "until"},
    {VAR, "var"},
    //{VOLATILE, "volatile"},
    {WHILE, "while"},
    //{WITH, "with"},
    {XOR, "xor"},
    {OF, "of"},
    {TRIPLEDOT, "..."},
    {DOUBLEDOT, ".."},
    {INC, "++"},
    {DEC, "--"},
    {PASS, "+="},
    {MASS, "-="},
    {MULASS, "*="},
    {DASS, "/="},
    {MODASS, "%="},
    {LSASS, "<<="},
    {RSASS, ">>="},
    {BAASS, "&="},
    {BOASS, "|="},
    {BXASS, "^="},
    {LEQ, "<="},
    {GEQ, ">="},
    {EQUAL, "=="},
    {NOTEQUAL, "!="},
    {LSHIFT, "<<"},
    {RSHIFT, ">>"},
    {'@', "@"},
    /* single-character tokens */
    {'(', "("}, {')', ")"}, {'[', "["}, {']', "]"},
    {'{', "{"}, {'}', "}"}, {'.', "."}, {',', ","},
    {';', ";"}, {'+', "+"},
    {'-', "-"}, {'*', "*"}, {'/', "/"}, {'%', "%"},
    {'=', "="}, {'<', "<"}, {'>', ">"}, {'?', "?"},
    {'!',"!"}, {'|',"|"}, {'^',"^"}, {':',":"},
    {'~', "~"},
};

typedef struct _token_node
{
    tTokenType TT;
    string    str;
    int  expected;
    long pos;
    long line_num;
    int level; // how deep it is inside parentheses
    string source;
} token_node;


typedef struct LexInfo
{
    token_node * TokenInfo; /* informations of the scanned tokens */
    long size;
    string source;        /* the source / path of the script */
    string input;
    long line_num;
    long pos;             /* the postion in the stream */
    long line_pos;         /* position in the current line */
    int level; // how deep it is inside paretheses
} LexInfo;

int id_is_keyword(string id);

string token_get_string(int token);

int is_valid_hex(char x);

token_node Lex_Step(LexInfo * li);

LexInfo Lex_Begin(const char * src, const char * input);

#endif
