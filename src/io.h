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

#ifndef libcfx2_io_h
#define libcfx2_io_h

#include <confix2.h>
#include <stdio.h>

/* File Output */
typedef struct
{
    FILE* file;
    const char* filename;
}
cfx2_FileStreamPriv;

/* Memory Output */
typedef struct
{
    char** text;
    size_t* capacity;
    size_t* used;
}
cfx2_MemoryStreamPriv;

int cfx2_buffer_input_from_file( cfx2_RdOpt* rd_opt, const char* filename );
int cfx2_buffer_input_from_string( cfx2_RdOpt* rd_opt, const char* string );

int cfx2_file_stream( cfx2_WrOpt* rd_opt, const char* filename );
int cfx2_memory_stream( cfx2_WrOpt* rd_opt, char** text, size_t* capacity, size_t* used );

#endif
