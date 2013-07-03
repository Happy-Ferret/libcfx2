
#include "tests.h"

static const char* filenames[] =
{
    "damaged1.cfx2",
    "damaged2.cfx2",
    "damaged3.cfx2",
    "damaged4.cfx2",
    NULL
};

int parseerror(void)
{
    const char** p_filename;
    int rc;

    /* load a couple of damaged documents, one after another */

    for (p_filename = &filenames[0]; *p_filename != NULL; p_filename++)
    {
        cfx2_Node* doc;
        cfx2_RdOpt rd_opt;
        
        rd_opt.on_error = &tests_parse_error;
        rd_opt.flags = 0;

        rc = cfx2_read_file( &doc, *p_filename, &rd_opt );

        if (rc == cfx2_ok)
            tests_fail(("loaded corrupted document '%s'", *p_filename))
        else if (rc != cfx2_syntax_error)
            tests_fail(("error in parsing '%s': %s", *p_filename, cfx2_get_error_desc(rc)))
    }

    return 0;
}
