#include "odb-cli/Banner.hpp"
#include "odb-sdk/Log.hpp"

extern "C" {
#include "odb-compiler/build_info.h"
}

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
static const char* widerBannerNoEscapeSequences_ =
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
static const char* wideBanner_ =
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
static const char* bannerNoEscapeSequences_ =
R"(              ▄▀▀█
            ▄▀`╫╫╠▀█
          ▄▀░"/╠╢╟▓▒▀█,
        ▄█▓▒░░╨╢R▒▓▓▌▒▀█,             ___                   ____  ____  ____
      ▄▀Å▀▓╬░]╗φ╫▀T▀▀▀██▀█,          / _ \ _ __   ___ _ __ |  _ \| __ )|  _ \
    ▄▀.┤D╠╬7┴j╟å J▒─▀█▄▀████,       | | | | '_ \ / _ \ '_ \| | | |  _ \| |_) |
  ▄▀ ^j]╚DD░÷╠╠╣~`╓▄▄▌▄▄██████,     | |_| | |_) |  __/ | | | |_| | |_) |  __/
╓▀   ^░░░ß░Ü<║╫▓▓███████████████,    \___/| .__/ \___|_| |_|____/|____/|_|
▐█▄  ~░─╚░*U⌐Å▒▒█████▀████████████,       |_|
  `██▄─ %.='╦╢╫▌█████▌▄▐██████████▌
     ▀██░⌂r3▄▒▓█████████▀▀└          %s
        ▀█▀▀▀▀`                      Version %s (%s)

)";
static const char* banner_ =
   PYRAMID_LEFT R"(              ▄▀)" PYRAMID_RIGHT R"(▀█
)" PYRAMID_LEFT R"(            ▄▀`╫)" PYRAMID_RIGHT R"(╫╠▀█
)" PYRAMID_LEFT R"(          ▄▀░"/)" PYRAMID_RIGHT R"(╠╢╟▓▒▀█,             )" TEXT       R"(
)" PYRAMID_LEFT R"(        ▄█▓▒░░╨)" PYRAMID_RIGHT R"(╢R▒▓▓▌▒▀█,           )" TEXT       R"(  ___                   ____  ____  ____
)" PYRAMID_LEFT R"(      ▄▀Å▀▓╬░])" PYRAMID_RIGHT R"(╗φ╫▀T▀▀▀██▀█,         )" TEXT       R"( / _ \ _ __   ___ _ __ |  _ \| __ )|  _ \
)" PYRAMID_LEFT R"(    ▄▀.┤D╠╬7┴j)" PYRAMID_RIGHT R"(╟å J▒─▀█▄▀████,       )" TEXT       R"(| | | | '_ \ / _ \ '_ \| | | |  _ \| |_) |
)" PYRAMID_LEFT R"(  ▄▀ ^j]╚DD░÷)" PYRAMID_RIGHT R"(╠╠╣~`╓▄▄▌▄▄██████,     )" TEXT       R"(| |_| | |_) |  __/ | | | |_| | |_) |  __/
)" PYRAMID_LEFT R"(╓▀   ^░░░ß░Ü<)" PYRAMID_RIGHT R"(║╫▓▓███████████████,   )" TEXT       R"( \___/| .__/ \___|_| |_|____/|____/|_|
)" PYRAMID_LEFT R"(▐█▄  ~░─╚░*U)" PYRAMID_RIGHT R"(⌐Å▒▒█████▀████████████, )" TEXT       R"(      |_|
)" PYRAMID_LEFT R"(  `██▄─ %.=')" PYRAMID_RIGHT R"(╦╢╫▌█████▌▄▐██████████▌
)" PYRAMID_LEFT R"(     ▀██░⌂r)" PYRAMID_RIGHT R"(3▄▒▓█████████▀▀└         )" URL        R"( %s
)" PYRAMID_LEFT R"(        ▀█▀)" PYRAMID_RIGHT R"(▀▀▀`                     )" VERSION_TEXT " Version " VERSION_NUMBER "%s " VERSION_TEXT "(" VERSION_NUMBER "%s" VERSION_TEXT R"()
)" RESET        R"(
)";

static const char* smallBannerNoEscapeSequences_ =
R"(              ▄▀▀█
            ▄▀`╫╫╠▀█
          ▄▀░"/╠╢╟▓▒▀█,
        ▄█▓▒░░╨╢R▒▓▓▌▒▀█,
      ▄▀Å▀▓╬░]╗φ╫▀T▀▀▀██▀█,
    ▄▀.┤D╠╬7┴j╟å J▒─▀█▄▀████,
  ▄▀ ^j]╚DD░÷╠╠╣~`╓▄▄▌▄▄██████,
╓▀   ^░░░ß░Ü<║╫▓▓███████████████,
▐█▄  ~░─╚░*U⌐Å▒▒█████▀████████████,
  `██▄─ %.='╦╢╫▌█████▌▄▐██████████▌
     ▀██░⌂r3▄▒▓█████████▀▀└
        ▀█▀▀▀▀`
%s
Version %s (%s)

)";
static const char* smallBanner_ =
   PYRAMID_LEFT R"(              ▄▀)" PYRAMID_RIGHT R"(▀█
)" PYRAMID_LEFT R"(            ▄▀`╫)" PYRAMID_RIGHT R"(╫╠▀█
)" PYRAMID_LEFT R"(          ▄▀░"/)" PYRAMID_RIGHT R"(╠╢╟▓▒▀█,
)" PYRAMID_LEFT R"(        ▄█▓▒░░╨)" PYRAMID_RIGHT R"(╢R▒▓▓▌▒▀█,
)" PYRAMID_LEFT R"(      ▄▀Å▀▓╬░])" PYRAMID_RIGHT R"(╗φ╫▀T▀▀▀██▀█,
)" PYRAMID_LEFT R"(    ▄▀.┤D╠╬7┴j)" PYRAMID_RIGHT R"(╟å J▒─▀█▄▀████,
)" PYRAMID_LEFT R"(  ▄▀ ^j]╚DD░÷)" PYRAMID_RIGHT R"(╠╠╣~`╓▄▄▌▄▄██████,
)" PYRAMID_LEFT R"(╓▀   ^░░░ß░Ü<)" PYRAMID_RIGHT R"(║╫▓▓███████████████,
)" PYRAMID_LEFT R"(▐█▄  ~░─╚░*U)" PYRAMID_RIGHT R"(⌐Å▒▒█████▀████████████,
)" PYRAMID_LEFT R"(  `██▄─ %.=')" PYRAMID_RIGHT R"(╦╢╫▌█████▌▄▐██████████▌
)" PYRAMID_LEFT R"(     ▀██░⌂r)" PYRAMID_RIGHT R"(3▄▒▓█████████▀▀└
)" PYRAMID_LEFT R"(        ▀█▀)" PYRAMID_RIGHT R"(▀▀▀`
)" URL          R"(%s
)" VERSION_TEXT   "Version " VERSION_NUMBER "%s " VERSION_TEXT "(" VERSION_NUMBER "%s" VERSION_TEXT R"()
)" RESET        R"(
)";

/*
   ____  _____  ____
  / __ \|  __ \|  _ \
 | |  | | |  | | |_) |
 | |  | | |  | |  _ <
 | |__| | |__| | |_) |
  \____/|_____/|____/

   ____                   _____  ____  _____
  / __ \                 |  __ \|  _ \|  __ \
 | |  | |_ __   ___ _ __ | |  | | |_) | |__) |
 | |  | | '_ \ / _ \ '_ \| |  | |  _ <|  ___/
 | |__| | |_) |  __/ | | | |__| | |_) | |
  \____/| .__/ \___|_| |_|_____/|____/|_|
        | |
        |_|

 _____             ____  _____ _____
|     |___ ___ ___|    \| __  |  _  |
|  |  | . | -_|   |  |  | __ -|   __|
|_____|  _|___|_|_|____/|_____|__|
      |_|

   ___                 ___  ___ ___
  / _ \ _ __  ___ _ _ |   \| _ ) _ \
 | (_) | '_ \/ -_) ' \| |) | _ \  _/
  \___/| .__/\___|_||_|___/|___/_|
       |_|
   ___                   ____  ____  ____
  / _ \ _ __   ___ _ __ |  _ \| __ )|  _ \
 | | | | '_ \ / _ \ '_ \| | | |  _ \| |_) |
 | |_| | |_) |  __/ | | | |_| | |_) |  __/
  \___/| .__/ \___|_| |_|____/|____/|_|
       |_|
*/
// ----------------------------------------------------------------------------
static void printBannerNormal()
{
    odb::Log::info.print(banner_, build_info_url(), build_info_version(), build_info_commit_hash());
}

// ----------------------------------------------------------------------------
static void printBannerNoEscapeSequences()
{
    odb::Log::info.print(bannerNoEscapeSequences_, build_info_url(), build_info_version(), build_info_commit_hash());
}

// ----------------------------------------------------------------------------
bool printBanner(const std::vector<std::string>& args)
{
    if (!printBanner_)
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
