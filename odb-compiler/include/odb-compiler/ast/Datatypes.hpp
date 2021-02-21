#pragma once

#include <cstdint>
#include <string>
#include <array>

/*!
 * @brief All of the DarkBASIC primitive types that can exist and what types
 * they map to in C++
 */
#define ODB_DATATYPE_LIST                  \
    X(DoubleInteger, int64_t)              \
    X(Integer,       int32_t)              \
    X(Dword,         uint32_t)             \
    X(Word,          uint16_t)             \
    X(Byte,          uint8_t)              \
    X(Boolean,       bool)                 \
    X(DoubleFloat,   double)               \
    X(Float,         float)                \
    X(String,        std::string)          \
    X(Complex,       odb::Complex<float>)  \
    X(Quat,          odb::Quat<float>)     \
    X(Vec2,          odb::Vec2<float>)     \
    X(Vec3,          odb::Vec3<float>)     \
    X(Vec4,          odb::Vec4<float>)     \
    X(Mat2x2,        odb::Mat2x2<float>)   \
    X(Mat2x3,        odb::Mat2x3<float>)   \
    X(Mat2x4,        odb::Mat2x4<float>)   \
    X(Mat3x2,        odb::Mat3x2<float>)   \
    X(Mat3x3,        odb::Mat3x3<float>)   \
    X(Mat3x4,        odb::Mat3x4<float>)   \
    X(Mat4x2,        odb::Mat4x2<float>)   \
    X(Mat4x3,        odb::Mat4x3<float>)   \
    X(Mat4x4,        odb::Mat4x4<float>)

namespace odb {

template <typename T> struct Complex : std::array<T, 2> {};
template <typename T> struct Quat    : std::array<T, 4> {};
template <typename T> struct Vec2    : std::array<T, 2> {};
template <typename T> struct Vec3    : std::array<T, 3> {};
template <typename T> struct Vec4    : std::array<T, 4> {};
template <typename T> struct Mat2x2  : std::array<std::array<T, 2>, 2> {};
template <typename T> struct Mat2x3  : std::array<std::array<T, 2>, 3> {};
template <typename T> struct Mat2x4  : std::array<std::array<T, 2>, 4> {};
template <typename T> struct Mat3x2  : std::array<std::array<T, 3>, 2> {};
template <typename T> struct Mat3x3  : std::array<std::array<T, 3>, 3> {};
template <typename T> struct Mat3x4  : std::array<std::array<T, 3>, 4> {};
template <typename T> struct Mat4x2  : std::array<std::array<T, 4>, 2> {};
template <typename T> struct Mat4x3  : std::array<std::array<T, 4>, 3> {};
template <typename T> struct Mat4x4  : std::array<std::array<T, 4>, 4> {};

}
