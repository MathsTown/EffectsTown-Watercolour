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

Type: ProPhotoRgb

Specification:
- Transfer-encoded ProPhoto RGB / ROMM RGB.
- Specification: CSS Color 4 predefined prophoto-rgb
  https://www.w3.org/TR/css-color-4/#predefined-prophoto-rgb

Reference white:
- ProPhoto RGB is D50-referenced.
- to_xyz_d50() is the native XYZ conversion; to_xyz() adapts to this library's CIE XYZ D65 hub.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- to_linear() applies the ProPhoto RGB 1.8 gamma transfer function with its linear toe, including extended handling for negative values.
- to_xyz_d50() converts through ProPhotoRgbLinear and the ProPhoto RGB D50 RGB-to-XYZ matrix.
- to_xyz() uses Bradford chromatic adaptation from D50 to D65.
- to_xyz_with_cat02() provides the explicit CAT02 adaptation path.
- to_hsv() and to_hsl() are conventional component-space transforms over transfer-encoded ProPhoto RGB values.

Components:
- red, green, blue: nominal 0..1 transfer-encoded colour channels; out-of-range values are preserved unless clamped.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, grey, RGB, RGBA.
- Operations: premultiply_alpha(), un_premultiply_alpha(), clamp(), clip(), clip_rgb(), fit_gamut(), no_negatives(), sanitise_floats().
- Conversion: to_linear(), to_hsv(), to_hsl(), to_xyz_d50(), to_xyz(), to_xyz_with_cat02().

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"
#include "generic_rgb.h"

namespace mt {

// Forward declaration of the linear ProPhoto RGB type.
template <ColourComponent F> struct ProPhotoRgbLinear;

// Forward declaration of the transfer-encoded ProPhoto RGB HSV type.
template <ColourComponent F> struct ProPhotoRgbHsv;

// Forward declaration of the transfer-encoded ProPhoto RGB HSL type.
template <ColourComponent F> struct ProPhotoRgbHsl;

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

// Forward declaration of the CIE XYZ D50 type.
template <ColourComponent F> struct XyzD50;

/**************************************************************************************************
 * ProPhotoRgb floating point or Simd floating point colour type.
 * Colourspace : ProPhoto RGB, transfer-encoded
 * Definition  : https://www.w3.org/TR/css-color-4/#predefined-prophoto-rgb
 * PixelOrder  : RGBA
 * Alpha       : Assumed not-premultiplied for all operations except unpremultiply_alpha().
 * ************************************************************************************************/
template <ColourComponent F> struct ProPhotoRgb {
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
	ProPhotoRgb() = default;

	// Default destructor.
	~ProPhotoRgb() = default;

	// Construct with a grey value and full alpha.
	ProPhotoRgb(F grey) noexcept : red(grey), green(grey), blue(grey), alpha(F(1)) { }

	// Construct with RGB values and full alpha.
	ProPhotoRgb(F red, F green, F blue) noexcept : red(red), green(green), blue(blue), alpha(F(1)) { }

	// Construct with ProPhoto RGB RGBA values.
	ProPhotoRgb(F red, F green, F blue, F alpha) noexcept : red(red), green(green), blue(blue), alpha(alpha) { }

	// Explicitly construct from generic RGB channel values.
	explicit ProPhotoRgb(const GenericRgb<F>& colour) noexcept : red(colour.red), green(colour.green), blue(colour.blue), alpha(colour.alpha) { }

	// Premultiply alpha into the red, green, and blue components.
	[[nodiscard]]
	ProPhotoRgb premultiply_alpha() const noexcept;

	// Un-premultiply alpha from the red, green, and blue components.
	[[nodiscard]]
	ProPhotoRgb un_premultiply_alpha() const noexcept;

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	ProPhotoRgb clamp() const noexcept;

	// Clip all components to the 0..1 range.
	[[nodiscard]]
	ProPhotoRgb clip() const noexcept;

	// Clip the red, green, and blue components to the 0..1 range, preserving alpha.
	[[nodiscard]]
	ProPhotoRgb clip_rgb() const noexcept;

	// Fit this colour into its RGB gamut (implemented in gamut.h).
	[[nodiscard]]
	ProPhotoRgb fit_gamut() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	ProPhotoRgb no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	ProPhotoRgb sanitise_floats() const noexcept;

	// Convert to generic RGB channel values.
	[[nodiscard]]
	GenericRgb<F> to_generic_rgb() const noexcept;

	// Convert to linear ProPhoto RGB RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgbLinear<F> to_linear() const noexcept;

	// Convert to ProPhoto RGB HSV (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgbHsv<F> to_hsv() const noexcept;

	// Convert to ProPhoto RGB HSL (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgbHsl<F> to_hsl() const noexcept;

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
 * Return this colour with alpha multiplied into the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> ProPhotoRgb<F>::premultiply_alpha() const noexcept {
	return ProPhotoRgb(red * alpha, green * alpha, blue * alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with alpha divided out of the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> ProPhotoRgb<F>::un_premultiply_alpha() const noexcept {
	const F safe_alpha = if_equal(alpha, F(0), F(1), alpha);
	return ProPhotoRgb(red / safe_alpha, green / safe_alpha, blue / safe_alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> ProPhotoRgb<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return ProPhotoRgb(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every component clipped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> ProPhotoRgb<F>::clip() const noexcept {
	return clamp();
}

/**************************************************************************************************
 * Return this colour with red, green, and blue clipped to the 0..1 range, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> ProPhotoRgb<F>::clip_rgb() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return ProPhotoRgb(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), alpha);
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> ProPhotoRgb<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return ProPhotoRgb(max(zero, red), max(zero, green), max(zero, blue), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline ProPhotoRgb<F> ProPhotoRgb<F>::sanitise_floats() const noexcept {
	return ProPhotoRgb(detail::sanitise_component(red), detail::sanitise_component(green), detail::sanitise_component(blue), detail::sanitise_component(alpha));
}

/**************************************************************************************************
 * Return this colour as generic RGB channel values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> ProPhotoRgb<F>::to_generic_rgb() const noexcept {
	return GenericRgb<F>(red, green, blue, alpha);
}

} // namespace mt
