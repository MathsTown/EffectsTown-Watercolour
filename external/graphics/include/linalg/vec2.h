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

Type:           Vec2
                (aliased to vec2, for easy glsl porting)

Description:    A 2-vector that supports floating point types and types implementing the concepts from the
                Maths Town SIMD library. All member functions, except constructors and self-assignment
                operators, are const to allow chaining.

Operators:      == - + - * / += -= *= /= with vector and scalar forms

Member Functions:
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

namespace mt {

/**************************************************************************************************
 * A 2-Vector (x,y)
 *
 * ************************************************************************************************/
template <typename F> struct Vec2 {
	using ComponentType = F;

	F x{};
	F y{};

	Vec2() = default;

	Vec2(F v) noexcept : x(v), y(v) { }

	Vec2(F x1, F y1) noexcept : x(x1), y(y1) { }

	[[nodiscard]]
	bool operator==(const Vec2& rhs) const noexcept
	    requires linalg_detail::BoolEqualityComparable<F>
	{
		return (x == rhs.x) && (y == rhs.y);
	}

	[[nodiscard]]
	Vec2 operator-() const noexcept {
		return Vec2(-x, -y);
	}

	Vec2& operator+=(const Vec2& rhs) noexcept {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	Vec2& operator-=(const Vec2& rhs) noexcept {
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	Vec2& operator*=(const Vec2& rhs) noexcept {
		x *= rhs.x;
		y *= rhs.y;
		return *this;
	}

	Vec2& operator/=(const Vec2& rhs) noexcept {
		x /= rhs.x;
		y /= rhs.y;
		return *this;
	}

	Vec2& operator+=(const F rhs) noexcept {
		x += rhs;
		y += rhs;
		return *this;
	}

	Vec2& operator-=(const F rhs) noexcept {
		x -= rhs;
		y -= rhs;
		return *this;
	}

	Vec2& operator*=(const F rhs) noexcept {
		x *= rhs;
		y *= rhs;
		return *this;
	}

	Vec2& operator/=(const F rhs) noexcept {
		x /= rhs;
		y /= rhs;
		return *this;
	}

	// Specilisations for SIMD Types
	// Vec2& operator+=(const float rhs) noexcept requires(mt::SimdFloat<F>) { x += F(rhs); y += F(rhs); return *this; }
	// Vec2& operator-=(const float rhs) noexcept requires(mt::SimdFloat<F>) { x -= F(rhs); y -= F(rhs); return *this; }
	// Vec2& operator*=(const float rhs) noexcept requires(mt::SimdFloat<F>) { x *= F(rhs); y *= F(rhs); return *this; }
	// Vec2& operator/=(const float rhs) noexcept requires(mt::SimdFloat<F>) { x /= F(rhs); y /= F(rhs); return *this; }

	// Vec2& operator+=(const Vec2<float>& rhs) noexcept requires(mt::SimdFloat32<F>) { x += F(rhs.x); y += F(rhs.y); return *this; }

	// Calculate the magnitude (length) of the vector.
	[[nodiscard]]
	inline F magnitude() const noexcept {
		using std::fma; // For correct overload resolution
		using std::sqrt; // For correct overload resolution
		return sqrt(fma(x, x, y * y));
	}

	// Calculate the length of the vector (Note: This is an alias for magnitude()).
	[[nodiscard]]
	inline F length() const noexcept {
		return this->magnitude();
	}

	[[nodiscard]]
	inline Vec2<F> normalize() const noexcept {
		const F m = magnitude();
		return Vec2(x / m, y / m);
	}

	[[nodiscard]]
	inline Vec2 floor() const {
		using std::floor; // For correct overload resolution
		return Vec2(floor(x), floor(y));
	}

	[[nodiscard]]
	inline Vec2 ceil() const {
		using std::ceil; // For correct overload resolution
		return Vec2(ceil(x), ceil(y));
	}

	[[nodiscard]]
	inline Vec2 round() const {
		using std::round; // For correct overload resolution
		return Vec2(round(x), round(y));
	}

	[[nodiscard]]
	inline Vec2 fract() const {
		return Vec2(mt::fract(x), mt::fract(y));
	}

	[[nodiscard]]
	inline Vec2 trunc() const {
		using std::trunc; // For correct overload resolution
		return Vec2(trunc(x), trunc(y));
	}

	[[nodiscard]]
	inline Vec2 abs() const {
		using std::abs; // For correct overload resolution
		return Vec2(abs(x), abs(y));
	}

	[[nodiscard]]
	inline Vec2 sqrt() const {
		using std::sqrt; // For correct overload resolution
		return Vec2(sqrt(x), sqrt(y));
	}

	[[nodiscard]]
	inline Vec2 exp() const {
		using std::exp; // For correct overload resolution
		return Vec2(exp(x), exp(y));
	}

	[[nodiscard]]
	inline Vec2 exp2() const {
		using std::exp2; // For correct overload resolution
		return Vec2(exp2(x), exp2(y));
	}

	[[nodiscard]]
	inline Vec2 log() const {
		using std::log; // For correct overload resolution
		return Vec2(log(x), log(y));
	}

	[[nodiscard]]
	inline Vec2 log2() const {
		using std::log2; // For correct overload resolution
		return Vec2(log2(x), log2(y));
	}

	[[nodiscard]]
	inline Vec2 radians() const {
		return Vec2(mt::radians(x), mt::radians(y));
	}

	[[nodiscard]]
	inline Vec2 degrees() const {
		return Vec2(mt::degrees(x), mt::degrees(y));
	}

	[[nodiscard]]
	inline Vec2 sin() const {
		using std::sin; // For correct overload resolution
		return Vec2(sin(x), sin(y));
	}

	[[nodiscard]]
	inline Vec2 cos() const {
		using std::cos; // For correct overload resolution
		return Vec2(cos(x), cos(y));
	}

	[[nodiscard]]
	inline Vec2 tan() const {
		using std::tan; // For correct overload resolution
		return Vec2(tan(x), tan(y));
	}

	[[nodiscard]]
	inline Vec2 asin() const {
		using std::asin; // For correct overload resolution
		return Vec2(asin(x), asin(y));
	}

	[[nodiscard]]
	inline Vec2 acos() const {
		using std::acos; // For correct overload resolution
		return Vec2(acos(x), acos(y));
	}

	[[nodiscard]]
	inline Vec2 atan() const {
		using std::atan; // For correct overload resolution
		return Vec2(atan(x), atan(y));
	}

	[[nodiscard]]
	inline Vec2 clamp() const {
		return clamp(F(0), F(1));
	}

	[[nodiscard]]
	inline Vec2 clamp(const Vec2& min_value, const Vec2& max_value) const {
		return Vec2(mt::clamp(x, min_value.x, max_value.x), mt::clamp(y, min_value.y, max_value.y));
	}

	template <typename F2, typename F3>
	[[nodiscard]]
	inline Vec2 clamp(F2 min_value, F3 max_value) const {
		return clamp(Vec2(static_cast<F>(min_value)), Vec2(static_cast<F>(max_value)));
	}
};

template <typename F> using vec2 = Vec2<F>;

template <typename F>
[[nodiscard]]
inline Vec2<F> operator+(Vec2<F> lhs, const Vec2<F>& rhs) noexcept {
	lhs += rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline Vec2<F> operator-(Vec2<F> lhs, const Vec2<F>& rhs) noexcept {
	lhs -= rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline Vec2<F> operator*(Vec2<F> lhs, const Vec2<F>& rhs) noexcept {
	lhs *= rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline Vec2<F> operator/(Vec2<F> lhs, const Vec2<F>& rhs) noexcept {
	lhs /= rhs;
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec2<F> operator+(Vec2<F> lhs, F2 rhs) noexcept {
	lhs += static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec2<F> operator-(Vec2<F> lhs, F2 rhs) noexcept {
	lhs -= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec2<F> operator*(Vec2<F> lhs, F2 rhs) noexcept {
	lhs *= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec2<F> operator/(Vec2<F> lhs, F2 rhs) noexcept {
	lhs /= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec2<F> operator+(F2 lhs, Vec2<F> rhs) noexcept {
	return rhs + static_cast<F>(lhs);
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec2<F> operator-(F2 lhs, Vec2<F> rhs) noexcept {
	return -rhs + static_cast<F>(lhs);
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec2<F> operator*(F2 lhs, Vec2<F> rhs) noexcept {
	return rhs * static_cast<F>(lhs);
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec2<F> operator/(F2 lhs, Vec2<F> rhs) noexcept {
	return Vec2(static_cast<F>(lhs) / rhs.x, static_cast<F>(lhs) / rhs.y);
}

// Dot Product of two vectors (a.x * b.x + a.y * b.y)
template <typename F>
[[nodiscard]]
inline F dot(const Vec2<F>& a, const Vec2<F>& b) noexcept {
	using std::fma; // For correct overload resolution
	return fma(a.x, b.x, a.y * b.y);
}

/**************************************************************************************************
 * SIMD refract for Vec2. Scalar-bool generic refract is in common.h.
 * This stays here because zeroing the result needs per-component mask/blend.
 * ************************************************************************************************/
template <typename F, typename Eta>
[[nodiscard]]
inline Vec2<F> refract(const Vec2<F>& incident, const Vec2<F>& normal, Eta eta) noexcept
    requires mt::SimdMath<F> && mt::SimdCompareOps<F>
{
	using std::sqrt; // For correct overload resolution
	const F eta_value = F(eta);
	const F n_dot_i = dot(incident, normal);
	const F zero = F(0);
	const F one = F(1);
	const F k = one - eta_value * eta_value * (one - n_dot_i * n_dot_i);
	const F sqrt_k = sqrt(if_less(k, zero, zero, k));
	const Vec2<F> refracted = eta_value * incident - (eta_value * n_dot_i + sqrt_k) * normal;
	return Vec2<F>(if_less(k, zero, zero, refracted.x), if_less(k, zero, zero, refracted.y));
}

template <typename F>
[[nodiscard]]
inline Vec2<F> normalize(const Vec2<F>& v) noexcept {
	return v.normalize();
}

template <typename F>
[[nodiscard]]
inline F distance(const Vec2<F>& a, const Vec2<F>& b) {
	using std::fma; // For correct overload resolution
	using std::sqrt; // For correct overload resolution
	const F dx = b.x - a.x;
	const F dy = b.y - a.y;
	return sqrt(fma(dx, dx, dy * dy));
}

template <typename F>
[[nodiscard]]
inline Vec2<F> floor(const Vec2<F>& a) {
	return a.floor();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> ceil(const Vec2<F>& a) {
	return a.ceil();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> round(const Vec2<F>& a) {
	return a.round();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> fract(const Vec2<F>& a) {
	return a.fract();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> trunc(const Vec2<F>& a) {
	return a.trunc();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> abs(const Vec2<F>& a) {
	return a.abs();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> sqrt(const Vec2<F>& a) {
	return a.sqrt();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> min(const Vec2<F>& a, const Vec2<F>& b) {
	using std::min; // For correct overload resolution
	return Vec2<F>(min(a.x, b.x), min(a.y, b.y));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec2<F> min(const Vec2<F>& a, F2 b) {
	using std::min; // For correct overload resolution
	const F value = static_cast<F>(b);
	return Vec2<F>(min(a.x, value), min(a.y, value));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec2<F> min(F2 a, const Vec2<F>& b) {
	return min(b, static_cast<F>(a));
}

template <typename F>
[[nodiscard]]
inline Vec2<F> max(const Vec2<F>& a, const Vec2<F>& b) {
	using std::max; // For correct overload resolution
	return Vec2<F>(max(a.x, b.x), max(a.y, b.y));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec2<F> max(const Vec2<F>& a, F2 b) {
	using std::max; // For correct overload resolution
	const F value = static_cast<F>(b);
	return Vec2<F>(max(a.x, value), max(a.y, value));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec2<F> max(F2 a, const Vec2<F>& b) {
	return max(b, static_cast<F>(a));
}

template <typename F>
[[nodiscard]]
inline Vec2<F> clamp(const Vec2<F>& value) {
	return value.clamp();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> clamp(const Vec2<F>& value, const Vec2<F>& min_value, const Vec2<F>& max_value) {
	return value.clamp(min_value, max_value);
}

template <typename F, typename F2, typename F3>
[[nodiscard]]
inline Vec2<F> clamp(const Vec2<F>& value, F2 min_value, F3 max_value) {
	return value.clamp(min_value, max_value);
}

template <typename F>
[[nodiscard]]
inline Vec2<F> step(const Vec2<F>& edge, const Vec2<F>& value) noexcept {
	return Vec2<F>(mt::step(edge.x, value.x), mt::step(edge.y, value.y));
}

template <typename F, typename Edge>
[[nodiscard]]
inline Vec2<F> step(Edge edge, const Vec2<F>& value) noexcept
    requires(!std::same_as<std::remove_cvref_t<Edge>, Vec2<F>>)
{
	const F edge_component = static_cast<F>(edge);
	return Vec2<F>(mt::step(edge_component, value.x), mt::step(edge_component, value.y));
}

template <typename F, typename Value>
[[nodiscard]]
inline Vec2<F> step(const Vec2<F>& edge, Value value) noexcept
    requires(!std::same_as<std::remove_cvref_t<Value>, Vec2<F>>)
{
	const F value_component = static_cast<F>(value);
	return Vec2<F>(mt::step(edge.x, value_component), mt::step(edge.y, value_component));
}

template <typename F>
[[nodiscard]]
inline Vec2<F> fma(const Vec2<F>& a, const Vec2<F>& b, const Vec2<F>& c) {
	using std::fma; // For correct overload resolution
	return Vec2<F>(fma(a.x, b.x, c.x), fma(a.y, b.y, c.y));
}

template <typename F>
[[nodiscard]]
inline Vec2<F> fms(const Vec2<F>& a, const Vec2<F>& b, const Vec2<F>& c) {
	return Vec2<F>(fms(a.x, b.x, c.x), fms(a.y, b.y, c.y));
}

template <typename F>
[[nodiscard]]
inline Vec2<F> fnma(const Vec2<F>& a, const Vec2<F>& b, const Vec2<F>& c) {
	return Vec2<F>(fnma(a.x, b.x, c.x), fnma(a.y, b.y, c.y));
}

template <typename F>
[[nodiscard]]
inline Vec2<F> fnms(const Vec2<F>& a, const Vec2<F>& b, const Vec2<F>& c) {
	return Vec2<F>(fnms(a.x, b.x, c.x), fnms(a.y, b.y, c.y));
}

template <typename F>
[[nodiscard]]
inline Vec2<F> pow(const Vec2<F>& a, const Vec2<F>& b) {
	using std::pow; // For correct overload resolution
	return Vec2<F>(pow(a.x, b.x), pow(a.y, b.y));
}

template <typename F, typename F2>
[[nodiscard]]
inline Vec2<F> pow(const Vec2<F>& a, F2 b) {
	using std::pow; // For correct overload resolution
	const F value = static_cast<F>(b);
	return Vec2<F>(pow(a.x, value), pow(a.y, value));
}

template <typename F>
[[nodiscard]]
inline Vec2<F> exp(const Vec2<F>& a) {
	return a.exp();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> exp2(const Vec2<F>& a) {
	return a.exp2();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> log(const Vec2<F>& a) {
	return a.log();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> log2(const Vec2<F>& a) {
	return a.log2();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> sin(const Vec2<F>& a) {
	return a.sin();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> cos(const Vec2<F>& a) {
	return a.cos();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> tan(const Vec2<F>& a) {
	return a.tan();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> asin(const Vec2<F>& a) {
	return a.asin();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> acos(const Vec2<F>& a) {
	return a.acos();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> atan(const Vec2<F>& a) {
	return a.atan();
}

template <typename F>
[[nodiscard]]
inline Vec2<F> atan2(const Vec2<F>& y, const Vec2<F>& x) {
	using std::atan2; // For correct overload resolution
	return Vec2<F>(atan2(y.x, x.x), atan2(y.y, x.y));
}

template <typename F>
[[nodiscard]]
inline F length(const Vec2<F>& a) {
	return a.magnitude();
}

template <typename F>
[[nodiscard]]
inline F magnitude(const Vec2<F>& a) {
	return a.magnitude();
}

} // namespace mt
