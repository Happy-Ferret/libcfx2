/*
    Copyright (c) 2010, 2011 Xeatheran Minexew

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

#include <confix2.h>

static const char* error_descs[] =
{
    "ok",
    "unexpected end of file",
    "document syntax error",
    "operation interrupted",
    "invalid function argument",
    "unable to open the specified file",
    "memory allocation error",
    "unable to find the specified node attribute",
    /* 0x08 cfx2_missing_node_name */   "node name empty or not specified",
    /* 0x09 cfx2_node_not_found */      "node not found",
};

libcfx2 const char* cfx2_get_error_desc( int error_code )
{
    if ( error_code >= cfx2_ok && error_code < cfx2_max_err )
        return error_descs[error_code];
    else
        return "unknown error code";
}
