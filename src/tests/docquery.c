
#include "tests.h"

#include "usertable.h"

#include <string.h>

typedef struct
{
    cfx2_ICmdListener listener;

    int visited;
}
MyListener;

static void on_node( struct cfx2_ICmdListener* listener, cfx2_Node* node )
{
    ((MyListener*) listener)->visited++;
}

int docquery()
{
    cfx2_Node* user_table, * Users;
    int rc;

    cfx2_Cmd* cmd;
    MyListener my_listener;

    rc = cfx2_read_file(usertable_filename, &user_table);

    if (rc != cfx2_ok)
        tests_fail(("failed to load '%s': %i", usertable_filename, rc))

    tests_assert((Users = cfx2_find_child(user_table, "Users")) != NULL)
    tests_assert((cmd = cfx2_compile("select randuser*")) != NULL)

    my_listener.listener.on_node = on_node;
    my_listener.visited = 0;

    cfx2_execute_command(Users, cmd, &my_listener.listener);

    tests_assert(my_listener.visited == 9)

    cfx2_release_command(cmd);
    cfx2_release_node_2(&user_table);

    return 0;
}
