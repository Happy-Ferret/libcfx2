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

#include "config.h"
#include "list.h"

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

int list_init( cfx2_List* list )
{
    list->items = NULL;
    list->length = 0;

    return 0;
}

void list_release( cfx2_List* list )
{
    libcfx2_free( list->items );
}

cfx2_uint8_t* list_add_item( cfx2_List* list, itemsize_t itemsize )
{
    size_t capacity, new_capacity;
    cfx2_uint8_t* ret;
    
    capacity = round_up_to_power_of_2( list->length * itemsize );
    new_capacity = round_up_to_power_of_2( ( list->length + 1 ) * itemsize );
    
    if ( new_capacity > capacity )
        list->items = ( cfx2_uint8_t* )realloc( list->items, new_capacity );

    ret = list->items + list->length * itemsize;
    list->length++;
    return ret;
}

/*
void cfx2_list_insert( cfx2_List* list, size_t index, void* item )
{
    if ( !list )
        return;

    if ( list->capacity < list->length + 1 )
    {
        list->capacity = list->capacity * 2 + 1;
        list->items = ( void** ) libcfx2_realloc( list->items, list->capacity * sizeof( cfx2_Node* ) );
    }

    if ( index > list->length )
        index = list->length;

    memmove( list->items + index + 1, list->items + index, ( list->length - index ) * sizeof( void* ) );
    list->items[index] = item;
}

int list_remove( cfx2_List* list, unsigned index )
{
    if ( !list )
        return 0;

    if ( index < list->length )
    {
        memmove( list->items + index, list->items + index + 1, ( list->length - index - 1 ) * sizeof( void* ) );
        list->length--;

        return 1;
    }

    return 0;
}

int list_remove_item( cfx2_List* list, void* entry )
{
    unsigned p;

    if ( !list )
        return 0;

    for ( p = 0; p < list->length; p++ )
        if ( list->items[p] == entry )
            break;

    return list_remove( list, p );
}
*/
