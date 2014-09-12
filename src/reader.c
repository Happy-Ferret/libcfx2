/*
    Copyright (c) 2010, 2011, 2013 Xeatheran Minexew

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
#include "config.h"
#include "lexer.h"
#include "list.h"
#include "node.h"

#include <confix2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    cfx2_Node* node;        /* base node */
    cfx2_int16_t index;     /* attribute index or -1 */
    cfx2_uint16_t offset;   /* offset within node or attrib struct */
}
Fixup_t;

typedef struct
{
    Lexer* lexer;
    int rc, terminated;

    /* shared buffers */
    int buf_level;
    size_t buf_used;
    char* bufs[PARSER_NUM_BUFS];
    char* buf;
    
    /* buffer pointer fixups */
    size_t num_fixups, max_fixups;
    Fixup_t* fixups;
}
ParseState;

static void push_buf( ParseState* state, ParseState* shadow_state )
{
    size_t offset, length;

    offset = offsetof( ParseState, buf_level );
    length = sizeof( ParseState ) - offset;

    memcpy( ( char* )shadow_state + offset, ( char* )state + offset, length );
    memset( ( char* )state + offset, 0, length );
    
    state->buf_level = -1;
}

static void pop_buf( ParseState* state, ParseState* shadow_state )
{
    size_t offset, length;

    offset = offsetof( ParseState, buf_level );
    length = sizeof( ParseState ) - offset;
    
    memcpy( ( char* )state + offset, ( char* )shadow_state + offset, length );
}

static void save_buf_to_node( ParseState* state, cfx2_Node* node )
{
    if ( state->buf_used > 0 )
    {
        node->shared = state->buf;
        
        ( ( SharedHeader_t* )node->shared )->capacity = ( size_t ) PARSER_MIN_BUF << state->buf_level;
        ( ( SharedHeader_t* )node->shared )->used = state->buf_used;
        
        if ( state->buf_level < PARSER_NUM_BUFS )
            state->bufs[state->buf_level] = ( char* )libcfx2_malloc( ( size_t ) PARSER_MIN_BUF << state->buf_level );
    }
}

static void release_bufs( ParseState* state )
{
    size_t i;

    for ( i = 0; i < PARSER_NUM_BUFS; i++ )
        libcfx2_free( state->bufs[i] );

    libcfx2_free( state->fixups );
}

static int shared_alloc( ParseState* state, char** ptr_out, const char* str_in, ptrdiff_t str_len,
        cfx2_Node* fixup_node, ptrdiff_t fixup_index, cfx2_uint16_t fixup_offset )
{
    size_t size;
    char* chunk;
    
    if ( fixup_index >= 0x7FFF )
        return cfx2_param_invalid;
    
    if ( str_len < 0 )
        str_len = strlen( str_in );
        
    size = sizeof( s_nref_t ) + str_len + 1;
    
    if ( state->buf_used == 0 )
        size += sizeof( SharedHeader_t );
    
    /* Will fit in current buffer? (if any) */
    if ( state->buf_level < 0 || state->buf_used + size > ( ( size_t ) PARSER_MIN_BUF << state->buf_level ) )
    {
        /* If not, migrate to a new one */
        
        int new_level;
        char* new_buf;
        
        size_t i;
        ptrdiff_t buf_diff;
        
        new_level = state->buf_level + 1;
        
        while ( state->buf_used + size > ( ( size_t ) PARSER_MIN_BUF << new_level ) )
            new_level++;
        
        if ( new_level < PARSER_NUM_BUFS )
        {
            if ( state->bufs[new_level] == NULL )
                state->bufs[new_level] = ( char* )libcfx2_malloc( ( size_t ) PARSER_MIN_BUF << new_level );
            
            new_buf = state->bufs[new_level];
        }
        else
            new_buf = ( char* )libcfx2_malloc( ( size_t ) PARSER_MIN_BUF << new_level );
        
        if ( state->buf_used != 0 )
        {
            memcpy( new_buf, state->buf, state->buf_used );
        
            buf_diff = new_buf - state->buf;
        
            for ( i = 0; i < state->num_fixups; i++ )
            {
                char** ptr_to_fix;

                if ( state->fixups[i].index != -1 )
                {
                    cfx2_Attrib* attr;
                    attr = &cfx2_item( state->fixups[i].node->attributes, state->fixups[i].index, cfx2_Attrib );
                    ptr_to_fix = ( char** )( ( char* )attr + state->fixups[i].offset );
                }
                else
                    ptr_to_fix = ( char** )( ( char* )state->fixups[i].node + state->fixups[i].offset );

                if ( *ptr_to_fix != NULL )
                    *ptr_to_fix += buf_diff;
            }

            if ( state->buf_level >= PARSER_NUM_BUFS )
                libcfx2_free( state->buf );
        }
        else
        {
            state->buf_used = sizeof( SharedHeader_t );
            size -= sizeof( SharedHeader_t );
        }
        
        state->buf_level = new_level;
        state->buf = new_buf;
    }
    
    chunk = state->buf + state->buf_used;
    
    *( s_nref_t* )chunk = 0;
    chunk += sizeof( s_nref_t );
    
    *ptr_out = chunk;
    state->buf_used += size;
    
    if ( str_in != NULL )
        memcpy( *ptr_out, str_in, str_len + 1 );
    
    /* add new fixup entry */
    if ( state->num_fixups + 1 > state->max_fixups )
    {
        state->max_fixups = (state->max_fixups > 0) ? state->max_fixups * 2 : 8;
        state->fixups = ( Fixup_t* )realloc( state->fixups, state->max_fixups * sizeof( Fixup_t ) );
    }
    
    state->fixups[state->num_fixups].node = fixup_node;
    state->fixups[state->num_fixups].index = ( cfx2_int16_t )fixup_index;
    state->fixups[state->num_fixups].offset = fixup_offset;
    state->num_fixups++;
    
    return 0;
}

static int get_token( ParseState* state, Token** token_out )
{
    state->rc = lexer_get_current( state->lexer, token_out );

    if ( state->rc == cfx2_ok )
        return 1;
    else
    {
        if ( state->rc == cfx2_EOF )
            state->rc = cfx2_ok;

        state->terminated = 1;
        return 0;
    }
}

static void free_token( ParseState* state )
{
    lexer_read( state->lexer, NULL );
}

static cfx2_Node* parse_node( ParseState* state, cfx2_Node* parent, int min_indent )
{
    cfx2_Node* node, * child;
    ParseState shadow_state;
    Token* token;

    /* Check whether there are any (more) nodes to process */
    if ( !get_token( state, &token ) )
        return NULL;

    /* node-name expected */
    if ( token->type != T_text )
    {
        /* Not terminating the parsing here would cause an infinite loop. */
        state->lexer->rd_opt->on_error( state->lexer->rd_opt, state->rc = cfx2_syntax_error, state->lexer->line, "Expected node name." );
        state->terminated = 1;
        return NULL;
    }

    /* If this node has lower indentation than expected on this level, it's none of our bussiness */
    if ( token->indent < min_indent )
        return NULL;

    min_indent = token->indent;

    /* When we know the node name, create the object */
    state->rc = cfx2_create_node( &node );
    
    if ( state->rc != 0 )
    {
        state->terminated = 1;
        return NULL;
    }
    
    state->rc = shared_alloc( state, &node->name, token->text, -1,
            node, -1, offsetof( cfx2_Node, name ) );

    if ( state->rc != 0 )
    {
        state->terminated = 1;
        return node;
    }
    
    free_token( state );

    /* Read the node plain value, if it has any. */
    if ( !get_token( state, &token ) )
        return node;

    if ( token->type == T_colon )
    {
        free_token( state );

        if ( !get_token( state, &token ) || token->type != T_text )
        {
            state->lexer->rd_opt->on_error( state->lexer->rd_opt, state->rc = cfx2_syntax_error, state->lexer->line, "Expected node value after ':' symbol." );
            state->terminated = 1;
            return node;
        }

        state->rc = shared_alloc( state, &node->text, token->text, -1,
                node, -1, offsetof( cfx2_Node, text ) );
        
        if ( state->rc != 0 )
        {
            state->terminated = 1;
            return node;
        }
        
        free_token( state );
    }

    /* Parse attributes, if present. */
    if ( !get_token( state, &token ) )
        return node;

    if ( token->type == T_paren_l )
    {
        free_token( state );

        for ( ; ; )
        {
            cfx2_Attrib* attr;

            if ( !get_token( state, &token ) || token->type != T_text )
            {
                state->lexer->rd_opt->on_error( state->lexer->rd_opt, state->rc = cfx2_syntax_error, state->lexer->line, "Expected attribute name." );
                state->terminated = 1;
                return node;
            }

            cfx2_attrib_new( &attr, node );

            if ( state->rc != 0 )
            {
                state->terminated = 1;
                return node;
            }

            state->rc = shared_alloc( state, &attr->name, token->text, -1,
                    node, node->attributes.length - 1, offsetof( cfx2_Attrib, name ) );
            
            if ( state->rc != 0 )
            {
                state->terminated = 1;
                return node;
            }
            
            free_token( state );

            if ( !get_token( state, &token ) )
            {
                state->lexer->rd_opt->on_error( state->lexer->rd_opt, state->rc = cfx2_syntax_error, state->lexer->line, "Expected one of ':', ',' or ')'." );
                return node;
            }

            if ( token->type == T_colon )
            {
                free_token( state );

                if ( !get_token( state, &token ) || token->type != T_text )
                {
                    cfx2_attrib_release( attr );

                    state->lexer->rd_opt->on_error( state->lexer->rd_opt, state->rc = cfx2_syntax_error, state->lexer->line, "Expected attribute value after ':'." );
                    state->terminated = 1;
                    return node;
                }

                state->rc = shared_alloc( state, &attr->value, token->text, -1,
                        node, node->attributes.length - 1, offsetof( cfx2_Attrib, value ) );
                
                if ( state->rc != 0 )
                {
                    state->terminated = 1;
                    return node;
                }
        
                free_token( state );
            }

            if ( !get_token( state, &token ) || token->type != T_comma )
                break;
            else
                free_token( state );
        }

        if ( !get_token( state, &token ) || token->type != T_paren_r )
        {
            state->lexer->rd_opt->on_error( state->lexer->rd_opt, state->rc = cfx2_syntax_error, state->lexer->line, "Expected ')' symbol after attribute list." );
            state->terminated = 1;
            return node;
        }

        free_token( state );
    }

    push_buf( state, &shadow_state );
    
    while ( !state->terminated )
    {
        child = parse_node( state, node, min_indent + 1 );

        if ( child && state->rc != cfx2_ok )
            cfx2_release_node( &child );

        if ( child )
            cfx2_add_child( node, child );
        else
            break;
    }
    
    save_buf_to_node( state, node );
    release_bufs( state );

    pop_buf( state, &shadow_state );
    
    return node;
}

static int parse_document( ParseState* state, cfx2_Node** doc_ptr )
{
    cfx2_Node* child;
    int rc;

    rc = cfx2_create_node( doc_ptr );

    if ( rc != cfx2_ok )
        return rc;

    while ( !state->terminated )
    {
        child = parse_node( state, NULL, 0 );

        if ( child && state->rc != cfx2_ok )
            cfx2_release_node( &child );

        if ( child )
            cfx2_add_child( *doc_ptr, child );
        else
            break;
    }

    save_buf_to_node( state, *doc_ptr );

    return state->rc;
}

libcfx2 int cfx2_read( cfx2_Node** doc_ptr, cfx2_RdOpt* rd_opt )
{
    ParseState state;
    Lexer lexer;
    int lexer_error;
    unsigned int i;

    /* Construct the lexer object */
    lexer_error = create_lexer( &lexer, rd_opt );

    if ( lexer_error )
        return lexer_error;

    /* State initialization begins here */
    state.lexer = &lexer;
    state.rc = cfx2_ok;
    state.terminated = 0;
    
    state.buf_level = -1;
    state.buf_used = 0;
    
    for ( i = 0; i < PARSER_NUM_BUFS; i++ )
        state.bufs[i] = NULL;
    
    state.buf = NULL;

    state.num_fixups = 0;
    state.max_fixups = 0;
    state.fixups = NULL;
    
    *doc_ptr = 0;

    /* And launch the parsing! */
    state.rc = parse_document( &state, doc_ptr );

    /* We don't need the input any more, so let's free it. */
    free( rd_opt->document );

    release_bufs( &state );

    if ( state.rc > 0 )
    {
        cfx2_release_node( doc_ptr );
        return state.rc;
    }

    return cfx2_ok;
}

libcfx2 int cfx2_read_file( cfx2_Node** doc_ptr, const char* filename, const cfx2_RdOpt* rd_opt_in )
{
    cfx2_RdOpt rd_opt;
    int rc;

    memset( &rd_opt, 0, sizeof( rd_opt ) );
    
    rc = cfx2_buffer_input_from_file( &rd_opt, filename );
    
    rd_opt.client_priv = ( void* )filename;
    
    if ( rd_opt_in != NULL )
    {
        if ( rd_opt_in->on_error != NULL )
        {
            rd_opt.client_priv = rd_opt_in->client_priv;
            rd_opt.on_error = rd_opt_in->on_error;
        }
        
        rd_opt.flags = rd_opt_in->flags;
    }

    return ( rc != 0 ) ? rc : cfx2_read( doc_ptr, &rd_opt );
}

libcfx2 int cfx2_read_from_string( cfx2_Node** doc_ptr, const char* string, const cfx2_RdOpt* rd_opt_in )
{
    cfx2_RdOpt rd_opt;
    int rc;
    
    if ( rd_opt_in != NULL )
        memcpy( &rd_opt, rd_opt_in, sizeof( cfx2_RdOpt ) );
    else
    {
        rd_opt.client_priv = NULL;
        rd_opt.on_error = NULL;
        rd_opt.flags = 0;
    }
    
    rc = cfx2_buffer_input_from_string( &rd_opt, string );

    return ( rc != 0 ) ? rc : cfx2_read( doc_ptr, &rd_opt );
}

libcfx2 cfx2_Node* cfx2_load_document( const char* filename )
{
    cfx2_Node* doc;

    if ( cfx2_read_file( &doc, filename, NULL ) == cfx2_ok )
        return doc;
    else
        return NULL;
}
