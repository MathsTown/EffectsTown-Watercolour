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

Type: Srgb

Specification:
- gamma-2.4 encoded sRGB RGB.
- Specification: CSS Color 4 predefined sRGB / IEC 61966-2-1
  https://www.w3.org/TR/css-color-4/#predefined-sRGB

Reference white:
- sRGB uses D65, with chromaticity x = 0.3127, y = 0.3290.
- This implementation converts through this library's CIE XYZ D65 hub.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- to_linear() applies the sRGB piecewise gamma 2.4 transfer function, including the CSS Color 4 extended handling for negative values.
- to_xyz() converts through SrgbLinear and the sRGB D65 RGB-to-XYZ matrix.
- to_hsv() and to_hsl() are conventional component-space transforms over gamma-2.4 encoded sRGB values.

Components:
- red, green, blue: nominal 0..1 gamma-2.4 encoded colour channels; out-of-range values are preserved unless clamped.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, grey, RGB, RGBA.
- Operations: premultiply_alpha(), un_premultiply_alpha(), clamp(), clip(), clip_rgb(), fit_gamut(), no_negatives(), sanitise_floats().
- Conversion: to_linear(), to_hsv(), to_hsl(), to_xyz().

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"
#include "generic_rgb.h"

namespace mt {

// Forward declaration of the linear sRGBA type.
template <ColourComponent F> struct SrgbLinear;

// Forward declaration of the gamma-2.4 encoded sRGB HSV type.
template <ColourComponent F> struct SrgbHsv;

// Forward declaration of the gamma-2.4 encoded sRGB HSL type.
template <ColourComponent F> struct SrgbHsl;

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

/**************************************************************************************************
 * Srgb floating point or Simd floating point colour type.
 * Colourspace : sRGB (gamma-2.4 encoded)
 * Definition  : https://www.w3.org/TR/css-color-4/#predefined-sRGB
 * PixelOrder  : RGBA
 * Alpha       : Assumed not-premultiplied for all operations except unpremultiply_alpha().
 * ************************************************************************************************/
template <ColourComponent F> struct Srgb {
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
	Srgb() = default;

	// Default destructor.
	~Srgb() = default;

	// Construct with a grey value and full alpha.
	Srgb(F grey) noexcept : red(grey), green(grey), blue(grey), alpha(F(1)) { }

	// Construct with red, green, and blue values and full alpha.
	Srgb(F red, F green, F blue) noexcept : red(red), green(green), blue(blue), alpha(F(1)) { }

	// Construct with red, green, blue, and alpha values.
	Srgb(F red, F green, F blue, F alpha) noexcept : red(red), green(green), blue(blue), alpha(alpha) { }

	// Explicitly construct from generic RGB channel values.
	explicit Srgb(const GenericRgb<F>& colour) noexcept : red(colour.red), green(colour.green), blue(colour.blue), alpha(colour.alpha) { }

	// Premultiply alpha into the red, green, and blue components.
	[[nodiscard]]
	Srgb premultiply_alpha() const noexcept;

	// Un-premultiply alpha from the red, green, and blue components.
	[[nodiscard]]
	Srgb un_premultiply_alpha() const noexcept;

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	Srgb clamp() const noexcept;

	// Clip all components to the 0..1 range.
	[[nodiscard]]
	Srgb clip() const noexcept;

	// Clip the red, green, and blue components to the 0..1 range, preserving alpha.
	[[nodiscard]]
	Srgb clip_rgb() const noexcept;

	// Fit this colour into its RGB gamut (implemented in gamut.h).
	[[nodiscard]]
	Srgb fit_gamut() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	Srgb no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	Srgb sanitise_floats() const noexcept;

	// Convert to generic RGB channel values.
	[[nodiscard]]
	GenericRgb<F> to_generic_rgb() const noexcept;

	// Convert to linear sRGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	SrgbLinear<F> to_linear() const noexcept;

	// Convert to sRGB HSV (implemented in colour_conversions.h).
	[[nodiscard]]
	SrgbHsv<F> to_hsv() const noexcept;

	// Convert to sRGB HSL (implemented in colour_conversions.h).
	[[nodiscard]]
	SrgbHsl<F> to_hsl() const noexcept;

	// Convert to CIE XYZ D65 (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz() const noexcept;
};

/**************************************************************************************************
 * Return this colour with alpha multiplied into the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Srgb<F> Srgb<F>::premultiply_alpha() const noexcept {
	return Srgb(red * alpha, green * alpha, blue * alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with alpha divided out of the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Srgb<F> Srgb<F>::un_premultiply_alpha() const noexcept {
	const F safe_alpha = if_equal(alpha, F(0), F(1), alpha);
	return Srgb(red / safe_alpha, green / safe_alpha, blue / safe_alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Srgb<F> Srgb<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Srgb(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every component clipped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Srgb<F> Srgb<F>::clip() const noexcept {
	return clamp();
}

/**************************************************************************************************
 * Return this colour with red, green, and blue clipped to the 0..1 range, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Srgb<F> Srgb<F>::clip_rgb() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Srgb(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), alpha);
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Srgb<F> Srgb<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return Srgb(max(zero, red), max(zero, green), max(zero, blue), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Srgb<F> Srgb<F>::sanitise_floats() const noexcept {
	return Srgb(detail::sanitise_component(red), detail::sanitise_component(green), detail::sanitise_component(blue), detail::sanitise_component(alpha));
}

/**************************************************************************************************
 * Return this colour as generic RGB channel values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> Srgb<F>::to_generic_rgb() const noexcept {
	return GenericRgb<F>(red, green, blue, alpha);
}

} // namespace mt
