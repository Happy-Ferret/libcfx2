/*
    Copyright (c) 2009, 2010, 2011 Xeatheran Minexew

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

#include <confix2.h>
#include <stdlib.h>
#include <string.h>

libcfx2 int cfx2_create_node( const char* name, cfx2_Node** node_ptr )
{
    cfx2_Node* node;

    if ( !node_ptr )
        return cfx2_param_invalid;

    node = ( cfx2_Node* )libcfx2_malloc( sizeof( cfx2_Node ) );

    if ( !node )
        return cfx2_alloc_error;

    if ( name )
    {
        node->name = ( char* )libcfx2_malloc( strlen( name ) + 1 );

        if ( !node->name )
        {
            libcfx2_free( node );
            return cfx2_alloc_error;
        }

        strcpy( node->name, name );
    }
    else
        node->name = 0;

    node->text = 0;

    node->children = 0;
    node->attributes = new_list();

    if ( !node->attributes )
    {
        cfx2_release_node( node );
        return cfx2_alloc_error;
    }

    *node_ptr = node;
    return cfx2_ok;
}

libcfx2 cfx2_Node* cfx2_new_node( const char* name )
{
    cfx2_Node* node;

    if ( cfx2_create_node( name, &node ) == cfx2_ok )
        return node;
    else
        return 0;
}

libcfx2 int cfx2_rename_node( cfx2_Node* node, const char* name )
{
    if ( !node )
        return cfx2_param_invalid;

    if ( node->name )
    {
        libcfx2_free( node->name );
        node->name = 0;
    }

    if ( name )
    {
        node->name = ( char* )libcfx2_malloc( strlen( name ) + 1 );

        if ( !node->name )
            return cfx2_alloc_error;

        strcpy( node->name, name );
    }

    return cfx2_ok;
}

libcfx2 int cfx2_set_node_text( cfx2_Node* node, const char* text )
{
    if ( !node )
        return cfx2_param_invalid;

    if ( node->text )
    {
        libcfx2_free( node->text );
        node->text = 0;
    }

    if ( text )
    {
        node->text = ( char* )libcfx2_malloc( strlen( text ) + 1 );

        if ( !node->text )
            return cfx2_alloc_error;

        strcpy( node->text, text );
    }

    return cfx2_ok;
}

libcfx2 int cfx2_release_node( cfx2_Node* node )
{
    unsigned i;

    if ( !node )
        return cfx2_param_invalid;

    if ( node->attributes )
    {
        for ( i = 0; i < node->attributes->length; i++ )
            cfx2_delete_attrib( ( cfx2_Attrib* )node->attributes->items[i] );

        cfx2_release_list( node->attributes );
    }

    if ( node->children )
    {
        for ( i = 0; i < node->children->length; i++ )
            cfx2_release_node( ( cfx2_Node* )node->children->items[i] );

        cfx2_release_list( node->children );
    }

    if ( node->text )
        libcfx2_free( node->text );

    libcfx2_free( node->name );
    libcfx2_free( node );

    return cfx2_ok;
}

libcfx2 int cfx2_release_node_2( cfx2_Node** node_ptr )
{
    int error;

    if ( !node_ptr )
        return cfx2_param_invalid;

    error = cfx2_release_node( *node_ptr );
    *node_ptr = 0;

    return error;
}

libcfx2 cfx2_Node* cfx2_clone_node( cfx2_Node* node )
{
    cfx2_Node* clone;
    size_t i;

    if ( !node )
        return 0;

    if ( cfx2_create_node( node->name, &clone ) != cfx2_ok )
        return 0;

    if ( cfx2_set_node_text( clone, node->text ) != cfx2_ok )
    {
        cfx2_release_node_2( &clone );
        return 0;
    }

    for ( i = 0; i < cfx2_list_length( node->attributes ); i++ )
    {
        cfx2_Attrib* attrib;

        attrib = cfx2_item( node->attributes, i, cfx2_Attrib* );

        cfx2_set_node_attrib( clone, attrib->name, attrib->value );
    }

    for ( i = 0; i < cfx2_list_length( node->children ); i++ )
        cfx2_add_child( clone, cfx2_clone_node( cfx2_item( node->children, i, cfx2_Node* ) ) );

    return clone;
}

libcfx2 cfx2_Node* cfx2_join_nodes( cfx2_Node* left, cfx2_Node* right )
{
    cfx2_Node* joint;
    size_t i;

    if ( !left || !right )
        return 0;

    if ( cfx2_create_node( left->name, &joint ) != cfx2_ok )
        return 0;

    if ( cfx2_set_node_text( joint, left->text ) != cfx2_ok )
    {
        cfx2_release_node_2( &joint );
        return 0;
    }

    for ( i = 0; i < cfx2_list_length( left->attributes ); i++ )
    {
        cfx2_Attrib* attrib;

        attrib = cfx2_item( left->attributes, i, cfx2_Attrib* );

        cfx2_set_node_attrib( joint, attrib->name, attrib->value );
    }

    for ( i = 0; i < cfx2_list_length( right->attributes ); i++ )
    {
        cfx2_Attrib* attrib;

        attrib = cfx2_item( right->attributes, i, cfx2_Attrib* );

        /* Check for duplicate attributes */
        if ( cfx2_find_attrib( joint, attrib->name ) != 0 )
            cfx2_set_node_attrib( joint, attrib->name, attrib->value );
    }

    for ( i = 0; i < cfx2_list_length( left->children ); i++ )
        cfx2_add_child( joint, cfx2_clone_node( cfx2_item( left->children, i, cfx2_Node* ) ) );

    for ( i = 0; i < cfx2_list_length( right->children ); i++ )
        cfx2_add_child( joint, cfx2_clone_node( cfx2_item( right->children, i, cfx2_Node* ) ) );

    return joint;
}

libcfx2 cfx2_Node* cfx2_merge_nodes( cfx2_Node* left, cfx2_Node* right )
{
    if ( !left || !right || left == right )
        return 0;

    while ( cfx2_list_length( right->attributes ) > 0 )
    {
        cfx2_Attrib* attrib;

        attrib = cfx2_item( right->attributes, 0, cfx2_Attrib* );

        /* Check for duplicate attributes */
        if ( cfx2_find_attrib( left, attrib->name ) != 0 )
            list_add( left->attributes, attrib );
        else
            cfx2_delete_attrib( attrib );

        list_remove( right->attributes, 0 );
    }

    while ( cfx2_has_children( right ) )
    {
        cfx2_add_child( left, cfx2_item( right->children, 0, cfx2_Node* ) );
        list_remove( right->children, 0 );
    }

    cfx2_release_node_2( &right );
    return left;
}

libcfx2 int cfx2_merge_nodes_2( cfx2_Node* left, cfx2_Node* right, cfx2_Node** output_ptr, int flags )
{
    cfx2_Node* merged;
    int error;
    size_t i;

    if ( left == right )
        return cfx2_param_invalid;

    if ( !output_ptr )
    {
        /* No output pointer */
        /* Only do any necessary releases */

        if ( flags & cfx2_release_left )
            cfx2_release_node( left );

        if ( flags & cfx2_release_right )
            cfx2_release_node( right );

        return cfx2_ok;
    }

    if ( flags & cfx2_release_left )
    {
        /* Left is marked for release */
        /* We'll use it as the resulting node */

        merged = left;

        /* Any name changes necessary ? */
        if ( flags & cfx2_name_from_right )
            cfx2_rename_node( merged, right->name );

        if ( flags & cfx2_text_from_right )
            cfx2_set_node_text( merged, right->text );
    }
    else if ( flags & cfx2_release_right )
    {
        /* Right is marked for release (left is not) */
        /* We'll use it as the resulting node */

        merged = right;

        /* Any name changes necessary ? */
        if ( flags & cfx2_name_from_left )
            cfx2_rename_node( merged, left->name );

        if ( flags & cfx2_text_from_left )
            cfx2_set_node_text( merged, left->text );
    }
    else
    {
        /* Both nodes need to be preserved */
        /* Use left's name unless required to do otherwise */

        if ( !( flags & cfx2_name_from_right ) )
            error = cfx2_create_node( left->name, &merged );
        else
            error = cfx2_create_node( right->name, &merged );

        if ( error )
            return error;

        if ( !( flags & cfx2_text_from_right ) )
            cfx2_set_node_text( merged, left->text );
        else
            cfx2_set_node_text( merged, right->text );
    }

    if ( ( flags & cfx2_release_left ) && ( flags & cfx2_release_right ) )
    {
        /* Both nodes are marked for release */
        /* Merge to the left */

        /* merged == left */
        /* move from right to left/merged */

        while ( cfx2_list_length( right->attributes ) > 0 )
        {
            cfx2_Attrib* attrib;

            attrib = cfx2_item( right->attributes, 0, cfx2_Attrib* );

            /* Check for duplicate attributes */
            if ( !cfx2_find_attrib( merged, attrib->name ) )
                list_add( merged->attributes, attrib );
            else if ( flags & cfx2_prefer_attribs_from_right )
            {
                cfx2_set_node_attrib( merged, attrib->name, attrib->value );
                cfx2_delete_attrib( attrib );
            }
            else
                cfx2_delete_attrib( attrib );

            list_remove( right->attributes, 0 );
        }

        if ( flags & cfx2_right_children_first )
        {
            i = 0;

            while ( cfx2_has_children( right ) )
            {
                cfx2_insert_child( merged, i++, cfx2_item( right->children, 0, cfx2_Node* ) );
                list_remove( right->children, 0 );
            }
        }
        else
            while ( cfx2_has_children( right ) )
            {
                cfx2_add_child( merged, cfx2_item( right->children, 0, cfx2_Node* ) );
                list_remove( right->children, 0 );
            }
    }
    else if ( flags & cfx2_release_left )
    {
        /* Only left is marked for release */
        /* Copy from right */

        /* merged == left */
        /* copy from right to merged */

        for ( i = 0; i < cfx2_list_length( right->attributes ); i++ )
        {
            cfx2_Attrib* attrib;

            attrib = cfx2_item( right->attributes, i, cfx2_Attrib* );

            /* Check for duplicate attributes */
            if ( ( flags & cfx2_prefer_attribs_from_right ) || !cfx2_find_attrib( merged, attrib->name ) )
                cfx2_set_node_attrib( merged, attrib->name, attrib->value );
        }

        if ( flags & cfx2_right_children_first )
        {
            for ( i = 0; i < cfx2_list_length( right->children ); i++ )
                cfx2_insert_child( merged, i, cfx2_clone_node( cfx2_item( right->children, 0, cfx2_Node* ) ) );
        }
        else
            for ( i = 0; i < cfx2_list_length( right->children ); i++ )
                cfx2_add_child( merged, cfx2_clone_node( cfx2_item( right->children, 0, cfx2_Node* ) ) );
    }
    else if ( flags & cfx2_release_right )
    {
        /* Only right is marked for release */
        /* Copy from left */

        /* merged == right */
        /* copy from left to merged */

        for ( i = 0; i < cfx2_list_length( left->attributes ); i++ )
        {
            cfx2_Attrib* attrib;

            attrib = cfx2_item( left->attributes, i, cfx2_Attrib* );

            /* Check for duplicate attributes */
            if ( !( flags & cfx2_prefer_attribs_from_right ) || !cfx2_find_attrib( merged, attrib->name ) )
                cfx2_set_node_attrib( merged, attrib->name, attrib->value );
        }

        if ( flags & cfx2_left_children_first )
        {
            for ( i = 0; i < cfx2_list_length( left->children ); i++ )
                cfx2_insert_child( merged, i, cfx2_clone_node( cfx2_item( left->children, 0, cfx2_Node* ) ) );
        }
        else
            for ( i = 0; i < cfx2_list_length( left->children ); i++ )
                cfx2_add_child( merged, cfx2_clone_node( cfx2_item( left->children, 0, cfx2_Node* ) ) );
    }
    else
    {
        /* None of the nodes can be released */
        /* Everything has to be copied */

        if ( !( flags & cfx2_prefer_attribs_from_right ) )
        {
            for ( i = 0; i < cfx2_list_length( left->attributes ); i++ )
            {
                cfx2_Attrib* attrib;

                attrib = cfx2_item( left->attributes, i, cfx2_Attrib* );

                cfx2_set_node_attrib( merged, attrib->name, attrib->value );
            }

            for ( i = 0; i < cfx2_list_length( right->attributes ); i++ )
            {
                cfx2_Attrib* attrib;

                attrib = cfx2_item( right->attributes, i, cfx2_Attrib* );

                /* Check for duplicate attributes */
                if ( !cfx2_find_attrib( merged, attrib->name ) )
                    cfx2_set_node_attrib( merged, attrib->name, attrib->value );
            }
        }
        else
        {
            for ( i = 0; i < cfx2_list_length( right->attributes ); i++ )
            {
                cfx2_Attrib* attrib;

                attrib = cfx2_item( right->attributes, i, cfx2_Attrib* );

                cfx2_set_node_attrib( merged, attrib->name, attrib->value );
            }

            for ( i = 0; i < cfx2_list_length( left->attributes ); i++ )
            {
                cfx2_Attrib* attrib;

                attrib = cfx2_item( left->attributes, i, cfx2_Attrib* );

                /* Check for duplicate attributes */
                if ( !cfx2_find_attrib( merged, attrib->name ) )
                    cfx2_set_node_attrib( merged, attrib->name, attrib->value );
            }
        }

        if ( !( flags & cfx2_right_children_first ) )
        {
            for ( i = 0; i < cfx2_list_length( left->children ); i++ )
                cfx2_add_child( merged, cfx2_clone_node( cfx2_item( left->children, i, cfx2_Node* ) ) );

            for ( i = 0; i < cfx2_list_length( right->children ); i++ )
                cfx2_add_child( merged, cfx2_clone_node( cfx2_item( right->children, i, cfx2_Node* ) ) );
        }
        else
        {
            for ( i = 0; i < cfx2_list_length( right->children ); i++ )
                cfx2_add_child( merged, cfx2_clone_node( cfx2_item( right->children, i, cfx2_Node* ) ) );

            for ( i = 0; i < cfx2_list_length( left->children ); i++ )
                cfx2_add_child( merged, cfx2_clone_node( cfx2_item( left->children, i, cfx2_Node* ) ) );
        }
    }

    *output_ptr = merged;
    return cfx2_ok;
}
