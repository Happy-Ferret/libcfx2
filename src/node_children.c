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
#include "node.h"

#include <confix2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int belongs_to( const char* string, cfx2_Node* owner )
{
    return string >= owner->shared
            && owner->shared != NULL
            && string < owner->shared + ( ( SharedHeader_t* )owner->shared )->capacity;
}

static int free_from_possible_owner( char** string, cfx2_Node* owner )
{
    if ( belongs_to( *string, owner ) )
        return cfx2_salloc( string, NULL, NULL, strlen( *string ) + 1, *string, 0 );
    else
        return 0;
}

libcfx2 int cfx2_add_child( cfx2_Node* parent, cfx2_Node* child )
{
    cfx2_Node** p_child;

    p_child = ( cfx2_Node** )cfx2_list_add_item( &parent->children, sizeof( cfx2_Node* ) );
    
    if ( p_child == NULL )
        return cfx2_alloc_error;

    *p_child = child;
    return cfx2_ok;
}

libcfx2 int cfx2_insert_child( cfx2_Node* parent, size_t index, cfx2_Node* child )
{
    cfx2_Node** p_child;

    p_child = ( cfx2_Node** )cfx2_list_insert_item( &parent->children, sizeof( cfx2_Node* ), index );
    
    if ( p_child == NULL )
        return cfx2_alloc_error;

    *p_child = child;
    return cfx2_ok;
}

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

libcfx2 cfx2_Node* cfx2_find_child( cfx2_Node* parent, const char* name )
{
    size_t i;

    for ( i = 0; i < cfx2_list_length( parent->children ); i++ )
        if ( strcmp( cfx2_item( parent->children, i, cfx2_Node* )->name, name ) == 0 )
            return cfx2_item( parent->children, i, cfx2_Node* );

    return NULL;
}

libcfx2 cfx2_Node* cfx2_find_child_by_test( cfx2_Node* parent, cfx2_FindTest test, void* user )
{
    size_t i;

    for ( i = 0; i < cfx2_list_length( parent->children ); i++ )
        if ( test( i, cfx2_item( parent->children, i, cfx2_Node* ), parent, user ) == 0 )
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

libcfx2 int cfx2_remove_child( cfx2_Node* parent, cfx2_Node* child )
{
    int rc;
    size_t i;

    if ( ( rc = free_from_possible_owner( &child->name, parent ) ) != 0 )
        return rc;

    if ( ( rc = free_from_possible_owner( &child->text, parent ) ) != 0 )
        return rc;

    for ( i = 0; i < cfx2_list_length( child->attributes ); i++ )
    {
        if ( ( rc = free_from_possible_owner( &cfx2_item( child->attributes, i, cfx2_Attrib ).name, parent ) ) != 0 )
            return rc;

        if ( ( rc = free_from_possible_owner( &cfx2_item( child->attributes, i, cfx2_Attrib ).value, parent ) ) != 0 )
            return rc;
    }

    if ( cfx2_list_remove_item( &parent->children, sizeof( cfx2_Node* ), &child ) )
        return cfx2_ok;
    else
        return cfx2_node_not_found;
}
