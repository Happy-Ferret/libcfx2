
#include "attrib.h"
#include "config.h"
#include "lexer.h"
#include "list.h"
#include "node.h"

#include <confix2.h>
#include <stdlib.h>
#include <string.h>

/* TODO: this could really use some documentation because of the function's recursiveness */

static cfx2_ResultType attrib_process_command( cfx2_Node* base, const char* command,
        char* buffer, int allow_modifications, void** output )
{
    unsigned pos = 0;
    cfx2_Attrib* attrib;

    /* Read an identifier form the command string. */
    while ( is_ident_char( *command ) )
        buffer[pos++] = *( command++ );

    buffer[pos] = 0;

    switch ( *command )
    {
        case 0:
        case ':':
        case '/':
        case '.':
            break;

        default:
            return cfx2_fail;
    }

    attrib = cfx2_find_attrib( base, buffer );

    if ( !attrib )
    {
        if ( allow_modifications )
        {
            int rc;
            
            rc = cfx2_attrib_new( &attrib, base );
            
            if ( rc != 0 )
                return cfx2_fail;
            
            rc = cfx2_salloc( &attrib->name, base, NULL, strlen( buffer ) + 1, buffer, cfx2_use_shared_buffer );
            
            if ( rc != 0 )
                return rc;
        }
        else
            return cfx2_fail;
    }

    if ( *command == 0 )
    {
        if ( output )
        {
            *output = ( void* )attrib;
            return cfx2_attrib;
        }
        else
            return cfx2_void;
    }
    else if ( *command == ':' && allow_modifications )
    {
        cfx2_attrib_set_value( attrib, command + 1 );

        if ( output )
        {
            *output = ( void* )attrib;
            return cfx2_attrib;
        }
        else
            return cfx2_void;
    }
    else
        return cfx2_fail;
}

static cfx2_ResultType process_command( cfx2_Node* base, const char* command, char* buffer, int allow_modifications, void** output )
{
    unsigned pos = 0;
    cfx2_Node* child;

    /* Read an identifier form the command string. */
    while ( is_ident_char( *command ) )
        buffer[pos++] = *( command++ );

    buffer[pos] = 0;

    switch ( *command )
    {
        case 0:
        case ':':
        case '/':
        case '.':
            break;

        default:
            return cfx2_fail;
    }

    if ( !buffer[0] )
        child = base;
    else
        child = cfx2_find_child( base, buffer );

    if ( !child )
    {
        /*
            The requested node does not exist.
            We'll try to create it then.
            If we aren't allowed to do so, we return NULL.
        */

        if ( allow_modifications )
            child = cfx2_create_child( base, buffer, 0, 0 );
        else
            return cfx2_fail;
    }

    if ( *command == 0 )
    {
        if ( output )
        {
            *output = ( void* )child;
            return cfx2_node;
        }
        else
            return cfx2_void;
    }
    else if ( *command == '/' )
        return process_command( child, command + 1, buffer, allow_modifications, output );
    else if ( *command == '.' )
        return attrib_process_command( child, command + 1, buffer, allow_modifications, output );
    else if ( *command == ':' && allow_modifications )
    {
        cfx2_set_node_text( child, command + 1 );

        if ( output )
        {
            *output = ( void* )child;
            return cfx2_node;
        }
        else
            return cfx2_void;
    }
    else
        return cfx2_fail;
}

libcfx2 cfx2_ResultType cfx2_query( cfx2_Node* base, const char* command,
        int allow_modifications, void** output )
{
    char* buffer;
    cfx2_ResultType type;

    if ( !base || !command )
        return cfx2_fail;

    /* this one is used by process_command() to store identifiers */
    buffer = ( char* )libcfx2_malloc( strlen( command ) + 1 );

    /* enter the matrix */
    type = process_command( base, command, buffer, allow_modifications, output );

    libcfx2_free( buffer );

    return type;
}

libcfx2 cfx2_Node* cfx2_query_node( cfx2_Node* base, const char* command, int allow_modifications )
{
    void* output;

    if ( cfx2_query( base, command, allow_modifications, &output ) == cfx2_node )
        return ( cfx2_Node* )output;
    else
        return 0;
}

libcfx2 cfx2_Attrib* cfx2_query_attrib( cfx2_Node* base, const char* command, int allow_modifications )
{
    void* output;

    if ( cfx2_query( base, command, allow_modifications, &output ) == cfx2_attrib )
        return ( cfx2_Attrib* )output;
    else
        return 0;
}

libcfx2 const char* cfx2_query_value( cfx2_Node* base, const char* command )
{
    void* output;
    cfx2_ResultType result;

    result = cfx2_query( base, command, 0, &output );

    if ( result == cfx2_node )
        return ( ( cfx2_Node* )output )->text;
    else if ( result == cfx2_attrib )
        return ( ( cfx2_Attrib* )output )->value;
    else
        return 0;
}
