/********************************************************************************************************

Authors:        (c) 2023-2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
********************************************************************************************************

Description:

    Compatibility include for the Maths Town graphics linear algebra library.

    The project still uses the old lowercase, global `vec2`/`vec3`/`vec4` names in a number of headers.
    Keep that source surface for now while routing the implementation through external/graphics.

*********************************************************************************************************/
#pragma once

#include <concepts>

#include "linalg.h"

template <typename F> using Vec2 = mt::Vec2<F>;
template <typename F> using Vec3 = mt::Vec3<F>;
template <typename F> using Vec4 = mt::Vec4<F>;
template <typename F> using Mat2 = mt::Mat2<F>;
template <typename F> using Mat3 = mt::Mat3<F>;
template <typename F> using Mat4 = mt::Mat4<F>;

template <typename F> using vec2 = mt::vec2<F>;
template <typename F> using vec3 = mt::vec3<F>;
template <typename F> using vec4 = mt::vec4<F>;
template <typename F> using mat2 = mt::mat2<F>;
template <typename F> using mat3 = mt::mat3<F>;
template <typename F> using mat4 = mt::mat4<F>;

using mt::abs;
using mt::acos;
using mt::asin;
using mt::atan;
using mt::atan2;
using mt::ceil;
using mt::clamp_to_int;
using mt::cos;
using mt::cross;
using mt::degrees;
using mt::distance;
using mt::dot;
using mt::exp;
using mt::exp2;
using mt::floor;
using mt::fma;
using mt::fms;
using mt::fnma;
using mt::fnms;
using mt::fract;
using mt::inverse;
using mt::length;
using mt::lerp;
using mt::log;
using mt::log2;
using mt::magnitude;
using mt::mix;
using mt::normalize;
using mt::pow;
using mt::radians;
using mt::reflect;
using mt::refract;
using mt::remap;
using mt::rescale;
using mt::rescale_from_01;
using mt::rescale_to_01;
using mt::round;
using mt::sin;
using mt::smoothstep;
using mt::sqrt;
using mt::step;
using mt::tan;
using mt::transpose;
using mt::trunc;
using mt::wrap;

template <typename F>
[[nodiscard]]
inline F clamp(F value, F min_value, F max_value) noexcept {
    return mt::clamp(value, min_value, max_value);
}

template <typename F, typename Min, typename Max>
[[nodiscard]]
inline F clamp(F value, Min min_value, Max max_value) noexcept
    requires(!std::same_as<F, Min> || !std::same_as<F, Max>)
{
    return mt::clamp(value, static_cast<F>(min_value), static_cast<F>(max_value));
}

template <typename F>
[[nodiscard]]
inline F clamp(F value) noexcept {
    return mt::clamp(value);
}

template <typename F>
[[nodiscard]]
inline F clamp_01(F value) noexcept {
    return clamp(value);
}
