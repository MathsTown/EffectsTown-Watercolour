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

Type: Aces2065_1

Specification:
- ACES2065-1 scene-linear RGB using AP0 primaries and the ACES white point.
- Specification:
  https://docs.acescentral.com/encodings/aces2065-1/

Reference white:
- ACES white point, x = 0.32168, y = 0.33767.
- Do not substitute D65 or a generic D60 value for the native ACES white point.
- to_xyz_aces() is the native ACES-white XYZ conversion; to_xyz() adapts to this library's CIE XYZ D65 hub.

Conversions:
- to_xyz_aces() uses the ACES2065-1 AP0 RGB-to-XYZ matrix as specified.
- to_xyz() adapts ACES-white XYZ to D65 using Bradford chromatic adaptation.
- to_xyz_with_cat02() provides the explicit CAT02 adaptation path.
- to_acescg() applies the specified AP0 to AP1 matrix; no chromatic adaptation is involved.
- to_hsv() and to_hsl() are conventional component-space transforms over scene-linear AP0 values.

Components:
- red, green, blue: scene-linear AP0 colour channels; values are not limited to 0..1 by the type.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, grey, RGB, RGBA.
- Operations: arithmetic operators, premultiply_alpha(), un_premultiply_alpha(), clamp(), clip(), clip_rgb(), fit_gamut(), no_negatives(), sanitise_floats().
- Conversion: to_hsv(), to_hsl(), to_xyz(), to_xyz_with_cat02(), to_xyz_aces(), to_acescg().

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"
#include "generic_rgb.h"

namespace mt {

// Forward declaration of the linear ACES2065-1 HSV type.
template <ColourComponent F> struct Aces2065_1Hsv;

// Forward declaration of the linear ACES2065-1 HSL type.
template <ColourComponent F> struct Aces2065_1Hsl;

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

// Forward declaration of the CIE XYZ ACES white type.
template <ColourComponent F> struct XyzAces;

// Forward declaration of the ACEScg type.
template <ColourComponent F> struct Acescg;

/**************************************************************************************************
 * Aces2065_1 floating point or Simd floating point colour type.
 * Colourspace : ACES2065-1, linear AP0 RGB
 * Definition  : https://docs.acescentral.com/encodings/aces2065-1/
 * PixelOrder  : RGBA
 * Alpha       : Assumed not-premultiplied for all operations except unpremultiply_alpha().
 * ************************************************************************************************/
template <ColourComponent F> struct Aces2065_1 {
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
	Aces2065_1() = default;

	// Default destructor.
	~Aces2065_1() = default;

	// Construct with a grey value and full alpha.
	Aces2065_1(F grey) noexcept : red(grey), green(grey), blue(grey), alpha(F(1)) { }

	// Construct with RGB values and full alpha.
	Aces2065_1(F red, F green, F blue) noexcept : red(red), green(green), blue(blue), alpha(F(1)) { }

	// Construct with linear ACES2065-1 RGBA values.
	Aces2065_1(F red, F green, F blue, F alpha) noexcept : red(red), green(green), blue(blue), alpha(alpha) { }

	// Explicitly construct from generic RGB channel values.
	explicit Aces2065_1(const GenericRgb<F>& colour) noexcept : red(colour.red), green(colour.green), blue(colour.blue), alpha(colour.alpha) { }

	// Add another ACES2065-1 colour to this colour, preserving this alpha.
	Aces2065_1& operator+=(const Aces2065_1& rhs) noexcept;

	// Subtract another ACES2065-1 colour from this colour, preserving this alpha.
	Aces2065_1& operator-=(const Aces2065_1& rhs) noexcept;

	// Scale this colour by a scalar, preserving alpha.
	Aces2065_1& operator*=(F scalar) noexcept;

	// Divide this colour by a scalar, preserving alpha.
	Aces2065_1& operator/=(F scalar) noexcept;

	// Return this colour plus another ACES2065-1 colour, preserving this alpha.
	[[nodiscard]]
	Aces2065_1 operator+(const Aces2065_1& rhs) const noexcept;

	// Return this colour minus another ACES2065-1 colour, preserving this alpha.
	[[nodiscard]]
	Aces2065_1 operator-(const Aces2065_1& rhs) const noexcept;

	// Return this colour scaled by a scalar, preserving alpha.
	[[nodiscard]]
	Aces2065_1 operator*(F scalar) const noexcept;

	// Return this colour divided by a scalar, preserving alpha.
	[[nodiscard]]
	Aces2065_1 operator/(F scalar) const noexcept;

	// Premultiply alpha into the red, green, and blue components.
	[[nodiscard]]
	Aces2065_1 premultiply_alpha() const noexcept;

	// Un-premultiply alpha from the red, green, and blue components.
	[[nodiscard]]
	Aces2065_1 un_premultiply_alpha() const noexcept;

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	Aces2065_1 clamp() const noexcept;

	// Clip all components to the 0..1 range.
	[[nodiscard]]
	Aces2065_1 clip() const noexcept;

	// Clip the red, green, and blue components to the 0..1 range, preserving alpha.
	[[nodiscard]]
	Aces2065_1 clip_rgb() const noexcept;

	// Fit this colour into its RGB gamut (implemented in gamut.h).
	[[nodiscard]]
	Aces2065_1 fit_gamut() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	Aces2065_1 no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	Aces2065_1 sanitise_floats() const noexcept;

	// Convert to generic RGB channel values.
	[[nodiscard]]
	GenericRgb<F> to_generic_rgb() const noexcept;

	// Convert to linear ACES2065-1 HSV (implemented in colour_conversions.h).
	[[nodiscard]]
	Aces2065_1Hsv<F> to_hsv() const noexcept;

	// Convert to linear ACES2065-1 HSL (implemented in colour_conversions.h).
	[[nodiscard]]
	Aces2065_1Hsl<F> to_hsl() const noexcept;

	// Convert to CIE XYZ D65 (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz() const noexcept;

	// Convert to CIE XYZ D65 with CAT02 chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz_with_cat02() const noexcept;

	// Convert to CIE XYZ with ACES white using the exact ACES2065-1 AP0 matrix (implemented in colour_conversions.h).
	[[nodiscard]]
	XyzAces<F> to_xyz_aces() const noexcept;

	// Convert to ACEScg AP1 RGB (implemented in colour_conversions.h).
	[[nodiscard]]
	Acescg<F> to_acescg() const noexcept;
};

/**************************************************************************************************
 * Add another ACES2065-1 colour to this colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline Aces2065_1<F>& Aces2065_1<F>::operator+=(const Aces2065_1<F>& rhs) noexcept {
	red += rhs.red;
	green += rhs.green;
	blue += rhs.blue;
	return *this;
}

/**************************************************************************************************
 * Subtract another ACES2065-1 colour from this colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline Aces2065_1<F>& Aces2065_1<F>::operator-=(const Aces2065_1<F>& rhs) noexcept {
	red -= rhs.red;
	green -= rhs.green;
	blue -= rhs.blue;
	return *this;
}

/**************************************************************************************************
 * Scale this colour by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline Aces2065_1<F>& Aces2065_1<F>::operator*=(F scalar) noexcept {
	red *= scalar;
	green *= scalar;
	blue *= scalar;
	return *this;
}

/**************************************************************************************************
 * Divide this colour by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline Aces2065_1<F>& Aces2065_1<F>::operator/=(F scalar) noexcept {
	red /= scalar;
	green /= scalar;
	blue /= scalar;
	return *this;
}

/**************************************************************************************************
 * Return this colour plus another ACES2065-1 colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1<F>::operator+(const Aces2065_1<F>& rhs) const noexcept {
	Aces2065_1 result = *this;
	result += rhs;
	return result;
}

/**************************************************************************************************
 * Return this colour minus another ACES2065-1 colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1<F>::operator-(const Aces2065_1<F>& rhs) const noexcept {
	Aces2065_1 result = *this;
	result -= rhs;
	return result;
}

/**************************************************************************************************
 * Return this colour scaled by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1<F>::operator*(F scalar) const noexcept {
	Aces2065_1 result = *this;
	result *= scalar;
	return result;
}

/**************************************************************************************************
 * Return this colour divided by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1<F>::operator/(F scalar) const noexcept {
	Aces2065_1 result = *this;
	result /= scalar;
	return result;
}

/**************************************************************************************************
 * Return an ACES2065-1 colour scaled by a scalar, preserving colour alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> operator*(F scalar, Aces2065_1<F> colour) noexcept {
	colour *= scalar;
	return colour;
}

/**************************************************************************************************
 * Return a SIMD ACES2065-1 colour scaled by a broadcast scalar, preserving colour alpha.
 * ************************************************************************************************/
template <SimdFloat F>
[[nodiscard]]
inline Aces2065_1<F> operator*(typename F::F scalar, Aces2065_1<F> colour) noexcept {
	colour *= F(scalar);
	return colour;
}

/**************************************************************************************************
 * Return this colour with alpha multiplied into the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1<F>::premultiply_alpha() const noexcept {
	return Aces2065_1(red * alpha, green * alpha, blue * alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with alpha divided out of the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1<F>::un_premultiply_alpha() const noexcept {
	const F safe_alpha = if_equal(alpha, F(0), F(1), alpha);
	return Aces2065_1(red / safe_alpha, green / safe_alpha, blue / safe_alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Aces2065_1(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every component clipped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1<F>::clip() const noexcept {
	return clamp();
}

/**************************************************************************************************
 * Return this colour with red, green, and blue clipped to the 0..1 range, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1<F>::clip_rgb() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Aces2065_1(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), alpha);
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return Aces2065_1(max(zero, red), max(zero, green), max(zero, blue), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1<F> Aces2065_1<F>::sanitise_floats() const noexcept {
	return Aces2065_1(detail::sanitise_component(red), detail::sanitise_component(green), detail::sanitise_component(blue), detail::sanitise_component(alpha));
}

/**************************************************************************************************
 * Return this colour as generic RGB channel values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> Aces2065_1<F>::to_generic_rgb() const noexcept {
	return GenericRgb<F>(red, green, blue, alpha);
}

} // namespace mt
