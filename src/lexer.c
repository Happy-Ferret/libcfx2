/*
    Copyright (c) 2009, 2010, 2013 Xeatheran Minexew

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

#include "config.h"
#include "lexer.h"

#include <stdlib.h>
#include <string.h>

static int ensure_enough_space( Lexer* lexer, unsigned int i )
{
    if ( i + 2 > lexer->current_token_text_capacity )
    {
        lexer->current_token_text_capacity = ( lexer->current_token_text_capacity != 0 )
                ? lexer->current_token_text_capacity * 2 : 32;

        lexer->current_token.text = ( char* )libcfx2_realloc( lexer->current_token.text,
                lexer->current_token_text_capacity );

        if ( lexer->current_token.text == NULL )
            return 0;
    }

    return 1;
}

static int read_ident( Lexer* lexer, char next_char )
{
    unsigned i = 0;

    /* Continue until an non-ident character is met */
    while ( is_ident_char( next_char ) )
    {
        if ( !ensure_enough_space( lexer, i ) )
            return cfx2_alloc_error;

        lexer->current_token.text[i++] = next_char;

        /* Read next char from the input */
        if ( !lexer->rd_opt->stream_read( lexer->rd_opt, &next_char, 1 ) )
        {
            next_char = 0;
            break;
        }
    }

    lexer->queued_char = next_char;

    lexer->current_token.text[i] = 0;
    return cfx2_ok;
}

static int read_string( Lexer* lexer, char terminating )
{
    unsigned i = 0;
    char next_char;

    for ( ; ; )
    {
        if ( !lexer->rd_opt->stream_read( lexer->rd_opt, &next_char, 1 ) )
            return cfx2_EOF;

        if ( next_char == terminating )
            break;

        if ( !ensure_enough_space( lexer, i ) )
            return cfx2_alloc_error;

        if ( next_char == '\\' )
            if ( !lexer->rd_opt->stream_read( lexer->rd_opt, &next_char, 1 ) )
                return cfx2_EOF;

        lexer->current_token.text[i++] = next_char;
    }

    lexer->current_token.text[i] = 0;
    return cfx2_ok;
}

int create_lexer( Lexer** lexer_ptr, cfx2_RdOpt* rd_opt )
{
    Lexer* lexer;

    lexer = ( Lexer* )libcfx2_malloc( sizeof( Lexer ) );

    if ( !lexer )
        return cfx2_alloc_error;

    lexer->rd_opt = rd_opt;
    lexer->line = 1;
    lexer->queued_char = 0;
    lexer->current_token.type = T_none;
    lexer->current_token.text = NULL;
    lexer->current_token_is_valid = 0;
    lexer->current_token_text_capacity = 0;

    *lexer_ptr = lexer;
    return cfx2_ok;
}

int lexer_get_current( Lexer* lexer, Token** token_out )
{
    Token* token;
    char resolutor;
    unsigned short indent;

    if ( lexer->current_token_is_valid )
    {
        if ( token_out != NULL )
            *token_out = &lexer->current_token;

        return cfx2_ok;
    }

    /* Skip all spaces, newlines, tabs etc. */
    label_skip_spaces:

    indent = 0;

    do
    {
        if ( lexer->queued_char )
        {
            resolutor = lexer->queued_char;
            lexer->queued_char = 0;
        }
        else if ( !lexer->rd_opt->stream_read( lexer->rd_opt, &resolutor, 1 ) )
            return cfx2_EOF;

        if ( resolutor == '\n' )
        {
            lexer->line++;
            indent = 0;
        }
        else if ( resolutor == ' ' )
            indent++;
        else if ( resolutor == '\t' )
            indent += 4;
    }
    while ( isspace( resolutor ) );

    /* A multi-line comment was found */
    if ( resolutor == '{' )
    {
        /* just look for '}' and don't forget to count lines */
        do
        {
            if ( !lexer->rd_opt->stream_read( lexer->rd_opt, &resolutor, 1 ) )
                return cfx2_EOF;

            if ( resolutor == '\n' )
                lexer->line++;
        }
        while ( resolutor != '}' );

        /* ...and go back to the start */
        goto label_skip_spaces;
    }

    token = &lexer->current_token;

    /* default result */
    token->indent = indent;
    token->line = lexer->line;

    /* Find out which token is it. */
    switch ( resolutor )
    {
        case '*': token->type = T_asterisk; break;
        case ':': token->type = T_colon; break;
        case ',': token->type = T_comma; break;
        case '=': token->type = T_equals; break;
        case '(': token->type = T_paren_l; break;
        case ')': token->type = T_paren_r; break;

        default:
        {
            int error_read;

            /* Identifier */
            if ( is_ident_char( resolutor ) )
            {
                error_read = read_ident( lexer, resolutor );

                if ( error_read )
                {
                    if ( error_read == cfx2_EOF )
                        lexer->rd_opt->on_error( lexer->rd_opt, error_read, lexer->line, "Unexpected end of input." );
                    else
                        lexer->rd_opt->on_error( lexer->rd_opt, error_read, lexer->line, "An internal error occured in the document lexical analyzer." );

                    return error_read;
                }

                token->type = T_text;
            }
            /* Text value */
            else if ( resolutor == '\'' )
            {
                error_read = read_string( lexer, '\'' );

                if ( error_read )
                    return error_read;

                token->type = T_text;
            }
            /* String */
            else if ( resolutor == '"' )
            {
                error_read = read_string( lexer, '"' );

                if ( error_read )
                    return error_read;

                token->type = T_string;
            }
            else
            {
                lexer->rd_opt->on_error( lexer->rd_opt, cfx2_syntax_error, lexer->line, "Unexpected character in input." );
                return cfx2_syntax_error;
            }
        }
    }

    if ( token_out != NULL )
        *token_out = token;
    
    lexer->current_token_is_valid = 1;
    return cfx2_ok;
}

int lexer_read( Lexer* lexer, Token** token_out )
{
    int rc;

    if ( !lexer->current_token_is_valid )
    {
        if ( ( rc = lexer_get_current( lexer, token_out ) ) != cfx2_ok )
            return rc;
    }

    if ( token_out != NULL )
        *token_out = &lexer->current_token;

    lexer->current_token_is_valid = 0;
    return cfx2_ok;
}

int lexer_token_is( Lexer* lexer, int token_type )
{
    if ( !lexer->current_token_is_valid )
    {
        if ( lexer_read( lexer, NULL ) != cfx2_ok )
            return 0;
    }

    return lexer->current_token.type == token_type;
}

void lexer_delete_token( Token* token )
{
}

int delete_lexer( Lexer* lexer )
{
    if ( lexer->current_token.type != T_none )
        lexer_delete_token( &lexer->current_token );

    libcfx2_free( lexer->current_token.text );

    lexer->rd_opt->stream_close( lexer->rd_opt );
    libcfx2_free( lexer );
    return cfx2_ok;
}
