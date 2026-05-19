/********************************************************************************************************

Authors:        (c) 2026 Maths Town

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

Type:           Vec4
                (aliased to vec4, for easy glsl porting)

Description:    A 4-vector that supports floating point types and types implementing the concepts from the
                Maths Town SIMD library. All member functions, except constructors and self-assignment
                operators, are const to allow chaining.

Operators:      == - + - * / += -= *= /= with vector and scalar forms

Member Functions:
                Swizzle:              xy xz xw yz yw zw xyz xyw xzw yzw
                Length:               magnitude length normalize
                Component Math:       floor ceil round fract trunc abs sqrt exp exp2 log log2
                Trig:                 degrees radians sin cos tan asin acos atan
                Range:                clamp clamp(min,max)

Free Functions:
                Operators:            + - * /
                Geometry:             dot refract normalize distance length magnitude
                Component Math:       floor ceil round fract trunc abs sqrt min max clamp pow exp exp2 log log2
                Trig:                 degrees radians sin cos tan asin acos atan atan2
                Fused Multiply Add:   fma fms fnma fnms
                Range:                rescale_from_01 rescale_to_01 rescale remap wrap smoothstep step
                Vector Helpers:       reflect refract mix lerp

*********************************************************************************************************/
#pragma once

#include <algorithm>
#include <cmath>
#include "common.h"
#include "simd-concepts.h"
#include "vec2.h"
#include "vec3.h"

namespace mt {

/**************************************************************************************************
 * A 4-Vector (x,y,z,w)
 * ************************************************************************************************/
template <typename F> struct Vec4 {
	using ComponentType = F;

	F x{};
	F y{};
	F z{};
	F w{};

	Vec4() = default;

	Vec4(F v) noexcept : x(v), y(v), z(v), w(v) { }

	Vec4(F x1, F y1, F z1, F w1) noexcept : x(x1), y(y1), z(z1), w(w1) { }

	Vec4(const Vec3<F>& xyz, F w1) noexcept : x(xyz.x), y(xyz.y), z(xyz.z), w(w1) { }

	Vec4(F x1, const Vec3<F>& yzw) noexcept : x(x1), y(yzw.x), z(yzw.y), w(yzw.z) { }

	Vec4(const Vec2<F>& xy, const Vec2<F>& zw) noexcept : x(xy.x), y(xy.y), z(zw.x), w(zw.y) { }

	Vec4(const Vec2<F>& xy, F z1, F w1) noexcept : x(xy.x), y(xy.y), z(z1), w(w1) { }

	Vec4(F x1, F y1, const Vec2<F>& zw) noexcept : x(x1), y(y1), z(zw.x), w(zw.y) { }

	[[nodiscard]]
	bool operator==(const Vec4& rhs) const noexcept
	    requires linalg_detail::BoolEqualityComparable<F>
	{
		return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
	}

	[[nodiscard]]
	Vec4 operator-() const noexcept {
		return Vec4(-x, -y, -z, -w);
	}

	Vec4& operator+=(const Vec4& rhs) noexcept {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		w += rhs.w;
		return *this;
	}

	Vec4& operator-=(const Vec4& rhs) noexcept {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		w -= rhs.w;
		return *this;
	}

	Vec4& operator*=(const Vec4& rhs) noexcept {
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		w *= rhs.w;
		return *this;
	}

	Vec4& operator/=(const Vec4& rhs) noexcept {
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;
		w /= rhs.w;
		return *this;
	}

	Vec4& operator+=(const F rhs) noexcept {
		x += rhs;
		y += rhs;
		z += rhs;
		w += rhs;
		return *this;
	}

	Vec4& operator-=(const F rhs) noexcept {
		x -= rhs;
		y -= rhs;
		z -= rhs;
		w -= rhs;
		return *this;
	}

	Vec4& operator*=(const F rhs) noexcept {
		x *= rhs;
		y *= rhs;
		z *= rhs;
		w *= rhs;
		return *this;
	}

	Vec4& operator/=(const F rhs) noexcept {
		x /= rhs;
		y /= rhs;
		z /= rhs;
		w /= rhs;
		return *this;
	}

	[[nodiscard]]
	inline Vec2<F> xy() const noexcept {
		return Vec2<F>(x, y);
	}

	[[nodiscard]]
	inline Vec2<F> xz() const noexcept {
		return Vec2<F>(x, z);
	}

	[[nodiscard]]
	inline Vec2<F> xw() const noexcept {
		return Vec2<F>(x, w);
	}

	[[nodiscard]]
	inline Vec2<F> yz() const noexcept {
		return Vec2<F>(y, z);
	}

	[[nodiscard]]
	inline Vec2<F> yw() const noexcept {
		return Vec2<F>(y, w);
	}

	[[nodiscard]]
	inline Vec2<F> zw() const noexcept {
		return Vec2<F>(z, w);
	}

	[[nodiscard]]
	inline Vec3<F> xyz() const noexcept {
		return Vec3<F>(x, y, z);
	}

	[[nodiscard]]
	inline Vec3<F> xyw() const noexcept {
		return Vec3<F>(x, y, w);
	}

	[[nodiscard]]
	inline Vec3<F> xzw() const noexcept {
		return Vec3<F>(x, z, w);
	}

	[[nodiscard]]
	inline Vec3<F> yzw() const noexcept {
		return Vec3<F>(y, z, w);
	}

	[[nodiscard]]
	inline F magnitude() const noexcept {
		using std::fma; // For correct overload resolution
		using std::sqrt; // For correct overload resolution
		return sqrt(fma(x, x, fma(y, y, fma(z, z, w * w))));
	}

	[[nodiscard]]
	inline F length() const noexcept {
		return this->magnitude();
	}

	[[nodiscard]]
	inline Vec4 normalize() const noexcept {
		const F m = magnitude();
		return Vec4(x / m, y / m, z / m, w / m);
	}

	[[nodiscard]]
	inline Vec4 floor() const {
		using std::floor; // For correct overload resolution
		return Vec4(floor(x), floor(y), floor(z), floor(w));
	}

	[[nodiscard]]
	inline Vec4 ceil() const {
		using std::ceil; // For correct overload resolution
		return Vec4(ceil(x), ceil(y), ceil(z), ceil(w));
	}

	[[nodiscard]]
	inline Vec4 round() const {
		using std::round; // For correct overload resolution
		return Vec4(round(x), round(y), round(z), round(w));
	}

	[[nodiscard]]
	inline Vec4 fract() const {
		return Vec4(mt::fract(x), mt::fract(y), mt::fract(z), mt::fract(w));
	}

	[[nodiscard]]
	inline Vec4 trunc() const {
		using std::trunc; // For correct overload resolution
		return Vec4(trunc(x), trunc(y), trunc(z), trunc(w));
	}

	[[nodiscard]]
	inline Vec4 abs() const {
		using std::abs; // For correct overload resolution
		return Vec4(abs(x), abs(y), abs(z), abs(w));
	}

	[[nodiscard]]
	inline Vec4 sqrt() const {
		using std::sqrt; // For correct overload resolution
		return Vec4(sqrt(x), sqrt(y), sqrt(z), sqrt(w));
	}

	[[nodiscard]]
	inline Vec4 exp() const {
		using std::exp; // For correct overload resolution
		return Vec4(exp(x), exp(y), exp(z), exp(w));
	}

	[[nodiscard]]
	inline Vec4 exp2() const {
		using std::exp2; // For correct overload resolution
		return Vec4(exp2(x), exp2(y), exp2(z), exp2(w));
	}

	[[nodiscard]]
	inline Vec4 log() const {
		using std::log; // For correct overload resolution
		return Vec4(log(x), log(y), log(z), log(w));
	}

	[[nodiscard]]
	inline Vec4 log2() const {
		using std::log2; // For correct overload resolution
		return Vec4(log2(x), log2(y), log2(z), log2(w));
	}

	[[nodiscard]]
	inline Vec4 radians() const {
		return Vec4(mt::radians(x), mt::radians(y), mt::radians(z), mt::radians(w));
	}

	[[nodiscard]]
	inline Vec4 degrees() const {
		return Vec4(mt::degrees(x), mt::degrees(y), mt::degrees(z), mt::degrees(w));
	}

	[[nodiscard]]
	inline Vec4 sin() const {
		using std::sin; // For correct overload resolution
		return Vec4(sin(x), sin(y), sin(z), sin(w));
	}

	[[nodiscard]]
	inline Vec4 cos() const {
		using std::cos; // For correct overload resolution
		return Vec4(cos(x), cos(y), cos(z), cos(w));
	}

	[[nodiscard]]
	inline Vec4 tan() const {
		using std::tan; // For correct overload resolution
		return Vec4(tan(x), tan(y), tan(z), tan(w));
	}

	[[nodiscard]]
	inline Vec4 asin() const {
		using std::asin; // For correct overload resolution
		return Vec4(asin(x), asin(y), asin(z), asin(w));
	}

	[[nodiscard]]
	inline Vec4 acos() const {
		using std::acos; // For correct overload resolution
		return Vec4(acos(x), acos(y), acos(z), acos(w));
	}

	[[nodiscard]]
	inline Vec4 atan() const {
		using std::atan; // For correct overload resolution
		return Vec4(atan(x), atan(y), atan(z), atan(w));
	}

	[[nodiscard]]
	inline Vec4 clamp() const {
		return clamp(F(0), F(1));
	}

	[[nodiscard]]
	inline Vec4 clamp(const Vec4& min_value, const Vec4& max_value) const {
		return Vec4(mt::clamp(x, min_value.x, max_value.x), mt::clamp(y, min_value.y, max_value.y), mt::clamp(z, min_value.z, max_value.z), mt::clamp(w, min_value.w, max_value.w));
	}

	template <typename F2, typename F3>
	[[nodiscard]]
	inline Vec4 clamp(F2 min_value, F3 max_value) const {
		return clamp(Vec4(static_cast<F>(min_value)), Vec4(static_cast<F>(max_value)));
	}
};

template <typename F> using vec4 = Vec4<F>;

template <typename F>
[[nodiscard]]
inline Vec4<F> operator+(Vec4<F> lhs, const Vec4<F>& rhs) noexcept {
	lhs += rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline Vec4<F> operator-(Vec4<F> lhs, const Vec4<F>& rhs) noexcept {
	lhs -= rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline Vec4<F> operator*(Vec4<F> lhs, const Vec4<F>& rhs) noexcept {
	lhs *= rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline Vec4<F> operator/(Vec4<F> lhs, const Vec4<F>& rhs) noexcept {
	lhs /= rhs;
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec4<F> operator+(Vec4<F> lhs, F2 rhs) noexcept {
	lhs += static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec4<F> operator-(Vec4<F> lhs, F2 rhs) noexcept {
	lhs -= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec4<F> operator*(Vec4<F> lhs, F2 rhs) noexcept {
	lhs *= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec4<F> operator/(Vec4<F> lhs, F2 rhs) noexcept {
	lhs /= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec4<F> operator+(F2 lhs, Vec4<F> rhs) noexcept {
	return rhs + static_cast<F>(lhs);
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec4<F> operator-(F2 lhs, Vec4<F> rhs) noexcept {
	return -rhs + static_cast<F>(lhs);
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec4<F> operator*(F2 lhs, Vec4<F> rhs) noexcept {
	return rhs * static_cast<F>(lhs);
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec4<F> operator/(F2 lhs, Vec4<F> rhs) noexcept {
	return Vec4<F>(static_cast<F>(lhs) / rhs.x, static_cast<F>(lhs) / rhs.y, static_cast<F>(lhs) / rhs.z, static_cast<F>(lhs) / rhs.w);
}

// Dot Product of two vectors (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w)
template <typename F>
[[nodiscard]]
inline F dot(const Vec4<F>& a, const Vec4<F>& b) noexcept {
	using std::fma; // For correct overload resolution
	return fma(a.x, b.x, fma(a.y, b.y, fma(a.z, b.z, a.w * b.w)));
}

/**************************************************************************************************
 * SIMD refract for Vec4. Scalar-bool generic refract is in common.h.
 * This stays here because zeroing the result needs per-component mask/blend.
 * ************************************************************************************************/
template <typename F, typename Eta>
[[nodiscard]]
inline Vec4<F> refract(const Vec4<F>& incident, const Vec4<F>& normal, Eta eta) noexcept
    requires mt::SimdMath<F> && mt::SimdCompareOps<F>
{
	using std::sqrt; // For correct overload resolution
	const F eta_value = F(eta);
	const F n_dot_i = dot(incident, normal);
	const F zero = F(0);
	const F one = F(1);
	const F k = one - eta_value * eta_value * (one - n_dot_i * n_dot_i);
	const F sqrt_k = sqrt(if_less(k, zero, zero, k));
	const Vec4<F> refracted = eta_value * incident - (eta_value * n_dot_i + sqrt_k) * normal;
	return Vec4<F>(if_less(k, zero, zero, refracted.x), if_less(k, zero, zero, refracted.y), if_less(k, zero, zero, refracted.z), if_less(k, zero, zero, refracted.w));
}

template <typename F>
[[nodiscard]]
inline Vec4<F> normalize(const Vec4<F>& v) noexcept {
	return v.normalize();
}

template <typename F>
[[nodiscard]]
inline F distance(const Vec4<F>& a, const Vec4<F>& b) {
	using std::fma; // For correct overload resolution
	using std::sqrt; // For correct overload resolution
	const F dx = b.x - a.x;
	const F dy = b.y - a.y;
	const F dz = b.z - a.z;
	const F dw = b.w - a.w;
	return sqrt(fma(dx, dx, fma(dy, dy, fma(dz, dz, dw * dw))));
}

template <typename F>
[[nodiscard]]
inline Vec4<F> floor(const Vec4<F>& a) {
	return a.floor();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> ceil(const Vec4<F>& a) {
	return a.ceil();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> round(const Vec4<F>& a) {
	return a.round();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> fract(const Vec4<F>& a) {
	return a.fract();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> trunc(const Vec4<F>& a) {
	return a.trunc();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> abs(const Vec4<F>& a) {
	return a.abs();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> sqrt(const Vec4<F>& a) {
	return a.sqrt();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> min(const Vec4<F>& a, const Vec4<F>& b) {
	using std::min; // For correct overload resolution
	return Vec4<F>(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec4<F> min(const Vec4<F>& a, F2 b) {
	using std::min; // For correct overload resolution
	const F value = static_cast<F>(b);
	return Vec4<F>(min(a.x, value), min(a.y, value), min(a.z, value), min(a.w, value));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec4<F> min(F2 a, const Vec4<F>& b) {
	return min(b, static_cast<F>(a));
}

template <typename F>
[[nodiscard]]
inline Vec4<F> max(const Vec4<F>& a, const Vec4<F>& b) {
	using std::max; // For correct overload resolution
	return Vec4<F>(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec4<F> max(const Vec4<F>& a, F2 b) {
	using std::max; // For correct overload resolution
	const F value = static_cast<F>(b);
	return Vec4<F>(max(a.x, value), max(a.y, value), max(a.z, value), max(a.w, value));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec4<F> max(F2 a, const Vec4<F>& b) {
	return max(b, static_cast<F>(a));
}

template <typename F>
[[nodiscard]]
inline Vec4<F> clamp(const Vec4<F>& value) {
	return value.clamp();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> clamp(const Vec4<F>& value, const Vec4<F>& min_value, const Vec4<F>& max_value) {
	return value.clamp(min_value, max_value);
}

template <typename F, typename F2, typename F3>
[[nodiscard]]
inline Vec4<F> clamp(const Vec4<F>& value, F2 min_value, F3 max_value) {
	return value.clamp(min_value, max_value);
}

template <typename F>
[[nodiscard]]
inline Vec4<F> step(const Vec4<F>& edge, const Vec4<F>& value) noexcept {
	return Vec4<F>(mt::step(edge.x, value.x), mt::step(edge.y, value.y), mt::step(edge.z, value.z), mt::step(edge.w, value.w));
}

template <typename F, typename Edge>
[[nodiscard]]
inline Vec4<F> step(Edge edge, const Vec4<F>& value) noexcept
    requires(!std::same_as<std::remove_cvref_t<Edge>, Vec4<F>>)
{
	const F edge_component = static_cast<F>(edge);
	return Vec4<F>(mt::step(edge_component, value.x), mt::step(edge_component, value.y), mt::step(edge_component, value.z), mt::step(edge_component, value.w));
}

template <typename F, typename Value>
[[nodiscard]]
inline Vec4<F> step(const Vec4<F>& edge, Value value) noexcept
    requires(!std::same_as<std::remove_cvref_t<Value>, Vec4<F>>)
{
	const F value_component = static_cast<F>(value);
	return Vec4<F>(mt::step(edge.x, value_component), mt::step(edge.y, value_component), mt::step(edge.z, value_component), mt::step(edge.w, value_component));
}

template <typename F>
[[nodiscard]]
inline Vec4<F> fma(const Vec4<F>& a, const Vec4<F>& b, const Vec4<F>& c) {
	using std::fma; // For correct overload resolution
	return Vec4<F>(fma(a.x, b.x, c.x), fma(a.y, b.y, c.y), fma(a.z, b.z, c.z), fma(a.w, b.w, c.w));
}

template <typename F>
[[nodiscard]]
inline Vec4<F> fms(const Vec4<F>& a, const Vec4<F>& b, const Vec4<F>& c) {
	return Vec4<F>(fms(a.x, b.x, c.x), fms(a.y, b.y, c.y), fms(a.z, b.z, c.z), fms(a.w, b.w, c.w));
}

template <typename F>
[[nodiscard]]
inline Vec4<F> fnma(const Vec4<F>& a, const Vec4<F>& b, const Vec4<F>& c) {
	return Vec4<F>(fnma(a.x, b.x, c.x), fnma(a.y, b.y, c.y), fnma(a.z, b.z, c.z), fnma(a.w, b.w, c.w));
}

template <typename F>
[[nodiscard]]
inline Vec4<F> fnms(const Vec4<F>& a, const Vec4<F>& b, const Vec4<F>& c) {
	return Vec4<F>(fnms(a.x, b.x, c.x), fnms(a.y, b.y, c.y), fnms(a.z, b.z, c.z), fnms(a.w, b.w, c.w));
}

template <typename F>
[[nodiscard]]
inline Vec4<F> pow(const Vec4<F>& a, const Vec4<F>& b) {
	using std::pow; // For correct overload resolution
	return Vec4<F>(pow(a.x, b.x), pow(a.y, b.y), pow(a.z, b.z), pow(a.w, b.w));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec4<F> pow(const Vec4<F>& a, F2 b) {
	using std::pow; // For correct overload resolution
	const F value = static_cast<F>(b);
	return Vec4<F>(pow(a.x, value), pow(a.y, value), pow(a.z, value), pow(a.w, value));
}

template <typename F>
[[nodiscard]]
inline Vec4<F> exp(const Vec4<F>& a) {
	return a.exp();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> exp2(const Vec4<F>& a) {
	return a.exp2();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> log(const Vec4<F>& a) {
	return a.log();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> log2(const Vec4<F>& a) {
	return a.log2();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> sin(const Vec4<F>& a) {
	return a.sin();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> cos(const Vec4<F>& a) {
	return a.cos();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> tan(const Vec4<F>& a) {
	return a.tan();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> asin(const Vec4<F>& a) {
	return a.asin();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> acos(const Vec4<F>& a) {
	return a.acos();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> atan(const Vec4<F>& a) {
	return a.atan();
}

template <typename F>
[[nodiscard]]
inline Vec4<F> atan2(const Vec4<F>& y, const Vec4<F>& x) {
	using std::atan2; // For correct overload resolution
	return Vec4<F>(atan2(y.x, x.x), atan2(y.y, x.y), atan2(y.z, x.z), atan2(y.w, x.w));
}

template <typename F>
[[nodiscard]]
inline F length(const Vec4<F>& a) {
	return a.magnitude();
}

template <typename F>
[[nodiscard]]
inline F magnitude(const Vec4<F>& a) {
	return a.magnitude();
}

} // namespace mt
