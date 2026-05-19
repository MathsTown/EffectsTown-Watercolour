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

RGB gamut mapping member definitions.

Public API:

RGB gamut operations:
- fit_gamut()

*********************************************************************************************************/
#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include "colour_conversions.h"

namespace mt {

namespace detail {

	template <typename>
	inline constexpr bool always_false_v = false;

	/**************************************************************************************************
	 * Return mask value 1 when all RGB components are inside the closed 0..1 unit cube.
	 * ************************************************************************************************/
	template <RgbaColour C>
	[[nodiscard]]
	inline typename C::ComponentType rgb_in_unit_gamut_mask(const C& colour) noexcept {
		using F = typename C::ComponentType;
		using std::min; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);

		F red_mask = if_less(colour.red, zero, zero, one);
		red_mask = if_greater(colour.red, one, zero, red_mask);
		red_mask = if_nan(colour.red, zero, red_mask);

		F green_mask = if_less(colour.green, zero, zero, one);
		green_mask = if_greater(colour.green, one, zero, green_mask);
		green_mask = if_nan(colour.green, zero, green_mask);

		F blue_mask = if_less(colour.blue, zero, zero, one);
		blue_mask = if_greater(colour.blue, one, zero, blue_mask);
		blue_mask = if_nan(colour.blue, zero, blue_mask);

		return min(red_mask, min(green_mask, blue_mask));
	}

	/**************************************************************************************************
	 * Return mask value 1 when all RGB components are safely inside the unit cube.
	 * ************************************************************************************************/
	template <RgbaColour C>
	[[nodiscard]]
	inline typename C::ComponentType rgb_inside_gamut_surface_mask(const C& colour, typename C::ComponentType low, typename C::ComponentType high) noexcept {
		using F = typename C::ComponentType;
		using std::min; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);

		F red_mask = if_less(colour.red, low, zero, one);
		red_mask = if_greater(colour.red, high, zero, red_mask);
		red_mask = if_nan(colour.red, zero, red_mask);

		F green_mask = if_less(colour.green, low, zero, one);
		green_mask = if_greater(colour.green, high, zero, green_mask);
		green_mask = if_nan(colour.green, zero, green_mask);

		F blue_mask = if_less(colour.blue, low, zero, one);
		blue_mask = if_greater(colour.blue, high, zero, blue_mask);
		blue_mask = if_nan(colour.blue, zero, blue_mask);

		return min(red_mask, min(green_mask, blue_mask));
	}

	/**************************************************************************************************
	 * Select between two RGB colours from a 0 or 1 component mask.
	 * ************************************************************************************************/
	template <RgbaColour C>
	[[nodiscard]]
	inline C select_rgb(typename C::ComponentType mask, const C& if_true, const C& if_false) noexcept {
		using F = typename C::ComponentType;
		const F one = component_constant<F>(1.0);
		return C(
			if_equal(mask, one, if_true.red, if_false.red),
			if_equal(mask, one, if_true.green, if_false.green),
			if_equal(mask, one, if_true.blue, if_false.blue),
			if_equal(mask, one, if_true.alpha, if_false.alpha));
	}

	/**************************************************************************************************
	 * Clip RGB components to 0..1 while preserving alpha unchanged.
	 * ************************************************************************************************/
	template <RgbaColour C>
	[[nodiscard]]
	inline C clip_rgb_preserve_alpha(const C& colour, typename C::ComponentType alpha) noexcept {
		using std::max; // For correct overload resolution
		using std::min; // For correct overload resolution
		using F = typename C::ComponentType;

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);
		return C(max(zero, min(colour.red, one)), max(zero, min(colour.green, one)), max(zero, min(colour.blue, one)), alpha);
	}

	template <RgbaColour C>
	struct GamutRaytraceResult {
		C colour;
		typename C::ComponentType found;
	};

	/**************************************************************************************************
	 * Cast a ray through the 0..1 RGB cube and return the first intersection in the start->end direction.
	 * ************************************************************************************************/
	template <RgbaColour C>
	[[nodiscard]]
	inline GamutRaytraceResult<C> raytrace_rgb_unit_cube(const C& start, const C& end) noexcept {
		using F = typename C::ComponentType;
		using std::abs; // For correct overload resolution
		using std::max; // For correct overload resolution
		using std::min; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);
		const F huge = component_constant<F>(1.0e30);
		const F epsilon = component_constant<F>(1.0e-12);

		const F red_delta = end.red - start.red;
		const F green_delta = end.green - start.green;
		const F blue_delta = end.blue - start.blue;

		const F red_active = if_greater(abs(red_delta), epsilon, one, zero);
		const F green_active = if_greater(abs(green_delta), epsilon, one, zero);
		const F blue_active = if_greater(abs(blue_delta), epsilon, one, zero);

		const F safe_red_delta = if_equal(red_active, zero, one, red_delta);
		const F safe_green_delta = if_equal(green_active, zero, one, green_delta);
		const F safe_blue_delta = if_equal(blue_active, zero, one, blue_delta);

		const F red_t1 = (zero - start.red) / safe_red_delta;
		const F red_t2 = (one - start.red) / safe_red_delta;
		const F green_t1 = (zero - start.green) / safe_green_delta;
		const F green_t2 = (one - start.green) / safe_green_delta;
		const F blue_t1 = (zero - start.blue) / safe_blue_delta;
		const F blue_t2 = (one - start.blue) / safe_blue_delta;

		F tnear = component_constant<F>(-1.0e30);
		F tfar = huge;
		tnear = max(tnear, if_equal(red_active, zero, -huge, min(red_t1, red_t2)));
		tfar = min(tfar, if_equal(red_active, zero, huge, max(red_t1, red_t2)));
		tnear = max(tnear, if_equal(green_active, zero, -huge, min(green_t1, green_t2)));
		tfar = min(tfar, if_equal(green_active, zero, huge, max(green_t1, green_t2)));
		tnear = max(tnear, if_equal(blue_active, zero, -huge, min(blue_t1, blue_t2)));
		tfar = min(tfar, if_equal(blue_active, zero, huge, max(blue_t1, blue_t2)));

		const F red_parallel_inside = if_equal(red_active, zero, if_less(start.red, zero, zero, if_greater(start.red, one, zero, one)), one);
		const F green_parallel_inside = if_equal(green_active, zero, if_less(start.green, zero, zero, if_greater(start.green, one, zero, one)), one);
		const F blue_parallel_inside = if_equal(blue_active, zero, if_less(start.blue, zero, zero, if_greater(start.blue, one, zero, one)), one);

		F found = min(red_parallel_inside, min(green_parallel_inside, blue_parallel_inside));
		found = min(found, if_greater(tnear, tfar, zero, one));
		found = min(found, if_less(tfar, zero, zero, one));

		const F candidate_t = if_less(tnear, zero, tfar, tnear);
		found = min(found, if_greater(candidate_t, huge * component_constant<F>(0.5), zero, one));
		const F t = if_equal(found, zero, zero, candidate_t);

		return { C(start.red + red_delta * t, start.green + green_delta * t, start.blue + blue_delta * t, start.alpha), found };
	}

	/**************************************************************************************************
	 * Convert OkLCh to the linear-light RGB space used as the destination gamut cube.
	 * ************************************************************************************************/
	template <RgbaColour LinearRgb>
	[[nodiscard]]
	inline LinearRgb oklch_to_linear_rgb(const Oklch<typename LinearRgb::ComponentType>& colour) noexcept {
		using F = typename LinearRgb::ComponentType;
		const Xyz<F> xyz = colour.to_xyz();

		if constexpr (std::same_as<LinearRgb, SrgbLinear<F>>) {
			return xyz.to_srgb_linear();
		} else if constexpr (std::same_as<LinearRgb, DisplayP3Linear<F>>) {
			return xyz.to_display_p3_linear();
		} else if constexpr (std::same_as<LinearRgb, AdobeRgbLinear<F>>) {
			return xyz.to_adobe_rgb_linear();
		} else if constexpr (std::same_as<LinearRgb, ProPhotoRgbLinear<F>>) {
			return xyz.to_prophoto_rgb_linear();
		} else if constexpr (std::same_as<LinearRgb, Rec709Linear<F>>) {
			return xyz.to_rec709_linear();
		} else if constexpr (std::same_as<LinearRgb, Rec2020Linear<F>>) {
			return xyz.to_rec2020_linear();
		} else if constexpr (std::same_as<LinearRgb, Aces2065_1<F>>) {
			return xyz.to_aces2065_1();
		} else if constexpr (std::same_as<LinearRgb, Acescg<F>>) {
			return xyz.to_acescg();
		} else {
			static_assert(always_false_v<LinearRgb>, "Unsupported RGB type for fit_gamut().");
		}
	}

	/**************************************************************************************************
	 * Convert from the linear-light gamut cube type back to the public RGB type.
	 * ************************************************************************************************/
	template <RgbaColour OutputRgb, RgbaColour LinearRgb>
	[[nodiscard]]
	inline OutputRgb linear_rgb_to_output_rgb(const LinearRgb& colour, typename OutputRgb::ComponentType alpha) noexcept {
		const LinearRgb clipped = clip_rgb_preserve_alpha(colour, alpha);

		if constexpr (std::same_as<OutputRgb, LinearRgb>) {
			return clipped;
		} else {
			return clipped.to_transfer();
		}
	}

	/**************************************************************************************************
	 * Fit an RGB colour into its destination gamut using the CSS Color 4 ray-trace gamut mapping.
	 *
	 * The colour is projected into OkLCh, chroma is reduced along constant lightness and hue, and the
	 * RGB cube intersection is solved in the destination's linear-light RGB space. Alpha is not part
	 * of the gamut and is preserved unchanged.
	 * ************************************************************************************************/
	template <RgbaColour OutputRgb, RgbaColour LinearRgb>
	[[nodiscard]]
	inline OutputRgb fit_gamut_raytrace_rgb(const OutputRgb& colour) noexcept {
		using F = typename OutputRgb::ComponentType;
		using std::min; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);
		const F low = component_constant<F>(1.0e-6);
		const F high = one - low;

		const Oklch<F> origin_oklch = colour.to_xyz().to_oklch();
		const LinearRgb original_rgb = oklch_to_linear_rgb<LinearRgb>(origin_oklch);
		const F in_gamut = rgb_in_unit_gamut_mask(original_rgb);

		const Oklch<F> anchor_oklch(origin_oklch.lightness, zero, origin_oklch.hue, colour.alpha);
		LinearRgb anchor = oklch_to_linear_rgb<LinearRgb>(anchor_oklch);
		LinearRgb current = original_rgb;
		LinearRgb last = original_rgb;
		F active = one - in_gamut;

		for (int i = 0; i < 4; ++i) {
			F inside_surface = zero;

			if (i > 0) {
				const Oklch<F> current_oklch = current.to_xyz().to_oklch();
				const Oklch<F> corrected_oklch(origin_oklch.lightness, current_oklch.chroma, origin_oklch.hue, colour.alpha);
				const LinearRgb corrected_rgb = oklch_to_linear_rgb<LinearRgb>(corrected_oklch);
				current = select_rgb(active, corrected_rgb, current);
				inside_surface = rgb_inside_gamut_surface_mask(current, low, high);
			}

			const GamutRaytraceResult<LinearRgb> intersection = raytrace_rgb_unit_cube(anchor, current);
			const F update = min(active, intersection.found);
			const F failed = active * (one - intersection.found);
			anchor = select_rgb(min(update, inside_surface), current, anchor);
			current = select_rgb(update, intersection.colour, current);
			current = select_rgb(failed, last, current);
			last = select_rgb(update, intersection.colour, last);
			active = min(active, intersection.found);
		}

		OutputRgb mapped = linear_rgb_to_output_rgb<OutputRgb>(current, colour.alpha);
		mapped = select_rgb(in_gamut, clip_rgb_preserve_alpha(colour, colour.alpha), mapped);

		const OutputRgb white(one, one, one, colour.alpha);
		const OutputRgb black(zero, zero, zero, colour.alpha);
		mapped = select_rgb(if_greater_equal(origin_oklch.lightness, one, one, zero), white, mapped);
		mapped = select_rgb(if_less_equal(origin_oklch.lightness, zero, one, zero), black, mapped);
		return mapped.clip_rgb();
	}

} // namespace detail

/**************************************************************************************************
 * Fit this transfer-encoded sRGB colour into the sRGB gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Srgb<F> Srgb<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<Srgb<F>, SrgbLinear<F>>(*this);
}

/**************************************************************************************************
 * Fit this linear-light sRGB colour into the sRGB gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinear<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<SrgbLinear<F>, SrgbLinear<F>>(*this);
}

/**************************************************************************************************
 * Fit this transfer-encoded Display P3 colour into the Display P3 gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3<F> DisplayP3<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<DisplayP3<F>, DisplayP3Linear<F>>(*this);
}

/**************************************************************************************************
 * Fit this linear-light Display P3 colour into the Display P3 gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> DisplayP3Linear<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<DisplayP3Linear<F>, DisplayP3Linear<F>>(*this);
}

/**************************************************************************************************
 * Fit this transfer-encoded Adobe RGB (1998) colour into the Adobe RGB (1998) gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgb<F> AdobeRgb<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<AdobeRgb<F>, AdobeRgbLinear<F>>(*this);
}

/**************************************************************************************************
 * Fit this linear-light Adobe RGB (1998) colour into the Adobe RGB (1998) gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgbLinear<F> AdobeRgbLinear<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<AdobeRgbLinear<F>, AdobeRgbLinear<F>>(*this);
}

/**************************************************************************************************
 * Fit this transfer-encoded ProPhoto RGB colour into the ProPhoto RGB gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> ProPhotoRgb<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<ProPhotoRgb<F>, ProPhotoRgbLinear<F>>(*this);
}

/**************************************************************************************************
 * Fit this linear-light ProPhoto RGB colour into the ProPhoto RGB gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinear<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<ProPhotoRgbLinear<F>, ProPhotoRgbLinear<F>>(*this);
}

/**************************************************************************************************
 * Fit this transfer-encoded Rec.709 colour into the Rec.709 gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709<F> Rec709<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<Rec709<F>, Rec709Linear<F>>(*this);
}

/**************************************************************************************************
 * Fit this linear-light Rec.709 colour into the Rec.709 gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Linear<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<Rec709Linear<F>, Rec709Linear<F>>(*this);
}

/**************************************************************************************************
 * Fit this transfer-encoded Rec.2020 colour into the Rec.2020 gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020<F> Rec2020<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<Rec2020<F>, Rec2020Linear<F>>(*this);
}

/**************************************************************************************************
 * Fit this linear-light Rec.2020 colour into the Rec.2020 gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Linear<F> Rec2020Linear<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<Rec2020Linear<F>, Rec2020Linear<F>>(*this);
}

/**************************************************************************************************
 * Fit this ACES2065-1 colour into the ACES2065-1 gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<Aces2065_1<F>, Aces2065_1<F>>(*this);
}

/**************************************************************************************************
 * Fit this ACEScg colour into the ACEScg gamut.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Acescg<F>::fit_gamut() const noexcept {
	return detail::fit_gamut_raytrace_rgb<Acescg<F>, Acescg<F>>(*this);
}

} // namespace mt
