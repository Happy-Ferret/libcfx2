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

#ifndef __libcfx2_io_h__
#define __libcfx2_io_h__

#include <confix2.h>
#include <stdio.h>

/* Buffer Input */
typedef struct
{
    cfx2_IInput IInput;

    char* data;
    size_t length, position;
}
cfx2_BufferInput;

/* File Output */
typedef struct
{
    cfx2_IOutput IOutput;

    FILE* file;
}
cfx2_FileOutput;

/* Buffer Input - factory functions */
int new_buffer_input_from_file( const char* file_name, cfx2_IInput** input );
int new_buffer_input_from_string( const char* string, cfx2_IInput** input );

/* File Output - factory functions */
int new_file_output( const char* file_name, cfx2_IOutput** output );

#endif
