/*
 * Eve programming language
 * Parser
 * check eve.h for copyright informations
 */

#include <stdlib.h>
#include <string.h>

#include "eve.h"
#include "error.h"
#include "lex.h"
#include "parse.h"
#include "types.h"

#include "threads.h"
#include "includes.h"



void initparser(LexInfo * li, tThread * thread)
{
    li->pos=0;
}

int op_is_unary(token_node token)
{
    switch (token.TT)
    {
    case '@':
    case NOT:
    case RETURN:
    case '^':
    case '-':
    case INC:
    case DEC:
        return 1;
    default:
        return 0;
    }
}

int is_constant(token_node token)
{
    return token.TT == LITERAL || token.TT == FLOATING || token.TT == INTEGER || token.TT == LITERAL ;
}

int token_is_op(token_node token)
{
    debugf("[token_is_op] testing %s\n", token.str)  ;
    switch (token.TT)
    {
    case INC:
    case DEC:
    case PASS:
    case MASS:
    case MULASS:
    case DASS:
    case MODASS:
    case LSASS:
    case RSASS:
    case BAASS:
    case BOASS:
    case BXASS:
    case LEQ:
    case GEQ:
    case EQUAL:
    case NOTEQUAL:
    case LSHIFT:
    case RSHIFT:
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '<':
    case '>':
    case '@':
    case OR:
    case AND:
    case NOT:
    case _IN:
    case XOR:
    case '=':
    case '^':
    case ':':
    case '.':
    case STATIC_CALL:
    case DYN_CALL:
    case RETURN:
        return 1;
    default:
        return 0;

    }
}

string currenttokenstring(LexInfo * li)
{
    return li->TokenInfo[li->pos].str;
}

// possible tokens to find in instructions:
int accept_token_in_instruction(token_node token)
{
    if(token_is_op(token))
        return 1;
    switch (token.TT)
    {
    case FALSE:
    case TRUE:
    case IDENTIFIER:
        return 1;
    default:
        return 0;

    }
}

int break_in_loop(tThread * thread)
{
    if(thread == NULL)
        return 0;
    if(thread->type == _while || thread->type == _for || thread->type == _foreach || thread->type == _repeat)
        return 1;
    else
        return break_in_loop(thread->parent);
}

tThread * get_parent_func(tThread * thread)
{
    if(thread == NULL)
        return NULL;
    if(thread->type == _func || thread->type == _proc)
        return thread;
    else
        return get_parent_func(thread->parent);
}

int get_closed_token(tTokenType token)
{
    switch(token)
    {
    case '(':
        return ')';
    case '[':
        return ']';
    case '{':
        return '}';
    }
}

// returns the position of the 1st occrence of a token in one level
// supported nested tokens like ( or [ or {
//  Lex table, starting  from li->pos.
int lex_find_token_pos(LexInfo * li, tTokenType token, int max_pos)
{
    int i = li->pos;
    int x = 0;
    if(token == '(' || token == '['|| token == '{')
    {
        for(; i <= max_pos; i++)
            if(li->TokenInfo[i].TT == token)
                x++;
            else if((li->TokenInfo[i].TT == get_closed_token(token)) && x != 0)
                x--;
            else if ((li->TokenInfo[i].TT == get_closed_token(token)) && x == 0)
                return i;
    }
    else
        for(; i <= max_pos; i++)
        {
            if((li->TokenInfo[i].TT == token) && (x ==0))
                return i;
            else if(li->TokenInfo[i].TT == '(' || li->TokenInfo[i].TT == '['|| li->TokenInfo[i].TT == '{')
                x++;
            else if((li->TokenInfo[i].TT == ')' || li->TokenInfo[i].TT == ']'|| li->TokenInfo[i].TT == '}') && x == 0)
                return -1;
            else if (li->TokenInfo[i].TT == ')' || li->TokenInfo[i].TT == ']'|| li->TokenInfo[i].TT == '}')
                x--;

        }
    return -1;
}

int priorityof(token_node t)
{
    switch (t.TT)
    {
    case '=':
        return 15;

    case AND:
        return 13;

    case OR:
        return 12;

    case EQUAL:
    case NOTEQUAL:
        return 9;

    case '<':
    case '>':
    case GEQ:
    case LEQ:
        return 8;

    case '+':
        return 6;

    case '*':
    case '/':
    case '%':
        return 5;

    case '.':
    case STATIC_CALL:
    case DYN_CALL:
        return 4;

    case '-':
    case '^':
    case '@':
    case ':':
        return 3;

    default:
        return 0;
    }
}

// return 1 if t1 is less prior than t2
int compare_op(token_node t1, token_node t2)
{
    if(t1.level < t2.level || func_is_defined(t2.str, _func))
        return 1;
    if (t1.level > t2.level || func_is_defined(t1.str, _func))
        return 0;
    if(priorityof(t1)<priorityof(t2))
        return 0;
    else
        return 1;
}


tTokenType nexttoken(LexInfo * li)
{
    debugf("[nexttoken] is %s\n", li->TokenInfo[li->pos+1].str);
    return li->TokenInfo[li->pos+1].TT;
}

tTokenType currenttoken(LexInfo * li)
{
    debugf("[current token] at pos %d\n", li->pos);
    return li->TokenInfo[li->pos].TT;
}

/*
 * Most hardcoded function; Generate instruction.
 * li: LexInfo
 * start: start position of parsing element
 * end: end position
 * thread: parent thread
 * p_instruction: parent instruction (first call should be NULL).
*/
tStatementNode * gen_instruction(LexInfo * li, int start, int end, tThread * thread, tStatementNode * p_intstruction)
{
    debugf("[gen_instruction] from %d to %d\n", start, end);
    // keep doing
    int min_pos = -1, i = start;
    int op_count = 0;
    int tmp_pos1, tmp_pos2;
    int token_type = -1;
    tStatementNode * tmp = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    tmp->parent = p_intstruction;
    tmp->parent_thread = thread;
    for (; i <= end; i++)
    {
        int test = is_class_func(li->TokenInfo[i].str);
        debugf("test = %s %d\n", li->TokenInfo[i].str, test);
        if(token_is_op(li->TokenInfo[i]) || func_is_defined(li->TokenInfo[i].str, _func) || func_is_defined(li->TokenInfo[i].str, _proc) || (test != 0))
        {
            // case of operation
            if(token_is_op(li->TokenInfo[i]))
            {
                debugf("token %s is op\n", li->TokenInfo[i].str);
                if(token_type == 0)
                {
                    if(!op_is_unary(li->TokenInfo[i]))
                        eve_syntax_error_expected_before(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[i].line_num, li->TokenInfo[i-1].pos, ';', li->TokenInfo[i].str);
                }
                else
                    token_type = 0;
            }
            else
            {
                if(token_type == 1)
                    eve_syntax_error_expected_before(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[i].line_num, li->TokenInfo[i-1].pos, ';', li->TokenInfo[i].str);
                else
                    token_type = 1;
            }
            op_count++;
            if(min_pos == -1)
                min_pos = i;
            else if(!compare_op(li->TokenInfo[min_pos], li->TokenInfo[i]))
                min_pos = i;
        }
        else if(li->TokenInfo[i].TT == IDENTIFIER)
        {
            debugf("token %s is ID\n", li->TokenInfo[i].str);
            if((!var_is_defined(li->TokenInfo[i].str, thread, 1)) && (!enum_is_defined(li->TokenInfo[i].str) && (li->TokenInfo[i-1].TT != '.') && (li->TokenInfo[i-1].TT != DYN_CALL)))
            {
                if ((li->TokenInfo[i-1].TT == '.') ||(li->TokenInfo[i-1].TT == DYN_CALL))
                {
                    tType t = find_type(li->TokenInfo[i-2].str);
                    debugf("well %s\n", t.name);
                    if(t.name == NULL)
                        eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, identifiers %s is not a class/struct.\n",
                                         li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, li->TokenInfo[i-2].str);
                }
                else
                    eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, identifier %s is not defined.\n",
                                     li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, li->TokenInfo[i].str);
            }
            if(token_type == 2)
                eve_syntax_error_expected_before(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[i].line_num, li->TokenInfo[i-1].pos, ';', li->TokenInfo[i].str);
            token_type = 2;
        }
        else
            token_type = -1;
    }
    debugf("[gen_instruction]op count = %d\n", op_count);
    // stop recursion when op_count = 0;
    if(op_count == 0)
    {
        // do adequate things it can be constant, string, record, function ...
        for(i=start; i<=end; i++)
            if((li->TokenInfo[i].TT != '(') && (li->TokenInfo[i].TT != ')'))
                break;
        debugf("[tStatement node] leaf token is %s\n", li->TokenInfo[i].str);
        tmp->type = li->TokenInfo[i];

        return tmp;
    }
    else
    {
        debugf("[tStatement node] min prior operation is: %s %c\n",li->TokenInfo[min_pos].str, li->TokenInfo[min_pos].TT);
        int func = 0;
        int v;
        if(token_is_op(li->TokenInfo[min_pos]))
        {
            debugf("[geninstruction] op %s.\n", li->TokenInfo[start].str);
            tmp->type  = li->TokenInfo[min_pos];

            if (op_is_unary(tmp->type))
            {
                if(!(tmp->type.TT == '-' && (end - start != 1)))
                {
                    tmp->unary = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
                    tmp->unary = gen_instruction(li, min_pos+1, end, thread, tmp);
                    tmp->parent = thread;
                    return tmp;
                }
            }

            tmp->left  = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
            tmp->right = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
            tmp->left  = gen_instruction(li, start, min_pos-1, thread, tmp);
            tmp->right = gen_instruction(li, min_pos+1, end, thread, tmp);
            return tmp;
        }
        if((func = func_is_defined(li->TokenInfo[min_pos].str, _func)) || (func_is_defined(li->TokenInfo[min_pos].str, _proc)) ||
                (0 != (v = is_class_func(li->TokenInfo[min_pos].str))))
        {
            tmp->type = li->TokenInfo[min_pos];
            if (func)
                tmp->type.TT = FUNC;
            else
                tmp->type.TT = PROC;
            //tmp->thread = ...
            tmp->acount = 0;
            debugf("[geninstruction] func %s is defined.\n", li->TokenInfo[min_pos].str);
            if(li->TokenInfo[min_pos+1].TT == '(')
            {
                if (li->TokenInfo[min_pos+2].TT == ')')
                    eve_syntax_error_expected_before(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->size-1].line_num,
                                                     li->TokenInfo[li->size-1].pos, IDENTIFIER, ")");

                tmp->args = (tStatementNode**)eve_malloc(sizeof(tStatementNode*));
                tmp_pos1 = min_pos+2;
                do
                {
                    li->pos = tmp_pos1;
                    debugf("[geninstruction] generating parameter number %d\n", tmp->acount) ;
                    tmp->args[tmp->acount] = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
                    tmp_pos2 = lex_find_token_pos(li, ',', li->size-1);
                    if(tmp_pos2 == -1)
                        // one param remaining
                        tmp_pos2 = lex_find_token_pos(li, ')', li->size-1);
                    if(tmp_pos2 == -1)
                        eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->size-1].line_num,
                                                  li->TokenInfo[li->pos].pos, ')'/*, "EOF"*/);
                    tmp->args[tmp->acount] = gen_instruction(li, tmp_pos1, tmp_pos2-1, thread, tmp);
                    tmp->acount++;
                    tmp_pos1 = tmp_pos2+1;

                }
                while(tmp_pos2 != lex_find_token_pos(li, ')', li->size-1));
                li->pos = tmp_pos1;
            }
            debugf("[geninstruction] generating parameters ended, pcount %d\n", tmp->acount) ;
            return tmp;
        }
    }
}

void match(LexInfo * li, tTokenType type)
{
    debugf("[match] looking for %s at pos %d, it's %s\n", token_get_string(type), li->pos, currenttokenstring(li));
    if(li->pos >= li->size)
    {
        eve_custom_error(EVE_WRONG_TOKEN_EXPECED, "Syntax error: file: '%s', %s expeced before EOF.", li->source, token_get_string(type));
    }
    if (li->TokenInfo[li->pos].TT != type)
        eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, type);
    debugf("[match] %s founded at pos %d\n", token_get_string(type), li->pos);
}

void lexstep(LexInfo * li)
{
    li->pos++;
    debugf("[lexstep] new pos = %d\n", li->pos);
}

int name_is_unique(string name, tThread * thread, int b)
{
    if (var_is_defined(name, thread, b) || type_is_defined(name) || func_is_defined(name, _func) || func_is_defined(name, _proc) || enum_is_defined(name))
        return 0;
    return 1;
}

/*
 * block ::= 'begin' <stmt> 'end'
 * block ::= 'begin' 'end'
 */
void doblock(LexInfo * li, tThread * thread)
{
    token_node current_token = li->TokenInfo[li->pos];
    lexstep(li);
    debugf("[doblock] new pos = %d\n", li->pos);

    thread->instructions[thread->icount] = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->type = current_token;
    thread->instructions[thread->icount]->thread = create_thread(_block, thread, "block", NULL, NULL, 0, 0, 0, 0);

    if(currenttoken(li) == END)
        eve_warning("file: '%s', line: %d, pos: %d, empty block founded.",
                    li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos);
    while(currenttoken(li)!= END)
        dostmt(li, thread->instructions[thread->icount]->thread);
    thread->icount++;
    match(li, END);
    lexstep(li);
}

/*
 * <if stmt> ::= 'if'   <Expr> 'then' <Stmt> 'end'
 * <if stmt> ::= <if stmt> 'elif' <Expr> 'then' <Stmt> 'end'
 * <if stmt> ::= <if stmt> 'else' <Stmt> 'end'
 * <if stmt> ::= <if stmt> 'elif' <Stmt> 'else' <Stmt> 'end'
 */

void doelse(LexInfo * li, tThread * thread)
{
    debugf("[doelse] new pos = %d\n", li->pos);
    tTokenType t = currenttoken(li);

    token_node current_token = li->TokenInfo[li->pos];
    lexstep(li);
    thread->instructions[thread->icount] = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->type = current_token;
    thread->instructions[thread->icount]->thread = create_thread(_else, thread, "else", NULL, NULL, 0, 0, 0, 0);
    while(currenttoken(li)!=END)
        dostmt(li, thread->instructions[thread->icount]->thread);
    thread->icount++;
    match(li, END);
}

void doelif(LexInfo * li, tThread * thread)
{
    debugf("[doelif] new pos = %d\n", li->pos);
    tTokenType t = currenttoken(li);

    token_node current_token = li->TokenInfo[li->pos];
    lexstep(li);
    int tmp_pos = lex_find_token_pos(li, THEN, li->size-1);
    if(tmp_pos == -1)
        eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, THEN);
    thread->instructions[thread->icount] = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->type = current_token;
    thread->instructions[thread->icount]->condition = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->condition = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);

    li->pos = tmp_pos;
    match(li, THEN);
    lexstep(li);
    thread->instructions[thread->icount]->thread = create_thread(_if, thread, "elif", NULL, NULL, 0, 0, 0, 0);
    while(currenttoken(li)!=ELIF && currenttoken(li)!=ELSE && currenttoken(li)!=END)
        dostmt(li, thread->instructions[thread->icount]->thread);
    thread->icount++;
    if(currenttoken(li) == ELIF)
        doelif(li, thread);
    else if (currenttoken(li) == ELSE)
        doelse(li, thread);
    else
        match(li, END);
    thread->if_coumpound--;
}

void doif(LexInfo * li, tThread * thread)
{
    debugf("[doif] new pos = %d\n", li->pos);
    tTokenType t = currenttoken(li);

    token_node current_token = li->TokenInfo[li->pos];
    lexstep(li);
    thread->if_coumpound++;
    int tmp_pos = lex_find_token_pos(li, THEN, li->size-1);
    if(tmp_pos == -1)
        eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, THEN);
    thread->instructions[thread->icount] = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->type = current_token;
    thread->instructions[thread->icount]->condition = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->condition = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);
    match_type(thread->instructions[thread->icount]->condition, "bool", thread);

    li->pos = tmp_pos;
    match(li, THEN);
    lexstep(li);
    thread->instructions[thread->icount]->thread = create_thread(_if, thread, "if", NULL, NULL, 0, 0, 0, 0);
    while(currenttoken(li)!=ELIF && currenttoken(li)!=ELSE && currenttoken(li)!=END)
    {
        debugf("[doif] not elif nor else nor end it's %s\n", currenttokenstring(li));
        dostmt(li, thread->instructions[thread->icount]->thread);
    }
    thread->icount++;
    if(currenttoken(li) == ELIF)
        doelif(li, thread);
    else if (currenttoken(li) == ELSE)
        doelse(li, thread);
    else
        match(li, END);
    thread->if_coumpound--;
    lexstep(li);
}

void dofor(LexInfo * li, tThread * thread)
{
    debugf("[dofor] new pos = %d\n", li->pos);
    tTokenType t = currenttoken(li);
    token_node current_token = li->TokenInfo[li->pos];
    lexstep(li);
    int tmp_pos = lex_find_token_pos(li, TO, li->size-1);
    if(tmp_pos == -1)
        eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, TO);
    thread->instructions[thread->icount] = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->type = current_token;
    thread->instructions[thread->icount]->from = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->from = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);
    li->pos = tmp_pos;
    lexstep(li);
    tmp_pos = lex_find_token_pos(li, DO, li->size-1);
    if(tmp_pos == -1)
        eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, DO);
    thread->instructions[thread->icount]->to = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->to = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);
    li->pos = tmp_pos;
    // step is optional
    /*if(currenttoken(li) == STEP)
    {
        tmp_pos = lex_find_token_pos(li, DO, li->size-1);
        if(tmp_pos == -1)
            eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, TO);
        thread->instructions[thread->icount] = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
        thread->instructions[thread->icount]->type = current_token;
        thread->instructions[thread->icount]->from = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
        thread->instructions[thread->icount]->from = gen_instruction(li, li->pos, tmp_pos-1);
        li->pos = tmp_pos;
        lexstep(li);
    }*/
    match(li, DO);
    lexstep(li);
    thread->instructions[thread->icount]->thread = create_thread(_for, thread, "for", NULL, NULL, 0, 0, 0, 0);
    while(currenttoken(li)!=END)
        dostmt(li, thread->instructions[thread->icount]->thread);
    thread->icount++;
    match(li, END);
    lexstep(li);
}

// <repeat stmt> :== 'repeat' <Stmt> 'until' <Expr> ';'
void dorepeat(LexInfo * li, tThread * thread)
{
    debugf("[dorepeat] new pos = %d\n", li->pos);
    tTokenType t = currenttoken(li);

    token_node current_token = li->TokenInfo[li->pos];
    lexstep(li);
    thread->instructions[thread->icount] = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->thread = create_thread(_repeat, thread, "repeat", NULL, NULL, 0, 0, 0, 0);
    while(currenttoken(li)!=UNTIL)
        dostmt(li, thread->instructions[thread->icount]->thread);
    match(li,UNTIL);
    lexstep(li);
    int tmp_pos = lex_find_token_pos(li, ';', li->size-1);
    if(tmp_pos == -1)
        eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, ';');
    thread->instructions[thread->icount]->type = current_token;
    thread->instructions[thread->icount]->condition = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->condition = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);
    match_type(thread->instructions[thread->icount]->condition, "bool", thread);
    li->pos = tmp_pos;
    thread->icount++;
    lexstep(li);
}

void dowhile(LexInfo * li, tThread * thread)
{
    debugf("[dowhile] new pos = %d\n", li->pos);
    tTokenType t = currenttoken(li);

    token_node current_token = li->TokenInfo[li->pos];
    lexstep(li);
    int tmp_pos = lex_find_token_pos(li, DO, li->size-1);
    if(tmp_pos == -1)
        eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, DO);
    thread->instructions[thread->icount] = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->type = current_token;
    thread->instructions[thread->icount]->condition = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->condition = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);

    match_type(thread->instructions[thread->icount]->condition, "bool", thread);

    li->pos = tmp_pos;
    match(li, DO);
    lexstep(li);
    thread->instructions[thread->icount]->thread = create_thread(_while, thread, "while", NULL, NULL, 0, 0, 0, 0);
    while(currenttoken(li)!=END)
        dostmt(li, thread->instructions[thread->icount]->thread);
    thread->icount++;
    match(li, END);
    lexstep(li);
}

// <Var Decl> ::= <Mod> <Type> <Var> <Var List>  ';'
//              |       <Type> <Var> <Var List>  ';'
void dovariable(LexInfo * li, tThread * thread)
{
    debugf("[dovariable]: \"%s\" in thread %s\n", currenttokenstring(li), thread->name);
    if(currenttoken(li) == IDENTIFIER)
    {

        // a type? means it's a variable declaration
        tMod tmp_mod = _none;
        int is_pointer = 0;
        if (is_mod(li->TokenInfo[li->pos]))
        {
            if(currenttoken(li) == VAR)
                eve_custom_error(EVE_WRONG_MOD_EXPECTED, "file: '%s', line: %d, pos: %d, can not define a variable with mod 'var'.\n",
                                 li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos);

            tmp_mod = token_to_mod(li->TokenInfo[li->pos]);
            lexstep(li);
        }
        match(li, IDENTIFIER);
        if(type_is_defined(currenttokenstring(li)))
        {
            string id = strdup(currenttokenstring(li));
l:
            lexstep(li);
            debugf("[dovariable]: \"%s\"\n", currenttokenstring(li));
            match(li, IDENTIFIER);
            int b = 1; // if local do not check in parents
            if(tmp_mod == _local)
                b = 0;
            if (!name_is_unique(currenttokenstring(li), thread, b))
                eve_custom_error(EVE_IDENTIFIER_REDEFINITON, "file: '%s', line: %d, pos: %d, identifier %s is already defined.\n",
                                 li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
            else
                register_variable(currenttokenstring(li), id, is_pointer, tmp_mod, thread);
            /* index operator */
            if (nexttoken(li) == ':')
            {
                int p = li->pos+2;
                if (li->TokenInfo[p].TT != INTEGER)
                    eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, right operator of index must be integer.\n",
                                     li->source, li->TokenInfo[p].line_num, li->TokenInfo[p].pos);

                int tmp_pos = lex_find_token_pos(li, ';', li->size-1);
                if(tmp_pos == -1)
                    eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, ';');
                thread->instructions[thread->icount] = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);
                thread->icount++;

                li->pos = tmp_pos;
            }
            else if(nexttoken(li) == '=' )
            {
                int tmp_pos = lex_find_token_pos(li, ',', li->size-1);
                if(tmp_pos == -1)
                    tmp_pos = lex_find_token_pos(li, ';', li->size-1);

                if(tmp_pos == -1)
                    eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, ';');
                thread->instructions[thread->icount] = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);
                string tname = get_op_type(thread->instructions[thread->icount]->left, thread);
                string type = find_type(tname).name;
                if(type == NULL)
                    eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, type of \"%s\"is not defined.\n",
                                     li->source, thread->instructions[thread->icount]->left->type.line_num,
                                     thread->instructions[thread->icount]->left->type.pos, thread->instructions[thread->icount]->left->type.str);
                match_type(thread->instructions[thread->icount]->right, type, thread);
                thread->icount++;
                li->pos = tmp_pos-1;
            }
            else if((nexttoken(li)=='.') || (nexttoken(li)== DYN_CALL))
            {
                int tmp_pos = lex_find_token_pos(li, ';', li->size-1);
                if(tmp_pos == -1)
                    eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, ';');
                thread->instructions[thread->icount] = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);
                string tname = get_op_type(thread->instructions[thread->icount]->left, thread);
                string type = find_type(tname).name;
                if(type == NULL)
                    eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, type of \"%s\"is not defined.\n",
                                     li->source, thread->instructions[thread->icount]->left->type.line_num,
                                     thread->instructions[thread->icount]->left->type.pos, thread->instructions[thread->icount]->left->type.str);
                if (thread->instructions[thread->icount]->type.TT == '=')
                    match_type(thread->instructions[thread->icount]->right, type, thread);
                else
                    match_type(thread->instructions[thread->icount], "void", thread);

                thread->icount++;
                li->pos = tmp_pos;
            }
            else
                eve_warning("file: '%s', line: %d, pos: %d, variable not initialized.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos);
            lexstep(li);
            debugf("[dovariable]: now is %s\n", currenttokenstring(li));
            if(currenttoken(li) == ',')
                goto l;
        }
        else if(var_is_defined(currenttokenstring(li), thread, 1))
        {
            if (nexttoken(li) == ':')
            {
                int p = li->pos+2;
                if (li->TokenInfo[p].TT != INTEGER)
                    eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, right operator of index must be integer.\n",
                                     li->source, li->TokenInfo[p].line_num, li->TokenInfo[p].pos);

                int tmp_pos = lex_find_token_pos(li, ';', li->size-1);
                if(tmp_pos == -1)
                    eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, ';');

                thread->instructions[thread->icount] = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);

                string tname = get_op_type(thread->instructions[thread->icount]->left, thread);
                string type = find_type(tname).name;
                match_type(thread->instructions[thread->icount]->right, type, thread);
                thread->icount++;
                li->pos = tmp_pos;
            }
            else if(nexttoken(li) == '=')
            {
                int tmp_pos = lex_find_token_pos(li, ';', li->size-1);
                if(tmp_pos == -1)
                    eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, ';');
                thread->instructions[thread->icount] = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);
                string tname = get_op_type(thread->instructions[thread->icount]->left, thread);
                debugf("type is %s\n", tname);
                string type = find_type(tname).name;
                if(type == NULL)
                    eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, type of \"%s\"is not defined.\n",
                                     li->source, thread->instructions[thread->icount]->left->type.line_num,
                                     thread->instructions[thread->icount]->left->type.pos, thread->instructions[thread->icount]->left->type.str);

                match_type(thread->instructions[thread->icount]->right, type, thread);
                thread->icount++;

                li->pos = tmp_pos;
            }
            else if((nexttoken(li)=='.') || (nexttoken(li)== DYN_CALL))
            {
                int tmp_pos = lex_find_token_pos(li, ';', li->size-1);
                if(tmp_pos == -1)
                    eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, ';');
                thread->instructions[thread->icount] = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);
                string tname = get_op_type(thread->instructions[thread->icount]->left, thread);
                string type = find_type(tname).name;
                if(type == NULL)
                    eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, type of \"%s\"is not defined.\n",
                                     li->source, thread->instructions[thread->icount]->left->type.line_num,
                                     thread->instructions[thread->icount]->left->type.pos, thread->instructions[thread->icount]->left->type.str);
                if (thread->instructions[thread->icount]->type.TT == '=')
                    match_type(thread->instructions[thread->icount]->right, type, thread);
                else
                    match_type(thread->instructions[thread->icount], "void", thread);

                thread->icount++;
                li->pos = tmp_pos;
            }
            else
                eve_warning("file: '%s', line: %d, pos: %d, assignment expression expected.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos);

        }
        else if (func_is_defined(currenttokenstring(li), _proc))
        {
            int tmp_pos = lex_find_token_pos(li, ';', li->size-1);
            debugf("tmp_pos = %d\n", tmp_pos);
            if(tmp_pos == -1)
                eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, ';');
            thread->instructions[thread->icount] = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);

            tThread * fn = find_func(thread->instructions[thread->icount]->type.str, _proc);

            if (fn->pcount > thread->instructions[thread->icount]->acount)
                eve_custom_error(EVE_WRONG_PARAM_NUMBER, "file: '%s', line: %d, pos: %d, parameters count of function %s is different from the definition",
                                 thread->instructions[thread->icount]->type.source, thread->instructions[thread->icount]->type.line_num, thread->instructions[thread->icount]->type.pos, thread->instructions[thread->icount]->type.str);
            if ((fn->unlimited_args==0) && (fn->pcount < thread->instructions[thread->icount]->acount))
                eve_custom_error(EVE_WRONG_PARAM_NUMBER, "file: '%s', line: %d, pos: %d, parameters count of function %s is different from the definition",
                                 thread->instructions[thread->icount]->type.source, thread->instructions[thread->icount]->type.line_num, thread->instructions[thread->icount]->type.pos, thread->instructions[thread->icount]->type.str);
            int i = 0;
            debugf("[func_is_defined]: func %s have %d param\n", fn->name, fn->pcount);
            for (; i < fn->pcount; i++)
            {
                debugf("[func_is_defined]: param %d is %s\n", i, fn->params[i].type);
                match_type(thread->instructions[thread->icount]->args[i], fn->params[i].type, thread);
            }

            for(i = fn->pcount; i < thread->instructions[thread->icount]->acount;  i++)
            {
                debugf("param %d\n", i);
                string mytype = get_op_type(thread->instructions[thread->icount]->args[i], thread);
                match_type(thread->instructions[thread->icount]->args[i], mytype, thread);
            }

            thread->icount++;
            li->pos = tmp_pos;
        }
        else if (func_is_defined(currenttokenstring(li), _func))
        {
            debugf("epic code is epic\n");
            int tmp_pos = lex_find_token_pos(li, ';', li->size-1);
            eve_warning("file: '%s', line: %d, pos: %d, function \"%s\" is used as a statement, not inside an expression/instruction.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));

            debugf("tmp_pos = %d\n", tmp_pos);
            if(tmp_pos == -1)
                eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, ';');
            thread->instructions[thread->icount] = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);

            tThread * fn = find_func(thread->instructions[thread->icount]->type.str, _func);

            if (fn->pcount > thread->instructions[thread->icount]->acount)
                eve_custom_error(EVE_WRONG_PARAM_NUMBER, "file: '%s', line: %d, pos: %d, parameters count of function %s is different from the definition",
                                 thread->instructions[thread->icount]->type.source, thread->instructions[thread->icount]->type.line_num, thread->instructions[thread->icount]->type.pos, thread->instructions[thread->icount]->type.str);
            if ((fn->unlimited_args==0) && (fn->pcount < thread->instructions[thread->icount]->acount))
                eve_custom_error(EVE_WRONG_PARAM_NUMBER, "file: '%s', line: %d, pos: %d, parameters count of function %s is different from the definition",
                                 thread->instructions[thread->icount]->type.source, thread->instructions[thread->icount]->type.line_num, thread->instructions[thread->icount]->type.pos, thread->instructions[thread->icount]->type.str);
            int i = 0;
            debugf("[func_is_defined]: func %s have %d param\n", fn->name, fn->pcount);
            for (; i < fn->pcount; i++)
            {
                debugf("[func_is_defined]: param %d is %s\n", i, fn->params[i].type);
                match_type(thread->instructions[thread->icount]->args[i], fn->params[i].type, thread);
            }

            for(i = fn->pcount; i < thread->instructions[thread->icount]->acount;  i++)
            {
                debugf("param %d\n", i);
                string mytype = get_op_type(thread->instructions[thread->icount]->args[i], thread);
                match_type(thread->instructions[thread->icount]->args[i], mytype, thread);
            }

            if ((thread->instructions[thread->icount]->type.TT == '.') || (thread->instructions[thread->icount]->type.TT == DYN_CALL))
            {
                string tname = get_op_type(thread->instructions[thread->icount], thread);
                string type = find_type(tname).name;
                if(type == NULL)
                    eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, type of \"%s\"is not defined.\n",
                                     li->source, thread->instructions[thread->icount]->left->type.line_num,
                                     thread->instructions[thread->icount]->left->type.pos, thread->instructions[thread->icount]->left->type.str);

                match_type(thread->instructions[thread->icount], type, thread);
            }

            thread->icount++;
            li->pos = tmp_pos;
        }
        // else if it's a proc or function call.
        // or it's an invalid identifier
        else
        {
            // identifier redefinition
            eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, identifier %s is not defined.\n",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
        }
    }
    else
    {
        int tmp_pos = lex_find_token_pos(li, ';', li->size-1);

        if(tmp_pos == -1)
            eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, ';');
        thread->instructions[thread->icount] = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);
        string tname = get_op_type(thread->instructions[thread->icount]->left, thread);
        string type = find_type(tname).name;
        if (token_is_op(thread->instructions[thread->icount]->type))
            switch(thread->instructions[thread->icount]->type.TT)
            {
            case '=':
                match_type(thread->instructions[thread->icount]->right, type, thread);
                break;
            case '.':
            case DYN_CALL:
                match_type(thread->instructions[thread->icount], "void", thread);
                break;

            default:
                eve_custom_error(EVE_INVALID_OPERATION, "file: '%s', line: %d, pos: %d, invalid operation '%s' assignment or method call expected.\n",
                                 thread->instructions[thread->icount]->type.source, thread->instructions[thread->icount]->type.line_num,
                                 thread->instructions[thread->icount]->type.pos, thread->instructions[thread->icount]->type.str);
            }

        if(type == NULL)
            eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, type of \"%s\"is not defined.\n",
                             li->source, thread->instructions[thread->icount]->left->type.line_num,
                             thread->instructions[thread->icount]->left->type.pos, thread->instructions[thread->icount]->left->type.str);
        thread->icount++;
        li->pos = tmp_pos;
    }
    match(li, ';');
    lexstep(li);
}

void parse_args(LexInfo * li, tVar * args, int * count, int * u_args)
{
    string tmpType, tmpName;
    args = (tVar*)eve_malloc(sizeof(tVar));
l:
    {
        tMod tmpmod = _none;
        lexstep(li);
        if(currenttoken(li) == TRIPLEDOT)
        {
            *u_args=1;
        }
        else
        {
            (*count)++;
            debugf("count = %d\n", *count);
            if (is_mod(li->TokenInfo[li->pos]))
            {
                if (currenttoken(li) == VAR)
                    tmpmod = _var;
                else if (currenttoken(li) == CONST)
                    tmpmod = _const;
                else
                    eve_custom_error(EVE_WRONG_MOD_EXPECTED, "file: '%s', line: %d, pos: %d, mod '%s' is not valid in parameters.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                lexstep(li);
                match(li, IDENTIFIER);
            }
            if (!type_is_defined(currenttokenstring(li)))
                eve_custom_error(EVE_UNDEFINED_DATA_TYPE, "file: '%s', line: %d, pos: %d, type '%s' is not defined.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
            tmpType = strdup(currenttokenstring(li));
            lexstep(li);
            match(li, IDENTIFIER);
            if ((!type_is_defined(currenttokenstring(li)) || (!is_mod(li->TokenInfo[li->pos]))))
            {
                tmpName = currenttokenstring(li);
                debugf("current name := %d : %s\n", *(count)-1, currenttokenstring(li));
                args = (tVar*)eve_realloc(args, (*count)*sizeof(tVar));
                args[*(count)-1].mod = tmpmod;
                args[*(count)-1].name = strdup(currenttokenstring(li));
                args[*(count)-1].type = strdup(tmpType);
            }
            if(nexttoken(li) == ',')
            {
                lexstep(li);
                goto l;
            }
        }
    }
}

/*
 * <Func Proto> ::= 'func' <type> <id> '(' <Params> ')' ';'
 *                | 'func' <type> <id> ';'
 */
void dofunction(LexInfo * li, tThread * thread)
{
    // current token is "func" or "proc"
    tThreadType current_type;
    if (currenttoken(li) == FUNC)
        current_type = _func;
    else
        current_type = _proc;

    lexstep(li);
    debugf("[dofunc] new pos = %d\n", li->pos);
    match(li, IDENTIFIER);
    string ttype;
    token_node tmp_token;
    if (current_type == _func)
    {

        if (!type_is_defined(currenttokenstring(li)))
            eve_custom_error(EVE_UNKNOWN_DATA_TYPE, "file: '%s', line: %d, pos: %d, identifier '%s' is not a valid data type.",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
        ttype = strdup(currenttokenstring(li));
        tmp_token = li->TokenInfo[li->pos];
        lexstep(li);
    }
    else
        ttype = strdup("void");

    debugf("type is = %s\n", ttype);
    match(li, IDENTIFIER);

    if (nexttoken(li) == '.')
    {
        // class
        tType T = find_type(currenttokenstring(li));
        token_node info = li->TokenInfo[li->pos];
        lexstep(li);
        if (T.type_kind != __class)
            eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, identifier '%s' is not a valid class.",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
        lexstep(li);
        match(li, IDENTIFIER);
        string name = strdup(currenttokenstring(li));
        int pos = is_member_func(name, &T.class_info);
        if(pos == -1)
            eve_custom_error(Eve_INVALID_CLASS_METHOD, "file: '%s', line: %d, pos: %d, class '%s' has no method '%s'.",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, T.name, name);
        tThread * def_func = get_member_func(name, &T.class_info);
        if (def_func->body_defined)
            eve_custom_error(EVE_METHOD_ALREADY_DEFINED, "file: '%s', line: %d, pos: %d, class '%s' method '%s' already defined.",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, T.name, name);
        char * type1, type2;
        if (current_type == _func)
        {
            type1 = strdup("function");
            type2 = strdup("procedure");
        }
        else
        {
            type1 = strdup("procedure");
            type2 = strdup("function");
        }
        if (def_func->type != current_type)
            eve_custom_error(EVE_WRONG_METHOD_DEFINITION, "file: '%s', line: %d, pos: %d, class '%s' method '%s' defined as %s not %s.",
                             info.source, info.line_num, info.pos, T.name, name, type2, type1);

        lexstep(li);
        int count = 0; // # of params
        tVar * args;
        int unlimited_args = 0;
        if (currenttoken(li) == '(')
        {
            string tmpType, tmpName;
            args = (tVar*)eve_malloc(sizeof(tVar));
lbl:
            {
                tMod tmpmod = _none;
                lexstep(li);
                if(currenttoken(li) == TRIPLEDOT)
                {
                    unlimited_args=1;
                }
                else
                {
                    count++;
                    debugf("count = %d\n", count);
                    if (is_mod(li->TokenInfo[li->pos]))
                    {
                        if (currenttoken(li) == VAR)
                            tmpmod = _var;
                        else if (currenttoken(li) == CONST)
                            tmpmod = _const;
                        else
                            eve_custom_error(EVE_WRONG_MOD_EXPECTED, "file: '%s', line: %d, pos: %d, mod '%s' is not valid in parameters.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                        lexstep(li);
                        match(li, IDENTIFIER);
                    }
                    if (!type_is_defined(currenttokenstring(li)))
                        eve_custom_error(EVE_UNDEFINED_DATA_TYPE, "file: '%s', line: %d, pos: %d, type '%s' is not defined.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                    tmpType = strdup(currenttokenstring(li));
                    lexstep(li);
                    match(li, IDENTIFIER);
                    if ((!type_is_defined(currenttokenstring(li)) || (!is_mod(li->TokenInfo[li->pos]))))
                    {
                        tmpName = currenttokenstring(li);
                        debugf("current name := %d : %s\n", (count)-1, currenttokenstring(li));
                        args = (tVar*)eve_realloc(args, (count)*sizeof(tVar));
                        args[count-1].mod = tmpmod;
                        args[count-1].name = strdup(currenttokenstring(li));
                        args[count-1].type = strdup(tmpType);
                    }
                    if(nexttoken(li) == ',')
                    {
                        lexstep(li);
                        goto lbl;
                    }
                }
            }
            lexstep(li);
            match(li, ')');
            lexstep(li);
        }
        if(currenttoken(li) == ';')
            eve_custom_error(EVE_DEFENITION_EXPECTED, "file: '%s', line: %d, pos: %d, expecting method definition before ';'.",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos);
        // Parameter checking and return type checking. Variables' names does not matter, types does!
        if (current_type == _func)
            if (strcmp(ttype, def_func->return_type) != 0)
                eve_custom_error(EVE_WRONG_FUNC_DEFINITION, "file: '%s', line: %d, pos: %d, function definition is not the same, return type '%s' expected but %s was found, of function %s",
                                 info.source, info.line_num, info.pos, def_func->return_type, ttype, name);

        int i = 0;
        if (def_func->pcount != count+1)
            eve_custom_error(EVE_WRONG_FUNC_DEFINITION, "file: '%s', line: %d, pos: %d, function definition is not the same, number of parameters does not match, of function %s",
                             li->source, info.line_num, info.pos, name);
        for(; i < count; i++)
            if(strcmp(def_func->params[i+1].type, args[i].type) != 0)
                eve_custom_error(EVE_WRONG_FUNC_DEFINITION, "file: '%s', line: %d, pos: %d, function definition is not the same, type '%s' expected but %s was found, of parameter number %d, of function %s",
                                 li->source, info.line_num, info.pos, def_func->params[i].type, args[i].type, i, name);
        if(unlimited_args != def_func->unlimited_args)
            eve_custom_error(EVE_WRONG_FUNC_DEFINITION, "file: '%s', line: %d, pos: %d, function definition is not the same, parameters does not match (multiple parameters), of function %s",
                             li->source, info.line_num, info.pos, name);

        T.class_info.methodes[pos] = create_thread(current_type, thread, def_func->name, ttype, def_func->params, count+1, 1, unlimited_args, 0);
        T.class_info.methodes[pos]->gen_name = def_func->gen_name;
        T.class_info.methodes[pos]->parent_class = &T.class_info;
        T.class_info.methodes[pos]->info = tmp_token;
        while(currenttoken(li)!=END)
            dostmt(li, T.class_info.methodes[pos]);
        match(li, END);
        lexstep(li);
        return ;

    }

    token_node tmp_token_name = li->TokenInfo[li->pos];
    string tname = strdup(currenttokenstring(li));
    // name should be unique
    tThread * def_func = NULL;
    if(!name_is_unique(tname, thread, 1))
    {
        if (func_is_defined(tname, current_type))
        {
            def_func = find_func(tname, current_type);
            if(def_func->body_defined == 1)
                eve_custom_error(EVE_ID_ALREADY_DEFINED, "file: '%s', line: %d, pos: %d, function/procedure '%s' is already defined.",
                                 li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
        }
        else
            eve_custom_error(EVE_ID_ALREADY_DEFINED, "file: '%s', line: %d, pos: %d, identifier '%s' is already defined.",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
    }
    // with or without parameters
    lexstep(li);
    int count = 0; // # of params
    tVar * args;
    int unlimited_args = 0;
    if (currenttoken(li) == '(')
    {
        string tmpType, tmpName;
        args = (tVar*)eve_malloc(sizeof(tVar));
l:
        {
            tMod tmpmod = _none;
            lexstep(li);
            if(currenttoken(li) == TRIPLEDOT)
            {
                unlimited_args=1;
            }
            else
            {
                count++;
                debugf("count = %d\n", count);
                if (is_mod(li->TokenInfo[li->pos]))
                {
                    if (currenttoken(li) == VAR)
                        tmpmod = _var;
                    else if (currenttoken(li) == CONST)
                        tmpmod = _const;
                    else
                        eve_custom_error(EVE_WRONG_MOD_EXPECTED, "file: '%s', line: %d, pos: %d, mod '%s' is not valid in parameters.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                    lexstep(li);
                    match(li, IDENTIFIER);
                }
                if (!type_is_defined(currenttokenstring(li)))
                    eve_custom_error(EVE_UNDEFINED_DATA_TYPE, "file: '%s', line: %d, pos: %d, type '%s' is not defined.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                tmpType = strdup(currenttokenstring(li));
                lexstep(li);
                match(li, IDENTIFIER);
                if ((!type_is_defined(currenttokenstring(li)) || (!is_mod(li->TokenInfo[li->pos]))))
                {
                    tmpName = currenttokenstring(li);
                    debugf("current name := %d : %s\n", (count)-1, currenttokenstring(li));
                    args = (tVar*)eve_realloc(args, (count)*sizeof(tVar));
                    args[count-1].mod = tmpmod;
                    args[count-1].name = strdup(currenttokenstring(li));
                    args[count-1].type = strdup(tmpType);
                }
                if(nexttoken(li) == ',')
                {
                    lexstep(li);
                    goto l;
                }
            }
        }
        lexstep(li);
        match(li, ')');
        lexstep(li);
    }
    // registering thread
    if(currenttoken(li)==';') // declaration
    {
        tThread * myfunc = create_thread(current_type, thread, tname, ttype, args, count, 0, unlimited_args, 0);
        myfunc->info = tmp_token_name;
        register_function(myfunc);
    }
    else // defenition
    {
        tThread * myfunc = create_thread(current_type, thread, tname, ttype, args, count, 1, unlimited_args, 0);
        myfunc->info = tmp_token_name;
        if(def_func == NULL)
        {
            register_function(myfunc);
            while(currenttoken(li)!=END)
                dostmt(li, myfunc);
            match(li, END);
        }
        else
        {
            // Parameter checking and return type checking. Variables' names does not matter, types does!
            if (current_type == _func)
                if (strcmp(ttype, def_func->return_type) != 0)
                    eve_custom_error(EVE_WRONG_FUNC_DEFINITION, "file: '%s', line: %d, pos: %d, function definition is not the same, return type '%s' expected but %s was found, of function %s",
                                     li->source, tmp_token.line_num, tmp_token.pos, def_func->return_type, ttype, tname);

            int i = 0;
            if (def_func->pcount != count)
                eve_custom_error(EVE_WRONG_FUNC_DEFINITION, "file: '%s', line: %d, pos: %d, function definition is not the same, number of parameters does not match, of function %s",
                                 li->source, tmp_token_name.line_num, tmp_token_name.pos, tname);
            for(; i < count; i++)
                if(strcmp(def_func->params[i].type, args[i].type) != 0)
                    eve_custom_error(EVE_WRONG_FUNC_DEFINITION, "file: '%s', line: %d, pos: %d, function definition is not the same, type '%s' expected but %s was found, of parameter number %d, of function %s",
                                     li->source, tmp_token_name.line_num, tmp_token_name.pos, def_func->params[i].type, args[i].type, i, tname);
            if(unlimited_args != def_func->unlimited_args)
                eve_custom_error(EVE_WRONG_FUNC_DEFINITION, "file: '%s', line: %d, pos: %d, function definition is not the same, parameters does not match (multiple parameters), of function %s",
                                 li->source, tmp_token_name.line_num, tmp_token_name.pos, tname);


            for (i = 0; i < global_functions_count; i++)
                if (strcmp(tname, global_functions[i]->name) == 0)
                    if(global_functions[i]->type == current_type)
                        break;

            global_functions[i] = create_thread(current_type, thread, tname, ttype, args, count, 1, unlimited_args, 0);
            global_functions[i]->info = tmp_token_name;
            while(currenttoken(li)!=END)
                dostmt(li, global_functions[i]);
            match(li, END);
        }

    }

    lexstep(li);
}

void docdef(LexInfo * li, tThread * thread)
{
    lexstep(li);
    if(currenttoken(li) == FUNC)
    {
        lexstep(li);
        match(li, IDENTIFIER);
        string type = strdup(currenttokenstring(li));
        lexstep(li);
        match(li, IDENTIFIER);
        string name = strdup(currenttokenstring(li));
        lexstep(li);
        tVar * args;
        int u_variables = 0;
        int count = 0;

        if (currenttoken(li) == '(')
        {
            string tmpType, tmpName;
            args = (tVar*)eve_malloc(sizeof(tVar));
l:
            {
                tMod tmpmod = _none;
                lexstep(li);
                if(currenttoken(li) == TRIPLEDOT)
                {
                    u_variables=1;
                }
                else
                {
                    count++;
                    debugf("count = %d\n", count);
                    if (is_mod(li->TokenInfo[li->pos]))
                    {
                        if (currenttoken(li) == VAR)
                            tmpmod = _var;
                        else if (currenttoken(li) == CONST)
                            tmpmod = _const;
                        else
                            eve_custom_error(EVE_WRONG_MOD_EXPECTED, "file: '%s', line: %d, pos: %d, mod '%s' is not valid in parameters.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                        lexstep(li);
                        match(li, IDENTIFIER);
                    }
                    if (!type_is_defined(currenttokenstring(li)))
                        eve_custom_error(EVE_UNDEFINED_DATA_TYPE, "file: '%s', line: %d, pos: %d, type '%s' is not defined.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                    tmpType = strdup(currenttokenstring(li));
                    lexstep(li);
                    match(li, IDENTIFIER);
                    if ((!type_is_defined(currenttokenstring(li)) || (!is_mod(li->TokenInfo[li->pos]))))
                    {
                        tmpName = currenttokenstring(li);
                        debugf("current name := %d : %s\n", (count)-1, currenttokenstring(li));
                        args = (tVar*)eve_realloc(args, (count)*sizeof(tVar));
                        args[count-1].mod = tmpmod;
                        args[count-1].name = strdup(currenttokenstring(li));
                        args[count-1].type = strdup(tmpType);
                    }
                    if(nexttoken(li) == ',')
                    {
                        lexstep(li);
                        goto l;
                    }
                }
            }
            lexstep(li);
            match(li, ')');
            lexstep(li);
        }
        register_function(create_thread(_func, thread, name, type, args, count, 0, u_variables, 1));

    }
    else if (currenttoken(li) == PROC)
    {
        lexstep(li);
        match(li, IDENTIFIER);
        string name = strdup(currenttokenstring(li));
        lexstep(li);
        tVar * args;
        int u_variables = 0;
        int count = 0;

        if (currenttoken(li) == '(')
        {
            string tmpType, tmpName;
            args = (tVar*)eve_malloc(sizeof(tVar));
l2:
            {
                tMod tmpmod = _none;
                lexstep(li);
                if(currenttoken(li) == TRIPLEDOT)
                {
                    u_variables=1;
                }
                else
                {
                    count++;
                    debugf("count = %d\n", count);
                    if (is_mod(li->TokenInfo[li->pos]))
                    {
                        if (currenttoken(li) == VAR)
                            tmpmod = _var;
                        else if (currenttoken(li) == CONST)
                            tmpmod = _const;
                        else
                            eve_custom_error(EVE_WRONG_MOD_EXPECTED, "file: '%s', line: %d, pos: %d, mod '%s' is not valid in parameters.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                        lexstep(li);
                        match(li, IDENTIFIER);
                    }
                    if (!type_is_defined(currenttokenstring(li)))
                        eve_custom_error(EVE_UNDEFINED_DATA_TYPE, "file: '%s', line: %d, pos: %d, type '%s' is not defined.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                    tmpType = strdup(currenttokenstring(li));
                    lexstep(li);
                    match(li, IDENTIFIER);
                    if ((!type_is_defined(currenttokenstring(li)) || (!is_mod(li->TokenInfo[li->pos]))))
                    {
                        tmpName = currenttokenstring(li);
                        debugf("current name := %d : %s\n", (count)-1, currenttokenstring(li));
                        args = (tVar*)eve_realloc(args, (count)*sizeof(tVar));
                        args[count-1].mod = tmpmod;
                        args[count-1].name = strdup(currenttokenstring(li));
                        args[count-1].type = strdup(tmpType);
                    }
                    if(nexttoken(li) == ',')
                    {
                        lexstep(li);
                        goto l2;
                    }
                }
            }
            lexstep(li);
            match(li, ')');
            lexstep(li);
        }
        register_function(create_thread(_proc, thread, name, "void", args, count, 0, u_variables, 1));

    }
    else
        eve_custom_error(EVE_STATEMENT_EXPECTED, "Syntax error: file: '%s', line %d pos %d, cdef expecting a function or a procedure.",
                         li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos);
    if(currenttoken(li)==FROM)
    {
        lexstep(li);
        match(li, IDENTIFIER);
        add_inc(currenttokenstring(li));
        lexstep(li);
    }
    match(li, ';');
    lexstep(li);
}

void docimport(LexInfo * li, tThread * thread)
{
    lexstep(li);
    match(li, LITERAL);
    add_cinc(currenttokenstring(li));
    lexstep(li);
    match(li, ';');
    lexstep(li);
}

void doinclude(LexInfo * li, tThread * thread)
{

    lexstep(li);
l:
    {
        match(li, IDENTIFIER);
        //printf("path = %s\n", getenv("EVEPATH"));
        /* here we should save the include path in the system's environment variables */
        string path = strdup("");
        strcat(path, currenttokenstring(li));
        strcat(path, ".eve");
        string bpath = strdup(lib_path);
        debugf("[doinclude] importing %s\n", path);
        if(nexttoken(li) == FROM)
        {
            // TODO
            lexstep(li);
            lexstep(li);
            match(li, LITERAL);
        }
        else
        {
            if(!file_exists(path))
            {
                strcat(bpath, path);
                strcpy(path, bpath);
                if(!file_exists(path))
                    eve_custom_error(EVE_STATEMENT_EXPECTED, "File reading error: file: '%s', line %d pos %d, included file \"%s\" does not exist.",
                                     li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, path);
            }
        }
        if (!file_is_imported(path))
        {
            add_imported_file(path);
            LexInfo l = Lex_Begin(path, readfile(path));
            start_parse(&l);
        }


        lexstep(li);
        if(currenttoken(li) == ',')
        {
            lexstep(li);
            goto l;
        }
    }
    match(li, ';');
}

void doreturn(LexInfo * li, tThread * thread)
{
    tThread * parent = get_parent_func(thread);

    if(parent == NULL)
        eve_custom_error(EVE_WRONG_CALL_OF_RETURN, "Syntax error: file: '%s', line %d pos %d, invalid call of return (not inside a procedure nor function).",
                         li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos);
    debugf("[doreturn thread is %s]", parent->name);
    thread->instructions[thread->icount] = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
    thread->instructions[thread->icount]->type = li->TokenInfo[li->pos];

    if(parent->type == _func)
    {
        int tmp_pos = lex_find_token_pos(li, ';', li->size-1);
        debugf("pos = %d\n", tmp_pos);
        if(tmp_pos == -1)
            eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->size-1].line_num,
                                      li->TokenInfo[li->size-1].pos, ';'/*, "EOF"*/);
        thread->instructions[thread->icount]->unary = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
        thread->instructions[thread->icount]->unary = gen_instruction(li, li->pos+1, tmp_pos-1, thread, NULL);
        match_type(thread->instructions[thread->icount]->unary, parent->return_type, thread);
        li->pos = tmp_pos;
    }
    else
        lexstep(li);
    thread->icount++;
    match(li, ';');

}

// break / continue
void dobreak(LexInfo * li, tThread * thread)
{
    if(break_in_loop(thread))
    {
        thread->instructions[thread->icount] = (tStatementNode*)eve_malloc(sizeof(tStatementNode));
        thread->instructions[thread->icount]->type = li->TokenInfo[li->pos];
        lexstep(li);
        match(li, ';');
        thread->icount++;
    }
    else
    {
        eve_custom_error(EVE_WRONG_CALL_OF_BREAK, "Syntax error: file: '%s', line %d pos %d, invalid call of %s (not inside a loop statement).",
                         li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, li->TokenInfo[li->pos].str);
    }
    lexstep(li);
}

void dotype(LexInfo * li, tThread * thread)
{
    lexstep(li);
    match(li, IDENTIFIER);
    if(!name_is_unique(currenttokenstring(li), thread, 1))
        eve_custom_error(EVE_IDENTIFIER_REDEFINITON, "file: '%s', line: %d, pos: %d, can not define type \"%s\", identifier is already defined.\n",
                         li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
    tType mytype;
    mytype.name = strdup(currenttokenstring(li));
    lexstep(li);
    match(li, IS);
    lexstep(li);
    if (strcmp(currenttokenstring(li), "array") == 0)
    {
        mytype.type_kind = __array;
        lexstep(li);
        match(li, OF);
        lexstep(li);
        if (!type_is_defined(currenttokenstring(li)))
            eve_custom_error(EVE_UNDEFINED_DATA_TYPE, "file: '%s', line: %d, pos: %d, can not define type \"%s\", identifier \"%s\" is not defined.\n",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, mytype.name ,currenttokenstring(li));
        mytype.pointer = 1;
        mytype.pointerto = currenttokenstring(li);
        lexstep(li);
        match(li, ';');
        register_type(mytype);
    }
    else if (strcmp(currenttokenstring(li), "enum") == 0)
    {
        lexstep(li);
        mytype.type_kind = __enum;
        mytype.ecount = 0;
        mytype.enums = (char**)eve_malloc(sizeof(char*));
l:
        match(li, IDENTIFIER);
        // need to check if the enum does not exist already.
        debugf("token %s is enum.\n", currenttokenstring(li));

        if (!name_is_unique(currenttokenstring(li), thread, 1))
            eve_custom_error(EVE_IDENTIFIER_REDEFINITON, "file: '%s', line: %d, pos: %d, identifier %s is already defined.\n",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
        mytype.enums[mytype.ecount] = strdup(currenttokenstring(li));
        mytype.ecount++;
        lexstep(li);
        if(currenttoken(li) == ',')
        {
            lexstep(li);
            goto l;
        }
        match(li, END);
        mytype.pointer = 0;
        mytype.pointerto = NULL;
        register_type(mytype);

    }
    else if (strcmp(currenttokenstring(li), "struct") == 0)
    {
        mytype.type_kind = __struct;
        register_type(mytype);
    }
    else
    {
        mytype.type_kind = __none;
        mytype.pointer = 0;
        if(currenttoken(li) == '^')
        {
            mytype.pointer = 1;
            lexstep(li);
        }
        match(li, IDENTIFIER);
        if(!type_is_defined(currenttokenstring(li)))
            eve_custom_error(EVE_UNDEFINED_DATA_TYPE, "file: '%s', line: %d, pos: %d, type '%s' is not defined.",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
        mytype.pointerto = strdup(currenttokenstring(li));

        lexstep(li);

        match(li, ';');
        register_type(mytype);
    }

    lexstep(li);
}

void dopointer(LexInfo * li, tThread * thread)
{
    int tmp_pos = lex_find_token_pos(li, ';', li->size-1);
    if(tmp_pos == -1)
        eve_syntax_error_expected(EVE_WRONG_TOKEN_EXPECED, li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, ';');
    thread->instructions[thread->icount] = gen_instruction(li, li->pos, tmp_pos-1, thread, NULL);
    string tname = get_op_type(thread->instructions[thread->icount]->left, thread);
    debugf("name = %s\n", tname);
    tType tmp = find_type(tname);
    string type = find_type_pointerto(tmp);
    if(type == NULL)
        eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, type of \"%s\"is not defined.\n",
                         li->source, thread->instructions[thread->icount]->left->type.line_num,
                         thread->instructions[thread->icount]->left->type.pos, thread->instructions[thread->icount]->left->type.str);

    match_type(thread->instructions[thread->icount]->right, type, thread);
    thread->icount++;

    li->pos = tmp_pos;
}

void doclassfunc(LexInfo * li,tThread * thread, class_ * c)
{
    tThreadType current_type;
    if (currenttoken(li) == FUNC)
        current_type = _func;
    else
        current_type = _proc;
    lexstep(li);
    char * func_type;
    if (current_type == _func)
    {

        match(li, IDENTIFIER);
        func_type = strdup(currenttokenstring((li)));
        if (!type_is_defined(func_type))
            eve_custom_error(EVE_UNKNOWN_DATA_TYPE,"file: '%s', line: %d, pos: %d, identifier '%s' is not a valid data type.",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, func_type);
        lexstep(li);
    }
    else
        func_type = strdup("void");

    match(li, IDENTIFIER);
    char * func_name = strdup(c->name);
    char * real_name = strdup(currenttokenstring(li));
    token_node func_info = li->TokenInfo[li->pos];
    strcat(func_name, "_");
    strcat(func_name, real_name);
    tVar * args = (tVar*)eve_malloc(sizeof(tVar));
    int count = 1;
    // first arg is self.
    lexstep(li);
    int u_args = 0;
    if (currenttoken(li) == '(')
    {
        string tmpType, tmpName;
l:
        {
            tMod tmpmod = _none;
            lexstep(li);
            if(currenttoken(li) == TRIPLEDOT)
            {
                u_args=1;
            }
            else
            {
                count++;
                debugf("count = %d\n", count);
                if (is_mod(li->TokenInfo[li->pos]))
                {
                    if (currenttoken(li) == VAR)
                        tmpmod = _var;
                    else if (currenttoken(li) == CONST)
                        tmpmod = _const;
                    else
                        eve_custom_error(EVE_WRONG_MOD_EXPECTED, "file: '%s', line: %d, pos: %d, mod '%s' is not valid in parameters.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                    lexstep(li);
                    match(li, IDENTIFIER);
                }
                if (!type_is_defined(currenttokenstring(li)))
                    eve_custom_error(EVE_UNDEFINED_DATA_TYPE, "file: '%s', line: %d, pos: %d, type '%s' is not defined.", li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                tmpType = strdup(currenttokenstring(li));
                lexstep(li);
                match(li, IDENTIFIER);
                if ((!type_is_defined(currenttokenstring(li)) || (!is_mod(li->TokenInfo[li->pos]))))
                {
                    tmpName = currenttokenstring(li);
                    debugf("current name := %d : %s\n", (count)-1, currenttokenstring(li));
                    args = (tVar*)eve_realloc(args, (count)*sizeof(tVar));
                    args[count-1].mod = tmpmod;
                    args[count-1].name = strdup(currenttokenstring(li));
                    args[count-1].type = strdup(tmpType);
                }
                if(nexttoken(li) == ',')
                {
                    lexstep(li);
                    goto l;
                }
            }
        }
        lexstep(li);
        match(li, ')');
        lexstep(li);
    }

    args[0].name = strdup("self");
    args[0].type = strdup(c->pointer_name);

    debugf("arguments of func %s:\n", func_name);
    // registering thread
    debugf("saving class func %s as %s, param count : %d\n", real_name, func_name, count);
    // are header and body seperated?
    if (currenttoken(li) == ';')
    {
        c->methodes = (tThread**)eve_realloc(c->methodes, (c->mcount+1)*sizeof(tThread*));
        c->methodes[c->mcount] = create_thread(current_type, thread, real_name, func_type, args, count, 0, u_args, 0);
        c->methodes[c->mcount]->info = func_info;
        c->methodes[c->mcount]->gen_name = strdup(func_name);
        c->methodes[c->mcount]->parent_class = c;
        c->mcount++;
        lexstep(li);
        return;
    }
    c->methodes = (tThread**)eve_realloc(c->methodes, (c->mcount+1)*sizeof(tThread*));
    c->methodes[c->mcount] = create_thread(current_type, thread, real_name, func_type, args, count, 1, u_args, 0);
    c->methodes[c->mcount]->info = func_info;
    c->methodes[c->mcount]->gen_name = strdup(func_name);
    c->methodes[c->mcount]->parent_class = c;
    c->mcount++;

    while(currenttoken(li)!=END)
        dostmt(li, c->methodes[c->mcount-1]);
    match(li, END);
    lexstep(li);
}

void doclasstype(LexInfo * li,tThread * thread, class_ * c)
{

}
void doclassvar(LexInfo * li,tThread * thread, class_ * c)
{

    if(currenttoken(li) == KEY)
    {
        // key should be defined only once.
        if (c->key_index != -1)
            eve_custom_error(EVE_UNKNOWN_DATA_TYPE,"file: '%s', line: %d, pos: %d, key variable already defined.",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos);
        // a key should be one
        lexstep(li);
        match(li, IDENTIFIER);
        char * type = strdup(currenttokenstring(li));
        if(!type_is_defined(type))
            eve_custom_error(EVE_UNKNOWN_DATA_TYPE,"file: '%s', line: %d, pos: %d, identifier '%s' is not a valid data type.",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, type);
        lexstep(li);
        c->variables = (tVar*)eve_realloc(c->variables, (c->vcount+1)*sizeof(tVar));
        c->variables[c->vcount].name = strdup(currenttokenstring(li));
        c->variables[c->vcount].type = strdup(type);
        c->variables[c->vcount].info = li->TokenInfo[li->pos];
        if (!name_is_unique(c->variables[c->vcount].name, thread, 1))
            eve_custom_error(EVE_ID_ALREADY_DEFINED, "file: '%s', line: %d, pos: %d, identifier '%s' is already defined.",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, c->variables[c->vcount].name);
        c->key_index = c->vcount;
        c->vcount++;

        lexstep(li);
        match(li, ';');
        lexstep(li);
    }
    else
    {
        int is_property = 0;
        if (currenttoken(li) == PROPERTY)
        {
            is_property = 1;
            lexstep(li);
        }
        match(li, IDENTIFIER);

        char * type = strdup(currenttokenstring(li));

        if(!type_is_defined(type))
            eve_custom_error(EVE_UNKNOWN_DATA_TYPE,"file: '%s', line: %d, pos: %d, identifier '%s' is not a valid data type.",
                             li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, type);
label:
        {
            lexstep(li);
            match(li, IDENTIFIER);
            debugf("registering var: type: %s name :%s\n", type, currenttokenstring(li));
            c->variables = (tVar*)eve_realloc(c->variables, (c->vcount+1)*sizeof(tVar));
            c->variables[c->vcount].name = strdup(currenttokenstring(li));
            c->variables[c->vcount].type = strdup(type);
            c->variables[c->vcount].info = li->TokenInfo[li->pos];
            c->variables[c->vcount].reads = NULL;
            c->variables[c->vcount].writes = NULL;
            c->variables[c->vcount].is_property = is_property;

            if (!name_is_unique(c->variables[c->vcount].name, thread, 1))
                eve_custom_error(EVE_ID_ALREADY_DEFINED, "file: '%s', line: %d, pos: %d, identifier '%s' is already defined.",
                                 li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, c->variables[c->vcount].name);

            lexstep(li);
            if  (currenttoken(li) == READ)
            {
                if (c->variables[c->vcount].reads != NULL)
                    eve_custom_error(EVE_READ_FUNC_EXISTS,"file: '%s', line: %d, pos: %d, variable '%s' already have a read function.",
                                     li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, c->variables[c->vcount].name);
                lexstep(li);
                match(li, IDENTIFIER);

                int p = is_member_func(currenttokenstring(li), c);
                tThread * thefunc = NULL;
                if (p != -1)
                    thefunc = c->methodes[p];

                if (thefunc == NULL)
                    eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, function/procedure %s is not defined.\n",
                                     li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                if (thefunc->type != _func)
                    eve_custom_error(EVE_INVALID_USE_OF_READ_WRITE, "file: '%s', line: %d, pos: %d, read can only be applied to functions.\n",
                                     li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                if (strcmp(thefunc->return_type, c->variables[c->vcount].type) != 0)
                    eve_custom_error(EVE_INVALID_USE_OF_READ_WRITE, "file: '%s', line: %d, pos: %d, read function's '%s' type does not match property's type '%s'.\n",
                                     li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, thefunc->return_type, c->variables[c->vcount].type);

                c->variables[c->vcount].reads = thefunc;
                lexstep(li);
            }

            if  (currenttoken(li) == WRITE)
            {
                if (c->variables[c->vcount].writes != NULL)
                    eve_custom_error(EVE_WRITE_FUNC_EXISTS,"file: '%s', line: %d, pos: %d, variable '%s' already have a write function.",
                                     li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, c->variables[c->vcount].name);
                lexstep(li);
                match(li, IDENTIFIER);

                int p = is_member_func(currenttokenstring(li), c);
                tThread * thefunc = NULL;
                if (p != -1)
                    thefunc = c->methodes[p];

                if (thefunc == NULL)
                    eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, function/procedure %s is not defined.\n",
                                     li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
                if (thefunc->type != _proc)
                    eve_custom_error(EVE_INVALID_USE_OF_READ_WRITE, "file: '%s', line: %d, pos: %d, read can only be applied to procedures.\n",
                                     li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));

                if ((thefunc->pcount != 2) || (strcmp(thefunc->params[1].type, c->variables[c->vcount].type)!=0))
                    eve_custom_error(EVE_INVALID_USE_OF_READ_WRITE, "file: '%s', line: %d, pos: %d, write procedure's parameter type '%s' does not match property's type '%s'.\n",
                                     li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, thefunc->return_type, c->variables[c->vcount].type);

                c->variables[c->vcount].writes = thefunc;
                lexstep(li);
            }

            c->vcount++;
        }
        if(currenttoken(li) == ',')
            goto label;
        match(li, ';');
        lexstep(li);
    }

}

void doclass(LexInfo * li, tThread * thread)
{
    debugf("[doclass] thread %s, ID = 0x%d pos %d \n", thread->name, (int)thread, li->pos);

    lexstep(li);
    match(li, IDENTIFIER);
    tType myclass;
    myclass.type_kind = __class;
    class_ c;
    c.vcount = 0;
    c.mcount = 0;
    c.key_index = -1; // no key.
    c.name = strdup(currenttokenstring(li));
    c.variables = (tVar*)eve_malloc(sizeof(tVar));

    tType pclass; // a pointer to this class

    pclass.name = strdup("__p");
    strcat(pclass.name, c.name);

    c.pointer_name = strdup(pclass.name);

    pclass.pointer = 1;
    pclass.pointerto = strdup(c.name);
    pclass.type_kind = __pclass;

    register_type(pclass);


    myclass.class_info = c;
    myclass.name = c.name;
    myclass.pointer = 0;
    myclass.pointerto = NULL;
    int tmp_index = global_types_count;

    if (!name_is_unique(c.name, thread, 1))
        eve_custom_error(EVE_UNKNOWN_DATA_TYPE,"file: '%s', line: %d, pos: %d, identifier '%s' is not a valid data type.",
                         li->source, li->TokenInfo[li->pos].line_num, li->TokenInfo[li->pos].pos, currenttokenstring(li));
    register_type(myclass);
    lexstep(li);
    if(currenttoken(li) == '(')
    {
        // templates ..
    }
    while(currenttoken(li) != END)
    {
        switch(currenttoken(li))
        {
        case FUNC:
        case PROC:
            doclassfunc(li, thread, &c);
            break;
        case TYPE:
            doclasstype(li, thread, &c);
            break;
        default:
            doclassvar(li, thread, &c);
            break;
        }
        myclass.class_info = c;
        global_types[tmp_index] = myclass;
    }
    match(li, END);
    lexstep(li);
    debugf("end of class\n");
}

void doctype(LexInfo * li, tThread * thread)
{
    lexstep(li);
    match(li, IDENTIFIER);
    tType tmp;
    tmp.name = strdup(currenttokenstring(li));
    tmp.pointer = 0;
    tmp.pointerto = NULL;
    tmp.type_kind = __ctype;
    lexstep(li);
    register_type(tmp);
    if (currenttoken(li) == FROM)
    {
        lexstep(li);
        match(li, IDENTIFIER);
        add_inc(currenttokenstring(li));
        lexstep(li);
        match(li, ';');
    }
    lexstep(li);
}

void dostmt(LexInfo * li, tThread * thread)
{
    debugf("[dostmt] thread %s, ID = 0x%d pos %d is %s\n", thread->name, (int)thread, li->pos, currenttokenstring(li));
    switch (li->TokenInfo[li->pos].TT)
    {
    case ';':
        lexstep(li);
        break;

    case BEGIN:
        doblock(li, thread);
        break;

    case IF:
        doif(li, thread);
        break;

    case IDENTIFIER:
    case EXTERN:
    case STATIC:
    case REGISTER:
    case LOCAL:
    case VOLATILE:
    case CONST:
    case VAR:
        dovariable(li, thread);
        break;

    case CDEF:
        docdef(li, thread);
        break;

    case CIMPORT:
        docimport(li, thread);
        break;

    case CTYPE:
        doctype(li, thread);
        break;

    case CLASS:
        doclass(li, thread);
        break;

    case FUNC:
    case PROC:
        dofunction(li, thread);
        break;

    case FOR:
        dofor(li, thread);
        break;

    case REPEAT:
        dorepeat(li, thread);
        break;

    case WHILE:
        dowhile(li, thread);
        break;

    case INCLUDE:
        doinclude(li, thread);
        break;

    case RETURN:
        doreturn(li, thread);
        break;

    case BREAK:
    case CONTINUE:
        dobreak(li, thread);
        break;

    case TYPE:
        dotype(li, thread);
        break;

    case '^':
        dopointer(li, thread);
        break;

        /* unexpected tokens */
    default:
        dovariable(li, thread);

    }
}

void start_parse(LexInfo * li)
{
    initparser(li, global_thread);
    //register_variable("xD", "int", 0, _none, head);
    debugf("size = %d\n", li->size);
    debugf("[startparse] starting parse task. \n");
    while(li->pos <= li->size-1)
        dostmt(li, global_thread);
}
