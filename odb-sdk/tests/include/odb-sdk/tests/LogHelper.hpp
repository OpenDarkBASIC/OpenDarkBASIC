#include <string>

#include "gmock/gmock.h"

extern "C" {
#include "odb-sdk/log.h"
}

struct LogOutput
{
    std::string text;
};

struct LogHelper
{
    LogHelper(char color_mode=0);
    ~LogHelper();

    const LogOutput& log() const;

private:
    struct log_interface old_log_interface;
};

struct LogEqMatcher : testing::MatcherInterface<const LogOutput&>
{
    explicit LogEqMatcher(const char* expected) : expected(expected) {}

    bool
    MatchAndExplain(
        const LogOutput&              logOutput,
        testing::MatchResultListener* listener) const override;

    void
    DescribeTo(::std::ostream* os) const override;
    void
    DescribeNegationTo(::std::ostream* os) const override;

    std::string expected;
};

inline testing::Matcher<const LogOutput&>
LogEq(const char* expected)
{
    return testing::MakeMatcher(new LogEqMatcher(expected));
}
