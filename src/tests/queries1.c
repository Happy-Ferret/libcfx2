
#include "tests.h"

#include "usertable.h"

#include <string.h>

static unsigned int hash_password(const char* password)
{
    int hash;

    for (hash = 0; *password != 0; password++)
         hash += (*password) - 'a';

    return (unsigned int)hash & 0x7F;
}

static int attempt_login(cfx2_Node* user_table, const char* username, const char* password, const char* home_dir)
{
    char query[255];
    const char* hasPassword, *homeDir;

    tests_assert(password != NULL)

    sprintf(query, "Users/%s.hasPassword", username);
    hasPassword = cfx2_query_value(user_table, query);
    tests_assert_2(hasPassword != NULL, query)

    if (strcmp(hasPassword, "1") == 0)
    {
        const char* passwordHash;
        int hash1, hash2;

        sprintf(query, "Users/%s.passwordHash", username);
        passwordHash = cfx2_query_value(user_table, query);
        tests_assert_2(passwordHash != NULL, query)

        hash1 = (int) strtoul(passwordHash, NULL, 0);
        hash2 = hash_password(password);

        if (hash1 != hash2)
            return 0;
    }
    else
        tests_assert(password[0] == 0)

    sprintf(query, "Users/%s.homeDir", username);
    homeDir = cfx2_query_value(user_table, query);
    tests_assert_2(homeDir != NULL, query)

    if (strcmp(homeDir, home_dir) != 0)
        tests_fail(("mismatch: '%s' (got) vs '%s' (expected)", homeDir, home_dir));

    return 1;
}

int queries1(void)
{
    cfx2_Node* user_table;
    tests_Perf perf;

    int rc;

    tests_perf_start(&perf);

    rc = cfx2_read_file(&user_table, usertable_filename, NULL);

    if (rc != cfx2_ok)
        tests_fail(("failed to load '%s': %s", usertable_filename, cfx2_get_error_desc(rc)))

    tests_perf_end(&perf, "read user table");

    tests_assert(attempt_login(user_table, "root", "haxhax", "/root") == 0)
    tests_assert(attempt_login(user_table, "root", "foobar", "/root") == 1)
    tests_assert(attempt_login(user_table, "guest", "", "/home/guest") == 1)

    cfx2_release_node(&user_table);

    return 0;
}
