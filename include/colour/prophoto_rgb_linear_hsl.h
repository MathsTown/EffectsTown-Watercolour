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

Type: ProPhotoRgbLinearHsl

Specification:
- Conventional HSL coordinates over linear-light ProPhoto RGB components.
- Underlying RGB specification: CSS Color 4 predefined prophoto-rgb
  https://www.w3.org/TR/css-color-4/#predefined-prophoto-rgb

Note: This library stores hue angles in the range 0.0 to 1.0; you must manually multiply by 360 to get degrees, and divide degree values by 360 to store them.

Reference white:
- Inherits the ProPhoto RGB D50 reference white from ProPhotoRgbLinear.

Conversions:
- ProPhotoRgbLinear::to_hsl() applies the common RGB to HSL transform to linear-light ProPhoto RGB values.
- to_prophoto_rgb_linear() applies the inverse HSL to RGB transform.
- No transfer conversion, RGB-to-XYZ matrix conversion, or chromatic adaptation is performed by this type.
- wrap_hue() wraps hue to a 0..1 turn.

Components:
- hue: 0..1 turn.
- saturation: normally 0..1.
- lightness: normally 0..1 for display-referred values.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, HSL, HSLA.
- Operations: clamp(), no_negatives(), sanitise_floats(), wrap_hue().
- Conversion: to_prophoto_rgb_linear().

*********************************************************************************************************/
#pragma once

#include <cmath>
#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"

namespace mt {

// Forward declaration of the linear ProPhoto RGB type.
template <ColourComponent F> struct ProPhotoRgbLinear;

/**************************************************************************************************
 * HSL coordinates for linear ProPhoto RGB.
 * Colourspace : Linear ProPhoto RGB HSL
 * RGB Source  : https://www.w3.org/TR/css-color-4/#predefined-prophoto-rgb
 * HSL Model   : HSL transform applied to linear ProPhoto RGB components.
 * Hue         : 0..1 turn
 * Alpha       : Passed through unchanged by conversions.
 * ************************************************************************************************/
template <ColourComponent F> struct ProPhotoRgbLinearHsl {
	// Component type stored in each channel.
	using ComponentType = F;

	// Hue component, measured as a 0..1 turn.
	F hue{};

	// Saturation component.
	F saturation{};

	// Lightness component.
	F lightness{};

	// Alpha component.
	F alpha{ F(1) };

	// Construct black with full alpha.
	ProPhotoRgbLinearHsl() = default;

	// Default destructor.
	~ProPhotoRgbLinearHsl() = default;

	// Construct with HSL values and full alpha.
	ProPhotoRgbLinearHsl(F hue, F saturation, F lightness) noexcept : hue(hue), saturation(saturation), lightness(lightness), alpha(F(1)) { }

	// Construct with HSLA values.
	ProPhotoRgbLinearHsl(F hue, F saturation, F lightness, F alpha) noexcept : hue(hue), saturation(saturation), lightness(lightness), alpha(alpha) { }

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	ProPhotoRgbLinearHsl clamp() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	ProPhotoRgbLinearHsl no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	ProPhotoRgbLinearHsl sanitise_floats() const noexcept;

	// Wrap hue to the 0..1 range.
	[[nodiscard]]
	ProPhotoRgbLinearHsl wrap_hue() const noexcept;

	// Convert to linear ProPhoto RGB RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgbLinear<F> to_prophoto_rgb_linear() const noexcept;
};

/**************************************************************************************************
 * Return this HSL colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinearHsl<F> ProPhotoRgbLinearHsl<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return ProPhotoRgbLinearHsl(max(zero, min(hue, one)), max(zero, min(saturation, one)), max(zero, min(lightness, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this HSL colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinearHsl<F> ProPhotoRgbLinearHsl<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return ProPhotoRgbLinearHsl(max(zero, hue), max(zero, saturation), max(zero, lightness), max(zero, alpha));
}

/**************************************************************************************************
 * Return this HSL colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinearHsl<F> ProPhotoRgbLinearHsl<F>::sanitise_floats() const noexcept {
	return ProPhotoRgbLinearHsl(detail::sanitise_component(hue), detail::sanitise_component(saturation), detail::sanitise_component(lightness), detail::sanitise_component(alpha));
}

/**************************************************************************************************
 * Return this HSL colour with hue wrapped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinearHsl<F> ProPhotoRgbLinearHsl<F>::wrap_hue() const noexcept {
	return ProPhotoRgbLinearHsl(detail::wrap_hue_turn(hue), saturation, lightness, alpha);
}

} // namespace mt
