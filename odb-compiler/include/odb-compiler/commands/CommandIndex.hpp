#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/commands/SDKType.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-sdk/Reference.hpp"
#include <string>
#include <vector>

namespace odb {
namespace cmd {

/*!
 * This class is a generic container for all commands. It acts as an
 * intermediate storage when collecting commands from plugins or config files.
 *
 * This class is not designed for fast command queries. It is recommended to
 * create a specialized container if this is required. An example of this is
 * the @see CommandMatcher class.
 */
class ODBCOMPILER_PUBLIC_API CommandIndex
{
public:
    void addCommand(Command* command);

    /*!
     * @brief Tries to find any globally conflicting commands, such as identical
     * commands coming from different plugins, or commands that share the same
     * overload.
     */
    bool findConflicts() const;

    const std::vector<Reference<Command>>& commands() const;
    std::vector<std::string> commandNamesAsList() const;
    std::vector<std::string> librariesAsList() const;

private:
    std::vector<Reference<Command>> commands_;
};

}
}
