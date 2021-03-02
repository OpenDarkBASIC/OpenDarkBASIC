#include "argdefgen/action.h"
#include "argdefgen/driver.h"
#include "argdefgen/node.h"
#include "argdefgen/gen.h"

int main(int argc, char** argv)
{
    struct adg_action** action_table;
    struct adg_driver driver;
    union adg_node* root;

    if (adg_driver_init(&driver) != 0)
        goto init_driver_failed;

    if ((root = adg_driver_parse_file(&driver, argv[2])) == NULL)
        goto parse_file_failed;
    adg_node_export_dot(root, argv[8]);

    if ((action_table = adg_action_table_from_nodes(root)) == NULL)
        goto create_action_list_failed;

    adg_gen_write_action_struct_cpp(stderr);
    adg_gen_write_action_table_cpp(action_table, stderr);

    adg_action_table_destroy(action_table);
    adg_node_destroy_recursive(root);
    adg_driver_deinit(&driver);

    return 0;

    create_action_list_failed : adg_node_destroy_recursive(root);
    parse_file_failed         : adg_driver_deinit(&driver);
    init_driver_failed        : return -1;
}
