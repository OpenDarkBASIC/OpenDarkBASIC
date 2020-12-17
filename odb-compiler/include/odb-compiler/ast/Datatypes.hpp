#pragma once

#include <cstdint>
#include <string>

/*!
 * @brief All of the DarkBASIC primitive types that can exist and what types
 * they map to in C++
 */
#define ODB_DATATYPE_LIST     \
    X(DoubleInteger, int64_t) \
    X(Integer, int32_t)       \
    X(Dword, uint32_t)        \
    X(Word, uint16_t)         \
    X(Byte, uint8_t)          \
    X(Boolean, bool)          \
    X(DoubleFloat, double)    \
    X(Float, float)           \
    X(String, std::string)
