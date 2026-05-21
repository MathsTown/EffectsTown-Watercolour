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

Type: Rec709Linear

Specification:
- Linear-light RGB using the ITU-R BT.709 primaries and D65 reference white.
- Underlying RGB specification: ITU-R BT.709
  https://www.itu.int/rec/R-REC-BT.709

Reference white:
- Rec.709 uses D65, with chromaticity x = 0.3127, y = 0.3290.
- This implementation converts through this library's CIE XYZ D65 hub.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- to_xyz() uses the Rec.709 D65 RGB-to-XYZ matrix derived from the BT.709 primaries.
- to_rec709_oetf() applies the BT.709 opto-electronic gamma 2.4 transfer function.
- to_rec709_gamma24() applies a pure gamma 2.4 display gamma 2.4 transfer function.
- to_hsv() and to_hsl() are conventional component-space transforms over linear-light Rec.709 values.

Components:
- red, green, blue: linear-light colour channels, normally 0..1 for display-referred values; out-of-range values are preserved unless clamped.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, grey, RGB, RGBA.
- Operations: arithmetic operators, premultiply_alpha(), un_premultiply_alpha(), clamp(), clip(), clip_rgb(), fit_gamut(), no_negatives(), sanitise_floats().
- Conversion: to_rec709_oetf(), to_rec709_gamma24(), to_hsv(), to_hsl(), to_xyz().

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"
#include "generic_rgb.h"

namespace mt {

// Forward declaration of the Rec.709 OETF-encoded type.
template <ColourComponent F> struct Rec709Oetf;

// Forward declaration of the Rec.709 gamma-2.4 encoded type.
template <ColourComponent F> struct Rec709Gamma24;

// Forward declaration of the linear Rec.709 HSV type.
template <ColourComponent F> struct Rec709LinearHsv;

// Forward declaration of the linear Rec.709 HSL type.
template <ColourComponent F> struct Rec709LinearHsl;

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

/**************************************************************************************************
 * Rec709Linear floating point or Simd floating point colour type.
 * Colourspace : Linear Rec.709
 * Definition  : https://www.itu.int/rec/R-REC-BT.709
 * PixelOrder  : RGBA
 * Alpha       : Assumed not-premultiplied for all operations except unpremultiply_alpha().
 * ************************************************************************************************/
template <ColourComponent F> struct Rec709Linear {
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
	Rec709Linear() = default;

	// Default destructor.
	~Rec709Linear() = default;

	// Construct with a grey value and full alpha.
	Rec709Linear(F grey) noexcept : red(grey), green(grey), blue(grey), alpha(F(1)) { }

	// Construct with RGB values and full alpha.
	Rec709Linear(F red, F green, F blue) noexcept : red(red), green(green), blue(blue), alpha(F(1)) { }

	// Construct with linear Rec.709 RGBA values.
	Rec709Linear(F red, F green, F blue, F alpha) noexcept : red(red), green(green), blue(blue), alpha(alpha) { }

	// Explicitly construct from generic RGB channel values.
	explicit Rec709Linear(const GenericRgb<F>& colour) noexcept : red(colour.red), green(colour.green), blue(colour.blue), alpha(colour.alpha) { }

	// Add another linear Rec.709 colour to this colour, preserving this alpha.
	Rec709Linear& operator+=(const Rec709Linear& rhs) noexcept;

	// Subtract another linear Rec.709 colour from this colour, preserving this alpha.
	Rec709Linear& operator-=(const Rec709Linear& rhs) noexcept;

	// Scale this colour by a scalar, preserving alpha.
	Rec709Linear& operator*=(F scalar) noexcept;

	// Divide this colour by a scalar, preserving alpha.
	Rec709Linear& operator/=(F scalar) noexcept;

	// Return this colour plus another linear Rec.709 colour, preserving this alpha.
	[[nodiscard]]
	Rec709Linear operator+(const Rec709Linear& rhs) const noexcept;

	// Return this colour minus another linear Rec.709 colour, preserving this alpha.
	[[nodiscard]]
	Rec709Linear operator-(const Rec709Linear& rhs) const noexcept;

	// Return this colour scaled by a scalar, preserving alpha.
	[[nodiscard]]
	Rec709Linear operator*(F scalar) const noexcept;

	// Return this colour divided by a scalar, preserving alpha.
	[[nodiscard]]
	Rec709Linear operator/(F scalar) const noexcept;

	// Premultiply alpha into the red, green, and blue components.
	[[nodiscard]]
	Rec709Linear premultiply_alpha() const noexcept;

	// Un-premultiply alpha from the red, green, and blue components.
	[[nodiscard]]
	Rec709Linear un_premultiply_alpha() const noexcept;

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	Rec709Linear clamp() const noexcept;

	// Clip all components to the 0..1 range.
	[[nodiscard]]
	Rec709Linear clip() const noexcept;

	// Clip the red, green, and blue components to the 0..1 range, preserving alpha.
	[[nodiscard]]
	Rec709Linear clip_rgb() const noexcept;

	// Fit this colour into its RGB gamut (implemented in gamut.h).
	[[nodiscard]]
	Rec709Linear fit_gamut() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	Rec709Linear no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	Rec709Linear sanitise_floats() const noexcept;

	// Convert to generic RGB channel values.
	[[nodiscard]]
	GenericRgb<F> to_generic_rgb() const noexcept;

	// Convert to gamma-2.4 encoded Rec.709 RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	Rec709Oetf<F> to_rec709_oetf() const noexcept;

	// Convert to gamma-2.4 encoded Rec.709 RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	Rec709Gamma24<F> to_rec709_gamma24() const noexcept;

	// Convert to linear Rec.709 HSV (implemented in colour_conversions.h).
	[[nodiscard]]
	Rec709LinearHsv<F> to_hsv() const noexcept;

	// Convert to linear Rec.709 HSL (implemented in colour_conversions.h).
	[[nodiscard]]
	Rec709LinearHsl<F> to_hsl() const noexcept;

	// Convert to CIE XYZ D65 (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz() const noexcept;
};

/**************************************************************************************************
 * Add another linear Rec.709 colour to this colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline Rec709Linear<F>& Rec709Linear<F>::operator+=(const Rec709Linear<F>& rhs) noexcept {
	red += rhs.red;
	green += rhs.green;
	blue += rhs.blue;
	return *this;
}

/**************************************************************************************************
 * Subtract another linear Rec.709 colour from this colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline Rec709Linear<F>& Rec709Linear<F>::operator-=(const Rec709Linear<F>& rhs) noexcept {
	red -= rhs.red;
	green -= rhs.green;
	blue -= rhs.blue;
	return *this;
}

/**************************************************************************************************
 * Scale this colour by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline Rec709Linear<F>& Rec709Linear<F>::operator*=(F scalar) noexcept {
	red *= scalar;
	green *= scalar;
	blue *= scalar;
	return *this;
}

/**************************************************************************************************
 * Divide this colour by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline Rec709Linear<F>& Rec709Linear<F>::operator/=(F scalar) noexcept {
	red /= scalar;
	green /= scalar;
	blue /= scalar;
	return *this;
}

/**************************************************************************************************
 * Return this colour plus another linear Rec.709 colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Linear<F>::operator+(const Rec709Linear<F>& rhs) const noexcept {
	Rec709Linear result = *this;
	result += rhs;
	return result;
}

/**************************************************************************************************
 * Return this colour minus another linear Rec.709 colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Linear<F>::operator-(const Rec709Linear<F>& rhs) const noexcept {
	Rec709Linear result = *this;
	result -= rhs;
	return result;
}

/**************************************************************************************************
 * Return this colour scaled by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Linear<F>::operator*(F scalar) const noexcept {
	Rec709Linear result = *this;
	result *= scalar;
	return result;
}

/**************************************************************************************************
 * Return this colour divided by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Linear<F>::operator/(F scalar) const noexcept {
	Rec709Linear result = *this;
	result /= scalar;
	return result;
}

/**************************************************************************************************
 * Return a linear Rec.709 colour scaled by a scalar, preserving colour alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> operator*(F scalar, Rec709Linear<F> colour) noexcept {
	colour *= scalar;
	return colour;
}

/**************************************************************************************************
 * Return a SIMD linear Rec.709 colour scaled by a broadcast scalar, preserving colour alpha.
 * ************************************************************************************************/
template <SimdFloat F>
[[nodiscard]]
inline Rec709Linear<F> operator*(typename F::F scalar, Rec709Linear<F> colour) noexcept {
	colour *= F(scalar);
	return colour;
}

/**************************************************************************************************
 * Return this colour with alpha multiplied into the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Linear<F>::premultiply_alpha() const noexcept {
	return Rec709Linear(red * alpha, green * alpha, blue * alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with alpha divided out of the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Linear<F>::un_premultiply_alpha() const noexcept {
	const F safe_alpha = if_equal(alpha, F(0), F(1), alpha);
	return Rec709Linear(red / safe_alpha, green / safe_alpha, blue / safe_alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Linear<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Rec709Linear(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every component clipped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Linear<F>::clip() const noexcept {
	return clamp();
}

/**************************************************************************************************
 * Return this colour with red, green, and blue clipped to the 0..1 range, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Linear<F>::clip_rgb() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Rec709Linear(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), alpha);
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Linear<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return Rec709Linear(max(zero, red), max(zero, green), max(zero, blue), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec709Linear<F> Rec709Linear<F>::sanitise_floats() const noexcept {
	return Rec709Linear(detail::sanitise_component(red), detail::sanitise_component(green), detail::sanitise_component(blue), detail::sanitise_component(alpha));
}

/**************************************************************************************************
 * Return this colour as generic RGB channel values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> Rec709Linear<F>::to_generic_rgb() const noexcept {
	return GenericRgb<F>(red, green, blue, alpha);
}

} // namespace mt
