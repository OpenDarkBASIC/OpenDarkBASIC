#pragma once

#include <gmock/gmock.h>
#include "odbc/Driver.hpp"

class ParserTestHarness : public testing::Test
{
public:
    void SetUp() override { driver = new odbc::Driver; }
    void TearDown() override { delete driver; }
    odbc::Driver* driver;
};
