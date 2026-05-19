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

Type: Acescg

Specification:
- ACEScg scene-linear RGB using AP1 primaries and the ACES white point.
- Specification:
  https://docs.acescentral.com/encodings/acescg/

Reference white:
- ACES white point, x = 0.32168, y = 0.33767.
- ACEScg uses the same white point as ACES2065-1.
- to_xyz_aces() is the native ACES-white XYZ conversion; to_xyz() adapts to this library's CIE XYZ D65 hub.

Conversions:
- to_xyz_aces() uses the ACEScg AP1 RGB-to-XYZ matrix as specified.
- to_xyz() adapts ACES-white XYZ to D65 using Bradford chromatic adaptation.
- to_xyz_with_cat02() provides the explicit CAT02 adaptation path.
- to_aces2065_1() applies the specified AP1 to AP0 matrix; no chromatic adaptation is involved.
- to_hsv() and to_hsl() are conventional component-space transforms over scene-linear AP1 values.

Components:
- red, green, blue: scene-linear AP1 colour channels; values are not limited to 0..1 by the type.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, grey, RGB, RGBA.
- Operations: arithmetic operators, premultiply_alpha(), un_premultiply_alpha(), clamp(), clip(), clip_rgb(), fit_gamut(), no_negatives(), sanitise_floats().
- Conversion: to_hsv(), to_hsl(), to_xyz(), to_xyz_with_cat02(), to_xyz_aces(), to_aces2065_1().

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"
#include "generic_rgb.h"

namespace mt {

// Forward declaration of the linear ACEScg HSV type.
template <ColourComponent F> struct AcescgHsv;

// Forward declaration of the linear ACEScg HSL type.
template <ColourComponent F> struct AcescgHsl;

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

// Forward declaration of the CIE XYZ ACES white type.
template <ColourComponent F> struct XyzAces;

// Forward declaration of the ACES2065-1 type.
template <ColourComponent F> struct Aces2065_1;

/**************************************************************************************************
 * Acescg floating point or Simd floating point colour type.
 * Colourspace : ACEScg, linear AP1 RGB
 * Definition  : https://docs.acescentral.com/encodings/acescg/
 * PixelOrder  : RGBA
 * Alpha       : Assumed not-premultiplied for all operations except unpremultiply_alpha().
 * ************************************************************************************************/
template <ColourComponent F> struct Acescg {
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
	Acescg() = default;

	// Default destructor.
	~Acescg() = default;

	// Construct with a grey value and full alpha.
	Acescg(F grey) noexcept : red(grey), green(grey), blue(grey), alpha(F(1)) { }

	// Construct with RGB values and full alpha.
	Acescg(F red, F green, F blue) noexcept : red(red), green(green), blue(blue), alpha(F(1)) { }

	// Construct with linear ACEScg RGBA values.
	Acescg(F red, F green, F blue, F alpha) noexcept : red(red), green(green), blue(blue), alpha(alpha) { }

	// Explicitly construct from generic RGB channel values.
	explicit Acescg(const GenericRgb<F>& colour) noexcept : red(colour.red), green(colour.green), blue(colour.blue), alpha(colour.alpha) { }

	// Add another ACEScg colour to this colour, preserving this alpha.
	Acescg& operator+=(const Acescg& rhs) noexcept;

	// Subtract another ACEScg colour from this colour, preserving this alpha.
	Acescg& operator-=(const Acescg& rhs) noexcept;

	// Scale this colour by a scalar, preserving alpha.
	Acescg& operator*=(F scalar) noexcept;

	// Divide this colour by a scalar, preserving alpha.
	Acescg& operator/=(F scalar) noexcept;

	// Return this colour plus another ACEScg colour, preserving this alpha.
	[[nodiscard]]
	Acescg operator+(const Acescg& rhs) const noexcept;

	// Return this colour minus another ACEScg colour, preserving this alpha.
	[[nodiscard]]
	Acescg operator-(const Acescg& rhs) const noexcept;

	// Return this colour scaled by a scalar, preserving alpha.
	[[nodiscard]]
	Acescg operator*(F scalar) const noexcept;

	// Return this colour divided by a scalar, preserving alpha.
	[[nodiscard]]
	Acescg operator/(F scalar) const noexcept;

	// Premultiply alpha into the red, green, and blue components.
	[[nodiscard]]
	Acescg premultiply_alpha() const noexcept;

	// Un-premultiply alpha from the red, green, and blue components.
	[[nodiscard]]
	Acescg un_premultiply_alpha() const noexcept;

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	Acescg clamp() const noexcept;

	// Clip all components to the 0..1 range.
	[[nodiscard]]
	Acescg clip() const noexcept;

	// Clip the red, green, and blue components to the 0..1 range, preserving alpha.
	[[nodiscard]]
	Acescg clip_rgb() const noexcept;

	// Fit this colour into its RGB gamut (implemented in gamut.h).
	[[nodiscard]]
	Acescg fit_gamut() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	Acescg no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	Acescg sanitise_floats() const noexcept;

	// Convert to generic RGB channel values.
	[[nodiscard]]
	GenericRgb<F> to_generic_rgb() const noexcept;

	// Convert to linear ACEScg HSV (implemented in colour_conversions.h).
	[[nodiscard]]
	AcescgHsv<F> to_hsv() const noexcept;

	// Convert to linear ACEScg HSL (implemented in colour_conversions.h).
	[[nodiscard]]
	AcescgHsl<F> to_hsl() const noexcept;

	// Convert to CIE XYZ D65 (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz() const noexcept;

	// Convert to CIE XYZ D65 with CAT02 chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz_with_cat02() const noexcept;

	// Convert to CIE XYZ with ACES white using the exact ACEScg AP1 matrix (implemented in colour_conversions.h).
	[[nodiscard]]
	XyzAces<F> to_xyz_aces() const noexcept;

	// Convert to ACES2065-1 AP0 RGB (implemented in colour_conversions.h).
	[[nodiscard]]
	Aces2065_1<F> to_aces2065_1() const noexcept;
};

/**************************************************************************************************
 * Add another ACEScg colour to this colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline Acescg<F>& Acescg<F>::operator+=(const Acescg<F>& rhs) noexcept {
	red += rhs.red;
	green += rhs.green;
	blue += rhs.blue;
	return *this;
}

/**************************************************************************************************
 * Subtract another ACEScg colour from this colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline Acescg<F>& Acescg<F>::operator-=(const Acescg<F>& rhs) noexcept {
	red -= rhs.red;
	green -= rhs.green;
	blue -= rhs.blue;
	return *this;
}

/**************************************************************************************************
 * Scale this colour by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline Acescg<F>& Acescg<F>::operator*=(F scalar) noexcept {
	red *= scalar;
	green *= scalar;
	blue *= scalar;
	return *this;
}

/**************************************************************************************************
 * Divide this colour by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F> inline Acescg<F>& Acescg<F>::operator/=(F scalar) noexcept {
	red /= scalar;
	green /= scalar;
	blue /= scalar;
	return *this;
}

/**************************************************************************************************
 * Return this colour plus another ACEScg colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Acescg<F>::operator+(const Acescg<F>& rhs) const noexcept {
	Acescg result = *this;
	result += rhs;
	return result;
}

/**************************************************************************************************
 * Return this colour minus another ACEScg colour, preserving this alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Acescg<F>::operator-(const Acescg<F>& rhs) const noexcept {
	Acescg result = *this;
	result -= rhs;
	return result;
}

/**************************************************************************************************
 * Return this colour scaled by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Acescg<F>::operator*(F scalar) const noexcept {
	Acescg result = *this;
	result *= scalar;
	return result;
}

/**************************************************************************************************
 * Return this colour divided by a scalar, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Acescg<F>::operator/(F scalar) const noexcept {
	Acescg result = *this;
	result /= scalar;
	return result;
}

/**************************************************************************************************
 * Return an ACEScg colour scaled by a scalar, preserving colour alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> operator*(F scalar, Acescg<F> colour) noexcept {
	colour *= scalar;
	return colour;
}

/**************************************************************************************************
 * Return a SIMD ACEScg colour scaled by a broadcast scalar, preserving colour alpha.
 * ************************************************************************************************/
template <SimdFloat F>
[[nodiscard]]
inline Acescg<F> operator*(typename F::F scalar, Acescg<F> colour) noexcept {
	colour *= F(scalar);
	return colour;
}

/**************************************************************************************************
 * Return this colour with alpha multiplied into the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Acescg<F>::premultiply_alpha() const noexcept {
	return Acescg(red * alpha, green * alpha, blue * alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with alpha divided out of the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Acescg<F>::un_premultiply_alpha() const noexcept {
	const F safe_alpha = if_equal(alpha, F(0), F(1), alpha);
	return Acescg(red / safe_alpha, green / safe_alpha, blue / safe_alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Acescg<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Acescg(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every component clipped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Acescg<F>::clip() const noexcept {
	return clamp();
}

/**************************************************************************************************
 * Return this colour with red, green, and blue clipped to the 0..1 range, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Acescg<F>::clip_rgb() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Acescg(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), alpha);
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Acescg<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return Acescg(max(zero, red), max(zero, green), max(zero, blue), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Acescg<F> Acescg<F>::sanitise_floats() const noexcept {
	return Acescg(detail::sanitise_component(red), detail::sanitise_component(green), detail::sanitise_component(blue), detail::sanitise_component(alpha));
}

/**************************************************************************************************
 * Return this colour as generic RGB channel values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> Acescg<F>::to_generic_rgb() const noexcept {
	return GenericRgb<F>(red, green, blue, alpha);
}

} // namespace mt
