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

#include "list.h"

#include <confix2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

libcfx2 int cfx2_add_child( cfx2_Node* parent, cfx2_Node* child )
{
    if ( !parent || !child )
        return cfx2_param_invalid;

    if ( !parent->children )
        parent->children = new_list();

    list_add( parent->children, child );
    return cfx2_ok;
}

libcfx2 int cfx2_insert_child( cfx2_Node* parent, size_t index, cfx2_Node* child )
{
    if ( !parent || !child )
        return cfx2_param_invalid;

    if ( !parent->children )
        parent->children = new_list();

    cfx2_list_insert( parent->children, index, child );
    return cfx2_ok;
}

libcfx2 cfx2_Node* cfx2_create_child( cfx2_Node* parent, const char* name, const char* text, cfx2_Uniqueness uniqueness )
{
    cfx2_Node* child = 0;

    if ( !parent || uniqueness > 1 )
        return 0;

    if ( uniqueness != cfx2_multiple )
        child = cfx2_find_child( parent, name );

    if ( !child )
    {
        child = cfx2_new_node( name );
        cfx2_add_child( parent, child );
    }

    if ( text )
        cfx2_set_node_text( child, text );

    return child;
}

libcfx2 cfx2_Node* cfx2_find_child_by_test( cfx2_Node* parent, cfx2_FindTest test, void* user )
{
    unsigned i;

    if ( !parent || !parent->children || !test )
       return 0;

    for ( i = 0; i < parent->children->length; i++ )
        if ( test( i, ( cfx2_Node* )parent->children->items[i], parent, user ) == 0 )
            return parent->children->items[i];

    return 0;
}

libcfx2 cfx2_Node* cfx2_find_child( cfx2_Node* parent, const char* name )
{
    unsigned i;

    if ( !parent || !parent->children || !name )
       return 0;

    for ( i = 0; i < parent->children->length; i++ )
        if ( strcmp( ( ( cfx2_Node* )parent->children->items[i] )->name, name ) == 0 )
            return parent->children->items[i];

    return 0;
}

libcfx2 int cfx2_iterate_child_nodes( cfx2_Node* parent, cfx2_IterateCallback callback, void* user )
{
    unsigned i;

    if ( !parent || !parent->children )
       return cfx2_param_invalid;

    for ( i = 0; i < parent->children->length; i++ )
        if ( callback( i, ( cfx2_Node* )parent->children->items[i], parent, user ) == cfx2_stop )
            return cfx2_interrupted;

    return cfx2_ok;
}

libcfx2 int cfx2_remove_child( cfx2_Node* parent, cfx2_Node* child )
{
    if ( !parent || !child )
       return cfx2_param_invalid;

    if ( parent->children && list_remove_item( parent->children, child ) )
        return cfx2_ok;
    else
        return cfx2_node_not_found;
}
