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

Type: ProPhotoRgbLinear

Specification:
- Linear-light ProPhoto RGB / ROMM RGB.
- Specification: CSS Color 4 predefined prophoto-rgb
  https://www.w3.org/TR/css-color-4/#predefined-prophoto-rgb

Reference white:
- ProPhoto RGB is D50-referenced.
- to_xyz_d50() is the native XYZ conversion; to_xyz() adapts to this library's CIE XYZ D65 hub.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- to_xyz_d50() uses the ProPhoto RGB D50 RGB-to-XYZ matrix.
- to_xyz() uses Bradford chromatic adaptation from D50 to D65.
- to_xyz_with_cat02() provides the explicit CAT02 adaptation path.
- to_prophoto_rgb() and to_transfer() apply the ProPhoto RGB 1.8 gamma transfer function with its linear toe.
- to_hsv() and to_hsl() are conventional component-space transforms over linear-light ProPhoto RGB values.

Components:
- red, green, blue: linear-light colour channels, normally 0..1 for display-referred values; out-of-range values are preserved unless clamped.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, grey, RGB, RGBA.
- Operations: arithmetic operators, premultiply_alpha(), un_premultiply_alpha(), clamp(), clip(), clip_rgb(), fit_gamut(), no_negatives(), sanitise_floats().
- Conversion: to_prophoto_rgb(), to_transfer(), to_hsv(), to_hsl(), to_xyz_d50(), to_xyz(), to_xyz_with_cat02().

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"

namespace mt {

// Forward declaration of the transfer-encoded ProPhoto RGB type.
template <ColourComponent F> struct ProPhotoRgb;

// Forward declaration of the linear ProPhoto RGB HSV type.
template <ColourComponent F> struct ProPhotoRgbLinearHsv;

// Forward declaration of the linear ProPhoto RGB HSL type.
template <ColourComponent F> struct ProPhotoRgbLinearHsl;

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

// Forward declaration of the CIE XYZ D50 type.
template <ColourComponent F> struct XyzD50;

/**************************************************************************************************
 * ProPhotoRgbLinear floating point or Simd floating point colour type.
 * Colourspace : Linear ProPhoto RGB
 * Definition  : https://www.w3.org/TR/css-color-4/#predefined-prophoto-rgb
 * PixelOrder  : RGBA
 * Alpha       : Assumed not-premultiplied for all operations except unpremultiply_alpha().
 * ************************************************************************************************/
template <ColourComponent F> struct ProPhotoRgbLinear {
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
	ProPhotoRgbLinear() = default;

	// Default destructor.
	~ProPhotoRgbLinear() = default;

	// Construct with a grey value and full alpha.
	ProPhotoRgbLinear(F grey) noexcept : red(grey), green(grey), blue(grey), alpha(F(1)) { }

	// Construct with RGB values and full alpha.
	ProPhotoRgbLinear(F red, F green, F blue) noexcept : red(red), green(green), blue(blue), alpha(F(1)) { }

	// Construct with linear ProPhoto RGB RGBA values.
	ProPhotoRgbLinear(F red, F green, F blue, F alpha) noexcept : red(red), green(green), blue(blue), alpha(alpha) { }

	// Add another linear ProPhoto RGB colour to this colour, preserving this alpha.
	ProPhotoRgbLinear& operator+=(const ProPhotoRgbLinear& rhs) noexcept;

	// Subtract another linear ProPhoto RGB colour from this colour, preserving this alpha.
	ProPhotoRgbLinear& operator-=(const ProPhotoRgbLinear& rhs) noexcept;

	// Scale this colour by a scalar, preserving alpha.
	ProPhotoRgbLinear& operator*=(F scalar) noexcept;

	// Divide this colour by a scalar, preserving alpha.
	ProPhotoRgbLinear& operator/=(F scalar) noexcept;

	// Return this colour plus another linear ProPhoto RGB colour, preserving this alpha.
	[[nodiscard]]
	ProPhotoRgbLinear operator+(const ProPhotoRgbLinear& rhs) const noexcept;

	// Return this colour minus another linear ProPhoto RGB colour, preserving this alpha.
	[[nodiscard]]
	ProPhotoRgbLinear operator-(const ProPhotoRgbLinear& rhs) const noexcept;

	// Return this colour scaled by a scalar, preserving alpha.
	[[nodiscard]]
	ProPhotoRgbLinear operator*(F scalar) const noexcept;

	// Return this colour divided by a scalar, preserving alpha.
	[[nodiscard]]
	ProPhotoRgbLinear operator/(F scalar) const noexcept;

	// Premultiply alpha into the red, green, and blue components.
	[[nodiscard]]
	ProPhotoRgbLinear premultiply_alpha() const noexcept;

	// Un-premultiply alpha from the red, green, and blue components.
	[[nodiscard]]
	ProPhotoRgbLinear un_premultiply_alpha() const noexcept;

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	ProPhotoRgbLinear clamp() const noexcept;

	// Clip all components to the 0..1 range.
	[[nodiscard]]
	ProPhotoRgbLinear clip() const noexcept;

	// Clip the red, green, and blue components to the 0..1 range, preserving alpha.
	[[nodiscard]]
	ProPhotoRgbLinear clip_rgb() const noexcept;

	// Fit this colour into its RGB gamut (implemented in gamut.h).
	[[nodiscard]]
	ProPhotoRgbLinear fit_gamut() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	ProPhotoRgbLinear no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	ProPhotoRgbLinear sanitise_floats() const noexcept;

	// Convert to transfer-encoded ProPhoto RGB RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgb<F> to_prophoto_rgb() const noexcept;

	// Convert to the matching transfer-encoded RGB type (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgb<F> to_transfer() const noexcept;

	// Convert to linear ProPhoto RGB HSV (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgbLinearHsv<F> to_hsv() const noexcept;

	// Convert to linear ProPhoto RGB HSL (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgbLinearHsl<F> to_hsl() const noexcept;

	// Convert to CIE XYZ D50 (implemented in colour_conversions.h).
	[[nodiscard]]
	XyzD50<F> to_xyz_d50() const noexcept;

	// Convert to CIE XYZ D65 with Bradford chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz() const noexcept;

	// Convert to CIE XYZ D65 with CAT02 chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz_with_cat02() const noexcept;
};

/**************************************************************************************************
 * Add another linear ProPhoto RGB colour to this colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline ProPhotoRgbLinear<F>& ProPhotoRgbLinear<F>::operator+=(const ProPhotoRgbLinear<F>& rhs) noexcept {
	red += rhs.red;
	green += rhs.green;
	blue += rhs.blue;
	return *this;
}

/**************************************************************************************************
 * Subtract another linear ProPhoto RGB colour from this colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline ProPhotoRgbLinear<F>& ProPhotoRgbLinear<F>::operator-=(const ProPhotoRgbLinear<F>& rhs) noexcept {
	red -= rhs.red;
	green -= rhs.green;
	blue -= rhs.blue;
	return *this;
}

/**************************************************************************************************
 * Scale this colour by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline ProPhotoRgbLinear<F>& ProPhotoRgbLinear<F>::operator*=(F scalar) noexcept {
	red *= scalar;
	green *= scalar;
	blue *= scalar;
	return *this;
}

/**************************************************************************************************
 * Divide this colour by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline ProPhotoRgbLinear<F>& ProPhotoRgbLinear<F>::operator/=(F scalar) noexcept {
	red /= scalar;
	green /= scalar;
	blue /= scalar;
	return *this;
}

/**************************************************************************************************
 * Return this colour plus another linear ProPhoto RGB colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinear<F>::operator+(const ProPhotoRgbLinear<F>& rhs) const noexcept {
	ProPhotoRgbLinear result = *this;
	result += rhs;
	return result;
}

/**************************************************************************************************
 * Return this colour minus another linear ProPhoto RGB colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinear<F>::operator-(const ProPhotoRgbLinear<F>& rhs) const noexcept {
	ProPhotoRgbLinear result = *this;
	result -= rhs;
	return result;
}

/**************************************************************************************************
 * Return this colour scaled by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinear<F>::operator*(F scalar) const noexcept {
	ProPhotoRgbLinear result = *this;
	result *= scalar;
	return result;
}

/**************************************************************************************************
 * Return this colour divided by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinear<F>::operator/(F scalar) const noexcept {
	ProPhotoRgbLinear result = *this;
	result /= scalar;
	return result;
}

/**************************************************************************************************
 * Return a linear ProPhoto RGB colour scaled by a scalar, preserving colour alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> operator*(F scalar, ProPhotoRgbLinear<F> colour) noexcept {
	colour *= scalar;
	return colour;
}

/**************************************************************************************************
 * Return a SIMD linear ProPhoto RGB colour scaled by a broadcast scalar, preserving colour alpha.
 * ************************************************************************************************/
template <SimdFloat F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> operator*(typename F::F scalar, ProPhotoRgbLinear<F> colour) noexcept {
	colour *= F(scalar);
	return colour;
}

/**************************************************************************************************
 * Return this colour with alpha multiplied into the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinear<F>::premultiply_alpha() const noexcept {
	return ProPhotoRgbLinear(red * alpha, green * alpha, blue * alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with alpha divided out of the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinear<F>::un_premultiply_alpha() const noexcept {
	const F safe_alpha = if_equal(alpha, F(0), F(1), alpha);
	return ProPhotoRgbLinear(red / safe_alpha, green / safe_alpha, blue / safe_alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinear<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return ProPhotoRgbLinear(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every component clipped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinear<F>::clip() const noexcept {
	return clamp();
}

/**************************************************************************************************
 * Return this colour with red, green, and blue clipped to the 0..1 range, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinear<F>::clip_rgb() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return ProPhotoRgbLinear(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), alpha);
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinear<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return ProPhotoRgbLinear(max(zero, red), max(zero, green), max(zero, blue), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgbLinear<F> ProPhotoRgbLinear<F>::sanitise_floats() const noexcept {
	return ProPhotoRgbLinear(detail::sanitise_component(red), detail::sanitise_component(green), detail::sanitise_component(blue), detail::sanitise_component(alpha));
}

} // namespace mt
