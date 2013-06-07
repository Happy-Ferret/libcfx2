/*
    Copyright (c) 2011, 2013 Xeatheran Minexew

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

#ifndef libcfx2_config_h_included
#define libcfx2_config_h_included

/*  Dynamic Allocation Functions  */
#define libcfx2_malloc      malloc
#define libcfx2_realloc     realloc
#define libcfx2_free        free

/*  Parser Utility Buffers  */
/*  default sizes (in bytes incl. overhead) are: 32 64 128 256 512 1024 */
#define PARSER_MIN_BUF      32
#define PARSER_NUM_BUFS     6

#endif
