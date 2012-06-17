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

#include "config.h"
#include "io.h"

#include <confix2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/*  Buffer Input                                                              */
/* -------------------------------------------------------------------------- */

#define input ( ( cfx2_BufferInput* )input_ )

static void BufferInput_finished( cfx2_IInput* input_ )
{
    libcfx2_free( input->data );
    libcfx2_free( input );
}

static void BufferInput_handle_error( cfx2_IInput* input_, int error_code, int line, const char* desc )
{
    printf( "libcfx2 Error %i: '%s'\n", error_code, desc );
    printf( "  This error occured when parsing a cfx2 document.\n" );

    if ( line >= 0 )
        printf( "    at line: %i\n", line );

    printf( "    library: " libcfx2_version_full "\n" );
    printf( "  Document reading failed.\n\n" );
}

static int BufferInput_is_EOF( cfx2_IInput* input_ )
{
    return !input || input->position >= input->length;
}

static size_t BufferInput_read( cfx2_IInput* input_, void* output, size_t length )
{
    if ( !input || !output )
        return 0;

    if ( input->position + length <= input->length )
    {
        if ( output )
        {
            if ( length == 1 )
                *( char* )output = input->data[input->position];
            else
                memcpy( output, input->data + input->position, length );
        }

        input->position += length;
        return length;
    }
    else
    {
        if ( output )
            memcpy( output, input->data + input->position, input->length - input->position );

        input->position = input->length;
        return input->length - input->position;
    }
}

#undef input

int new_buffer_input_from_file( const char* file_name, cfx2_IInput** input_ptr )
{
    cfx2_BufferInput* input;
    FILE* file;

    if ( !input_ptr )
        return cfx2_param_invalid;

    file = fopen( file_name, "rb" );

    if ( !file )
        return cfx2_cant_open_file;

    input = ( cfx2_BufferInput* )libcfx2_malloc( sizeof( cfx2_BufferInput ) );

    if ( !input )
    {
        fclose( file );
        return cfx2_alloc_error;
    }

    fseek( file, 0, SEEK_END );
    input->length = ftell( file );
    fseek( file, 0, SEEK_SET );

    input->data = ( char* )libcfx2_malloc( input->length + 1 );

    if ( !input->data )
    {
        fclose( file );
        libcfx2_free( input );
        return cfx2_alloc_error;
    }

    input->length = fread( input->data, 1, input->length, file );
    input->data[input->length] = 0;
    fclose( file );

    input->IInput.finished = &BufferInput_finished;
    input->IInput.handle_error = &BufferInput_handle_error;
    input->IInput.is_EOF = &BufferInput_is_EOF;
    input->IInput.read = &BufferInput_read;
    input->position = 0;

    *input_ptr = ( cfx2_IInput* )input;
    return cfx2_ok;
}

int new_buffer_input_from_string( const char* string, cfx2_IInput** input_ptr )
{
    cfx2_BufferInput* input;

    if ( !input_ptr )
        return cfx2_param_invalid;

    input = ( cfx2_BufferInput* )libcfx2_malloc( sizeof( cfx2_BufferInput ) );

    if ( !input )
        return cfx2_alloc_error;

    input->IInput.finished = &BufferInput_finished;
    input->IInput.handle_error = &BufferInput_handle_error;
    input->IInput.is_EOF = &BufferInput_is_EOF;
    input->IInput.read = &BufferInput_read;
    input->length = strlen( string );
    input->position = 0;

    input->data = ( char* )libcfx2_malloc( input->length + 1 );

    if ( !input->data )
    {
        libcfx2_free( input );
        return cfx2_alloc_error;
    }

    memcpy( input->data, string, input->length + 1 );

    *input_ptr = ( cfx2_IInput* )input;
    return cfx2_ok;
}

/* -------------------------------------------------------------------------- */
/*  File Output                                                               */
/* -------------------------------------------------------------------------- */

#define output ( ( cfx2_FileOutput* )output_ )

static void FileOutput_finished( cfx2_IOutput* output_ )
{
    fclose( output->file );
    libcfx2_free( output );
}

static void FileOutput_handle_error( cfx2_IOutput* output_, int error_code, const char* desc )
{
    printf( "libcfx2 Error %i: '%s'\n", error_code, desc );
    printf( "  This error occured during document serialization.\n" );
    printf( "    library: " libcfx2_version_full "\n" );
    printf( "  Document serialization will be interrupted.\n\n" );
}

static size_t FileOutput_write( cfx2_IOutput* output_, const void* input, size_t length )
{
    return fwrite( input, 1, length, output->file );
}

#undef output

int new_file_output( const char* file_name, cfx2_IOutput** output_ptr )
{
    cfx2_FileOutput* output;

    output = ( cfx2_FileOutput* )libcfx2_malloc( sizeof( cfx2_FileOutput ) );

    if ( !output )
        return cfx2_alloc_error;

    output->IOutput.finished = &FileOutput_finished;
    output->IOutput.handle_error = &FileOutput_handle_error;
    output->IOutput.write = &FileOutput_write;

    output->file = fopen( file_name, "wt" );

    if ( !output->file )
    {
        libcfx2_free( output );
        return cfx2_cant_open_file;
    }

    *output_ptr = ( cfx2_IOutput* )output;
    return cfx2_ok;
}

#undef output

/* -------------------------------------------------------------------------- */
/*  Buffer Output                                                             */
/* -------------------------------------------------------------------------- */

#define output ( ( cfx2_BufferOutput* )output_ )

static void BufferOutput_finished( cfx2_IOutput* output_ )
{
    libcfx2_free( output );
}

static size_t BufferOutput_write( cfx2_IOutput* output_, const void* input, size_t length )
{
    if ( *output->used + length > *output->capacity )
    {
        *output->capacity += length + 64;
        *output->text = (char *) realloc( *output->text, *output->capacity );
    }

    memcpy( *output->text + *output->used, input, length );
    *output->used += length;

    return length;
}

#undef output

int libcfx2_new_buffer_output( cfx2_IOutput** output_ptr, char** text, size_t* capacity, size_t* used )
{
    cfx2_BufferOutput* output;

    output = ( cfx2_BufferOutput* )libcfx2_malloc( sizeof( cfx2_BufferOutput ) );

    if ( !output )
        return cfx2_alloc_error;

    output->IOutput.finished =      &BufferOutput_finished;
    output->IOutput.handle_error =  &FileOutput_handle_error;
    output->IOutput.write =         &BufferOutput_write;

    output->text = text;
    output->capacity = capacity;
    output->used = used;

    *output_ptr = &output->IOutput;
    return cfx2_ok;
}

#undef output