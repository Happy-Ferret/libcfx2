
#include "tests.h"
#include "huge.h"

void generate_random_alphanum(char* buffer, size_t size)
{
    size_t i;

    for (i = 0; i < size - 1; i++)
        buffer[i] = ((rand() & 1) == 0 ? 'A' : 'a') + (rand() % 26);

    buffer[i] = 0;
}

void generate_random_ascii(char* buffer, size_t size)
{
    size_t i;

    for (i = 0; i < size - 1; i++)
        buffer[i] = 32 + (rand() % 95);

    buffer[i] = 0;
}

int gen_huge(void)
{
    cfx2_Node* doc;
    tests_Perf perf;

    size_t i;
    int j, rc;

    tests_info(("will generate %i nodes", huge_node_count))

    doc = cfx2_new_node(NULL);
    tests_assert(doc != NULL)

    srand(0);

    tests_perf_start(&perf);

    for (i = 0; i < huge_node_count; i++)
    {
        cfx2_Node* child;
        char name[16], text[32], attrib_name[8], attrib_value[16];

        generate_random_alphanum(name, sizeof(name));
        generate_random_ascii(text, sizeof(text));

        child = cfx2_create_child(doc, name, text, cfx2_multiple);
        tests_assert(child != NULL)

        cfx2_preallocate_shared_buffer(child, 3 * (sizeof(attrib_name) + sizeof(attrib_value) + 8), 0);

        for (j = 0; j < 3; j++)
        {
            generate_random_alphanum(attrib_name, sizeof(attrib_name));
            generate_random_ascii(attrib_value, sizeof(attrib_value));

            tests_assert(cfx2_set_node_attrib(child, attrib_name, attrib_value) == cfx2_ok)
        }
    }

    tests_perf_end(&perf, "generate document in memory");
    tests_memory_usage_check();
    tests_perf_start(&perf);

    rc = cfx2_save_document(doc, huge_filename);

    if (rc != cfx2_ok)
        tests_fail(("failed to save '%s': %i", huge_filename, rc))

    tests_perf_end(&perf, "save document to disk");

    cfx2_release_node(&doc);

    return 0;
}
