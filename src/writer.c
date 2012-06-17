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

#include "io.h"
#include "lexer.h"

#include <confix2.h>
#include <stdio.h>
#include <string.h>

static char err_desc_buffer[200];

static void write_string( const char* text, cfx2_IOutput* output )
{
    output->write( output, text, strlen( text ) );
}

static void write_string_safe( const char* text, cfx2_IOutput* output )
{
    const static char apo = '\'', esc = '\\';

    output->write( output, &apo, 1 );

    while ( *text )
    {
        if ( *text == apo || *text == esc )
            output->write( output, &esc, 1 );

        output->write( output, text, 1 );
        text++;
    }

    output->write( output, &apo, 1 );
}

static void write_string_escaped( const char* text, cfx2_IOutput* output )
{
    const char* text2;

    for ( text2 = text; *text2; text2++ )
        if ( !is_ident_char( *text2 ) )
        {
            write_string_safe( text, output );
            return;
        }

    write_string( text, output );
}

static int write_node( cfx2_Node* node, unsigned depth, cfx2_IOutput* output, cfx2_Node* parent, int is_last )
{
    unsigned i;
    int error;

    for ( i = 0; i < depth; i++ )
        output->write( output, "  ", 2 );

    if ( !node->name || !node->name[0] )
    {
        /* node name can't be empty except for the top node */
        libcfx2_snprintf( err_desc_buffer, sizeof( err_desc_buffer ) / sizeof( *err_desc_buffer ),
                "Node name empty or not specified. Parent node: %s%s%s", parent ? "`" : "", parent ? parent->name : "document root", parent ? "`" : "" );

        output->handle_error( output, cfx2_missing_node_name, err_desc_buffer );
        return cfx2_missing_node_name;
    }
    else
    {
        write_string_escaped( node->name, output );

        if ( node->text )
        {
            write_string( ": ", output );
            write_string_safe( node->text, output );
        }

        if ( node->attributes->length > 0 )
        {
            write_string( " (", output );
            for ( i = 0; i < node->attributes->length; i++ )
            {
                write_string_escaped( ( ( cfx2_Attrib* )node->attributes->items[i] )->name, output );

                if ( ( ( cfx2_Attrib* )node->attributes->items[i] )->value )
                {
                    write_string( ": ", output );
                    write_string_safe( ( ( cfx2_Attrib* )node->attributes->items[i] )->value, output );
                }

                if ( i + 1 < node->attributes->length )
                    write_string( ", ", output );
            }
            write_string( ")", output );
        }

        write_string( "\n", output );

        for ( i = 0; i < cfx2_list_length( node->children ); i++ )
        {
            error = write_node( cfx2_item( node->children, i, cfx2_Node* ), depth + 1, output, node, i >= cfx2_list_length( node->children ) - 1 );

            if ( error != cfx2_ok )
                return error;
        }

        if ( depth == 0 && !is_last )
            write_string( "\n", output );
    }

    return cfx2_ok;
}

static int write_top_node( cfx2_Node* document, cfx2_IOutput* output )
{
    unsigned i;
    int error;

    for ( i = 0; i < cfx2_list_length( document->children ); i++ )
    {
        error = write_node( cfx2_item( document->children, i, cfx2_Node* ), 0, output, 0, i >= cfx2_list_length( document->children ) - 1 );

        if ( error != cfx2_ok )
            return error;
    }

    return cfx2_ok;
}

libcfx2 int cfx2_write( cfx2_Node* document, cfx2_IOutput* output )
{
    int error;

    if ( !output )
        return cfx2_param_invalid;

    if ( !document )
    {
        output->finished( output );
        return cfx2_param_invalid;
    }

    error = write_top_node( document, output );
    output->finished( output );

    return error;
}

libcfx2 int cfx2_write_to_buffer( cfx2_Node* document, char** text, size_t* capacity, size_t* used )
{
    cfx2_IOutput* output;
    int error;

    if ( !document || !text || !capacity || !used )
        return cfx2_param_invalid;

    error = libcfx2_new_buffer_output( &output, text, capacity, used );

    if ( error != cfx2_ok )
        return error;

    return cfx2_write( document, output );
}

libcfx2 int cfx2_save_document( cfx2_Node* document, const char* file_name )
{
    cfx2_IOutput* output;
    int error;

    if ( !document || !file_name )
        return cfx2_param_invalid;

    error = new_file_output( file_name, &output );

    if ( error != cfx2_ok )
        return error;

    return cfx2_write( document, output );
}
