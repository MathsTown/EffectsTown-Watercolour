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

Type: SrgbLinear

Specification:
- Linear-light sRGB RGB.
- Specification: CSS Color 4 predefined srgb-linear / IEC 61966-2-1 primaries
  https://www.w3.org/TR/css-color-4/#predefined-sRGB

Reference white:
- sRGB uses D65, with chromaticity x = 0.3127, y = 0.3290.
- This implementation converts through this library's CIE XYZ D65 hub.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- to_xyz() uses the sRGB D65 RGB-to-XYZ matrix.
- to_srgb() and to_transfer() apply the sRGB piecewise transfer function, including the CSS Color 4 extended handling for negative values.
- to_hsv() and to_hsl() are conventional component-space transforms over linear-light sRGB values.

Components:
- red, green, blue: linear-light colour channels, normally 0..1 for display-referred values; out-of-range values are preserved unless clamped.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, grey, RGB, RGBA.
- Operations: arithmetic operators, premultiply_alpha(), un_premultiply_alpha(), clamp(), clip(), clip_rgb(), fit_gamut(), no_negatives(), sanitise_floats().
- Conversion: to_srgb(), to_transfer(), to_hsv(), to_hsl(), to_xyz().

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"

namespace mt {

// Forward declaration of the transfer-encoded sRGBA type.
template <ColourComponent F> struct Srgb;

// Forward declaration of the linear sRGB HSV type.
template <ColourComponent F> struct SrgbLinearHsv;

// Forward declaration of the linear sRGB HSL type.
template <ColourComponent F> struct SrgbLinearHsl;

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

/**************************************************************************************************
 * SrgbLinear colour type
 * Colourspace : Linear sRGB
 * Definition  : https://www.w3.org/TR/css-color-4/#predefined-sRGB-linear
 * PixelOrder  : RGBA
 * Alpha       : Assumed not-premultiplied for all operations except unpremultiply_alpha().
 * ************************************************************************************************/
template <ColourComponent F> struct SrgbLinear {
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
	SrgbLinear() = default;

	// Default destructor.
	~SrgbLinear() = default;

	// Construct with a grey value and full alpha.
	SrgbLinear(F grey) noexcept : red(grey), green(grey), blue(grey), alpha(F(1)) { }

	// Construct with red, green, and blue values and full alpha.
	SrgbLinear(F red, F green, F blue) noexcept : red(red), green(green), blue(blue), alpha(F(1)) { }

	// Construct with red, green, blue, and alpha values.
	SrgbLinear(F red, F green, F blue, F alpha) noexcept : red(red), green(green), blue(blue), alpha(alpha) { }

	// Add another linear sRGB colour to this colour, preserving this alpha.
	SrgbLinear& operator+=(const SrgbLinear& rhs) noexcept;

	// Subtract another linear sRGB colour from this colour, preserving this alpha.
	SrgbLinear& operator-=(const SrgbLinear& rhs) noexcept;

	// Scale this colour by a scalar, preserving alpha.
	SrgbLinear& operator*=(F scalar) noexcept;

	// Divide this colour by a scalar, preserving alpha.
	SrgbLinear& operator/=(F scalar) noexcept;

	// Return this colour plus another linear sRGB colour, preserving this alpha.
	[[nodiscard]]
	SrgbLinear operator+(const SrgbLinear& rhs) const noexcept;

	// Return this colour minus another linear sRGB colour, preserving this alpha.
	[[nodiscard]]
	SrgbLinear operator-(const SrgbLinear& rhs) const noexcept;

	// Return this colour scaled by a scalar, preserving alpha.
	[[nodiscard]]
	SrgbLinear operator*(F scalar) const noexcept;

	// Return this colour divided by a scalar, preserving alpha.
	[[nodiscard]]
	SrgbLinear operator/(F scalar) const noexcept;

	// Premultiply alpha into the red, green, and blue components.
	[[nodiscard]]
	SrgbLinear premultiply_alpha() const noexcept;

	// Un-premultiply alpha from the red, green, and blue components.
	[[nodiscard]]
	SrgbLinear un_premultiply_alpha() const noexcept;

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	SrgbLinear clamp() const noexcept;

	// Clip all components to the 0..1 range.
	[[nodiscard]]
	SrgbLinear clip() const noexcept;

	// Clip the red, green, and blue components to the 0..1 range, preserving alpha.
	[[nodiscard]]
	SrgbLinear clip_rgb() const noexcept;

	// Fit this colour into its RGB gamut (implemented in gamut.h).
	[[nodiscard]]
	SrgbLinear fit_gamut() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	SrgbLinear no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	SrgbLinear sanitise_floats() const noexcept;

	// Convert to transfer-encoded sRGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	Srgb<F> to_srgb() const noexcept;

	// Convert to the matching transfer-encoded RGB type (implemented in colour_conversions.h).
	[[nodiscard]]
	Srgb<F> to_transfer() const noexcept;

	// Convert to linear sRGB HSV (implemented in colour_conversions.h).
	[[nodiscard]]
	SrgbLinearHsv<F> to_hsv() const noexcept;

	// Convert to linear sRGB HSL (implemented in colour_conversions.h).
	[[nodiscard]]
	SrgbLinearHsl<F> to_hsl() const noexcept;

	// Convert to CIE XYZ D65 (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz() const noexcept;
};

/**************************************************************************************************
 * Add another linear sRGB colour to this colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline SrgbLinear<F>& SrgbLinear<F>::operator+=(const SrgbLinear<F>& rhs) noexcept {
	red += rhs.red;
	green += rhs.green;
	blue += rhs.blue;
	return *this;
}

/**************************************************************************************************
 * Subtract another linear sRGB colour from this colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline SrgbLinear<F>& SrgbLinear<F>::operator-=(const SrgbLinear<F>& rhs) noexcept {
	red -= rhs.red;
	green -= rhs.green;
	blue -= rhs.blue;
	return *this;
}

/**************************************************************************************************
 * Scale this colour by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline SrgbLinear<F>& SrgbLinear<F>::operator*=(F scalar) noexcept {
	red *= scalar;
	green *= scalar;
	blue *= scalar;
	return *this;
}

/**************************************************************************************************
 * Divide this colour by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline SrgbLinear<F>& SrgbLinear<F>::operator/=(F scalar) noexcept {
	red /= scalar;
	green /= scalar;
	blue /= scalar;
	return *this;
}

/**************************************************************************************************
 * Return this colour plus another linear sRGB colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinear<F>::operator+(const SrgbLinear<F>& rhs) const noexcept {
	SrgbLinear result = *this;
	result += rhs;
	return result;
}

/**************************************************************************************************
 * Return this colour minus another linear sRGB colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinear<F>::operator-(const SrgbLinear<F>& rhs) const noexcept {
	SrgbLinear result = *this;
	result -= rhs;
	return result;
}

/**************************************************************************************************
 * Return this colour scaled by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinear<F>::operator*(F scalar) const noexcept {
	SrgbLinear result = *this;
	result *= scalar;
	return result;
}

/**************************************************************************************************
 * Return this colour divided by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinear<F>::operator/(F scalar) const noexcept {
	SrgbLinear result = *this;
	result /= scalar;
	return result;
}

/**************************************************************************************************
 * Return a linear sRGB colour scaled by a scalar, preserving colour alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> operator*(F scalar, SrgbLinear<F> colour) noexcept {
	colour *= scalar;
	return colour;
}

/**************************************************************************************************
 * Return a SIMD linear sRGB colour scaled by a broadcast scalar, preserving colour alpha.
 * ************************************************************************************************/
template <SimdFloat F>
[[nodiscard]]
inline SrgbLinear<F> operator*(typename F::F scalar, SrgbLinear<F> colour) noexcept {
	colour *= F(scalar);
	return colour;
}

/**************************************************************************************************
 * Return this colour with alpha multiplied into the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinear<F>::premultiply_alpha() const noexcept {
	return SrgbLinear(red * alpha, green * alpha, blue * alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with alpha divided out of the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinear<F>::un_premultiply_alpha() const noexcept {
	const F safe_alpha = if_equal(alpha, F(0), F(1), alpha);
	return SrgbLinear(red / safe_alpha, green / safe_alpha, blue / safe_alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinear<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return SrgbLinear(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every component clipped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinear<F>::clip() const noexcept {
	return clamp();
}

/**************************************************************************************************
 * Return this colour with red, green, and blue clipped to the 0..1 range, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinear<F>::clip_rgb() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return SrgbLinear(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), alpha);
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinear<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return SrgbLinear(max(zero, red), max(zero, green), max(zero, blue), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline SrgbLinear<F> SrgbLinear<F>::sanitise_floats() const noexcept {
	return SrgbLinear(detail::sanitise_component(red), detail::sanitise_component(green), detail::sanitise_component(blue), detail::sanitise_component(alpha));
}

} // namespace mt
