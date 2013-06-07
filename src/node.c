/*
    Copyright (c) 2009, 2010, 2011, 2013 Xeatheran Minexew

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
#include <stdlib.h>
#include <string.h>

/* warning C4127: conditional expression is constant */
/* warning C4293: '>>' : shift count negative or too big, undefined behavior */
/* (only triggers in unreachable code) */
#pragma warning( disable : 4127 )
#pragma warning( disable : 4293 )

/*
 *  http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 */
static size_t round_up_to_power_of_2( size_t v )
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    
    if (sizeof(v) >= 8)
        v |= v >> 32;
    
    v++;
    
    return v;
}

static void fixup_attrib( cfx2_Attrib* attr, char* min, char* max, ptrdiff_t buf_diff )
{
    if ( attr->name >= min && attr->name <= max )
        attr->name += buf_diff;

    if ( attr->value >= min && attr->value <= max )
        attr->value += buf_diff;
}

static void fixup_node( cfx2_Node* node, char* min, char* max, ptrdiff_t buf_diff )
{
    size_t i;

    if ( node->name >= min && node->name <= max )
        node->name += buf_diff;

    if ( node->text >= min && node->text <= max )
        node->text += buf_diff;

    for ( i = 0; i < cfx2_list_length( node->attributes ); i++ )
        fixup_attrib( &cfx2_item( node->attributes, i, cfx2_Attrib ), min, max, buf_diff );
}

static int release_node( cfx2_Node* node )
{
    unsigned i;

    if ( !node )
        return cfx2_param_invalid;

    for ( i = 0; i < cfx2_list_length( node->attributes ); i++ )
        cfx2_attrib_release( &cfx2_item( node->attributes, i, cfx2_Attrib ) );

    cfx2_list_release( &node->attributes );

    for ( i = 0; i < cfx2_list_length( node->children ); i++ )
        release_node( cfx2_item( node->children, i, cfx2_Node* ) );

    cfx2_list_release( &node->children );

    if ( node->text )
        cfx2_sfree( node->text );

    cfx2_sfree( node->name );
    
    libcfx2_free( node->shared );
    libcfx2_free( node );

    return cfx2_ok;
}

int cfx2_alloc_shared( char** ptr, cfx2_Node* node, size_t size )
{
    SharedHeader_t* sh;
    int rc;
    
    if ( node->shared == NULL )
        if ( ( rc = cfx2_preallocate_shared_buffer( node, size, 0 ) ) != 0 )
            return rc;
    
    sh = ( SharedHeader_t* )node->shared;
        
    if ( sh->used + size > sh->capacity )
        return cfx2_alloc_error;

    *ptr = node->shared + sizeof( SharedHeader_t ) + sh->used;
    sh->used += size;
    return 0;
}

int cfx2_salloc( char** ptr, cfx2_Node* parent, cfx2_Node* node, size_t size,
                const char* initdata, int flags )
{
    char* chunk;
    
    chunk = NULL;
    size += sizeof( s_nref_t );
    
    if ( flags & cfx2_use_shared_buffer )
    {
        chunk = NULL;
        
        if ( parent != NULL )
            cfx2_alloc_shared( &chunk, parent, size );
        else
            cfx2_alloc_shared( &chunk, node, size );
        
        if ( chunk != NULL )
            *( s_nref_t* )chunk = 0;
    }
    
    if ( chunk == NULL )
    {
        chunk = ( char* ) libcfx2_malloc( size );
        
        if ( chunk == NULL )
            return cfx2_alloc_error;
        
        *( s_nref_t* )chunk = 1;
    }
    
    chunk += sizeof( s_nref_t );
    size -= sizeof( s_nref_t );
    
    if ( initdata != NULL )
        memcpy( chunk, initdata, size );
    
    *ptr = chunk;
    return 0;
}

void cfx2_sfree( char* chunk )
{
    if ( chunk == NULL )
        return;
    
    chunk -= sizeof( s_nref_t );
    
    if ( *( s_nref_t* )chunk == 0 )
        return;
        
    if ( --( *( s_nref_t* )chunk ) == 0 )
        libcfx2_free( chunk );
}

libcfx2 int cfx2_create_node( cfx2_Node** node_ptr )
{
    cfx2_Node* node;

    node = ( cfx2_Node* )libcfx2_malloc( sizeof( cfx2_Node ) );

    if ( !node )
        return cfx2_alloc_error;

    node->name = NULL;
    node->text = NULL;

    cfx2_list_init( &node->children );
    cfx2_list_init( &node->attributes );

    node->shared = NULL;
    
    *node_ptr = node;
    return cfx2_ok;
}

libcfx2 cfx2_Node* cfx2_new_node( const char* name )
{
    cfx2_Node* node;

    if ( cfx2_create_node( &node ) != 0 )
        return NULL;
    
    if ( cfx2_rename_node( node, name ) != 0 )
        cfx2_release_node( &node );

    return node;
}

libcfx2 void cfx2_release_node( cfx2_Node** node_ptr )
{
    release_node( *node_ptr );
    *node_ptr = 0;
}

libcfx2 int cfx2_preallocate_shared_buffer( cfx2_Node* node, size_t size, int flags )
{
    SharedHeader_t* sh;
    char* new_shared;
    size_t capacity;

    if ( node->shared != NULL )
    {
        sh = ( SharedHeader_t* )node->shared;

        if ( size < sh->capacity )
            return cfx2_param_invalid;
    }

    if ( size < 16 )
        capacity = 16;
    else
        capacity = round_up_to_power_of_2( size );
        
    new_shared = ( char* )libcfx2_malloc( sizeof( SharedHeader_t ) + capacity );
        
    if ( new_shared == NULL )
        return cfx2_alloc_error;
        
    if ( node->shared != NULL )
    {
        ptrdiff_t buf_diff;

        memcpy( new_shared, node->shared, sizeof( SharedHeader_t ) + sh->used );

        buf_diff = new_shared - node->shared;
        fixup_node( node, node->shared, node->shared + sh->capacity, buf_diff );

        libcfx2_free( node->shared );
    }

    node->shared = new_shared;

    sh = ( SharedHeader_t* )new_shared;
    sh->capacity = capacity;
    sh->used = 0;

    return 0;
}

libcfx2 int cfx2_rename_node( cfx2_Node* node, const char* name )
{
    if ( node->name != NULL )
    {
        cfx2_sfree( node->name );
        node->name = NULL;
    }

    if ( name != NULL )
        return cfx2_salloc( &node->name, NULL, node, strlen( name ) + 1, name, cfx2_use_shared_buffer );

    return cfx2_ok;
}

libcfx2 int cfx2_set_node_text( cfx2_Node* node, const char* text )
{
    if ( node->text != NULL )
    {
        cfx2_sfree( node->text );
        node->text = NULL;
    }

    if ( text != NULL )
        return cfx2_salloc( &node->text, NULL, node, strlen( text ) + 1, text, cfx2_use_shared_buffer );

    return cfx2_ok;
}

libcfx2 cfx2_Node* cfx2_clone_node( cfx2_Node* node, int flags )
{
    cfx2_Node* clone;
    size_t i;

    if ( !node )
        return 0;

    if ( cfx2_create_node( &clone ) != 0 )
        return 0;

    /* TODO: Use shared buffer */

    if ( cfx2_rename_node( clone, node->name ) != 0
            || cfx2_set_node_text( clone, node->text ) != 0 )
    {
        cfx2_release_node( &clone );
        return NULL;
    }

    for ( i = 0; i < cfx2_list_length( node->attributes ); i++ )
    {
        cfx2_Attrib* attrib;

        attrib = cfx2_item( node->attributes, i, cfx2_Attrib* );

        cfx2_set_node_attrib( clone, attrib->name, attrib->value );
    }

    if ( flags & cfx2_clone_recursive )
    {
        for ( i = 0; i < cfx2_list_length( node->children ); i++ )
            cfx2_add_child( clone, cfx2_clone_node( cfx2_item( node->children, i, cfx2_Node* ), flags ) );
    }

    return clone;
}

#if 0
libcfx2 int cfx2_merge_nodes( cfx2_Node* left, cfx2_Node* right, cfx2_Node** output_ptr, int flags )
{
    cfx2_Node* merged;
    int rc;
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
            rc = cfx2_create_node( left->name, &merged );
        else
            rc = cfx2_create_node( right->name, &merged );

        if ( rc )
            return rc;

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
#endif
