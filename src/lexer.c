/*
    Copyright (c) 2009, 2010 Xeatheran Minexew

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

int new_lexer( cfx2_IInput* input, Lexer** lexer_ptr )
{
    Lexer* lexer;

    if ( !input )
        return cfx2_param_invalid;

    lexer = ( Lexer* )libcfx2_malloc( sizeof( Lexer ) );

    if ( !lexer )
        return cfx2_alloc_error;

    lexer->input = input;
    lexer->line = 1;
    lexer->queued_char = 0;
    lexer->current_token.type = T_none;

    *lexer_ptr = lexer;
    return cfx2_ok;
}

typedef struct
{
    char* text;
    unsigned capacity;
}
Buffer;

static int begin_buffer( Buffer* buffer )
{
    buffer->capacity = 8;

    buffer->text = ( char* )libcfx2_malloc( buffer->capacity );

    if ( !buffer->text )
        return cfx2_alloc_error;

    return cfx2_ok;
}

static int min_size( Buffer* buffer, unsigned i )
{
    if ( i + 2 >= buffer->capacity )
    {
        buffer->text = ( char* ) libcfx2_realloc( buffer->text, buffer->capacity += 4 );

        if ( !buffer->text )
            return cfx2_alloc_error;
    }

    return cfx2_ok;
}

static int require_char( Lexer* lexer, Buffer* buffer, char* next_char_ptr )
{
    if ( !lexer->input->read( lexer->input, next_char_ptr, 1 ) )
    {
        libcfx2_free( buffer->text );
        return cfx2_EOF;
    }

    return cfx2_ok;
}

/*
  Reads an identifier from the input, allocating memory as needed.
  'next_char' is the first character, which is expected to be already read by the lexer.
*/
static int read_ident( Lexer* lexer, char next_char, char** buffer_ptr )
{
    Buffer buffer;
    unsigned i = 0;

    if ( begin_buffer( &buffer ) )
        return cfx2_alloc_error;

    /* Continue until an non-ident character is met */
    while ( is_ident_char( next_char ) )
    {
        if ( min_size( &buffer, i ) )
            return cfx2_alloc_error;

        buffer.text[i++] = next_char;

        /* Read next char from the input */
        if ( !lexer->input->read( lexer->input, &next_char, 1 ) )
        {
            next_char = 0;
            break;
        }
    }

    lexer->queued_char = next_char;

    buffer.text[i] = 0;
    *buffer_ptr = buffer.text;
    return cfx2_ok;
}

/*
  Reads a text or string value from the input, allocating memory as needed.
*/
static int read_string( Lexer* lexer, char** buffer_ptr, char terminating )
{
    Buffer buffer;
    unsigned i = 0;
    char next_char;

    if ( begin_buffer( &buffer ) )
        return cfx2_alloc_error;

    for ( ; ; )
    {
        if ( require_char( lexer, &buffer, &next_char ) )
            return cfx2_EOF;

        if ( next_char == terminating )
            break;

        if ( min_size( &buffer, i ) )
            return cfx2_alloc_error;

        if ( next_char == '\\' )
            if ( require_char( lexer, &buffer, &next_char ) )
                return cfx2_EOF;

        buffer.text[i++] = next_char;
    }

    buffer.text[i] = 0;
    *buffer_ptr = buffer.text;
    return cfx2_ok;
}

/* Read a token from the input stream. */
int lexer_read( Lexer* lexer, Token* token )
{
    unsigned char resolutor;
    unsigned short indent;

    /* Check arguments */
    if ( !lexer || !lexer->input || !token )
        return cfx2_param_invalid;

    if ( lexer->current_token.type != T_none )
    {
        memcpy( token, &lexer->current_token, sizeof( Token ) );
        lexer->current_token.type = T_none;
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
        else if ( !lexer->input->read( lexer->input, &resolutor, 1 ) )
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
            if ( !lexer->input->read( lexer->input, &resolutor, 1 ) )
                return cfx2_EOF;

            if ( resolutor == '\n' )
                lexer->line++;
        }
        while ( resolutor != '}' );

        /* ...and go back to the start */
        goto label_skip_spaces;
    }

    /* default result */
    token->indent = indent;
    token->line = lexer->line;
    token->text = 0;

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
            char* text;

            /* Identifier */
            if ( is_ident_char( resolutor ) )
            {
                error_read = read_ident( lexer, resolutor, &text );

                if ( error_read )
                {
                    if ( error_read == cfx2_EOF )
                        lexer->input->handle_error( lexer->input, error_read, lexer->line, "Unexpected end of input." );
                    else
                        lexer->input->handle_error( lexer->input, error_read, lexer->line, "An internal error occured in the document lexical analyzer." );

                    return error_read;
                }

                token->type = T_text;
                token->text = text;
            }
            /* Text value */
            else if ( resolutor == '\'' )
            {
                error_read = read_string( lexer, &text, '\'' );

                if ( error_read )
                    return error_read;

                token->type = T_text;
                token->text = text;
            }
            /* String */
            else if ( resolutor == '"' )
            {
                error_read = read_string( lexer, &text, '"' );

                if ( error_read )
                    return error_read;

                token->type = T_string;
                token->text = text;
            }
            else
            {
                lexer->input->handle_error( lexer->input, cfx2_syntax_error, lexer->line, "Unexpected character in input." );
                return cfx2_syntax_error;
            }
        }
    }

    return cfx2_ok;
}

int lexer_get_current( Lexer* lexer, Token* token )
{
    int error;

    if ( lexer->current_token.type == T_none )
    {
        error = lexer_read( lexer, &lexer->current_token );

        if ( error )
            return error;
    }

    memcpy( token, &lexer->current_token, sizeof( Token ) );
    return cfx2_ok;
}

int lexer_token_is( Lexer* lexer, int token_type )
{
    if ( lexer->current_token.type == T_none )
        lexer_read( lexer, &lexer->current_token );

    return lexer->current_token.type == token_type;
}

void lexer_delete_token( Token* token )
{
    libcfx2_free( token->text );
}

int delete_lexer( Lexer* lexer )
{
    if ( lexer->current_token.type != T_none )
        lexer_delete_token( &lexer->current_token );

    lexer->input->finished( lexer->input );
    libcfx2_free( lexer );
    return cfx2_ok;
}
