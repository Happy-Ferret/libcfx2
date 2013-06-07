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

#include "io.h"
#include "lexer.h"

#include <confix2.h>
#include <stdio.h>
#include <string.h>

static char err_desc_buffer[200];

static void write_string( const char* text, cfx2_WrOpt* wr_opt )
{
    wr_opt->stream_write( wr_opt, text, strlen( text ) );
}

static void write_string_safe( const char* text, cfx2_WrOpt* wr_opt )
{
    const static char apo = '\'', esc = '\\';

    wr_opt->stream_write( wr_opt, &apo, 1 );

    while ( *text )
    {
        if ( *text == apo || *text == esc )
            wr_opt->stream_write( wr_opt, &esc, 1 );

        wr_opt->stream_write( wr_opt, text, 1 );
        text++;
    }

    wr_opt->stream_write( wr_opt, &apo, 1 );
}

static void write_string_escaped( const char* text, cfx2_WrOpt* wr_opt )
{
    const char* text2;

    for ( text2 = text; *text2; text2++ )
        if ( !is_ident_char( *text2 ) )
        {
            write_string_safe( text, wr_opt );
            return;
        }

    write_string( text, wr_opt );
}

static int write_node( cfx2_Node* node, unsigned depth, cfx2_WrOpt* wr_opt,
        cfx2_Node* parent, int is_last )
{
    unsigned i;
    int rc;

    for ( i = 0; i < depth; i++ )
        wr_opt->stream_write( wr_opt, "  ", 2 );

    if ( !node->name || !node->name[0] )
    {
        /* node name can't be empty except for the top node */
        libcfx2_snprintf( err_desc_buffer, sizeof( err_desc_buffer ) / sizeof( *err_desc_buffer ),
                "Node name empty or not specified. Parent node: %s%s%s", parent ? "`" : "", parent ? parent->name : "document root", parent ? "`" : "" );

        wr_opt->on_error( wr_opt, cfx2_missing_node_name, -1, err_desc_buffer );
        return cfx2_missing_node_name;
    }
    else
    {
        write_string_escaped( node->name, wr_opt );

        if ( node->text )
        {
            write_string( ": ", wr_opt );
            write_string_safe( node->text, wr_opt );
        }

        if ( cfx2_list_length( node->attributes ) > 0 )
        {
            write_string( " (", wr_opt );
            for ( i = 0; i < cfx2_list_length( node->attributes ); i++ )
            {
                write_string_escaped( cfx2_item( node->attributes, i, cfx2_Attrib ).name, wr_opt );

                /* FIXME: This must be asserted */
                if ( cfx2_item( node->attributes, i, cfx2_Attrib ).value != NULL )
                {
                    write_string( ": ", wr_opt );
                    write_string_safe( cfx2_item( node->attributes, i, cfx2_Attrib ).value, wr_opt );
                }

                if ( i + 1 < cfx2_list_length( node->attributes ) )
                    write_string( ", ", wr_opt );
            }
            write_string( ")", wr_opt );
        }

        write_string( "\n", wr_opt );

        for ( i = 0; i < cfx2_list_length( node->children ); i++ )
        {
            rc = write_node( cfx2_item( node->children, i, cfx2_Node* ), depth + 1, wr_opt, node, i >= cfx2_list_length( node->children ) - 1 );

            if ( rc != 0 )
                return rc;
        }

        if ( depth == 0 && !is_last )
            write_string( "\n", wr_opt );
    }

    return cfx2_ok;
}

static int write_top_node( cfx2_WrOpt* wr_opt, cfx2_Node* doc )
{
    unsigned i;
    int rc;

    for ( i = 0; i < cfx2_list_length( doc->children ); i++ )
    {
        rc = write_node( cfx2_item( doc->children, i, cfx2_Node* ), 0, wr_opt, 0, i >= cfx2_list_length( doc->children ) - 1 );

        if ( rc != 0 )
            return rc;
    }

    return cfx2_ok;
}

libcfx2 int cfx2_write( cfx2_Node* doc, cfx2_WrOpt* wr_opt )
{
    int rc;

    rc = write_top_node( wr_opt, doc );
    wr_opt->stream_close( wr_opt );
 
    return rc;
}

libcfx2 int cfx2_write_to_buffer( cfx2_Node* doc, char** text, size_t* capacity, size_t* used )
{
    cfx2_WrOpt wr_opt;
    int rc;

    rc = cfx2_memory_stream( &wr_opt, text, capacity, used );

    if ( rc != 0 )
        return rc;

    return cfx2_write( doc, &wr_opt );
}

libcfx2 int cfx2_save_document( cfx2_Node* doc, const char* filename )
{
    cfx2_WrOpt wr_opt;
    int rc;

    rc = cfx2_file_stream( &wr_opt, filename );

    if ( rc != 0 )
        return rc;

    return cfx2_write( doc, &wr_opt );
}
