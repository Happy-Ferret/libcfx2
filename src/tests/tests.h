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

#ifndef libcfx2_tests_h_included
#define libcfx2_tests_h_included

#include <confix2.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(DEBUG) || defined(_DEBUG)
#define tests_DEBUG
#endif

typedef int (*tests_TestFunc)();

typedef struct
{
    const char*     name;
    tests_TestFunc  func;
}
tests_Case;

typedef struct
{
    clock_t         time0;
}
tests_Perf;

#define tests_assert(assertion_) { if (!(assertion_)) tests_fail(("failed assertion '%s'", #assertion_)); }
#define tests_assert_2(assertion_, error_) { if (!(assertion_)) tests_fail(("failed assertion '%s': %s", #assertion_, error_)); }
#define tests_info(error_) { printf("#### %s:\t", tests_get_current_name()); printf error_; printf("\n"); }
#define tests_fail(error_) { printf("#### %s:\tTEST FAILED:\t", tests_get_current_name()); printf error_; printf("\n\n"); exit(-1); }

const char*     tests_get_current_name();
void            tests_memory_usage_check();
void            tests_print_node_recursive(cfx2_Node* node);

void            tests_perf_start(tests_Perf* perf);
void            tests_perf_end(tests_Perf* perf, const char* desc);

void tests_parse_error( cfx2_IInput* input_, int error_code, int line, const char* desc );

#endif
