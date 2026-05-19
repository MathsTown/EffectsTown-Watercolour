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

Common colour concepts and free functions.

Public API:

Concepts:
- ColourComponent
- RgbaColour
- XyzaColour
- HsvaColour
- HslaColour
- LabaColour
- LchaColour

Interpolation:
- mix()
- mix_longest_path()
- lerp()

RGB blends:
- blend_normal()
- blend_multiply()
- blend_screen()
- blend_overlay()
- blend_darken()
- blend_lighten()
- blend_color_dodge()
- blend_color_burn()
- blend_hard_light()
- blend_soft_light()
- blend_difference()
- blend_exclusion()
- blend_hue()
- blend_saturation()
- blend_color()
- blend_luminosity()
- blend_linear_burn()
- blend_linear_dodge()
- blend_add()
- blend_linear_light()
- blend_vivid_light()
- blend_pin_light()
- blend_hard_mix()
- blend_subtract()
- blend_divide()

Matte operations:
- matte_stencil_alpha()
- matte_stencil_luma()
- matte_silhouette_alpha()
- matte_silhouette_luma()
- matte_alpha_add()

*********************************************************************************************************/
#pragma once

#include <algorithm>
#include <concepts>
#include <cmath>
#include <limits>
#include "simd-common.h"
#include "simd-concepts.h"

namespace mt {

/**************************************************************************************************
 * Concept for colour component types.
 * ************************************************************************************************/
// Accept scalar floating point and Maths Town SIMD floating point component types.
template <typename F>
concept ColourComponent = std::floating_point<F> || SimdFloat<F>;

/**************************************************************************************************
 * Concept for RGBA colour types.
 * ************************************************************************************************/
// Accept colour types with red, green, blue, alpha, and common RGB gamut operations.
template <typename C>
concept RgbaColour = requires(C colour) {
	typename C::ComponentType;
	requires ColourComponent<typename C::ComponentType>;
	{ colour.red } -> std::same_as<typename C::ComponentType&>;
	{ colour.green } -> std::same_as<typename C::ComponentType&>;
	{ colour.blue } -> std::same_as<typename C::ComponentType&>;
	{ colour.alpha } -> std::same_as<typename C::ComponentType&>;
	{ C(colour.red, colour.green, colour.blue, colour.alpha) } -> std::same_as<C>;
	{ colour.clamp() } -> std::same_as<C>;
	{ colour.clip() } -> std::same_as<C>;
	{ colour.clip_rgb() } -> std::same_as<C>;
	{ colour.fit_gamut() } -> std::same_as<C>;
};

/**************************************************************************************************
 * Concept for XYZA colour types.
 * ************************************************************************************************/
// Accept colour types with x, y, z, and alpha components.
template <typename C>
concept XyzaColour = requires(C colour) {
	typename C::ComponentType;
	requires ColourComponent<typename C::ComponentType>;
	{ colour.x } -> std::same_as<typename C::ComponentType&>;
	{ colour.y } -> std::same_as<typename C::ComponentType&>;
	{ colour.z } -> std::same_as<typename C::ComponentType&>;
	{ colour.alpha } -> std::same_as<typename C::ComponentType&>;
	{ C(colour.x, colour.y, colour.z, colour.alpha) } -> std::same_as<C>;
};

/**************************************************************************************************
 * Concept for HSVA colour types.
 * ************************************************************************************************/
// Accept colour types with hue, saturation, value, and alpha components.
template <typename C>
concept HsvaColour = requires(C colour) {
	typename C::ComponentType;
	requires ColourComponent<typename C::ComponentType>;
	{ colour.hue } -> std::same_as<typename C::ComponentType&>;
	{ colour.saturation } -> std::same_as<typename C::ComponentType&>;
	{ colour.value } -> std::same_as<typename C::ComponentType&>;
	{ colour.alpha } -> std::same_as<typename C::ComponentType&>;
	{ C(colour.hue, colour.saturation, colour.value, colour.alpha) } -> std::same_as<C>;
};

/**************************************************************************************************
 * Concept for HSLA colour types.
 * ************************************************************************************************/
// Accept colour types with hue, saturation, lightness, and alpha components.
template <typename C>
concept HslaColour = requires(C colour) {
	typename C::ComponentType;
	requires ColourComponent<typename C::ComponentType>;
	{ colour.hue } -> std::same_as<typename C::ComponentType&>;
	{ colour.saturation } -> std::same_as<typename C::ComponentType&>;
	{ colour.lightness } -> std::same_as<typename C::ComponentType&>;
	{ colour.alpha } -> std::same_as<typename C::ComponentType&>;
	{ C(colour.hue, colour.saturation, colour.lightness, colour.alpha) } -> std::same_as<C>;
};

/**************************************************************************************************
 * Concept for LABA colour types.
 * ************************************************************************************************/
// Accept colour types with lightness, a, b, and alpha components.
template <typename C>
concept LabaColour = requires(C colour) {
	typename C::ComponentType;
	requires ColourComponent<typename C::ComponentType>;
	{ colour.lightness } -> std::same_as<typename C::ComponentType&>;
	{ colour.a } -> std::same_as<typename C::ComponentType&>;
	{ colour.b } -> std::same_as<typename C::ComponentType&>;
	{ colour.alpha } -> std::same_as<typename C::ComponentType&>;
	{ C(colour.lightness, colour.a, colour.b, colour.alpha) } -> std::same_as<C>;
};

/**************************************************************************************************
 * Concept for LCHA colour types.
 * ************************************************************************************************/
// Accept colour types with lightness, chroma, hue, and alpha components.
template <typename C>
concept LchaColour = requires(C colour) {
	typename C::ComponentType;
	requires ColourComponent<typename C::ComponentType>;
	{ colour.lightness } -> std::same_as<typename C::ComponentType&>;
	{ colour.chroma } -> std::same_as<typename C::ComponentType&>;
	{ colour.hue } -> std::same_as<typename C::ComponentType&>;
	{ colour.alpha } -> std::same_as<typename C::ComponentType&>;
	{ C(colour.lightness, colour.chroma, colour.hue, colour.alpha) } -> std::same_as<C>;
};

namespace detail {

	/**************************************************************************************************
	 * Return a floating point colour component with invalid values replaced by finite canonical values.
	 * Replaces NaN with 0.0
	 * Replaces -0.0 with 0.0
	 * Replaces inf with std::numeric_limits<F>::max()
	 * Replaces -inf with std::numeric_limits<F>::lowest()
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F sanitise_component(F value) noexcept {
		using std::max; // For correct overload resolution
		using std::min; // For correct overload resolution
		const F zero = F(0);
		const F min_value = []() noexcept {
			if constexpr (std::floating_point<F>) {
				return std::numeric_limits<F>::lowest();
			} else {
				using Lane = typename F::F;
				return F(std::numeric_limits<Lane>::lowest());
			}
		}();
		const F max_value = []() noexcept {
			if constexpr (std::floating_point<F>) {
				return std::numeric_limits<F>::max();
			} else {
				using Lane = typename F::F;
				return F(std::numeric_limits<Lane>::max());
			}
		}();

		value = if_nan(value, zero, value);
		value = max(min_value, min(value, max_value));
		return if_equal(value, zero, zero, value);
	}

	/**************************************************************************************************
	 * Wrap a hue measured as a 0..1 turn.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F wrap_hue_turn(F hue) noexcept {
		using std::floor; // For correct overload resolution
		const F one = F(1);
		hue = hue - floor(hue);
		// Floating-point rounding can leave the wrapped hue at exactly 1.0.
		return if_greater_equal(hue, one, hue - one, hue);
	}

	template <ColourComponent F>
	[[nodiscard]]
	inline F clamp_unit(F value) noexcept {
		using std::max; // For correct overload resolution
		using std::min; // For correct overload resolution
		const F zero = F(0);
		const F one = F(1);
		return max(zero, min(value, one));
	}

	/**************************************************************************************************
	 * Composite a blended top RGB value over bottom using straight-alpha source-over maths.
	 *
	 * The public blend functions calculate the blend-mode RGB explicitly and then call this function
	 * to apply top alpha and layer opacity consistently. The blend RGB is the value produced where
	 * bottom already exists; where bottom is transparent, top contributes its own RGB as normal.
	 * ************************************************************************************************/
	template <RgbaColour C>
	[[nodiscard]]
	inline C composite_blended_rgb(const C& bottom, const C& top, typename C::ComponentType blended_red, typename C::ComponentType blended_green, typename C::ComponentType blended_blue, typename C::ComponentType opacity) noexcept {
		using F = typename C::ComponentType;

		const F zero = F(0);
		const F one = F(1);
		const F bottom_alpha = clamp_unit(bottom.alpha);
		const F top_alpha = clamp_unit(top.alpha * opacity);
		const F out_alpha = top_alpha + bottom_alpha * (one - top_alpha);
		const F safe_out_alpha = if_equal(out_alpha, zero, one, out_alpha);

		const F top_red = top.red * (one - bottom_alpha) + blended_red * bottom_alpha;
		const F top_green = top.green * (one - bottom_alpha) + blended_green * bottom_alpha;
		const F top_blue = top.blue * (one - bottom_alpha) + blended_blue * bottom_alpha;

		const F red = (top_red * top_alpha + bottom.red * bottom_alpha * (one - top_alpha)) / safe_out_alpha;
		const F green = (top_green * top_alpha + bottom.green * bottom_alpha * (one - top_alpha)) / safe_out_alpha;
		const F blue = (top_blue * top_alpha + bottom.blue * bottom_alpha * (one - top_alpha)) / safe_out_alpha;

		return C(red, green, blue, out_alpha);
	}

	template <RgbaColour C>
	[[nodiscard]]
	inline typename C::ComponentType rgb_luma_w3c(const C& colour) noexcept {
		using F = typename C::ComponentType;
		return colour.red * F(0.3f) + colour.green * F(0.59f) + colour.blue * F(0.11f);
	}

	template <RgbaColour C>
	[[nodiscard]]
	inline typename C::ComponentType rgb_saturation_w3c(const C& colour) noexcept {
		using std::max; // For correct overload resolution
		using std::min; // For correct overload resolution
		return max(colour.red, max(colour.green, colour.blue)) - min(colour.red, min(colour.green, colour.blue));
	}

	/**************************************************************************************************
	 * Clip an RGB colour after W3C non-separable luma changes.
	 * ************************************************************************************************/
	template <RgbaColour C>
	[[nodiscard]]
	inline C rgb_clip_color_w3c(const C& colour) noexcept {
		using F = typename C::ComponentType;
		using std::max; // For correct overload resolution
		using std::min; // For correct overload resolution

		const F zero = F(0);
		const F one = F(1);
		const F luma = rgb_luma_w3c(colour);
		const F min_channel = min(colour.red, min(colour.green, colour.blue));
		const F max_channel = max(colour.red, max(colour.green, colour.blue));

		const F low_denominator = luma - min_channel;
		const F safe_low_denominator = if_equal(low_denominator, zero, one, low_denominator);
		F red = if_less(min_channel, zero, luma + ((colour.red - luma) * luma) / safe_low_denominator, colour.red);
		F green = if_less(min_channel, zero, luma + ((colour.green - luma) * luma) / safe_low_denominator, colour.green);
		F blue = if_less(min_channel, zero, luma + ((colour.blue - luma) * luma) / safe_low_denominator, colour.blue);

		const F high_denominator = max_channel - luma;
		const F safe_high_denominator = if_equal(high_denominator, zero, one, high_denominator);
		red = if_greater(max_channel, one, luma + ((red - luma) * (one - luma)) / safe_high_denominator, red);
		green = if_greater(max_channel, one, luma + ((green - luma) * (one - luma)) / safe_high_denominator, green);
		blue = if_greater(max_channel, one, luma + ((blue - luma) * (one - luma)) / safe_high_denominator, blue);

		return C(red, green, blue, colour.alpha);
	}

	/**************************************************************************************************
	 * Set W3C non-separable luma while preserving the colour as much as possible.
	 * ************************************************************************************************/
	template <RgbaColour C>
	[[nodiscard]]
	inline C rgb_set_luma_w3c(const C& colour, typename C::ComponentType luma) noexcept {
		const auto delta = luma - rgb_luma_w3c(colour);
		return rgb_clip_color_w3c(C(colour.red + delta, colour.green + delta, colour.blue + delta, colour.alpha));
	}

	/**************************************************************************************************
	 * Set W3C non-separable saturation by stretching the RGB channel spread.
	 * ************************************************************************************************/
	template <RgbaColour C>
	[[nodiscard]]
	inline C rgb_set_saturation_w3c(const C& colour, typename C::ComponentType saturation) noexcept {
		using F = typename C::ComponentType;
		using std::max; // For correct overload resolution
		using std::min; // For correct overload resolution

		const F zero = F(0);
		const F one = F(1);
		const F min_channel = min(colour.red, min(colour.green, colour.blue));
		const F max_channel = max(colour.red, max(colour.green, colour.blue));
		const F mid_channel = colour.red + colour.green + colour.blue - min_channel - max_channel;
		const F channel_delta = max_channel - min_channel;
		const F safe_channel_delta = if_equal(channel_delta, zero, one, channel_delta);
		const F scaled_mid = if_equal(channel_delta, zero, zero, ((mid_channel - min_channel) * saturation) / safe_channel_delta);

		const F red = if_equal(colour.red, min_channel, zero, if_equal(colour.red, max_channel, saturation, scaled_mid));
		const F green = if_equal(colour.green, min_channel, zero, if_equal(colour.green, max_channel, saturation, scaled_mid));
		const F blue = if_equal(colour.blue, min_channel, zero, if_equal(colour.blue, max_channel, saturation, scaled_mid));

		return C(red, green, blue, colour.alpha);
	}

} // namespace detail

/**************************************************************************************************
 * Mix two RGBA colours in their current coordinate space.
 *
 * This is a straight component-wise interpolation of red, green, blue, and alpha:
 * amount = 0 returns a, amount = 1 returns b.
 *
 * No colour-space conversion, transfer decoding, clamping, or alpha compositing is performed.
 * For transfer-encoded RGB types this mixes encoded channel values; for linear RGB types this mixes
 * linear-light values. Choose the colour type deliberately before calling mix().
 * ************************************************************************************************/
template <RgbaColour C>
[[nodiscard]]
inline C mix(const C& a, const C& b, typename C::ComponentType amount) noexcept {
	const auto red = a.red + (b.red - a.red) * amount;
	const auto green = a.green + (b.green - a.green) * amount;
	const auto blue = a.blue + (b.blue - a.blue) * amount;
	const auto alpha = a.alpha + (b.alpha - a.alpha) * amount;
	return C(red, green, blue, alpha);
}

/**************************************************************************************************
 * Mix two XYZA colours in their current coordinate space.
 *
 * This is a straight component-wise interpolation of x, y, z, and alpha:
 * amount = 0 returns a, amount = 1 returns b.
 *
 * No colour-space conversion, chromatic adaptation, clamping, or alpha compositing is performed.
 * ************************************************************************************************/
template <XyzaColour C>
[[nodiscard]]
inline C mix(const C& a, const C& b, typename C::ComponentType amount) noexcept {
	const auto x = a.x + (b.x - a.x) * amount;
	const auto y = a.y + (b.y - a.y) * amount;
	const auto z = a.z + (b.z - a.z) * amount;
	const auto alpha = a.alpha + (b.alpha - a.alpha) * amount;
	return C(x, y, z, alpha);
}

/**************************************************************************************************
 * Mix two HSVA colours in their current coordinate space, treating hue as circular.
 *
 * Saturation, value, and alpha are interpolated linearly. Hue is stored as a 0..1 turn and is
 * interpolated along the shortest circular path, so mixing hue 0.95 to 0.05 crosses through 0.0
 * rather than sweeping the long way around the hue wheel. Exact half-turn ties choose the negative
 * direction.
 *
 * amount = 0 returns a, amount = 1 returns b. No RGB conversion, clamping, or alpha compositing is
 * performed.
 * ************************************************************************************************/
template <HsvaColour C>
[[nodiscard]]
inline C mix(const C& a, const C& b, typename C::ComponentType amount) noexcept {
	using F = typename C::ComponentType;
	using std::floor; // For correct overload resolution

	const F half = F(0.5f);
	F hue_delta = b.hue - a.hue;
	hue_delta = hue_delta - floor(hue_delta + half);

	const F hue = detail::wrap_hue_turn(a.hue + hue_delta * amount);
	const F saturation = a.saturation + (b.saturation - a.saturation) * amount;
	const F value = a.value + (b.value - a.value) * amount;
	const F alpha = a.alpha + (b.alpha - a.alpha) * amount;
	return C(hue, saturation, value, alpha);
}

/**************************************************************************************************
 * Mix two HSLA colours in their current coordinate space, treating hue as circular.
 *
 * Saturation, lightness, and alpha are interpolated linearly. Hue is stored as a 0..1 turn and is
 * interpolated along the shortest circular path, so mixing hue 0.95 to 0.05 crosses through 0.0
 * rather than sweeping the long way around the hue wheel. Exact half-turn ties choose the negative
 * direction.
 *
 * amount = 0 returns a, amount = 1 returns b. No RGB conversion, clamping, or alpha compositing is
 * performed.
 * ************************************************************************************************/
template <HslaColour C>
[[nodiscard]]
inline C mix(const C& a, const C& b, typename C::ComponentType amount) noexcept {
	using F = typename C::ComponentType;
	using std::floor; // For correct overload resolution

	const F half = F(0.5f);
	F hue_delta = b.hue - a.hue;
	hue_delta = hue_delta - floor(hue_delta + half);

	const F hue = detail::wrap_hue_turn(a.hue + hue_delta * amount);
	const F saturation = a.saturation + (b.saturation - a.saturation) * amount;
	const F lightness = a.lightness + (b.lightness - a.lightness) * amount;
	const F alpha = a.alpha + (b.alpha - a.alpha) * amount;
	return C(hue, saturation, lightness, alpha);
}

/**************************************************************************************************
 * Mix two LABA colours in their current coordinate space.
 *
 * This is a straight component-wise interpolation of lightness, a, b, and alpha:
 * amount = 0 returns a, amount = 1 returns b.
 *
 * No colour-space conversion, clamping, or alpha compositing is performed.
 * ************************************************************************************************/
template <LabaColour C>
[[nodiscard]]
inline C mix(const C& a, const C& b, typename C::ComponentType amount) noexcept {
	const auto lightness = a.lightness + (b.lightness - a.lightness) * amount;
	const auto green_red = a.a + (b.a - a.a) * amount;
	const auto blue_yellow = a.b + (b.b - a.b) * amount;
	const auto alpha = a.alpha + (b.alpha - a.alpha) * amount;
	return C(lightness, green_red, blue_yellow, alpha);
}

/**************************************************************************************************
 * Mix two LCHA colours in their current coordinate space, treating hue as circular.
 *
 * Lightness, chroma, and alpha are interpolated linearly. Hue is stored as a 0..1 turn and is
 * interpolated along the shortest circular path, so mixing hue 0.95 to 0.05 crosses through 0.0
 * rather than sweeping the long way around the hue wheel. Exact half-turn ties choose the negative
 * direction.
 *
 * amount = 0 returns a, amount = 1 returns b. No LAB conversion, clamping, or alpha compositing is
 * performed.
 * ************************************************************************************************/
template <LchaColour C>
[[nodiscard]]
inline C mix(const C& a, const C& b, typename C::ComponentType amount) noexcept {
	using F = typename C::ComponentType;
	using std::floor; // For correct overload resolution

	const F half = F(0.5f);
	F hue_delta = b.hue - a.hue;
	hue_delta = hue_delta - floor(hue_delta + half);

	const F lightness = a.lightness + (b.lightness - a.lightness) * amount;
	const F chroma = a.chroma + (b.chroma - a.chroma) * amount;
	const F hue = detail::wrap_hue_turn(a.hue + hue_delta * amount);
	const F alpha = a.alpha + (b.alpha - a.alpha) * amount;
	return C(lightness, chroma, hue, alpha);
}

/**************************************************************************************************
 * Mix two HSVA colours along the longest circular path around the hue wheel.
 *
 * Saturation, value, and alpha are interpolated linearly. Hue is stored as a 0..1 turn and is
 * interpolated along the opposite route from mix(), so mixing hue 0.05 to 0.95 travels through 0.5
 * rather than crossing through 0.0. Identical hues make one positive turn, and exact half-turn ties
 * choose the positive direction.
 *
 * amount = 0 returns a, amount = 1 returns b after hue wrapping. No RGB conversion, clamping, or
 * alpha compositing is performed.
 * ************************************************************************************************/
template <HsvaColour C>
[[nodiscard]]
inline C mix_longest_path(const C& a, const C& b, typename C::ComponentType amount) noexcept {
	using F = typename C::ComponentType;
	using std::floor; // For correct overload resolution

	const F zero = F(0);
	const F half = F(0.5f);
	const F one = F(1);
	F hue_delta = b.hue - a.hue;
	hue_delta = hue_delta - floor(hue_delta + half);
	hue_delta = if_less_equal(hue_delta, zero, hue_delta + one, hue_delta - one);

	const F hue = detail::wrap_hue_turn(a.hue + hue_delta * amount);
	const F saturation = a.saturation + (b.saturation - a.saturation) * amount;
	const F value = a.value + (b.value - a.value) * amount;
	const F alpha = a.alpha + (b.alpha - a.alpha) * amount;
	return C(hue, saturation, value, alpha);
}

/**************************************************************************************************
 * Mix two HSLA colours along the longest circular path around the hue wheel.
 *
 * Saturation, lightness, and alpha are interpolated linearly. Hue is stored as a 0..1 turn and is
 * interpolated along the opposite route from mix(), so mixing hue 0.05 to 0.95 travels through 0.5
 * rather than crossing through 0.0. Identical hues make one positive turn, and exact half-turn ties
 * choose the positive direction.
 *
 * amount = 0 returns a, amount = 1 returns b after hue wrapping. No RGB conversion, clamping, or
 * alpha compositing is performed.
 * ************************************************************************************************/
template <HslaColour C>
[[nodiscard]]
inline C mix_longest_path(const C& a, const C& b, typename C::ComponentType amount) noexcept {
	using F = typename C::ComponentType;
	using std::floor; // For correct overload resolution

	const F zero = F(0);
	const F half = F(0.5f);
	const F one = F(1);
	F hue_delta = b.hue - a.hue;
	hue_delta = hue_delta - floor(hue_delta + half);
	hue_delta = if_less_equal(hue_delta, zero, hue_delta + one, hue_delta - one);

	const F hue = detail::wrap_hue_turn(a.hue + hue_delta * amount);
	const F saturation = a.saturation + (b.saturation - a.saturation) * amount;
	const F lightness = a.lightness + (b.lightness - a.lightness) * amount;
	const F alpha = a.alpha + (b.alpha - a.alpha) * amount;
	return C(hue, saturation, lightness, alpha);
}

/**************************************************************************************************
 * Mix two LCHA colours along the longest circular path around the hue wheel.
 *
 * Lightness, chroma, and alpha are interpolated linearly. Hue is stored as a 0..1 turn and is
 * interpolated along the opposite route from mix(), so mixing hue 0.05 to 0.95 travels through 0.5
 * rather than crossing through 0.0. Identical hues make one positive turn, and exact half-turn ties
 * choose the positive direction.
 *
 * amount = 0 returns a, amount = 1 returns b after hue wrapping. No LAB conversion, clamping, or
 * alpha compositing is performed.
 * ************************************************************************************************/
template <LchaColour C>
[[nodiscard]]
inline C mix_longest_path(const C& a, const C& b, typename C::ComponentType amount) noexcept {
	using F = typename C::ComponentType;
	using std::floor; // For correct overload resolution

	const F zero = F(0);
	const F half = F(0.5f);
	const F one = F(1);
	F hue_delta = b.hue - a.hue;
	hue_delta = hue_delta - floor(hue_delta + half);
	hue_delta = if_less_equal(hue_delta, zero, hue_delta + one, hue_delta - one);

	const F lightness = a.lightness + (b.lightness - a.lightness) * amount;
	const F chroma = a.chroma + (b.chroma - a.chroma) * amount;
	const F hue = detail::wrap_hue_turn(a.hue + hue_delta * amount);
	const F alpha = a.alpha + (b.alpha - a.alpha) * amount;
	return C(lightness, chroma, hue, alpha);
}

/**************************************************************************************************
 * Alias for mix().
 *
 * lerp() exists for users who prefer linear interpolation terminology. It deliberately forwards to
 * mix(), so each colour type keeps the interpolation behaviour defined by its mix() overload.
 * ************************************************************************************************/
template <typename C>
    requires(RgbaColour<C> || XyzaColour<C> || HsvaColour<C> || HslaColour<C> || LabaColour<C> || LchaColour<C>)
[[nodiscard]]
inline C lerp(const C& a, const C& b, typename C::ComponentType amount) noexcept {
	return mix(a, b, amount);
}

/**************************************************************************************************
 * RGB blend and matte operations.
 *
 * These functions use bottom/top layer notation. They operate in the current RGB coordinate space,
 * treat alpha and opacity as straight-alpha coverage, and return a straight-alpha colour. They do
 * not convert colour spaces or premultiply the stored result. The RGB blend modes follow W3C-style
 * source-over blending: the mode formula is applied where bottom exists, while transparent bottom
 * lets top contribute normally.
 * ************************************************************************************************/
template <RgbaColour C>
[[nodiscard]]
inline C blend_normal(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	return detail::composite_blended_rgb(bottom, top, top.red, top.green, top.blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_multiply(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	const auto red = bottom.red * top.red;
	const auto green = bottom.green * top.green;
	const auto blue = bottom.blue * top.blue;
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_screen(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	const auto red = bottom.red + top.red - bottom.red * top.red;
	const auto green = bottom.green + top.green - bottom.green * top.green;
	const auto blue = bottom.blue + top.blue - bottom.blue * top.blue;
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_overlay(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	const F half = F(0.5f);
	const F one = F(1);
	const F two = F(2);
	const F red = if_less_equal(bottom.red, half, two * bottom.red * top.red, one - two * (one - bottom.red) * (one - top.red));
	const F green = if_less_equal(bottom.green, half, two * bottom.green * top.green, one - two * (one - bottom.green) * (one - top.green));
	const F blue = if_less_equal(bottom.blue, half, two * bottom.blue * top.blue, one - two * (one - bottom.blue) * (one - top.blue));
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_darken(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using std::min; // For correct overload resolution
	const auto red = min(bottom.red, top.red);
	const auto green = min(bottom.green, top.green);
	const auto blue = min(bottom.blue, top.blue);
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_lighten(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using std::max; // For correct overload resolution
	const auto red = max(bottom.red, top.red);
	const auto green = max(bottom.green, top.green);
	const auto blue = max(bottom.blue, top.blue);
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_color_dodge(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	const F red_denominator = one - top.red;
	const F green_denominator = one - top.green;
	const F blue_denominator = one - top.blue;
	const F red = if_equal(bottom.red, zero, zero, if_equal(top.red, one, one, min(one, bottom.red / if_equal(red_denominator, zero, one, red_denominator))));
	const F green = if_equal(bottom.green, zero, zero, if_equal(top.green, one, one, min(one, bottom.green / if_equal(green_denominator, zero, one, green_denominator))));
	const F blue = if_equal(bottom.blue, zero, zero, if_equal(top.blue, one, one, min(one, bottom.blue / if_equal(blue_denominator, zero, one, blue_denominator))));
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_color_burn(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	const F red = if_equal(bottom.red, one, one, if_equal(top.red, zero, zero, one - min(one, (one - bottom.red) / if_equal(top.red, zero, one, top.red))));
	const F green = if_equal(bottom.green, one, one, if_equal(top.green, zero, zero, one - min(one, (one - bottom.green) / if_equal(top.green, zero, one, top.green))));
	const F blue = if_equal(bottom.blue, one, one, if_equal(top.blue, zero, zero, one - min(one, (one - bottom.blue) / if_equal(top.blue, zero, one, top.blue))));
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_hard_light(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	const F half = F(0.5f);
	const F one = F(1);
	const F two = F(2);
	const F red = if_less_equal(top.red, half, two * bottom.red * top.red, one - two * (one - bottom.red) * (one - top.red));
	const F green = if_less_equal(top.green, half, two * bottom.green * top.green, one - two * (one - bottom.green) * (one - top.green));
	const F blue = if_less_equal(top.blue, half, two * bottom.blue * top.blue, one - two * (one - bottom.blue) * (one - top.blue));
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_soft_light(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	using std::max; // For correct overload resolution
	using std::sqrt; // For correct overload resolution
	const F zero = F(0);
	const F quarter = F(0.25f);
	const F half = F(0.5f);
	const F one = F(1);
	const F two = F(2);
	const F four = F(4);
	const F twelve = F(12);
	const F sixteen = F(16);

	const F red_d = if_less_equal(bottom.red, quarter, ((sixteen * bottom.red - twelve) * bottom.red + four) * bottom.red, sqrt(max(zero, bottom.red)));
	const F green_d = if_less_equal(bottom.green, quarter, ((sixteen * bottom.green - twelve) * bottom.green + four) * bottom.green, sqrt(max(zero, bottom.green)));
	const F blue_d = if_less_equal(bottom.blue, quarter, ((sixteen * bottom.blue - twelve) * bottom.blue + four) * bottom.blue, sqrt(max(zero, bottom.blue)));

	const F red = if_less_equal(top.red, half, bottom.red - (one - two * top.red) * bottom.red * (one - bottom.red), bottom.red + (two * top.red - one) * (red_d - bottom.red));
	const F green = if_less_equal(top.green, half, bottom.green - (one - two * top.green) * bottom.green * (one - bottom.green), bottom.green + (two * top.green - one) * (green_d - bottom.green));
	const F blue = if_less_equal(top.blue, half, bottom.blue - (one - two * top.blue) * bottom.blue * (one - bottom.blue), bottom.blue + (two * top.blue - one) * (blue_d - bottom.blue));
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_difference(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using std::abs; // For correct overload resolution
	const auto red = abs(bottom.red - top.red);
	const auto green = abs(bottom.green - top.green);
	const auto blue = abs(bottom.blue - top.blue);
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_exclusion(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	const F two = F(2);
	const F red = bottom.red + top.red - two * bottom.red * top.red;
	const F green = bottom.green + top.green - two * bottom.green * top.green;
	const F blue = bottom.blue + top.blue - two * bottom.blue * top.blue;
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_hue(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	const auto blended = detail::rgb_set_luma_w3c(detail::rgb_set_saturation_w3c(top, detail::rgb_saturation_w3c(bottom)), detail::rgb_luma_w3c(bottom));
	return detail::composite_blended_rgb(bottom, top, blended.red, blended.green, blended.blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_saturation(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	const auto blended = detail::rgb_set_luma_w3c(detail::rgb_set_saturation_w3c(bottom, detail::rgb_saturation_w3c(top)), detail::rgb_luma_w3c(bottom));
	return detail::composite_blended_rgb(bottom, top, blended.red, blended.green, blended.blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_color(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	const auto blended = detail::rgb_set_luma_w3c(top, detail::rgb_luma_w3c(bottom));
	return detail::composite_blended_rgb(bottom, top, blended.red, blended.green, blended.blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_luminosity(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	const auto blended = detail::rgb_set_luma_w3c(bottom, detail::rgb_luma_w3c(top));
	return detail::composite_blended_rgb(bottom, top, blended.red, blended.green, blended.blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_linear_burn(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	const F one = F(1);
	const F red = detail::clamp_unit(bottom.red + top.red - one);
	const F green = detail::clamp_unit(bottom.green + top.green - one);
	const F blue = detail::clamp_unit(bottom.blue + top.blue - one);
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_linear_dodge(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	const auto red = detail::clamp_unit(bottom.red + top.red);
	const auto green = detail::clamp_unit(bottom.green + top.green);
	const auto blue = detail::clamp_unit(bottom.blue + top.blue);
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_add(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	return blend_linear_dodge(bottom, top, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_linear_light(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	const F one = F(1);
	const F two = F(2);
	const F red = detail::clamp_unit(bottom.red + two * top.red - one);
	const F green = detail::clamp_unit(bottom.green + two * top.green - one);
	const F blue = detail::clamp_unit(bottom.blue + two * top.blue - one);
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_vivid_light(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F half = F(0.5f);
	const F one = F(1);
	const F two = F(2);

	const F red_burn_top = two * top.red;
	const F green_burn_top = two * top.green;
	const F blue_burn_top = two * top.blue;
	const F red_dodge_top = two * top.red - one;
	const F green_dodge_top = two * top.green - one;
	const F blue_dodge_top = two * top.blue - one;

	const F red_burn = if_equal(bottom.red, one, one, if_equal(red_burn_top, zero, zero, one - min(one, (one - bottom.red) / if_equal(red_burn_top, zero, one, red_burn_top))));
	const F green_burn = if_equal(bottom.green, one, one, if_equal(green_burn_top, zero, zero, one - min(one, (one - bottom.green) / if_equal(green_burn_top, zero, one, green_burn_top))));
	const F blue_burn = if_equal(bottom.blue, one, one, if_equal(blue_burn_top, zero, zero, one - min(one, (one - bottom.blue) / if_equal(blue_burn_top, zero, one, blue_burn_top))));

	const F red_dodge_denominator = one - red_dodge_top;
	const F green_dodge_denominator = one - green_dodge_top;
	const F blue_dodge_denominator = one - blue_dodge_top;
	const F red_dodge = if_equal(bottom.red, zero, zero, if_equal(red_dodge_top, one, one, min(one, bottom.red / if_equal(red_dodge_denominator, zero, one, red_dodge_denominator))));
	const F green_dodge = if_equal(bottom.green, zero, zero, if_equal(green_dodge_top, one, one, min(one, bottom.green / if_equal(green_dodge_denominator, zero, one, green_dodge_denominator))));
	const F blue_dodge = if_equal(bottom.blue, zero, zero, if_equal(blue_dodge_top, one, one, min(one, bottom.blue / if_equal(blue_dodge_denominator, zero, one, blue_dodge_denominator))));

	const F red = if_less_equal(top.red, half, red_burn, red_dodge);
	const F green = if_less_equal(top.green, half, green_burn, green_dodge);
	const F blue = if_less_equal(top.blue, half, blue_burn, blue_dodge);
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_pin_light(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F half = F(0.5f);
	const F one = F(1);
	const F two = F(2);
	const F red = if_less_equal(top.red, half, min(bottom.red, two * top.red), max(bottom.red, two * top.red - one));
	const F green = if_less_equal(top.green, half, min(bottom.green, two * top.green), max(bottom.green, two * top.green - one));
	const F blue = if_less_equal(top.blue, half, min(bottom.blue, two * top.blue), max(bottom.blue, two * top.blue - one));
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_hard_mix(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F half = F(0.5f);
	const F one = F(1);
	const F two = F(2);

	const F red_burn_top = two * top.red;
	const F green_burn_top = two * top.green;
	const F blue_burn_top = two * top.blue;
	const F red_dodge_top = two * top.red - one;
	const F green_dodge_top = two * top.green - one;
	const F blue_dodge_top = two * top.blue - one;

	const F red_burn = if_equal(bottom.red, one, one, if_equal(red_burn_top, zero, zero, one - min(one, (one - bottom.red) / if_equal(red_burn_top, zero, one, red_burn_top))));
	const F green_burn = if_equal(bottom.green, one, one, if_equal(green_burn_top, zero, zero, one - min(one, (one - bottom.green) / if_equal(green_burn_top, zero, one, green_burn_top))));
	const F blue_burn = if_equal(bottom.blue, one, one, if_equal(blue_burn_top, zero, zero, one - min(one, (one - bottom.blue) / if_equal(blue_burn_top, zero, one, blue_burn_top))));

	const F red_dodge_denominator = one - red_dodge_top;
	const F green_dodge_denominator = one - green_dodge_top;
	const F blue_dodge_denominator = one - blue_dodge_top;
	const F red_dodge = if_equal(bottom.red, zero, zero, if_equal(red_dodge_top, one, one, min(one, bottom.red / if_equal(red_dodge_denominator, zero, one, red_dodge_denominator))));
	const F green_dodge = if_equal(bottom.green, zero, zero, if_equal(green_dodge_top, one, one, min(one, bottom.green / if_equal(green_dodge_denominator, zero, one, green_dodge_denominator))));
	const F blue_dodge = if_equal(bottom.blue, zero, zero, if_equal(blue_dodge_top, one, one, min(one, bottom.blue / if_equal(blue_dodge_denominator, zero, one, blue_dodge_denominator))));

	const F vivid_red = if_less_equal(top.red, half, red_burn, red_dodge);
	const F vivid_green = if_less_equal(top.green, half, green_burn, green_dodge);
	const F vivid_blue = if_less_equal(top.blue, half, blue_burn, blue_dodge);
	const F red = if_less(vivid_red, half, zero, one);
	const F green = if_less(vivid_green, half, zero, one);
	const F blue = if_less(vivid_blue, half, zero, one);
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_subtract(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	const auto red = detail::clamp_unit(bottom.red - top.red);
	const auto green = detail::clamp_unit(bottom.green - top.green);
	const auto blue = detail::clamp_unit(bottom.blue - top.blue);
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C blend_divide(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	const F zero = F(0);
	const F one = F(1);
	const F red = if_equal(top.red, zero, one, detail::clamp_unit(bottom.red / if_equal(top.red, zero, one, top.red)));
	const F green = if_equal(top.green, zero, one, detail::clamp_unit(bottom.green / if_equal(top.green, zero, one, top.green)));
	const F blue = if_equal(top.blue, zero, one, detail::clamp_unit(bottom.blue / if_equal(top.blue, zero, one, top.blue)));
	return detail::composite_blended_rgb(bottom, top, red, green, blue, opacity);
}

template <RgbaColour C>
[[nodiscard]]
inline C matte_stencil_alpha(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	const F one = F(1);
	const F amount = detail::clamp_unit(opacity);
	const F matte = detail::clamp_unit(top.alpha);
	const F alpha = detail::clamp_unit(bottom.alpha) * (one + (matte - one) * amount);
	return C(bottom.red, bottom.green, bottom.blue, alpha);
}

template <RgbaColour C>
[[nodiscard]]
inline C matte_stencil_luma(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	const F one = F(1);
	const F amount = detail::clamp_unit(opacity);
	const F matte = detail::clamp_unit(detail::rgb_luma_w3c(top) * detail::clamp_unit(top.alpha));
	const F alpha = detail::clamp_unit(bottom.alpha) * (one + (matte - one) * amount);
	return C(bottom.red, bottom.green, bottom.blue, alpha);
}

template <RgbaColour C>
[[nodiscard]]
inline C matte_silhouette_alpha(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	const F one = F(1);
	const F amount = detail::clamp_unit(opacity);
	const F matte = one - detail::clamp_unit(top.alpha);
	const F alpha = detail::clamp_unit(bottom.alpha) * (one + (matte - one) * amount);
	return C(bottom.red, bottom.green, bottom.blue, alpha);
}

template <RgbaColour C>
[[nodiscard]]
inline C matte_silhouette_luma(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	using F = typename C::ComponentType;
	const F one = F(1);
	const F amount = detail::clamp_unit(opacity);
	const F matte = one - detail::clamp_unit(detail::rgb_luma_w3c(top) * detail::clamp_unit(top.alpha));
	const F alpha = detail::clamp_unit(bottom.alpha) * (one + (matte - one) * amount);
	return C(bottom.red, bottom.green, bottom.blue, alpha);
}

template <RgbaColour C>
[[nodiscard]]
inline C matte_alpha_add(const C& bottom, const C& top, typename C::ComponentType opacity = typename C::ComponentType(1)) noexcept {
	const auto alpha = detail::clamp_unit(detail::clamp_unit(bottom.alpha) + detail::clamp_unit(top.alpha * opacity));
	return C(bottom.red, bottom.green, bottom.blue, alpha);
}

} // namespace mt
