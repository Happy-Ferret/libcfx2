/*
    Copyright (c) 2010, 2013 Xeatheran Minexew

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
#include "list.h"
#include "node.h"

#include <confix2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cfx2_attrib_new( cfx2_Attrib** ptr, cfx2_Node* node )
{
    cfx2_Attrib* attrib;
    
    attrib = ( cfx2_Attrib* )list_add_item( &node->attributes, sizeof( cfx2_Attrib ) );
    
    if ( attrib == NULL )
        return cfx2_alloc_error;
    
    attrib->name = NULL;
    attrib->value = NULL;
    
    *ptr = attrib;
    return 0;
}

void cfx2_attrib_release( cfx2_Attrib* attrib )
{
    cfx2_sfree( attrib->name );
    cfx2_sfree( attrib->value );
}

int cfx2_attrib_set_value( cfx2_Attrib* attrib, const char* value )
{
    if ( !attrib )
        return cfx2_param_invalid;

    if ( attrib->value )
        cfx2_sfree( attrib->value );

    if ( value )
    {
        attrib->value = ( char* )libcfx2_malloc( strlen( value ) + 1 );

        if ( !attrib->value )
            return cfx2_alloc_error;

        strcpy( attrib->value, value );
    }
    else
        attrib->value = 0;

    return cfx2_ok;
}

libcfx2 cfx2_Attrib* cfx2_find_attrib( cfx2_Node* node, const char* name )
{
    size_t i;

    for ( i = 0; i < cfx2_list_length( node->attributes ); i++ )
        if ( strcmp( cfx2_item( node->attributes, i, cfx2_Attrib ).name, name ) == 0 )
            return &cfx2_item( node->attributes, i, cfx2_Attrib );

    return 0;
}

/*libcfx2 int cfx2_remove_attrib( cfx2_Node* node, const char* name )
{
    size_t i;

    if ( node == NULL || name == NULL || name[0] == 0 )
        return NULL;
    
    for ( i = 0; i < cfx2_list_length( node->attributes ); i++ )
        if ( strcmp( cfx2_item( node->attributes, i, cfx2_Attrib ).name, name ) == 0 )
        {
            cfx2_delete_attrib( ( cfx2_Attrib* )( node->attributes->items[i] ) );
            list_remove( node->attributes, i );
            return cfx2_ok;
        }

    return cfx2_attrib_not_found;
}*/

libcfx2 int cfx2_get_node_attrib( cfx2_Node* node, const char* name, const char** value )
{
    cfx2_Attrib* attrib;

    attrib = cfx2_find_attrib( node, name );

    if ( attrib != NULL && attrib->value != NULL )
    {
        *value = attrib->value;
        return cfx2_ok;
    }
    else
        return cfx2_attrib_not_found;
}

libcfx2 int cfx2_get_node_attrib_int( cfx2_Node* node, const char* name, long* value )
{
    cfx2_Attrib* attrib;

    attrib = cfx2_find_attrib( node, name );

    if ( attrib != NULL && attrib->value != NULL )
    {
        *value = strtol( attrib->value, 0, 0 );
        return cfx2_ok;
    }
    else
        return cfx2_attrib_not_found;
}

libcfx2 int cfx2_get_node_attrib_float( cfx2_Node* node, const char* name, double* value )
{
    cfx2_Attrib* attrib;

    attrib = cfx2_find_attrib( node, name );

    if ( attrib != NULL && attrib->value != NULL )
    {
        *value = strtod( attrib->value, 0 );
        return cfx2_ok;
    }
    else
        return cfx2_attrib_not_found;
}

libcfx2 int cfx2_set_node_attrib( cfx2_Node* node, const char* name, const char* value )
{
    cfx2_Attrib* attrib;
    int rc;

    attrib = cfx2_find_attrib( node, name );

    if ( attrib != NULL )
        return cfx2_attrib_set_value( attrib, value );

    rc = cfx2_attrib_new( &attrib, node );

    if ( rc != 0 )
        return rc;

    /* FIXME: Further returns leave unitialized attribute */
    
    rc = cfx2_salloc( &attrib->name, NULL, node, strlen( name ) + 1, name, cfx2_use_shared_buffer );
    
    if ( rc != 0 )
        return rc;
    
    rc = cfx2_salloc( &attrib->value, NULL, node, strlen( value ) + 1, value, cfx2_use_shared_buffer );

    if ( rc != 0 )
        return rc;

    return 0;
}

libcfx2 int cfx2_set_node_attrib_int( cfx2_Node* node, const char* name, long value )
{
    char buffer[40];

    libcfx2_snprintf( buffer, sizeof( buffer ) / sizeof( *buffer ), "%li", value );
    return cfx2_set_node_attrib( node, name, buffer );
}

libcfx2 int cfx2_set_node_attrib_float( cfx2_Node* node, const char* name, double value )
{
    char buffer[40];

    libcfx2_snprintf( buffer, sizeof( buffer ) / sizeof( *buffer ), "%g", value );
    return cfx2_set_node_attrib( node, name, buffer );
}
