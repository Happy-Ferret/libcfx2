
#include "tests.h"

#include "../io.h"

static const char* filenames[] =
{
    "damaged1.cfx2",
    "damaged2.cfx2",
    "damaged3.cfx2",
    "damaged4.cfx2",
    NULL
};

int parseerror()
{
    const char** p_filename;
    int rc;

    /* load a couple of damaged documents, one after another */

    for (p_filename = &filenames[0]; *p_filename != NULL; p_filename++)
    {
        cfx2_Node* doc;
        cfx2_IInput* input;

        rc = new_buffer_input_from_file( *p_filename, &input );

        if (rc)
            tests_fail(("failed to load document '%s'", *p_filename))

        input->handle_error = &tests_parse_error;

        rc = cfx2_read( input, &doc );

        if (rc == cfx2_ok)
            tests_fail(("loaded corrupted document '%s'", *p_filename))
        else if (rc != cfx2_syntax_error)
            tests_fail(("error in parsing '%s': %i", *p_filename, rc))
    }

    return 0;
}
