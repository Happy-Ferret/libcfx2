/*
    Copyright (c) 2010 Xeatheran Minexew

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
#include "list.h"

#include <confix2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cfx2_add_attrib( cfx2_Node* node, cfx2_Attrib* attrib )
{
    if ( !node )
        return cfx2_param_invalid;

    if ( !node->attributes )
        node->attributes = new_list();

    list_add( node->attributes, attrib );
    return cfx2_ok;
}

int cfx2_new_attrib( cfx2_Attrib** attrib_ptr, const char* name )
{
    cfx2_Attrib* attrib;

    if ( !name || !name[0] )
        return cfx2_param_invalid;

    attrib = ( cfx2_Attrib* )libcfx2_malloc( sizeof( cfx2_Attrib ) );

    if ( !attrib )
        return cfx2_alloc_error;

    attrib->name = ( char* )libcfx2_malloc( strlen( name ) + 1 );
    attrib->value = 0;

    if ( !attrib->name )
    {
        libcfx2_free( attrib );
        return cfx2_alloc_error;
    }

    strcpy( attrib->name, name );

    *attrib_ptr = attrib;
    return cfx2_ok;
}

int cfx2_set_attrib_value( cfx2_Attrib* attrib, const char* value )
{
    if ( !attrib )
        return cfx2_param_invalid;

    if ( attrib->value )
        libcfx2_free( attrib->value );

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

int cfx2_delete_attrib( cfx2_Attrib* attrib )
{
    if ( !attrib )
        return cfx2_param_invalid;

    libcfx2_free( attrib->name );
    libcfx2_free( attrib->value );
    libcfx2_free( attrib );

    return cfx2_ok;
}

libcfx2 cfx2_Attrib* cfx2_find_attrib( cfx2_Node* node, const char* name )
{
    unsigned i;

    if ( !node || !node->attributes || !name || !name[0] )
       return 0;

    for ( i = 0; i < node->attributes->length; i++ )
        if ( strcmp( ( ( cfx2_Attrib* )( node->attributes->items[i] ) )->name, name ) == 0 )
            return ( cfx2_Attrib* )( node->attributes->items[i] );

    return 0;
}

libcfx2 int cfx2_remove_attrib( cfx2_Node* node, const char* name )
{
    unsigned i;

    if ( !node || !name || !name[0] )
       return cfx2_param_invalid;

    if ( !node->attributes )
        return cfx2_attrib_not_found;

    for ( i = 0; i < node->attributes->length; i++ )
        if ( strcmp( ( ( cfx2_Attrib* )( node->attributes->items[i] ) )->name, name ) == 0 )
        {
            cfx2_delete_attrib( ( cfx2_Attrib* )( node->attributes->items[i] ) );
            list_remove( node->attributes, i );
            return cfx2_ok;
        }

    return cfx2_attrib_not_found;
}

libcfx2 int cfx2_get_node_attrib( cfx2_Node* node, const char* name, const char** value )
{
    cfx2_Attrib* attrib;

    attrib = cfx2_find_attrib( node, name );

    if ( attrib != 0 && attrib->value )
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

    if ( attrib != 0 && attrib->value )
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

    if ( attrib != 0 && attrib->value )
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

    attrib = cfx2_find_attrib( node, name );

    if ( attrib )
        return cfx2_set_attrib_value( attrib, value );
    else
    {
        int err;

        if ( !node || !name || !name[0] )
            return cfx2_param_invalid;

        err = cfx2_new_attrib( &attrib, name );

        if ( err )
            return err;

        err = cfx2_set_attrib_value( attrib, value );

        if ( err )
            return err;

        return cfx2_add_attrib( node, attrib );
    }
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
