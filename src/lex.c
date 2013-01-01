/*
 * Eve programming language
 * Lexer
 * check eve.h for copyright informations
 */

#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "eve.h"
#include "lex.h"
#include "memory.h"
#include "error.h"

int id_is_keyword(string id)
{
    int i = 0;
    for(; i < TokensCount; i++)
        if (!strcmp(id, Tokens[i].str))
            return Tokens[i].type;
    return IDENTIFIER;
}

string token_get_string(int token)
{
    int i = 0;
    if (token == IDENTIFIER)
        return "identifier";
    if (token == LITERAL)
        return "string";
    if (token == CHAR)
        return "char";
    for(; i < TokensCount; i++)
        if (token == Tokens[i].type)
            return Tokens[i].str;
    return NULL;
}

int is_valid_hex(char x)
{
    if ((toupper(x) <= 'F' && toupper(x)>= 'A') || (isdigit(x)))
        return 1;
    return 0;
}

token_node Lex_Step(LexInfo * li)
{
    token_node tmp;
    int size = 0;
    int dot_founded = 0;
    long tmp_pos;
    /* skipping comments, spaces and empty lines */

    int done = 1;
    do
    {
        done = 1;
        /* Skipping empty lines */
        while(li->input[li->pos] == '\n')
        {
            done = 0;
            li->line_pos = 0;
            li->pos++;
            li->line_num++;
        }

        if (isspace(li->input[li->pos]))
        {
            li->pos++;
            li->line_pos++;
            done = 0;
        }

        if(li->input[li->pos] == '#')
        {
            li->line_pos = 0;
            done = 0;
            while(li->input[li->pos] != '\n')
            {
                li->pos++;
            }
            li->line_num++;
            li->pos++;
        }
    }
    while(done==0);

    /* INIT after skipping spaces/lines */

    tmp.line_num = li->line_num;
    tmp.str = (string)eve_malloc(sizeof(char));
    tmp.TT = NODATA;

    if(li->input[li->pos] == '+')
    {
        // ++
        if(li->input[li->pos+1] == '+')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '+';
            tmp.str[1] = '+';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // +=
        else if(li->input[li->pos+1] == '=')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '+';
            tmp.str[1] = '=';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // +
        else
        {
            tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
            tmp.str[0] = '+';
            size = 1;
            li->pos++;
            li->line_pos++;
        }
    }

    else if(li->input[li->pos] == '-')
    {
        // --
        if(li->input[li->pos+1] == '-')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '-';
            tmp.str[1] = '-';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // ->
        if(li->input[li->pos+1] == '>')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '-';
            tmp.str[1] = '>';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // -=
        else if(li->input[li->pos+1] == '=')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '-';
            tmp.str[1] = '=';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // -
        else
        {
            tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
            tmp.str[0] = '-';
            size = 1;
            li->pos++;
            li->line_pos++;
        }
    }

    else if(li->input[li->pos] == '*')
    {
        // *=
        if(li->input[li->pos+1] == '=')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '*';
            tmp.str[1] = '=';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // *
        else
        {
            tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
            tmp.str[0] = '*';
            size = 1;
            li->pos++;
            li->line_pos++;
        }
    }
    else if(li->input[li->pos] == '/')
    {
        // /=
        if(li->input[li->pos+1] == '=')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '*';
            tmp.str[1] = '=';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // /
        else
        {
            tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
            tmp.str[0] = '/';
            size = 1;
            li->pos++;
            li->line_pos++;
        }
    }
    else if(li->input[li->pos] == '<')
    {
        // <<
        if(li->input[li->pos+1] == '<')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '<';
            tmp.str[1] = '<';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // <=
        if(li->input[li->pos+1] == '=')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '<';
            tmp.str[1] = '=';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // <
        else
        {
            tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
            tmp.str[0] = '<';
            size = 1;
            li->pos++;
            li->line_pos++;
        }
    }
    else if(li->input[li->pos] == '>')
    {
        // >>
        if(li->input[li->pos+1] == '>')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '>';
            tmp.str[1] = '>';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // >=
        if(li->input[li->pos+1] == '=')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '>';
            tmp.str[1] = '=';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // <
        else
        {
            tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
            tmp.str[0] = '>';
            size = 1;
            li->pos++;
            li->line_pos++;
        }
    }
    else if(li->input[li->pos] == '&')
    {
        // &&
        if(li->input[li->pos+1] == '&')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '&';
            tmp.str[1] = '&';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // &
        else
        {
            tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
            tmp.str[0] = '&';
            size = 1;
            li->pos++;
            li->line_pos++;
        }
    }
    else if(li->input[li->pos] == '|')
    {
        // ||
        if(li->input[li->pos+1] == '|')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '|';
            tmp.str[1] = '|';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // |
        else
        {
            tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
            tmp.str[0] = '|';
            size = 1;
            li->pos++;
            li->line_pos++;
        }
    }
    else if(li->input[li->pos] == '=')
    {
        // ==
        if(li->input[li->pos+1] == '=')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '=';
            tmp.str[1] = '=';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // =
        else
        {
            tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
            tmp.str[0] = '=';
            size = 1;
            li->pos++;
            li->line_pos++;
        }
    }
    else if(li->input[li->pos] == '!')
    {
        // !=
        if(li->input[li->pos+1] == '=')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '!';
            tmp.str[1] = '=';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // !
        else
        {
            tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
            tmp.str[0] = '!';
            size = 1;
            li->pos++;
            li->line_pos++;
        }
    }
    else if(li->input[li->pos] == '%')
    {
        // %=
        if(li->input[li->pos+1] == '=')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '%';
            tmp.str[1] = '=';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // %
        else
        {
            tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
            tmp.str[0] = '%';
            size = 1;
            li->pos++;
            li->line_pos++;
        }
    }

    else if ((li->input[li->pos] == '('))
    {
        li->level++;
        tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
        tmp.str[0] = '(';
        size = 1;
        li->pos++;
        li->line_pos++;
    }

    else if ((li->input[li->pos] == ')'))
    {
        li->level--;
        tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
        tmp.str[0] = ')';
        size = 1;
        li->pos++;
        li->line_pos++;
    }

    // alpha, it's keyword or identifier
    else if(isalpha(li->input[li->pos]) || li->input[li->pos] == '_')
    {
        while((isalnum(li->input[li->pos]) || li->input[li->pos] == '_' ) && (li->pos <= strlen(li->input)-1))
        {
            tmp.str = (string)eve_realloc(tmp.str, (size+1)*sizeof(char));
            tmp.str[size] = li->input[li->pos];
            li->pos++;
            li->line_pos++;
            size++;
        }
        if((strcmp(tmp.str, "true") == 0)||(strcmp(tmp.str, "false")==0))
            tmp.TT = BOOLEAN;

    }

    // digit, integer or string
    else if(isdigit(li->input[li->pos]))
    {
        tmp.TT = INTEGER;
        while((isdigit(li->input[li->pos]) || li->input[li->pos] == '.' ) && (li->pos <= strlen(li->input)-1))
        {
            if (li->input[li->pos] == '.')
            {
                tmp.TT = FLOATING;
                if (dot_founded == 0)
                    dot_founded = 1;
                else
                    eve_syntax_error(EVE_INVALID_FLOAT_NUMBER, li->source, li->line_num, li->line_pos);
            }
            tmp.str = (string)eve_realloc(tmp.str, (size+1)*sizeof(char));
            tmp.str[size] = li->input[li->pos];
            li->pos++;
            li->line_pos++;
            size++;
        }
    }

    // a string
    else if(li->input[li->pos] == '"')
    {
        do
        {
            tmp.str = (string)eve_realloc(tmp.str, (size+1)*sizeof(char));
            tmp.str[size] = li->input[li->pos];
            li->pos++;
            li->line_pos++;
            size++;
        }
        while((li->input[li->pos] != '"' ) && (li->pos <= strlen(li->input)-1));
        tmp.TT = LITERAL;
        tmp.str = (string)eve_realloc(tmp.str, (size+1)*sizeof(char));
        tmp.str[size] = li->input[li->pos];
        li->pos++;
        li->line_pos++;
        size++;
    }

    // a color ex: $0000FF
    else if(li->input[li->pos] == '$')
    {

        tmp.str = (string)eve_realloc(tmp.str, (size+1)*sizeof(char));
        tmp.str[size] = '$';
        li->pos++;
        li->line_pos++;
        tmp_pos = li->pos-1;
        size++;
        while(is_valid_hex(li->input[li->pos]) && (li->pos <= strlen(li->input)-1) )
        {

            tmp.str = (string)eve_realloc(tmp.str, (size+1)*sizeof(char));
            tmp.str[size] = li->input[li->pos];
            li->pos++;
            li->line_pos++;
            size++;
        }
        tmp.TT = HEX;
        //if(li->pos - tmp_pos-1 != 6)
        //eve_syntax_error(EVE_INVALID_HEX_NUMBER, li->source, li->line_num, li->line_pos);
    }
    else if (li->input[li->pos] == '.')
    {
        // ...
        if((li->input[li->pos+1] == '.') && (li->input[li->pos+2] == '.'))
        {
            tmp.str = (string)eve_realloc(tmp.str, 4*sizeof(char));
            tmp.str[0] = '.';
            tmp.str[1] = '.';
            tmp.str[2] = '.';
            size = 3;
            li->pos+=3;
            li->line_pos+=3;
        }
        // ..
        else if(li->input[li->pos+1] == '.')
        {
            tmp.str = (string)eve_realloc(tmp.str, 3*sizeof(char));
            tmp.str[0] = '.';
            tmp.str[1] = '.';
            size = 2;
            li->pos+=2;
            li->line_pos+=2;
        }
        // .
        else
        {
            tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
            tmp.str[0] = '.';
            size = 1;
            li->pos++;
            li->line_pos++;
        }
    }
    // one signle token
    else
    {
        tmp.str = (string)eve_realloc(tmp.str, 2*sizeof(char));
        tmp.str[0] = li->input[li->pos];
        size = 1;
        li->pos++;
        li->line_pos++;
    }
    tmp.pos = li->line_pos;
    tmp.level = li->level;
    tmp.str[size] = '\0';
    tmp.source = li->source;
    tmp.defined_as = _none_;
    if (tmp.TT == NODATA)
        tmp.TT = id_is_keyword(tmp.str);

    return tmp;
}

LexInfo Lex_Begin(const string src, const string input)
{
    LexInfo li;
    debugf("Starting lexical analysis.\n");
    /* initializations */
    li.source = src;
    li.input = input;
    li.line_num = 0;
    li.pos = 0;
    li.line_pos = 0;
    li.size = 0;
    li.TokenInfo = (token_node *)eve_malloc(sizeof(token_node));
    token_node tmpToken;
    while(li.pos<strlen(input))
    {
        tmpToken = Lex_Step(&li);
        if (tmpToken.str[0] != '\0')
        {
            li.TokenInfo = (token_node *)eve_realloc(li.TokenInfo, (li.size+1) * sizeof(token_node));
            li.TokenInfo[li.size] = tmpToken;
            debugf("%d|%s", li.size, li.TokenInfo[li.size].str);
            li.size++;
        }
    }
    debugf("|\n");
    return li;
}
