/*
    Copyright (c) 2013 Xeatheran Minexew

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

#ifndef libcfx2_node_h
#define libcfx2_node_h

#define cfx2_use_shared_buffer  1

typedef unsigned int s_nref_t;

typedef struct SharedHeader_t SharedHeader_t;

struct SharedHeader_t
{
    size_t capacity, used;
};

int cfx2_salloc( char** ptr, cfx2_Node* parent, cfx2_Node* node, size_t size,
        const char* initdata, int flags );
void cfx2_sfree( char* chunk );

int cfx2_alloc_shared( char** ptr, cfx2_Node* node, size_t size );

#endif
