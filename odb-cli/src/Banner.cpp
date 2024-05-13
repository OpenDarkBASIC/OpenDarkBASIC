#include "odb-cli/Banner.hpp"

extern "C" {
#include "odb-compiler/build_info.h"
#include "odb-sdk/cli_colors.h"
#include "odb-sdk/log.h"
}

#define PYRAMID_LEFT   FG_YELLOW
#define PYRAMID_RIGHT  FGB_YELLOW
#define TEXT           FGB_CYAN
#define URL            FGB_WHITE
#define VERSION_TEXT   FGB_WHITE
#define VERSION_NUMBER FGB_CYAN

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
  `██▄─ &.='╦╢╫▌█████▌▄▐██████████▌
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
)" PYRAMID_LEFT R"(  `██▄─ &.=')" PYRAMID_RIGHT R"(╦╢╫▌█████▌▄▐██████████▌
)" PYRAMID_LEFT R"(     ▀██░⌂r)" PYRAMID_RIGHT R"(3▄▒▓█████████▀▀└         )" URL        R"( %s
)" PYRAMID_LEFT R"(        ▀█▀)" PYRAMID_RIGHT R"(▀▀▀`                     )" VERSION_TEXT " Version " VERSION_NUMBER "%s " VERSION_TEXT "(" VERSION_NUMBER "%s" VERSION_TEXT R"()
)" COL_RESET    R"(
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
  `██▄─ &.='╦╢╫▌█████▌▄▐██████████▌
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
)" PYRAMID_LEFT R"(  `██▄─ &.=')" PYRAMID_RIGHT R"(╦╢╫▌█████▌▄▐██████████▌
)" PYRAMID_LEFT R"(     ▀██░⌂r)" PYRAMID_RIGHT R"(3▄▒▓█████████▀▀└         )" URL        R"( %s
)" PYRAMID_LEFT R"(        ▀█▀)" PYRAMID_RIGHT R"(▀▀▀`                     )" VERSION_TEXT " Version " VERSION_NUMBER "%s " VERSION_TEXT "(" VERSION_NUMBER "%s" VERSION_TEXT R"()
)" COL_RESET    R"(
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
  `██▄─ &.='╦╢╫▌█████▌▄▐██████████▌
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
)" PYRAMID_LEFT R"(  `██▄─ &.=')" PYRAMID_RIGHT R"(╦╢╫▌█████▌▄▐██████████▌
)" PYRAMID_LEFT R"(     ▀██░⌂r)" PYRAMID_RIGHT R"(3▄▒▓█████████▀▀└
)" PYRAMID_LEFT R"(        ▀█▀)" PYRAMID_RIGHT R"(▀▀▀`
)" URL          R"(%s
)" VERSION_TEXT   "Version " VERSION_NUMBER "%s " VERSION_TEXT "(" VERSION_NUMBER "%s" VERSION_TEXT R"()
)" COL_RESET    R"(
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
    log_raw(banner_, build_info_url(), build_info_version(), build_info_commit_hash());
}

// ----------------------------------------------------------------------------
static void printBannerNoEscapeSequences()
{
    log_raw(bannerNoEscapeSequences_, build_info_url(), build_info_version(), build_info_commit_hash());
}

// ----------------------------------------------------------------------------
bool printBanner(const std::vector<std::string>& args)
{
    if (!printBanner_)
        return true;

    //if (odb::Log::info.colorEnabled())
        printBannerNormal();
    //else
    //    printBannerNoEscapeSequences();

    return true;
}

// ----------------------------------------------------------------------------
bool disableBanner(const std::vector<std::string>& args)
{
    printBanner_ = false;
    return true;
}
