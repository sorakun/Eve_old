/*
 * Eve programming language
 * C Code generator
 * check eve.h for copyright informations
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "eve.h"
#include "error.h"
#include "lex.h"
#include "parse.h"
#include "includes.h"

FILE * out;

string get_token_str(token_node token)
{
    switch (token.TT)
    {
    case AND:
        return "&&";
    case OR:
        return "||";
    case NOT:
        return "!";
    case XOR:
        return "^";
    case '@':
        return "&";
    case '^':
        return "*";
    case ELIF:
        return "else if";
    case NIL:
        return "NULL";
    default:
        return token.str;
    }
}

/* EOS = END OF STATEMENT */
void outputnode(tStatementNode * node, int EOS)
{

    int i=0;
    debugf("OUTPUT NODE = %s\n", node->type.str);
    if (func_is_defined(node->type.str) || proc_is_defined(node->type.str))
    {
        fprintf(out, "%s(", node->type.str);
        for (i=0; i<node->acount; i++)
        {
            debugf("param %d of %s is %s\n", i, node->type.str, node->args[i]->type.str);
            outputnode(node->args[i],  0);
            if (i+1 < node->acount)
                fprintf(out, ", ");
        }
        fprintf(out, ")", node->type.str);
    }
    else if (node->type.TT == BEGIN)
    {
        fprintf(out, "{\n");
        gen_code_func(node->thread);
        fprintf(out, "\n}\n");
    }
    else if (node->type.TT == IF || node->type.TT == ELIF || node->type.TT == ELSE)
    {

        fprintf(out, "%s", get_token_str(node->type));
        /* <ifstatement> ::= <if>"(" <condition> ")" "{" <instruction> "}"
                           | <ifstatement> <else if>"(" <condition> ")" "{" <instruction> "}"
                           | <else> "{" <instruction> "}"
        */
        if (node->type.TT != ELSE)
        {
            fprintf(out, " ( ");
            outputnode(node->condition,  0);
            fprintf(out, " ) ");
        }

        fprintf(out, "\n{\n");
        EOS = 0;
        gen_code_func(node->thread);
        fprintf(out, "}\n");
    }
    else if (node->type.TT == FOR)
    {
        fprintf(out, "for(");
        outputnode(node->from, 0);
        fprintf(out, ";");
        outputnode(node->from->left, 0);
        fprintf(out, "!=");
        outputnode(node->to, 0);
        fprintf(out, ";");
        fprintf(out, "%s ++)\n{\n", node->from->left->type.str);
        gen_code_func(node->thread);
        fprintf(out, "}\n");
    }
    else if (node->type.TT == WHILE)
    {
        fprintf(out, "while (");
        outputnode(node->condition, 0);
        fprintf(out, ")\n{\n");
        gen_code_func(node->thread);
        fprintf(out, "}\n");
    }
    else if (node->type.TT == REPEAT)
    {
        fprintf(out, "do {\n");
        gen_code_func(node->thread);
        fprintf(out, "}\n");
        fprintf(out, "while(!(");
        outputnode(node->condition, 0);
        fprintf(out, "));\n");
    }
    else if (node->type.TT == RETURN)
    {
        fprintf(out, "return ");
        if(node->unary!=NULL)
            outputnode(node->unary, 0);
        //fprintf(out, "");
    }
    else if (node->type.TT == BREAK || node->type.TT == CONTINUE)
    {
        fprintf(out, node->type.str);
    }
    else if (token_is_op(node->type))
    {
        debugf("[codegen]: node %s is operation.\n", node->type.str);
        fprintf(out, "(");
        if (op_is_unary(node->type) && (node->left == NULL))
        {
            debugf("[codegen]: op %s is unary\n", node->type.str);
            fprintf(out, " %s ", get_token_str(node->type));
            outputnode(node->unary, 0);
        }
        else
        {
            /* special tokens, like index ":" or addintion for string, or string multiplication, etc ... */
            if(node->type.TT == ':')
            {
                outputnode(node->left,  0);
                fprintf(out, " [ ");
                outputnode(node->right,  0);
                fprintf(out, " ] ");
            }

            else if(node->type.TT == '+')
            {
                if(strcmp(get_op_type(node->left, node->parent_thread), "str") == 0)
                {
                    // Right token is automatically a string

                    fprintf(out, "addstr(");
                    outputnode(node->left,  0);
                    fprintf(out, ", ");
                    outputnode(node->right,  0);
                    fprintf(out, ")");
                }
                else
                {
                    outputnode(node->left,  0);
                    fprintf(out, " %s ", get_token_str(node->type));
                    outputnode(node->right,  0);
                }
            }
            else if (node->type.TT == '*')
            {
                int left_is_string = (strcmp(get_op_type(node->left, node->parent_thread), "str") == 0);
                if ((left_is_string) ||
                    (strcmp(get_op_type(node->right, node->parent_thread), "str") == 0))
                {
                    //check which node is the string:
                    fprintf(out, "repstr(");

                    if(left_is_string)
                    {
                        outputnode(node->left,  0);
                        fprintf(out, ", ");
                        outputnode(node->right,  0);
                    }
                    else
                    {
                        outputnode(node->right,  0);
                        fprintf(out, ", ");
                        outputnode(node->left,  0);
                    }

                    fprintf(out, ")");
                }
                else
                {
                    outputnode(node->left,  0);
                    fprintf(out, " %s ", get_token_str(node->type));
                    outputnode(node->right,  0);
                }
            }
            else
            {
                outputnode(node->left,  0);
                fprintf(out, " %s ", get_token_str(node->type));
                outputnode(node->right,  0);
            }
        }
        fprintf(out, ")");
    }
    else
        fprintf(out, " %s ", get_token_str(node->type));

    if (EOS == 1)
        fprintf(out, ";\n", get_token_str(node->type));
}

void generate_function(tThread * func)
{
    fprintf(out, "%s %s (", func->return_type, func->name);
    int i = 0;
    for(; i<func->pcount; i++)
    {
        fprintf(out, "%s %s", func->params[i].type, func->params[i].name);
        if (i+1 < func->pcount)
            fprintf(out, ", ");
    }
    if(func->unlimited_args)
        fprintf(out, "...");
    fprintf(out, ")\n{\n");
    gen_code_func(func);
    fprintf(out, "}\n");
}

void gen_code_func(tThread * func)
{
    // generating variables
    int i;
    // generating global variables
    for (i=0; i < func->vcount; i++)
    {
        if (func->vars[i].mod != _none)
            fprintf(out, "%s ", mod_to_str(func->vars[i].mod));
        fprintf(out, "%s ", func->vars[i].type);

        fprintf(out, "%s;\n", func->vars[i].name);
    }

    for(i =0; i <func->icount; i++)
    {
        debugf("// generating instruction # %d of %d thread %s it's %s\n", i+1, func->icount, func->name, func->instructions[i]->type.str);
        if (func->instructions[i]->type.TT != BEGIN &&
                func->instructions[i]->type.TT != REPEAT && func->instructions[i]->type.TT != WHILE &&
                func->instructions[i]->type.TT !=  IF    && func->instructions[i]->type.TT !=  ELIF  &&
                func->instructions[i]->type.TT != ELSE)
            outputnode(func->instructions[i],  1);
        else
            outputnode(func->instructions[i],  0);
    }
}

string gen_code(string file, tThread * main)
{
    int i;
    string fname = strdup(file);
    string ext = strdup(".c");
    strcat(fname, ext);
    out= fopen(fname, "w");
    fprintf(out, "#include <eve.h>\n#include <stdlib.h>\n#include <string.h>\n");
    for(i=0; i<included_files_count; i++)
    {
        fprintf(out, "#include <%s.h>\n", included_files[i]);
    }

    // generating types
    for (i = BASIC_TYPES_COUNT; i<global_types_count; i++)
    {
        if((global_types[i].type_kind == __none) || (global_types[i].type_kind == __array))
        {
            fprintf(out, "typedef %s ", global_types[i].pointerto);
            if(global_types[i].pointer)
            {
                fprintf(out, " *");
            }
            fprintf(out, " %s;\n", global_types[i].name);
        }
        else if (global_types[i].type_kind == __enum)
        {
            fprintf(out, "typedef enum %s{\n", global_types[i].name);
            int j = 0;
            for(; j<global_types[i].ecount; j++)
            {
                fprintf(out, "%s, ", global_types[i].enums[j]);
            }
            fprintf(out, "\n}%s;\n", global_types[i].name);
        }
    }

    // generating global variables
    for (i=0; i < main->vcount; i++)
    {
        if (main->vars[i].mod != _none)
            fprintf(out, "%s ", mod_to_str(main->vars[i].mod));
        fprintf(out, "%s ", main->vars[i].type);

        fprintf(out, "%s;\n", main->vars[i].name);
    }

    for (i=0; i < global_functions_count; i++)
    {
        if(!global_functions[i]->cdef)
            generate_function(global_functions[i]);
    }
    fclose(out);
    return fname;
}
