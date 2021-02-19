#include "odb-cli/Banner.hpp"
#include "odb-compiler/BuildInfo.hpp"
#include "odb-sdk/Log.hpp"

#define B_BLACK   "\u001b[1;30m"
#define B_RED     "\u001b[1;31m"
#define B_GREEN   "\u001b[1;32m"
#define B_YELLOW  "\u001b[1;33m"
#define B_BLUE    "\u001b[1;34m"
#define B_MAGENTA "\u001b[1;35m"
#define B_CYAN    "\u001b[1;36m"
#define B_WHITE   "\u001b[1;37m"
#define N_BLACK   "\u001b[22;30m"
#define N_RED     "\u001b[22;31m"
#define N_GREEN   "\u001b[22;32m"
#define N_YELLOW  "\u001b[22;33m"
#define N_BLUE    "\u001b[22;34m"
#define N_MAGENTA "\u001b[22;35m"
#define N_CYAN    "\u001b[22;36m"
#define N_WHITE   "\u001b[22;37m"
#define RESET     "\u001b[0m"

#define PYRAMID_LEFT   N_YELLOW
#define PYRAMID_RIGHT  B_YELLOW
#define TEXT           B_CYAN
#define URL            B_WHITE
#define VERSION_TEXT   B_WHITE
#define VERSION_NUMBER B_CYAN

static bool printBanner_ = true;
static const char* bannerNoEscapeSequences_ =
R"(              ▄▀▀█
            ▄▀`╫╫╠▀█
          ▄▀░"/╠╢╟▓▒▀█,               ____                   _____             _    ____           _____ _____ _____
        ▄█▓▒░░╨╢R▒▓▓▌▒▀█,            / __ \                 |  __ \           | |  |  _ \   /\    / ____|_   _/ ____|
      ▄▀Å▀▓╬░]╗φ╫▀T▀▀▀██▀█,         | |  | |_ __   ___ _ __ | |  | | __ _ _ __| | _| |_) | /  \  | (___   | || |
    ▄▀.┤D╠╬7┴j╟å J▒─▀█▄▀████,       | |  | | '_ \ / _ | '_ \| |  | |/ _` | '__| |/ |  _ < / /\ \  \___ \  | || |
  ▄▀ ^j]╚DD░÷╠╠╣~`╓▄▄▌▄▄██████,     | |__| | |_) |  __| | | | |__| | (_| | |  |   <| |_) / ____ \ ____) |_| || |____
╓▀   ^░░░ß░Ü<║╫▓▓███████████████,    \____/| .__/ \___|_| |_|_____/ \__,_|_|  |_|\_|____/_/    \_|_____/|_____\_____|
▐█▄  ~░─╚░*U⌐Å▒▒█████▀████████████,        | |
  `██▄─ %.='╦╢╫▌█████▌▄▐██████████▌
     ▀██░⌂r3▄▒▓█████████▀▀└          %s
        ▀█▀▀▀▀`                      Version %s (%s)

)";
static const char* banner_ =
   PYRAMID_LEFT R"(              ▄▀)" PYRAMID_RIGHT R"(▀█
)" PYRAMID_LEFT R"(            ▄▀`╫)" PYRAMID_RIGHT R"(╫╠▀█
)" PYRAMID_LEFT R"(          ▄▀░"/)" PYRAMID_RIGHT R"(╠╢╟▓▒▀█,             )" TEXT       R"(  ____                   _____             _    ____           _____ _____ _____
)" PYRAMID_LEFT R"(        ▄█▓▒░░╨)" PYRAMID_RIGHT R"(╢R▒▓▓▌▒▀█,           )" TEXT       R"( / __ \                 |  __ \           | |  |  _ \   /\    / ____|_   _/ ____|
)" PYRAMID_LEFT R"(      ▄▀Å▀▓╬░])" PYRAMID_RIGHT R"(╗φ╫▀T▀▀▀██▀█,         )" TEXT       R"(| |  | |_ __   ___ _ __ | |  | | __ _ _ __| | _| |_) | /  \  | (___   | || |
)" PYRAMID_LEFT R"(    ▄▀.┤D╠╬7┴j)" PYRAMID_RIGHT R"(╟å J▒─▀█▄▀████,       )" TEXT       R"(| |  | | '_ \ / _ | '_ \| |  | |/ _` | '__| |/ |  _ < / /\ \  \___ \  | || |
)" PYRAMID_LEFT R"(  ▄▀ ^j]╚DD░÷)" PYRAMID_RIGHT R"(╠╠╣~`╓▄▄▌▄▄██████,     )" TEXT       R"(| |__| | |_) |  __| | | | |__| | (_| | |  |   <| |_) / ____ \ ____) |_| || |____
)" PYRAMID_LEFT R"(╓▀   ^░░░ß░Ü<)" PYRAMID_RIGHT R"(║╫▓▓███████████████,   )" TEXT       R"( \____/| .__/ \___|_| |_|_____/ \__,_|_|  |_|\_|____/_/    \_|_____/|_____\_____|
)" PYRAMID_LEFT R"(▐█▄  ~░─╚░*U)" PYRAMID_RIGHT R"(⌐Å▒▒█████▀████████████, )" TEXT       R"(       | |
)" PYRAMID_LEFT R"(  `██▄─ %.=')" PYRAMID_RIGHT R"(╦╢╫▌█████▌▄▐██████████▌
)" PYRAMID_LEFT R"(     ▀██░⌂r)" PYRAMID_RIGHT R"(3▄▒▓█████████▀▀└         )" URL        R"( %s
)" PYRAMID_LEFT R"(        ▀█▀)" PYRAMID_RIGHT R"(▀▀▀`                     )" VERSION_TEXT " Version " VERSION_NUMBER "%s " VERSION_TEXT "(" VERSION_NUMBER "%s" VERSION_TEXT R"()
)" RESET        R"(
)";

// ----------------------------------------------------------------------------
static void printBannerNormal()
{
    odb::Log::info.print(banner_, odb::BuildInfo::url(), odb::BuildInfo::version(), odb::BuildInfo::commitHash());
}

// ----------------------------------------------------------------------------
static void printBannerNoEscapeSequences()
{
    odb::Log::info.print(bannerNoEscapeSequences_, odb::BuildInfo::url(), odb::BuildInfo::version(), odb::BuildInfo::commitHash());
}

// ----------------------------------------------------------------------------
bool printBanner(const std::vector<std::string>& args)
{
    if (printBanner_ == false)
        return true;

    if (odb::Log::info.colorEnabled())
        printBannerNormal();
    else
        printBannerNoEscapeSequences();

    return true;
}

// ----------------------------------------------------------------------------
bool disableBanner(const std::vector<std::string>& args)
{
    printBanner_ = false;
    return true;
}
