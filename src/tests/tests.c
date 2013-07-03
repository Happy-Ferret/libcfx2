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

#include "tests.h"

#include <string.h>

#if defined(_MSC_VER) && defined(_DEBUG)
#define crt_heap_check
#define crt_heap_break 0

#include <crtdbg.h>
#endif

int gen_huge(void);
int parseerror(void);
int parse_huge(void);
int queries1(void);
int unparent(void);

static const tests_Case testcases[] =
{
#define entry(name_) { #name_, &name_ }

    entry(gen_huge),
    entry(parseerror),
    entry(parse_huge),
    entry(queries1),
    entry(unparent),

#undef entry

    { NULL, NULL }
};

static const tests_Case* current;

static int memory_usage_check_enabled;

static void print_node( cfx2_Node* node, unsigned int depth )
{
    unsigned i;

    tests_assert(node->name != NULL)

    for ( i = 0; i < depth; i++ )
        printf( "  " );

    printf( "'%s'", node->name );

    if ( node->text )
        printf( ": '%s'", node->text );

    printf( " (" );

    for ( i = 0; i < cfx2_list_length( node->attributes ); i++ )
    {
        cfx2_Attrib* attrib = &cfx2_item( node->attributes, i, cfx2_Attrib );
        printf( "'%s': '%s'", attrib->name, attrib->value );

        if ( i + 1 < cfx2_list_length( node->attributes ) )
            printf( ", " );
    }

    printf( ")\n" );

    if ( cfx2_has_children( node ) )
        for ( i = 0; i < cfx2_list_length( node->children ); i++ )
            print_node( cfx2_item( node->children, i, cfx2_Node* ), depth + 1 );
}

const char* tests_get_current_name(void)
{
    return current->name;
}

void tests_memory_usage_check(void)
{
    if (memory_usage_check_enabled)
    {
        printf("####!!!! Please check application memory usage.\n");
        printf("####!!!! Press enter to continue.\n");
        getchar();
    }
}

int tests_parse_error( cfx2_RdOpt* rd_opt, int rc, int line, const char* desc )
{
    printf( "line %i: %s\n", line, desc );
    return 0;
}

void tests_perf_start(tests_Perf* perf)
{
    perf->time0 = clock();
}

void tests_perf_end(tests_Perf* perf, const char* desc)
{
    printf("#### %s:\ttime(%s): %i ms\n", current->name, desc, (int)((clock() - perf->time0) * 1000 / CLOCKS_PER_SEC));
}

void tests_print_node_recursive(cfx2_Node* node)
{
    print_node(node, 0);
}

static int perform_test(const tests_Case* testcase)
{
    current = testcase;

    fprintf(stderr, "####==== Performing testcase '%s'\n", current->name);

    return current->func();
}

int main( int argc, char** argv )
{
    const char* testcase_name;
    int i, rc;

#ifdef crt_heap_check
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);

#if crt_heap_break != 0
    _CrtSetBreakAlloc(crt_heap_break);
#endif
#endif

    testcase_name = "*";
    rc = 0;

    memory_usage_check_enabled = 0;

    for (i = 1; i < argc; i++)
    {
        if (strncmp(argv[i], "-m", 2) == 0)
            memory_usage_check_enabled = (argv[i][2] == '1');
        else
            testcase_name = argv[i];
    }

    fprintf(stderr, "#### Testing: %s\n", libcfx2_version_full);
    fprintf(stderr, "#### Running test case: %s\n", testcase_name);
    fprintf(stderr, "\n");

    if (strcmp(testcase_name, "*") != 0)
    {
        for (current = &testcases[0]; current->name != NULL; current++)
            if (strcmp(current->name, argv[1]) == 0)
                break;

        if (current->name == NULL)
        {
            fprintf(stderr, "ERROR: undefined testcase '%s'\n", argv[1]);
            return -1;
        }

        rc = perform_test(current);
    }
    else
    {
        for (current = &testcases[0]; rc == 0 && current->name != NULL; current++)
            rc = perform_test(current);
    }

    if (rc == 0)
        fprintf(stderr, "#### All tests completed successfully.\n");

    return rc;
}
