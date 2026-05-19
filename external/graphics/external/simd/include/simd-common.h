#pragma once
/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Shared compare, blend, and branchless conditional helpers.
*********************************************************************************************************/

#include <cmath>
#include <concepts>

#include "simd-concepts.h"

namespace mt {

template <typename T>
concept GenericComparable = requires (const T a, const T b) {
	{ a == b } -> std::convertible_to<bool>;
	{ a < b } -> std::convertible_to<bool>;
	{ a <= b } -> std::convertible_to<bool>;
	{ a > b } -> std::convertible_to<bool>;
	{ a >= b } -> std::convertible_to<bool>;
};

template <typename T>
concept GenericBlendable = requires (const T if_false, const T if_true, bool mask) {
	{ mask ? if_true : if_false } -> std::convertible_to<T>;
};

template <GenericComparable T>
inline static bool compare_equal(const T a, const T b) noexcept(noexcept(a == b)) {
	return a == b;
}

template <GenericComparable T>
inline static bool compare_less(const T a, const T b) noexcept(noexcept(a < b)) {
	return a < b;
}

template <GenericComparable T>
inline static bool compare_less_equal(const T a, const T b) noexcept(noexcept(a <= b)) {
	return a <= b;
}

template <GenericComparable T>
inline static bool compare_greater(const T a, const T b) noexcept(noexcept(a > b)) {
	return a > b;
}

template <GenericComparable T>
inline static bool compare_greater_equal(const T a, const T b) noexcept(noexcept(a >= b)) {
	return a >= b;
}

template <GenericBlendable T>
[[nodiscard("Value Calculated and not used (blend)")]]
inline static T blend(const T if_false, const T if_true, bool mask) noexcept(noexcept(mask ? if_true : if_false)) {
	return mask ? if_true : if_false;
}

template <typename T>
requires requires (const T value_a, const T value_b, const T if_true, const T if_false) {
	compare_equal(value_a, value_b);
	{ blend(if_false, if_true, compare_equal(value_a, value_b)) } -> std::same_as<T>;
}
[[nodiscard("Value Calculated and not used (if_equal)")]]
inline static T if_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept(noexcept(blend(if_false, if_true, compare_equal(value_a, value_b)))) {
	return blend(if_false, if_true, compare_equal(value_a, value_b));
}

template <typename T>
requires requires (const T value_a, const T value_b, const T if_true, const T if_false) {
	compare_less(value_a, value_b);
	{ blend(if_false, if_true, compare_less(value_a, value_b)) } -> std::same_as<T>;
}
[[nodiscard("Value Calculated and not used (if_less)")]]
inline static T if_less(const T value_a, const T value_b, const T if_true, const T if_false) noexcept(noexcept(blend(if_false, if_true, compare_less(value_a, value_b)))) {
	return blend(if_false, if_true, compare_less(value_a, value_b));
}

template <typename T>
requires requires (const T value_a, const T value_b, const T if_true, const T if_false) {
	compare_less_equal(value_a, value_b);
	{ blend(if_false, if_true, compare_less_equal(value_a, value_b)) } -> std::same_as<T>;
}
[[nodiscard("Value Calculated and not used (if_less_equal)")]]
inline static T if_less_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept(noexcept(blend(if_false, if_true, compare_less_equal(value_a, value_b)))) {
	return blend(if_false, if_true, compare_less_equal(value_a, value_b));
}

template <typename T>
requires requires (const T value_a, const T value_b, const T if_true, const T if_false) {
	compare_greater(value_a, value_b);
	{ blend(if_false, if_true, compare_greater(value_a, value_b)) } -> std::same_as<T>;
}
[[nodiscard("Value Calculated and not used (if_greater)")]]
inline static T if_greater(const T value_a, const T value_b, const T if_true, const T if_false) noexcept(noexcept(blend(if_false, if_true, compare_greater(value_a, value_b)))) {
	return blend(if_false, if_true, compare_greater(value_a, value_b));
}

template <typename T>
requires requires (const T value_a, const T value_b, const T if_true, const T if_false) {
	compare_greater_equal(value_a, value_b);
	{ blend(if_false, if_true, compare_greater_equal(value_a, value_b)) } -> std::same_as<T>;
}
[[nodiscard("Value Calculated and not used (if_greater_equal)")]]
inline static T if_greater_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept(noexcept(blend(if_false, if_true, compare_greater_equal(value_a, value_b)))) {
	return blend(if_false, if_true, compare_greater_equal(value_a, value_b));
}

template <std::floating_point T>
inline static bool isnan(const T value) noexcept {
	return std::isnan(value);
}

template <typename T>
requires requires (const T value_a, const T if_true, const T if_false) {
	isnan(value_a);
	{ blend(if_false, if_true, isnan(value_a)) } -> std::same_as<T>;
}
[[nodiscard("Value Calculated and not used (if_nan)")]]
inline static T if_nan(const T value_a, const T if_true, const T if_false) noexcept(noexcept(blend(if_false, if_true, isnan(value_a)))) {
	return blend(if_false, if_true, isnan(value_a));
}

} // namespace mt
