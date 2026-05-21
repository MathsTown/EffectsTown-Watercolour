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

Type:           colour_conversions


*********************************************************************************************************/
#pragma once

#include <cmath>
#include "simd-common.h"
#include "srgb.h"
#include "srgb_linear.h"
#include "xyz.h"
#include "xyz_d50.h"
#include "xyz_aces.h"
#include "cielab.h"
#include "cielch.h"
#include "oklab.h"
#include "oklch.h"
#include "display_p3.h"
#include "display_p3_linear.h"
#include "display_p3_hsv.h"
#include "display_p3_hsl.h"
#include "adobe_rgb.h"
#include "adobe_rgb_linear.h"
#include "adobe_rgb_hsv.h"
#include "adobe_rgb_hsl.h"
#include "adobe_rgb_linear_hsv.h"
#include "adobe_rgb_linear_hsl.h"
#include "prophoto_rgb.h"
#include "prophoto_rgb_linear.h"
#include "prophoto_rgb_hsv.h"
#include "prophoto_rgb_hsl.h"
#include "prophoto_rgb_linear_hsv.h"
#include "prophoto_rgb_linear_hsl.h"
#include "rec709_oetf.h"
#include "rec709_linear.h"
#include "rec709_oetf_hsv.h"
#include "rec709_oetf_hsl.h"
#include "rec709_gamma24.h"
#include "rec709_gamma24_hsv.h"
#include "rec709_gamma24_hsl.h"
#include "rec709_linear_hsv.h"
#include "rec709_linear_hsl.h"
#include "rec2020_oetf.h"
#include "rec2020_linear.h"
#include "rec2020_oetf_hsv.h"
#include "rec2020_oetf_hsl.h"
#include "rec2020_gamma24.h"
#include "rec2020_gamma24_hsv.h"
#include "rec2020_gamma24_hsl.h"
#include "rec2020_linear_hsv.h"
#include "rec2020_linear_hsl.h"
#include "aces2065_1.h"
#include "aces2065_1_hsv.h"
#include "aces2065_1_hsl.h"
#include "acescg.h"
#include "acescg_hsv.h"
#include "acescg_hsl.h"
#include "srgb_hsv.h"
#include "srgb_hsl.h"
#include "srgb_linear_hsv.h"
#include "srgb_linear_hsl.h"

namespace mt {

namespace detail {

	/**************************************************************************************************
	 * Return a colour component constant with the right scalar or SIMD lane type.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F component_constant(double value) noexcept {
		if constexpr (std::floating_point<F>) {
			return static_cast<F>(value);
		} else {
			return F(static_cast<typename F::F>(value));
		}
	}

	/**************************************************************************************************
	 * Convert one sRGB gamma-2.4 encoded component to linear light.
	 * Uses the extended sRGB gamma 2.4 transfer function from CSS Color 4 for negative values.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F srgb_transfer_to_linear(F value) noexcept {
		using std::abs; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);
		const F encoded_edge = component_constant<F>(0.04045);
		const F linear_scale = component_constant<F>(12.92);
		const F srgb_offset = component_constant<F>(0.055);
		const F srgb_scale = component_constant<F>(1.055);
		const F standard_gamma = component_constant<F>(2.4);

		const F sign = if_less(value, zero, component_constant<F>(-1.0), one);
		const F magnitude = abs(value);
		const F low = value / linear_scale;
		const F high = sign * pow((magnitude + srgb_offset) / srgb_scale, standard_gamma);
		return if_less_equal(magnitude, encoded_edge, low, high);
	}

	/**************************************************************************************************
	 * Convert one linear-light component to sRGB gamma-2.4 encoded form.
	 * Uses the extended sRGB gamma 2.4 transfer function from CSS Color 4 for negative values.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F linear_to_srgb_transfer(F value) noexcept {
		using std::abs; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);
		const F linear_edge = component_constant<F>(0.0031308);
		const F linear_scale = component_constant<F>(12.92);
		const F srgb_offset = component_constant<F>(0.055);
		const F srgb_scale = component_constant<F>(1.055);
		const F inverse_standard_gamma = component_constant<F>(1.0) / component_constant<F>(2.4);

		const F sign = if_less(value, zero, component_constant<F>(-1.0), one);
		const F magnitude = abs(value);
		const F low = value * linear_scale;
		const F high = sign * (srgb_scale * pow(magnitude, inverse_standard_gamma) - srgb_offset);
		return if_less_equal(magnitude, linear_edge, low, high);
	}

	/**************************************************************************************************
	 * Convert one Adobe RGB (1998) gamma-2.4 encoded component to linear light.
	 * Adobe RGB (1998) defines a pure power gamma 2.4 transfer function, extended by reflection for negatives.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F adobe_rgb_transfer_to_linear(F value) noexcept {
		using std::abs; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);
		const F sign = if_less(value, zero, component_constant<F>(-1.0), one);
		return sign * pow(abs(value), component_constant<F>(2.19921875));
	}

	/**************************************************************************************************
	 * Convert one linear-light component to Adobe RGB (1998) gamma-2.4 encoded form.
	 * Adobe RGB (1998) defines a pure power gamma 2.4 transfer function, extended by reflection for negatives.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F linear_to_adobe_rgb_transfer(F value) noexcept {
		using std::abs; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);
		const F sign = if_less(value, zero, component_constant<F>(-1.0), one);
		return sign * pow(abs(value), component_constant<F>(1.0) / component_constant<F>(2.19921875));
	}

	/**************************************************************************************************
	 * Convert one ProPhoto RGB gamma-2.4 encoded component to linear light.
	 * Uses the extended CSS Color 4 gamma 2.4 transfer function for negative values.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F prophoto_rgb_transfer_to_linear(F value) noexcept {
		using std::abs; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);
		const F encoded_edge = component_constant<F>(16.0) / component_constant<F>(512.0);
		const F linear_scale = component_constant<F>(16.0);
		const F standard_gamma = component_constant<F>(1.8);

		const F sign = if_less(value, zero, component_constant<F>(-1.0), one);
		const F magnitude = abs(value);
		const F low = value / linear_scale;
		const F high = sign * pow(magnitude, standard_gamma);
		return if_less_equal(magnitude, encoded_edge, low, high);
	}

	/**************************************************************************************************
	 * Convert one linear-light component to ProPhoto RGB gamma-2.4 encoded form.
	 * Uses the extended CSS Color 4 gamma 2.4 transfer function for negative values.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F linear_to_prophoto_rgb_transfer(F value) noexcept {
		using std::abs; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);
		const F linear_edge = component_constant<F>(1.0) / component_constant<F>(512.0);
		const F linear_scale = component_constant<F>(16.0);
		const F inverse_standard_gamma = component_constant<F>(1.0) / component_constant<F>(1.8);

		const F sign = if_less(value, zero, component_constant<F>(-1.0), one);
		const F magnitude = abs(value);
		const F low = value * linear_scale;
		const F high = sign * pow(magnitude, inverse_standard_gamma);
		return if_less_equal(magnitude, linear_edge, low, high);
	}

	/**************************************************************************************************
	 * Convert one Rec.709 OETF-encoded component to linear light.
	 * Rec.709 defines a linear toe followed by a 0.45 power law over the valid 0..1 range.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F rec709_oetf_to_linear(F value) noexcept {
		using std::max; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F encoded_edge = component_constant<F>(0.081);
		const F linear_scale = component_constant<F>(4.5);
		const F transfer_scale = component_constant<F>(1.099);
		const F transfer_offset = component_constant<F>(0.099);
		const F inverse_gamma = component_constant<F>(1.0) / component_constant<F>(0.45);

		const F low = value / linear_scale;
		const F high_base = max(zero, (value + transfer_offset) / transfer_scale);
		const F high = pow(high_base, inverse_gamma);
		return if_less(value, encoded_edge, low, high);
	}

	/**************************************************************************************************
	 * Convert one linear-light component to Rec.709 OETF-encoded form.
	 * Rec.709 defines a linear toe followed by a 0.45 power law over the valid 0..1 range.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F linear_to_rec709_oetf(F value) noexcept {
		using std::max; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F linear_edge = component_constant<F>(0.018);
		const F linear_scale = component_constant<F>(4.5);
		const F transfer_scale = component_constant<F>(1.099);
		const F transfer_offset = component_constant<F>(0.099);
		const F gamma = component_constant<F>(0.45);

		const F low = value * linear_scale;
		const F high = transfer_scale * pow(max(zero, value), gamma) - transfer_offset;
		return if_less(value, linear_edge, low, high);
	}

	/**************************************************************************************************
	 * Convert one Rec.709 gamma-2.4 encoded component to linear light.
	 * This is a pure display-gamma transfer, extended by reflection for negative values.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F rec709_gamma24_to_linear(F value) noexcept {
		using std::abs; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);
		const F sign = if_less(value, zero, component_constant<F>(-1.0), one);
		return sign * pow(abs(value), component_constant<F>(2.4));
	}

	/**************************************************************************************************
	 * Convert one linear-light component to Rec.709 gamma-2.4 encoded form.
	 * This is a pure display-gamma transfer, extended by reflection for negative values.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F linear_to_rec709_gamma24(F value) noexcept {
		using std::abs; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);
		const F sign = if_less(value, zero, component_constant<F>(-1.0), one);
		return sign * pow(abs(value), component_constant<F>(1.0) / component_constant<F>(2.4));
	}

	/**************************************************************************************************
	 * Convert one Rec.2020 OETF-encoded component to linear light.
	 * This uses BT.2020's non-12-bit constants: alpha = 1.099 and beta = 0.018.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F rec2020_oetf_to_linear(F value) noexcept {
		using std::max; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F linear_beta = component_constant<F>(0.018);
		const F encoded_edge = component_constant<F>(4.5) * linear_beta;
		const F linear_scale = component_constant<F>(4.5);
		const F transfer_scale = component_constant<F>(1.099);
		const F transfer_offset = transfer_scale - component_constant<F>(1.0);
		const F inverse_gamma = component_constant<F>(1.0) / component_constant<F>(0.45);

		const F low = value / linear_scale;
		const F high_base = max(zero, (value + transfer_offset) / transfer_scale);
		const F high = pow(high_base, inverse_gamma);
		return if_less(value, encoded_edge, low, high);
	}

	/**************************************************************************************************
	 * Convert one linear-light component to Rec.2020 OETF-encoded form.
	 * This uses BT.2020's non-12-bit constants: alpha = 1.099 and beta = 0.018.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F linear_to_rec2020_oetf(F value) noexcept {
		using std::max; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F linear_edge = component_constant<F>(0.018);
		const F linear_scale = component_constant<F>(4.5);
		const F transfer_scale = component_constant<F>(1.099);
		const F transfer_offset = transfer_scale - component_constant<F>(1.0);
		const F gamma = component_constant<F>(0.45);

		const F low = value * linear_scale;
		const F high = transfer_scale * pow(max(zero, value), gamma) - transfer_offset;
		return if_less(value, linear_edge, low, high);
	}

	/**************************************************************************************************
	 * Convert one Rec.2020 gamma-2.4 encoded component to linear light.
	 * This is a pure display-gamma transfer, extended by reflection for negative values.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F rec2020_gamma24_to_linear(F value) noexcept {
		using std::abs; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);
		const F sign = if_less(value, zero, component_constant<F>(-1.0), one);
		return sign * pow(abs(value), component_constant<F>(2.4));
	}

	/**************************************************************************************************
	 * Convert one linear-light component to Rec.2020 gamma-2.4 encoded form.
	 * This is a pure display-gamma transfer, extended by reflection for negative values.
	 * ************************************************************************************************/
	template <ColourComponent F>
	[[nodiscard]]
	inline F linear_to_rec2020_gamma24(F value) noexcept {
		using std::abs; // For correct overload resolution
		using std::pow; // For correct overload resolution

		const F zero = component_constant<F>(0.0);
		const F one = component_constant<F>(1.0);
		const F sign = if_less(value, zero, component_constant<F>(-1.0), one);
		return sign * pow(abs(value), component_constant<F>(1.0) / component_constant<F>(2.4));
	}

	/**************************************************************************************************
	 * Convert an RGBA colour to an HSVA colour with the same component type.
	 * ************************************************************************************************/
	template <HsvaColour Hsv, RgbaColour Rgb>
	    requires std::same_as<typename Hsv::ComponentType, typename Rgb::ComponentType>
	[[nodiscard]]
	inline Hsv rgb_to_hsv(const Rgb& rgb) noexcept {
		using F = typename Rgb::ComponentType;
		using std::max; // For correct overload resolution
		using std::min; // For correct overload resolution

		const F zero = F(0);
		const F one = F(1);
		const F two = F(2);
		const F four = F(4);
		const F six = F(6);

		const F max_rgb = max(rgb.red, max(rgb.green, rgb.blue));
		const F min_rgb = min(rgb.red, min(rgb.green, rgb.blue));
		const F delta = max_rgb - min_rgb;
		const F safe_delta = if_equal(delta, zero, one, delta);

		const F red_hue = ((rgb.green - rgb.blue) / safe_delta) / six;
		const F green_hue = (((rgb.blue - rgb.red) / safe_delta) + two) / six;
		const F blue_hue = (((rgb.red - rgb.green) / safe_delta) + four) / six;
		F hue = if_equal(max_rgb, rgb.red, red_hue, if_equal(max_rgb, rgb.green, green_hue, blue_hue));
		hue = wrap_hue_turn(hue);
		hue = if_equal(delta, zero, zero, hue);

		const F saturation = if_equal(max_rgb, zero, zero, delta / max_rgb);
		return Hsv(hue, saturation, max_rgb, rgb.alpha);
	}

	/**************************************************************************************************
	 * Convert an HSVA colour to an RGBA colour with the same component type.
	 * ************************************************************************************************/
	template <RgbaColour Rgb, HsvaColour Hsv>
	    requires std::same_as<typename Rgb::ComponentType, typename Hsv::ComponentType>
	[[nodiscard]]
	inline Rgb hsv_to_rgb(const Hsv& hsv) noexcept {
		using F = typename Hsv::ComponentType;
		using std::floor; // For correct overload resolution

		const F one = F(1);
		const F two = F(2);
		const F three = F(3);
		const F four = F(4);
		const F five = F(5);
		const F six = F(6);

		const F hue = wrap_hue_turn(hsv.hue);
		const F sector = hue * six;
		const F fraction = sector - floor(sector);
		const F p = hsv.value * (one - hsv.saturation);
		const F q = hsv.value * (one - hsv.saturation * fraction);
		const F t = hsv.value * (one - hsv.saturation * (one - fraction));

		const F red = if_less(hue, one / six, hsv.value, if_less(hue, two / six, q, if_less(hue, three / six, p, if_less(hue, four / six, p, if_less(hue, five / six, t, hsv.value)))));
		const F green = if_less(hue, one / six, t, if_less(hue, two / six, hsv.value, if_less(hue, three / six, hsv.value, if_less(hue, four / six, q, if_less(hue, five / six, p, p)))));
		const F blue = if_less(hue, one / six, p, if_less(hue, two / six, p, if_less(hue, three / six, t, if_less(hue, four / six, hsv.value, if_less(hue, five / six, hsv.value, q)))));

		return Rgb(red, green, blue, hsv.alpha);
	}

	/**************************************************************************************************
	 * Convert an RGBA colour to an HSLA colour with the same component type.
	 * ************************************************************************************************/
	template <HslaColour Hsl, RgbaColour Rgb>
	    requires std::same_as<typename Hsl::ComponentType, typename Rgb::ComponentType>
	[[nodiscard]]
	inline Hsl rgb_to_hsl(const Rgb& rgb) noexcept {
		using F = typename Rgb::ComponentType;
		using std::abs; // For correct overload resolution
		using std::max; // For correct overload resolution
		using std::min; // For correct overload resolution

		const F zero = F(0);
		const F one = F(1);
		const F two = F(2);
		const F four = F(4);
		const F six = F(6);

		const F max_rgb = max(rgb.red, max(rgb.green, rgb.blue));
		const F min_rgb = min(rgb.red, min(rgb.green, rgb.blue));
		const F delta = max_rgb - min_rgb;
		const F safe_delta = if_equal(delta, zero, one, delta);
		const F lightness = (max_rgb + min_rgb) / two;

		const F red_hue = ((rgb.green - rgb.blue) / safe_delta) / six;
		const F green_hue = (((rgb.blue - rgb.red) / safe_delta) + two) / six;
		const F blue_hue = (((rgb.red - rgb.green) / safe_delta) + four) / six;
		F hue = if_equal(max_rgb, rgb.red, red_hue, if_equal(max_rgb, rgb.green, green_hue, blue_hue));
		hue = wrap_hue_turn(hue);
		hue = if_equal(delta, zero, zero, hue);

		const F denominator = one - abs(two * lightness - one);
		const F safe_denominator = if_equal(denominator, zero, one, denominator);
		const F saturation = if_equal(delta, zero, zero, delta / safe_denominator);

		return Hsl(hue, saturation, lightness, rgb.alpha);
	}

	/**************************************************************************************************
	 * Convert an HSLA colour to an RGBA colour with the same component type.
	 * ************************************************************************************************/
	template <RgbaColour Rgb, HslaColour Hsl>
	    requires std::same_as<typename Rgb::ComponentType, typename Hsl::ComponentType>
	[[nodiscard]]
	inline Rgb hsl_to_rgb(const Hsl& hsl) noexcept {
		using F = typename Hsl::ComponentType;

		const F one = F(1);
		const F two = F(2);
		const F three = F(3);
		const F six = F(6);

		const F hue = wrap_hue_turn(hsl.hue);
		const F q = if_less(hsl.lightness, F(0.5), hsl.lightness * (one + hsl.saturation), hsl.lightness + hsl.saturation - hsl.lightness * hsl.saturation);
		const F p = two * hsl.lightness - q;

		const F red_hue = wrap_hue_turn(hue + one / three);
		const F green_hue = hue;
		const F blue_hue = wrap_hue_turn(hue - one / three);

		const F red = if_less(red_hue, one / six, p + (q - p) * six * red_hue, if_less(red_hue, one / two, q, if_less(red_hue, two / three, p + (q - p) * (two / three - red_hue) * six, p)));
		const F green = if_less(green_hue, one / six, p + (q - p) * six * green_hue, if_less(green_hue, one / two, q, if_less(green_hue, two / three, p + (q - p) * (two / three - green_hue) * six, p)));
		const F blue = if_less(blue_hue, one / six, p + (q - p) * six * blue_hue, if_less(blue_hue, one / two, q, if_less(blue_hue, two / three, p + (q - p) * (two / three - blue_hue) * six, p)));

		return Rgb(red, green, blue, hsl.alpha);
	}

} // namespace detail

/**************************************************************************************************
 * Convert gamma-2.4 encoded sRGBA to linear sRGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> Srgb<F>::to_linear() const noexcept {
	return SrgbLinear<F>(detail::srgb_transfer_to_linear(red), detail::srgb_transfer_to_linear(green), detail::srgb_transfer_to_linear(blue), alpha);
}

/**************************************************************************************************
 * Convert linear sRGBA to gamma-2.4 encoded sRGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Srgb<F> SrgbLinear<F>::to_srgb() const noexcept {
	return Srgb<F>(detail::linear_to_srgb_transfer(red), detail::linear_to_srgb_transfer(green), detail::linear_to_srgb_transfer(blue), alpha);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded sRGBA to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Srgb<F>::to_xyz() const noexcept {
	return to_linear().to_xyz();
}

/**************************************************************************************************
 * Convert linear sRGBA to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> SrgbLinear<F>::to_xyz() const noexcept {
	const F xyz_x = detail::component_constant<F>(0.4123907992659593) * red + detail::component_constant<F>(0.357584339383878) * green + detail::component_constant<F>(0.1804807884018343) * blue;
	const F xyz_y = detail::component_constant<F>(0.2126390058715103) * red + detail::component_constant<F>(0.7151686787677559) * green + detail::component_constant<F>(0.07219231536073371) * blue;
	const F xyz_z = detail::component_constant<F>(0.01933081871559182) * red + detail::component_constant<F>(0.1191947797946259) * green + detail::component_constant<F>(0.9505321522496606) * blue;
	return Xyz<F>(xyz_x, xyz_y, xyz_z, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to linear sRGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> Xyz<F>::to_srgb_linear() const noexcept {
	const F red = detail::component_constant<F>(3.240969941904523) * x + detail::component_constant<F>(-1.537383177570094) * y + detail::component_constant<F>(-0.4986107602930034) * z;
	const F green = detail::component_constant<F>(-0.9692436362808798) * x + detail::component_constant<F>(1.875967501507721) * y + detail::component_constant<F>(0.04155505740717563) * z;
	const F blue = detail::component_constant<F>(0.05563007969699364) * x + detail::component_constant<F>(-0.2039769588889765) * y + detail::component_constant<F>(1.056971514242879) * z;
	return SrgbLinear<F>(red, green, blue, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to gamma-2.4 encoded sRGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Srgb<F> Xyz<F>::to_srgb() const noexcept {
	return to_srgb_linear().to_srgb();
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Display P3 RGBA to linear Display P3 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> DisplayP3<F>::to_linear() const noexcept {
	return DisplayP3Linear<F>(detail::srgb_transfer_to_linear(red), detail::srgb_transfer_to_linear(green), detail::srgb_transfer_to_linear(blue), alpha);
}

/**************************************************************************************************
 * Convert linear Display P3 RGBA to gamma-2.4 encoded Display P3 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3<F> DisplayP3Linear<F>::to_display_p3() const noexcept {
	return DisplayP3<F>(detail::linear_to_srgb_transfer(red), detail::linear_to_srgb_transfer(green), detail::linear_to_srgb_transfer(blue), alpha);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Display P3 RGBA to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> DisplayP3<F>::to_xyz() const noexcept {
	return to_linear().to_xyz();
}

/**************************************************************************************************
 * Convert linear Display P3 RGBA to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> DisplayP3Linear<F>::to_xyz() const noexcept {
	const F xyz_x = detail::component_constant<F>(0.4865709486482) * red + detail::component_constant<F>(0.2656676931691) * green + detail::component_constant<F>(0.1982172852344) * blue;
	const F xyz_y = detail::component_constant<F>(0.2289745640697) * red + detail::component_constant<F>(0.6917385218365) * green + detail::component_constant<F>(0.07928691409374) * blue;
	const F xyz_z = detail::component_constant<F>(-0.00000000000000003972075516933) * red + detail::component_constant<F>(0.0451133818589) * green + detail::component_constant<F>(1.043944368901) * blue;
	return Xyz<F>(xyz_x, xyz_y, xyz_z, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to linear Display P3 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> Xyz<F>::to_display_p3_linear() const noexcept {
	const F red = detail::component_constant<F>(2.493496911941) * x + detail::component_constant<F>(-0.931383617919) * y + detail::component_constant<F>(-0.402710784451) * z;
	const F green = detail::component_constant<F>(-0.829488969562) * x + detail::component_constant<F>(1.762664060318) * y + detail::component_constant<F>(0.023624685842) * z;
	const F blue = detail::component_constant<F>(0.035845830244) * x + detail::component_constant<F>(-0.076172389268) * y + detail::component_constant<F>(0.956884524008) * z;
	return DisplayP3Linear<F>(red, green, blue, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to gamma-2.4 encoded Display P3 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3<F> Xyz<F>::to_display_p3() const noexcept {
	return to_display_p3_linear().to_display_p3();
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to CIE XYZ D50.
 * Uses Bradford chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzD50<F> Xyz<F>::to_xyz_d50() const noexcept {
	const F d50_x = detail::component_constant<F>(1.0479297925449966) * x + detail::component_constant<F>(0.02294687060160961) * y + detail::component_constant<F>(-0.05019226628920518) * z;
	const F d50_y = detail::component_constant<F>(0.0296278087700558) * x + detail::component_constant<F>(0.9904344267538798) * y + detail::component_constant<F>(-0.0170737990634188) * z;
	const F d50_z = detail::component_constant<F>(-0.00924304064620451) * x + detail::component_constant<F>(0.01505519149029814) * y + detail::component_constant<F>(0.7518742814281372) * z;
	return XyzD50<F>(d50_x, d50_y, d50_z, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to CIE XYZ D50.
 * Uses CAT02 chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzD50<F> Xyz<F>::to_xyz_d50_with_cat02() const noexcept {
	const F d50_x = detail::component_constant<F>(1.0425738924111991) * x + detail::component_constant<F>(0.030891075263705207) * y + detail::component_constant<F>(-0.052812565931937373) * z;
	const F d50_y = detail::component_constant<F>(0.022193451065252102) * x + detail::component_constant<F>(1.0018566328072378) * y + detail::component_constant<F>(-0.021073749209281616) * z;
	const F d50_z = detail::component_constant<F>(-0.0011648800532352028) * x + detail::component_constant<F>(-0.0034205274827750882) * y + detail::component_constant<F>(0.76178907552447983) * z;
	return XyzD50<F>(d50_x, d50_y, d50_z, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D50 to CIE XYZ D65.
 * Uses Bradford chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> XyzD50<F>::to_xyz() const noexcept {
	const F d65_x = detail::component_constant<F>(0.955473421488075) * x + detail::component_constant<F>(-0.0230984549487646) * y + detail::component_constant<F>(0.06325924320057069) * z;
	const F d65_y = detail::component_constant<F>(-0.02836970933386368) * x + detail::component_constant<F>(1.009995398081304) * y + detail::component_constant<F>(0.02104144119191732) * z;
	const F d65_z = detail::component_constant<F>(0.01231401486448198) * x + detail::component_constant<F>(-0.02050764929889896) * y + detail::component_constant<F>(1.330365926242124) * z;
	return Xyz<F>(d65_x, d65_y, d65_z, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D50 to CIE XYZ D65.
 * Uses CAT02 chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> XyzD50<F>::to_xyz_with_cat02() const noexcept {
	const F d65_x = detail::component_constant<F>(0.95986331480375953) * x + detail::component_constant<F>(-0.029371839290647482) * y + detail::component_constant<F>(0.065731934773703718) * z;
	const F d65_y = detail::component_constant<F>(-0.021234333154949731) * x + detail::component_constant<F>(0.9988908612350017) * y + detail::component_constant<F>(0.026160700537248446) * z;
	const F d65_z = detail::component_constant<F>(0.0013724179601714652) * x + detail::component_constant<F>(0.0044402303499848457) * y + detail::component_constant<F>(1.3129172960720605) * z;
	return Xyz<F>(d65_x, d65_y, d65_z, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to CIE XYZ with ACES white.
 * Uses Bradford chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzAces<F> Xyz<F>::to_xyz_aces() const noexcept {
	const F aces_x = detail::component_constant<F>(1.0130349146499857) * x + detail::component_constant<F>(0.0061052578232071667) * y + detail::component_constant<F>(-0.01497094362658747) * z;
	const F aces_y = detail::component_constant<F>(0.0076982301254151474) * x + detail::component_constant<F>(0.99816335211827711) * y + detail::component_constant<F>(-0.005032038535111874) * z;
	const F aces_z = detail::component_constant<F>(-0.0028413174324390562) * x + detail::component_constant<F>(0.0046851567225372082) * y + detail::component_constant<F>(0.9245061374576633) * z;
	return XyzAces<F>(aces_x, aces_y, aces_z, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to CIE XYZ with ACES white.
 * Uses CAT02 chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzAces<F> Xyz<F>::to_xyz_aces_with_cat02() const noexcept {
	const F aces_x = detail::component_constant<F>(1.0119593492787886) * x + detail::component_constant<F>(0.008007966657797648) * y + detail::component_constant<F>(-0.015779377662330899) * z;
	const F aces_y = detail::component_constant<F>(0.0057710781535406574) * x + detail::component_constant<F>(1.0013620155134944) * y + detail::component_constant<F>(-0.0062872432111409412) * z;
	const F aces_z = detail::component_constant<F>(-0.00033762208926845118) * x + detail::component_constant<F>(-0.0010466140323350836) * y + detail::component_constant<F>(0.92758413646559934) * z;
	return XyzAces<F>(aces_x, aces_y, aces_z, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ with ACES white to CIE XYZ D65.
 * Uses Bradford chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> XyzAces<F>::to_xyz() const noexcept {
	const F d65_x = detail::component_constant<F>(0.98722400870301763) * x + detail::component_constant<F>(-0.0061132286068568851) * y + detail::component_constant<F>(0.015953288335912634) * z;
	const F d65_y = detail::component_constant<F>(-0.0075983718116623541) * x + detail::component_constant<F>(1.0018614847396536) * y + detail::component_constant<F>(0.00533003579138894) * z;
	const F d65_z = detail::component_constant<F>(0.0030725770585315268) * x + detail::component_constant<F>(-0.0050959615111305819) * y + detail::component_constant<F>(1.0816806030657953) * z;
	return Xyz<F>(d65_x, d65_y, d65_z, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ with ACES white to CIE XYZ D65.
 * Uses CAT02 chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> XyzAces<F>::to_xyz_with_cat02() const noexcept {
	const F d65_x = detail::component_constant<F>(0.98823254712181341) * x + detail::component_constant<F>(-0.0078854543998661715) * y + detail::component_constant<F>(0.016757635451624009) * z;
	const F d65_y = detail::component_constant<F>(-0.0056931919334120744) * x + detail::component_constant<F>(0.99869233982950012) * y + detail::component_constant<F>(0.0066723722028848914) * z;
	const F d65_z = detail::component_constant<F>(0.00035327314234229011) * x + detail::component_constant<F>(0.0011239768688093177) * y + detail::component_constant<F>(1.0780829488488732) * z;
	return Xyz<F>(d65_x, d65_y, d65_z, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D50 to CIELAB D50.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Cielab<F> XyzD50<F>::to_cielab() const noexcept {
	using std::cbrt; // For correct overload resolution

	const F epsilon = detail::component_constant<F>(216.0 / 24389.0);
	const F kappa = detail::component_constant<F>(24389.0 / 27.0);
	const F sixteen = F(16);
	const F one_hundred_and_sixteen = F(116);

	const F d50_x = detail::component_constant<F>(0.9642956764295677);
	const F d50_z = detail::component_constant<F>(0.8251046025104604);

	const F x_ratio = x / d50_x;
	const F y_ratio = y;
	const F z_ratio = z / d50_z;

	const F x_cube_root = cbrt(x_ratio);
	const F y_cube_root = cbrt(y_ratio);
	const F z_cube_root = cbrt(z_ratio);

	const F x_linear = (kappa * x_ratio + sixteen) / one_hundred_and_sixteen;
	const F y_linear = (kappa * y_ratio + sixteen) / one_hundred_and_sixteen;
	const F z_linear = (kappa * z_ratio + sixteen) / one_hundred_and_sixteen;

	const F lab_x = if_less_equal(x_ratio, epsilon, x_linear, x_cube_root);
	const F lab_y = if_less_equal(y_ratio, epsilon, y_linear, y_cube_root);
	const F lab_z = if_less_equal(z_ratio, epsilon, z_linear, z_cube_root);

	const F lab_l = one_hundred_and_sixteen * lab_y - sixteen;
	const F lab_a = F(500) * (lab_x - lab_y);
	const F lab_b = F(200) * (lab_y - lab_z);
	return Cielab<F>(lab_l, lab_a, lab_b, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D50 to CIELCH D50.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Cielch<F> XyzD50<F>::to_cielch() const noexcept {
	return to_cielab().to_cielch();
}

/**************************************************************************************************
 * Convert CIELAB D50 to CIE XYZ D50.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzD50<F> Cielab<F>::to_xyz_d50() const noexcept {
	const F kappa = detail::component_constant<F>(24389.0 / 27.0);
	const F delta = detail::component_constant<F>(6.0 / 29.0);
	const F sixteen = F(16);
	const F one_hundred_and_sixteen = F(116);

	const F d50_x = detail::component_constant<F>(0.9642956764295677);
	const F d50_z = detail::component_constant<F>(0.8251046025104604);

	const F lab_y = (lightness + sixteen) / one_hundred_and_sixteen;
	const F lab_x = a / F(500) + lab_y;
	const F lab_z = lab_y - b / F(200);

	const F x_cube = lab_x * lab_x * lab_x;
	const F y_cube = lab_y * lab_y * lab_y;
	const F z_cube = lab_z * lab_z * lab_z;

	const F x_linear = (one_hundred_and_sixteen * lab_x - sixteen) / kappa;
	const F y_linear = (one_hundred_and_sixteen * lab_y - sixteen) / kappa;
	const F z_linear = (one_hundred_and_sixteen * lab_z - sixteen) / kappa;

	const F x_ratio = if_less(delta, lab_x, x_cube, x_linear);
	const F y_ratio = if_less(delta, lab_y, y_cube, y_linear);
	const F z_ratio = if_less(delta, lab_z, z_cube, z_linear);

	return XyzD50<F>(x_ratio * d50_x, y_ratio, z_ratio * d50_z, alpha);
}

/**************************************************************************************************
 * Convert CIELAB D50 to CIELCH D50.
 * Hue is returned as a 0..1 turn. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Cielch<F> Cielab<F>::to_cielch() const noexcept {
	using std::atan2; // For correct overload resolution
	using std::hypot; // For correct overload resolution

	const F tau = detail::component_constant<F>(6.283185307179586476925286766559);
	const F chroma = hypot(a, b);
	F hue = atan2(b, a) / tau;
	hue = detail::wrap_hue_turn(hue);
	const F zero = F(0);
	hue = if_equal(chroma, zero, zero, hue);
	return Cielch<F>(lightness, chroma, hue, alpha);
}

/**************************************************************************************************
 * Convert CIELCH D50 to CIELAB D50.
 * Hue is interpreted as a 0..1 turn. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Cielab<F> Cielch<F>::to_cielab() const noexcept {
	using std::cos; // For correct overload resolution
	using std::sin; // For correct overload resolution

	const Cielch<F> wrapped = wrap_hue();
	const F tau = detail::component_constant<F>(6.283185307179586476925286766559);
	const F hue_radians = wrapped.hue * tau;
	const F lab_a = wrapped.chroma * cos(hue_radians);
	const F lab_b = wrapped.chroma * sin(hue_radians);
	return Cielab<F>(wrapped.lightness, lab_a, lab_b, wrapped.alpha);
}

/**************************************************************************************************
 * Convert CIELCH D50 to CIE XYZ D50.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzD50<F> Cielch<F>::to_xyz_d50() const noexcept {
	return to_cielab().to_xyz_d50();
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to Oklab.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Oklab<F> Xyz<F>::to_oklab() const noexcept {
	using std::cbrt; // For correct overload resolution

	const F lms_l = detail::component_constant<F>(0.8189330101) * x + detail::component_constant<F>(0.3618667424) * y + detail::component_constant<F>(-0.1288597137) * z;
	const F lms_m = detail::component_constant<F>(0.0329845436) * x + detail::component_constant<F>(0.9293118715) * y + detail::component_constant<F>(0.0361456387) * z;
	const F lms_s = detail::component_constant<F>(0.0482003018) * x + detail::component_constant<F>(0.2643662691) * y + detail::component_constant<F>(0.6338517070) * z;

	const F lms_l_root = cbrt(lms_l);
	const F lms_m_root = cbrt(lms_m);
	const F lms_s_root = cbrt(lms_s);

	const F lab_l = detail::component_constant<F>(0.2104542553) * lms_l_root + detail::component_constant<F>(0.7936177850) * lms_m_root + detail::component_constant<F>(-0.0040720468) * lms_s_root;
	const F lab_a = detail::component_constant<F>(1.9779984951) * lms_l_root + detail::component_constant<F>(-2.4285922050) * lms_m_root + detail::component_constant<F>(0.4505937099) * lms_s_root;
	const F lab_b = detail::component_constant<F>(0.0259040371) * lms_l_root + detail::component_constant<F>(0.7827717662) * lms_m_root + detail::component_constant<F>(-0.8086757660) * lms_s_root;
	return Oklab<F>(lab_l, lab_a, lab_b, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to Oklch.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Oklch<F> Xyz<F>::to_oklch() const noexcept {
	return to_oklab().to_oklch();
}

/**************************************************************************************************
 * Convert Oklab to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Oklab<F>::to_xyz() const noexcept {
	const F lms_l_root = detail::component_constant<F>(0.9999999984505196) * lightness + detail::component_constant<F>(0.39633779217376774) * a + detail::component_constant<F>(0.2158037580607588) * b;
	const F lms_m_root = detail::component_constant<F>(1.0000000088817607) * lightness + detail::component_constant<F>(-0.10556134232365633) * a + detail::component_constant<F>(-0.0638541747717059) * b;
	const F lms_s_root = detail::component_constant<F>(1.0000000546724108) * lightness + detail::component_constant<F>(-0.08948418209496574) * a + detail::component_constant<F>(-1.2914855378640917) * b;

	const F lms_l = lms_l_root * lms_l_root * lms_l_root;
	const F lms_m = lms_m_root * lms_m_root * lms_m_root;
	const F lms_s = lms_s_root * lms_s_root * lms_s_root;

	const F xyz_x = detail::component_constant<F>(1.2270138511035211) * lms_l + detail::component_constant<F>(-0.5577999806518222) * lms_m + detail::component_constant<F>(0.2812561489664678) * lms_s;
	const F xyz_y = detail::component_constant<F>(-0.0405801784232806) * lms_l + detail::component_constant<F>(1.11225686961683) * lms_m + detail::component_constant<F>(-0.07167667866560119) * lms_s;
	const F xyz_z = detail::component_constant<F>(-0.0763812845057069) * lms_l + detail::component_constant<F>(-0.4214819784180127) * lms_m + detail::component_constant<F>(1.5861632204407947) * lms_s;
	return Xyz<F>(xyz_x, xyz_y, xyz_z, alpha);
}

/**************************************************************************************************
 * Convert Oklab to Oklch.
 * Hue is returned as a 0..1 turn. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Oklch<F> Oklab<F>::to_oklch() const noexcept {
	using std::atan2; // For correct overload resolution
	using std::hypot; // For correct overload resolution

	const F tau = detail::component_constant<F>(6.283185307179586476925286766559);
	const F chroma = hypot(a, b);
	F hue = atan2(b, a) / tau;
	hue = detail::wrap_hue_turn(hue);
	const F zero = F(0);
	hue = if_equal(chroma, zero, zero, hue);
	return Oklch<F>(lightness, chroma, hue, alpha);
}

/**************************************************************************************************
 * Convert Oklch to Oklab.
 * Hue is interpreted as a 0..1 turn. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Oklab<F> Oklch<F>::to_oklab() const noexcept {
	using std::cos; // For correct overload resolution
	using std::sin; // For correct overload resolution

	const Oklch<F> wrapped = wrap_hue();
	const F tau = detail::component_constant<F>(6.283185307179586476925286766559);
	const F hue_radians = wrapped.hue * tau;
	const F lab_a = wrapped.chroma * cos(hue_radians);
	const F lab_b = wrapped.chroma * sin(hue_radians);
	return Oklab<F>(wrapped.lightness, lab_a, lab_b, wrapped.alpha);
}

/**************************************************************************************************
 * Convert Oklch to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Oklch<F>::to_xyz() const noexcept {
	return to_oklab().to_xyz();
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Adobe RGB (1998) RGBA to linear Adobe RGB (1998) RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgbLinear<F> AdobeRgb<F>::to_linear() const noexcept {
	return AdobeRgbLinear<F>(detail::adobe_rgb_transfer_to_linear(red), detail::adobe_rgb_transfer_to_linear(green), detail::adobe_rgb_transfer_to_linear(blue), alpha);
}

/**************************************************************************************************
 * Convert linear Adobe RGB (1998) RGBA to gamma-2.4 encoded Adobe RGB (1998) RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgb<F> AdobeRgbLinear<F>::to_adobe_rgb() const noexcept {
	return AdobeRgb<F>(detail::linear_to_adobe_rgb_transfer(red), detail::linear_to_adobe_rgb_transfer(green), detail::linear_to_adobe_rgb_transfer(blue), alpha);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Adobe RGB (1998) RGBA to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> AdobeRgb<F>::to_xyz() const noexcept {
	return to_linear().to_xyz();
}

/**************************************************************************************************
 * Convert linear Adobe RGB (1998) RGBA to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> AdobeRgbLinear<F>::to_xyz() const noexcept {
	const F xyz_x = detail::component_constant<F>(0.57667) * red + detail::component_constant<F>(0.18556) * green + detail::component_constant<F>(0.18823) * blue;
	const F xyz_y = detail::component_constant<F>(0.29734) * red + detail::component_constant<F>(0.62736) * green + detail::component_constant<F>(0.07529) * blue;
	const F xyz_z = detail::component_constant<F>(0.02703) * red + detail::component_constant<F>(0.07069) * green + detail::component_constant<F>(0.99134) * blue;
	return Xyz<F>(xyz_x, xyz_y, xyz_z, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to linear Adobe RGB (1998) RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgbLinear<F> Xyz<F>::to_adobe_rgb_linear() const noexcept {
	const F red = detail::component_constant<F>(2.04159) * x + detail::component_constant<F>(-0.56501) * y + detail::component_constant<F>(-0.34473) * z;
	const F green = detail::component_constant<F>(-0.96924) * x + detail::component_constant<F>(1.87597) * y + detail::component_constant<F>(0.04156) * z;
	const F blue = detail::component_constant<F>(0.01344) * x + detail::component_constant<F>(-0.11836) * y + detail::component_constant<F>(1.01517) * z;
	return AdobeRgbLinear<F>(red, green, blue, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to gamma-2.4 encoded Adobe RGB (1998) RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgb<F> Xyz<F>::to_adobe_rgb() const noexcept {
	return to_adobe_rgb_linear().to_adobe_rgb();
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Adobe RGB (1998) RGBA to Adobe RGB (1998) HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgbHsv<F> AdobeRgb<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<AdobeRgbHsv<F>>(*this);
}

/**************************************************************************************************
 * Convert Adobe RGB (1998) HSV to gamma-2.4 encoded Adobe RGB (1998) RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgb<F> AdobeRgbHsv<F>::to_adobe_rgb() const noexcept {
	return detail::hsv_to_rgb<AdobeRgb<F>>(*this);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Adobe RGB (1998) RGBA to Adobe RGB (1998) HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgbHsl<F> AdobeRgb<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<AdobeRgbHsl<F>>(*this);
}

/**************************************************************************************************
 * Convert Adobe RGB (1998) HSL to gamma-2.4 encoded Adobe RGB (1998) RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgb<F> AdobeRgbHsl<F>::to_adobe_rgb() const noexcept {
	return detail::hsl_to_rgb<AdobeRgb<F>>(*this);
}

/**************************************************************************************************
 * Convert linear Adobe RGB (1998) RGBA to linear Adobe RGB (1998) HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgbLinearHsv<F> AdobeRgbLinear<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<AdobeRgbLinearHsv<F>>(*this);
}

/**************************************************************************************************
 * Convert linear Adobe RGB (1998) HSV to linear Adobe RGB (1998) RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgbLinear<F> AdobeRgbLinearHsv<F>::to_adobe_rgb_linear() const noexcept {
	return detail::hsv_to_rgb<AdobeRgbLinear<F>>(*this);
}

/**************************************************************************************************
 * Convert linear Adobe RGB (1998) RGBA to linear Adobe RGB (1998) HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgbLinearHsl<F> AdobeRgbLinear<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<AdobeRgbLinearHsl<F>>(*this);
}

/**************************************************************************************************
 * Convert linear Adobe RGB (1998) HSL to linear Adobe RGB (1998) RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgbLinear<F> AdobeRgbLinearHsl<F>::to_adobe_rgb_linear() const noexcept {
	return detail::hsl_to_rgb<AdobeRgbLinear<F>>(*this);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded ProPhoto RGB RGBA to linear ProPhoto RGB RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgb<F>::to_linear() const noexcept {
	return ProPhotoRgbLinear<F>(detail::prophoto_rgb_transfer_to_linear(red), detail::prophoto_rgb_transfer_to_linear(green), detail::prophoto_rgb_transfer_to_linear(blue), alpha);
}

/**************************************************************************************************
 * Convert linear ProPhoto RGB RGBA to gamma-2.4 encoded ProPhoto RGB RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> ProPhotoRgbLinear<F>::to_prophoto_rgb() const noexcept {
	return ProPhotoRgb<F>(detail::linear_to_prophoto_rgb_transfer(red), detail::linear_to_prophoto_rgb_transfer(green), detail::linear_to_prophoto_rgb_transfer(blue), alpha);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded ProPhoto RGB RGBA to CIE XYZ D50.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzD50<F> ProPhotoRgb<F>::to_xyz_d50() const noexcept {
	return to_linear().to_xyz_d50();
}

/**************************************************************************************************
 * Convert linear ProPhoto RGB RGBA to CIE XYZ D50.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzD50<F> ProPhotoRgbLinear<F>::to_xyz_d50() const noexcept {
	const F xyz_x = detail::component_constant<F>(0.79776664490064230) * red + detail::component_constant<F>(0.13518129740053308) * green + detail::component_constant<F>(0.03134773412839220) * blue;
	const F xyz_y = detail::component_constant<F>(0.28807482881940130) * red + detail::component_constant<F>(0.71183523424187300) * green + detail::component_constant<F>(0.00008993693872564) * blue;
	const F xyz_z = detail::component_constant<F>(0.82510460251046020) * blue;
	return XyzD50<F>(xyz_x, xyz_y, xyz_z, alpha);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded ProPhoto RGB RGBA to CIE XYZ D65.
 * Uses Bradford chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> ProPhotoRgb<F>::to_xyz() const noexcept {
	return to_linear().to_xyz();
}

/**************************************************************************************************
 * Convert linear ProPhoto RGB RGBA to CIE XYZ D65.
 * Uses Bradford chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> ProPhotoRgbLinear<F>::to_xyz() const noexcept {
	return to_xyz_d50().to_xyz();
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded ProPhoto RGB RGBA to CIE XYZ D65.
 * Uses CAT02 chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> ProPhotoRgb<F>::to_xyz_with_cat02() const noexcept {
	return to_linear().to_xyz_with_cat02();
}

/**************************************************************************************************
 * Convert linear ProPhoto RGB RGBA to CIE XYZ D65.
 * Uses CAT02 chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> ProPhotoRgbLinear<F>::to_xyz_with_cat02() const noexcept {
	return to_xyz_d50().to_xyz_with_cat02();
}

/**************************************************************************************************
 * Convert CIE XYZ D50 to linear ProPhoto RGB RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> XyzD50<F>::to_prophoto_rgb_linear() const noexcept {
	const F red = detail::component_constant<F>(1.3457868816471583) * x + detail::component_constant<F>(-0.25557208737979464) * y + detail::component_constant<F>(-0.05110186497554526) * z;
	const F green = detail::component_constant<F>(-0.5446307051249019) * x + detail::component_constant<F>(1.5082477428451468) * y + detail::component_constant<F>(0.02052744743642139) * z;
	const F blue = detail::component_constant<F>(1.2119675456389454) * z;
	return ProPhotoRgbLinear<F>(red, green, blue, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D50 to gamma-2.4 encoded ProPhoto RGB RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> XyzD50<F>::to_prophoto_rgb() const noexcept {
	return to_prophoto_rgb_linear().to_prophoto_rgb();
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to linear ProPhoto RGB RGBA.
 * Uses Bradford chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> Xyz<F>::to_prophoto_rgb_linear() const noexcept {
	return to_xyz_d50().to_prophoto_rgb_linear();
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to gamma-2.4 encoded ProPhoto RGB RGBA.
 * Uses Bradford chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> Xyz<F>::to_prophoto_rgb() const noexcept {
	return to_prophoto_rgb_linear().to_prophoto_rgb();
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to linear ProPhoto RGB RGBA.
 * Uses CAT02 chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> Xyz<F>::to_prophoto_rgb_linear_with_cat02() const noexcept {
	return to_xyz_d50_with_cat02().to_prophoto_rgb_linear();
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to gamma-2.4 encoded ProPhoto RGB RGBA.
 * Uses CAT02 chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> Xyz<F>::to_prophoto_rgb_with_cat02() const noexcept {
	return to_prophoto_rgb_linear_with_cat02().to_prophoto_rgb();
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded ProPhoto RGB RGBA to ProPhoto RGB HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbHsv<F> ProPhotoRgb<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<ProPhotoRgbHsv<F>>(*this);
}

/**************************************************************************************************
 * Convert ProPhoto RGB HSV to gamma-2.4 encoded ProPhoto RGB RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> ProPhotoRgbHsv<F>::to_prophoto_rgb() const noexcept {
	return detail::hsv_to_rgb<ProPhotoRgb<F>>(*this);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded ProPhoto RGB RGBA to ProPhoto RGB HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbHsl<F> ProPhotoRgb<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<ProPhotoRgbHsl<F>>(*this);
}

/**************************************************************************************************
 * Convert ProPhoto RGB HSL to gamma-2.4 encoded ProPhoto RGB RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> ProPhotoRgbHsl<F>::to_prophoto_rgb() const noexcept {
	return detail::hsl_to_rgb<ProPhotoRgb<F>>(*this);
}

/**************************************************************************************************
 * Convert linear ProPhoto RGB RGBA to linear ProPhoto RGB HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinearHsv<F> ProPhotoRgbLinear<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<ProPhotoRgbLinearHsv<F>>(*this);
}

/**************************************************************************************************
 * Convert linear ProPhoto RGB HSV to linear ProPhoto RGB RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinearHsv<F>::to_prophoto_rgb_linear() const noexcept {
	return detail::hsv_to_rgb<ProPhotoRgbLinear<F>>(*this);
}

/**************************************************************************************************
 * Convert linear ProPhoto RGB RGBA to linear ProPhoto RGB HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinearHsl<F> ProPhotoRgbLinear<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<ProPhotoRgbLinearHsl<F>>(*this);
}

/**************************************************************************************************
 * Convert linear ProPhoto RGB HSL to linear ProPhoto RGB RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinearHsl<F>::to_prophoto_rgb_linear() const noexcept {
	return detail::hsl_to_rgb<ProPhotoRgbLinear<F>>(*this);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Rec.709 RGBA to linear Rec.709 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Oetf<F>::to_linear() const noexcept {
	return Rec709Linear<F>(detail::rec709_oetf_to_linear(red), detail::rec709_oetf_to_linear(green), detail::rec709_oetf_to_linear(blue), alpha);
}

/**************************************************************************************************
 * Convert linear Rec.709 RGBA to gamma-2.4 encoded Rec.709 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Oetf<F> Rec709Linear<F>::to_rec709_oetf() const noexcept {
	return Rec709Oetf<F>(detail::linear_to_rec709_oetf(red), detail::linear_to_rec709_oetf(green), detail::linear_to_rec709_oetf(blue), alpha);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Rec.709 RGBA to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Rec709Oetf<F>::to_xyz() const noexcept {
	return to_linear().to_xyz();
}

/**************************************************************************************************
 * Convert linear Rec.709 RGBA to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Rec709Linear<F>::to_xyz() const noexcept {
	const F xyz_x = detail::component_constant<F>(0.4123907992659593) * red + detail::component_constant<F>(0.357584339383878) * green + detail::component_constant<F>(0.1804807884018343) * blue;
	const F xyz_y = detail::component_constant<F>(0.2126390058715103) * red + detail::component_constant<F>(0.7151686787677559) * green + detail::component_constant<F>(0.07219231536073371) * blue;
	const F xyz_z = detail::component_constant<F>(0.01933081871559182) * red + detail::component_constant<F>(0.1191947797946259) * green + detail::component_constant<F>(0.9505321522496606) * blue;
	return Xyz<F>(xyz_x, xyz_y, xyz_z, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to linear Rec.709 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Xyz<F>::to_rec709_linear() const noexcept {
	const F red = detail::component_constant<F>(3.240969941904523) * x + detail::component_constant<F>(-1.537383177570094) * y + detail::component_constant<F>(-0.4986107602930034) * z;
	const F green = detail::component_constant<F>(-0.9692436362808798) * x + detail::component_constant<F>(1.875967501507721) * y + detail::component_constant<F>(0.04155505740717563) * z;
	const F blue = detail::component_constant<F>(0.05563007969699364) * x + detail::component_constant<F>(-0.2039769588889765) * y + detail::component_constant<F>(1.056971514242879) * z;
	return Rec709Linear<F>(red, green, blue, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to gamma-2.4 encoded Rec.709 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Oetf<F> Xyz<F>::to_rec709_oetf() const noexcept {
	return to_rec709_linear().to_rec709_oetf();
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Rec.709 RGBA to Rec.709 HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709OetfHsv<F> Rec709Oetf<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<Rec709OetfHsv<F>>(*this);
}

/**************************************************************************************************
 * Convert Rec.709 HSV to gamma-2.4 encoded Rec.709 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Oetf<F> Rec709OetfHsv<F>::to_rec709_oetf() const noexcept {
	return detail::hsv_to_rgb<Rec709Oetf<F>>(*this);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Rec.709 RGBA to Rec.709 HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709OetfHsl<F> Rec709Oetf<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<Rec709OetfHsl<F>>(*this);
}

/**************************************************************************************************
 * Convert Rec.709 HSL to gamma-2.4 encoded Rec.709 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Oetf<F> Rec709OetfHsl<F>::to_rec709_oetf() const noexcept {
	return detail::hsl_to_rgb<Rec709Oetf<F>>(*this);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Rec.709 RGBA to linear Rec.709 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Gamma24<F>::to_linear() const noexcept {
	return Rec709Linear<F>(detail::rec709_gamma24_to_linear(red), detail::rec709_gamma24_to_linear(green), detail::rec709_gamma24_to_linear(blue), alpha);
}

/**************************************************************************************************
 * Convert linear Rec.709 RGBA to gamma-2.4 encoded Rec.709 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Gamma24<F> Rec709Linear<F>::to_rec709_gamma24() const noexcept {
	return Rec709Gamma24<F>(detail::linear_to_rec709_gamma24(red), detail::linear_to_rec709_gamma24(green), detail::linear_to_rec709_gamma24(blue), alpha);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Rec.709 RGBA to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Rec709Gamma24<F>::to_xyz() const noexcept {
	return to_linear().to_xyz();
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to gamma-2.4 encoded Rec.709 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Gamma24<F> Xyz<F>::to_rec709_gamma24() const noexcept {
	return to_rec709_linear().to_rec709_gamma24();
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Rec.709 RGBA to Rec.709 HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Gamma24Hsv<F> Rec709Gamma24<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<Rec709Gamma24Hsv<F>>(*this);
}

/**************************************************************************************************
 * Convert Rec.709 gamma-2.4 HSV to gamma-2.4 encoded Rec.709 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Gamma24<F> Rec709Gamma24Hsv<F>::to_rec709_gamma24() const noexcept {
	return detail::hsv_to_rgb<Rec709Gamma24<F>>(*this);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Rec.709 RGBA to Rec.709 HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Gamma24Hsl<F> Rec709Gamma24<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<Rec709Gamma24Hsl<F>>(*this);
}

/**************************************************************************************************
 * Convert Rec.709 gamma-2.4 HSL to gamma-2.4 encoded Rec.709 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Gamma24<F> Rec709Gamma24Hsl<F>::to_rec709_gamma24() const noexcept {
	return detail::hsl_to_rgb<Rec709Gamma24<F>>(*this);
}

/**************************************************************************************************
 * Convert linear Rec.709 RGBA to linear Rec.709 HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709LinearHsv<F> Rec709Linear<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<Rec709LinearHsv<F>>(*this);
}

/**************************************************************************************************
 * Convert linear Rec.709 HSV to linear Rec.709 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709LinearHsv<F>::to_rec709_linear() const noexcept {
	return detail::hsv_to_rgb<Rec709Linear<F>>(*this);
}

/**************************************************************************************************
 * Convert linear Rec.709 RGBA to linear Rec.709 HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709LinearHsl<F> Rec709Linear<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<Rec709LinearHsl<F>>(*this);
}

/**************************************************************************************************
 * Convert linear Rec.709 HSL to linear Rec.709 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709LinearHsl<F>::to_rec709_linear() const noexcept {
	return detail::hsl_to_rgb<Rec709Linear<F>>(*this);
}

/**************************************************************************************************
 * Convert Rec.2020 OETF-encoded RGBA to linear Rec.2020 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Linear<F> Rec2020Oetf<F>::to_linear() const noexcept {
	return Rec2020Linear<F>(detail::rec2020_oetf_to_linear(red), detail::rec2020_oetf_to_linear(green), detail::rec2020_oetf_to_linear(blue), alpha);
}

/**************************************************************************************************
 * Convert linear Rec.2020 RGBA to Rec.2020 OETF-encoded RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Oetf<F> Rec2020Linear<F>::to_rec2020_oetf() const noexcept {
	return Rec2020Oetf<F>(detail::linear_to_rec2020_oetf(red), detail::linear_to_rec2020_oetf(green), detail::linear_to_rec2020_oetf(blue), alpha);
}

/**************************************************************************************************
 * Convert Rec.2020 OETF-encoded RGBA to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Rec2020Oetf<F>::to_xyz() const noexcept {
	return to_linear().to_xyz();
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Rec.2020 RGBA to linear Rec.2020 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Linear<F> Rec2020Gamma24<F>::to_linear() const noexcept {
	return Rec2020Linear<F>(detail::rec2020_gamma24_to_linear(red), detail::rec2020_gamma24_to_linear(green), detail::rec2020_gamma24_to_linear(blue), alpha);
}

/**************************************************************************************************
 * Convert linear Rec.2020 RGBA to gamma-2.4 encoded Rec.2020 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Gamma24<F> Rec2020Linear<F>::to_rec2020_gamma24() const noexcept {
	return Rec2020Gamma24<F>(detail::linear_to_rec2020_gamma24(red), detail::linear_to_rec2020_gamma24(green), detail::linear_to_rec2020_gamma24(blue), alpha);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Rec.2020 RGBA to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Rec2020Gamma24<F>::to_xyz() const noexcept {
	return to_linear().to_xyz();
}

/**************************************************************************************************
 * Convert linear Rec.2020 RGBA to CIE XYZ D65.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Rec2020Linear<F>::to_xyz() const noexcept {
	const F xyz_x = detail::component_constant<F>(0.63695804830129121) * red + detail::component_constant<F>(0.14461690358620841) * green + detail::component_constant<F>(0.16888097516417208) * blue;
	const F xyz_y = detail::component_constant<F>(0.26270021201126698) * red + detail::component_constant<F>(0.67799807151887115) * green + detail::component_constant<F>(0.059301716469861952) * blue;
	const F xyz_z = detail::component_constant<F>(4.9941065744660742e-17) * red + detail::component_constant<F>(0.028072693049087445) * green + detail::component_constant<F>(1.0609850577107909) * blue;
	return Xyz<F>(xyz_x, xyz_y, xyz_z, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to linear Rec.2020 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Linear<F> Xyz<F>::to_rec2020_linear() const noexcept {
	const F red = detail::component_constant<F>(1.7166511879712678) * x + detail::component_constant<F>(-0.35567078377639244) * y + detail::component_constant<F>(-0.2533662813736598) * z;
	const F green = detail::component_constant<F>(-0.66668435183248875) * x + detail::component_constant<F>(1.6164812366349386) * y + detail::component_constant<F>(0.015768545813911117) * z;
	const F blue = detail::component_constant<F>(0.01763985744531079) * x + detail::component_constant<F>(-0.04277061325780853) * y + detail::component_constant<F>(0.94210312123547391) * z;
	return Rec2020Linear<F>(red, green, blue, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to gamma-2.4 encoded Rec.2020 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Gamma24<F> Xyz<F>::to_rec2020_gamma24() const noexcept {
	return to_rec2020_linear().to_rec2020_gamma24();
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to Rec.2020 OETF-encoded RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Oetf<F> Xyz<F>::to_rec2020_oetf() const noexcept {
	return to_rec2020_linear().to_rec2020_oetf();
}

/**************************************************************************************************
 * Convert Rec.2020 OETF-encoded RGBA to Rec.2020 HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020OetfHsv<F> Rec2020Oetf<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<Rec2020OetfHsv<F>>(*this);
}

/**************************************************************************************************
 * Convert Rec.2020 HSV to Rec.2020 OETF-encoded RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Oetf<F> Rec2020OetfHsv<F>::to_rec2020_oetf() const noexcept {
	return detail::hsv_to_rgb<Rec2020Oetf<F>>(*this);
}

/**************************************************************************************************
 * Convert Rec.2020 OETF-encoded RGBA to Rec.2020 HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020OetfHsl<F> Rec2020Oetf<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<Rec2020OetfHsl<F>>(*this);
}

/**************************************************************************************************
 * Convert Rec.2020 HSL to Rec.2020 OETF-encoded RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Oetf<F> Rec2020OetfHsl<F>::to_rec2020_oetf() const noexcept {
	return detail::hsl_to_rgb<Rec2020Oetf<F>>(*this);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Rec.2020 RGBA to Rec.2020 HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Gamma24Hsv<F> Rec2020Gamma24<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<Rec2020Gamma24Hsv<F>>(*this);
}

/**************************************************************************************************
 * Convert Rec.2020 HSV to gamma-2.4 encoded Rec.2020 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Gamma24<F> Rec2020Gamma24Hsv<F>::to_rec2020_gamma24() const noexcept {
	return detail::hsv_to_rgb<Rec2020Gamma24<F>>(*this);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Rec.2020 RGBA to Rec.2020 HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Gamma24Hsl<F> Rec2020Gamma24<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<Rec2020Gamma24Hsl<F>>(*this);
}

/**************************************************************************************************
 * Convert Rec.2020 HSL to gamma-2.4 encoded Rec.2020 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Gamma24<F> Rec2020Gamma24Hsl<F>::to_rec2020_gamma24() const noexcept {
	return detail::hsl_to_rgb<Rec2020Gamma24<F>>(*this);
}

/**************************************************************************************************
 * Convert linear Rec.2020 RGBA to linear Rec.2020 HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020LinearHsv<F> Rec2020Linear<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<Rec2020LinearHsv<F>>(*this);
}

/**************************************************************************************************
 * Convert linear Rec.2020 HSV to linear Rec.2020 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Linear<F> Rec2020LinearHsv<F>::to_rec2020_linear() const noexcept {
	return detail::hsv_to_rgb<Rec2020Linear<F>>(*this);
}

/**************************************************************************************************
 * Convert linear Rec.2020 RGBA to linear Rec.2020 HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020LinearHsl<F> Rec2020Linear<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<Rec2020LinearHsl<F>>(*this);
}

/**************************************************************************************************
 * Convert linear Rec.2020 HSL to linear Rec.2020 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Linear<F> Rec2020LinearHsl<F>::to_rec2020_linear() const noexcept {
	return detail::hsl_to_rgb<Rec2020Linear<F>>(*this);
}

/**************************************************************************************************
 * Convert ACES2065-1 RGBA to CIE XYZ with ACES white.
 * This is the exact ACES2065-1 AP0 RGB to XYZ matrix from SMPTE ST 2065-1.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzAces<F> Aces2065_1<F>::to_xyz_aces() const noexcept {
	const F xyz_x = detail::component_constant<F>(0.95255239589999996) * red + detail::component_constant<F>(0.000093678599999999994) * blue;
	const F xyz_y = detail::component_constant<F>(0.34396644980000002) * red + detail::component_constant<F>(0.72816609659999998) * green + detail::component_constant<F>(-0.072132546399999997) * blue;
	const F xyz_z = detail::component_constant<F>(1.0088251844) * blue;
	return XyzAces<F>(xyz_x, xyz_y, xyz_z, alpha);
}

/**************************************************************************************************
 * Convert ACES2065-1 RGBA to CIE XYZ D65.
 * Converts through ACES-white XYZ and uses Bradford chromatic adaptation to this library's D65 XYZ hub.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Aces2065_1<F>::to_xyz() const noexcept {
	return to_xyz_aces().to_xyz();
}

/**************************************************************************************************
 * Convert ACES2065-1 RGBA to CIE XYZ D65.
 * Converts through ACES-white XYZ and uses CAT02 chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Aces2065_1<F>::to_xyz_with_cat02() const noexcept {
	return to_xyz_aces().to_xyz_with_cat02();
}

/**************************************************************************************************
 * Convert CIE XYZ with ACES white to ACES2065-1 RGBA.
 * This is the exact inverse ACES2065-1 AP0 matrix from SMPTE ST 2065-1.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> XyzAces<F>::to_aces2065_1() const noexcept {
	const F red = detail::component_constant<F>(1.049811017540059) * x + detail::component_constant<F>(-0.00009748450763173491) * z;
	const F green = detail::component_constant<F>(-0.49590302315673573) * x + detail::component_constant<F>(1.3733130458411404) * y + detail::component_constant<F>(0.098240036068043352) * z;
	const F blue = detail::component_constant<F>(0.99125201815292829) * z;
	return Aces2065_1<F>(red, green, blue, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to ACES2065-1 RGBA.
 * Uses Bradford chromatic adaptation to ACES-white XYZ, then applies the exact AP0 matrix inverse.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Xyz<F>::to_aces2065_1() const noexcept {
	return to_xyz_aces().to_aces2065_1();
}

/**************************************************************************************************
 * Convert ACES2065-1 RGBA to ACES2065-1 HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1Hsv<F> Aces2065_1<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<Aces2065_1Hsv<F>>(*this);
}

/**************************************************************************************************
 * Convert ACES2065-1 HSV to ACES2065-1 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1Hsv<F>::to_aces2065_1() const noexcept {
	return detail::hsv_to_rgb<Aces2065_1<F>>(*this);
}

/**************************************************************************************************
 * Convert ACES2065-1 RGBA to ACES2065-1 HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1Hsl<F> Aces2065_1<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<Aces2065_1Hsl<F>>(*this);
}

/**************************************************************************************************
 * Convert ACES2065-1 HSL to ACES2065-1 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1Hsl<F>::to_aces2065_1() const noexcept {
	return detail::hsl_to_rgb<Aces2065_1<F>>(*this);
}

/**************************************************************************************************
 * Convert ACEScg RGBA to CIE XYZ with ACES white.
 * This is the exact ACEScg AP1 RGB to XYZ matrix. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzAces<F> Acescg<F>::to_xyz_aces() const noexcept {
	const F xyz_x = detail::component_constant<F>(0.66245418110850551) * red + detail::component_constant<F>(0.13400420645643313) * green + detail::component_constant<F>(0.15618768700490779) * blue;
	const F xyz_y = detail::component_constant<F>(0.27222871678091459) * red + detail::component_constant<F>(0.67408176581114831) * green + detail::component_constant<F>(0.053689517407937051) * blue;
	const F xyz_z = detail::component_constant<F>(-0.0055746494903941091) * red + detail::component_constant<F>(0.004060733528982825) * green + detail::component_constant<F>(1.0103391003129971) * blue;
	return XyzAces<F>(xyz_x, xyz_y, xyz_z, alpha);
}

/**************************************************************************************************
 * Convert ACEScg RGBA to CIE XYZ D65.
 * Converts through ACES-white XYZ and uses Bradford chromatic adaptation to this library's D65 XYZ hub.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Acescg<F>::to_xyz() const noexcept {
	return to_xyz_aces().to_xyz();
}

/**************************************************************************************************
 * Convert ACEScg RGBA to CIE XYZ D65.
 * Converts through ACES-white XYZ and uses CAT02 chromatic adaptation. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Acescg<F>::to_xyz_with_cat02() const noexcept {
	return to_xyz_aces().to_xyz_with_cat02();
}

/**************************************************************************************************
 * Convert CIE XYZ with ACES white to ACEScg RGBA.
 * This is the exact inverse ACEScg AP1 matrix. Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> XyzAces<F>::to_acescg() const noexcept {
	const F red = detail::component_constant<F>(1.6410233796943254) * x + detail::component_constant<F>(-0.32480329418478993) * y + detail::component_constant<F>(-0.23642469523761217) * z;
	const F green = detail::component_constant<F>(-0.66366285872298303) * x + detail::component_constant<F>(1.6153315916573381) * y + detail::component_constant<F>(0.016756347685530141) * z;
	const F blue = detail::component_constant<F>(0.011721894328375374) * x + detail::component_constant<F>(-0.0082844419962374086) * y + detail::component_constant<F>(0.98839485853902154) * z;
	return Acescg<F>(red, green, blue, alpha);
}

/**************************************************************************************************
 * Convert CIE XYZ D65 to ACEScg RGBA.
 * Uses Bradford chromatic adaptation to ACES-white XYZ, then applies the exact AP1 matrix inverse.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Xyz<F>::to_acescg() const noexcept {
	return to_xyz_aces().to_acescg();
}

/**************************************************************************************************
 * Convert ACES2065-1 AP0 RGB to ACEScg AP1 RGB.
 * Both spaces use the ACES white point, so this is a direct RGB matrix transform.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Aces2065_1<F>::to_acescg() const noexcept {
	const F acescg_red = detail::component_constant<F>(1.4514393160716577) * red + detail::component_constant<F>(-0.23651074688936011) * green + detail::component_constant<F>(-0.21492856930836363) * blue;
	const F acescg_green = detail::component_constant<F>(-0.076553773314262663) * red + detail::component_constant<F>(1.1762296998117889) * green + detail::component_constant<F>(-0.099675926450360497) * blue;
	const F acescg_blue = detail::component_constant<F>(0.0083161484249607738) * red + detail::component_constant<F>(-0.0060324497909093073) * green + detail::component_constant<F>(0.99771630141298207) * blue;
	return Acescg<F>(acescg_red, acescg_green, acescg_blue, alpha);
}

/**************************************************************************************************
 * Convert ACEScg AP1 RGB to ACES2065-1 AP0 RGB.
 * Both spaces use the ACES white point, so this is a direct RGB matrix transform.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Acescg<F>::to_aces2065_1() const noexcept {
	const F aces2065_1_red = detail::component_constant<F>(0.69545224135856754) * red + detail::component_constant<F>(0.14067869647073031) * green + detail::component_constant<F>(0.16386906221356926) * blue;
	const F aces2065_1_green = detail::component_constant<F>(0.044794563352499497) * red + detail::component_constant<F>(0.85967111844296851) * green + detail::component_constant<F>(0.095534318210286157) * blue;
	const F aces2065_1_blue = detail::component_constant<F>(-0.0055258825581107627) * red + detail::component_constant<F>(0.0040252103059766325) * green + detail::component_constant<F>(1.0015006722516306) * blue;
	return Aces2065_1<F>(aces2065_1_red, aces2065_1_green, aces2065_1_blue, alpha);
}

/**************************************************************************************************
 * Convert ACEScg RGBA to ACEScg HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AcescgHsv<F> Acescg<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<AcescgHsv<F>>(*this);
}

/**************************************************************************************************
 * Convert ACEScg HSV to ACEScg RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> AcescgHsv<F>::to_acescg() const noexcept {
	return detail::hsv_to_rgb<Acescg<F>>(*this);
}

/**************************************************************************************************
 * Convert ACEScg RGBA to ACEScg HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AcescgHsl<F> Acescg<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<AcescgHsl<F>>(*this);
}

/**************************************************************************************************
 * Convert ACEScg HSL to ACEScg RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> AcescgHsl<F>::to_acescg() const noexcept {
	return detail::hsl_to_rgb<Acescg<F>>(*this);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Display P3 RGBA to Display P3 HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Hsv<F> DisplayP3<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<DisplayP3Hsv<F>>(*this);
}

/**************************************************************************************************
 * Convert Display P3 HSV to gamma-2.4 encoded Display P3 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3<F> DisplayP3Hsv<F>::to_display_p3() const noexcept {
	return detail::hsv_to_rgb<DisplayP3<F>>(*this);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded Display P3 RGBA to Display P3 HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Hsl<F> DisplayP3<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<DisplayP3Hsl<F>>(*this);
}

/**************************************************************************************************
 * Convert Display P3 HSL to gamma-2.4 encoded Display P3 RGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3<F> DisplayP3Hsl<F>::to_display_p3() const noexcept {
	return detail::hsl_to_rgb<DisplayP3<F>>(*this);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded sRGBA to sRGB HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbHsv<F> Srgb<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<SrgbHsv<F>>(*this);
}

/**************************************************************************************************
 * Convert sRGB HSV to gamma-2.4 encoded sRGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Srgb<F> SrgbHsv<F>::to_srgb() const noexcept {
	return detail::hsv_to_rgb<Srgb<F>>(*this);
}

/**************************************************************************************************
 * Convert gamma-2.4 encoded sRGBA to sRGB HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbHsl<F> Srgb<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<SrgbHsl<F>>(*this);
}

/**************************************************************************************************
 * Convert sRGB HSL to gamma-2.4 encoded sRGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Srgb<F> SrgbHsl<F>::to_srgb() const noexcept {
	return detail::hsl_to_rgb<Srgb<F>>(*this);
}

/**************************************************************************************************
 * Convert linear sRGBA to linear sRGB HSV.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinearHsv<F> SrgbLinear<F>::to_hsv() const noexcept {
	return detail::rgb_to_hsv<SrgbLinearHsv<F>>(*this);
}

/**************************************************************************************************
 * Convert linear sRGB HSV to linear sRGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinearHsv<F>::to_srgb_linear() const noexcept {
	return detail::hsv_to_rgb<SrgbLinear<F>>(*this);
}

/**************************************************************************************************
 * Convert linear sRGBA to linear sRGB HSL.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinearHsl<F> SrgbLinear<F>::to_hsl() const noexcept {
	return detail::rgb_to_hsl<SrgbLinearHsl<F>>(*this);
}

/**************************************************************************************************
 * Convert linear sRGB HSL to linear sRGBA.
 * Alpha is passed through unchanged.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinearHsl<F>::to_srgb_linear() const noexcept {
	return detail::hsl_to_rgb<SrgbLinear<F>>(*this);
}

} // namespace mt
