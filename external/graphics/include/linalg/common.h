/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Type:           common

Description:    Shared linalg helpers for scalar, vector-like, and Maths Town SIMD-aware code.

Concepts:       BoolEqualityComparable BoolOrderComparable

Scalar Helpers: fract fms fnma fnms clamp clamp_to_int

Angle Helpers: degrees radians

Range:          rescale_from_01 rescale_to_01 rescale remap wrap smoothstep step

Vector Helpers: reflect refract mix lerp
*********************************************************************************************************/

#pragma once

#include <algorithm>
#include <concepts>
#include <cmath>
#include <numbers>
#include <type_traits>

namespace mt::linalg_detail {

template <typename T>
concept BoolEqualityComparable = requires(const T& a, const T& b) {
	{ a == b } -> std::convertible_to<bool>;
};

template <typename T>
concept BoolOrderComparable = requires(const T& a, const T& b) {
	{ a < b } -> std::convertible_to<bool>;
	{ a > b } -> std::convertible_to<bool>;
};

template <typename T, typename = void> struct ConstantCastType {
	using type = T;
};

template <typename T> struct ConstantCastType<T, std::void_t<typename T::F>> {
	using type = typename T::F;
};

template <typename T, typename = void> struct WrapConstantType {
	using type = typename ConstantCastType<T>::type;
};

template <typename T> struct WrapConstantType<T, std::void_t<typename T::ComponentType>> {
	using type = typename ConstantCastType<typename T::ComponentType>::type;
};

}

namespace mt {

/**************************************************************************************************
 * Fractional part of a value.
 * ************************************************************************************************/
template <typename F>
[[nodiscard]]
constexpr inline F fract(F value) {
	using std::floor; // For correct overload resolution
	return value - floor(value);
}

/**************************************************************************************************
 * Fused multiply-subtract: a * b - c.
 * ************************************************************************************************/
template <std::floating_point F>
[[nodiscard]]
inline F fms(F a, F b, F c) noexcept {
	return std::fma(a, b, -c);
}

/**************************************************************************************************
 * Fused negative multiply-add: -(a * b) + c.
 * ************************************************************************************************/
template <std::floating_point F>
[[nodiscard]]
inline F fnma(F a, F b, F c) noexcept {
	return std::fma(-a, b, c);
}

/**************************************************************************************************
 * Fused negative multiply-subtract: -(a * b) - c.
 * ************************************************************************************************/
template <std::floating_point F>
[[nodiscard]]
inline F fnms(F a, F b, F c) noexcept {
	return std::fma(-a, b, -c);
}

/**************************************************************************************************
 * Converts degrees to radians.
 * ************************************************************************************************/
template <typename T>
[[nodiscard]]
inline T radians(const T& degrees_value) noexcept {
	using Constant = typename linalg_detail::WrapConstantType<T>::type;
	const Constant factor = static_cast<Constant>(std::numbers::pi_v<long double> / 180.0L);
	return degrees_value * factor;
}

/**************************************************************************************************
 * Converts radians to degrees.
 * ************************************************************************************************/
template <typename T>
[[nodiscard]]
inline T degrees(const T& radians_value) noexcept {
	using Constant = typename linalg_detail::WrapConstantType<T>::type;
	const Constant factor = static_cast<Constant>(180.0L / std::numbers::pi_v<long double>);
	return radians_value * factor;
}

/**************************************************************************************************
 * Reflect function. Normal must be normalized.
 * Generic path for vector-like types with dot() and vector/scalar arithmetic.
 * ************************************************************************************************/
template <class T>
[[nodiscard]]
inline T reflect(const T& incident, const T& normal) noexcept
    requires requires(const T& incident, const T& normal) {
	    // Formula needs dot() plus vector-scalar multiply/subtract.
	    dot(normal, incident);
	    incident - normal * dot(normal, incident);
    }
{
	const auto n_dot_i = dot(normal, incident);
	return incident - normal * (decltype(n_dot_i)(2) * n_dot_i);
}

/**************************************************************************************************
 * Refract function. Incident and normal must be normalized.
 * Scalar-bool path; SIMD vec overloads live in vec*.h because they need mask/blend semantics.
 * ************************************************************************************************/
template <class T, typename F>
[[nodiscard]]
inline T refract(const T& incident, const T& normal, F eta) noexcept
    requires requires(const T& incident, const T& normal) {
	    // Excludes SIMD dot results; those use the vec-specific mask/blend overloads.
	    { dot(incident, normal) } -> linalg_detail::BoolOrderComparable;
	    incident - normal * dot(normal, incident);
    }
{
	using std::sqrt; // For correct overload resolution
	const auto n_dot_i = dot(incident, normal);
	const auto one = decltype(n_dot_i)(1);
	const auto k = one - eta * eta * (one - n_dot_i * n_dot_i);
	if (k < decltype(k)(0)) {
		return T{};
	}
	return eta * incident - (eta * n_dot_i + sqrt(k)) * normal;
}

/**************************************************************************************************
 * Rescales a value in the range 0..1 to a new range. No clamping.
 * ************************************************************************************************/
template <typename F>
[[nodiscard]]
inline F rescale_from_01(F value, F new_min, F new_max) noexcept {
	return value * (new_max - new_min) + new_min;
}

/**************************************************************************************************
 * Rescales a value in the range 0..1 to a new range using scalar bounds. No clamping.
 * ************************************************************************************************/
template <typename T, std::floating_point F>
[[nodiscard]]
inline T rescale_from_01(const T& value, F new_min, F new_max) noexcept
    requires(!std::same_as<T, F>)
{
	using Constant = typename linalg_detail::WrapConstantType<T>::type;
	const Constant min_component = static_cast<Constant>(new_min);
	const Constant max_component = static_cast<Constant>(new_max);
	return value * (max_component - min_component) + min_component;
}

/**************************************************************************************************
 * Rescales a value to the range 0..1. No clamping.
 * ************************************************************************************************/
template <typename F>
[[nodiscard]]
inline F rescale_to_01(F value, F old_min, F old_max) noexcept {
	return (value - old_min) / (old_max - old_min);
}

/**************************************************************************************************
 * Rescales a value to the range 0..1 using scalar bounds. No clamping.
 * ************************************************************************************************/
template <typename T, std::floating_point F>
[[nodiscard]]
inline T rescale_to_01(const T& value, F old_min, F old_max) noexcept
    requires(!std::same_as<T, F>)
{
	using Constant = typename linalg_detail::WrapConstantType<T>::type;
	const Constant min_component = static_cast<Constant>(old_min);
	const Constant max_component = static_cast<Constant>(old_max);
	return (value - min_component) / (max_component - min_component);
}

/**************************************************************************************************
 * Rescales a value from one range to another. No clamping.
 * ************************************************************************************************/
template <typename F>
[[nodiscard]]
inline F rescale(F value, F old_min, F old_max, F new_min, F new_max) noexcept {
	return rescale_from_01(rescale_to_01(value, old_min, old_max), new_min, new_max);
}

/**************************************************************************************************
 * Rescales a value from one range to another using scalar bounds. No clamping.
 * ************************************************************************************************/
template <typename T, std::floating_point F>
[[nodiscard]]
inline T rescale(const T& value, F old_min, F old_max, F new_min, F new_max) noexcept
    requires(!std::same_as<T, F>)
{
	using Constant = typename linalg_detail::WrapConstantType<T>::type;
	const Constant old_min_component = static_cast<Constant>(old_min);
	const Constant old_max_component = static_cast<Constant>(old_max);
	const Constant new_min_component = static_cast<Constant>(new_min);
	const Constant new_max_component = static_cast<Constant>(new_max);
	const Constant old_range = old_max_component - old_min_component;
	const Constant new_range = new_max_component - new_min_component;
	return (value - old_min_component) / old_range * new_range + new_min_component;
}

/**************************************************************************************************
 * Rescales a value from one range to another. No clamping. (alias of rescale)
 * ************************************************************************************************/
template <typename F>
[[nodiscard]]
inline F remap(F value, F old_min, F old_max, F new_min, F new_max) noexcept {
	return rescale(value, old_min, old_max, new_min, new_max);
}

/**************************************************************************************************
 * Rescales a value from one range to another using scalar bounds. No clamping. (alias of rescale)
 * ************************************************************************************************/
template <typename T, std::floating_point F>
[[nodiscard]]
inline T remap(const T& value, F old_min, F old_max, F new_min, F new_max) noexcept
    requires(!std::same_as<T, F>)
{
	using Constant = typename linalg_detail::WrapConstantType<T>::type;
	const Constant old_min_component = static_cast<Constant>(old_min);
	const Constant old_max_component = static_cast<Constant>(old_max);
	const Constant new_min_component = static_cast<Constant>(new_min);
	const Constant new_max_component = static_cast<Constant>(new_max);
	const Constant old_range = old_max_component - old_min_component;
	const Constant new_range = new_max_component - new_min_component;
	return (value - old_min_component) / old_range * new_range + new_min_component;
}

/**************************************************************************************************
 * Wraps a value to the half-open range min_value..max_value.
 * ************************************************************************************************/
template <typename F>
[[nodiscard]]
inline F wrap(F value, F min_value, F max_value) noexcept {
	using std::floor; // For correct overload resolution
	const F range = max_value - min_value;
	return value - range * floor((value - min_value) / range);
}

/**************************************************************************************************
 * Wraps a value to the half-open range min_value..max_value using scalar bounds.
 * ************************************************************************************************/
template <typename T, std::floating_point F>
[[nodiscard]]
inline T wrap(const T& value, F min_value, F max_value) noexcept
    requires(!std::same_as<T, F>)
{
	using std::floor; // For correct overload resolution
	using Constant = typename linalg_detail::WrapConstantType<T>::type;
	const Constant min_component = static_cast<Constant>(min_value);
	const Constant max_component = static_cast<Constant>(max_value);
	const Constant range = max_component - min_component;
	return value - range * floor((value - min_component) / range);
}

/**************************************************************************************************
 * Clamp a value.
 * Uses scalar comparisons when available; otherwise relies on ADL min/max, such as SIMD overloads.
 * ************************************************************************************************/
template <typename F>
[[nodiscard]]
inline F clamp(F value, F min_value, F max_value) noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	return min(max(value, min_value), max_value);
}

/**************************************************************************************************
 * Clamp a value to the range 0..1.
 * ************************************************************************************************/
template <typename F>
[[nodiscard]]
inline F clamp(F value) noexcept {
	return clamp(value, F(0), F(1));
}

/**************************************************************************************************
 * Convert to int and clamp.
 * ************************************************************************************************/
template <typename F>
[[nodiscard]]
inline int clamp_to_int(F value, int min_value, int max_value) noexcept
    requires std::convertible_to<F, double>
{
	const int rounded = static_cast<int>(std::round(value));
	return clamp(rounded, min_value, max_value);
}

/**************************************************************************************************
 * Hermite interpolation between 0 and 1 when value is in the range edge0..edge1.
 * ************************************************************************************************/
template <typename F>
[[nodiscard]]
inline F smoothstep(F edge0, F edge1, F value) noexcept {
	const auto t = clamp((value - edge0) / (edge1 - edge0));
	return t * t * (F(3) - F(2) * t);
}

/**************************************************************************************************
 * Hermite interpolation overload for float edge values with non-float value types.
 * ************************************************************************************************/
template <typename F>
[[nodiscard]]
inline F smoothstep(float edge0f, float edge1f, F value) noexcept
    requires(!std::same_as<F, float>)
{
	return smoothstep(F(edge0f), F(edge1f), value);
}

/**************************************************************************************************
 * Step function.
 * Scalar paths return 0 when value < edge, otherwise 1. SIMD paths rely on ADL if_less().
 * ************************************************************************************************/
template <typename F>
[[nodiscard]]
inline F step(F edge, F value) noexcept {
	if constexpr (linalg_detail::BoolOrderComparable<F>) {
		return (value < edge) ? F(0) : F(1);
	} else {
		return if_less(value, edge, F(0), F(1));
	}
}

/**************************************************************************************************
 * Mix: Linear interpolation (same as lerp)
 * ************************************************************************************************/
template <class T, typename F>
[[nodiscard]]
inline T mix(T v1, T v2, F weight) noexcept {
	return v2 * weight + (F(1) - weight) * v1;
}

/**************************************************************************************************
 * Linear interpolation (same as mix)
 * ************************************************************************************************/
template <class T, typename F>
[[nodiscard]]
inline T lerp(T v1, T v2, F weight) noexcept {
	return mix(v1, v2, weight);
}

} // namespace mt
