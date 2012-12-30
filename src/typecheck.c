/*
 * Eve programming language
 * Type checking
 * check eve.h for copyright informations
 */

#include <stdlib.h>
#include <string.h>

#include "eve.h"
#include "error.h"
#include "lex.h"
#include "parse.h"
#include "types.h"
#include "variables.h"
#include "threads.h"


void match_type(tStatementNode * node, string type, tThread * thread);

int compare_type(string t1, string t2)
{
    //debugf("comparing %s with %s", t1, t2);
    tType tt1 = find_type_root(find_type(t1)), tt2 = find_type_root(find_type(t2));


    if (tt1.name != NULL)
    {
        if(tt2.name != NULL)
        {
            if((strcmp(tt1.name, tt2.name) == 0) && (tt1.pointer == tt2.pointer))
                return 1;
        }
    }

    return 0;
}

string get_op_type(tStatementNode * node, tThread * thread)
{
    //debugf("[get_op_type] op = %s\n", get_token_str(node->type.str));
    int i = 0;
    switch(node->type.TT)
    {
    case INTEGER:
    case HEX:
        return "int";
    case FLOATING:
        return "float";
    case LITERAL:
        return "str";
    case BOOLEAN:
        return "bool";
    case CHAR:
        return "char";
    case IDENTIFIER:
    {
        debugf("ID\n");
        // is it an enum?
        if (enum_is_defined(node->type.str))
        {
            for(i=BASIC_TYPES_COUNT; i<global_types_count; i++)
            {
                if (global_types[i].type_kind == __enum)
                {
                    int j = 0;
                    for(; j<global_types[i].ecount; j++)
                    {
                        if (strcmp(global_types[i].enums[j], node->type.str) == 0)
                            return global_types[i].name;
                    }
                }
            }
        }
        if(var_is_defined(node->type.str, thread, 1))
        {
            tVar tmpVar = find_variable(node->type.str, thread);
            tType tmp = find_type(tmpVar.type);
            string type = tmp.name;
            if(type == NULL)
                eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, type of \"%s\"is not defined.\n",
                                 node->type.source, node->type.line_num,
                                 node->type.pos, node->type.str);
            return type;
        }
    }

    case NIL:
        if (node->parent != NULL)
        {
            if (node->parent->left != NULL)
            {
                if (node->parent->left->type.TT != NULL)
                {
                    string tmpTypeName = get_op_type(node->parent->left, thread);
                    tType tmpType = find_type(tmpTypeName);
                    if (tmpType.pointer)
                        return tmpTypeName;
                    else // for a more clear error
                        eve_custom_error(EVE_INVALID_USE_OF_NIL, "file: '%s', line: %d, pos: %d, Cannot assign nil to a non-pointer variable.",
                                         node->type.source, node->type.line_num, node->type.pos);
                }
            }

        }
        //eve_custom_error(EVE_INVALID_USE_OF_NIL, "file: '%s', line: %d, pos: %d, Invalid use of nil.",
        //                 node->type.source, node->type.line_num, node->type.pos);
    }

    if(token_is_op(node->type))
    {
        string op1;
        string op2;
        string type;
        tType tmp;
        int i;
        switch (node->type.TT)
        {
        case '+':
            op1 = get_op_type(node->left, thread);
            if(strcmp(op1, "int") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "int") == 0)
                    return "int";
                else if (strcmp(op2, "float") == 0)
                    return "float";
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            else if(strcmp(op1, "float") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "int") == 0)
                    return "float";
                else if (strcmp(op2, "float") == 0)
                    return "float";
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            else if(strcmp(op1, "str") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "str") == 0)
                    return "str";
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            else
                eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                 node->type.source, node->type.line_num, node->type.pos ,op1, op2);

        case '-':
            if(node->unary == NULL)
            {
                op1 = get_op_type(node->left, thread);
                if(strcmp(op1, "int") == 0)
                {
                    op2 = get_op_type(node->right, thread);
                    if(strcmp(op2, "int") == 0)
                        return "int";
                    else if (strcmp(op2, "float") == 0)
                        return "float";
                    else
                        eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                         node->type.source, node->type.line_num, node->type.pos ,op1, op2);
                }
                else if(strcmp(op1, "float") == 0)
                {
                    op2 = get_op_type(node->right, thread);
                    if(strcmp(op2, "int") == 0)
                        return "float";
                    else if (strcmp(op2, "float") == 0)
                        return "float";
                    else
                        eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                         node->type.source, node->type.line_num, node->type.pos ,op1, op2);
                }
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }

            else
            {
                op1 = get_op_type(node->unary, thread);
                if((strcmp(op1, "int") == 0 )|| (strcmp(op1, "float") == 0))
                    return op1;
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }

        case '*':
            op1 = get_op_type(node->left, thread);
            if(strcmp(op1, "int") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "int") == 0)
                    return "int";
                else if (strcmp(op2, "float") == 0)
                    return "float";
                else if (strcmp(op2, "str") == 0)
                    return "float";
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            else if(strcmp(op1, "float") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "int") == 0)
                    return "float";
                else if (strcmp(op2, "float") == 0)
                    return "float";
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            else if(strcmp(op1, "str") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "int") == 0)
                    return "str";
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            else
                eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                 node->type.source, node->type.line_num, node->type.pos ,op1, op2);


        case '%':
            op1 = get_op_type(node->left, thread);
            if(strcmp(op1, "int") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "int") == 0)
                    return "int";
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }

        case '/':
        {
            op1 = get_op_type(node->left, thread);
            if(strcmp(op1, "int") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "int") == 0)
                    return "int";
                else if (strcmp(op2, "float") == 0)
                    return "float";
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            else if(strcmp(op1, "float") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "int") == 0)
                    return "float";
                else if (strcmp(op2, "float") == 0)
                    return "float";
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            /*
            else if(strcmp(op1, "str") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "int") == 0)
                    return "p_str"; // array of string / pointer to string
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            */
            else
                eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                 node->type.source, node->type.line_num, node->type.pos ,op1, op2);
        }
        /* NOT GENERATED BY THE COMPILER */
        case INC:
        case DEC:
        {
            op1 = get_op_type(node->unary, thread);
            if(strcmp(op1, "int") == 0)
                return "int";
            else if(strcmp(op1, "float") == 0)
                return "float";
            else if(find_type(op1).type_kind == _array)
                return "void";

            else
                eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type \"%s\" of unary operator \"++\".",
                                 node->type.source, node->type.line_num, node->type.pos ,op1);
        }
        /* NOT GENERATED BY THE COMPILER */
        case PASS:
        {
            op1 = get_op_type(node->left, thread);
            if(strcmp(op1, "int") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "int") == 0)
                    return "int";
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            else if(strcmp(op1, "float") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "int") == 0)
                    return "float";
                else if (strcmp(op2, "float") == 0)
                    return "float";
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            else if(strcmp(op1, "str") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "str") == 0)
                    return "str"; // array of string / pointer to string
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            else
                eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                 node->type.source, node->type.line_num, node->type.pos ,op1, op2);
        }

        case '<':
        case '>':
        case LEQ:
        case GEQ:
            op1 = get_op_type(node->left, thread);
            if(strcmp(op1, "int") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "int") == 0)
                    return "bool";
                else if (strcmp(op2, "float") == 0)
                    return "bool";
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            else if(strcmp(op1, "float") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "int") == 0)
                    return "bool";
                else if (strcmp(op2, "float") == 0)
                    return "bool";
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            else if(strcmp(op1, "str") == 0)
            {
                op2 = get_op_type(node->right, thread);
                if(strcmp(op2, "str") == 0)
                    return "bool"; // array of string / pointer to string
                else
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
            }
            else
                eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                 node->type.source, node->type.line_num, node->type.pos ,op1, op2);

        case '=':
            type = strdup(get_op_type(node->right, thread));
            match_type(node->left, type, thread);
            if (find_variable(node->left, thread).mod == _const)
                return type;

        case NOTEQUAL:
        case EQUAL:
            debugf("EQUAL/NOTEQUAL\n");

            op2 = get_op_type(node->right, thread);
            op1 = get_op_type(node->left, thread);
            {
                if(strcmp(op1, op2)!= 0)
                {
                    eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded.",
                                     node->type.source, node->type.line_num, node->type.pos ,op1, op2);
                }
                debugf("returns!\n");
                return "bool";
            }

        case AND:
        case OR:
        case XOR:
            match_type(node->left, "bool", thread);
            match_type(node->right, "bool", thread);
            return "bool";

        case NOT:
            match_type(node->unary, "bool", thread);
            return "bool";

        case _IN:
            return "bool";

        case '@':
            tmp = find_type(get_op_type(node->unary, thread));
            type = find_type_pointerto(tmp);
            if(type == NULL)
                eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, type of \"%s\"is not defined.\n",
                                 node->type.source, node->type.line_num,
                                 node->type.pos, node->type.str);
            return type;
        case '^':
            tmp = find_type(get_op_type(node->unary, thread));
            type = find_type_pointerto(tmp);
            if(type == NULL)
                eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, type of \"%s\"is not defined.\n",
                                 node->type.source, node->type.line_num,
                                 node->type.pos, node->type.str);
            return type;
        case ':':
            match_type(node->right, "int", thread);
            tmp = find_type(get_op_type(node->left, thread));
            type = find_type_pointerto(tmp);
            if(type == NULL)
                eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, type of \"%s\"is not defined.\n",
                                 node->type.source, node->type.line_num,
                                 node->type.pos, node->type.str);
            return type;

        case '.': case DYN_CALL:
        {
            string tname = get_op_type(node->left, thread);
            debugf("type of %s is %s\n", node->left->type.str, tname);
            tmp = find_type(tname);
            if (node->type.TT == DYN_CALL)
                if (tmp.pointer == 0)
                    eve_custom_error(EVE_WRONG_PARAM_NUMBER, "file: '%s', line: %d, pos: %d, left operand of operator '->' must be pointer type.",
                                     node->type.source, node->left->type.line_num, node->left->type.pos);
            type = find_type_pointerto(tmp);


            debugf("type = %s\n", type);
            debugf("class = %s.%s\n", node->left->type.str, node->right->type.str);
            if (tmp.type_kind == __class)
            {

                int pos = is_member_func(node->right->type.str, &tmp.class_info);
                int pos2 = is_member_data(node->right->type.str, &tmp.class_info);
                debugf("func: %d, data: %d\n", pos, pos2);
                if(pos != -1)
                {
                    int i = 1;
                    debugf("parameters match default %d got %d+1\n", tmp.class_info.methodes[pos]->pcount, node->right->acount);
                    if (tmp.class_info.methodes[pos]->pcount > node->right->acount+1)
                        eve_custom_error(EVE_WRONG_PARAM_NUMBER, "file: '%s', line: %d, pos: %d, parameters count of function %s is different from the definition",
                                         node->type.source, node->right->type.line_num, node->right->type.pos, node->right->type.str);
                    if ((tmp.class_info.methodes[pos]->unlimited_args ==0) && (tmp.class_info.methodes[pos]->pcount < node->acount))
                        eve_custom_error(EVE_WRONG_PARAM_NUMBER, "file: '%s', line: %d, pos: %d, parameters count of function %s is different from the definition",
                                         node->right->type.source, node->right->type.line_num, node->right->type.pos, node->right->type.str);
                    for (; i< tmp.class_info.methodes[pos]->pcount; i++)
                        match_type(node->right->args[i-1], tmp.class_info.methodes[pos]->params[i].type, thread);

                    node->right->member_func = 1;
                    node->right->gen_name = strdup(tmp.class_info.methodes[pos]->gen_name);
                    return tmp.class_info.methodes[pos]->return_type;
                }


                else if(pos2 != -1)
                {
                    node->right->member_func = 0;
                    return tmp.class_info.variables[pos2].type;
                }
                else
                    eve_custom_error(Eve_INVALID_CLASS_OPERAND, "file: '%s', line: %d, pos: %d, type of \"%s\"is not a valid class method.\n",
                                     node->type.source, node->type.line_num, node->type.pos, node->type.str);
            }
            eve_custom_error(Eve_INVALID_CLASS_OPERAND, "file: '%s', line: %d, pos: %d, type of \"%s\"is not a valid class operand.\n",
                             node->type.source, node->type.line_num, node->type.pos, node->left->type.str);
        }
        }
    }
    if(func_is_defined(node->type.str))
    {
        tThread * fn = find_func(node->type.str, _func);

        if (fn->pcount > node->acount)
            eve_custom_error(EVE_WRONG_PARAM_NUMBER, "file: '%s', line: %d, pos: %d, parameters count of function %s is different from the definition",
                             node->type.source, node->type.line_num, node->type.pos, node->type.str);
        if ((fn->unlimited_args==0) && (fn->pcount < node->acount))
            eve_custom_error(EVE_WRONG_PARAM_NUMBER, "file: '%s', line: %d, pos: %d, parameters count of function %s is different from the definition",
                             node->type.source, node->type.line_num, node->type.pos, node->type.str);
        int i = 0;
        debugf("[func_is_defined]: func %s have %d param\n", fn->name, fn->pcount);
        for (; i < fn->pcount; i++)
        {
            debugf("[func_is_defined]: param %d is %s\n", i, fn->params[i].type);
            match_type(node->args[i], fn->params[i].type, thread);
        }
        tType tmp = find_type(fn->return_type);
        string type = find_type_pointerto(tmp);
        if(type == NULL)
            eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, type of \"%s\"is not defined.\n",
                             node->type.source, node->type.line_num,
                             node->type.pos, node->type.str);
        return type;
    }
    if (proc_is_defined(node->type.str))
    {
        eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Procedure %s can not be called in statements",
                         node->type.source, node->type.line_num, node->type.pos, node->type.str);
    }
    eve_custom_error(EVE_UNDEFINED_IDENTIFIER, "file: '%s', line: %d, pos: %d, identifiers '%s' is not defined.",
                     node->type.source, node->type.line_num, node->type.pos, node->type.str);
}

void match_type(tStatementNode * node, string type, tThread * thread)
{
    debugf("node is %s, require type %s\n", node->type.str, type);
    string typ = strdup(get_op_type(node, thread));
    if(!compare_type(typ, type))
        eve_custom_error(EVE_INVALID_DATA_TYPE, "file: '%s', line: %d, pos: %d, Invalid data type. type %s expected but %s was founded of variable/value %s",
                         node->type.source, node->type.line_num, node->type.pos ,type, typ, node->type.str);
}

