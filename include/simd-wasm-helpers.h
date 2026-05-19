#pragma once

#include "environment.h"

#if MT_SIMD_ARCH_WASM && defined(__wasm_simd128__)

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <wasm_simd128.h>

namespace mt::simd_wasm_detail {

template <typename Scalar, std::size_t N>
inline std::array<Scalar, N> to_array(v128_t v) noexcept {
	std::array<Scalar, N> lanes{};
	wasm_v128_store(lanes.data(), v);
	return lanes;
}

template <typename Scalar, std::size_t N>
inline v128_t from_array(const std::array<Scalar, N>& lanes) noexcept {
	return wasm_v128_load(lanes.data());
}

template <typename Scalar, std::size_t N>
inline Scalar lane_get(v128_t v, int i) noexcept {
	return to_array<Scalar, N>(v)[i];
}

template <typename Scalar, std::size_t N>
inline v128_t lane_set(v128_t v, int i, Scalar value) noexcept {
	auto lanes = to_array<Scalar, N>(v);
	lanes[static_cast<std::size_t>(i)] = value;
	return from_array<Scalar, N>(lanes);
}

template <typename Scalar, std::size_t N>
inline v128_t make_sequential(Scalar first) noexcept {
	std::array<Scalar, N> lanes{};
	for (std::size_t i = 0; i < N; ++i) {
		lanes[i] = static_cast<Scalar>(first + static_cast<Scalar>(i));
	}
	return from_array<Scalar, N>(lanes);
}

template <typename Scalar>
inline v128_t splat(Scalar value) noexcept {
	if constexpr (std::is_same_v<Scalar, float>) {
		return wasm_f32x4_splat(value);
	} else if constexpr (std::is_same_v<Scalar, double>) {
		return wasm_f64x2_splat(value);
	} else if constexpr (sizeof(Scalar) == 4) {
		uint32_t lane = static_cast<uint32_t>(value);
		return wasm_v128_load32_splat(&lane);
	} else {
		uint64_t lane = static_cast<uint64_t>(value);
		return wasm_v128_load64_splat(&lane);
	}
}

template <typename Scalar, std::size_t N, typename Fn>
inline v128_t map_unary(v128_t value, Fn fn) noexcept {
	auto lanes = to_array<Scalar, N>(value);
	for (auto& lane : lanes) {
		lane = static_cast<Scalar>(fn(lane));
	}
	return from_array<Scalar, N>(lanes);
}

template <typename Scalar, std::size_t N, typename Fn>
inline v128_t map_binary(v128_t lhs, v128_t rhs, Fn fn) noexcept {
	auto a = to_array<Scalar, N>(lhs);
	auto b = to_array<Scalar, N>(rhs);
	for (std::size_t i = 0; i < N; ++i) {
		a[i] = static_cast<Scalar>(fn(a[i], b[i]));
	}
	return from_array<Scalar, N>(a);
}

template <typename Scalar, std::size_t N, typename Fn>
inline v128_t map_ternary(v128_t a, v128_t b, v128_t c, Fn fn) noexcept {
	auto va = to_array<Scalar, N>(a);
	auto vb = to_array<Scalar, N>(b);
	auto vc = to_array<Scalar, N>(c);
	for (std::size_t i = 0; i < N; ++i) {
		va[i] = static_cast<Scalar>(fn(va[i], vb[i], vc[i]));
	}
	return from_array<Scalar, N>(va);
}

template <typename Scalar, std::size_t N>
inline v128_t mask_from_bools(const std::array<bool, N>& lanes) noexcept {
	std::array<Scalar, N> mask{};
	for (std::size_t i = 0; i < N; ++i) {
		mask[i] = lanes[i] ? static_cast<Scalar>(~Scalar(0)) : static_cast<Scalar>(0);
	}
	return from_array<Scalar, N>(mask);
}

template <typename Scalar, std::size_t N, typename Pred>
inline v128_t compare_scalar(v128_t lhs, v128_t rhs, Pred pred) noexcept {
	auto a = to_array<Scalar, N>(lhs);
	auto b = to_array<Scalar, N>(rhs);
	std::array<bool, N> lanes{};
	for (std::size_t i = 0; i < N; ++i) {
		lanes[i] = pred(a[i], b[i]);
	}
	using MaskScalar = std::conditional_t<(sizeof(Scalar) == 4), uint32_t, uint64_t>;
	return mask_from_bools<MaskScalar, N>(lanes);
}

template <typename Scalar, std::size_t N, typename Pred>
inline v128_t mask_from_predicate(v128_t value, Pred pred) noexcept {
	auto lanes = to_array<Scalar, N>(value);
	std::array<bool, N> result{};
	for (std::size_t i = 0; i < N; ++i) {
		result[i] = pred(lanes[i]);
	}
	using MaskScalar = std::conditional_t<(sizeof(Scalar) == 4), uint32_t, uint64_t>;
	return mask_from_bools<MaskScalar, N>(result);
}

template <typename Scalar, std::size_t N>
inline v128_t clamp_vector(v128_t value, v128_t lo, v128_t hi) noexcept {
	auto v = to_array<Scalar, N>(value);
	auto l = to_array<Scalar, N>(lo);
	auto h = to_array<Scalar, N>(hi);
	for (std::size_t i = 0; i < N; ++i) {
		if (v[i] < l[i]) {
			v[i] = l[i];
		}
		if (v[i] > h[i]) {
			v[i] = h[i];
		}
	}
	return from_array<Scalar, N>(v);
}

template <typename Scalar, std::size_t N>
inline v128_t bitcast_from_array(const std::array<Scalar, N>& lanes) noexcept {
	return from_array<Scalar, N>(lanes);
}

template <typename ToScalar, std::size_t N>
inline std::array<ToScalar, N> bitcast_to_array(v128_t value) noexcept {
	std::array<ToScalar, N> lanes{};
	wasm_v128_store(lanes.data(), value);
	return lanes;
}

} // namespace mt::simd_wasm_detail

#endif
