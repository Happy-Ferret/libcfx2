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

#include "config.h"
#include "list.h"

#include <confix2.h>
#include <stdlib.h>
#include <string.h>

cfx2_List* new_list()
{
    cfx2_List* list = ( cfx2_List* )libcfx2_malloc( sizeof( cfx2_List ) );

    if ( !list )
        return 0;

    list->capacity = 0;
    list->items = 0;
    list->length = 0;

    return list;
}

libcfx2 void cfx2_release_list( cfx2_List* list )
{
    if ( !list )
        return;

    if ( list->items )
        libcfx2_free( list->items );

    libcfx2_free( list );
}

void list_add( cfx2_List* list, void* item )
{
    if ( !list )
        return;

    if ( list->capacity < list->length + 1 )
    {
        list->capacity = list->capacity * 2 + 1;
        list->items = ( void** ) libcfx2_realloc( list->items, list->capacity * sizeof( cfx2_Node* ) );
    }

    list->items[list->length++] = item;
}

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
