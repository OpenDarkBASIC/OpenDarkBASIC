#include "odb-cli/Banner.hpp"
#include "odb-compiler/config.hpp"

static bool printBanner_ = true;
static const char* banner_ =
        R"(             ▄▀▀█
            ▄▀`╫╫╠▀█
          ▄▀░"/╠╢╟▓▒▀█,               ____                   _____             _    ____           _____ _____ _____
        ▄█▓▒░░╨╢R▒▓▓▌▒▀█,            / __ \                 |  __ \           | |  |  _ \   /\    / ____|_   _/ ____|
      ▄▀Å▀▓╬░]╗φ╫▀T▀▀▀██▀█,         | |  | |_ __   ___ _ __ | |  | | __ _ _ __| | _| |_) | /  \  | (___   | || |
    ▄▀.┤D╠╬7┴j╟å J▒─▀█▄▀████,       | |  | | '_ \ / _ | '_ \| |  | |/ _` | '__| |/ |  _ < / /\ \  \___ \  | || |
  ▄▀ ^j]╚DD░÷╠╠╣~`╓▄▄▌▄▄██████,     | |__| | |_) |  __| | | | |__| | (_| | |  |   <| |_) / ____ \ ____) |_| || |____
╓▀   ^░░░ß░Ü<║╫▓▓███████████████,    \____/| .__/ \___|_| |_|_____/ \__,_|_|  |_|\_|____/_/    \_|_____/|_____\_____|
▐█▄  ~░─╚░*U⌐Å▒▒█████▀████████████,        | |
  `██▄─ %.='╦╢╫▌█████▌▄▐██████████▌
     ▀██░⌂r3▄▒▓█████████▀▀└
        ▀█▀▀▀▀`
)";


bool printBanner(const std::vector<std::string>& args)
{
    if (printBanner_)
    {
#define UP "\u001b[%dA"
#define RIGHT "\u001b[%dC"
        fprintf(stderr, "%s", banner_);
        fprintf(stderr, UP, 2);
        fprintf(stderr, RIGHT, 37);
        fprintf(stderr, "github.com/OpenDarkBASIC/OpenDarkBASIC\n");
        fprintf(stderr, RIGHT, 37);
        fprintf(stderr, "Version " ODBCOMPILER_VERSION_STR "\n\n");
    }

    return true;
}

bool disableBanner(const std::vector<std::string>& args)
{
    printBanner_ = false;
    return true;
}
