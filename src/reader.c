/*
    Copyright (c) 2010, 2011 Xeatheran Minexew

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

#include "attrib.h"
#include "lexer.h"
#include "list.h"

#include <confix2.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    Lexer* lexer;
    Token token;
    int error, terminated, token_valid;
}
ParseState;

static int get_token( ParseState* state )
{
    if ( !state->token_valid )
    {
        state->error = lexer_read( state->lexer, &state->token );

        if ( state->error == cfx2_ok )
            state->token_valid = 1;
        else
        {
            if ( state->error == cfx2_EOF )
                state->error = cfx2_ok;

            state->terminated = 1;
            state->token_valid = 0;
        }
    }

    return state->token_valid;
}

static void free_token( ParseState* state )
{
    if ( state->token_valid )
    {
        lexer_delete_token( &state->token );
        state->token_valid = 0;
    }
}

static cfx2_Node* node( ParseState* state, int min_indent )
{
    cfx2_Node* my_node, * child;

    /* Check whether there are any (more) nodes to process */
    if ( !get_token( state ) )
        return 0;

    /* node-name expected */
    if ( state->token.type != T_text )
    {
        /* Not terminating the parsing here would cause an infinite loop. */
        state->lexer->input->handle_error( state->lexer->input, state->error = cfx2_syntax_error, state->lexer->line, "Expected node name." );
        state->terminated = 1;
        return 0;
    }

    /* If this node has lower indentation than expected on this level, it's none of our bussiness */
    if ( state->token.indent < min_indent )
        return 0;

    min_indent = state->token.indent;

    /* When we know the node name, create the object */
    state->error = cfx2_create_node( state->token.text, &my_node );
    free_token( state );

    /* Read the node plain value, if it has any. */
    if ( !get_token( state ) )
        return my_node;

    if ( state->token.type == T_colon )
    {
        free_token( state );

        if ( !get_token( state ) || state->token.type != T_text )
        {
            state->lexer->input->handle_error( state->lexer->input, state->error = cfx2_syntax_error, state->lexer->line, "Expected node value after ':' symbol." );
            state->terminated = 1;
            return my_node;
        }

        state->error = cfx2_set_node_text( my_node, state->token.text );

        if ( state->error )
        {
            state->lexer->input->handle_error( state->lexer->input, state->error, state->lexer->line, "Memory allocation error." );
            state->terminated = 1;
            return my_node;
        }

        free_token( state );
    }

    /* Parse attributes, if present. */
    if ( !get_token( state ) )
        return my_node;

    if ( state->token.type == T_paren_l )
    {
        free_token( state );

        for ( ; ; )
        {
            cfx2_Attrib* attr;

            if ( !get_token( state ) || state->token.type != T_text )
            {
                state->lexer->input->handle_error( state->lexer->input, state->error = cfx2_syntax_error, state->lexer->line, "Expected attribute name." );
                state->terminated = 1;
                return my_node;
            }

            state->error = cfx2_new_attrib( &attr, state->token.text );

            if ( state->error != cfx2_ok )
            {
                state->lexer->input->handle_error( state->lexer->input, state->error, state->lexer->line, "Memory allocation error." );
                state->terminated = 1;
                return my_node;
            }

            free_token( state );

            if ( !get_token( state ) )
            {
                state->lexer->input->handle_error( state->lexer->input, state->error = cfx2_syntax_error, state->lexer->line, "Expected one of ':', ',', ')' symbols." );
                return my_node;
            }

            if ( state->token.type == T_colon )
            {
                free_token( state );

                if ( !get_token( state ) || state->token.type != T_text )
                {
                    state->lexer->input->handle_error( state->lexer->input, state->error = cfx2_syntax_error, state->lexer->line, "Expected attribute value after ':'." );
                    state->terminated = 1;
                    return my_node;
                }

                cfx2_set_attrib_value( attr, state->token.text );
                free_token( state );
            }

            list_add( my_node->attributes, attr );

            if ( !( state->token_valid || get_token( state ) ) || state->token.type != T_comma )
                break;
            else
                free_token( state );
        }

        if ( !get_token( state ) || state->token.type != T_paren_r )
        {
            state->lexer->input->handle_error( state->lexer->input, state->error = cfx2_syntax_error, state->lexer->line, "Expected ')' symbol after attribute list." );
            state->terminated = 1;
            return my_node;
        }

        free_token( state );
    }

    while ( !state->terminated )
    {
        child = node( state, min_indent + 1 );

        if ( child && state->error != cfx2_ok )
            cfx2_release_node_2( &child );

        if ( child )
            cfx2_add_child( my_node, child );
        else
            break;
    }

    return my_node;
}

static int document( ParseState* state, cfx2_Node** doc_ptr )
{
    cfx2_Node* child;
    int error;

    error = cfx2_create_node( 0, doc_ptr );

    if ( error != cfx2_ok )
        return error;

    /* Loop as long as #section gives more nodes */
    while ( !state->terminated )
    {
        child = node( state, 0 );

        if ( child && state->error != cfx2_ok )
            cfx2_release_node_2( &child );

        if ( child )
            cfx2_add_child( *doc_ptr, child );
        else
            return state->error;
    }

    return cfx2_ok;
}

libcfx2 int cfx2_read( cfx2_IInput* input, cfx2_Node** doc_ptr )
{
    ParseState state;
    Lexer* lexer;
    int lexer_error;

    /* First of all, test the arguments */
    /* This is a bit more complicated than we would like it to be,
        because of the need for releasing /input/ */

    if ( !input )
        return cfx2_param_invalid;

    if ( !doc_ptr )
    {
        input->finished( input );
        return cfx2_param_invalid;
    }

    /* Ok. Construct the lexer object */
    lexer_error = new_lexer( input, &lexer );

    if ( lexer_error )
    {
        input->finished( input );
        return lexer_error;
    }

    /* State initialization begins here */
    state.lexer = lexer;
    state.error = cfx2_ok;
    state.terminated = 0;
    state.token_valid = 0;

    *doc_ptr = 0;

    /* And launch the parsing! */
    state.error = document( &state, doc_ptr );

    /* We don't need the input any more, so let's free it. */
    delete_lexer( lexer );

    /* Release all (state->token)-allocated memory left */
    free_token( &state );

    if ( state.error > 0 )
    {
        cfx2_release_node_2( doc_ptr );
        return state.error;
    }

    return cfx2_ok;
}

libcfx2 int cfx2_read_file( const char* file_name, cfx2_Node** doc )
{
    cfx2_IInput* input;
    int error;

    error = new_buffer_input_from_file( file_name, &input );

    return error ? error : cfx2_read( input, doc );
}

libcfx2 int cfx2_read_from_string( const char* string, cfx2_Node** doc )
{
    cfx2_IInput* input;
    int error;

    error = new_buffer_input_from_string( string, &input );

    return error ? error : cfx2_read( input, doc );
}

libcfx2 cfx2_Node* cfx2_load_document( const char* file_name )
{
    cfx2_Node* doc;

    if ( cfx2_read_file( file_name, &doc ) == cfx2_ok )
        return doc;
    else
        return 0;
}
