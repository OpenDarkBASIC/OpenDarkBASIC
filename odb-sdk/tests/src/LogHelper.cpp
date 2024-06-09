#include "odb-sdk/tests/LogHelper.hpp"

extern "C" {
#include "odb-sdk/cli_colors.h"
}

int
diffPos(const std::string& a, const std::string& b)
{
    int pos = 0;
    for (; pos < a.size() && pos < b.size(); ++pos)
        if (a[pos] != b[pos])
            break;
    return pos;
}

bool
LogEqMatcher::MatchAndExplain(
    const LogOutput& logOutput, testing::MatchResultListener* listener) const
{
    int         pos = diffPos(logOutput.text, expected);
    std::string highlighted = logOutput.text;
    highlighted.insert(pos, FGB_RED);
    std::replace(highlighted.begin(), highlighted.end(), ' ', '.');
    *listener << "Log:\n" << highlighted;
    return logOutput.text == expected;
}

void
LogEqMatcher::DescribeTo(::std::ostream* os) const
{
    std::string highlighted = expected;
    std::replace(highlighted.begin(), highlighted.end(), ' ', '.');
    *os << "Log output equals:\n" << highlighted;
}

void
LogEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    std::string highlighted = expected;
    std::replace(highlighted.begin(), highlighted.end(), ' ', '.');
    *os << "Log output does not equal:\n" << highlighted;
}

static LogOutput log_output;
static void
write_str(const char* fmt, va_list ap)
{
    va_list ap2;
    va_copy(ap2, ap);
    int len = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    size_t off = log_output.text.size();
    log_output.text.resize(log_output.text.size() + len + 1);
    vsprintf(log_output.text.data() + off, fmt, ap);
    log_output.text.resize(log_output.text.size() - 1);
}

LogHelper::LogHelper()
{
    struct log_interface i = {write_str, 0};
    old_log_interface = log_configure(i);
}

LogHelper::~LogHelper()
{
    log_configure(old_log_interface);
    log_output.text.clear();
}

const LogOutput&
LogHelper::log() const
{
    return log_output;
}
