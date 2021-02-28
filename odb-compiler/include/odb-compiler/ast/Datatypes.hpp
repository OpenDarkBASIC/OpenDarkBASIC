#pragma once

#include <cstdint>
#include <string>
#include <array>

/*!
 * @brief All of the DarkBASIC primitive types that can exist and what types
 * they map to in C++
 */
#define ODB_DATATYPE_LIST                       \
    X(DoubleInteger, int64_t)                   \
    X(Integer,       int32_t)                   \
    X(Dword,         uint32_t)                  \
    X(Word,          uint16_t)                  \
    X(Byte,          uint8_t)                   \
    X(Boolean,       bool)                      \
    X(DoubleFloat,   double)                    \
    X(Float,         float)                     \
    X(String,        std::string)               \
    X(Complex,       odb::ast::Complex<float>)  \
    X(Mat2x2,        odb::ast::Mat2x2<float>)   \
    X(Mat2x3,        odb::ast::Mat2x3<float>)   \
    X(Mat2x4,        odb::ast::Mat2x4<float>)   \
    X(Mat3x2,        odb::ast::Mat3x2<float>)   \
    X(Mat3x3,        odb::ast::Mat3x3<float>)   \
    X(Mat3x4,        odb::ast::Mat3x4<float>)   \
    X(Mat4x2,        odb::ast::Mat4x2<float>)   \
    X(Mat4x3,        odb::ast::Mat4x3<float>)   \
    X(Mat4x4,        odb::ast::Mat4x4<float>)   \
    X(Quat,          odb::ast::Quat<float>)     \
    X(Vec2,          odb::ast::Vec2<float>)     \
    X(Vec3,          odb::ast::Vec3<float>)     \
    X(Vec4,          odb::ast::Vec4<float>)

namespace odb::ast {

template <typename T> struct Complex {
    Complex() : real(0), imag(0){}
    Complex(T real, T imag) : real(real), imag(imag){}
    T real, imag; };
template <typename T> struct Quat {
    Quat() : r(1), i(0), j(0), k(0) {}
    Quat(T r, T i, T j, T k) : r(r), i(i), j(j), k(k){}
    T r, i, j, k; };
template <typename T> struct Vec2 {
    Vec2() : x(0), y(0) {}
    Vec2(T x, T y) : x(x), y(y){}
    T x, y; };
template <typename T> struct Vec3 {
    Vec3() : x(0), y(0), z(0) {}
    Vec3(T x, T y, T z) : x(x), y(y), z(z){}
    T x, y, z; };
template <typename T> struct Vec4 {
    Vec4() : x(0), y(0), z(0), w(1) {}
    Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w){}
    T x, y, z, w; };
template <typename T> struct Mat2x2 {
    Mat2x2() : e0(1, 0)
             , e1(0, 1) {}
    Mat2x2(Vec2<T> e0, Vec2<T> e1) : e0(e0), e1(e1){}
    Vec2<T> e0, e1; };
template <typename T> struct Mat2x3 {
    Mat2x3() : e0(0, 0)
             , e1(0, 0)
             , e2(0, 0) {}
    Mat2x3(Vec2<T> e0, Vec2<T> e1, Vec2<T> e2) : e0(e0), e1(e1), e2(e2){}
    Vec2<T> e0, e1, e2; };
template <typename T> struct Mat2x4 {
    Mat2x4() : e0(0, 0)
             , e1(0, 0)
             , e2(0, 0)
             , e3(0, 0) {}
    Mat2x4(Vec2<T> e0, Vec2<T> e1, Vec2<T> e2, Vec2<T> e3) : e0(e0), e1(e1), e2(e2), e3(e3){}
    Vec2<T> e0, e1, e2, e3; };
template <typename T> struct Mat3x2 {
    Mat3x2() : e0(0, 0, 0)
             , e1(0, 0, 0) {}
    Mat3x2(Vec3<T> e0, Vec3<T> e1) : e0(e0), e1(e1){}
    Vec3<T> e0, e1; };
template <typename T> struct Mat3x3 {
    Mat3x3() : e0(1, 0, 0)
             , e1(0, 1, 0)
             , e2(0, 0, 1) {}
    Mat3x3(Vec3<T> e0, Vec3<T> e1, Vec3<T> e2) : e0(e0), e1(e1), e2(e2){}
    Vec3<T> e0, e1, e2; };
template <typename T> struct Mat3x4 {
    Mat3x4() : e0(0, 0, 0)
             , e1(0, 0, 0)
             , e2(0, 0, 0)
             , e3(0, 0, 0) {}
    Mat3x4(Vec3<T> e0, Vec3<T> e1, Vec3<T> e2, Vec3<T> e3) : e0(e0), e1(e1), e2(e2), e3(e3){}
    Vec3<T> e0, e1, e2, e3; };
template <typename T> struct Mat4x2 {
    Mat4x2() : e0(0, 0, 0, 0)
             , e1(0, 0, 0, 0) {}
    Mat4x2(Vec4<T> e0, Vec4<T> e1) : e0(e0), e1(e1){}
    Vec4<T> e0, e1; };
template <typename T> struct Mat4x3 {
    Mat4x3() : e0(0, 0, 0, 0)
             , e1(0, 0, 0, 0)
             , e2(0, 0, 0, 0) {}
    Mat4x3(Vec4<T> e0, Vec4<T> e1, Vec4<T> e2) : e0(e0), e1(e1), e2(e2){}
    Vec4<T> e0, e1, e2; };
template <typename T> struct Mat4x4 {
    Mat4x4() : e0(1, 0, 0, 0)
             , e1(0, 1, 0, 0)
             , e2(0, 0, 1, 0)
             , e3(0, 0, 0, 1) {}
    Mat4x4(Vec4<T> e0, Vec4<T> e1, Vec4<T> e2, Vec4<T> e3) : e0(e0), e1(e1), e2(e2), e3(e3){}
    Vec4<T> e0, e1, e2, e3; };

}
