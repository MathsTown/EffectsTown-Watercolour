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

Type:           Vec3
                (aliased to vec3, for easy glsl porting)

Description:    A 3-vector that supports floating point types and types implementing the concepts from the
                Maths Town SIMD library. All member functions, except constructors and self-assignment
                operators, are const to allow chaining.

Operators:      == - + - * / += -= *= /= with vector and scalar forms

Member Functions:
                Swizzle:              xy yz xz
                Length:               magnitude length normalize
                Component Math:       floor ceil round fract trunc abs sqrt exp exp2 log log2
                Trig:                 degrees radians sin cos tan asin acos atan
                Range:                clamp clamp(min,max)

Free Functions:
                Operators:            + - * /
                Geometry:             dot cross refract normalize distance length magnitude
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

namespace mt {

/**************************************************************************************************
 * A 3-Vector (x,y,z)
 * ************************************************************************************************/
template <typename F> struct Vec3 {
	using ComponentType = F;

	F x{};
	F y{};
	F z{};

	Vec3() = default;

	Vec3(F v) noexcept : x(v), y(v), z(v) { }

	Vec3(F x1, F y1, F z1) noexcept : x(x1), y(y1), z(z1) { }

	Vec3(const Vec2<F>& v, F z1) noexcept : x(v.x), y(v.y), z(z1) { }

	Vec3(F x1, const Vec2<F>& v) noexcept : x(x1), y(v.x), z(v.y) { }

	[[nodiscard]]
	bool operator==(const Vec3& rhs) const noexcept
	    requires linalg_detail::BoolEqualityComparable<F>
	{
		return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
	}

	[[nodiscard]]
	Vec3 operator-() const noexcept {
		return Vec3(-x, -y, -z);
	}

	Vec3& operator+=(const Vec3& rhs) noexcept {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	Vec3& operator-=(const Vec3& rhs) noexcept {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	Vec3& operator*=(const Vec3& rhs) noexcept {
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		return *this;
	}

	Vec3& operator/=(const Vec3& rhs) noexcept {
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;
		return *this;
	}

	Vec3& operator+=(const F rhs) noexcept {
		x += rhs;
		y += rhs;
		z += rhs;
		return *this;
	}

	Vec3& operator-=(const F rhs) noexcept {
		x -= rhs;
		y -= rhs;
		z -= rhs;
		return *this;
	}

	Vec3& operator*=(const F rhs) noexcept {
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}

	Vec3& operator/=(const F rhs) noexcept {
		x /= rhs;
		y /= rhs;
		z /= rhs;
		return *this;
	}

	[[nodiscard]]
	inline F magnitude() const noexcept {
		using std::fma; // For correct overload resolution
		using std::sqrt; // For correct overload resolution
		return sqrt(fma(x, x, fma(y, y, z * z)));
	}

	[[nodiscard]]
	inline F length() const noexcept {
		return this->magnitude();
	}

	[[nodiscard]]
	inline Vec3 normalize() const noexcept {
		const F m = magnitude();
		return Vec3(x / m, y / m, z / m);
	}

	[[nodiscard]]
	inline Vec3 floor() const {
		using std::floor; // For correct overload resolution
		return Vec3(floor(x), floor(y), floor(z));
	}

	[[nodiscard]]
	inline Vec3 ceil() const {
		using std::ceil; // For correct overload resolution
		return Vec3(ceil(x), ceil(y), ceil(z));
	}

	[[nodiscard]]
	inline Vec3 round() const {
		using std::round; // For correct overload resolution
		return Vec3(round(x), round(y), round(z));
	}

	[[nodiscard]]
	inline Vec3 fract() const {
		return Vec3(mt::fract(x), mt::fract(y), mt::fract(z));
	}

	[[nodiscard]]
	inline Vec3 trunc() const {
		using std::trunc; // For correct overload resolution
		return Vec3(trunc(x), trunc(y), trunc(z));
	}

	[[nodiscard]]
	inline Vec3 abs() const {
		using std::abs; // For correct overload resolution
		return Vec3(abs(x), abs(y), abs(z));
	}

	[[nodiscard]]
	inline Vec3 sqrt() const {
		using std::sqrt; // For correct overload resolution
		return Vec3(sqrt(x), sqrt(y), sqrt(z));
	}

	[[nodiscard]]
	inline Vec3 exp() const {
		using std::exp; // For correct overload resolution
		return Vec3(exp(x), exp(y), exp(z));
	}

	[[nodiscard]]
	inline Vec3 exp2() const {
		using std::exp2; // For correct overload resolution
		return Vec3(exp2(x), exp2(y), exp2(z));
	}

	[[nodiscard]]
	inline Vec3 log() const {
		using std::log; // For correct overload resolution
		return Vec3(log(x), log(y), log(z));
	}

	[[nodiscard]]
	inline Vec3 log2() const {
		using std::log2; // For correct overload resolution
		return Vec3(log2(x), log2(y), log2(z));
	}

	[[nodiscard]]
	inline Vec3 radians() const {
		return Vec3(mt::radians(x), mt::radians(y), mt::radians(z));
	}

	[[nodiscard]]
	inline Vec3 degrees() const {
		return Vec3(mt::degrees(x), mt::degrees(y), mt::degrees(z));
	}

	[[nodiscard]]
	inline Vec3 sin() const {
		using std::sin; // For correct overload resolution
		return Vec3(sin(x), sin(y), sin(z));
	}

	[[nodiscard]]
	inline Vec3 cos() const {
		using std::cos; // For correct overload resolution
		return Vec3(cos(x), cos(y), cos(z));
	}

	[[nodiscard]]
	inline Vec3 tan() const {
		using std::tan; // For correct overload resolution
		return Vec3(tan(x), tan(y), tan(z));
	}

	[[nodiscard]]
	inline Vec3 asin() const {
		using std::asin; // For correct overload resolution
		return Vec3(asin(x), asin(y), asin(z));
	}

	[[nodiscard]]
	inline Vec3 acos() const {
		using std::acos; // For correct overload resolution
		return Vec3(acos(x), acos(y), acos(z));
	}

	[[nodiscard]]
	inline Vec3 atan() const {
		using std::atan; // For correct overload resolution
		return Vec3(atan(x), atan(y), atan(z));
	}

	[[nodiscard]]
	inline Vec3 clamp() const {
		return clamp(F(0), F(1));
	}

	[[nodiscard]]
	inline Vec3 clamp(const Vec3& min_value, const Vec3& max_value) const {
		return Vec3(mt::clamp(x, min_value.x, max_value.x), mt::clamp(y, min_value.y, max_value.y), mt::clamp(z, min_value.z, max_value.z));
	}

	template <typename F2, typename F3>
	[[nodiscard]]
	inline Vec3 clamp(F2 min_value, F3 max_value) const {
		return clamp(Vec3(static_cast<F>(min_value)), Vec3(static_cast<F>(max_value)));
	}

	[[nodiscard]]
	inline Vec2<F> xy() const noexcept {
		return Vec2<F>(x, y);
	}

	[[nodiscard]]
	inline Vec2<F> yz() const noexcept {
		return Vec2<F>(y, z);
	}

	[[nodiscard]]
	inline Vec2<F> xz() const noexcept {
		return Vec2<F>(x, z);
	}
};

template <typename F> using vec3 = Vec3<F>;

template <typename F>
[[nodiscard]]
inline Vec3<F> operator+(Vec3<F> lhs, const Vec3<F>& rhs) noexcept {
	lhs += rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline Vec3<F> operator-(Vec3<F> lhs, const Vec3<F>& rhs) noexcept {
	lhs -= rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline Vec3<F> operator*(Vec3<F> lhs, const Vec3<F>& rhs) noexcept {
	lhs *= rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline Vec3<F> operator/(Vec3<F> lhs, const Vec3<F>& rhs) noexcept {
	lhs /= rhs;
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec3<F> operator+(Vec3<F> lhs, F2 rhs) noexcept {
	lhs += static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec3<F> operator-(Vec3<F> lhs, F2 rhs) noexcept {
	lhs -= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec3<F> operator*(Vec3<F> lhs, F2 rhs) noexcept {
	lhs *= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec3<F> operator/(Vec3<F> lhs, F2 rhs) noexcept {
	lhs /= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec3<F> operator+(F2 lhs, Vec3<F> rhs) noexcept {
	return rhs + static_cast<F>(lhs);
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec3<F> operator-(F2 lhs, Vec3<F> rhs) noexcept {
	return -rhs + static_cast<F>(lhs);
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec3<F> operator*(F2 lhs, Vec3<F> rhs) noexcept {
	return rhs * static_cast<F>(lhs);
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec3<F> operator/(F2 lhs, Vec3<F> rhs) noexcept {
	return Vec3<F>(static_cast<F>(lhs) / rhs.x, static_cast<F>(lhs) / rhs.y, static_cast<F>(lhs) / rhs.z);
}

// Dot Product of two vectors (a.x * b.x + a.y * b.y + a.z * b.z)
template <typename F>
[[nodiscard]]
inline F dot(const Vec3<F>& a, const Vec3<F>& b) noexcept {
	using std::fma; // For correct overload resolution
	return fma(a.x, b.x, fma(a.y, b.y, a.z * b.z));
}

template <typename F>
[[nodiscard]]
inline Vec3<F> cross(const Vec3<F>& a, const Vec3<F>& b) noexcept {
	return Vec3<F>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

/**************************************************************************************************
 * SIMD refract for Vec3. Scalar-bool generic refract is in common.h.
 * This stays here because zeroing the result needs per-component mask/blend.
 * ************************************************************************************************/
template <typename F, typename Eta>
[[nodiscard]]
inline Vec3<F> refract(const Vec3<F>& incident, const Vec3<F>& normal, Eta eta) noexcept
    requires mt::SimdMath<F> && mt::SimdCompareOps<F>
{
	using std::sqrt; // For correct overload resolution
	const F eta_value = F(eta);
	const F n_dot_i = dot(incident, normal);
	const F zero = F(0);
	const F one = F(1);
	const F k = one - eta_value * eta_value * (one - n_dot_i * n_dot_i);
	const F sqrt_k = sqrt(if_less(k, zero, zero, k));
	const Vec3<F> refracted = eta_value * incident - (eta_value * n_dot_i + sqrt_k) * normal;
	return Vec3<F>(if_less(k, zero, zero, refracted.x), if_less(k, zero, zero, refracted.y), if_less(k, zero, zero, refracted.z));
}

template <typename F>
[[nodiscard]]
inline Vec3<F> normalize(const Vec3<F>& v) noexcept {
	return v.normalize();
}

template <typename F>
[[nodiscard]]
inline F distance(const Vec3<F>& a, const Vec3<F>& b) {
	using std::fma; // For correct overload resolution
	using std::sqrt; // For correct overload resolution
	const F dx = b.x - a.x;
	const F dy = b.y - a.y;
	const F dz = b.z - a.z;
	return sqrt(fma(dx, dx, fma(dy, dy, dz * dz)));
}

template <typename F>
[[nodiscard]]
inline Vec3<F> floor(const Vec3<F>& a) {
	return a.floor();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> ceil(const Vec3<F>& a) {
	return a.ceil();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> round(const Vec3<F>& a) {
	return a.round();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> fract(const Vec3<F>& a) {
	return a.fract();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> trunc(const Vec3<F>& a) {
	return a.trunc();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> abs(const Vec3<F>& a) {
	return a.abs();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> sqrt(const Vec3<F>& a) {
	return a.sqrt();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> min(const Vec3<F>& a, const Vec3<F>& b) {
	using std::min; // For correct overload resolution
	return Vec3<F>(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec3<F> min(const Vec3<F>& a, F2 b) {
	using std::min; // For correct overload resolution
	const F value = static_cast<F>(b);
	return Vec3<F>(min(a.x, value), min(a.y, value), min(a.z, value));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec3<F> min(F2 a, const Vec3<F>& b) {
	return min(b, static_cast<F>(a));
}

template <typename F>
[[nodiscard]]
inline Vec3<F> max(const Vec3<F>& a, const Vec3<F>& b) {
	using std::max; // For correct overload resolution
	return Vec3<F>(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec3<F> max(const Vec3<F>& a, F2 b) {
	using std::max; // For correct overload resolution
	const F value = static_cast<F>(b);
	return Vec3<F>(max(a.x, value), max(a.y, value), max(a.z, value));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec3<F> max(F2 a, const Vec3<F>& b) {
	return max(b, static_cast<F>(a));
}

template <typename F>
[[nodiscard]]
inline Vec3<F> clamp(const Vec3<F>& value) {
	return value.clamp();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> clamp(const Vec3<F>& value, const Vec3<F>& min_value, const Vec3<F>& max_value) {
	return value.clamp(min_value, max_value);
}

template <typename F, typename F2, typename F3>
[[nodiscard]]
inline Vec3<F> clamp(const Vec3<F>& value, F2 min_value, F3 max_value) {
	return value.clamp(min_value, max_value);
}

template <typename F>
[[nodiscard]]
inline Vec3<F> step(const Vec3<F>& edge, const Vec3<F>& value) noexcept {
	return Vec3<F>(mt::step(edge.x, value.x), mt::step(edge.y, value.y), mt::step(edge.z, value.z));
}

template <typename F, typename Edge>
[[nodiscard]]
inline Vec3<F> step(Edge edge, const Vec3<F>& value) noexcept
    requires(!std::same_as<std::remove_cvref_t<Edge>, Vec3<F>>)
{
	const F edge_component = static_cast<F>(edge);
	return Vec3<F>(mt::step(edge_component, value.x), mt::step(edge_component, value.y), mt::step(edge_component, value.z));
}

template <typename F, typename Value>
[[nodiscard]]
inline Vec3<F> step(const Vec3<F>& edge, Value value) noexcept
    requires(!std::same_as<std::remove_cvref_t<Value>, Vec3<F>>)
{
	const F value_component = static_cast<F>(value);
	return Vec3<F>(mt::step(edge.x, value_component), mt::step(edge.y, value_component), mt::step(edge.z, value_component));
}

template <typename F>
[[nodiscard]]
inline Vec3<F> fma(const Vec3<F>& a, const Vec3<F>& b, const Vec3<F>& c) {
	using std::fma; // For correct overload resolution
	return Vec3<F>(fma(a.x, b.x, c.x), fma(a.y, b.y, c.y), fma(a.z, b.z, c.z));
}

template <typename F>
[[nodiscard]]
inline Vec3<F> fms(const Vec3<F>& a, const Vec3<F>& b, const Vec3<F>& c) {
	return Vec3<F>(fms(a.x, b.x, c.x), fms(a.y, b.y, c.y), fms(a.z, b.z, c.z));
}

template <typename F>
[[nodiscard]]
inline Vec3<F> fnma(const Vec3<F>& a, const Vec3<F>& b, const Vec3<F>& c) {
	return Vec3<F>(fnma(a.x, b.x, c.x), fnma(a.y, b.y, c.y), fnma(a.z, b.z, c.z));
}

template <typename F>
[[nodiscard]]
inline Vec3<F> fnms(const Vec3<F>& a, const Vec3<F>& b, const Vec3<F>& c) {
	return Vec3<F>(fnms(a.x, b.x, c.x), fnms(a.y, b.y, c.y), fnms(a.z, b.z, c.z));
}

template <typename F>
[[nodiscard]]
inline Vec3<F> pow(const Vec3<F>& a, const Vec3<F>& b) {
	using std::pow; // For correct overload resolution
	return Vec3<F>(pow(a.x, b.x), pow(a.y, b.y), pow(a.z, b.z));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec3<F> pow(const Vec3<F>& a, F2 b) {
	using std::pow; // For correct overload resolution
	const F value = static_cast<F>(b);
	return Vec3<F>(pow(a.x, value), pow(a.y, value), pow(a.z, value));
}

template <typename F>
[[nodiscard]]
inline Vec3<F> exp(const Vec3<F>& a) {
	return a.exp();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> exp2(const Vec3<F>& a) {
	return a.exp2();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> log(const Vec3<F>& a) {
	return a.log();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> log2(const Vec3<F>& a) {
	return a.log2();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> sin(const Vec3<F>& a) {
	return a.sin();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> cos(const Vec3<F>& a) {
	return a.cos();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> tan(const Vec3<F>& a) {
	return a.tan();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> asin(const Vec3<F>& a) {
	return a.asin();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> acos(const Vec3<F>& a) {
	return a.acos();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> atan(const Vec3<F>& a) {
	return a.atan();
}

template <typename F>
[[nodiscard]]
inline Vec3<F> atan2(const Vec3<F>& y, const Vec3<F>& x) {
	using std::atan2; // For correct overload resolution
	return Vec3<F>(atan2(y.x, x.x), atan2(y.y, x.y), atan2(y.z, x.z));
}

template <typename F>
[[nodiscard]]
inline F length(const Vec3<F>& a) {
	return a.magnitude();
}

template <typename F>
[[nodiscard]]
inline F magnitude(const Vec3<F>& a) {
	return a.magnitude();
}

} // namespace mt
