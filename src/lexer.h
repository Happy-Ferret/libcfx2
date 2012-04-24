/*
    Copyright (c) 2009, 2010, 2011 Xeatheran Minexew

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#ifndef __libcfx2_lexer_h__
#define __libcfx2_lexer_h__

#include "io.h"

#include <confix2.h>
#include <ctype.h>

typedef short TokenType;
#define T_none      ( -1 )
#define T_error     0
#define T_colon     1
#define T_comma     2
#define T_text      3
#define T_paren_l   4
#define T_paren_r   5
#define T_asterisk  6
#define T_string    7
#define T_equals    8

#define is_ident_char( c ) ( isalnum( ( unsigned char ) c ) || (c) == '_' || (c) == '-' || (c) == '~' || (c) == '!' || (c) == '@' || (c) == '#' || (c) == '$' || (c) == '%' )

/*

    Symbols:
        *:,=()  - tokens in cfx2 documents
        '"      - test values & strings
        ./      - paths
        {}      - DQL

*/

typedef struct
{
    TokenType type;
    unsigned short indent;
    int line;
    char* text;
}
Token;

typedef struct
{
    cfx2_IInput* input;
    unsigned line;
    char queued_char;

    Token current_token;
}
Lexer;

int new_lexer( cfx2_IInput*, Lexer** lexer_ptr );
int lexer_read( Lexer* lexer, Token* token );
int lexer_get_current( Lexer* lexer, Token* token );
int lexer_token_is( Lexer* lexer, int token_type );
void lexer_delete_token( Token* );
int delete_lexer( Lexer* lexer );

#endif
