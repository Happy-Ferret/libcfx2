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

#include "list.h"

#include <confix2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

libcfx2 int cfx2_add_child( cfx2_Node* parent, cfx2_Node* child )
{
    cfx2_Node** p_child;

    p_child = ( cfx2_Node** )list_add_item( &parent->children, sizeof( cfx2_Node* ) );
    
    if ( p_child == NULL )
        return cfx2_alloc_error;

    *p_child = child;
    return cfx2_ok;
}

/*
libcfx2 int cfx2_insert_child( cfx2_Node* parent, size_t index, cfx2_Node* child )
{
    if ( !parent || !child )
        return cfx2_param_invalid;

    if ( !parent->children )
        parent->children = new_list();

    cfx2_list_insert( parent->children, index, child );
    return cfx2_ok;
}
*/

libcfx2 cfx2_Node* cfx2_create_child( cfx2_Node* parent, const char* name, const char* text, cfx2_Uniqueness uniqueness )
{
    cfx2_Node* child;

    if ( uniqueness > 1 )
        return NULL;

    if ( uniqueness != cfx2_multiple )
        child = cfx2_find_child( parent, name );
    else
        child = NULL;

    if ( child == NULL )
    {
        child = cfx2_new_node( name );
        cfx2_add_child( parent, child );
    }

    if ( text != NULL )
        cfx2_set_node_text( child, text );

    return child;
}

libcfx2 cfx2_Node* cfx2_find_child_by_test( cfx2_Node* parent, cfx2_FindTest test, void* user )
{
    size_t i;

    for ( i = 0; i < cfx2_list_length( parent->children ); i++ )
        if ( test( i, cfx2_item( parent->children, i, cfx2_Node* ), parent, user ) == 0 )
            return cfx2_item( parent->children, i, cfx2_Node* );

    return NULL;
}

libcfx2 cfx2_Node* cfx2_find_child( cfx2_Node* parent, const char* name )
{
    size_t i;

    for ( i = 0; i < cfx2_list_length( parent->children ); i++ )
        if ( strcmp( cfx2_item( parent->children, i, cfx2_Node* )->name, name ) == 0 )
            return cfx2_item( parent->children, i, cfx2_Node* );

    return NULL;
}

libcfx2 int cfx2_iterate_child_nodes( cfx2_Node* parent, cfx2_IterateCallback callback, void* user )
{
    size_t i;

    for ( i = 0; i < cfx2_list_length( parent->children ); i++ )
        if ( callback( i, cfx2_item( parent->children, i, cfx2_Node* ), parent, user ) == cfx2_stop )
            return cfx2_interrupted;

    return cfx2_ok;
}

/*
libcfx2 int cfx2_remove_child( cfx2_Node* parent, cfx2_Node* child )
{
    if ( parent->children && list_remove_item( parent->children, child ) )
        return cfx2_ok;
    else
        return cfx2_node_not_found;
}
*/
