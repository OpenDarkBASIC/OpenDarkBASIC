#include "argdefgen/driver.h"
#include "argdefgen/node.h"

int main(int argc, char** argv)
{
    struct adg_driver driver;
    union adg_node* root;

    if (adg_driver_init(&driver) != 0)
        goto init_driver_failed;

    if ((root = adg_driver_parse_file(&driver, argv[2])) == NULL)
        goto parse_file_failed;

    adg_node_export_dot(root, argv[8]);
    adg_node_destroy_recursive(root);
    adg_driver_deinit(&driver);

    return 0;

    parse_file_failed  : adg_driver_deinit(&driver);
    init_driver_failed : return -1;
}
