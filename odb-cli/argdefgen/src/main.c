#include "argdefgen/driver.h"

int main(int argc, char** argv)
{
    struct adg_driver driver;
    if (adg_driver_init(&driver) != 0)
        goto init_driver_failed;

    if (adg_driver_parse_file(&driver, argv[2]) != 0)
        goto parse_file_failed;

    adg_driver_deinit(&driver);
    return 0;

    parse_file_failed  : adg_driver_deinit(&driver);
    init_driver_failed : return -1;
}
