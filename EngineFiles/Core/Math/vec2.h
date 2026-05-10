/*
Copyright (c) 2020 Chan Jer Shyan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef VEC2_H
#define VEC2_H

#include <cmath>
#include <numbers>

template <class T>
class vec2 {
public:
    T x, y;

    vec2() : x(0), y(0) {}
    vec2(T x, T y) : x(x), y(y) {}
    vec2(const vec2&) = default;

    vec2& operator=(const vec2&) = default;

    vec2 operator+(const vec2& v) const {
        return vec2<T>(x + v.x, y + v.y);
    }

    vec2 operator-(const vec2& v) const {
        return vec2<T>(x - v.x, y - v.y);
    }

    vec2& operator+=(const vec2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    vec2& operator-=(const vec2& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    vec2 operator+(double s) const {
        return vec2<T>(x + static_cast<T>(s), y + static_cast<T>(s));
    }

    vec2 operator-(double s) const {
        return vec2<T>(x - static_cast<T>(s), y - static_cast<T>(s));
    }

    vec2 operator*(double s) const {
        return vec2<T>(x * static_cast<T>(s), y * static_cast<T>(s));
    }

    vec2 operator/(double s) const {
        return vec2<T>(
            x / static_cast<T>(s),
            y / static_cast<T>(s)
        );
    }

    vec2& operator+=(double s) {
        x += static_cast<T>(s);
        y += static_cast<T>(s);
        return *this;
    }

    vec2& operator-=(double s) {
        x -= static_cast<T>(s);
        y -= static_cast<T>(s);
        return *this;
    }

    vec2& operator*=(double s) {
        x *= static_cast<T>(s);
        y *= static_cast<T>(s);
        return *this;
    }

    vec2& operator/=(double s) {
        x /= static_cast<T>(s);
        y /= static_cast<T>(s);
        return *this;
    }

    void set(T x, T y) {
        this->x = x;
        this->y = y;
    }

    void rotate(double deg) {
        double theta = deg * (std::numbers::pi_v<float> / 180.0);
        double c = std::cos(theta);
        double s = std::sin(theta);

        T tx = static_cast<T>(x * c - y * s);
        T ty = static_cast<T>(x * s + y * c);

        x = tx;
        y = ty;
    }

    vec2& normalize() {
        T len = length();
        if (len != T(0)) {
            *this /= len;
        }
        return *this;
    }

    T dist(const vec2& v) const {
        return (*this - v).length();
    }

    T length() const {
        return static_cast<T>(std::sqrt(x * x + y * y));
    }

    void truncate(T maxLength) {
        T len = length();
        if (len > maxLength && len != T(0)) {
            *this *= (maxLength / len);
        }
    }

    vec2 ortho() const {
        return vec2<T>(y, -x);
    }

    static T dot(const vec2& a, const vec2& b) {
        return a.x * b.x + a.y * b.y;
    }

    static T cross(const vec2& a, const vec2& b) {
        return a.x * b.y - a.y * b.x;
    }
};

using vec2f = vec2<float>;
using vec2d = vec2<double>;

#endif

