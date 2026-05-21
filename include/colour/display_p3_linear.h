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

Type: DisplayP3Linear

Specification:
- Linear-light Display P3 RGB.
- Specification: CSS Color 4 predefined display-p3-linear
  https://www.w3.org/TR/css-color-4/#predefined-display-p3

Reference white:
- Display P3 uses D65, with chromaticity x = 0.3127, y = 0.3290.
- This implementation converts through this library's CIE XYZ D65 hub.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- to_xyz() uses the Display P3 D65 RGB-to-XYZ matrix.
- to_display_p3() applies the sRGB piecewise gamma 2.4 transfer function used by Display P3.

Components:
- red, green, blue: linear-light colour channels, normally 0..1 for display-referred values; out-of-range values are preserved unless clamped.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, grey, RGB, RGBA.
- Operations: arithmetic operators, premultiply_alpha(), un_premultiply_alpha(), clamp(), clip(), clip_rgb(), fit_gamut(), no_negatives(), sanitise_floats().
- Conversion: to_display_p3(), to_xyz().

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"
#include "generic_rgb.h"

namespace mt {

// Forward declaration of the gamma-2.4 encoded Display P3 type.
template <ColourComponent F> struct DisplayP3;

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

/**************************************************************************************************
 * DisplayP3Linear floating point or Simd floating point colour type.
 * Colourspace : Linear Display P3
 * Definition  : https://www.w3.org/TR/css-color-4/#predefined-display-p3-linear
 * PixelOrder  : RGBA
 * Alpha       : Assumed not-premultiplied for all operations except unpremultiply_alpha().
 * ************************************************************************************************/
template <ColourComponent F> struct DisplayP3Linear {
	// Component type stored in each channel.
	using ComponentType = F;

	// Red component.
	F red{};

	// Green component.
	F green{};

	// Blue component.
	F blue{};

	// Alpha component.
	F alpha{ F(1) };

	// Construct black with full alpha.
	DisplayP3Linear() = default;

	// Default destructor.
	~DisplayP3Linear() = default;

	// Construct with a grey value and full alpha.
	DisplayP3Linear(F grey) noexcept : red(grey), green(grey), blue(grey), alpha(F(1)) { }

	// Construct with RGB values and full alpha.
	DisplayP3Linear(F red, F green, F blue) noexcept : red(red), green(green), blue(blue), alpha(F(1)) { }

	// Construct with linear Display P3 RGBA values.
	DisplayP3Linear(F red, F green, F blue, F alpha) noexcept : red(red), green(green), blue(blue), alpha(alpha) { }

	// Explicitly construct from generic RGB channel values.
	explicit DisplayP3Linear(const GenericRgb<F>& colour) noexcept : red(colour.red), green(colour.green), blue(colour.blue), alpha(colour.alpha) { }

	// Add another linear Display P3 colour to this colour, preserving this alpha.
	DisplayP3Linear& operator+=(const DisplayP3Linear& rhs) noexcept;

	// Subtract another linear Display P3 colour from this colour, preserving this alpha.
	DisplayP3Linear& operator-=(const DisplayP3Linear& rhs) noexcept;

	// Scale this colour by a scalar, preserving alpha.
	DisplayP3Linear& operator*=(F scalar) noexcept;

	// Divide this colour by a scalar, preserving alpha.
	DisplayP3Linear& operator/=(F scalar) noexcept;

	// Return this colour plus another linear Display P3 colour, preserving this alpha.
	[[nodiscard]]
	DisplayP3Linear operator+(const DisplayP3Linear& rhs) const noexcept;

	// Return this colour minus another linear Display P3 colour, preserving this alpha.
	[[nodiscard]]
	DisplayP3Linear operator-(const DisplayP3Linear& rhs) const noexcept;

	// Return this colour scaled by a scalar, preserving alpha.
	[[nodiscard]]
	DisplayP3Linear operator*(F scalar) const noexcept;

	// Return this colour divided by a scalar, preserving alpha.
	[[nodiscard]]
	DisplayP3Linear operator/(F scalar) const noexcept;

	// Premultiply alpha into the red, green, and blue components.
	[[nodiscard]]
	DisplayP3Linear premultiply_alpha() const noexcept;

	// Un-premultiply alpha from the red, green, and blue components.
	[[nodiscard]]
	DisplayP3Linear un_premultiply_alpha() const noexcept;

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	DisplayP3Linear clamp() const noexcept;

	// Clip all components to the 0..1 range.
	[[nodiscard]]
	DisplayP3Linear clip() const noexcept;

	// Clip the red, green, and blue components to the 0..1 range, preserving alpha.
	[[nodiscard]]
	DisplayP3Linear clip_rgb() const noexcept;

	// Fit this colour into its RGB gamut (implemented in gamut.h).
	[[nodiscard]]
	DisplayP3Linear fit_gamut() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	DisplayP3Linear no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	DisplayP3Linear sanitise_floats() const noexcept;

	// Convert to generic RGB channel values.
	[[nodiscard]]
	GenericRgb<F> to_generic_rgb() const noexcept;

	// Convert to gamma-2.4 encoded Display P3 RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	DisplayP3<F> to_display_p3() const noexcept;

	// Convert to CIE XYZ D65 (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz() const noexcept;
};

/**************************************************************************************************
 * Add another linear Display P3 colour to this colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline DisplayP3Linear<F>& DisplayP3Linear<F>::operator+=(const DisplayP3Linear<F>& rhs) noexcept {
	red += rhs.red;
	green += rhs.green;
	blue += rhs.blue;
	return *this;
}

/**************************************************************************************************
 * Subtract another linear Display P3 colour from this colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline DisplayP3Linear<F>& DisplayP3Linear<F>::operator-=(const DisplayP3Linear<F>& rhs) noexcept {
	red -= rhs.red;
	green -= rhs.green;
	blue -= rhs.blue;
	return *this;
}

/**************************************************************************************************
 * Scale this colour by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline DisplayP3Linear<F>& DisplayP3Linear<F>::operator*=(F scalar) noexcept {
	red *= scalar;
	green *= scalar;
	blue *= scalar;
	return *this;
}

/**************************************************************************************************
 * Divide this colour by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline DisplayP3Linear<F>& DisplayP3Linear<F>::operator/=(F scalar) noexcept {
	red /= scalar;
	green /= scalar;
	blue /= scalar;
	return *this;
}

/**************************************************************************************************
 * Return this colour plus another linear Display P3 colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> DisplayP3Linear<F>::operator+(const DisplayP3Linear<F>& rhs) const noexcept {
	DisplayP3Linear result = *this;
	result += rhs;
	return result;
}

/**************************************************************************************************
 * Return this colour minus another linear Display P3 colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> DisplayP3Linear<F>::operator-(const DisplayP3Linear<F>& rhs) const noexcept {
	DisplayP3Linear result = *this;
	result -= rhs;
	return result;
}

/**************************************************************************************************
 * Return this colour scaled by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> DisplayP3Linear<F>::operator*(F scalar) const noexcept {
	DisplayP3Linear result = *this;
	result *= scalar;
	return result;
}

/**************************************************************************************************
 * Return this colour divided by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> DisplayP3Linear<F>::operator/(F scalar) const noexcept {
	DisplayP3Linear result = *this;
	result /= scalar;
	return result;
}

/**************************************************************************************************
 * Return a linear Display P3 colour scaled by a scalar, preserving colour alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> operator*(F scalar, DisplayP3Linear<F> colour) noexcept {
	colour *= scalar;
	return colour;
}

/**************************************************************************************************
 * Return a SIMD linear Display P3 colour scaled by a broadcast scalar, preserving colour alpha.
 * ************************************************************************************************/
template <SimdFloat F>
[[nodiscard]]
inline DisplayP3Linear<F> operator*(typename F::F scalar, DisplayP3Linear<F> colour) noexcept {
	colour *= F(scalar);
	return colour;
}

/**************************************************************************************************
 * Return this colour with alpha multiplied into the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> DisplayP3Linear<F>::premultiply_alpha() const noexcept {
	return DisplayP3Linear(red * alpha, green * alpha, blue * alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with alpha divided out of the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> DisplayP3Linear<F>::un_premultiply_alpha() const noexcept {
	const F safe_alpha = if_equal(alpha, F(0), F(1), alpha);
	return DisplayP3Linear(red / safe_alpha, green / safe_alpha, blue / safe_alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> DisplayP3Linear<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return DisplayP3Linear(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every component clipped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> DisplayP3Linear<F>::clip() const noexcept {
	return clamp();
}

/**************************************************************************************************
 * Return this colour with red, green, and blue clipped to the 0..1 range, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> DisplayP3Linear<F>::clip_rgb() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return DisplayP3Linear(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), alpha);
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> DisplayP3Linear<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return DisplayP3Linear(max(zero, red), max(zero, green), max(zero, blue), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline DisplayP3Linear<F> DisplayP3Linear<F>::sanitise_floats() const noexcept {
	return DisplayP3Linear(detail::sanitise_component(red), detail::sanitise_component(green), detail::sanitise_component(blue), detail::sanitise_component(alpha));
}

/**************************************************************************************************
 * Return this colour as generic RGB channel values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> DisplayP3Linear<F>::to_generic_rgb() const noexcept {
	return GenericRgb<F>(red, green, blue, alpha);
}

} // namespace mt
