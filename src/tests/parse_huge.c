
#include "tests.h"

#include "huge.h"

int parse_huge()
{
    cfx2_Node* doc;
    tests_Perf perf;

    int rc;

    tests_perf_start(&perf);

    rc = cfx2_read_file(huge_filename, &doc);

    if (rc != cfx2_ok)
        tests_fail(("failed to load '%s': %i", huge_filename, rc))

    tests_assert(cfx2_list_length(doc->children) == huge_node_count)

    tests_perf_end(&perf, "load document");

    tests_memory_usage_check();

    cfx2_release_node_2(&doc);

    return 0;
}
