
#include "tests.h"

#include <string.h>

#define unparent_filename   "unparent.cfx2"

int unparent(void)
{
    cfx2_Node* doc;
    cfx2_Node* child;
    const char* value;

    int rc;

    rc = cfx2_read_file(&doc, unparent_filename, NULL);

    if (rc != cfx2_ok)
        tests_fail(("failed to load '%s': %s", unparent_filename, cfx2_get_error_desc(rc)))

    tests_assert(cfx2_list_length(doc->children) == 1)

    child = cfx2_item(doc->children, 0, cfx2_Node*);

    rc = cfx2_remove_child(doc, child);

    if (rc != cfx2_ok)
        tests_fail(("failed to remove child node: %s", cfx2_get_error_desc(rc)))

    cfx2_release_node(&doc);

    value = cfx2_query_value(child, ".attrib2");
    tests_assert(value != NULL)
    tests_assert(strcmp(value, "value2") == 0)

    value = cfx2_query_value(child, "Child2");
    tests_assert(value != NULL)
    tests_assert(strcmp(value, "text2") == 0)

    cfx2_release_node(&child);

    return 0;
}
