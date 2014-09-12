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

#include "config.h"
#include "io.h"

#include <confix2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/*  Buffer Input                                                              */
/* -------------------------------------------------------------------------- */

static int BufferInput_on_error( cfx2_RdOpt* rd_opt, int error_code, int line, const char* desc )
{
    printf( "libcfx2 Error %i: '%s'\n", error_code, desc );
    printf( "  This error occured when parsing a cfx2 document.\n" );
    
    if ( rd_opt->client_priv != NULL )
        printf( "    in file: %s\n", ( const char* )rd_opt->client_priv );
    
    if ( line >= 0 )
        printf( "    at line: %i\n", line );
    
    printf( "    library: " libcfx2_version_full "\n" );
    printf( "  Document reading failed.\n\n" );
    
    return 0;
}

#undef input

int cfx2_buffer_input_from_file( cfx2_RdOpt* rd_opt, const char* filename )
{
    FILE* file;

    file = fopen( filename, "rb" );

    if ( file == NULL )
        return cfx2_cant_open_file;

    fseek( file, 0, SEEK_END );
    rd_opt->document_len = ftell( file );
    fseek( file, 0, SEEK_SET );

    rd_opt->document = ( char* )libcfx2_malloc( rd_opt->document_len + 1 );

    if ( !rd_opt->document )
    {
        fclose( file );
        return cfx2_alloc_error;
    }

    rd_opt->document_len = fread( rd_opt->document, 1, rd_opt->document_len, file );
    rd_opt->document[rd_opt->document_len] = 0;
    fclose( file );

    rd_opt->client_priv = NULL;
    rd_opt->on_error = BufferInput_on_error;

    return cfx2_ok;
}

int cfx2_buffer_input_from_string( cfx2_RdOpt* rd_opt, const char* string )
{
    rd_opt->document_len = strlen( string );
    rd_opt->document = ( char* )libcfx2_malloc( rd_opt->document_len + 1 );

    if ( !rd_opt->document )
        return cfx2_alloc_error;

    memcpy( rd_opt->document, string, rd_opt->document_len + 1 );

    rd_opt->on_error = BufferInput_on_error;

    return cfx2_ok;
}

/* -------------------------------------------------------------------------- */
/*  File Output                                                               */
/* -------------------------------------------------------------------------- */

#define output ( ( cfx2_FileStreamPriv* )wr_opt->stream_priv )

static int FileStream_on_error( cfx2_WrOpt* wr_opt, int rc, int line, const char* desc )
{
    printf( "libcfx2 Error %i: '%s'\n", rc, desc );
    printf( "  This error occured during document serialization.\n" );
    printf( "    library: " libcfx2_version_full "\n" );
    printf( "  Document serialization will be interrupted.\n\n" );
    
    return 0;
}

static void FileStream_stream_close( cfx2_WrOpt* wr_opt )
{
    fclose( output->file );
    libcfx2_free( output );
}

static size_t FileStream_stream_write( cfx2_WrOpt* wr_opt, const char* input, size_t length )
{
    return fwrite( input, 1, length, output->file );
}

#undef output

int cfx2_file_stream( cfx2_WrOpt* wr_opt, const char* filename )
{
    cfx2_FileStreamPriv* output;

    output = ( cfx2_FileStreamPriv* )libcfx2_malloc( sizeof( cfx2_FileStreamPriv ) );

    output->file = fopen( filename, "wt" );

    if ( !output->file )
    {
        libcfx2_free( output );
        return cfx2_cant_open_file;
    }

    wr_opt->client_priv = NULL;
    wr_opt->on_error = FileStream_on_error;
    wr_opt->stream_priv = output;
    wr_opt->stream_close = FileStream_stream_close;
    wr_opt->stream_write = FileStream_stream_write;
    
    return cfx2_ok;
}

#undef output

/* -------------------------------------------------------------------------- */
/*  Memory Output                                                             */
/* -------------------------------------------------------------------------- */

#define output ( ( cfx2_MemoryStreamPriv* )wr_opt->stream_priv )

static void MemoryStream_stream_close( cfx2_WrOpt* wr_opt )
{
    libcfx2_free( output );
}

static size_t MemoryStream_stream_write( cfx2_WrOpt* wr_opt, const char* input, size_t length )
{
    if ( *output->used + length > *output->capacity )
    {
        *output->capacity += length + 64;
        *output->text = ( char* )realloc( *output->text, *output->capacity );
    }

    memcpy( *output->text + *output->used, input, length );
    *output->used += length;

    return length;
}

#undef output

int cfx2_memory_stream( cfx2_WrOpt* wr_opt, char** text, size_t* capacity, size_t* used )
{
    cfx2_MemoryStreamPriv* output;

    output = ( cfx2_MemoryStreamPriv* )libcfx2_malloc( sizeof( cfx2_MemoryStreamPriv ) );

    if ( !output )
        return cfx2_alloc_error;

    output->text = text;
    output->capacity = capacity;
    output->used = used;

    wr_opt->client_priv = NULL;
    wr_opt->on_error = FileStream_on_error;
    wr_opt->stream_priv = output;
    wr_opt->stream_write = MemoryStream_stream_write;
    wr_opt->stream_close = MemoryStream_stream_close;

    return cfx2_ok;
}

#undef output
